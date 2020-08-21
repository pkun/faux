/** @file pollfd.c
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <poll.h>

#include "faux/faux.h"
#include "faux/net.h"
#include "faux/vec.h"
#include "private.h"


/** @brief Function to search specified fd within pollfd structures.
 */
static int cmp_by_fd(const void *key, const void *item)
{
	int k = *(int *)key;
	struct pollfd *i = (struct pollfd *)item;
	if (k == i->fd)
		return 0;
	return -1;
}


/** @brief Allocates memory for faux_pollfd_t object.
 *
 * @return Allocated faux_pollfd_t object or NULL on error.
 */
faux_pollfd_t *faux_pollfd_new(void)
{
	faux_pollfd_t *faux_pollfd = NULL;

	faux_pollfd = faux_zmalloc(sizeof(*faux_pollfd));
	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;

	faux_pollfd->vec = faux_vec_new(sizeof(struct pollfd), cmp_by_fd);

	return faux_pollfd;
}


/** @brief Frees previously allocated faux_pollfd_t object.
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 */
void faux_pollfd_free(faux_pollfd_t *faux_pollfd)
{
	if (!faux_pollfd)
		return;
	faux_vec_free(faux_pollfd->vec);
	faux_free(faux_pollfd);
}


struct pollfd *faux_pollfd_vector(faux_pollfd_t *faux_pollfd)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;

	return faux_vec_data(faux_pollfd->vec);
}


size_t faux_pollfd_len(faux_pollfd_t *faux_pollfd)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return 0;

	return faux_vec_len(faux_pollfd->vec);
}


struct pollfd *faux_pollfd_item(faux_pollfd_t *faux_pollfd, unsigned int index)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;

	return (struct pollfd *)faux_vec_item(faux_pollfd->vec, index);
}


struct pollfd *faux_pollfd_find(faux_pollfd_t *faux_pollfd, int fd)
{
	int index = 0;

	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;
	assert(fd >= 0);
	if (fd < 0)
		return NULL;

	index = faux_vec_find(faux_pollfd->vec, &fd, 0);
	if (index < 0)
		return NULL;

	return (struct pollfd *)faux_vec_item(faux_pollfd->vec, index);
}

struct pollfd *faux_pollfd_add(faux_pollfd_t *faux_pollfd, int fd)
{
	struct pollfd *pollfd = NULL;

	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;
	assert(fd >= 0);
	if (fd < 0)
		return NULL;

	// Don't add duplicate fd
	pollfd = faux_pollfd_find(faux_pollfd, fd);
	if (pollfd)
		return pollfd;

	// Create new item
	pollfd = faux_vec_add(faux_pollfd->vec);
	assert(pollfd);
	if (!pollfd)
		return NULL;
	pollfd->fd = fd;

	return pollfd;
}


int faux_pollfd_del_by_fd(faux_pollfd_t *faux_pollfd, int fd)
{
	int index = 0;

	assert(faux_pollfd);
	if (!faux_pollfd)
		return -1;
	assert(fd >= 0);
	if (fd < 0)
		return -1;

	index = faux_vec_find(faux_pollfd->vec, &fd, 0);
	if (index < 0) // Not found
		return -1;

	return faux_vec_del(faux_pollfd->vec, index);
}


int faux_pollfd_del_by_index(faux_pollfd_t *faux_pollfd, unsigned int index)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return -1;

	return faux_vec_del(faux_pollfd->vec, index);
}


void faux_pollfd_init_iterator(faux_pollfd_t *faux_pollfd, faux_pollfd_iterator_t *iterator)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return;
	if (!iterator)
		return;
	*iterator = 0;
}


struct pollfd *faux_pollfd_each(faux_pollfd_t *faux_pollfd, faux_pollfd_iterator_t *iterator)
{
	unsigned int old_iterator = 0;

	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;
	if (!iterator)
		return NULL;

	old_iterator = *iterator;
	(*iterator)++;

	return faux_pollfd_item(faux_pollfd, old_iterator);
}


struct pollfd *faux_pollfd_each_active(faux_pollfd_t *faux_pollfd, faux_pollfd_iterator_t *iterator)
{
	struct pollfd *pollfd = NULL;

	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;
	if (!iterator)
		return NULL;

	while ((pollfd = faux_pollfd_each(faux_pollfd, iterator))) {
		if (pollfd->revents != 0)
			return pollfd;
	}

	return NULL;
}
