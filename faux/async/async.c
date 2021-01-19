/** @file async.c
 * @brief Asynchronous input and output.
 *
 * Class uses non-blocking input and output and has internal input and output
 * buffers. Class has associated file descriptor to work with it.
 *
 * For async writing user uses faux_async_write() function. It writes all
 * given data to internal buffer and then tries to really write it to file
 * descriptor. If not all data was written in non-blocking mode then function
 * executes special callback "stall" function to inform us about non-empty
 * output buffer. "Stall" callback function can make programm to inspect fd
 * for write possibility. Then programm must call faux_async_out() to really
 * write the rest of the data to fd. Function also can execute "stall" callback.
 *
 * For async reading user can call faux_sync_in(). For example this function
 * can be called after select() or poll() when data is available on interested
 * fd. Function reads data in non-blocking mode and stores to internal buffer.
 * User can specify read "limits" - min and max. When amount of reded data is
 * greater or equal to "min" limit then "read" callback will be executed.
 * The "read" callback will get allocated buffer with received data. The
 * length of the data is greater or equal to "min" limit and less or equal to
 * "max" limit.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "faux/faux.h"
#include "faux/str.h"
#include "faux/net.h"
#include "faux/async.h"

#include "private.h"

#define DATA_CHUNK 4096

/** @brief Create new async I/O object.
 *
 * Constructor gets associated file descriptor to operate on it. File
 * descriptor must be nonblocked. If not so then constructor will set
 * nonblock flag itself.
 *
 * @param [in] fd File descriptor.
 * @return Allocated object or NULL on error.
 */
faux_async_t *faux_async_new(int fd)
{
	faux_async_t *async = NULL;
	int fflags = 0;

	// Prepare FD
	if (fd < 0) // Illegal fd
		return NULL;
	if ((fflags = fcntl(fd, F_GETFL)) == -1)
		return NULL;
	if (fcntl(fd, F_SETFL, fflags | O_NONBLOCK) == -1)
		return NULL;

	async = faux_zmalloc(sizeof(*async));
	assert(async);
	if (!async)
		return NULL;

	// Init
	async->fd = fd;

	// Read (Input)
	async->read_cb = NULL;
	async->read_udata = NULL;
	async->min = 1;
	async->max = FAUX_ASYNC_UNLIMITED;
	async->i_list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, faux_free);
	async->i_rpos = 0;
	async->i_wpos = 0;
	async->i_size = 0;
	async->i_overflow = 10000000l; // ~ 10M

	// Write (Output)
	async->stall_cb = NULL;
	async->stall_udata = NULL;
	async->o_overflow = 10000000l; // ~ 10M
	async->o_list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, faux_free);
	async->o_rpos = 0;
	async->o_wpos = 0;
	async->o_size = 0;

	return async;
}


/** @brief Free async I/O object.
 *
 * @param [in] Async I/O object.
 */
void faux_async_free(faux_async_t *async)
{
	if (!async)
		return;

	faux_list_free(async->i_list);
	faux_list_free(async->o_list);

	faux_free(async);
}


/** @brief Get file descriptor from async I/O object.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @return Serviced file descriptor.
 */
int faux_async_fd(const faux_async_t *async)
{
	assert(async);
	if (!async)
		return -1;

	return async->fd;
}


/** @brief Set read callback and associated user data.
 *
 * If callback function pointer is NULL then class will drop all readed data.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @param [in] read_cb Read callback.
 * @param [in] user_data Associated user data.
 */
void faux_async_set_read_cb(faux_async_t *async,
	faux_async_read_cb_f read_cb, void *user_data)
{
	assert(async);
	if (!async)
		return;

	async->read_cb = read_cb;
	async->read_udata = user_data;
}


/** @brief Set read limits.
 *
 * Read limits define conditions when the read callback will be executed.
 * Buffer must contain data amount greater or equal to "min" value. Callback
 * will not get data amount greater than "max" value. If min == max then
 * callback will be executed with fixed data size. The "max" value can be "0".
 * It means indefinite i.e. data transferred to callback can be really large.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @param [in] min Minimal data amount.
 * @param [in] max Maximal data amount. The "0" means indefinite.
 * @return BOOL_TRUE - success, BOOL_FALSE - error.
 */
bool_t faux_async_set_read_limits(faux_async_t *async, size_t min, size_t max)
{
	assert(async);
	if (!async)
		return BOOL_FALSE;
	if (min < 1)
		return BOOL_FALSE;
	if ((min > max) && (max != 0))
		return BOOL_FALSE;

	async->min = min;
	async->max = max;

	return BOOL_TRUE;
}


/** @brief Set stall callback and associated user data.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @param [in] stall_cb Stall callback.
 * @param [in] user_data Associated user data.
 */
void faux_async_set_stall_cb(faux_async_t *async,
	faux_async_stall_cb_f stall_cb, void *user_data)
{
	assert(async);
	if (!async)
		return;

	async->stall_cb = stall_cb;
	async->stall_udata = user_data;
}


