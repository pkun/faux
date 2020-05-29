/** @file argv.c
 * @brief Functions to parse string to arguments.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "private.h"
#include "faux/faux.h"
#include "faux/str.h"
#include "faux/list.h"
#include "faux/argv.h"


/** @brief Allocates new argv object.
 *
 * Before working with argument list it must be allocated and initialized.
 *
 * @return Allocated and initialized argument list or NULL on error.
 */
faux_argv_t *faux_argv_new(void)
{
	faux_argv_t *fargv = NULL;

	fargv = faux_zmalloc(sizeof(*fargv));
	assert(fargv);
	if (!fargv)
		return NULL;

	// Init
	fargv->list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, (void (*)(void *))faux_str_free);
	fargv->quotes = NULL;

	return fargv;
}


/** @brief Frees the argv object object.
 *
 * After using the argv object must be freed. Function frees argv object.
 */
void faux_argv_free(faux_argv_t *fargv)
{
	assert(fargv);
	if (!fargv)
		return;

	faux_list_free(fargv->list);
	faux_str_free(fargv->quotes);
	faux_free(fargv);
}


/** @brief Initializes iterator to iterate through the entire argv object.
 *
 * Before iterating with the faux_argv_each() function the iterator must be
 * initialized. This function do it.
 *
 * @param [in] fargv Allocated and initialized argv object.
 * @return Initialized iterator.
 * @sa faux_argv_each()
 */
faux_argv_node_t *faux_argv_iter(const faux_argv_t *fargv)
{
	assert(fargv);
	if (!fargv)
		return NULL;

	return (faux_argv_node_t *)faux_list_head(fargv->list);
}


/** @brief Iterate entire argv object for arguments.
 *
 * Before iteration the iterator must be initialized by faux_argv_iter()
 * function. Doesn't use faux_argv_each() with uninitialized iterator.
 *
 * On each call function returns string (argument) and modifies iterator.
 * Stop iteration when function returns NULL.
 *
 * @param [in,out] iter Iterator.
 * @return String.
 * @sa faux_argv_iter()
 */
const char *faux_argv_each(faux_argv_node_t **iter)
{
	return (const char *)faux_list_each((faux_list_node_t **)iter);
}


void faux_argv_quotes(faux_argv_t *fargv, const char *quotes)
{
	assert(fargv);
	if (!fargv)
		return;

	faux_str_free(fargv->quotes);
	if (!quotes) {
		fargv->quotes = NULL; // No additional quotes
		return;
	}
	fargv->quotes = faux_str_dup(quotes);
}


ssize_t faux_argv_parse_str(faux_argv_t *fargv, const char *str)
{
	assert(fargv);
	if (!fargv)
		return -1;


	return 0;
}

