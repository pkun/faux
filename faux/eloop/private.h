#include "faux/faux.h"
#include "faux/list.h"
#include "faux/net.h"
#include "faux/vec.h"
#include "faux/sched.h"


struct faux_eloop_s {
	faux_eloop_cb_f *default_event_cb;
	faux_list_t *scheds;
	faux_sched_t *faux_sched;
	faux_list_t *fds;
	faux_pollfd_t *pollfds;
	faux_list_t *signals;
	sigset_t sig_set;
};


typedef struct faux_eloop_context_s {
	faux_eloop_cb_f *event_cb;
	void *user_data;
} faux_eloop_context_t;

typedef struct faux_eloop_shed_s {
	int ev_id;
	faux_eloop_context_t context;
} faux_eloop_sched_t;

typedef struct faux_eloop_fd_s {
	int fd;
	short events;
	faux_eloop_context_t context;
} faux_eloop_fd_t;

typedef struct faux_eloop_signal_s {
	int signo;
	faux_eloop_context_t context;
} faux_eloop_signal_t;
