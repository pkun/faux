/** @file ev.c
 * Single event for scheduling.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "private.h"
#include "faux/str.h"
#include "faux/sched.h"


/** @brief Callback function to compare two events by time.
 *
 * It's used for ordering within schedule list.
 *
 * @param [in] first First event to compare.
 * @param [in] second Second event to compare.
 * @return
 * > 0 if first > second,
 * 0 - equal,
 * < 0 if first < second
 */
int faux_ev_compare(const void *first, const void *second)
{
	const faux_ev_t *f = (const faux_ev_t *)first;
	const faux_ev_t *s = (const faux_ev_t *)second;

	return faux_timespec_cmp(&(f->time), &(s->time));
}


/** @brief Callback function to compare key and list item by ID.
 *
 * It's used to search for specified ID within schedule list.
 *
 * @param [in] key Pointer to key value
 * @param [in] list_item Pointer to list item.
 * @return
 * > 0 if key > list_item,
 * 0 - equal,
 * < 0 if key < list_item
 */
int faux_ev_compare_id(const void *key, const void *list_item)
{
	int *f = (int *)key;
	const faux_ev_t *s = (const faux_ev_t *)list_item;

	return ((*f == s->id) ? 0 : 1);
}


/** @brief Callback function to compare key and list item by data pointer.
 *
 * It's used to search for specified data pointer within schedule list.
 *
 * @param [in] key Pointer to key value
 * @param [in] list_item Pointer to list item.
 * @return
 * > 0 if key > list_item,
 * 0 - equal,
 * < 0 if key < list_item
 */
int faux_ev_compare_data(const void *key, const void *list_item)
{
	void *f = (void *)key;
	const faux_ev_t *s = (const faux_ev_t *)list_item;

	return ((f == s->data) ? 0 : 1);
}


/** @brief Allocates and initialize ev object.
 *
 * @param [in] time Time of event.
 * @param [in] ev_id ID of event.
 * @param [in] data Pointer to arbitrary linked data.
 * @return Allocated and initialized ev object.
 */
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
	ev->cycle_num = 0;
	faux_nsec_to_timespec(&(ev->period), 0l);
	faux_ev_reschedule(ev, time);

	return ev;
}


/** @brief Frees ev object.
 *
 * @param [in] ptr Pointer to ev object.
 */
void faux_ev_free(void *ptr)
{
	faux_ev_t *ev = (faux_ev_t *)ptr;

	if (!ev)
		return;
	faux_free(ev);
}


/** @brief Makes event periodic.
 *
 * By default new events are not periodic.
 *
 * @param [in] ev Allocated and initialized ev object.
 * @param [in] period Period of periodic event.
 * @param [in] cycle_num Number of cycles. FAUX_SHED_INFINITE - infinite.
 * @return 0 - success, < 0 on error.
 */
int faux_ev_periodic(faux_ev_t *ev,
	const struct timespec *period, unsigned int cycle_num)
{
	assert(ev);
	assert(period);
	// When cycle_num == 0 then periodic has no meaning
	if (!ev || !period || cycle_num == 0)
		return -1;

	ev->periodic = FAUX_SCHED_PERIODIC;
	ev->cycle_num = cycle_num;
	ev->period = *period;

	return 0;
}


/** @brief Checks is event periodic.
 *
 * @param [in] ev Allocated and initialized ev object.
 * @return FAUX_SCHED_PERIODIC - periodic, FAUX_SCHED_ONCE - non-periodic.
 */
faux_sched_periodic_t faux_ev_is_periodic(faux_ev_t *ev)
{
	assert(ev);
	if (!ev)
		return FAUX_SCHED_ONCE;

	return ev->periodic;
}


/** @brief Decrements number of periodic cycles.
 *
 * On every completed cycle the internal cycles counter must be decremented.
 *
 * @param [in] ev Allocated and initialized ev object.
 * @param [out] new_cycle_num Returns new number of cycles. Can be NULL.
 * @return FAUX_SCHED_PERIODIC - periodic, FAUX_SCHED_ONCE - non-periodic.
 */
int faux_ev_dec_cycles(faux_ev_t *ev, unsigned int *new_cycle_num)
{
	assert(ev);
	if (!ev)
		return -1;
	if (!faux_ev_is_periodic(ev))
		return -1; // Non-periodic event
	if ((ev->cycle_num != FAUX_SCHED_INFINITE) &&
		(ev->cycle_num > 0))
		ev->cycle_num--;

	if (new_cycle_num)
		*new_cycle_num = ev->cycle_num;

	return 0;
}

/** Reschedules existent event to newly specified time.
 *
 * Note: faux_ev_new() use it. Be carefull.
 *
 * @param [in] ev Allocated and initialized ev object.
 * @param [in] new_time New time of event (FAUX_SCHED_NOW for now).
 * @return 0 - success, < 0 on error.
 */
int faux_ev_reschedule(faux_ev_t *ev, const struct timespec *new_time)
{
	assert(ev);
	if (!ev)
		return -1;

	if (new_time) {
		ev->time = *new_time;
	} else { // Time isn't given so use "NOW"
		faux_timespec_now(&(ev->time));
	}

	return 0;
}


/** Reschedules existent event using period.
 *
 * New scheduled time is calculated as "now" + "period".
 * Function decrements number of cycles. If number of cycles is
 * FAUX_SCHED_INFINITE then number of cycles will not be decremented.
 *
 * @param [in] ev Allocated and initialized ev object.
 * @return 0 - success, < 0 on error.
 */
int faux_ev_reschedule_period(faux_ev_t *ev)
{
	struct timespec new_time = {};

	assert(ev);
	if (!ev)
		return -1;
	if (!faux_ev_is_periodic(ev))
		return -1;
	if (ev->cycle_num <= 1)
		return -1; // We don't need to reschedule if last cycle left

	faux_timespec_sum(&new_time, &(ev->time), &(ev->period));
	faux_ev_reschedule(ev, &new_time);

	if (ev->cycle_num != FAUX_SCHED_INFINITE)
		faux_ev_dec_cycles(ev, NULL);

	return 0;
}


/** @brief Calculates time left from now to the event.
 *
 * @param [in] ev Allocated and initialized ev object.
 * @param [out] left Calculated time left.
 * @return 0 - success, < 0 on error.
 */
int faux_ev_time_left(faux_ev_t *ev, struct timespec *left)
{
	struct timespec now = {};

	assert(ev);
	assert(left);
	if (!ev || !left)
		return -1;

	faux_timespec_now(&now);
	if (faux_timespec_cmp(&now, &(ev->time)) > 0) { // Already happend
		faux_nsec_to_timespec(left, 0l);
		return 0;
	}
	faux_timespec_diff(left, &(ev->time), &now);

	return 0;
}


/** Returns ID of event object.
 *
 * @param [in] ev Allocated and initialized ev object.
 * @return Event's ID.
 */
int faux_ev_id(const faux_ev_t *ev)
{
	assert(ev);
	if (!ev)
		return -1;

	return ev->id;
}


/** Returns data pointer of event object.
 *
 * @param [in] ev Allocated and initialized ev object.
 * @return Data pointer.
 */
void *faux_ev_data(const faux_ev_t *ev)
{
	assert(ev);
	if (!ev)
		return NULL;

	return ev->data;
}


/** Returns time of event object.
 *
 * @param [in] ev Allocated and initialized ev object.
 * @return Pointer to static timespec.
 */
const struct timespec *faux_ev_time(const faux_ev_t *ev)
{
	assert(ev);
	if (!ev)
		return NULL;

	return &(ev->time);
}
