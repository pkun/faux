/** @file net.c
 * @brief Network related functions.
 */

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <signal.h>

#include "faux/faux.h"
#include "faux/time.h"
#include "faux/net.h"

ssize_t faux_send(int fd, const void *buf, size_t n,
	const struct timespec *timeout, const sigset_t *sigmask)
{
	sigset_t all_sigmask = {}; // All signals mask
	sigset_t orig_sigmask = {}; // Saved signal mask
	fd_set fdset = {};
	ssize_t bytes_written = 0;
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

	// Block signals to prevent race conditions right before pselect()
	// Catch signals while pselect() only
	// Now blocks all signals
	sigfillset(&all_sigmask);
	sigprocmask(SIG_SETMASK, &all_sigmask, &orig_sigmask);

	// Handlers for pselect()
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	// Calculate deadline - the time when timeout must occur.
	if (timeout) {
		faux_timespec_now(&now);
		faux_timespec_sum(&deadline, &now, timeout);
	}

	do {
		struct timespec *select_timeout = NULL;
		struct timespec to = {};
		int sn = 0;

		if (timeout) {
			if (faux_timespec_before_now(&deadline))
				break; // Timeout already occured
			faux_timespec_now(&now);
			faux_timespec_diff(&to, &deadline, &now);
			select_timeout = &to;
		}

		sn = pselect(fd + 1, 0, &fdset, 0, select_timeout, sigmask);
		// All unneded signals are masked so don't process EINTR
		// in special way. Just break the loop
		if (sn < 0)
			break;

		// Timeout: break the loop. User don't want to wait any more
		if (0 == sn)
			break;

		bytes_written = send(fd, data, left, 0);
		// The send() call can't be interrupted because all signals are
		// blocked now. So any "-1" result is a really error.
		if (bytes_written < 0)
			break;

		// Insufficient space
		if (0 == bytes_written)
			break;

		data += bytes_written;
		left = left - bytes_written;
		total_written += bytes_written;

	} while (left > 0);

	sigprocmask(SIG_SETMASK, &orig_sigmask, NULL);

	return total_written;
}


/** @brief Sends data to socket.
 *
 * The system send() can be interrupted by signal. This function will retry to
 * send in a case of interrupted call.
 *
 * @param [in] fd Socket.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @param [in] flags Flags.
 * @return Number of bytes written or < 0 on error.
 */
#if 0
ssize_t faux_send(int fd, const void *buf, size_t n, int flags)
{
	ssize_t bytes_written = 0;

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	do {
		bytes_written = send(fd, buf, n, flags);
	} while ((bytes_written < 0) && (EINTR == errno));

	return bytes_written;
}


/** @brief Sends data block to socket.
 *
 * The system send() can be interrupted by signal or can write less bytes
 * than specified. This function will continue to send data until all data
 * will be sent or error occured.
 *
 * @param [in] fd Socket.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @param [in] flags Flags.
 * @return Number of bytes written.
 * < n then insufficient space or error (but some data was already written).
 * < 0 - error.
 */
ssize_t faux_send_block(int fd, const void *buf, size_t n, int flags)
{
	ssize_t bytes_written = 0;
	size_t total_written = 0;
	size_t left = n;
	const void *data = buf;

	do {
		bytes_written = faux_send(fd, data, left, flags);
		if (bytes_written < 0) { // Error
			if (total_written != 0)
				return total_written;
			return -1;
		}
		if (0 == bytes_written) // Insufficient space
			return total_written;
		data += bytes_written;
		left = left - bytes_written;
		total_written += bytes_written;
	} while (left > 0);

	return total_written;
}


/** @brief Sends struct iovec data blocks to socket.
 *
 * This function is like a faux_send_block() function but uses scatter/gather.
 *
 * @see faux_send_block().
 * @param [in] fd Socket.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @param [in] flags Flags.
 * @return Number of bytes written.
 * < n then insufficient space or error (but some data was already written).
 * < 0 - error.
 */
ssize_t faux_sendv_block(int fd, const struct iovec *iov, int iovcnt, int flags)
{
	ssize_t bytes_written = 0;
	size_t total_written = 0;
	int i = 0;

	if (!iov)
		return -1;
	if (iovcnt == 0)
		return 0;

	for (i = 0; i < iovcnt; i++) {
		if (iov[i].iov_len == 0)
			continue;
		bytes_written = faux_send_block(fd, iov[i].iov_base, iov[i].iov_len, flags);
		if (bytes_written < 0) { // Error
			if (total_written != 0)
				return total_written;
			return -1;
		}
		if (0 == bytes_written) // Insufficient space
			return total_written;
		total_written += bytes_written;
	}

	return total_written;
}


/** @brief Receive data from socket.
 *
 * The system recv() can be interrupted by signal. This function will retry to
 * receive if it was interrupted by signal.
 *
 * @param [in] fd Socket.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @param [in] flags Flags.
 * @return Number of bytes readed or < 0 on error.
 * 0 bytes indicates EOF
 */
ssize_t faux_recv(int fd, void *buf, size_t n, int flags)
{
	ssize_t bytes_readed = 0;

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	do {
		bytes_readed = recv(fd, buf, n, flags);
	} while ((bytes_readed < 0) && (EINTR == errno));

	return bytes_readed;
}


/** @brief Receive data block from socket.
 *
 * The system recv() can be interrupted by signal or can read less bytes
 * than specified. This function will continue to read data until all data
 * will be readed or error occured.
 *
 * @param [in] fd Socket.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @param [in] flags Flags.
 * @return Number of bytes readed.
 * < n EOF or error (but some data was already readed).
 * < 0 Error.
 */
size_t faux_recv_block(int fd, void *buf, size_t n, int flags)
{
	ssize_t bytes_readed = 0;
	size_t total_readed = 0;
	size_t left = n;
	void *data = buf;

	do {
		bytes_readed = recv(fd, data, left, flags);
		if (bytes_readed < 0) {
			if (total_readed != 0)
				return total_readed;
			return -1;
		}
		if (0 == bytes_readed) // EOF
			return total_readed;
		data += bytes_readed;
		left = left - bytes_readed;
		total_readed += bytes_readed;
	} while (left > 0);

	return total_readed;
}

#endif
