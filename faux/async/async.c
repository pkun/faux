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
