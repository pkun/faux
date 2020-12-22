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
};


struct faux_sched_s {
	faux_list_t *list;
};


C_DECL_BEGIN

int faux_ev_compare(const void *first, const void *second);
int faux_ev_compare_id(const void *key, const void *list_item);
int faux_ev_compare_data(const void *key, const void *list_item);

C_DECL_END
