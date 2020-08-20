/** @file vec.c
 * Implementation of variable length vector of arbitrary structures.
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include "private.h"


faux_vec_t *faux_vec_new(size_t item_size, faux_vec_kcmp_fn matchFn)
{
	faux_vec_t *faux_vec = NULL;

	faux_vec = faux_zmalloc(sizeof(*faux_vec));
	assert(faux_vec);
	if (!faux_vec)
		return NULL;

	// Init
	faux_vec->data = NULL;
	faux_vec->item_size = item_size;
	faux_vec->len = 0;
	faux_vec->kcmpFn = matchFn;

	return faux_vec;
}


void faux_vec_free(faux_vec_t *faux_vec)
{
	if (!faux_vec)
		return;
	faux_free(faux_vec->data);
	faux_free(faux_vec);
}


size_t faux_vec_len(const faux_vec_t *faux_vec)
{
	assert(faux_vec);
	if (!faux_vec)
		return 0;

	return faux_vec->len;
}


size_t faux_vec_item_size(const faux_vec_t *faux_vec)
{
	assert(faux_vec);
	if (!faux_vec)
		return 0;

	return faux_vec->item_size;
}


void *faux_vec_item(const faux_vec_t *faux_vec, unsigned int index)
{
	assert(faux_vec);
	if (!faux_vec)
		return NULL;

	if ((index + 1) > faux_vec_len(faux_vec))
		return NULL;

	return (char *)faux_vec->data + index * faux_vec_item_size(faux_vec);
}


void *faux_vec_data(const faux_vec_t *faux_vec)
{
	assert(faux_vec);
	if (!faux_vec)
		return NULL;

	return faux_vec->data;
}


void *faux_vec_add(faux_vec_t *faux_vec)
{
	void *new_vector = NULL;
	size_t new_data_len = 0;

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

	// Return newly created item (it's last one)
	return faux_vec_item(faux_vec, faux_vec_len(faux_vec) - 1);
}


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
