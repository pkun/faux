/** @brief Mechanism to shedule events.
 */

#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>

#include "private.h"
#include "faux/faux.h"
#include "faux/time.h"
#include "faux/list.h"
#include "faux/schev.h"


/** @brief Allocates new schev (SCHedule EVent) object.
 *
 * Before working with schev object it must be allocated and initialized.
 *
 * @return Allocated and initialized schev object or NULL on error.
 */
faux_schev_t *faux_schev_new(void)
{
	faux_schev_t *schev = NULL;

	schev = faux_zmalloc(sizeof(*schev));
	if (!schev)
		return NULL;

	// Init
	schev->list = faux_list_new(FAUX_LIST_SORTED, FAUX_LIST_NONUNIQUE,
		faux_ev_compare, NULL, faux_ev_free);

	return schev;
}


/** @brief Frees the schev object.
 *
 * After using the schev object must be freed. Function frees object itself
 * and all events stored within schev object.
 */
void faux_schev_free(faux_schev_t *schev)
{
	assert(schev);
	if (!schev)
		return;

	faux_list_free(schev->list);
	faux_free(schev);
}


static int _schev_schedule_ev(faux_schev_t *schev, faux_ev_t *ev)
{
	faux_list_node_t *node = NULL;

	assert(schev);
	assert(ev);
	if (!schev || !ev)
		return -1;

	node = faux_list_add(schev->list, ev);
	if (!node) // Something went wrong
		return -1;

	return 0;
}

/** @brief Internal function to add event to scheduling list.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] time Absolute time of future event.
 * @param [in] ev_id Event ID.
 * @param [in] data Pointer to arbitrary data linked to event.
 * @param [in] periodic Periodic flag.
 * @param [in] period Periodic interval.
 * @param [in] cycles_num Number of cycles (FAUX_SCHEV_CYCLES_INFINITE for infinite).
 * @return 0 - success, < 0 on error.
 */
static int _schev_schedule(faux_schev_t *schev, const struct timespec *time,
	int ev_id, void *data, faux_schev_periodic_t periodic,
	const struct timespec *period, int cycles_num)
{
	faux_ev_t *ev = NULL;

	ev = faux_ev_new(time, ev_id, data);
	assert(ev);
	if (!ev)
		return -1;
	if (FAUX_SCHEV_PERIODIC == periodic)
		faux_ev_periodic(ev, period, cycles_num);

	if (_schev_schedule_ev(schev, ev) < 0) { // Something went wrong
		faux_ev_free(ev);
		return -1;
	}

	return 0;
}


/** @brief Adds non-periodic event to scheduling list using absolute time.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] time Absolute time of future event.
 * @param [in] ev_id Event ID.
 * @param [in] data Pointer to arbitrary data linked to event.
 * @return 0 - success, < 0 on error.
 */
int faux_schev_schedule(
	faux_schev_t *schev, const struct timespec *time, int ev_id, void *data)
{
	return _schev_schedule(schev, time, ev_id, data,
		FAUX_SCHEV_ONCE, NULL, 0);
}


/** @brief Adds event to scheduling list using interval.
 *
 * Add interval to the list. The absolute time is calculated by
 * adding specified interval to the current absolute time.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] interval Interval (NULL means "now").
 * @param [in] ev_id Event ID.
 * @param [in] data Pointer to arbitrary data linked to event.
 * @return 0 - success, < 0 on error.
 */
int faux_schev_schedule_interval(faux_schev_t *schev,
	const struct timespec *interval, int ev_id, void *data)
{
	struct timespec t = {};
	struct timespec plan = {};

	assert(schev);
	if (!schev)
		return -1;

	if (!interval)
		return faux_schev_schedule(schev, FAUX_SCHEV_NOW, ev_id, data);
	clock_gettime(FAUX_SCHEV_CLOCK_SOURCE, &t);
	faux_timespec_sum(&plan, &t, interval);

	return faux_schev_schedule(schev, &plan, ev_id, data);
}


