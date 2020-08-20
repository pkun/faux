#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "faux/vec.h"

int kmatch(const void *key, const void *item)
{
	uint32_t k = *(uint32_t *)key;
	uint32_t i = *(uint32_t *)item;
	if (k == i)
		return 0;
	return -1;
}

#define VEC_LEN 6
int testc_faux_vec(void)
{
	// Source vector
	const uint32_t src_vec[VEC_LEN] = { 0, 1, 2, 3, 4, 5 };
	unsigned int i = 0;
	int ret = -1; // Pessimistic return value
	faux_vec_t *vec = NULL;

	// Fill the vector
	vec = faux_vec_new(sizeof(uint32_t), kmatch);
	for (i = 0; i < VEC_LEN; i++) {
		uint32_t *val = faux_vec_add(vec);
		*val = src_vec[i];
	}

	// Compare whole vectors
	if (memcmp(faux_vec_data(vec), src_vec,
		faux_vec_len(vec) * faux_vec_item_size(vec))) {
		fprintf(stderr, "Vector object is not equal to etalon vector\n");
		goto err;
	}

	// Out-of-range
	if (faux_vec_item(vec, VEC_LEN) != NULL) {
		fprintf(stderr, "Broken out-of-range\n");
		goto err;
	}

	// Get item by index
	if (*(uint32_t *)faux_vec_item(vec, VEC_LEN - 2) != (VEC_LEN - 2)) {
		fprintf(stderr, "Broken faux_vec_item()\n");
		goto err;
	}

	// Remove last item
	if (faux_vec_del(vec, VEC_LEN - 1) != (VEC_LEN - 1)) {
		fprintf(stderr, "Broken last item deletion\n");
		goto err;
	}

	if (memcmp(faux_vec_data(vec), src_vec,
		faux_vec_len(vec) * faux_vec_item_size(vec))) {
		fprintf(stderr, "Vector object is not equal to etalon vector after del\n");
		goto err;
	}

	// Restore full vector
	*(uint32_t *)faux_vec_add(vec) = VEC_LEN - 1;

	// Remove not-last item
	if (faux_vec_del(vec, VEC_LEN - 3) != (VEC_LEN - 1)) {
		fprintf(stderr, "Broken non-last item deletion\n");
		goto err;
	}

	// Get item by index but it fill the hole now
	if (*(uint32_t *)faux_vec_item(vec, VEC_LEN - 3) != (VEC_LEN - 2)) {
		fprintf(stderr, "Broken faux_vec_item() after non-last delete\n");
		goto err;
	}

	// Find item
	if (faux_vec_find(vec, &src_vec[VEC_LEN - 1], 0) != (VEC_LEN - 2)) {
		fprintf(stderr, "Can't find item by key\n");
		goto err;
	}

	// Find item start from some item
	if (faux_vec_find(vec, &src_vec[1], 2) >= 0) {
		fprintf(stderr, "The start_index doesn't work\n");
		goto err;
	}

	ret = 0;
err:
	faux_vec_free(vec);

	return ret;
}
