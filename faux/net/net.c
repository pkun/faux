/** @file net.c
 * @brief Class for comfortable send and receive data to and from socket.
 * Class supports timeout for all receive, send functions. It supports signal
 * mask to specify signals that can interrupt send/receive operation. It
 * supports isbreak_func() callback function for consistent working with
 * signals and remove race conditions. It's hard to specify all necessary
 * parameter as a function argument so class hides long functions from user.
 * Parameters of sending or receving can be specified using class methods.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>

#include "faux/faux.h"
#include "faux/str.h"
#include "faux/net.h"

#include "private.h"


/** @brief Allocates memory for faux_net_t object.
 *
 * @return Allocated faux_net_t object or NULL on error.
 */
faux_net_t *faux_net_new(void)
{
	faux_net_t *faux_net = NULL;

	faux_net = faux_zmalloc(sizeof(*faux_net));
	assert(faux_net);
	if (!faux_net)
		return NULL;

	faux_net->fd = -1;
	faux_net->isbreak_func = NULL;
	faux_net_sigmask_fill(faux_net);
	faux_net_set_timeout(faux_net, NULL);

	return faux_net;
}


/** @brief Frees previously allocated faux_net_t object.
 *
 * @param [in] faux_net Allocated faux_net_t object.
 */
void faux_net_free(faux_net_t *faux_net)
{
	if (!faux_net)
		return;
	faux_free(faux_net);
}


/** @brief Sets system descriptor to work with.
 *
 * It's a socket descriptor. Object will operate over this descriptor.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] fd The descriptor to link object to it.
 */
void faux_net_set_fd(faux_net_t *faux_net, int fd)
{
	if (!faux_net)
		return;
	faux_net->fd = fd;
}


/** @brief Unsets socket descriptor.
 *
 * @param [in] faux_net The faux_net_t object.
 */
void faux_net_unset_fd(faux_net_t *faux_net)
{
	if (!faux_net)
		return;
	faux_net->fd = -1;
}


/** @brief Gets file descriptor from object.
 *
 * @param [in] faux_net The faux_net_t object.
 * @return File descriptor from object.
 */
int faux_net_get_fd(faux_net_t *faux_net)
{
	if (!faux_net)
		return -1;
	return faux_net->fd;
}


/** @brief Sets timeout for send operation.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] send_timeout Timeout to set.
 */
void faux_net_set_send_timeout(faux_net_t *faux_net, struct timespec *send_timeout)
{
	assert(faux_net);
	if (!faux_net)
		return;
	if (!send_timeout) {
		faux_net->send_timeout = NULL;
	} else {
		faux_net->send_timeout_val = *send_timeout;
		faux_net->send_timeout = &(faux_net->send_timeout_val);
	}
}


/** @brief Sets timeout for receive operation.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] send_timeout Timeout to set.
 */
void faux_net_set_recv_timeout(faux_net_t *faux_net, struct timespec *recv_timeout)
{
	assert(faux_net);
	if (!faux_net)
		return;
	if (!recv_timeout) {
		faux_net->recv_timeout = NULL;
	} else {
		faux_net->recv_timeout_val = *recv_timeout;
		faux_net->recv_timeout = &(faux_net->recv_timeout_val);
	}
}


/** @brief Sets timeout both for send and receive operation.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] send_timeout Timeout to set.
 */
void faux_net_set_timeout(faux_net_t *faux_net, struct timespec *timeout)
{
	assert(faux_net);
	if (!faux_net)
		return;
	faux_net_set_send_timeout(faux_net, timeout);
	faux_net_set_recv_timeout(faux_net, timeout);
}


/** @brief Sets isbreak_func() funtion for network operation.
 *
 * Callback function informs network functions to interrupt actions.
 * See faux_send_block() function for explanation.
 *
 * @sa faux_send_block()
 * @param [in] faux_net The faux_net_t object.
 * @param [in] isbreak_func Callback function.
 */
