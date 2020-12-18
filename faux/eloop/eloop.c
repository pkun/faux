/** @file eloop.c
 * @brief Class for
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <sys/signalfd.h>

#include "faux/faux.h"
#include "faux/str.h"
#include "faux/net.h"
#include "faux/sched.h"
#include "faux/eloop.h"

#include "private.h"


static int faux_eloop_sched_compare(const void *first, const void *second)
{
	const faux_eloop_sched_t *f = (const faux_eloop_sched_t *)first;
	const faux_eloop_sched_t *s = (const faux_eloop_sched_t *)second;

	return (f->ev_id - s->ev_id);
}


static int faux_eloop_sched_kcompare(const void *key, const void *list_item)
{
	int *f = (int *)key;
	const faux_eloop_sched_t *s = (const faux_eloop_sched_t *)list_item;

	return (*f - s->ev_id);
}


static int faux_eloop_fd_compare(const void *first, const void *second)
{
	const faux_eloop_fd_t *f = (const faux_eloop_fd_t *)first;
	const faux_eloop_fd_t *s = (const faux_eloop_fd_t *)second;

	return (f->fd - s->fd);
}


static int faux_eloop_fd_kcompare(const void *key, const void *list_item)
{
	int *f = (int *)key;
	const faux_eloop_fd_t *s = (const faux_eloop_fd_t *)list_item;

	return (*f - s->fd);
}


static int faux_eloop_signal_compare(const void *first, const void *second)
{
	const faux_eloop_signal_t *f = (const faux_eloop_signal_t *)first;
	const faux_eloop_signal_t *s = (const faux_eloop_signal_t *)second;

	return (f->signo - s->signo);
}


static int faux_eloop_signal_kcompare(const void *key, const void *list_item)
{
	int *f = (int *)key;
	const faux_eloop_signal_t *s = (const faux_eloop_signal_t *)list_item;

	return (*f - s->signo);
}


faux_eloop_t *faux_eloop_new(faux_eloop_cb_f *default_event_cb)
{
	faux_eloop_t *eloop = NULL;

	eloop = faux_zmalloc(sizeof(*eloop));
	assert(eloop);
	if (!eloop)
		return NULL;

	// Init
	eloop->default_event_cb = default_event_cb;

	// Sched
	eloop->scheds = faux_list_new(FAUX_LIST_SORTED, FAUX_LIST_UNIQUE,
		faux_eloop_sched_compare, faux_eloop_sched_kcompare, faux_free);
	assert(eloop->scheds);
	eloop->faux_sched = faux_sched_new();
	assert(eloop->faux_sched);

	// FD
	eloop->fds = faux_list_new(FAUX_LIST_SORTED, FAUX_LIST_UNIQUE,
		faux_eloop_fd_compare, faux_eloop_fd_kcompare, faux_free);
	assert(eloop->fds);
	eloop->pollfds = faux_pollfd_new();
	assert(eloop->pollfds);

	// Signal
	eloop->signals = faux_list_new(FAUX_LIST_SORTED, FAUX_LIST_UNIQUE,
		faux_eloop_signal_compare, faux_eloop_signal_kcompare, faux_free);
	assert(eloop->signals);
	sigemptyset(&eloop->sig_set);

	return eloop;
}


void faux_eloop_free(faux_eloop_t *eloop)
{
	if (!eloop)
		return;

	faux_list_free(eloop->signals);
	faux_pollfd_free(eloop->pollfds);
	faux_list_free(eloop->fds);
	faux_sched_free(eloop->faux_sched);
	faux_list_free(eloop->scheds);

	faux_free(eloop);
}


bool_t faux_eloop_loop(faux_eloop_t *eloop)
{
	bool_t retval = BOOL_TRUE;
	bool_t stop = BOOL_FALSE;
	sigset_t blocked_signals;
	sigset_t orig_sig_set;

#ifdef HAVE_SIGNALFD
	int signal_fd = -1;
#endif

	// Block signals to prevent race conditions while loop and ppoll()
	// Catch signals while ppoll() only
	sigfillset(&blocked_signals);
	sigprocmask(SIG_SETMASK, &blocked_signals, &orig_sig_set);

#ifdef HAVE_SIGNALFD
	// Create Linux-specific signal file descriptor. Wait for all signals.
	// Unneeded signals will be filtered out later.
	signal_fd = signalfd(-1, &blocked_signals, SFD_NONBLOCK | SFD_CLOEXEC);
	faux_pollfd_add(eloop->pollfds, signal_fd, POLLIN);
#endif

/*
	// Set signal handler
	syslog(LOG_DEBUG, "Set signal handlers\n");
	sigemptyset(&sig_set);
	sigaddset(&sig_set, SIGTERM);
	sigaddset(&sig_set, SIGINT);
	sigaddset(&sig_set, SIGQUIT);

	sig_act.sa_flags = 0;
	sig_act.sa_mask = sig_set;
	sig_act.sa_handler = &sighandler;
	sigaction(SIGTERM, &sig_act, NULL);
	sigaction(SIGINT, &sig_act, NULL);
	sigaction(SIGQUIT, &sig_act, NULL);

	// SIGHUP handler
	sigemptyset(&sig_set);
	sigaddset(&sig_set, SIGHUP);

	sig_act.sa_flags = 0;
	sig_act.sa_mask = sig_set;
	sig_act.sa_handler = &sighup_handler;
	sigaction(SIGHUP, &sig_act, NULL);

	// SIGCHLD handler
	sigemptyset(&sig_set);
	sigaddset(&sig_set, SIGCHLD);

	sig_act.sa_flags = 0;
	sig_act.sa_mask = sig_set;
	sig_act.sa_handler = &sigchld_handler;
	sigaction(SIGCHLD, &sig_act, NULL);

*/



	// Main loop
	while (!stop) {
		int sn = 0;
		struct timespec *timeout = NULL;
//		struct timespec next_interval = {};
		faux_pollfd_iterator_t pollfd_iter;
		struct pollfd *pollfd = NULL;
//		pid_t pid = -1;

		// Re-read config file on SIGHUP
/*		if (sighup) {
			if (access(opts->cfgfile, R_OK) == 0) {
				syslog(LOG_INFO, "Re-reading config file \"%s\"\n", opts->cfgfile);
				if (config_parse(opts->cfgfile, opts) < 0)
					syslog(LOG_ERR, "Error while config file parsing.\n");
			} else if (opts->cfgfile_userdefined) {
				syslog(LOG_ERR, "Can't find config file \"%s\"\n", opts->cfgfile);
			}
			sighup = 0;
		}
*/
		// Find out next scheduled interval
/*		if (faux_sched_next_interval(eloop->sched, &next_interval) < 0)
			timeout = NULL;
		else
			timeout = &next_interval;
*/
		// Wait for events
//		sn = ppoll(faux_pollfd_vector(fds), faux_pollfd_len(fds), timeout, &orig_sig_set);
		sn = ppoll(faux_pollfd_vector(eloop->pollfds), faux_pollfd_len(eloop->pollfds), timeout, NULL);
		if (sn < 0) {
			if ((EAGAIN == errno) || (EINTR == errno))
				continue;
			retval = BOOL_FALSE;
printf("ppoll() error\n");
			break;
		}

		// Scheduled event
		if (0 == sn) {
//			int id = 0; // Event idenftifier
//			void *data = NULL; // Event data
//			faux_eloop_info_sched_t info = {};

printf("Sheduled event\n");
			// Some scheduled events
/*			while(faux_sched_pop(sched, &id, &data) == 0) {
				syslog(LOG_DEBUG, "sched: Update event\n");
			}
*/			continue;
		}

		
		// File descriptor
		faux_pollfd_init_iterator(eloop->pollfds, &pollfd_iter);
		while ((pollfd = faux_pollfd_each_active(eloop->pollfds, &pollfd_iter))) {
			int fd = pollfd->fd;
			faux_eloop_info_fd_t info = {};
			faux_eloop_cb_f *event_cb = NULL;
			faux_eloop_fd_t *entry = NULL;
			bool_t r = BOOL_TRUE;

#ifdef HAVE_SIGNALFD
			// Read special signal file descriptor
			if (fd == signal_fd) {
				struct signalfd_siginfo signal_info = {};

				while (faux_read_block(fd, &signal_info,
					sizeof(signal_info)) == sizeof(signal_info)) {
					faux_eloop_info_signal_t sinfo = {};
					faux_eloop_signal_t *sentry =
						(faux_eloop_signal_t *)faux_list_kfind(
						eloop->signals, &signal_info.ssi_signo);

					if (!sentry) // Not registered signal. Drop it.
						continue;
					event_cb = sentry->context.event_cb;
					if (!event_cb)
						event_cb = eloop->default_event_cb;
					if (!event_cb) // Callback is not defined
						continue;
					sinfo.signo = sentry->signo;

					// Execute callback
					r = event_cb(eloop, FAUX_ELOOP_SIGNAL, &sinfo,
						sentry->context.user_data);
					// BOOL_FALSE return value means "break the loop"
					if (!r)
						stop = BOOL_TRUE;
				}
				continue; // Another fds are common, not signal
			}
#endif

			// Prepare event data
			entry = (faux_eloop_fd_t *)faux_list_kfind(eloop->fds, &fd);
			assert(entry);
			if (!entry) // Something went wrong
				continue;
			event_cb = entry->context.event_cb;
			if (!event_cb)
				event_cb = eloop->default_event_cb;
			if (!event_cb) // Callback function is not defined for this event
				continue;
			info.fd = fd;
			info.revents = pollfd->revents;

			// Execute callback
			r = event_cb(eloop, FAUX_ELOOP_FD, &info, entry->context.user_data);
			// BOOL_FALSE return value means "break the loop"
			if (!r)
				stop = BOOL_TRUE;
		}

	} // Loop end

