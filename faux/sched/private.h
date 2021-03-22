#include "faux/faux.h"
#include "faux/list.h"
#include "faux/time.h"
#include "faux/sched.h"


struct faux_ev_s {
	struct timespec time; // Planned time of event
	struct timespec period; // Period for periodic event
	unsigned int cycle_num; // Number of cycles for periodic event
	faux_sched_periodic_e periodic; // Periodic flag
	int id; // Type of event
	void *data; // Arbitrary data linked to event
	faux_list_free_fn free_data_cb; // Callback to free user data
	bool_t busy;
};


struct faux_sched_s {
	faux_list_t *list;
};


C_DECL_BEGIN

FAUX_HIDDEN int faux_ev_compare(const void *first, const void *second);
FAUX_HIDDEN int faux_ev_compare_id(const void *key, const void *list_item);
FAUX_HIDDEN int faux_ev_compare_data(const void *key, const void *list_item);
FAUX_HIDDEN int faux_ev_compare_ptr(const void *key, const void *list_item);

FAUX_HIDDEN void faux_ev_free_forced(void *ptr);
FAUX_HIDDEN void faux_ev_set_busy(faux_ev_t *ev, bool_t busy);
FAUX_HIDDEN bool_t faux_ev_dec_cycles(faux_ev_t *ev, unsigned int *new_cycle_num);
FAUX_HIDDEN bool_t faux_ev_reschedule(faux_ev_t *ev, const struct timespec *new_time);
FAUX_HIDDEN bool_t faux_ev_reschedule_period(faux_ev_t *ev);

C_DECL_END
