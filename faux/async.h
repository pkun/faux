/** @file async.h
 * @brief Public interface for ASYNChronous I/O class.
 */

#ifndef _faux_async_h
#define _faux_async_h

#include <faux/faux.h>
#include <faux/sched.h>

typedef struct faux_async_s faux_async_t;

/*
typedef enum {
	FAUX_ELOOP_NULL = 0,
	FAUX_ELOOP_SIGNAL = 1,
	FAUX_ELOOP_SCHED = 2,
	FAUX_ELOOP_FD = 3
} faux_eloop_type_e;
*/

// Callback function prototypes
typedef bool_t (*faux_async_read_cb_f)(faux_async_t *async,
	void *data, size_t len, void *user_data);
typedef bool_t (*faux_async_stall_cb_f)(faux_async_t *async,
	size_t len, void *user_data);


C_DECL_BEGIN

faux_async_t *faux_async_new(int fd);
void faux_async_free(faux_async_t *async);
int faux_async_fd(const faux_async_t *async);
void faux_async_set_read_cb(faux_async_t *async,
	faux_async_read_cb_f read_cb, void *user_data);
bool_t faux_async_set_read_limits(faux_async_t *async, size_t min, size_t max);
void faux_async_set_stall_cb(faux_async_t *async,
	faux_async_stall_cb_f stall_cb, void *user_data);
void faux_async_set_overflow(faux_async_t *async, size_t overflow);
ssize_t faux_async_write(faux_async_t *async, void *data, size_t len);
ssize_t faux_async_out(faux_async_t *async);

C_DECL_END

#endif // _faux_async_h