/** @brief Set write overflow value.
 *
 * "Overflow" is a value when engine consider data consumer as a stalled.
 * Data gets into the async I/O object buffer but object can't write it to
 * serviced fd for too long time. So it accumulates great amount of data.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @param [in] overflow Overflow value.
 */
void faux_async_set_write_overflow(faux_async_t *async, size_t overflow)
{
	assert(async);
	if (!async)
		return;

	async->o_overflow = overflow;
}


/** @brief Set read overflow value.
 *
 * "Overflow" is a value when engine consider data consumer as a stalled.
 * Data gets into the async I/O object buffer but object can't write it to
 * serviced fd for too long time. So it accumulates great amount of data.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @param [in] overflow Overflow value.
 */
void faux_async_set_read_overflow(faux_async_t *async, size_t overflow)
{
	assert(async);
	if (!async)
		return;

	async->i_overflow = overflow;
}


/** @brief Get amount of unused space within current data chunk.
 *
 * Inernal static function.
 *
 * @param [in] list Internal buffer (list of chunks) to inspect.
 * @param [in] pos Current write position within last chunk
 * @return Size of unused space or < 0 on error.
 */
static ssize_t free_space(faux_list_t *list, size_t pos)
{
	if (!list)
		return -1;

	if (faux_list_len(list) == 0)
		return 0;

	return (DATA_CHUNK - pos);
}


/** @brief Async data write.
 *
 * All given data will be stored to internal buffer (list of data chunks).
 * Then function will try to write stored data to file descriptor in
 * non-blocking mode. Note some data can be left within buffer. In this case
 * the "stall" callback will be executed to inform about it. To try to write
 * the rest of the data user can be call faux_async_out() function. Both
 * functions will not block.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @param [in] data Data buffer to write.
 * @param [in] len Data length to write.
 * @return Length of stored/writed data or < 0 on error.
 */
ssize_t faux_async_write(faux_async_t *async, void *data, size_t len)
{
	void *new_chunk = NULL;
	size_t data_left = len;

	assert(async);
	if (!async)
		return -1;
	assert(data);
	if (!data)
		return -1;

	while (data_left != 0) {
		ssize_t bytes_free = 0;
		size_t copy_len = 0;
		char *chunk_ptr = NULL;

		// Allocate new chunk if necessary
		bytes_free = free_space(async->o_list, async->o_wpos);
		if (bytes_free < 0)
			return -1;
		if (0 == bytes_free) {
			new_chunk = faux_malloc(DATA_CHUNK);
			assert(new_chunk);
			faux_list_add(async->o_list, new_chunk);
			async->o_wpos = 0;
			bytes_free = free_space(async->o_list, async->o_wpos);
		}

		// Copy data
		chunk_ptr = faux_list_data(faux_list_tail(async->o_list));
		copy_len = (data_left < (size_t)bytes_free) ? data_left : (size_t)bytes_free;
		memcpy(chunk_ptr + async->o_wpos, data + len - data_left,
			copy_len);
		async->o_wpos += copy_len;
		data_left -= copy_len;
		async->o_size += copy_len;
		if (async->o_size >= async->o_overflow)
			return -1;
	}

	// Try to real write data to fd in nonblocked mode
	faux_async_out(async);

	return len;
}


/** @brief Get amount of available data within first chunk.
 *
 * Inernal static function.
 *
 * @param [in] list Internal buffer (list of chunks) to inspect.
 * @param [in] rpos Current read position within chunk.
 * @param [in] wpos Current write position within chunk.
 * @return Available data length or < 0 on error.
 */
static ssize_t data_avail(faux_list_t *list, size_t rpos, size_t wpos)
{
	size_t len = 0;

	if (!list)
		return -1;

	len = faux_list_len(list);
	if (len == 0)
		return 0;
	if (len > 1)
		return (DATA_CHUNK - rpos);

	// Single chunk
	return (wpos - rpos);
}


/** @brief Write output buffer to fd in non-blocking mode.
 *
 * Previously data must be written to internal buffer by faux_async_write()
 * function. But some data can be left within internal buffer because can't be
 * written to fd in non-blocking mode. This function tries to write the rest of
 * data to fd in non-blocking mode. So function doesn't block. It can be called
 * after select() or poll() if fd is ready to be written to. If function can't
 * to write all buffer to fd it executes "stall" callback to inform about it.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @return Length of data actually written or < 0 on error.
 */
