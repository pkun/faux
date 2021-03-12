/** @file fs.c
 * @brief Enchanced base filesystem operations.
 */

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "faux/str.h"


/** @brief Reports size of file or directory.
 *
 * Function works recursively so directory size is a sum of all file size
 * inside it and size of subdirs.
 *
 * @param [in] path Filesystem path.
 * @return Size of filesystem object or < 0 on error.
 */
ssize_t faux_filesize(const char *path)
{
	struct stat statbuf = {};
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	ssize_t sum = 0;

	assert(path);
	if (!path)
		return -1;

	if (stat(path, &statbuf) < 0)
		return -1;

	// Regular file
	if (!S_ISDIR(statbuf.st_mode))
		return statbuf.st_size;

	// Directory
	dir = opendir(path);
	if (!dir)
		return -1;
	// Get each file from 'path' directory
	for (entry = readdir(dir); entry; entry = readdir(dir)) {
		char *fn = NULL;
		ssize_t r = 0;
		// Ignore "." and ".."
		if ((faux_str_casecmp(entry->d_name, ".") == 0) ||
			(faux_str_casecmp(entry->d_name, "..") == 0))
			continue;
		// Construct filename
		fn = faux_str_sprintf("%s/%s", path, entry->d_name);
		r = faux_filesize(fn);
		faux_str_free(fn);
		if (r < 0)
			continue;
		sum += r;
	}
	closedir(dir);

	return sum;
}


/** @brief If given path is directory.
 *
 * @param [in] path Filesystem path.
 * @return 0 - success, < 0 on error.
 */
bool_t faux_isdir(const char *path)
{
	struct stat statbuf = {};

	assert(path);
	if (!path)
		return BOOL_FALSE;

	if (stat(path, &statbuf) < 0)
		return BOOL_FALSE;

	if (S_ISDIR(statbuf.st_mode))
		return BOOL_TRUE;

	return BOOL_FALSE;
}


/** @brief If given path is regular file.
 *
 * @param [in] path Filesystem path.
 * @return 0 - success, < 0 on error.
 */
bool_t faux_isfile(const char *path)
{
	struct stat statbuf = {};

	assert(path);
	if (!path)
		return BOOL_FALSE;

	if (stat(path, &statbuf) < 0)
		return BOOL_FALSE;

	if (S_ISREG(statbuf.st_mode))
		return BOOL_TRUE;

	return BOOL_FALSE;
}


/** @brief Removes filesystem objects recursively.
 *
 * Function can remove file or directory (recursively).
 *
 * @param [in] path File/directory name.
 * @return BOOL_TRUE - success, BOOL_FALSE on error.
 */
bool_t faux_rm(const char *path)
{
	DIR *dir = NULL;
	struct dirent *dir_entry = NULL;

	assert(path);
	if (!path)
		return BOOL_FALSE;

	// Common file (not dir)
	if (!faux_isdir(path)) {
		if (unlink(path) < 0)
			return BOOL_FALSE;
		return BOOL_TRUE;
	}

	// Directory
	if ((dir = opendir(path)) == NULL)
		return BOOL_FALSE;
	while ((dir_entry = readdir(dir))) {
		if (!strcmp(dir_entry->d_name, ".") ||
			!strcmp(dir_entry->d_name, ".."))
			continue;
		faux_rm(dir_entry->d_name);
	}
	closedir(dir);

	if (rmdir(path) < 0)
		return BOOL_FALSE;

	return BOOL_TRUE;
}


/** @brief Expand tilde within path due to HOME env var.
 *
 * If first character of path is tilde then expand it to value of
 * environment variable HOME. If tilde is not the first character or
 * HOME is not defined then return copy of original path.
 *
 * @warning The resulting string must be freed by faux_str_free() later.
 *
 * @param [in] path Path to expand.
 * @return Expanded string or NULL on error.
 */
char *faux_expand_tilde(const char *path)
{
	char *home_dir = getenv("HOME");
	char *result = NULL;

	assert(path);
	if (!path)
		return NULL;

	// Tilde can be the first character only to be expanded
	if (home_dir && (path[0] == '~'))
		result = faux_str_sprintf("%s%s", home_dir, &path[1]);
	else
		result = faux_str_dup(path);

	return result;
}