#ifdef HAVE_SIGNALFD
	// Close signal file descriptor
	faux_pollfd_del_by_fd(eloop->pollfds, signal_fd);
	close(signal_fd);
#endif

	// Unblock signals
	sigprocmask(SIG_SETMASK, &orig_sig_set, NULL);


	return retval;
}


bool_t faux_eloop_add_fd(faux_eloop_t *eloop, int fd, short events,
	faux_eloop_cb_f *event_cb, void *user_data)
{
	faux_eloop_fd_t *entry = NULL;
	faux_list_node_t *new_node = NULL;

	if (!eloop || (fd < 0))
		return BOOL_FALSE;

	entry = faux_zmalloc(sizeof(*entry));
	if (!entry)
		return BOOL_FALSE;
	entry->fd = fd;
	entry->events = events;
	entry->context.event_cb = event_cb;
	entry->context.user_data = user_data;

	if (!(new_node = faux_list_add(eloop->fds, entry))) {
		faux_free(entry);
		return BOOL_FALSE;
	}

	if (!faux_pollfd_add(eloop->pollfds, entry->fd, entry->events)) {
		faux_list_del(eloop->fds, new_node);
		faux_free(entry);
		return BOOL_FALSE;
	}

	return BOOL_TRUE;
}


bool_t faux_eloop_del_fd(faux_eloop_t *eloop, int fd)
{
	if (!eloop || (fd < 0))
		return BOOL_FALSE;

	if (faux_list_kdel(eloop->fds, &fd) < 0)
		return BOOL_FALSE;

	if (faux_pollfd_del_by_fd(eloop->pollfds, fd) < 0)
		return BOOL_FALSE;

	return BOOL_TRUE;
}


