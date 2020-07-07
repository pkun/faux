/** @file event.h
 * @brief Public interface for event schedule functions.
 */

#ifndef _faux_sched_h
#define _faux_sched_h

#include <faux/faux.h>
#include <faux/time.h>

#define FAUX_SCHED_NOW NULL

typedef enum {
	FAUX_SCHED_PERIODIC = BOOL_TRUE,
	FAUX_SCHED_ONCE = BOOL_FALSE
	} faux_sched_periodic_t;

typedef struct faux_ev_s faux_ev_t;
typedef struct faux_sched_s faux_sched_t;
typedef faux_list_node_t faux_sched_node_t;


C_DECL_BEGIN


C_DECL_END

#endif /* _faux_sched_h */
