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

#ifdef HAVE_SIGNALFD
#define SIGNALFD_FLAGS (SFD_NONBLOCK | SFD_CLOEXEC)

#else // Standard signals
static void *faux_eloop_static_user_data = NULL;

static void faux_eloop_static_sighandler(int signo)
{
	faux_list_t *signal_list =
		(faux_list_t *)faux_eloop_static_user_data;
	faux_eloop_signal_t *signal = NULL;

	if (!signal_list)
		return;
	signal = faux_list_kfind(signal_list, &signo);
	if (!signal)
		return;
	signal->set = BOOL_TRUE;
}
#endif

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
	eloop->working = BOOL_FALSE;
	eloop->default_event_cb = default_event_cb;

	// Sched
	eloop->sched = faux_sched_new();
	assert(eloop->sched);

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
	sigfillset(&eloop->sig_mask);
#ifdef HAVE_SIGNALFD
	eloop->signal_fd = -1;
#endif

	return eloop;
}


void faux_eloop_free(faux_eloop_t *eloop)
{
	if (!eloop)
		return;

	faux_list_free(eloop->signals);
	faux_pollfd_free(eloop->pollfds);
	faux_list_free(eloop->fds);
	faux_sched_free(eloop->sched);

	faux_free(eloop);
}


bool_t faux_eloop_loop(faux_eloop_t *eloop)
{
	bool_t retval = BOOL_TRUE;
	bool_t stop = BOOL_FALSE;
	sigset_t blocked_signals;
	sigset_t orig_sig_set;
	sigset_t *sigset_for_ppoll = NULL;

	// If event loop is active already and we try to start nested loop
	// then return.
	if (eloop->working)
		return BOOL_FALSE;
	eloop->working = BOOL_TRUE;

	// Block signals to prevent race conditions while loop and ppoll()
	// Catch signals while ppoll() only
	sigfillset(&blocked_signals);
	sigprocmask(SIG_SETMASK, &blocked_signals, &orig_sig_set);

#ifdef HAVE_SIGNALFD
	// Create Linux-specific signal file descriptor. Wait for all signals.
	// Unneeded signals will be filtered out later.
	eloop->signal_fd = signalfd(eloop->signal_fd, &eloop->sig_set,
		SIGNALFD_FLAGS);
	faux_pollfd_add(eloop->pollfds, eloop->signal_fd, POLLIN);

#else // Standard signal processing
	sigset_for_ppoll = &eloop->sig_mask;
	faux_eloop_static_user_data = eloop->signals;

	if (faux_list_len(eloop->signals) != 0) {
		faux_list_node_t *iter = faux_list_head(eloop->signals);
		faux_eloop_signal_t *sig = NULL;
		struct sigaction sig_act = {};

		sig_act.sa_flags = 0;
		sig_act.sa_mask = eloop->sig_set;
		sig_act.sa_handler = &faux_eloop_static_sighandler;
		while ((sig = (faux_eloop_signal_t *)faux_list_each(&iter))) {
			sig->set = BOOL_FALSE;
			sigaction(sig->signo, &sig_act, &sig->oldact);
		}
	}
#endif

	// Main loop
	while (!stop) {
		int sn = 0;
		struct timespec *timeout = NULL;
		struct timespec next_interval = {};
		faux_pollfd_iterator_t pollfd_iter;
		struct pollfd *pollfd = NULL;

		// Find out next scheduled interval
		if (faux_sched_next_interval(eloop->sched, &next_interval) < 0)
			timeout = NULL;
		else
			timeout = &next_interval;

		// Wait for events
		sn = ppoll(faux_pollfd_vector(eloop->pollfds),
			faux_pollfd_len(eloop->pollfds), timeout, sigset_for_ppoll);

		if ((sn < 0) && (errno != EINTR)) {
			retval = BOOL_FALSE;
			break;
		}

#ifndef HAVE_SIGNALFD // Standard signals
		// Signals
		if ((sn < 0) && (EINTR == errno)) {
			faux_list_node_t *iter = faux_list_head(eloop->signals);
			faux_eloop_signal_t *sig = NULL;
			while ((sig = (faux_eloop_signal_t *)faux_list_each(&iter))) {
				faux_eloop_info_signal_t sinfo = {};
				faux_eloop_cb_f *event_cb = NULL;
				bool_t r = BOOL_TRUE;

				if (BOOL_FALSE == sig->set)
					continue;
				sig->set = BOOL_FALSE;

				event_cb = sig->context.event_cb;
				if (!event_cb)
					event_cb = eloop->default_event_cb;
				if (!event_cb) // Callback is not defined
					continue;
				sinfo.signo = sig->signo;

				// Execute callback
				r = event_cb(eloop, FAUX_ELOOP_SIGNAL, &sinfo,
					sig->context.user_data);
				// BOOL_FALSE return value means "break the loop"
				if (!r)
					stop = BOOL_TRUE;
			}
		}
#endif

		// Scheduled event
		if (0 == sn) {
			int ev_id = 0; // Event idenftifier
			faux_eloop_context_t *context = NULL; // Event data

			// Some scheduled events
			while(faux_sched_pop(eloop->sched, &ev_id, (void **)&context) == 0) {
				faux_eloop_info_sched_t info = {};
				faux_eloop_cb_f *event_cb = NULL;
				bool_t r = BOOL_TRUE;

				event_cb = context->event_cb;
				if (!event_cb)
					event_cb = eloop->default_event_cb;
				if (!event_cb) // Callback is not defined
					continue;
				info.ev_id = ev_id;

				// Execute callback
				r = event_cb(eloop, FAUX_ELOOP_SCHED, &info,
					context->user_data);
				// BOOL_FALSE return value means "break the loop"
				if (!r)
					stop = BOOL_TRUE;

				// Free non-periodic event's data
				if (!faux_sched_id_exist(eloop->sched, ev_id))
					faux_free(context);
			}
			continue;
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
			if (fd == eloop->signal_fd) {
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
	faux_pollfd_del_by_fd(eloop->pollfds, eloop->signal_fd);
	close(eloop->signal_fd);
	eloop->signal_fd = -1;

#else // Standard signals. Restore signal handlers
	if (faux_list_len(eloop->signals) != 0) {
		faux_list_node_t *iter = faux_list_head(eloop->signals);
		faux_eloop_signal_t *sig = NULL;

		while ((sig = (faux_eloop_signal_t *)faux_list_each(&iter))) {
			sig->set = BOOL_FALSE;
			sigaction(sig->signo, &sig->oldact, NULL);
		}
	}
#endif

	// Unblock signals
	sigprocmask(SIG_SETMASK, &orig_sig_set, NULL);

	// Deactivate loop flag
	eloop->working = BOOL_FALSE;

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

	if (!faux_pollfd_del_by_fd(eloop->pollfds, fd))
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
	sigdelset(&eloop->sig_mask, signo);

	entry = faux_zmalloc(sizeof(*entry));
	if (!entry) {
		sigdelset(&eloop->sig_set, signo);
		sigaddset(&eloop->sig_mask, signo);
		return BOOL_FALSE;
	}
	entry->signo = signo;
	entry->set = BOOL_FALSE;
	entry->context.event_cb = event_cb;
	entry->context.user_data = user_data;

	if (!faux_list_add(eloop->signals, entry)) {
		faux_free(entry);
		sigdelset(&eloop->sig_set, signo);
		sigaddset(&eloop->sig_mask, signo);
		return BOOL_FALSE;
	}

	if (eloop->working) { // Add signal on the fly
#ifdef HAVE_SIGNALFD
		// Reattach signalfd handler with updated sig_set
		eloop->signal_fd = signalfd(eloop->signal_fd, &eloop->sig_set,
			SIGNALFD_FLAGS);

#else // Standard signals
		struct sigaction sig_act = {};
		sig_act.sa_flags = 0;
		sig_act.sa_mask = eloop->sig_set;
		sig_act.sa_handler = &faux_eloop_static_sighandler;
		sigaction(signo, &sig_act, &entry->oldact);
#endif
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
	sigaddset(&eloop->sig_mask, signo);

	if (eloop->working) { // Del signal on the fly
#ifdef HAVE_SIGNALFD
		// Reattach signalfd handler with updated sig_set
		eloop->signal_fd = signalfd(eloop->signal_fd, &eloop->sig_set,
			SIGNALFD_FLAGS);

#else // Standard signals
		faux_eloop_signal_t *sig = faux_list_kfind(eloop->signals, &signo);
		sigaction(signo, &sig->oldact, NULL);
#endif
	}

	faux_list_kdel(eloop->signals, &signo);

	return BOOL_TRUE;
}


static faux_eloop_context_t *faux_eloop_new_context(
	faux_eloop_cb_f *event_cb, void *data)
{
	faux_eloop_context_t *context = NULL;

	context = faux_zmalloc(sizeof(*context));
	assert(context);
	if (!context)
		return NULL;

	context->event_cb = event_cb;
	context->user_data = data;

	return context;
}


bool_t faux_eloop_add_sched_once(faux_eloop_t *eloop, const struct timespec *time,
	int ev_id, faux_eloop_cb_f *event_cb, void *data)
{
	faux_eloop_context_t *context = NULL;

	assert(eloop);
	if (!eloop)
		return BOOL_FALSE;

	if (faux_sched_id_exist(eloop->sched, ev_id))
		return BOOL_FALSE; // ID must be unique

	context = faux_eloop_new_context(event_cb, data);
	assert(context);
	if (!context)
		return BOOL_FALSE;

	if (faux_sched_once(eloop->sched, time, ev_id, context) < 0) {
		faux_free(context);
		return BOOL_FALSE;
	}

	return BOOL_TRUE;
}


bool_t faux_eloop_add_sched_once_delayed(faux_eloop_t *eloop, const struct timespec *interval,
	int ev_id, faux_eloop_cb_f *event_cb, void *data)
{
	faux_eloop_context_t *context = NULL;

	assert(eloop);
	if (!eloop)
		return BOOL_FALSE;

	if (faux_sched_id_exist(eloop->sched, ev_id))
		return BOOL_FALSE; // ID must be unique

	context = faux_eloop_new_context(event_cb, data);
	assert(context);
	if (!context)
		return BOOL_FALSE;

	if (faux_sched_once_delayed(eloop->sched, interval, ev_id, context) < 0) {
		faux_free(context);
		return BOOL_FALSE;
	}

	return BOOL_TRUE;
}


bool_t faux_eloop_add_sched_periodic(faux_eloop_t *eloop, const struct timespec *time,
	int ev_id, faux_eloop_cb_f *event_cb, void *data,
	const struct timespec *period, unsigned int cycle_num)
{
	faux_eloop_context_t *context = NULL;

	assert(eloop);
	if (!eloop)
		return BOOL_FALSE;

	if (faux_sched_id_exist(eloop->sched, ev_id))
		return BOOL_FALSE; // ID must be unique

	context = faux_eloop_new_context(event_cb, data);
	assert(context);
	if (!context)
		return BOOL_FALSE;

	if (faux_sched_periodic(eloop->sched, time, ev_id, context,
		period, cycle_num) < 0) {
		faux_free(context);
		return BOOL_FALSE;
	}

	return BOOL_TRUE;
}


bool_t faux_eloop_add_sched_periodic_delayed(faux_eloop_t *eloop,
	int ev_id, faux_eloop_cb_f *event_cb, void *data,
	const struct timespec *period, unsigned int cycle_num)
{
	faux_eloop_context_t *context = NULL;

	assert(eloop);
	if (!eloop)
		return BOOL_FALSE;

	if (faux_sched_id_exist(eloop->sched, ev_id))
		return BOOL_FALSE; // ID must be unique

	context = faux_eloop_new_context(event_cb, data);
	assert(context);
	if (!context)
		return BOOL_FALSE;

	if (faux_sched_periodic_delayed(eloop->sched, ev_id, context,
		period, cycle_num) < 0) {
		faux_free(context);
		return BOOL_FALSE;
	}

	return BOOL_TRUE;
}


bool_t faux_eloop_del_sched(faux_eloop_t *eloop, int id)
{
	faux_eloop_context_t *context = NULL;

	assert(eloop);
	if (!eloop)
		return BOOL_FALSE;

	if (!faux_sched_get_by_id(eloop->sched, id, (void **)&context, NULL))
		return BOOL_FALSE;

	faux_sched_remove_by_id(eloop->sched, id);
	faux_free(context);

	return BOOL_TRUE;
}
