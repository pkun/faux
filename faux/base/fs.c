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

/** @brief Removes filesystem objects recursively.
 *
 * Function can remove file or directory (recursively).
 *
 * @param [in] path File/directory name.
 * @return 0 - success, < 0 on error.
 */
int faux_rm(const char *path) {

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
