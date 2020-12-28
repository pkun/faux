#include "faux/faux.h"
#include "faux/list.h"
#include "faux/net.h"


struct faux_async_s {
	int fd;
	// Read
	faux_async_read_cb_f read_cb; // Read callback
	void *read_udata;
	ssize_t min;
	ssize_t max;
	// Write
	faux_async_stall_cb_f stall_cb; // Stall callback
	void *stall_udata;
};
