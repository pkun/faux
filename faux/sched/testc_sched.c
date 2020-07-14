#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "faux/time.h"
#include "faux/sched.h"

int testc_faux_sched(void)
{
	faux_sched_t *sched = NULL;
	long long int nsec = 500000000l;
	struct timespec pol_s = {}; // One half of second
	struct timespec now = {};
	struct timespec t = {};
	int id = 78;
	char *str = "test";
	int e_id = 0;
	void *e_str = NULL;
	struct timespec twait = {};

	faux_nsec_to_timespec(&pol_s, nsec);
	faux_timespec_now(&now);
	faux_timespec_sum(&t, &now, &pol_s);

	sched = faux_sched_new();
	if (!sched)
		return -1;

	// Wait and get event
	faux_sched_once(sched, &t, id, str);
	nanosleep(&pol_s, NULL); // wait
	if (faux_sched_pop(sched, &e_id, &e_str) < 0)
		return -1;
	if (e_id != id)
		return -1;
	if (e_str != str)
		return -1;

	// Don't wait so pop must return -1
	faux_timespec_sum(&t, &t, &pol_s);
	faux_sched_once(sched, &t, id, str);
	// Don't wait. Pop must return -1
	if (faux_sched_pop(sched, &e_id, &e_str) == 0)
		return -1;
	// Get next event interval. It must be greater than 0 and greater
	// than full interval (half of second)
	if (faux_sched_next_interval(sched, &twait) < 0)
		return -1;
	if (faux_timespec_cmp(&twait, &(struct timespec){0, 0}) <= 0)
		return -1;
	if (faux_timespec_cmp(&twait, &pol_s) >= 0)
		return -1;

	faux_sched_free(sched);

	return 0;
}
