/** @file net.c
 * @brief Network related functions.
 */

// For ppol()
#define _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <signal.h>
#include <poll.h>

#include "faux/faux.h"
#include "faux/time.h"
#include "faux/net.h"

#ifdef HAVE_PTHREAD
#define setsigmask pthread_sigmask
#else
#define setsigmask sigprocmask
#endif


/** @brief Sends data to socket. Uses timeout and signal mask.
 *
 * The function acts like a pselect(). It gets timeout interval to interrupt
 * too long sending. It gets signal mask to atomically set it while blocking
 * within select() like function. But it doesn't blocks signals before it.
 * User code must do it. The function can be interrupted by unblocked signal or
 * by timeout. Else it will send() all data given.
 *
 * @param [in] fd Socket.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @return Number of bytes written or < 0 on error.
 */
ssize_t faux_send(int fd, const void *buf, size_t n,
	const struct timespec *timeout, const sigset_t *sigmask)
{
	size_t total_written = 0;
	size_t left = n;
	const void *data = buf;
	struct timespec now = {};
	struct timespec deadline = {};

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	// Calculate deadline - the time when timeout must occur.
	if (timeout) {
		faux_timespec_now(&now);
		faux_timespec_sum(&deadline, &now, timeout);
	}

	do {
		ssize_t bytes_written = 0;
		struct pollfd fds = {};
		struct timespec *poll_timeout = NULL;
		struct timespec to = {};
		int sn = 0;

		if (timeout) {
			if (faux_timespec_before_now(&deadline))
				break; // Timeout already occured
			faux_timespec_now(&now);
			faux_timespec_diff(&to, &deadline, &now);
			poll_timeout = &to;
		}

		// Handlers for poll()
		faux_bzero(&fds, sizeof(fds));
		fds.fd = fd;
		fds.events = POLLOUT;

		sn = ppoll(&fds, 1, poll_timeout, sigmask);
		// When kernel can't allocate some internal structures it can
		// return EAGAIN so retry.
		if ((sn < 0) && (EAGAIN == errno))
			continue;
		// All unneded signals are masked so don't process EINTR
		// in special way. Just break the loop
		if (sn < 0)
			break;
		// Timeout: break the loop. User don't want to wait any more
		if (0 == sn)
			break;
		// Some unknown event (not POLLOUT). So retry polling
		if (!(fds.revents & POLLOUT))
			continue;

		// The send() call is non-blocking but it's not obvious that
		// it can't return EINTR. Probably it can. Due to the fact the
		// call is non-blocking re-send() on any signal i.e. any EINTR.
		do {
			bytes_written = send(fd, data, left, MSG_DONTWAIT | MSG_NOSIGNAL);
		} while ((bytes_written < 0) && (EINTR == errno));
		if (bytes_written < 0)
			break;
		// Insufficient space
		if (0 == bytes_written)
			break;

		data += bytes_written;
		left = left - bytes_written;
		total_written += bytes_written;

	} while (left > 0);

	return total_written;
}

ssize_t faux_send_block(int fd, const void *buf, size_t n,
	const struct timespec *timeout, const sigset_t *sigmask,
	int (*isbreak_func)(void))
{
	sigset_t all_sigmask = {}; // All signals mask
	sigset_t orig_sigmask = {}; // Saved signal mask
	ssize_t bytes_num = 0;

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	// Block signals to prevent race conditions right before pselect()
	// Catch signals while pselect() only
	// Now blocks all signals
	sigfillset(&all_sigmask);
	setsigmask(SIG_SETMASK, &all_sigmask, &orig_sigmask);

	// Signal handler can set var to interrupt exchange.
	// Get value of this var by special callback function.
	if (isbreak_func && isbreak_func())
		return -1;

	bytes_num = faux_send(fd, buf, n, timeout, sigmask);

	setsigmask(SIG_SETMASK, &orig_sigmask, NULL);

	return bytes_num;
}


/** @brief Sends struct iovec data blocks to socket.
 *
 * This function is like a faux_send() function but uses scatter/gather.
 *
 * @see faux_send().
 * @param [in] fd Socket.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @param [in] flags Flags.
 * @return Number of bytes written.
 * < total_length then insufficient space, timeout or
 * error (but some data were already sent).
 * < 0 - error.
 */
