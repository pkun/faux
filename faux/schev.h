/** @file event.h
 * @brief Public interface for event schedule functions.
 */

#ifndef _faux_schev_h
#define _faux_schev_h

#include <faux/faux.h>
#include <faux/time.h>

#define FAUX_SCHEV_NOW NULL

typedef enum {
	FAUX_SCHEV_PERIODIC = BOOL_TRUE,
	FAUX_SCHEV_ONCE = BOOL_FALSE
	} faux_schev_periodic_t;

typedef struct faux_ev_s faux_ev_t;
typedef struct faux_schev_s faux_schev_t;
typedef faux_list_node_t faux_schev_node_t;


C_DECL_BEGIN


C_DECL_END

#endif /* _faux_schev_h */
