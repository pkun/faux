/** @file async.c
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
	async->max = 0; // Indefinite
	async->i_list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, faux_free);
	async->i_rpos = 0;
	async->i_wpos = 0;
	async->i_size = 0;

	// Write (Output)
	async->stall_cb = NULL;
	async->stall_udata = NULL;
	async->overflow = 10000000l; // ~ 10M
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


/** @brief Set overflow value.
 *
 * "Overflow" is a value when engine consider data consumer as a stalled.
 * Data gets into the async I/O object buffer but object can't write it to
 * serviced fd for too long time. So it accumulates great amount of data.
 *
 * @param [in] async Allocated and initialized async I/O object.
 * @param [in] overflow Overflow value.
 */
void faux_async_set_overflow(faux_async_t *async, size_t overflow)
{
	assert(async);
	if (!async)
		return;

	async->overflow = overflow;
}


static ssize_t free_space(faux_list_t *list, size_t pos)
{
	if (!list)
		return -1;

	if (faux_list_len(list) == 0)
		return 0;

	return (DATA_CHUNK - pos);
}


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
		if (async->o_size >= async->overflow)
			return -1;
	}

	// Try to real write data to fd in nonblocked mode
	faux_async_out(async);

	return len;
}


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
		chunk_ptr = faux_list_data(faux_list_head(async->o_list));
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