ssize_t faux_async_out(faux_async_t *async)
{
	ssize_t total_written = 0;

	assert(async);
	if (!async)
		return -1;

	while (async->o_size > 0) {
		faux_list_node_t *node = NULL;
		char *chunk_ptr = NULL;
		ssize_t data_to_write = 0;
		ssize_t bytes_written = 0;
		bool_t postpone = BOOL_FALSE;

		node = faux_list_head(async->o_list);
		if (!node) // List is empty while o_size > 0
			return -1;
		chunk_ptr = faux_list_data(node);
		data_to_write = data_avail(async->o_list,
			async->o_rpos, async->o_wpos);
		if (data_to_write <= 0) // Strange case
			return -1;

		bytes_written = write(async->fd, chunk_ptr + async->o_rpos,
			data_to_write);
		if (bytes_written > 0) {
			async->o_size -= bytes_written;
			total_written += bytes_written;
		}

		if (bytes_written < 0) {
			if ( // Something went wrong
				(errno != EINTR) &&
				(errno != EAGAIN) &&
				(errno != EWOULDBLOCK)
			)
				return -1;
			// Postpone next read
			postpone = BOOL_TRUE;

		// Not whole data block was written
		} else if (bytes_written != data_to_write) {
			async->o_rpos += bytes_written;
			// Postpone next read
			postpone = BOOL_TRUE;
		}

		// Postponed
		if (postpone) {
			// Execute callback
			if (async->stall_cb)
				async->stall_cb(async, async->o_size,
					async->stall_udata);
			break;
		}

		// Not postponed. Current chunk was fully written. So
		// remove it from list.
		async->o_rpos = 0;
		faux_list_del(async->o_list, node);
	}

	return total_written;
}


/** @brief Read data and store it to internal buffer in non-blocking mode.
 *
 * Reads fd and puts data to internal buffer. It can't be blocked. If length of
 * data stored within internal buffer is greater or equal than "min" limit then
 * function will execute "read" callback. It allocates linear buffer, copies
 * data to it and give it to callback. Note this function will never free
 * allocated buffer. So callback must do it or it must be done later. Function
 * will not allocate buffer larger than "max" read limit. If "max" limit is "0"
 * (it means indefinite) then function will pass all available data to callback.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @return Length of data actually readed or < 0 on error.
 */
ssize_t faux_async_in(faux_async_t *async)
{
	void *new_chunk = NULL;
	ssize_t total_readed = 0;
	ssize_t bytes_readed = 0;
	ssize_t bytes_free = 0; // Free space within current (last) chunk

	assert(async);
	if (!async)
		return -1;

	do {
		char *chunk_ptr = NULL;

		// Allocate new chunk if necessary
		bytes_free = free_space(async->i_list, async->i_wpos);
		if (bytes_free < 0)
			return -1;
		if (0 == bytes_free) { // We need to allocate additional chunk
			new_chunk = faux_malloc(DATA_CHUNK);
			assert(new_chunk);
			faux_list_add(async->i_list, new_chunk);
			async->i_wpos = 0;
			bytes_free = free_space(async->i_list, async->i_wpos);
		}

		// Read data to last chunk
		chunk_ptr = faux_list_data(faux_list_tail(async->i_list));
		bytes_readed = read(async->fd, chunk_ptr + async->i_wpos, bytes_free);
		if (bytes_readed < 0) {
			if ( // Something went wrong
				(errno != EINTR) &&
				(errno != EAGAIN) &&
				(errno != EWOULDBLOCK)
			)
				return -1;
		}
		if (bytes_readed > 0) {
			async->i_wpos += bytes_readed;
			async->i_size += bytes_readed;
			total_readed += bytes_readed;
		}
		if (async->i_size >= async->i_overflow)
			return -1;

		// Check for amount of stored data
		while (async->i_size >= async->min) {

			size_t copy_len = async->min;
			size_t full_size = 0;
			char *buf = NULL;
			char *buf_ptr = NULL;

			if (FAUX_ASYNC_UNLIMITED == async->max) { // Indefinite
				copy_len = async->i_size; // Take all data
			} else {
				copy_len = (async->i_size < async->max) ?
					async->i_size : async->max;
			}

			full_size = copy_len; // Save full length value
			buf = faux_malloc(full_size);
			buf_ptr = buf;
			while (copy_len > 0) {
				size_t data_to_write = 0;
				faux_list_node_t *node = faux_list_head(async->i_list);
				char *chunk_ptr = NULL;

				if (!node) // Something went wrong
					return -1;
				chunk_ptr = faux_list_data(node);
				data_to_write = data_avail(async->i_list,
					async->i_rpos, async->i_wpos);
				if (copy_len < data_to_write)
					data_to_write = copy_len;
				memcpy(buf_ptr, chunk_ptr + async->i_rpos,
					data_to_write);
				copy_len -= data_to_write;
				async->i_size -= data_to_write;
				async->i_rpos += data_to_write;
				buf_ptr += data_to_write;
				if (data_avail(async->i_list,
					async->i_rpos, async->i_wpos) <= 0) {
					async->i_rpos = 0;
					faux_list_del(async->i_list, node);
				}
			}
			// Execute callback
			if (async->read_cb)
				async->read_cb(async, buf,
					full_size, async->read_udata);

		}

	} while (bytes_readed == bytes_free);

	return total_readed;
}