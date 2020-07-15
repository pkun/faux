#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "faux/time.h"
#include "faux/sched.h"

int testc_faux_sched_once(void)
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

	// Schedule event
	faux_sched_once(sched, &t, id, str);
	// Don't wait so pop must return -1
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
	// Wait and get event
	nanosleep(&pol_s, NULL); // wait
	if (faux_sched_pop(sched, &e_id, &e_str) < 0)
		return -1;
	if (e_id != id)
		return -1;
	if (e_str != str)
		return -1;

	// Schedule event delayed
	faux_sched_once(sched, &pol_s, id, str);
	// Wait and get event
	nanosleep(&pol_s, NULL); // wait
	e_id = 0;
	e_str = NULL;
	if (faux_sched_pop(sched, &e_id, &e_str) < 0)
		return -1;
	if (e_id != id)
		return -1;
	if (e_str != str)
		return -1;

	faux_sched_free(sched);

	return 0;
}


int testc_faux_sched_periodic(void)
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

	faux_nsec_to_timespec(&pol_s, nsec);
	faux_timespec_now(&now);
	faux_timespec_sum(&t, &now, &pol_s);

	sched = faux_sched_new();
	if (!sched)
		return -1;

	// Schedule event
	faux_sched_periodic_delayed(sched, id, str, &pol_s, 2);
	// Don't wait so pop must return -1
	if (faux_sched_pop(sched, &e_id, &e_str) == 0) {
		printf("faux_shed_pop: Immediately event\n");
		return -1;
	}
	// Wait and get one event
	nanosleep(&pol_s, NULL); // wait
	if (faux_sched_pop(sched, &e_id, &e_str) < 0) {
		printf("faux_shed_pop: Can't get 1/2 event\n");
		return -1;
	}
	if (e_id != id)
		return -1;
	if (e_str != str)
		return -1;
	if (faux_sched_pop(sched, &e_id, &e_str) == 0) { // another event?
		printf("faux_shed_pop: Two events at once\n");
		return -1;
	}
	nanosleep(&pol_s, NULL); // wait next time
	if (faux_sched_pop(sched, &e_id, &e_str) < 0) {
		printf("faux_shed_pop: Can't get 2/2 event\n");
		return -1;
	}
	nanosleep(&pol_s, NULL); // wait third time
	if (faux_sched_pop(sched, &e_id, &e_str) == 0) { // no events any more
		printf("faux_shed_pop: The 3/2 event\n");
		return -1;
	}

	faux_sched_free(sched);

	return 0;
}


int testc_faux_sched_infinite(void)
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

	faux_nsec_to_timespec(&pol_s, nsec);
	faux_timespec_now(&now);
	faux_timespec_sum(&t, &now, &pol_s);

	sched = faux_sched_new();
	if (!sched)
		return -1;

	// Schedule event
	faux_sched_periodic_delayed(sched, id, str, &pol_s,
		FAUX_SCHED_INFINITE);
	// Don't wait so pop must return -1
	if (faux_sched_pop(sched, &e_id, &e_str) == 0) {
		printf("faux_shed_pop: Immediately event\n");
		return -1;
	}
	// Wait and get one event
	nanosleep(&pol_s, NULL); // wait
	if (faux_sched_pop(sched, &e_id, &e_str) < 0) {
		printf("faux_shed_pop: Can't get 1 event\n");
		return -1;
	}
	if (e_id != id)
		return -1;
	if (e_str != str)
		return -1;
	if (faux_sched_pop(sched, &e_id, &e_str) == 0) { // another event?
		printf("faux_shed_pop: Two events at once\n");
		return -1;
	}
	nanosleep(&pol_s, NULL); // wait next time
	if (faux_sched_pop(sched, &e_id, &e_str) < 0) {
		printf("faux_shed_pop: Can't get 2 event\n");
		return -1;
	}
	faux_sched_empty(sched); // Empty the list
	nanosleep(&pol_s, NULL); // wait third time
	if (faux_sched_pop(sched, &e_id, &e_str) == 0) {
		printf("faux_shed_pop: Event after empty operation\n");
		return -1;
	}

	faux_sched_free(sched);

	return 0;
}
