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

faux_vec_t *faux_vec_new(size_t item_size, faux_vec_kcmp_fn matchFn);
void faux_vec_free(faux_vec_t *faux_vec);
size_t faux_vec_len(const faux_vec_t *faux_vec);
size_t faux_vec_item_size(const faux_vec_t *faux_vec);
void *faux_vec_item(const faux_vec_t *faux_vec, unsigned int index);
void *faux_vec_data(const faux_vec_t *faux_vec);
void *faux_vec_add(faux_vec_t *faux_vec);
ssize_t faux_vec_del(faux_vec_t *faux_vec, unsigned int index);
int faux_vec_find_fn(const faux_vec_t *faux_vec, faux_vec_kcmp_fn matchFn,
	const void *userkey, unsigned int start_index);
int faux_vec_find(const faux_vec_t *faux_vec, const void *userkey,
	unsigned int start_index);

C_DECL_END

#endif				/* _faux_vec_h */

