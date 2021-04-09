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
#include "faux/buf.h"
#include "faux/net.h"
#include "faux/async.h"

#include "private.h"

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
	async->ibuf = faux_buf_new(DATA_CHUNK);
	faux_buf_set_limit(async->ibuf, FAUX_ASYNC_IN_OVERFLOW);

	// Write (Output)
	async->stall_cb = NULL;
	async->stall_udata = NULL;
	async->obuf = faux_buf_new(DATA_CHUNK);
	faux_buf_set_limit(async->obuf, FAUX_ASYNC_OUT_OVERFLOW);

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

	faux_buf_free(async->ibuf);
	faux_buf_free(async->obuf);

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
	faux_async_read_cb_fn read_cb, void *user_data)
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
	faux_async_stall_cb_fn stall_cb, void *user_data)
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

	faux_buf_set_limit(async->obuf, overflow);
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

	faux_buf_set_limit(async->ibuf, overflow);
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
	ssize_t data_written = len;

	assert(async);
	if (!async)
		return -1;
	assert(data);
	if (!data)
		return -1;

	data_written = faux_buf_write(async->obuf, data, len);
	if (data_written < 0)
		return -1;

	// Try to real write data to fd in nonblocked mode
	faux_async_out(async);

	return len;
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
	ssize_t avail = 0;

	assert(async);
	if (!async)
		return -1;

	while ((avail = faux_buf_len(async->obuf)) > 0) {
		ssize_t data_to_write = 0;
		ssize_t bytes_written = 0;
		bool_t postpone = BOOL_FALSE;
		void *data = NULL;

		data_to_write = faux_buf_dread_lock_easy(async->obuf, &data);
		if (data_to_write <= 0)
			return -1;

		bytes_written = write(async->fd, data, data_to_write);
		if (bytes_written > 0) {
			total_written += bytes_written;
			faux_buf_dread_unlock_easy(async->obuf, bytes_written);
		} else {
			faux_buf_dread_unlock_easy(async->obuf, 0);
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
			// Postpone next read
			postpone = BOOL_TRUE;
		}

		// Postponed
		if (postpone) {
			// Execute callback
			if (async->stall_cb)
				async->stall_cb(async,
					faux_buf_len(async->obuf),
					async->stall_udata);
			break;
		}
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
	ssize_t total_readed = 0;
	ssize_t bytes_readed = 0;
	ssize_t locked_len = 0;

	assert(async);
	if (!async)
		return -1;

	do {
		void *data = NULL;
		size_t bytes_stored = 0;

		locked_len = faux_buf_dwrite_lock_easy(async->ibuf, &data);
		if (locked_len <= 0)
			return -1;
		// Read data
		bytes_readed = read(async->fd, data, locked_len);
		if (bytes_readed < 0) {
			if ( // Something went wrong
				(errno != EINTR) &&
				(errno != EAGAIN) &&
				(errno != EWOULDBLOCK)
			)
				return -1;
		}
		faux_buf_dwrite_unlock_easy(async->ibuf, bytes_readed);
		total_readed += bytes_readed;

		// Check for amount of stored data
		while ((bytes_stored = faux_buf_len(async->ibuf)) >= async->min) {
			size_t copy_len = 0;
			char *buf = NULL;

			if (FAUX_ASYNC_UNLIMITED == async->max) { // Indefinite
				copy_len = bytes_stored; // Take all data
			} else {
				copy_len = (bytes_stored < async->max) ?
					bytes_stored : async->max;
			}
			buf = faux_malloc(copy_len);
			assert(buf);
			faux_buf_read(async->ibuf, buf, copy_len);

			// Execute callback
			if (async->read_cb)
				async->read_cb(async, buf,
					copy_len, async->read_udata);
		}
	} while (bytes_readed == locked_len);

	return total_readed;
}
