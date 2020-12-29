#include "faux/faux.h"
#include "faux/list.h"
#include "faux/net.h"


struct faux_async_s {
	int fd;
	// Read
	faux_async_read_cb_f read_cb; // Read callback
	void *read_udata;
	size_t min;
	size_t max;
	faux_list_t *i_list;
	size_t i_pos;
	// Write
	faux_async_stall_cb_f stall_cb; // Stall callback
	void *stall_udata;
	faux_list_t *o_list;
	size_t o_pos;
	size_t overflow;
};
