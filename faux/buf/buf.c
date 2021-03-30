/** @file buf.c
 * @brief Dynamic buffer.
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "faux/faux.h"
#include "faux/str.h"
#include "faux/buf.h"

#define DATA_CHUNK 4096

struct faux_buf_s {
	size_t limit;
	faux_list_t *list;
	size_t rpos;
	size_t wpos;
	size_t size;
};


/** @brief Create new buf I/O object.
 *
 * Constructor gets associated file descriptor to operate on it. File
 * descriptor must be nonblocked. If not so then constructor will set
 * nonblock flag itself.
 *
 * @param [in] fd File descriptor.
 * @return Allocated object or NULL on error.
 */
faux_buf_t *faux_buf_new(void)
{
	faux_buf_t *buf = NULL;

	buf = faux_zmalloc(sizeof(*buf));
	assert(buf);
	if (!buf)
		return NULL;

	// Init
	buf->limit = FAUX_BUF_UNLIMITED;
	buf->list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, faux_free);
	buf->rpos = 0;
	buf->wpos = 0;
	buf->size = 0;

	return buf;
}


/** @brief Free buf I/O object.
 *
 * @param [in] buf I/O object.
 */
void faux_buf_free(faux_buf_t *buf)
{
	if (!buf)
		return;

	faux_list_free(buf->list);

	faux_free(buf);
}



ssize_t faux_buf_limit(const faux_buf_t *buf)
{
	assert(buf);
	if (!buf)
		return -1;

	return buf->limit;
}


/** @brief Set size limit.
 *
 * Read limits define conditions when the read callback will be executed.
 * Buffer must contain data amount greater or equal to "min" value. Callback
 * will not get data amount greater than "max" value. If min == max then
 * callback will be executed with fixed data size. The "max" value can be "0".
 * It means indefinite i.e. data transferred to callback can be really large.
 *
 * @param [in] buf Allocated and initialized buf I/O object.
 * @param [in] min Minimal data amount.
 * @param [in] max Maximal data amount. The "0" means indefinite.
 * @return BOOL_TRUE - success, BOOL_FALSE - error.
 */
bool_t faux_buf_set_limit(faux_buf_t *buf, size_t limit)
{
	assert(buf);
	if (!buf)
		return BOOL_FALSE;

	buf->limit = limit;

	return BOOL_TRUE;
}


#if 0


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


/** @brief buf data write.
 *
 * All given data will be stored to internal buffer (list of data chunks).
 * Then function will try to write stored data to file descriptor in
 * non-blocking mode. Note some data can be left within buffer. In this case
 * the "stall" callback will be executed to inform about it. To try to write
 * the rest of the data user can be call faux_buf_out() function. Both
 * functions will not block.
 *
 * @param [in] buf Allocated and initialized buf I/O object.
 * @param [in] data Data buffer to write.
 * @param [in] len Data length to write.
 * @return Length of stored/writed data or < 0 on error.
 */