bool_t faux_eloop_add_signal(faux_eloop_t *eloop, int signo,
	faux_eloop_cb_f *event_cb, void *user_data)
{
	faux_eloop_signal_t *entry = NULL;

	if (!eloop || (signo < 0))
		return BOOL_FALSE;

	if (sigismember(&eloop->sig_set, signo) == 1)
		return BOOL_FALSE; // Already exists

	// Firstly try to add signal to sigset. Library function will validate
	// signal number value.
	if (sigaddset(&eloop->sig_set, signo) < 0)
		return BOOL_FALSE; // Invalid signal number

	entry = faux_zmalloc(sizeof(*entry));
	if (!entry) {
		sigdelset(&eloop->sig_set, signo);
		return BOOL_FALSE;
	}
	entry->signo = signo;
	entry->context.event_cb = event_cb;
	entry->context.user_data = user_data;

	if (!faux_list_add(eloop->signals, entry)) {
		faux_free(entry);
		sigdelset(&eloop->sig_set, signo);
		return BOOL_FALSE;
	}

	return BOOL_TRUE;
}


bool_t faux_eloop_del_signal(faux_eloop_t *eloop, int signo)
{
	if (!eloop || (signo < 0))
		return BOOL_FALSE;

	if (sigismember(&eloop->sig_set, signo) != 1)
		return BOOL_FALSE; // Doesn't exist

	sigdelset(&eloop->sig_set, signo);
	faux_list_kdel(eloop->signals, &signo);

	return BOOL_TRUE;
}
