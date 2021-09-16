#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "faux/str.h"
#include "faux/async.h"
#include "faux/testc_helpers.h"


static bool_t stall_cb(faux_async_t *async, size_t len, void *user_data)
{
	bool_t *o_flag = (bool_t *)user_data;

	if (!o_flag)
		return BOOL_FALSE;
	*o_flag = BOOL_TRUE;

	async = async; // Happy compiler
	len = len; // Happy compiler

	return BOOL_TRUE;
}


int testc_faux_async_write(void)
{
	const size_t len = 9000000l;
	const size_t read_chunk = 1000;
	char *read_buf = NULL;
	ssize_t readed = 0;
	char *src_file = NULL;
	int ret = -1; // Pessimistic return value
	char *src_fn = NULL;
	char *dst_fn = NULL;
	unsigned int i = 0;
	unsigned char counter = 0;
	int fd = -1;
	faux_async_t *out = NULL;
	bool_t o_flag = BOOL_FALSE;
	int pipefd[2] = {-1, -1};

	// Prepare files
	src_file = faux_zmalloc(len);
	for (i = 0; i < len; i++) {
		src_file[i] = counter;
		counter++;
	}
	src_fn = faux_testc_tmpfile_deploy(src_file, len);

	if (pipe(pipefd) < 0)
		goto parse_error;
	read_buf = faux_malloc(read_chunk);

	dst_fn = faux_str_sprintf("%s/dst", getenv(FAUX_TESTC_TMPDIR_ENV));
	fd = open(dst_fn, O_WRONLY | O_CREAT | O_TRUNC, 0600);

	out = faux_async_new(pipefd[1]);
	faux_async_set_stall_cb(out, stall_cb, &o_flag);
	faux_async_set_write_overflow(out, len + 1);
	if (faux_async_write(out, src_file, len) < 0) {
		fprintf(stderr, "faux_async_write() error\n");
		goto parse_error;
	}

	// "Async" pipe write and sync pipe read
	while (o_flag) {
		o_flag = BOOL_FALSE;
		faux_async_out(out);
		readed = read(pipefd[0], read_buf, read_chunk);
		if (readed < 0)
			continue;
		if (write(fd, read_buf, readed) < 0)
			continue;
	}

	// Read the rest data
	close(pipefd[1]);
	pipefd[1] = -1;
	while ((readed = read(pipefd[0], read_buf, read_chunk)) > 0)
		if (write(fd, read_buf, readed) < 0)
			continue;

	// Compare etalon file and generated file
	if (faux_testc_file_cmp(dst_fn, src_fn) != 0) {
		fprintf(stderr, "Destination file %s is not equal to source %s\n",
			dst_fn, src_fn);
		goto parse_error;
	}

	ret = 0; // success

parse_error:
	if (pipefd[0] >= 0)
		close(pipefd[0]);
	if (pipefd[1] >= 0)
		close(pipefd[1]);
	faux_async_free(out);
	faux_str_free(dst_fn);
	faux_str_free(src_fn);

	return ret;
}


static bool_t read_cb(faux_async_t *async, faux_buf_t *buf, size_t len,
	void *user_data)
{
	int fd = *((int *)user_data);
	char *data = NULL;

	data = malloc(len);
	faux_buf_read(buf, data, len);
	faux_write_block(fd, data, len);
	faux_free(data);

	async = async; // Happy compiler

	return BOOL_TRUE;
}


int testc_faux_async_read(void)
{
	const size_t len = 9000000l;
	const size_t write_chunk = 2000;
	const size_t read_chunk = 5000;
	size_t left = 0;
	char *src_file = NULL;
	int ret = -1; // Pessimistic return value
	char *src_fn = NULL;
	char *dst_fn = NULL;
	unsigned int i = 0;
	unsigned char counter = 0;
	faux_async_t *out = NULL;
	int pipefd[2] = {-1, -1};
	int fd = -1;

	// Prepare files
	src_file = faux_zmalloc(len);
	for (i = 0; i < len; i++) {
		src_file[i] = counter;
		counter++;
	}
	src_fn = faux_testc_tmpfile_deploy(src_file, len);

	if (pipe(pipefd) < 0)
		goto parse_error;

	dst_fn = faux_str_sprintf("%s/dst", getenv(FAUX_TESTC_TMPDIR_ENV));
	fd = open(dst_fn, O_WRONLY | O_CREAT | O_TRUNC, 0600);

	out = faux_async_new(pipefd[0]);
	faux_async_set_read_cb(out, read_cb, &fd);
	faux_async_set_read_limits(out, read_chunk, read_chunk);

	// Sync pipe write and async pipe read
	left = len;
	while (left > 0) {
		ssize_t bytes_written = 0;

		bytes_written = write(pipefd[1], src_file + len - left,
			left < write_chunk ? left : write_chunk);
		if (bytes_written < 0)
			continue;
		left -= bytes_written;
		faux_async_in(out);
	}

	// Compare etalon file and generated file
	if (faux_testc_file_cmp(dst_fn, src_fn) != 0) {
		fprintf(stderr, "Destination file %s is not equal to source %s\n",
			dst_fn, src_fn);
		goto parse_error;
	}

	ret = 0; // success

parse_error:
	if (pipefd[0] >= 0)
		close(pipefd[0]);
	if (pipefd[1] >= 0)
		close(pipefd[1]);
	faux_async_free(out);
	faux_str_free(dst_fn);
	faux_str_free(src_fn);

	return ret;
}
