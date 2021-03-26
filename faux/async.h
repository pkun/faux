/** @file async.h
 * @brief Public interface for ASYNChronous I/O class.
 */

#ifndef _faux_async_h
#define _faux_async_h

#include <faux/faux.h>
#include <faux/sched.h>

#define FAUX_ASYNC_UNLIMITED 0

typedef struct faux_async_s faux_async_t;


// Callback function prototypes
typedef bool_t (*faux_async_read_cb_fn)(faux_async_t *async,
	void *data, size_t len, void *user_data);
typedef bool_t (*faux_async_stall_cb_fn)(faux_async_t *async,
	size_t len, void *user_data);


C_DECL_BEGIN

faux_async_t *faux_async_new(int fd);
void faux_async_free(faux_async_t *async);
int faux_async_fd(const faux_async_t *async);
void faux_async_set_read_cb(faux_async_t *async,
	faux_async_read_cb_fn read_cb, void *user_data);
bool_t faux_async_set_read_limits(faux_async_t *async, size_t min, size_t max);
void faux_async_set_stall_cb(faux_async_t *async,
	faux_async_stall_cb_fn stall_cb, void *user_data);
void faux_async_set_write_overflow(faux_async_t *async, size_t overflow);
void faux_async_set_read_overflow(faux_async_t *async, size_t overflow);
ssize_t faux_async_write(faux_async_t *async, void *data, size_t len);
ssize_t faux_async_out(faux_async_t *async);
ssize_t faux_async_in(faux_async_t *async);

C_DECL_END

#endif // _faux_async_h
