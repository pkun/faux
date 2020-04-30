/** @file io.c
 * @brief Enchanced base IO functions.
 */

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

/** @brief Writes data to file.
 *
 * The system write() can be interrupted by signal or can write less bytes
 * than specified. This function will continue to write data until all data
 * will be written or error occured.
 *
 * @param [in] fd File descriptor.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @return Number of bytes written or < 0 on error.
 */
ssize_t faux_write(int fd, const void *buf, size_t n) {

	ssize_t bytes_written = 0;
	size_t left = n;
	const void *data = buf;

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	do {
		bytes_written = write(fd, data, left);
		if (bytes_written < 0) {
			if (EINTR == errno)
				continue;
			return -1;
		}
		if (0 == bytes_written) // Insufficient space
			return -1;
		data += bytes_written;
		left = left - bytes_written;
	} while (left > 0);

	return n;
}


/** @brief Reads data from file.
 *
 * The system read() can be interrupted by signal or can read less bytes
 * than specified. This function will continue to read data until all data
 * will be readed or error occured.
 *
 * @param [in] fd File descriptor.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @return Number of bytes readed or < 0 on error.
 */
ssize_t faux_read(int fd, void *buf, size_t n) {

	ssize_t bytes_readed = 0;
	ssize_t total_readed = 0;
	size_t left = n;
	void *data = buf;

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	do {
		bytes_readed = read(fd, data, left);
		if (bytes_readed < 0) {
			if (EINTR == errno)
				continue;
			if (total_readed > 0)
				return total_readed;
			return -1;
		}
		if (0 == bytes_readed) // EOF
			return total_readed;
		data += bytes_readed;
		total_readed += bytes_readed;
		left = left - bytes_readed;
	} while (left > 0);

	return total_readed;
}