ssize_t faux_sendv(int fd, const struct iovec *iov, int iovcnt,
	const struct timespec *timeout, const sigset_t *sigmask)
{
	size_t total_written = 0;
	int i = 0;
	struct timespec now = {};
	struct timespec deadline = {};

	if (!iov)
		return -1;
	if (iovcnt == 0)
		return 0;

	// Calculate deadline - the time when timeout must occur.
	if (timeout) {
		faux_timespec_now(&now);
		faux_timespec_sum(&deadline, &now, timeout);
	}

	for (i = 0; i < iovcnt; i++) {
		ssize_t bytes_written = 0;
		struct timespec *send_timeout = NULL;
		struct timespec to = {};

		if (timeout) {
			if (faux_timespec_before_now(&deadline))
				break; // Timeout already occured
			faux_timespec_now(&now);
			faux_timespec_diff(&to, &deadline, &now);
			send_timeout = &to;
		}
		if (iov[i].iov_len == 0)
			continue;
		bytes_written = faux_send(fd, iov[i].iov_base, iov[i].iov_len,
			send_timeout, sigmask);
		if (bytes_written < 0) { // Error
			if (total_written != 0)
				break;
			return -1;
		}
		if (0 == bytes_written) // Insufficient space or timeout
			break;
		total_written += bytes_written;
	}

	return total_written;
}


ssize_t faux_sendv_block(int fd, const struct iovec *iov, int iovcnt,
	const struct timespec *timeout, const sigset_t *sigmask,
	int (*isbreak_func)(void))
{
	sigset_t all_sigmask = {}; // All signals mask
	sigset_t orig_sigmask = {}; // Saved signal mask
	ssize_t bytes_num = 0;

	assert(fd != -1);
	if ((-1 == fd))
		return -1;
	if (!iov)
		return -1;
	if (iovcnt == 0)
		return 0;

	// Block signals to prevent race conditions right before pselect()
	// Catch signals while pselect() only
	// Now blocks all signals
	sigfillset(&all_sigmask);
	setsigmask(SIG_SETMASK, &all_sigmask, &orig_sigmask);

	// Signal handler can set var to interrupt exchange.
	// Get value of this var by special callback function.
	if (isbreak_func && isbreak_func())
		return -1;

	bytes_num = faux_sendv(fd, iov, iovcnt, timeout, sigmask);

	setsigmask(SIG_SETMASK, &orig_sigmask, NULL);

	return bytes_num;
}


ssize_t faux_recv(int fd, void *buf, size_t n,
	const struct timespec *timeout, const sigset_t *sigmask)
{
	size_t total_readed = 0;
	size_t left = n;
	void *data = buf;
	struct timespec now = {};
	struct timespec deadline = {};

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	// Calculate deadline - the time when timeout must occur.
	if (timeout) {
		faux_timespec_now(&now);
		faux_timespec_sum(&deadline, &now, timeout);
	}

	do {
		ssize_t bytes_readed = 0;
		struct pollfd fds = {};
		struct timespec *poll_timeout = NULL;
		struct timespec to = {};
		int sn = 0;

		if (timeout) {
			if (faux_timespec_before_now(&deadline))
				break; // Timeout already occured
			faux_timespec_now(&now);
			faux_timespec_diff(&to, &deadline, &now);
			poll_timeout = &to;
		}

		// Handlers for poll()
		faux_bzero(&fds, sizeof(fds));
		fds.fd = fd;
		fds.events = POLLIN;

		sn = ppoll(&fds, 1, poll_timeout, sigmask);
		// When kernel can't allocate some internal structures it can
		// return EAGAIN so retry.
		if ((sn < 0) && (EAGAIN == errno))
			continue;
		// All unneded signals are masked so don't process EINTR
		// in special way. Just break the loop
		if (sn < 0)
			break;
		// Timeout: break the loop. User don't want to wait any more
		if (0 == sn)
			break;
		// Some unknown event (not POLLIN). So retry polling
		if (!(fds.revents & POLLIN))
			continue;

		// The send() call is non-blocking but it's not obvious that
		// it can't return EINTR. Probably it can. Due to the fact the
		// call is non-blocking re-send() on any signal i.e. any EINTR.
		do {
			bytes_readed = recv(fd, data, left, MSG_DONTWAIT | MSG_NOSIGNAL);
		} while ((bytes_readed < 0) && (EINTR == errno));
		if (bytes_readed < 0)
			break;
		// EOF
		if (0 == bytes_readed)
			break;

		data += bytes_readed;
		left = left - bytes_readed;
		total_readed += bytes_readed;

	} while (left > 0);

	return total_readed;
}


