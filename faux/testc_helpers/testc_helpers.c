/** @file testc_helpers.c
 * @brief Testc helper functions
 *
 * This file implements helpers for writing tests for 'testc' utility.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "faux/ctype.h"
#include "faux/str.h"
#include "faux/file.h"
#include "faux/testc_helpers.h"


ssize_t faux_testc_file_deploy(const char *fn, const void *buf, size_t len)
{
	faux_file_t *f = NULL;
	ssize_t bytes_written = 0;

	assert(fn);
	assert(buf);
	if (!fn || !buf)
		return -1;

	f = faux_file_open(fn, O_WRONLY | O_CREAT | O_TRUNC, 00644);
	if (!f)
		return -1;
	bytes_written = faux_file_write_block(f, buf, len);
	faux_file_close(f);
	if (bytes_written < 0)
		return -1;

	return bytes_written;
}


ssize_t faux_testc_file_deploy_str(const char *fn, const char *str)
{
	assert(str);
	if (!str)
		return -1;

	return faux_testc_file_deploy(fn, str, strlen(str));
}


char *faux_testc_tmpfile_deploy(const void *buf, size_t len)
{
	char *template = NULL;
	int fd = -1;
	faux_file_t *f = NULL;
	ssize_t bytes_written = 0;
	char *env_tmpdir = NULL;

	assert(buf);
	if (!buf)
		return NULL;

	env_tmpdir = getenv(FAUX_TESTC_TMPDIR_ENV);
	if (env_tmpdir)
		template = faux_str_sprintf("%s/tmpfile-XXXXXX", env_tmpdir);
	else
		template = faux_str_sprintf("/tmp/testc-tmpfile-XXXXXX");
	assert(template);
	if (!template)
		return NULL;

	fd = mkstemp(template);
	if (fd < 0)
		return NULL;

	f = faux_file_fdopen(fd);
	if (!f)
		return NULL;

	bytes_written = faux_file_write_block(f, buf, len);
	faux_file_close(f);
	if (bytes_written < 0)
		return NULL;

	return template;
}


char *faux_testc_tmpfile_deploy_str(const char *str)
{
	assert(str);
	if (!str)
		return NULL;

	return faux_testc_tmpfile_deploy(str, strlen(str));
}


#define CHUNK_SIZE 1024

int faux_testc_file_cmp(const char *first_file, const char *second_file)
{
	int ret = -1; // Pessimistic retval
	faux_file_t *f = NULL;
	faux_file_t *s = NULL;
	char buf_f[CHUNK_SIZE];
	char buf_s[CHUNK_SIZE];
	ssize_t readed_f = 0;
	ssize_t readed_s = 0;

	assert(first_file);
	assert(second_file);
	if (!first_file || !second_file)
		return -1;

	f = faux_file_open(first_file, O_RDONLY, 0);
	s = faux_file_open(second_file, O_RDONLY, 0);
	if (!f || !s)
		goto cmp_error;

	do {
		readed_f = faux_file_read_block(f, buf_f, CHUNK_SIZE);
		readed_s = faux_file_read_block(s, buf_s, CHUNK_SIZE);
		if (readed_f != readed_s)
			goto cmp_error;
		if (readed_f < 0)
			goto cmp_error;
		if (0 == readed_f)
			break; // EOF
		if (memcmp(buf_f, buf_s, readed_f) != 0)
			goto cmp_error;
	} while (CHUNK_SIZE == readed_f); // Not full chunk so EOF is near

	ret = 0; // Equal

cmp_error:
	faux_file_close(f);
	faux_file_close(s);

	return ret;
}
