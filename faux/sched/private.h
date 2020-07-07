#include "faux/faux.h"
#include "faux/list.h"
#include "faux/time.h"
#include "faux/sched.h"


struct faux_ev_s {
	struct timespec time; // Planned time of event
	struct timespec period; // Period for periodic event
	int cycles_num; // Number of cycles for periodic event
	faux_sched_periodic_t periodic; // Periodic flag
	int id; // Type of event
	void *data; // Arbitrary data linked to event
};

struct faux_sched_s {
	faux_list_t *list;
};


C_DECL_BEGIN

int faux_ev_compare(const void *first, const void *second);
int faux_ev_compare_id(const void *key, const void *list_item);
int faux_ev_compare_data(const void *key, const void *list_item);

faux_ev_t *faux_ev_new(const struct timespec *time, int ev_id, void *data);
void faux_ev_free(void *ptr);

int faux_ev_periodic(faux_ev_t *ev,
	const struct timespec *interval, int cycles_num);
int faux_ev_dec_cycles(faux_ev_t *ev, int *new_cycles_num);
int faux_ev_reschedule(faux_ev_t *ev, const struct timespec *new_time);
int faux_ev_reschedule_period(faux_ev_t *ev);
int faux_ev_time_left(faux_ev_t *ev, struct timespec *left);

int faux_ev_id(const faux_ev_t *ev);
void *faux_ev_data(const faux_ev_t *ev);
const struct timespec *faux_ev_time(const faux_ev_t *ev);
faux_sched_periodic_t faux_ev_is_periodic(faux_ev_t *ev);

C_DECL_END
