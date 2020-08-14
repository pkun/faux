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

#if 0
ssize_t faux_net_send(faux_net_t *faux_net, int sock, int flags)
{
	unsigned int vec_entries_num = 0;
	struct iovec *iov = NULL;
	unsigned int i = 0;
	faux_list_node_t *iter = NULL;
	size_t ret = 0;

	assert(faux_net);
	assert(faux_net->hdr);
	if (!faux_net || !faux_net->hdr)
		return -1;

	// Calculate number if struct iovec entries.
	// n = (msg header) + ((param hdr) + (param data)) * (param_num)
	vec_entries_num = 1 + (2 * faux_net_get_param_num(faux_net));
	iov = faux_zmalloc(vec_entries_num * sizeof(*iov));

	// Message header
	iov[i].iov_base = faux_net->hdr;
	iov[i].iov_len = sizeof(*faux_net->hdr);
	i++;

	// Parameter headers
	for (iter = faux_net_init_param_iter(faux_net);
		iter; iter = faux_list_next_node(iter)) {
		crsp_phdr_t *phdr = NULL;
		phdr = (crsp_phdr_t *)faux_list_data(iter);
		iov[i].iov_base = phdr;
		iov[i].iov_len = sizeof(*phdr);
		i++;
	}

	// Parameter data
	for (iter = faux_net_init_param_iter(faux_net);
		iter; iter = faux_list_next_node(iter)) {
		crsp_phdr_t *phdr = NULL;
		void *data = NULL;
		phdr = (crsp_phdr_t *)faux_list_data(iter);
		data = (char *)phdr + sizeof(*phdr);
		iov[i].iov_base = data;
		iov[i].iov_len = ntohl(phdr->param_len);
		i++;
	}

	ret = faux_sendv_block(sock, iov, vec_entries_num, flags);
	faux_free(iov);

	// Debug
	if (faux_net && ret > 0) {
		printf("(o) ");
		faux_net_debug(faux_net);
	}

	return ret;
}


faux_net_t *faux_net_recv(int sock, int flags)
{
	faux_net_t *faux_net = NULL;
	size_t received = 0;
	crsp_phdr_t *phdr = NULL;
	unsigned int param_num = 0;
	size_t phdr_whole_len = 0;
	size_t max_data_len = 0;
	size_t cur_data_len = 0;
	unsigned int i = 0;
	char *data = NULL;

	faux_net = faux_net_allocate();
	assert(faux_net);
	if (!faux_net)
		return NULL;

	// Receive message header
	received = faux_recv_block(sock, faux_net->hdr, sizeof(*faux_net->hdr),
		flags);
	if (received != sizeof(*faux_net->hdr)) {
		faux_net_free(faux_net);
		return NULL;
	}
	if (!faux_net_check_hdr(faux_net)) {
		faux_net_free(faux_net);
		return NULL;
	}

	// Receive parameter headers
	param_num = faux_net_get_param_num(faux_net);
	if (param_num != 0) {
		phdr_whole_len = param_num * sizeof(*phdr);
		phdr = faux_zmalloc(phdr_whole_len);
		received = faux_recv_block(sock, phdr, phdr_whole_len, flags);
		if (received != phdr_whole_len) {
			faux_free(phdr);
			faux_net_free(faux_net);
			return NULL;
		}
		// Find out maximum data length
		for (i = 0; i < param_num; i++) {
			cur_data_len = ntohl(phdr[i].param_len);
			if (cur_data_len > max_data_len)
				max_data_len = cur_data_len;
		}

		// Receive parameter data
		data = faux_zmalloc(max_data_len);
		for (i = 0; i < param_num; i++) {
			cur_data_len = ntohl(phdr[i].param_len);
			if (0 == cur_data_len)
				continue;
			received = faux_recv_block(sock, data, cur_data_len, flags);
			if (received != cur_data_len) {
				faux_free(data);
				faux_free(phdr);
				faux_net_free(faux_net);
				return NULL;
			}
			faux_net_add_param_internal(faux_net, phdr[i].param_type,
				data, cur_data_len, BOOL_FALSE);
		}

		faux_free(data);
		faux_free(phdr);
	}

	// Debug
	if (faux_net) {
		printf("(i) ");
		faux_net_debug(faux_net);
	}

	return faux_net;
}
#endif