ssize_t faux_recv_block(int fd, void *buf, size_t n,
	const struct timespec *timeout, const sigset_t *sigmask,
	int (*isbreak_func)(void))
{
	sigset_t all_sigmask = {}; // All signals mask
	sigset_t orig_sigmask = {}; // Saved signal mask
	ssize_t bytes_num = 0;

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	// Block signals to prevent race conditions right before pselect()
	// Catch signals while pselect() only
	// Now blocks all signals
	sigfillset(&all_sigmask);
	setsigmask(SIG_SETMASK, &all_sigmask, &orig_sigmask);

	// Signal handler can set var to interrupt exchange.
	// Get value of this var by special callback function.
	if (isbreak_func && isbreak_func())
		return -1;

	bytes_num = faux_recv(fd, buf, n, timeout, sigmask);

	setsigmask(SIG_SETMASK, &orig_sigmask, NULL);

	return bytes_num;
}

ssize_t faux_recvv(int fd, struct iovec *iov, int iovcnt,
	const struct timespec *timeout, const sigset_t *sigmask)
{
	size_t total_readed = 0;
	int i = 0;
	struct timespec now = {};
	struct timespec deadline = {};

	if (!iov)
		return -1;
	if (iovcnt == 0)
		return 0;

	// Calculate deadline - the time when timeout must occur.
	if (timeout) {
		faux_timespec_now(&now);
		faux_timespec_sum(&deadline, &now, timeout);
	}

	for (i = 0; i < iovcnt; i++) {
		ssize_t bytes_readed = 0;
		struct timespec *recv_timeout = NULL;
		struct timespec to = {};

		if (timeout) {
			if (faux_timespec_before_now(&deadline))
				break; // Timeout already occured
			faux_timespec_now(&now);
			faux_timespec_diff(&to, &deadline, &now);
			recv_timeout = &to;
		}
		if (iov[i].iov_len == 0)
			continue;
		bytes_readed = faux_recv(fd, iov[i].iov_base, iov[i].iov_len,
			recv_timeout, sigmask);
		if (bytes_readed < 0) { // Error
			if (total_readed != 0)
				break;
			return -1;
		}
		if (0 == bytes_readed) // EOF or timeout
			break;
		total_readed += bytes_readed;
	}

	return total_readed;
}


ssize_t faux_recvv_block(int fd, struct iovec *iov, int iovcnt,
	const struct timespec *timeout, const sigset_t *sigmask,
	int (*isbreak_func)(void))
{
	sigset_t all_sigmask = {}; // All signals mask
	sigset_t orig_sigmask = {}; // Saved signal mask
	ssize_t bytes_num = 0;

	assert(fd != -1);
	if ((-1 == fd))
		return -1;
	if (!iov)
		return -1;
	if (iovcnt == 0)
		return 0;

	// Block signals to prevent race conditions right before pselect()
	// Catch signals while pselect() only
	// Now blocks all signals
	sigfillset(&all_sigmask);
	setsigmask(SIG_SETMASK, &all_sigmask, &orig_sigmask);

	// Signal handler can set var to interrupt exchange.
	// Get value of this var by special callback function.
	if (isbreak_func && isbreak_func())
		return -1;

	bytes_num = faux_recvv(fd, iov, iovcnt, timeout, sigmask);

	setsigmask(SIG_SETMASK, &orig_sigmask, NULL);

	return bytes_num;
}
