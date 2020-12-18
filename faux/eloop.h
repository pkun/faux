/** @file eloop.h
 * @brief Public interface for Event Loop.
 */

#ifndef _faux_eloop_h
#define _faux_eloop_h

#include <faux/faux.h>

typedef struct faux_eloop_s faux_eloop_t;

typedef enum {
	FAUX_ELOOP_NULL = 0,
	FAUX_ELOOP_SIGNAL = 1,
	FAUX_ELOOP_SCHED = 2,
	FAUX_ELOOP_FD = 3
} faux_eloop_type_e;

typedef bool_t faux_eloop_cb_f(faux_eloop_t *eloop, faux_eloop_type_e type,
	void *associated_data, void *user_data);

typedef struct {
	int ev_id;
} faux_eloop_info_sched_t;

typedef struct {
	int fd;
	short revents;
} faux_eloop_info_fd_t;

typedef struct {
	int signo;
} faux_eloop_info_signal_t;


C_DECL_BEGIN

faux_eloop_t *faux_eloop_new(faux_eloop_cb_f *default_event_cb);
void faux_eloop_free(faux_eloop_t *eloop);
bool_t faux_eloop_loop(faux_eloop_t *eloop);
bool_t faux_eloop_add_fd(faux_eloop_t *eloop, int fd, short events,
	faux_eloop_cb_f *event_cb, void *user_data);
bool_t faux_eloop_del_fd(faux_eloop_t *eloop, int fd);
bool_t faux_eloop_add_signal(faux_eloop_t *eloop, int signo,
	faux_eloop_cb_f *event_cb, void *user_data);
bool_t faux_eloop_del_signal(faux_eloop_t *eloop, int signo);

C_DECL_END

#endif
