#include "faux/vec.h"

struct faux_vec_s {
	void *data;
	size_t len;
	size_t item_size;
	faux_vec_kcmp_fn kcmpFn; // Function to compare key and vector's item
};
