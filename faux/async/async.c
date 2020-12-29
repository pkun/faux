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
	async->i_pos = 0;

	// Write (Output)
	async->stall_cb = NULL;
	async->stall_udata = NULL;
	async->overflow = 10000000l; // ~ 10M
	async->o_list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, faux_free);
	async->o_pos = 0;

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
