#include "faux/faux.h"
#include "faux/buf.h"
#include "faux/net.h"

#define DATA_CHUNK 4096

struct faux_async_s {
	int fd;

	// Read
	faux_async_read_cb_fn read_cb; // Read callback
	void *read_udata;
	size_t min;
	size_t max;
	faux_buf_t *ibuf;

	// Write
	faux_async_stall_cb_fn stall_cb; // Stall callback
	void *stall_udata;
	faux_buf_t *obuf;
};
