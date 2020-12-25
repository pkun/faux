/** @brief Mechanism to shedule events.
 *
 * It's an ordered list of events. Events are ordered by the time. The earlier
 * events are closer to list head. The events can be one-time ("once") and
 * periodic. Periodic events have period and number of cycles (can be infinite).
 * User can schedule events specifying absolute time of future event or interval
 * from now to the moment of event. Periodic events will be rescheduled
 * automatically using specified period.
 *
 * User can get interval from now to next event time. User can get upcoming
 * events one-by-one.
 *
 * Each scheduled event can has arbitrary ID and pointer to arbitrary data
 * linked to this event. The ID can be used for type of event for
 * example or something else. The linked data can be a service structure.
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
#include "faux/sched.h"


/** @brief Allocates new sched object.
 *
 * Before working with sched object it must be allocated and initialized.
 *
 * @return Allocated and initialized sched object or NULL on error.
 */
faux_sched_t *faux_sched_new(void)
{
	faux_sched_t *sched = NULL;

	sched = faux_zmalloc(sizeof(*sched));
	if (!sched)
		return NULL;

	// Init
	sched->list = faux_list_new(FAUX_LIST_SORTED, FAUX_LIST_NONUNIQUE,
		faux_ev_compare, NULL, faux_ev_free_forced);

	return sched;
}


/** @brief Frees the sched object.
 *
 * After using the sched object must be freed. Function frees object itself
 * and all events stored within sched object.
 */
void faux_sched_free(faux_sched_t *sched)
{
	if (!sched)
		return;

	faux_list_free(sched->list);
	faux_free(sched);
}


/** @brief Adds time event (faux_ev_t) to scheduling list.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] ev Allocated and initialized event object.
 * @return BOOL_TRUE - success, BOOL_FALSE on error.
 */
bool_t faux_sched_add(faux_sched_t *sched, faux_ev_t *ev)
{
	faux_list_node_t *node = NULL;

	assert(sched);
	assert(ev);
	if (!sched || !ev)
		return BOOL_FALSE;
	if (faux_ev_is_busy(ev))
		return BOOL_FALSE; // Don't add busy (already scheduled) event

	node = faux_list_add(sched->list, ev);
	if (!node) // Something went wrong
		return BOOL_FALSE;
	faux_ev_set_busy(ev, BOOL_TRUE);

	return BOOL_TRUE;
}


/** @brief Internal function to add constructed event to scheduling list.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] time Absolute time of future event.
 * @param [in] ev_id Event ID.
 * @param [in] data Pointer to arbitrary data linked to event.
 * @param [in] periodic Periodic flag.
 * @param [in] period Periodic interval.
 * @param [in] cycle_num Number of cycles (FAUX_SCHED_INFINITE for infinite).
 * @return Pointer to newly created faux_ev_t object or NULL on error.
 */
static faux_ev_t *_sched(faux_sched_t *sched, const struct timespec *time,
	int ev_id, void *data, faux_sched_periodic_e periodic,
	const struct timespec *period, unsigned int cycle_num)
{
	faux_ev_t *ev = NULL;

	ev = faux_ev_new(ev_id, data);
	assert(ev);
	if (!ev)
		return NULL;
	faux_ev_set_time(ev, time);
	if (FAUX_SCHED_PERIODIC == periodic)
		faux_ev_set_periodic(ev, period, cycle_num);

	if (!faux_sched_add(sched, ev)) { // Something went wrong
		faux_ev_free(ev);
		return NULL;
	}

	return ev;
}


/** @brief Adds non-periodic event to scheduling list using absolute time.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] time Absolute time of future event (FAUX_SCHED_NOW for now).
 * @param [in] ev_id Event ID.
 * @param [in] data Pointer to arbitrary data linked to event.
 * @return Pointer to newly created faux_ev_t object or NULL on error.
 */
