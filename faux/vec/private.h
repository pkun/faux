#include "faux/vec.h"

struct faux_vec_s {
	void *data;
	size_t len;
	size_t item_size;
};