/** @brief Adds periodic event to sched list using absolute time for first one.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] time Absolute time of first event.
 * @param [in] ev_id Event ID.
 * @param [in] data Pointer to arbitrary data linked to event.
 * @param [in] period Period of periodic event.
 * @param [in] cycle_num Number of cycles.
 * @return 0 - success, < 0 on error.
 */
int faux_schev_periodic(
	faux_schev_t *schev, const struct timespec *time, int ev_id, void *data,
	const struct timespec *period, int cycle_num)
{
	return _schev_schedule(schev, time, ev_id, data,
		FAUX_SCHEV_ONCE, period, cycle_num);
}


/** @brief Adds periodic event to sched list using period for first one.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] ev_id Event ID.
 * @param [in] data Pointer to arbitrary data linked to event.
 * @param [in] period Period of periodic event.
 * @param [in] cycle_num Number of cycles.
 * @return 0 - success, < 0 on error.
 */
int faux_schev_periodic_delayed(
	faux_schev_t *schev, int ev_id, void *data,
	const struct timespec *period, int cycle_num)
{
	struct timespec t = {};
	struct timespec plan = {};

	assert(schev);
	assert(period);
	if (!schev || !period)
		return -1;

	clock_gettime(FAUX_SCHEV_CLOCK_SOURCE, &t);
	faux_timespec_sum(&plan, &t, period);
	return faux_schev_periodic(schev, &plan, ev_id, data,
		period, cycle_num);
}


/** @brief Returns the interval from current time and next scheduled event.
 *
 * If event is in the past then return null interval.
 * If no events was scheduled then return -1.
 */
int faux_schev_next_interval(faux_schev_t *schev, struct timespec *interval)
{
	faux_ev_t *ev = NULL;
	faux_list_node_t *iter = NULL;

	assert(schev);
	assert(interval);
	if (!schev || !interval)
		return -1;

	iter = faux_list_head(schev->list);
	if (!iter)
		return -1;
	ev = (faux_ev_t *)faux_list_data(iter);

	return faux_ev_time_left(ev, interval);
}

/** @brief Remove all entries from the list.
 *
 *
 */
void faux_schev_empty(faux_schev_t *schev)
{
	assert(schev);
	if (!schev)
		return;

	faux_list_empty(schev->list);
}

/** @brief Pop already coming events from list.
 *
 * Pop (get and remove from list) timestamp if it's in the past.
 * If the timestamp is in the future then do nothing.
 */
int faux_schev_pop(faux_schev_t *schev, int *ev_id, void **data)
{
	struct timespec now = {};
	faux_list_node_t *iter = NULL;
	faux_ev_t *ev = NULL;

	assert(schev);
	if (!schev)
		return -1;

	iter = faux_list_head(schev->list);
	if (!iter)
		return -1;
	ev = (faux_ev_t *)faux_list_data(iter);
	clock_gettime(FAUX_SCHEV_CLOCK_SOURCE, &now);
	if (faux_timespec_cmp(faux_ev_time(ev), &now) > 0)
		return -1; // No events for this time
	faux_list_takeaway(schev->list, iter); // Remove entry from list

	if (ev_id)
		*ev_id = faux_ev_id(ev);
	if (data)
		*data = faux_ev_data(ev);

	if (faux_ev_reschedule_interval(ev) < 0) {
		faux_ev_free(ev);
	} else {
		_schev_schedule_ev(schev, ev);
	}

	return 0;
}

#if 0
/* Remove all timestamps with specified ID from the list. */
void remove_ev(lub_list_t *list, int id)
{
	lub_list_node_t *iter = lub_list__get_head(list);
	if (!iter)
		return;
	while (iter) {
		lub_list_node_t *node = iter;
		schev_t *tmp = (schev_t *)lub_list_node__get_data(node);
		iter = lub_list_iterator_next(node);
		if (tmp->id == id) {
			lub_list_del(list, node);
			lub_list_node_free(node);
			free(tmp);
		}
	}
}

#endif
