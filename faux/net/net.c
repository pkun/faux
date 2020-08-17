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


static faux_net_t *faux_net_allocate(void)
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


faux_net_t *faux_net_new_by_fd(int fd)
{
	faux_net_t *faux_net = NULL;

	faux_net = faux_net_allocate();
	assert(faux_net);
	if (!faux_net)
		return NULL;

	faux_net->fd = fd;

	return faux_net;
}


void faux_net_free(faux_net_t *faux_net)
{
	if (!faux_net)
		return;
	faux_free(faux_net);
}


void faux_net_set_send_timeout(faux_net_t *faux_net, struct timespec *send_timeout)
{
	assert(faux_net);
	if (!faux_net)
		return;
	if (!send_timeout) {
		faux_net->send_timeout = NULL;
	} else {
		faux_net->send_timeout_val = *send_timeout;
		faux_net->send_timeout = &faux_net->send_timeout_val;
	}
}


void faux_net_set_recv_timeout(faux_net_t *faux_net, struct timespec *recv_timeout)
{
	assert(faux_net);
	if (!faux_net)
		return;
	if (!recv_timeout) {
		faux_net->recv_timeout = NULL;
	} else {
		faux_net->recv_timeout_val = *recv_timeout;
		faux_net->recv_timeout = &faux_net->recv_timeout_val;
	}
}


void faux_net_set_timeout(faux_net_t *faux_net, struct timespec *timeout)
{
	assert(faux_net);
	if (!faux_net)
		return;
	faux_net_set_send_timeout(faux_net, timeout);
	faux_net_set_recv_timeout(faux_net, timeout);
}


void faux_net_set_isbreak_func(faux_net_t *faux_net, int (*isbreak_func)(void))
{
	assert(faux_net);
	if (!faux_net)
		return;
	faux_net->isbreak_func = isbreak_func;
}


void faux_net_sigmask_empty(faux_net_t *faux_net)
{
	assert(faux_net);
	if (!faux_net)
		return;
	sigemptyset(&faux_net->sigmask);
}


void faux_net_sigmask_fill(faux_net_t *faux_net)
{
	assert(faux_net);
	if (!faux_net)
		return;
	sigfillset(&faux_net->sigmask);
}


void faux_net_sigmask_add(faux_net_t *faux_net, int signum)
{
	assert(faux_net);
	if (!faux_net)
		return;
	sigaddset(&faux_net->sigmask, signum);
}


void faux_net_sigmask_del(faux_net_t *faux_net, int signum)
{
	assert(faux_net);
	if (!faux_net)
		return;
	sigdelset(&faux_net->sigmask, signum);
}


ssize_t faux_net_send(faux_net_t *faux_net, const void *buf, size_t n)
{

	return faux_send_block(faux_net->fd, buf, n, faux_net->send_timeout,
		&faux_net->sigmask, faux_net->isbreak_func);
}


ssize_t faux_net_sendv(faux_net_t *faux_net,
	const struct iovec *iov, int iovcnt)
{
	return faux_sendv_block(faux_net->fd, iov, iovcnt, faux_net->send_timeout,
		&faux_net->sigmask, faux_net->isbreak_func);
}


ssize_t faux_net_recv(faux_net_t *faux_net, void *buf, size_t n)
{

	return faux_recv_block(faux_net->fd, buf, n, faux_net->recv_timeout,
		&faux_net->sigmask, faux_net->isbreak_func);
}


ssize_t faux_net_recvv(faux_net_t *faux_net, struct iovec *iov, int iovcnt)
{
	return faux_recvv_block(faux_net->fd, iov, iovcnt, faux_net->recv_timeout,
		&faux_net->sigmask, faux_net->isbreak_func);
}
