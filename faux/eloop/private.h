#include "faux/faux.h"
#include "faux/list.h"
#include "faux/net.h"
#include "faux/vec.h"
#include "faux/sched.h"


struct faux_eloop_s {
	bool_t working; // Is event loop active now. Can detect nested loop.
	faux_eloop_cb_f *default_event_cb; // Default callback function
	faux_sched_t *sched; // Service shed structure
	faux_list_t *fds; // List of registered file descriptors
	faux_pollfd_t *pollfds; // Service object for ppoll()
	faux_list_t *signals; // List of registered signals
	sigset_t sig_set; // Set of registered signals (1 for interested signal)
	sigset_t sig_mask; // Mask of registered signals (0 - interested) = not sig_set
#ifdef HAVE_SIGNALFD
	int signal_fd; // Handler for signalfd(). Valid when loop is active only
#endif
};


typedef struct faux_eloop_context_s {
	faux_eloop_cb_f *event_cb;
	void *user_data;
} faux_eloop_context_t;

typedef struct faux_eloop_fd_s {
	int fd;
	short events;
	faux_eloop_context_t context;
} faux_eloop_fd_t;

typedef struct faux_eloop_signal_s {
	int signo;
	struct sigaction oldact;
	bool_t set;
	faux_eloop_context_t context;
} faux_eloop_signal_t;
