/** @file vec.c
 * Implementation of variable length vector of arbitrary structures.
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include "private.h"


/** @brief Allocates and initalizes new vector.
 *
 * Callback function matchFn can be used later to find item by user specified
 * key. Function can compare key and item's data.
 *
 * @param [in] item_size Size of single vector's item.
 * @param [in] matchFn Callback function to compare user key and item's data.
 * @return Allocated and initialized vector or NULL on error.
 */
faux_vec_t *faux_vec_new(size_t item_size, faux_vec_kcmp_fn matchFn)
{
	faux_vec_t *faux_vec = NULL;

	faux_vec = faux_zmalloc(sizeof(*faux_vec));
	assert(faux_vec);
	if (!faux_vec)
		return NULL;
	if (0 == item_size)
		return NULL;

	// Init
	faux_vec->data = NULL;
	faux_vec->item_size = item_size;
	faux_vec->len = 0;
	faux_vec->kcmpFn = matchFn;

	return faux_vec;
}


/** @brief Frees previously allocated vector object.
 *
 * @param [in] faux_vec Allocated vector object.
 */
void faux_vec_free(faux_vec_t *faux_vec)
{
	if (!faux_vec)
		return;
	faux_free(faux_vec->data);
	faux_free(faux_vec);
}


/** @brief Gets vector length in items.
 *
 * @param [in] faux_vec Allocated vector object.
 * @return Number of items.
 */
size_t faux_vec_len(const faux_vec_t *faux_vec)
{
	assert(faux_vec);
	if (!faux_vec)
		return 0;

	return faux_vec->len;
}


/** @brief Gets size of item.
 *
 * @param [in] faux_vec Allocated vector object.
 * @return Size of item in bytes.
 */
size_t faux_vec_item_size(const faux_vec_t *faux_vec)
{
	assert(faux_vec);
	if (!faux_vec)
		return 0;

	return faux_vec->item_size;
}


/** @brief Gets item by index.
 *
 * Gets pointer to item's data.
 *
 * @param [in] faux_vec Allocated vector object.
 * @return Pointer to item or NULL on error.
 */
void *faux_vec_item(const faux_vec_t *faux_vec, unsigned int index)
{
	assert(faux_vec);
	if (!faux_vec)
		return NULL;

	if ((index + 1) > faux_vec_len(faux_vec))
		return NULL;

	return (char *)faux_vec->data + index * faux_vec_item_size(faux_vec);
}


/** @brief Gets pointer to whole vector.
 *
 * @param [in] faux_vec Allocated vector object.
 * @return Pointer to vector or NULL on error.
 */

void *faux_vec_data(const faux_vec_t *faux_vec)
{
	assert(faux_vec);
	if (!faux_vec)
		return NULL;

	return faux_vec->data;
}


/** @brief Adds item to vector and gets pointer to newly created item.
 *
 * @param [in] faux_vec Allocated vector object.
 * @return Newly created item or NULL on error.
 */
void *faux_vec_add(faux_vec_t *faux_vec)
{
	void *new_vector = NULL;
	size_t new_data_len = 0;
	void *new_item = NULL;

	assert(faux_vec);
	if (!faux_vec)
		return NULL;

	// Allocate space to hold new vector
	new_data_len = (faux_vec_len(faux_vec) + 1) * faux_vec_item_size(faux_vec);
	new_vector = realloc(faux_vec->data, new_data_len);
	assert(new_vector);
	if (!new_vector)
		return NULL;
	faux_vec->len++;
	faux_vec->data = new_vector;

	// Newly created item (it's last one)
	new_item = faux_vec_item(faux_vec, faux_vec_len(faux_vec) - 1);
	faux_bzero(new_item, faux_vec_item_size(faux_vec));

	return new_item;
}


/** @brief Removes item from vector by index.
 *
 * Function removes item by index and then fill hole with the following items.
 * It saves items sequence and frees vector memory.
 *
 * @param [in] faux_vec Allocated vector object.
 * @param [in] index Index of item to remove.
 * @return New number of items within vector after removing or < 0 on error.
 */
ssize_t faux_vec_del(faux_vec_t *faux_vec, unsigned int index)
{
	void *new_vector = NULL;
	size_t new_data_len = 0;

	assert(faux_vec);
	if (!faux_vec)
		return -1;

	if ((index + 1) > faux_vec_len(faux_vec))
		return -1;

	// Move following items to fill the space of deleted item
	if (index != (faux_vec_len(faux_vec) - 1)) { // Is it last item?
		void *item_to_del = faux_vec_item(faux_vec, index);
		void *next_item = faux_vec_item(faux_vec, index + 1);
		unsigned int items_to_move =
			faux_vec_len(faux_vec) - (index + 1);
		memmove(item_to_del, next_item,
			items_to_move * faux_vec_item_size(faux_vec));
	}

	// Re-allocate space to hold new vector
	faux_vec->len--;
	new_data_len = faux_vec_len(faux_vec) * faux_vec_item_size(faux_vec);
	new_vector = realloc(faux_vec->data, new_data_len);
	assert(new_vector);
	if (!new_vector)
		return -1;
	faux_vec->data = new_vector;

	return faux_vec_len(faux_vec);
}


/** @brief Finds item by user defined key using specified callback function.
 *
 * It iterates through the vector and try to find item with the specified key
 * value. It starts searching with specified item index and returns index of
 * found item. So it can be used to iterate all the vector with duplicate keys.
 *
 * @param [in] faux_vec Allocated vector object.
 * @param [in] matchFn Callback function to compare user key and item's data.
 * @param [in] userkey User defined key to compare item to.
 * @param [in] start_index Item's index to start searching from.
 * @return Index of found item or < 0 on error or "not found" case.
 */
int faux_vec_find_fn(const faux_vec_t *faux_vec, faux_vec_kcmp_fn matchFn,
	const void *userkey, unsigned int start_index)
{
	unsigned int i = 0;

	assert(faux_vec);
	if (!faux_vec)
		return -1;
	assert(userkey);
	if (!userkey)
		return -1;
	assert(matchFn);
	if (!matchFn)
		return -1;

	for (i = start_index; i < faux_vec_len(faux_vec); i++) {
		if (matchFn(userkey, faux_vec_item(faux_vec, i)) == 0)
			return i;
	}

	return -1;
}


/** @brief Finds item by user defined key.
 *
 * It acts like a faux_vec_find_fn() function but uses callback function
 * specified while faux_vec_new() call.
 *
 * @sa faux_vec_find_fn()
 * @sa faux_vec_new()
 * @param [in] faux_vec Allocated vector object.
 * @param [in] userkey User defined key to compare item to.
 * @param [in] start_index Item's index to start searching from.
 * @return Index of found item or < 0 on error or "not found" case.
 */
int faux_vec_find(const faux_vec_t *faux_vec, const void *userkey,
	unsigned int start_index)
{
	assert(faux_vec);
	if (!faux_vec)
		return -1;
	assert(faux_vec->kcmpFn);
	if (!faux_vec->kcmpFn)
		return -1;

	return faux_vec_find_fn(faux_vec, faux_vec->kcmpFn,
		userkey, start_index);
}


/** @brief Deletes all vector's items.
 *
 * @param [in] faux_vec Allocated vector object.
 */
void faux_vec_del_all(faux_vec_t *faux_vec)
{
	if (!faux_vec)
		return;
	faux_free(faux_vec->data);
	faux_vec->data = NULL;
	faux_vec->len = 0;
}
