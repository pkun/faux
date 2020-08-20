/** @file vec.h
 * @brief Public interface for a variable length vector.
 */

#ifndef _faux_vec_h
#define _faux_vec_h

#include <stddef.h>

#include <faux/faux.h>

typedef struct faux_vec_s faux_vec_t;

typedef int (*faux_vec_kcmp_fn)(const void *key, const void *item);

C_DECL_BEGIN


C_DECL_END

#endif				/* _faux_vec_h */

