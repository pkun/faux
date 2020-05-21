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

/** @brief Removes filesystem objects recursively.
 *
 * Function can remove file or directory (recursively).
 *
 * @param [in] path File/directory name.
 * @return 0 - success, < 0 on error.
 */
int faux_rm(const char *path)
{
	struct stat statbuf = {};
	DIR *dir = NULL;
	struct dirent *dir_entry = NULL;

	assert(path);
	if (!path)
		return -1;

	if (lstat(path, &statbuf) < 0)
		return -1;

	// Common file (not dir)
	if (!S_ISDIR(statbuf.st_mode))
		return unlink(path);

	// Directory
	if ((dir = opendir(path)) == NULL)
		return -1;
	while ((dir_entry = readdir(dir))) {
		if (!strcmp(dir_entry->d_name, ".") ||
			!strcmp(dir_entry->d_name, ".."))
			continue;
		faux_rm(dir_entry->d_name);
	}
	closedir(dir);

	return rmdir(path);
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
