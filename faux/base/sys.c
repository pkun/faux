/** @file sys.c
 * @brief System-related faux functions.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "faux/faux.h"


/** Implementation of daemon() function.
 *
 * The original daemon() function is not POSIX. Additionally parent doesn't
 * create PID file as UNIX services do. So after parent exiting there is no
 * PID file yet. This function fix that problems.
 *
 * @param [in] nochdir On zero changes working dir to "/". Compatible with daemon().
 * @param [in] noclose On zero redirects standard streams to "/dev/null". Compatible with daemon().
 * @param [in] pidfile Name of PID file to create.
 * @param [in] mode PID file mode.
 * @return BOOL_TRUE on success, BOOL_FALSE else.
 * @sa daemon()
 */
bool_t faux_daemon(int nochdir, int noclose, const char *pidfile, mode_t mode)
{
	pid_t pid = -1;

	pid = fork();
	if (-1 == pid)
		return BOOL_FALSE;

	// Parent
	if (pid > 0) {
		// Parent writes PID file
		if (pidfile && (pidfile[0] != '\0')) {
			int fd = -1;
			if ((fd = open(pidfile,
				O_WRONLY | O_CREAT | O_EXCL | O_TRUNC,
				mode)) >= 0) {
				char str[20] = {};
				snprintf(str, sizeof(str), "%u\n", pid);
				str[sizeof(str) - 1] = '\0';
				write(fd, str, strlen(str));
				close(fd);
			}
		}
		_exit(0); // Exit parent
	}

	// Child
	if (setsid() == -1)
		return BOOL_FALSE;
	if (0 == nochdir) {
		if (chdir("/"))
			return BOOL_FALSE;
	}
	if (0 == noclose) {
		int fd = -1;
		fd = open("/dev/null", O_RDWR, 0);
		if (fd < 0)
			return BOOL_FALSE;
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO)
			close(fd);
	}

	return BOOL_TRUE;
}