faux_ev_t *faux_sched_once(
	faux_sched_t *sched, const struct timespec *time, int ev_id, void *data)
{
	return _sched(sched, time, ev_id, data,
		FAUX_SCHED_ONCE, NULL, 0);
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
 * @return Pointer to newly created faux_ev_t object or NULL on error.
 */
faux_ev_t *faux_sched_once_delayed(faux_sched_t *sched,
	const struct timespec *interval, int ev_id, void *data)
{
	struct timespec now = {};
	struct timespec plan = {};

	assert(sched);
	if (!sched)
		return NULL;

	if (!interval)
		return faux_sched_once(sched, FAUX_SCHED_NOW, ev_id, data);
	faux_timespec_now(&now);
	faux_timespec_sum(&plan, &now, interval);

	return faux_sched_once(sched, &plan, ev_id, data);
}


/** @brief Adds periodic event to sched list using absolute time for first one.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] time Absolute time of first event.
 * @param [in] ev_id Event ID.
 * @param [in] data Pointer to arbitrary data linked to event.
 * @param [in] period Period of periodic event.
 * @param [in] cycle_num Number of cycles.
 * @return Pointer to newly created faux_ev_t object or NULL on error.
 */
faux_ev_t *faux_sched_periodic(
	faux_sched_t *sched, const struct timespec *time, int ev_id, void *data,
	const struct timespec *period, unsigned int cycle_num)
{
	return _sched(sched, time, ev_id, data,
		FAUX_SCHED_PERIODIC, period, cycle_num);
}


/** @brief Adds periodic event to sched list using period for first one.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] ev_id Event ID.
 * @param [in] data Pointer to arbitrary data linked to event.
 * @param [in] period Period of periodic event.
 * @param [in] cycle_num Number of cycles.
 * @return Pointer to newly created faux_ev_t object or NULL on error.
 */
faux_ev_t *faux_sched_periodic_delayed(
	faux_sched_t *sched, int ev_id, void *data,
	const struct timespec *period, unsigned int cycle_num)
{
	struct timespec now = {};
	struct timespec plan = {};

	assert(sched);
	assert(period);
	if (!sched || !period)
		return NULL;

	faux_timespec_now(&now);
	faux_timespec_sum(&plan, &now, period);
	return faux_sched_periodic(sched, &plan, ev_id, data,
		period, cycle_num);
}


/** @brief Returns the interval from current time and next scheduled event.
 *
 * If event is in the past then return null interval.
 * If no events was scheduled then return BOOL_FALSE.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [out] interval Calculated interval.
 * @return BOOL_TRUE - success, BOOL_FALSE on error or there is no scheduled events.
 */
bool_t faux_sched_next_interval(const faux_sched_t *sched, struct timespec *interval)
{
	faux_ev_t *ev = NULL;
	faux_list_node_t *iter = NULL;

	assert(sched);
	assert(interval);
	if (!sched || !interval)
		return BOOL_FALSE;

	iter = faux_list_head(sched->list);
	if (!iter)
		return BOOL_FALSE;
	ev = (faux_ev_t *)faux_list_data(iter);

	if (!faux_ev_time_left(ev, interval))
		return BOOL_FALSE;

	return BOOL_TRUE;
}


/** @brief Remove all entries from the list.
 *
 * @param [in] sched Allocated and initialized sched object.
 */
void faux_sched_del_all(faux_sched_t *sched)
{
	assert(sched);
	if (!sched)
		return;

	faux_list_del_all(sched->list);
}


/** @brief Pop already coming events from list.
 *
 * Pop (get and remove from list) timestamp if it's in the past.
 * If the timestamp is in the future then do nothing.
 * The event object can be rescheduled in a case of periodic event or
 * removed from the scheduled list. Removed event must be freed by user.
 * User can inspect event object's busy flag to decide if freeing is needed.
 * If busy flag is BOOL_TRUE then event is rescheduled. If busy flag is
 * BOOL_FALSE then object is ready to be freed.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @return Event object or NULL on error or there is no already coming events.
 */
faux_ev_t *faux_sched_pop(faux_sched_t *sched)
{
	faux_list_node_t *iter = NULL;
	faux_ev_t *ev = NULL;

	assert(sched);
	if (!sched)
		return NULL;

	iter = faux_list_head(sched->list);
	if (!iter)
		return NULL;
	ev = (faux_ev_t *)faux_list_data(iter);
	if (!faux_timespec_before_now(faux_ev_time(ev)))
		return NULL; // No events for this time
	faux_list_takeaway(sched->list, iter); // Remove entry from list
	faux_ev_set_busy(ev, BOOL_FALSE);

	if (faux_ev_reschedule_period(ev))
		faux_sched_add(sched, ev);

	return ev;
}


/** @brief Deletes all events with specified value from list.
 *
 * Static function.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] value Pointer to key value.
 * @param [in] cmp_f Callback to compare key and entry.
 * @return Number of removed entries or < 0 on error.
 */
static ssize_t faux_sched_del_by_something(faux_sched_t *sched, void *value,
	faux_list_kcmp_fn cmp_f)
{
	faux_list_node_t *node = NULL;
	faux_list_node_t *saved = NULL;
	ssize_t nodes_deleted = 0;

	assert(sched);
	if (!sched)
		return -1;

	while ((node = faux_list_match_node(sched->list, cmp_f,
		value, &saved))) {
		faux_list_del(sched->list, node);
		nodes_deleted++;
	}

	return nodes_deleted;
}


/** @brief Delete event from list.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] ptr Pointer to event object.
 * @return Number of removed entries or < 0 on error.
 */
ssize_t faux_sched_del(faux_sched_t *sched, faux_ev_t *ev)
{
	return faux_sched_del_by_something(sched, ev, faux_ev_compare_ptr);
}


/** @brief Deletes all events with specified ID from list.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] id ID to remove.
 * @return Number of removed entries or < 0 on error.
 */
ssize_t faux_sched_del_by_id(faux_sched_t *sched, int id)
{
	return faux_sched_del_by_something(sched, &id, faux_ev_compare_id);
}


/** @brief Deletes all events with specified data pointer from list.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] data Data to search entries to remove.
 * @return Number of removed entries or < 0 on error.
 */
ssize_t faux_sched_del_by_data(faux_sched_t *sched, void *data)
{
	return faux_sched_del_by_something(sched, data, faux_ev_compare_data);
}


/** @brief Get scheduled event by specified value.
 *
 * Static function.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] value Value to search for.
 * @param [in] cmp_f Callback to compare key and entry.
 * @param [in,out] saved Iterator.
 * @return Event (faux_ev_t) pointer or NULL on error or not found.
 */
static faux_ev_t *faux_sched_get_by_something(faux_sched_t *sched, void *value,
	faux_list_kcmp_fn cmp_f, faux_list_node_t **saved)
{
	faux_list_node_t *node = NULL;
	faux_ev_t *ev = NULL;

	assert(sched);
	if (!sched)
		return NULL;

	node = faux_list_match_node(sched->list, cmp_f, value, saved);
	if (!node)
		return NULL;

	ev = (faux_ev_t *)faux_list_data(node);

	return ev;
}


/** @brief Get sched entries with specified event ID.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] ev_id Event ID to search for.
 * @param [in,out] saved Iterator.
 * @return Event (faux_ev_t) pointer or NULL on error or not found.
 */
faux_ev_t *faux_sched_get_by_id(faux_sched_t *sched, int ev_id,
	faux_list_node_t **saved)
{
	return faux_sched_get_by_something(sched, &ev_id,
		faux_ev_compare_id, saved);
}


/** @brief Get sched entries with specified user data pointer.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] data Pointer to user data to search for.
 * @param [in,out] saved Iterator.
 * @return Event (faux_ev_t) pointer or NULL on error or not found.
 */
faux_ev_t *faux_sched_get_by_data(faux_sched_t *sched, void *data,
	faux_list_node_t **saved)
{
	return faux_sched_get_by_something(sched, data,
		faux_ev_compare_data, saved);
}
