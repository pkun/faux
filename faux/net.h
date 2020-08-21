/** @file net.h
 * @brief Public interface for faux net functions.
 */

#ifndef _faux_net_h
#define _faux_net_h

#include <faux/faux.h>

typedef struct faux_net_s faux_net_t;
typedef struct faux_pollfd_s faux_pollfd_t;
typedef int faux_pollfd_iterator_t;


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

// Network class
faux_net_t *faux_net_new(void);
void faux_net_free(faux_net_t *faux_net);
void faux_net_set_fd(faux_net_t *faux_net, int fd);
int faux_net_get_fd(faux_net_t *faux_net);
void faux_net_set_send_timeout(faux_net_t *faux_net, struct timespec *send_timeout);
void faux_net_set_recv_timeout(faux_net_t *faux_net, struct timespec *recv_timeout);
void faux_net_set_timeout(faux_net_t *faux_net, struct timespec *timeout);
void faux_net_set_isbreak_func(faux_net_t *faux_net, int (*isbreak_func)(void));
void faux_net_sigmask_empty(faux_net_t *faux_net);
void faux_net_sigmask_fill(faux_net_t *faux_net);
void faux_net_sigmask_add(faux_net_t *faux_net, int signum);
void faux_net_sigmask_del(faux_net_t *faux_net, int signum);
ssize_t faux_net_send(faux_net_t *faux_net, const void *buf, size_t n);
ssize_t faux_net_sendv(faux_net_t *faux_net,
	const struct iovec *iov, int iovcnt);
ssize_t faux_net_recv(faux_net_t *faux_net, void *buf, size_t n);
ssize_t faux_net_recvv(faux_net_t *faux_net, struct iovec *iov, int iovcnt);

// Pollfd class
faux_pollfd_t *faux_pollfd_new(void);
void faux_pollfd_free(faux_pollfd_t *faux_pollfd);
struct pollfd *faux_pollfd_vector(faux_pollfd_t *faux_pollfd);
size_t faux_pollfd_len(faux_pollfd_t *faux_pollfd);
struct pollfd *faux_pollfd_item(faux_pollfd_t *faux_pollfd, unsigned int index);
struct pollfd *faux_pollfd_find(faux_pollfd_t *faux_pollfd, int fd);
struct pollfd *faux_pollfd_add(faux_pollfd_t *faux_pollfd, int fd, short events);
int faux_pollfd_del_by_fd(faux_pollfd_t *faux_pollfd, int fd);
int faux_pollfd_del_by_index(faux_pollfd_t *faux_pollfd, unsigned int index);
void faux_pollfd_init_iterator(faux_pollfd_t *faux_pollfd, faux_pollfd_iterator_t *iterator);
struct pollfd *faux_pollfd_each(faux_pollfd_t *faux_pollfd, faux_pollfd_iterator_t *iterator);
struct pollfd *faux_pollfd_each_active(faux_pollfd_t *faux_pollfd, faux_pollfd_iterator_t *iterator);

C_DECL_END

#endif
