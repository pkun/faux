/** @file ev.c
 * Single event for scheduling.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "private.h"
#include "faux/str.h"
#include "faux/schev.h"

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
	if (time) {
		ev->time = *time;
	} else {
		struct timespec t = {};
		clock_gettime(FAUX_SCHEV_CLOCK_SOURCE, &t);
		ev->time = t;
	}
	ev->id = ev_id;
	ev->data = data;
	ev->periodic = FAUX_SCHEV_ONCE; // Not periodic by default
	ev->cycles_num = 0;
	faux_nsec_to_timespec(&ev->interval, 0l);

	return ev;
}


void faux_ev_free(void *ptr)
{
	faux_ev_t *ev = (faux_ev_t *)ptr;

	if (!ev)
		return;
	faux_free(ev);
}


int faux_ev_periodic(faux_ev_t *schev,
	struct timespec *interval, int cycles_num)
{
	assert(schev);
	assert(interval);
	// When cycles_num == 0 then periodic has no meaning
	if (!schev || !interval || cycles_num == 0)
		return -1;

	schev->periodic = FAUX_SCHEV_PERIODIC;
	schev->cycles_num = cycles_num;
	schev->interval = *interval;

	return 0;
}


faux_schev_periodic_t faux_ev_is_periodic(faux_ev_t *schev)
{
	assert(schev);
	if (!schev)
		return FAUX_SCHEV_ONCE;

	return schev->periodic;
}

int faux_ev_dec_cycles(faux_ev_t *schev, int *new_cycles_num)
{
	assert(schev);
	if (!schev)
		return -1;
	if (schev->periodic != FAUX_SCHEV_PERIODIC)
		return -1; // Non-periodic event
	if ((schev->cycles_num != FAUX_SCHEV_CYCLES_INFINITE) &&
		(schev->cycles_num > 0))
		schev->cycles_num--;

	if (new_cycles_num)
		*new_cycles_num = schev->cycles_num;

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