ssize_t faux_buf_write(faux_buf_t *buf, void *data, size_t len)
{
	void *new_chunk = NULL;
	size_t data_left = len;

	assert(buf);
	if (!buf)
		return -1;
	assert(data);
	if (!data)
		return -1;

	while (data_left != 0) {
		ssize_t bytes_free = 0;
		size_t copy_len = 0;
		char *chunk_ptr = NULL;

		// Allocate new chunk if necessary
		bytes_free = free_space(buf->o_list, buf->o_wpos);
		if (bytes_free < 0)
			return -1;
		if (0 == bytes_free) {
			new_chunk = faux_malloc(DATA_CHUNK);
			assert(new_chunk);
			faux_list_add(buf->o_list, new_chunk);
			buf->o_wpos = 0;
			bytes_free = free_space(buf->o_list, buf->o_wpos);
		}

		// Copy data
		chunk_ptr = faux_list_data(faux_list_tail(buf->o_list));
		copy_len = (data_left < (size_t)bytes_free) ? data_left : (size_t)bytes_free;
		memcpy(chunk_ptr + buf->o_wpos, data + len - data_left,
			copy_len);
		buf->o_wpos += copy_len;
		data_left -= copy_len;
		buf->o_size += copy_len;
		if (buf->o_size >= buf->o_overflow)
			return -1;
	}

	// Try to real write data to fd in nonblocked mode
	faux_buf_out(buf);

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
 * Previously data must be written to internal buffer by faux_buf_write()
 * function. But some data can be left within internal buffer because can't be
 * written to fd in non-blocking mode. This function tries to write the rest of
 * data to fd in non-blocking mode. So function doesn't block. It can be called
 * after select() or poll() if fd is ready to be written to. If function can't
 * to write all buffer to fd it executes "stall" callback to inform about it.
 *
 * @param [in] buf Allocated and initialized buf I/O object.
 * @return Length of data actually written or < 0 on error.
 */
ssize_t faux_buf_out(faux_buf_t *buf)
{
	ssize_t total_written = 0;

	assert(buf);
	if (!buf)
		return -1;

	while (buf->o_size > 0) {
		faux_list_node_t *node = NULL;
		char *chunk_ptr = NULL;
		ssize_t data_to_write = 0;
		ssize_t bytes_written = 0;
		bool_t postpone = BOOL_FALSE;

		node = faux_list_head(buf->o_list);
		if (!node) // List is empty while o_size > 0
			return -1;
		chunk_ptr = faux_list_data(node);
		data_to_write = data_avail(buf->o_list,
			buf->o_rpos, buf->o_wpos);
		if (data_to_write <= 0) // Strange case
			return -1;

		bytes_written = write(buf->fd, chunk_ptr + buf->o_rpos,
			data_to_write);
		if (bytes_written > 0) {
			buf->o_size -= bytes_written;
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
			buf->o_rpos += bytes_written;
			// Postpone next read
			postpone = BOOL_TRUE;
		}

		// Postponed
		if (postpone) {
			// Execute callback
			if (buf->stall_cb)
				buf->stall_cb(buf, buf->o_size,
					buf->stall_udata);
			break;
		}

		// Not postponed. Current chunk was fully written. So
		// remove it from list.
		buf->o_rpos = 0;
		faux_list_del(buf->o_list, node);
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
 * @param [in] buf Allocated and initialized buf I/O object.
 * @return Length of data actually readed or < 0 on error.
 */
ssize_t faux_buf_in(faux_buf_t *buf)
{
	void *new_chunk = NULL;
	ssize_t total_readed = 0;
	ssize_t bytes_readed = 0;
	ssize_t bytes_free = 0; // Free space within current (last) chunk

	assert(buf);
	if (!buf)
		return -1;

	do {
		char *chunk_ptr = NULL;

		// Allocate new chunk if necessary
		bytes_free = free_space(buf->i_list, buf->i_wpos);
		if (bytes_free < 0)
			return -1;
		if (0 == bytes_free) { // We need to allocate additional chunk
			new_chunk = faux_malloc(DATA_CHUNK);
			assert(new_chunk);
			faux_list_add(buf->i_list, new_chunk);
			buf->i_wpos = 0;
			bytes_free = free_space(buf->i_list, buf->i_wpos);
		}

		// Read data to last chunk
		chunk_ptr = faux_list_data(faux_list_tail(buf->i_list));
		bytes_readed = read(buf->fd, chunk_ptr + buf->i_wpos, bytes_free);
		if (bytes_readed < 0) {
			if ( // Something went wrong
				(errno != EINTR) &&
				(errno != EAGAIN) &&
				(errno != EWOULDBLOCK)
			)
				return -1;
		}
		if (bytes_readed > 0) {
			buf->i_wpos += bytes_readed;
			buf->i_size += bytes_readed;
			total_readed += bytes_readed;
		}
		if (buf->i_size >= buf->i_overflow)
			return -1;

		// Check for amount of stored data
		while (buf->i_size >= buf->min) {

			size_t copy_len = 0;
			size_t full_size = 0;
			char *buf = NULL;
			char *buf_ptr = NULL;

			if (FAUX_buf_UNLIMITED == buf->max) { // Indefinite
				copy_len = buf->i_size; // Take all data
			} else {
				copy_len = (buf->i_size < buf->max) ?
					buf->i_size : buf->max;
			}

			full_size = copy_len; // Save full length value
			buf = faux_malloc(full_size);
			buf_ptr = buf;
			while (copy_len > 0) {
				size_t data_to_write = 0;
				faux_list_node_t *node = faux_list_head(buf->i_list);
				char *chunk_ptr = NULL;

				if (!node) // Something went wrong
					return -1;
				chunk_ptr = faux_list_data(node);
				data_to_write = data_avail(buf->i_list,
					buf->i_rpos, buf->i_wpos);
				if (copy_len < data_to_write)
					data_to_write = copy_len;
				memcpy(buf_ptr, chunk_ptr + buf->i_rpos,
					data_to_write);
				copy_len -= data_to_write;
				buf->i_size -= data_to_write;
				buf->i_rpos += data_to_write;
				buf_ptr += data_to_write;
				if (data_avail(buf->i_list,
					buf->i_rpos, buf->i_wpos) <= 0) {
					buf->i_rpos = 0;
					faux_list_del(buf->i_list, node);
				}
			}
			// Execute callback
			if (buf->read_cb)
				buf->read_cb(buf, buf,
					full_size, buf->read_udata);

		}

	} while (bytes_readed == bytes_free);

	return total_readed;
}
#endif
