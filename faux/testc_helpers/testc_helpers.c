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


ssize_t faux_testc_file_deploy(const char *fn, const char *str) {

	faux_file_t *f = NULL;
	ssize_t bytes_written = 0;

	assert(fn);
	assert(str);
	if (!fn || !str)
		return -1;

	f = faux_file_open(fn, O_WRONLY | O_CREAT | O_TRUNC, 00644);
	if (!f)
		return -1;
	bytes_written = faux_file_write_block(f, str, strlen(str));
	faux_file_close(f);
	if (bytes_written < 0)
		return -1;

	return bytes_written;
}

char *faux_testc_tmpfile_deploy(const char *str) {

	char template[] = "/tmp/testc_tmpfile_XXXXXX";
	int fd = -1;
	faux_file_t *f = NULL;
	ssize_t bytes_written = 0;

	assert(str);
	if (!str)
		return NULL;

	fd = mkstemp(template);
	if (fd < 0)
		return NULL;

	f = faux_file_fdopen(fd);
	if (!f)
		return NULL;

	bytes_written = faux_file_write_block(f, str, strlen(str));
	faux_file_close(f);
	if (bytes_written < 0)
		return NULL;

	return faux_str_dup(template);
}
