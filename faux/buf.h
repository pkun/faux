/** @file buf.h
 * @brief Public interface for dynamic buffer class.
 */

#ifndef _faux_buf_h
#define _faux_buf_h

#include <faux/faux.h>
#include <faux/sched.h>

#define FAUX_BUF_UNLIMITED 0

typedef struct faux_buf_s faux_buf_t;


C_DECL_BEGIN

faux_buf_t *faux_buf_new(size_t chunk_size);
void faux_buf_free(faux_buf_t *buf);
ssize_t faux_buf_len(const faux_buf_t *buf);
ssize_t faux_buf_limit(const faux_buf_t *buf);
bool_t faux_buf_set_limit(faux_buf_t *buf, size_t limit);
bool_t faux_buf_will_be_overflow(const faux_buf_t *buf, size_t add_len);
size_t faux_buf_is_wlocked(const faux_buf_t *buf);
size_t faux_buf_is_rlocked(const faux_buf_t *buf);
ssize_t faux_buf_write(faux_buf_t *buf, const void *data, size_t len);
ssize_t faux_buf_read(faux_buf_t *buf, void *data, size_t len);
ssize_t faux_buf_dread_lock(faux_buf_t *buf, size_t len,
	struct iovec **iov, size_t *iov_num);
ssize_t faux_buf_dread_unlock(faux_buf_t *buf, size_t really_readed,
	struct iovec *iov);
ssize_t faux_buf_dwrite_lock(faux_buf_t *buf, size_t len,
	struct iovec **iov_out, size_t *iov_num_out);
ssize_t faux_buf_dwrite_unlock(faux_buf_t *buf, size_t really_written,
	struct iovec *iov);
ssize_t faux_buf_dwrite_lock_easy(faux_buf_t *buf, void **data);
ssize_t faux_buf_dwrite_unlock_easy(faux_buf_t *buf, size_t really_written);
ssize_t faux_buf_dread_lock_easy(faux_buf_t *buf, void **data);
ssize_t faux_buf_dread_unlock_easy(faux_buf_t *buf, size_t really_readed);

C_DECL_END

#endif // _faux_buf_h
