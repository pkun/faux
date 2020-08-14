/** @file net.h
 * @brief Public interface for faux net functions.
 */

#ifndef _faux_net_h
#define _faux_net_h

#include <faux/faux.h>

C_DECL_BEGIN

// Network base I/O functions
ssize_t faux_send(int fd, const void *buf, size_t n,
	const struct timespec *timeout, const sigset_t *sigmask);
ssize_t faux_send_block(int fd, const void *buf, size_t n,
	const struct timespec *timeout, const sigset_t *sigmask,
	int (*isbreak_func)(void));
ssize_t faux_sendv(int fd, const struct iovec *iov, int iovcnt,
	const struct timespec *timeout, const sigset_t *sigmask);
ssize_t faux_sendv_block(int fd, const struct iovec *iov, int iovcnt,
	const struct timespec *timeout, const sigset_t *sigmask,
	int (*isbreak_func)(void));
ssize_t faux_recv(int fd, void *buf, size_t n,
	const struct timespec *timeout, const sigset_t *sigmask);
ssize_t faux_recv_block(int fd, void *buf, size_t n,
	const struct timespec *timeout, const sigset_t *sigmask,
	int (*isbreak_func)(void));
ssize_t faux_recvv(int fd, struct iovec *iov, int iovcnt,
	const struct timespec *timeout, const sigset_t *sigmask);
ssize_t faux_recvv_block(int fd, struct iovec *iov, int iovcnt,
	const struct timespec *timeout, const sigset_t *sigmask,
	int (*isbreak_func)(void));

C_DECL_END

#endif
