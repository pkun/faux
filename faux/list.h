/** @file list.h
 * @brief Public interface for a bidirectional list.
 */

#ifndef _faux_list_h
#define _faux_list_h

#include <stddef.h>

#include <faux/faux.h>

typedef enum {
	FAUX_LIST_SORTED = BOOL_TRUE,
	FAUX_LIST_UNSORTED = BOOL_FALSE
	} faux_list_sorted_e;

typedef enum {
	FAUX_LIST_UNIQUE = BOOL_TRUE,
	FAUX_LIST_NONUNIQUE = BOOL_FALSE
	} faux_list_unique_e;

typedef struct faux_list_node_s faux_list_node_t;
typedef struct faux_list_s faux_list_t;

typedef int (*faux_list_cmp_fn)(const void *new_item, const void *list_item);
typedef int (*faux_list_kcmp_fn)(const void *key, const void *list_item);
typedef void (*faux_list_free_fn)(void *list_item);

C_DECL_BEGIN

// list_node_t methods
faux_list_node_t *faux_list_prev_node(const faux_list_node_t *node);
faux_list_node_t *faux_list_next_node(const faux_list_node_t *node);
void *faux_list_data(const faux_list_node_t *node);
faux_list_node_t *faux_list_each_node(faux_list_node_t **iter);
faux_list_node_t *faux_list_eachr_node(faux_list_node_t **iter);
void *faux_list_each(faux_list_node_t **iter);
void *faux_list_eachr(faux_list_node_t **iter);

// list_t methods
faux_list_t *faux_list_new(faux_list_sorted_e sorted, faux_list_unique_e unique,
	faux_list_cmp_fn cmpFn, faux_list_kcmp_fn kcmpFn,
	faux_list_free_fn freeFn);
void faux_list_free(faux_list_t *list);

faux_list_node_t *faux_list_head(const faux_list_t *list);
faux_list_node_t *faux_list_tail(const faux_list_t *list);
size_t faux_list_len(const faux_list_t *list);
bool_t faux_list_is_empty(const faux_list_t *list);

faux_list_node_t *faux_list_add(faux_list_t *list, void *data);
faux_list_node_t *faux_list_add_find(faux_list_t *list, void *data);
void *faux_list_takeaway(faux_list_t *list, faux_list_node_t *node);
bool_t faux_list_del(faux_list_t *list, faux_list_node_t *node);
bool_t faux_list_kdel(faux_list_t *list, const void *userkey);
ssize_t faux_list_del_all(faux_list_t *list);

faux_list_node_t *faux_list_match_node(const faux_list_t *list,
	faux_list_kcmp_fn matchFn, const void *userkey,
	faux_list_node_t **saveptr);
faux_list_node_t *faux_list_kmatch_node(const faux_list_t *list,
	const void *userkey, faux_list_node_t **saveptr);
void *faux_list_match(const faux_list_t *list,
	faux_list_kcmp_fn matchFn, const void *userkey,
	faux_list_node_t **saveptr);
void *faux_list_kmatch(const faux_list_t *list,
	const void *userkey, faux_list_node_t **saveptr);
faux_list_node_t *faux_list_find_node(const faux_list_t *list,
	faux_list_kcmp_fn matchFn, const void *userkey);
faux_list_node_t *faux_list_kfind_node(const faux_list_t *list,
	const void *userkey);
void *faux_list_find(const faux_list_t *list,
	faux_list_kcmp_fn matchFn, const void *userkey);
void *faux_list_kfind(const faux_list_t *list,
	const void *userkey);

C_DECL_END

#endif				/* _faux_list_h */

