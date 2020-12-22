/** @file event.h
 * @brief Public interface for event schedule functions.
 */

#ifndef _faux_sched_h
#define _faux_sched_h

#include <faux/list.h>
#include <faux/faux.h>
#include <faux/time.h>

#define FAUX_SCHED_NOW NULL
#define FAUX_SCHED_INFINITE (unsigned int)(-1l)

typedef enum {
	FAUX_SCHED_PERIODIC = BOOL_TRUE,
	FAUX_SCHED_ONCE = BOOL_FALSE
	} faux_sched_periodic_e;

typedef struct faux_ev_s faux_ev_t;
typedef struct faux_sched_s faux_sched_t;
typedef faux_list_node_t faux_sched_node_t;


C_DECL_BEGIN

// Time event
faux_ev_t *faux_ev_new(const struct timespec *time,
	int ev_id, void *data, faux_list_free_fn free_data_cb);
void faux_ev_free(void *ptr);
bool_t faux_ev_periodic(faux_ev_t *ev,
	const struct timespec *interval, unsigned int cycle_num);
int faux_ev_dec_cycles(faux_ev_t *ev, unsigned int *new_cycle_num);
int faux_ev_reschedule(faux_ev_t *ev, const struct timespec *new_time);
int faux_ev_reschedule_period(faux_ev_t *ev);
int faux_ev_time_left(faux_ev_t *ev, struct timespec *left);
int faux_ev_id(const faux_ev_t *ev);
void *faux_ev_data(const faux_ev_t *ev);
const struct timespec *faux_ev_time(const faux_ev_t *ev);
faux_sched_periodic_e faux_ev_is_periodic(faux_ev_t *ev);

// Time event scheduler
faux_sched_t *faux_sched_new(void);
void faux_sched_free(faux_sched_t *sched);
int faux_sched_once(
	faux_sched_t *sched, const struct timespec *time, int ev_id, void *data);
int faux_sched_once_delayed(faux_sched_t *sched,
	const struct timespec *interval, int ev_id, void *data);
int faux_sched_periodic(
	faux_sched_t *sched, const struct timespec *time, int ev_id, void *data,
	const struct timespec *period, unsigned int cycle_num);
int faux_sched_periodic_delayed(
	faux_sched_t *sched, int ev_id, void *data,
	const struct timespec *period, unsigned int cycle_num);
int faux_sched_next_interval(faux_sched_t *sched, struct timespec *interval);
void faux_sched_empty(faux_sched_t *sched);
int faux_sched_pop(faux_sched_t *sched, int *ev_id, void **data);
int faux_sched_remove_by_id(faux_sched_t *sched, int id);
int faux_sched_remove_by_data(faux_sched_t *sched, void *data);
const struct timespec *faux_sched_time_by_data(faux_sched_t *sched, void *data);
bool_t faux_sched_id_exist(faux_sched_t *sched, int id);
bool_t faux_sched_get_by_id(faux_sched_t *sched, int ev_id, void **data,
	faux_list_node_t **saved);

C_DECL_END

#endif /* _faux_sched_h */
