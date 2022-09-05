/** @file argv.h
 * @brief Parse string to arguments.
 */

#ifndef _faux_argv_h
#define _faux_argv_h

#include <faux/faux.h>
#include <faux/list.h>

typedef struct faux_argv_s faux_argv_t;
typedef faux_list_node_t faux_argv_node_t;

C_DECL_BEGIN

faux_argv_t *faux_argv_new(void);
faux_argv_t *faux_argv_dup(const faux_argv_t *origin);
void faux_argv_free(faux_argv_t *fargv);
void faux_argv_set_quotes(faux_argv_t *fargv, const char *quotes);

ssize_t faux_argv_len(faux_argv_t *fargv);
faux_argv_node_t *faux_argv_iter(const faux_argv_t *fargv);
const char *faux_argv_each(faux_argv_node_t **iter);
const char *faux_argv_current(faux_argv_node_t *iter);
const char *faux_argv_index(const faux_argv_t *fargv, size_t index);

ssize_t faux_argv_parse(faux_argv_t *fargv, const char *str);
bool_t faux_argv_add(faux_argv_t *fargv, const char *arg);

bool_t faux_argv_is_continuable(const faux_argv_t *fargv);
void faux_argv_set_continuable(faux_argv_t *fargv, bool_t continuable);
void faux_argv_del_continuable(faux_argv_t *fargv);

bool_t faux_argv_is_last(faux_argv_node_t *iter);

char *faux_argv_line(const faux_argv_t *fargv);

C_DECL_END

#endif				/* _faux_argv_h */
