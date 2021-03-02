/** @file error.h
 * @brief Public interface to work with advanced error class.
 */

#ifndef _faux_error_h
#define _faux_error_h

#include <stdio.h>

#include <faux/faux.h>
#include <faux/list.h>

typedef struct faux_error_s faux_error_t;
typedef faux_list_node_t faux_error_node_t;

C_DECL_BEGIN

faux_error_t *faux_error_new(void);
void faux_error_free(faux_error_t *error);
void faux_error_reset(faux_error_t *error);
ssize_t faux_error_len(const faux_error_t *error);
bool_t faux_error(const faux_error_t *error);
bool_t faux_error_add(faux_error_t *error, const char *str);
bool_t faux_error_sprintf(faux_error_t *error, const char *fmt, ...);

faux_error_node_t *faux_error_iter(const faux_error_t *error);
faux_error_node_t *faux_error_iterr(const faux_error_t *error);
const char *faux_error_each(faux_error_node_t **iter);
const char *faux_error_eachr(faux_error_node_t **iter);
bool_t faux_error_fshow(const faux_error_t *error, FILE *handle);
bool_t faux_error_show(const faux_error_t *error);

C_DECL_END

#endif				/* _faux_error_h */
