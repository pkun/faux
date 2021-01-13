#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "faux/faux.h"
#include "faux/str.h"
#include "faux/testc_helpers.h"


int testc_faux_filesize(void)
{
	const char *dirname = "subdir";
	const char *basedir = getenv(FAUX_TESTC_TMPDIR_ENV);
	const char *fd1 = "asdfghjkl"; // 9 bytes
	const char *fd2 = "asdfghjklzxcvbnm"; // 16 bytes
	const char *fd3 = "asdfghjklzxcvbnmqwertyuiop"; // 26 bytes
	ssize_t r = 0;
	ssize_t etalon_filesize = 0;
	int ret = -1; // Pessimistic
	char *fn1 = NULL;
	char *dn1 = NULL;
	char *fn2 = NULL;
	char *fn3 = NULL;

	// Prepare filenames
	fn1 = faux_str_sprintf("%s/f1", basedir);
	dn1 = faux_str_sprintf("%s/%s", basedir, dirname);
	fn2 = faux_str_sprintf("%s/f2", dn1);
	fn3 = faux_str_sprintf("%s/f3", dn1);

	// Create files and dirs
	mkdir(dn1, 0777);
	if ((r = faux_testc_file_deploy_str(fn1, fd1)) < 0)
		goto err;
	etalon_filesize += r;
	if ((r = faux_testc_file_deploy_str(fn2, fd2)) < 0)
		goto err;
	etalon_filesize += r;
	if ((r = faux_testc_file_deploy_str(fn3, fd3)) < 0)
		goto err;
	etalon_filesize += r;

	// Debug
	printf("Dir: %s\n", basedir);
	printf("File: %s\n", fn1);

	// Get filesize
	if ((ssize_t)strlen(fd1) != faux_filesize(fn1)) {
		printf("Wrong filesize (%ld %ld)\n",
			strlen(fd1), faux_filesize(fn1));
		goto err;
	}
	if (etalon_filesize != faux_filesize(basedir)) {
		printf("Wrong dirsize (%ld %ld)\n",
			etalon_filesize, faux_filesize(basedir));
		goto err;
	}

	ret = 0;
err:
	faux_str_free(fn1);
	faux_str_free(dn1);
	faux_str_free(fn2);
	faux_str_free(fn3);

	return ret;
}
