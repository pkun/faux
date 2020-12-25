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
faux_ev_t *faux_ev_new(int ev_id, void *data);
void faux_ev_free(void *ptr);
bool_t faux_ev_is_busy(const faux_ev_t *ev);
void faux_ev_set_free_data_cb(faux_ev_t *ev, faux_list_free_fn free_data_cb);
bool_t faux_ev_set_time(faux_ev_t *ev, const struct timespec *new_time);
const struct timespec *faux_ev_time(const faux_ev_t *ev);
bool_t faux_ev_set_periodic(faux_ev_t *ev,
	const struct timespec *interval, unsigned int cycle_num);
faux_sched_periodic_e faux_ev_is_periodic(const faux_ev_t *ev);
bool_t faux_ev_time_left(const faux_ev_t *ev, struct timespec *left);
int faux_ev_id(const faux_ev_t *ev);
void *faux_ev_data(const faux_ev_t *ev);

// Time event scheduler
faux_sched_t *faux_sched_new(void);
void faux_sched_free(faux_sched_t *sched);
bool_t faux_sched_add(faux_sched_t *sched, faux_ev_t *ev);
faux_ev_t *faux_sched_once(
	faux_sched_t *sched, const struct timespec *time, int ev_id, void *data);
faux_ev_t *faux_sched_once_delayed(faux_sched_t *sched,
	const struct timespec *interval, int ev_id, void *data);
faux_ev_t *faux_sched_periodic(
	faux_sched_t *sched, const struct timespec *time, int ev_id, void *data,
	const struct timespec *period, unsigned int cycle_num);
faux_ev_t *faux_sched_periodic_delayed(
	faux_sched_t *sched, int ev_id, void *data,
	const struct timespec *period, unsigned int cycle_num);
bool_t faux_sched_next_interval(const faux_sched_t *sched, struct timespec *interval);
void faux_sched_del_all(faux_sched_t *sched);
faux_ev_t *faux_sched_pop(faux_sched_t *sched);
ssize_t faux_sched_del(faux_sched_t *sched, faux_ev_t *ev);
ssize_t faux_sched_del_by_id(faux_sched_t *sched, int id);
ssize_t faux_sched_del_by_data(faux_sched_t *sched, void *data);
faux_ev_t *faux_sched_get_by_id(faux_sched_t *sched, int ev_id,
	faux_list_node_t **saved);
faux_ev_t *faux_sched_get_by_data(faux_sched_t *sched, void *data,
	faux_list_node_t **saved);

C_DECL_END

#endif /* _faux_sched_h */
