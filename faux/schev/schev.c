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


/** @brief Internal function to add event to scheduling list.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] time Absolute time of future event.
 * @param [in] Event ID.
 * @param [in] Pointer to arbitrary data linked to event.
 * @param [in] Periodic flag.
 * @param [in] Number of cycles (FAUX_SCHEV_CYCLES_INFINITE for infinite).
 * @param [in] Periodic interval.
 * @return 0 - success, < 0 on error.
 */
static int _schev_schedule(faux_schev_t *schev, const struct timespec *time,
	int ev_id, void *data, faux_schev_periodic_t periodic,
	const struct timespec *interval, int cycles_num)
{
	faux_ev_t *ev = NULL;
	faux_list_node_t *node = NULL;

	assert(schev);
	if (!schev)
		return -1;

	ev = faux_ev_new(time, ev_id, data);
	assert(ev);
	if (!ev)
		return -1;
	if (FAUX_SCHEV_PERIODIC == periodic)
		faux_ev_periodic(ev, interval, cycles_num);

	node = faux_list_add(schev->list, ev);
	if (!node) { // Something went wrong
		faux_ev_free(ev);
		return -1;
	}

	return 0;
}


/** @brief Adds non-periodic event to scheduling list using absolute time.
 *
 * @param [in] sched Allocated and initialized sched object.
 * @param [in] time Absolute time of future event.
 * @param [in] Event ID.
 * @param [in] Pointer to arbitrary data linked to event.
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
 * @param [in] Event ID.
 * @param [in] Pointer to arbitrary data linked to event.
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
