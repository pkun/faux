#include "faux/faux.h"
#include "faux/list.h"
#include "faux/net.h"

struct faux_net_s {
	int fd; // File (socket) descriptor
	int (*isbreak_func)(void);
	sigset_t sigmask;
	struct timespec send_timeout_val;
	struct timespec recv_timeout_val;
	struct timespec *send_timeout;
	struct timespec *recv_timeout;
};
