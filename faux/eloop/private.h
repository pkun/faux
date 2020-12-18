#include "faux/faux.h"
#include "faux/list.h"
#include "faux/net.h"
#include "faux/vec.h"
#include "faux/sched.h"


struct faux_eloop_s {
	bool_t working; // Is event loop active now. Can detect nested loop.
	faux_eloop_cb_f *default_event_cb; // Default callback function
	faux_list_t *scheds; // List of registered sched events
	faux_sched_t *faux_sched; // Service shed structure
	faux_list_t *fds; // List of registered file descriptors
	faux_pollfd_t *pollfds; // Service object for ppoll()
	faux_list_t *signals; // List of registered signals
	sigset_t sig_set; // Mask of registered signals
#ifdef HAVE_SIGNALFD
	int signal_fd; // Handler for signalfd(). Valid when loop is active only
#endif
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