void faux_net_set_isbreak_func(faux_net_t *faux_net, int (*isbreak_func)(void))
{
	assert(faux_net);
	if (!faux_net)
		return;
	faux_net->isbreak_func = isbreak_func;
}


/** @brief Emptyes signal mask using while network operations.
 *
 * @param [in] faux_net The faux_net_t object.
 */
void faux_net_sigmask_empty(faux_net_t *faux_net)
{
	assert(faux_net);
	if (!faux_net)
		return;
	sigemptyset(&(faux_net->sigmask));
}


/** @brief Fills signal mask using while network operations.
 *
 * @param [in] faux_net The faux_net_t object.
 */
void faux_net_sigmask_fill(faux_net_t *faux_net)
{
	assert(faux_net);
	if (!faux_net)
		return;
	sigfillset(&(faux_net->sigmask));
}


/** @brief Adds signal to signal mask using while network operations.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] signum Signal number to add.
 */
void faux_net_sigmask_add(faux_net_t *faux_net, int signum)
{
	assert(faux_net);
	if (!faux_net)
		return;
	sigaddset(&(faux_net->sigmask), signum);
}


/** @brief Removes signal to signal mask using while network operations.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] signum Signal number to remove.
 */
void faux_net_sigmask_del(faux_net_t *faux_net, int signum)
{
	assert(faux_net);
	if (!faux_net)
		return;
	sigdelset(&(faux_net->sigmask), signum);
}


/** @brief Sends data to socket associated with given objects.
 *
 * Function uses previously set parameters such as descriptor, timeout,
 * signal mask, callback function.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] buf Data buffer to send
 * @param [in] n Number of bytes to send.
 * @return Number of bytes was succesfully sent or < 0 on error.
 */
ssize_t faux_net_send(faux_net_t *faux_net, const void *buf, size_t n)
{

	return faux_send_block(faux_net->fd, buf, n, faux_net->send_timeout,
		&(faux_net->sigmask), faux_net->isbreak_func);
}


/** @brief Sends data vector to socket associated with given objects.
 *
 * Function uses previously set parameters such as descriptor, timeout,
 * signal mask, callback function.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] iov Array of struct iovec structures.
 * @param [in] iovcnt Number of iov array members.
 * @return Number of bytes was succesfully sent or < 0 on error.
 */
ssize_t faux_net_sendv(faux_net_t *faux_net,
	const struct iovec *iov, int iovcnt)
{
	return faux_sendv_block(faux_net->fd, iov, iovcnt, faux_net->send_timeout,
		&(faux_net->sigmask), faux_net->isbreak_func);
}


/** @brief Receives data from socket associated with given objects.
 *
 * Function uses previously set parameters such as descriptor, timeout,
 * signal mask, callback function.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] buf Data buffer for receiving.
 * @param [in] n Number of bytes to receive.
 * @return Number of bytes was succesfully received or < 0 on error.
 */
ssize_t faux_net_recv(faux_net_t *faux_net, void *buf, size_t n)
{

	return faux_recv_block(faux_net->fd, buf, n, faux_net->recv_timeout,
		&(faux_net->sigmask), faux_net->isbreak_func);
}


/** @brief Receives data vector from socket associated with given objects.
 *
 * Function uses previously set parameters such as descriptor, timeout,
 * signal mask, callback function.
 *
 * @param [in] faux_net The faux_net_t object.
 * @param [in] iov Array of struct iovec structures.
 * @param [in] iovcnt Number of iov array members.
 * @return Number of bytes was succesfully received or < 0 on error.
 */
ssize_t faux_net_recvv(faux_net_t *faux_net, struct iovec *iov, int iovcnt)
{
	return faux_recvv_block(faux_net->fd, iov, iovcnt, faux_net->recv_timeout,
		&(faux_net->sigmask), faux_net->isbreak_func);
}
