/** @file eloop.h
 * @brief Public interface for Event Loop.
 */

#ifndef _faux_eloop_h
#define _faux_eloop_h

#include <faux/faux.h>
#include <faux/sched.h>

typedef struct faux_eloop_s faux_eloop_t;

typedef enum {
	FAUX_ELOOP_NULL = 0,
	FAUX_ELOOP_SIGNAL = 1,
	FAUX_ELOOP_SCHED = 2,
	FAUX_ELOOP_FD = 3
} faux_eloop_type_e;

typedef struct {
	int ev_id;
	faux_ev_t *ev;
} faux_eloop_info_sched_t;

typedef struct {
	int fd;
	short revents;
} faux_eloop_info_fd_t;

typedef struct {
	int signo;
} faux_eloop_info_signal_t;

// Callback function prototype
typedef bool_t (*faux_eloop_cb_f)(faux_eloop_t *eloop, faux_eloop_type_e type,
	void *associated_data, void *user_data);


C_DECL_BEGIN

faux_eloop_t *faux_eloop_new(faux_eloop_cb_f default_event_cb);
void faux_eloop_free(faux_eloop_t *eloop);
bool_t faux_eloop_loop(faux_eloop_t *eloop);

bool_t faux_eloop_add_fd(faux_eloop_t *eloop, int fd, short events,
	faux_eloop_cb_f event_cb, void *user_data);
bool_t faux_eloop_del_fd(faux_eloop_t *eloop, int fd);

bool_t faux_eloop_add_signal(faux_eloop_t *eloop, int signo,
	faux_eloop_cb_f event_cb, void *user_data);
bool_t faux_eloop_del_signal(faux_eloop_t *eloop, int signo);

faux_ev_t *faux_eloop_add_sched_once(faux_eloop_t *eloop, const struct timespec *time,
	int ev_id, faux_eloop_cb_f event_cb, void *data);
faux_ev_t *faux_eloop_add_sched_once_delayed(faux_eloop_t *eloop, const struct timespec *interval,
	int ev_id, faux_eloop_cb_f event_cb, void *data);
faux_ev_t *faux_eloop_add_sched_periodic(faux_eloop_t *eloop, const struct timespec *time,
	int ev_id, faux_eloop_cb_f event_cb, void *data,
	const struct timespec *period, unsigned int cycle_num);
faux_ev_t *faux_eloop_add_sched_periodic_delayed(faux_eloop_t *eloop,
	int ev_id, faux_eloop_cb_f event_cb, void *data,
	const struct timespec *period, unsigned int cycle_num);
ssize_t faux_eloop_del_sched(faux_eloop_t *eloop, faux_ev_t *ev);
ssize_t faux_eloop_del_sched_by_id(faux_eloop_t *eloop, int ev_id);
bool_t faux_eloop_include_fd_event(faux_eloop_t *eloop, int fd, short event);
bool_t faux_eloop_exclude_fd_event(faux_eloop_t *eloop, int fd, short event);

C_DECL_END

#endif
