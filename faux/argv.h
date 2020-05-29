/** @file argv.h
 * @brief Parse string to arguments.
 */

#ifndef _faux_argv_h
#define _faux_argv_h

#include "faux/faux.h"
#include "faux/list.h"

typedef struct faux_argv_s faux_argv_t;
typedef faux_list_node_t faux_argv_node_t;

C_DECL_BEGIN

faux_argv_t *faux_argv_new(void);
void faux_argv_free(faux_argv_t *fargv);
void faux_argv_quotes(faux_argv_t *fargv, const char *quotes);

faux_argv_node_t *faux_argv_iter(const faux_argv_t *fargv);
const char *faux_argv_each(faux_argv_node_t **iter);

ssize_t faux_argv_parse_str(faux_argv_t *fargv, const char *str);

C_DECL_END

#endif				/* _faux_argv_h */
