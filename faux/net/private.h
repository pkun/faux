#include "faux/faux.h"
#include "faux/list.h"
#include "faux/net.h"
#include "faux/vec.h"

struct faux_net_s {
	int fd; // File (socket) descriptor
	int (*isbreak_func)(void);
	sigset_t sigmask;
	struct timespec send_timeout_val;
	struct timespec recv_timeout_val;
	struct timespec *send_timeout;
	struct timespec *recv_timeout;
};

struct faux_pollfd_s {
	faux_vec_t *vec;
};