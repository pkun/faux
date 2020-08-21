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


/** @brief Callback function to search specified fd within pollfd structures.
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


/** @brief Returns whole "struct pollfd" vector.
 *
 * It can be used while poll() call.
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @return Pointer to "struct pollfd" vector or NULL on error.
 */
struct pollfd *faux_pollfd_vector(faux_pollfd_t *faux_pollfd)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;

	return faux_vec_data(faux_pollfd->vec);
}


/** @brief Returns number of "struct pollfd" items within object.
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @return Number of items.
 */
size_t faux_pollfd_len(faux_pollfd_t *faux_pollfd)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return 0;

	return faux_vec_len(faux_pollfd->vec);
}


/** @brief Returns "struct pollfd" item by specified index.
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @param [in] index Index of item to get.
 * @return Pointer to item or NULL on error.
 */
struct pollfd *faux_pollfd_item(faux_pollfd_t *faux_pollfd, unsigned int index)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;

	return (struct pollfd *)faux_vec_item(faux_pollfd->vec, index);
}


/** @brief Finds item with specified fd value.
 *
 * File descriptor is a key for array. Object can contain the only one item
 * with the same fd value.
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @param [in] fd File descriptor to search for.
 * @return Pointer to found item or NULL on error or in "not found" case.
 */
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


/** @brief Adds new item to object.
 *
 * The file descriptors are unique within array. So function try to find item
 * with the specified fd. If it's found the correspondent item will be returned
 * but new item will not be created.
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @param [in] fd File descriptor to set to newly created item.
 * @param [in] events The events interested in.
 * @return Pointer to new item or NULL on error.
 */
struct pollfd *faux_pollfd_add(faux_pollfd_t *faux_pollfd, int fd, short events)
{
	struct pollfd *pollfd = NULL;

	assert(faux_pollfd);
	if (!faux_pollfd)
		return NULL;
	assert(fd >= 0);
	if (fd < 0)
		return NULL;

	// Don't add duplicated fd
	pollfd = faux_pollfd_find(faux_pollfd, fd);
	if (!pollfd) {
		// Create new item
		pollfd = faux_vec_add(faux_pollfd->vec);
		assert(pollfd);
		if (!pollfd)
			return NULL;
		pollfd->fd = fd;
	}

	pollfd->events = events;

	return pollfd;
}


/** @brief Removes item specified by fd.
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @param [in] fd File descriptor to remove.
 * @return 0 - success, < 0 on error.
 */
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


/** @brief Removes item specified by index.
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @param [in] index Index of item to remove.
 * @return 0 - success, < 0 on error.
 */
int faux_pollfd_del_by_index(faux_pollfd_t *faux_pollfd, unsigned int index)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return -1;

	return faux_vec_del(faux_pollfd->vec, index);
}


/** @brief Initilizes iterator to iterate through all the vector.
 *
 * @sa faux_pollfd_each()
 * @sa faux_pollfd_each_active()
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @param [out] iterator Iterator to initialize.
 */
void faux_pollfd_init_iterator(faux_pollfd_t *faux_pollfd, faux_pollfd_iterator_t *iterator)
{
	assert(faux_pollfd);
	if (!faux_pollfd)
		return;
	if (!iterator)
		return;
	*iterator = 0;
}


/** @brief Iterate through all the vector.
 *
 * The iterator must be initialized first by faux_pollfd_init_iterator().
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @param [out] iterator Initialized iterator.
 * @return Pointer to item or NULL on error or on end of vector.
 */
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


/** @brief Iterate through all active items of vector.
 *
 * The iterator must be initialized first by faux_pollfd_init_iterator().
 * Function returns items that has non-null value in "revent" field i.e.
 * active items.
 *
 * @param [in] faux_pollfd Allocated faux_pollfd_t object.
 * @param [out] iterator Initialized iterator.
 * @return Pointer to active item or NULL on error or on end of vector.
 */
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
