/** @file ev.c
 * Single event for scheduling.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "private.h"
#include "faux/str.h"
#include "faux/sched.h"

int faux_ev_compare(const void *first, const void *second)
{
	const faux_ev_t *f = (const faux_ev_t *)first;
	const faux_ev_t *s = (const faux_ev_t *)second;

	return faux_timespec_cmp(&f->time, &s->time);
}


int faux_ev_compare_id(const void *key, const void *list_item)
{
	int *f = (int *)key;
	const faux_ev_t *s = (const faux_ev_t *)list_item;

	return ((*f == s->id) ? 0 : 1);
}


int faux_ev_compare_data(const void *key, const void *list_item)
{
	void *f = (void *)key;
	const faux_ev_t *s = (const faux_ev_t *)list_item;

	return ((f == s->data) ? 0 : 1);
}


faux_ev_t *faux_ev_new(const struct timespec *time, int ev_id, void *data)
{
	faux_ev_t *ev = NULL;

	ev = faux_zmalloc(sizeof(*ev));
	assert(ev);
	if (!ev)
		return NULL;

	// Initialize
	ev->id = ev_id;
	ev->data = data;
	ev->periodic = FAUX_SCHED_ONCE; // Not periodic by default
	ev->cycles_num = 0;
	faux_nsec_to_timespec(&ev->interval, 0l);
	faux_ev_reschedule(ev, time);

	return ev;
}


void faux_ev_free(void *ptr)
{
	faux_ev_t *ev = (faux_ev_t *)ptr;

	if (!ev)
		return;
	faux_free(ev);
}


int faux_ev_periodic(faux_ev_t *ev,
	const struct timespec *interval, int cycles_num)
{
	assert(ev);
	assert(interval);
	// When cycles_num == 0 then periodic has no meaning
	if (!ev || !interval || cycles_num == 0)
		return -1;

	ev->periodic = FAUX_SCHED_PERIODIC;
	ev->cycles_num = cycles_num;
	ev->interval = *interval;

	return 0;
}


faux_sched_periodic_t faux_ev_is_periodic(faux_ev_t *ev)
{
	assert(ev);
	if (!ev)
		return FAUX_SCHED_ONCE;

	return ev->periodic;
}


int faux_ev_dec_cycles(faux_ev_t *ev, int *new_cycles_num)
{
	assert(ev);
	if (!ev)
		return -1;
	if (ev->periodic != FAUX_SCHED_PERIODIC)
		return -1; // Non-periodic event
	if ((ev->cycles_num != FAUX_SCHED_CYCLES_INFINITE) &&
		(ev->cycles_num > 0))
		ev->cycles_num--;

	if (new_cycles_num)
		*new_cycles_num = ev->cycles_num;

	return 0;
}

/**
 *
 * Note: faux_ev_new() use it. Be carefull.
 */
int faux_ev_reschedule(faux_ev_t *ev, const struct timespec *new_time)
{
	assert(ev);
	if (!ev)
		return -1;

	if (new_time) {
		ev->time = *new_time;
	} else { // Time isn't given so use "NOW"
		struct timespec t = {};
		clock_gettime(FAUX_SCHED_CLOCK_SOURCE, &t);
		ev->time = t;
	}

	return 0;
}


int faux_ev_reschedule_interval(faux_ev_t *ev)
{
	struct timespec new_time = {};

	assert(ev);
	if (!ev)
		return -1;
	if (ev->periodic != FAUX_SCHED_PERIODIC)
		return -1;
	if (0 == ev->cycles_num)
		return -1;

	faux_timespec_sum(&new_time, &ev->time, &ev->interval);
	faux_ev_reschedule(ev, &new_time);

	if (ev->cycles_num != FAUX_SCHED_CYCLES_INFINITE)
		faux_ev_dec_cycles(ev, NULL);

	return 0;
}


int faux_ev_time_left(faux_ev_t *ev, struct timespec *left)
{
	struct timespec now = {};

	assert(ev);
	assert(left);
	if (!ev || !left)
		return -1;

	clock_gettime(FAUX_SCHED_CLOCK_SOURCE, &now);
	if (faux_timespec_cmp(&now, &ev->time) > 0) { // Already happend
		faux_nsec_to_timespec(left, 0l);
		return 0;
	}
	faux_timespec_diff(left, &ev->time, &now);

	return 0;
}


int faux_ev_id(const faux_ev_t *ev)
{
	assert(ev);
	if (!ev)
		return -1;

	return ev->id;
}


void *faux_ev_data(const faux_ev_t *ev)
{
	assert(ev);
	if (!ev)
		return NULL;

	return ev->data;
}


const struct timespec *faux_ev_time(const faux_ev_t *ev)
{
	assert(ev);
	if (!ev)
		return NULL;

	return &ev->time;
}
