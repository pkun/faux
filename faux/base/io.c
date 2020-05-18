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
 * The system write() can be interrupted by signal. This function will retry to
 * write in a case of interrupted call.
 *
 * @param [in] fd File descriptor.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @return Number of bytes written or < 0 on error.
 */
ssize_t faux_write(int fd, const void *buf, size_t n) {

	ssize_t bytes_written = 0;

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	do {
		bytes_written = write(fd, buf, n);
	} while ((bytes_written < 0) && (EINTR == errno));

	return bytes_written;
}


/** @brief Writes data block to file.
 *
 * The system write() can be interrupted by signal or can write less bytes
 * than specified. This function will continue to write data until all data
 * will be written or error occured.
 *
 * @param [in] fd File descriptor.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @return Number of bytes written.
 * < n then insufficient space or error (but some data was already written).
 * < 0 - error.
 */
ssize_t faux_write_block(int fd, const void *buf, size_t n) {

	ssize_t bytes_written = 0;
	size_t total_written = 0;
	size_t left = n;
	const void *data = buf;

	do {
		bytes_written = faux_write(fd, data, left);
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


/** @brief Reads data from file.
 *
 * The system read() can be interrupted by signal. This function will retry to
 * read if it was interrupted by signal.
 *
 * @param [in] fd File descriptor.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @return Number of bytes readed or < 0 on error.
 * 0 bytes indicates EOF
 */
ssize_t faux_read(int fd, void *buf, size_t n) {

	ssize_t bytes_readed = 0;

	assert(fd != -1);
	assert(buf);
	if ((-1 == fd) || !buf)
		return -1;
	if (0 == n)
		return 0;

	do {
		bytes_readed = read(fd, buf, n);
	} while ((bytes_readed < 0) && (EINTR == errno));

	return bytes_readed;
}


/** @brief Reads data block from file.
 *
 * The system read() can be interrupted by signal or can read less bytes
 * than specified. This function will continue to read data until all data
 * will be readed or error occured.
 *
 * @param [in] fd File descriptor.
 * @param [in] buf Buffer to write.
 * @param [in] n Number of bytes to write.
 * @return Number of bytes readed.
 * < n EOF or error (but some data was already readed).
 * < 0 Error.
 */
size_t faux_read_block(int fd, void *buf, size_t n) {

	ssize_t bytes_readed = 0;
	size_t total_readed = 0;
	size_t left = n;
	void *data = buf;

	do {
		bytes_readed = read(fd, data, left);
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
