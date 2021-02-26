/** @file error.c
 * @brief Functions for working with errors.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "private.h"
#include "faux/faux.h"
#include "faux/str.h"
#include "faux/error.h"


/** @brief Allocates new error object.
 *
 * Before working with error object it must be allocated and initialized.
 *
 * @return Allocated and initialized error object or NULL on error.
 */
faux_error_t *faux_error_new(void)
{
	faux_error_t *error = NULL;

	error = faux_zmalloc(sizeof(*error));
	if (!error)
		return NULL;

	// Init
	error->list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, (void (*)(void *))faux_str_free);

	return error;
}


/** @brief Frees the error object.
 *
 * After using the error object must be freed.
 *
 * @param [in] error Allocated and initialized error object.
 */
void faux_error_free(faux_error_t *error)
{
	if (!error)
		return;

	faux_list_free(error->list);
	faux_free(error);
}


/** @brief Reset error object to empty state.
 *
 * After using the error object must be freed.
 * @param [in] error Allocated and initialized error object.
 */
void faux_error_reset(faux_error_t *error)
{
	if (!error)
		return;

	faux_list_del_all(error->list);
}


/** @brief Gets current error stack length.
 *
 * @param [in] error Allocated and initialized error object.
 * @return BOOL_TRUE if object contains errors or BOOL_FALSE.
 */
ssize_t faux_error_len(const faux_error_t *error)
{
	if (!error)
		return -1;

	return faux_list_len(error->list);
}


/** @brief Current status of error object.
 *
 * If error list contains any entries then function returns BOOL_TRUE else
 * BOOL_FALSE.
 *
 * @param [in] error Allocated and initialized error object.
 * @return BOOL_TRUE if object contains errors or BOOL_FALSE.
 */
bool_t faux_error(const faux_error_t *error)
{
	if (faux_error_len(error) > 0)
		return BOOL_TRUE;

	return BOOL_FALSE;
}


/** @brief Adds error message to message stack.
 *
 * @param [in] error Allocated and initialized error object.
 * @return success - BOOL_TRUE, fail - BOOL_FALSE.
 */
bool_t faux_error_add(faux_error_t *error, const char *str)
{
	char *tmp = NULL;

	// If error == NULL it's not bug
	if (!error)
		return BOOL_FALSE;
	if (!str)
		return BOOL_FALSE;

	tmp = faux_str_dup(str);
	if (!tmp)
		return BOOL_FALSE;
	if (!faux_list_add(error->list, tmp)) {
		faux_str_free(tmp);
		return BOOL_FALSE;
	}

	return BOOL_TRUE;
}


/** @brief Initializes iterator to iterate through the entire error object.
 *
 * Before iterating with the faux_error_each() function the iterator must be
 * initialized. This function do it.
 *
 * @param [in] error Allocated and initialized error object.
 * @return Initialized iterator.
 * @sa faux_error_each()
 */
faux_error_node_t *faux_error_iter(const faux_error_t *error)
{
	assert(error);
	if (!error)
		return NULL;

	return (faux_error_node_t *)faux_list_head(error->list);
}


/** @brief Initializes iterator to iterate through error object in reverse order.
 *
 * Before iterating with the faux_error_eachr() function the iterator must be
 * initialized. This function do it.
 *
 * @param [in] error Allocated and initialized error object.
 * @return Initialized iterator.
 * @sa faux_error_each()
 */
faux_error_node_t *faux_error_iterr(const faux_error_t *error)
{
	assert(error);
	if (!error)
		return NULL;

	return (faux_error_node_t *)faux_list_tail(error->list);
}


/** @brief Iterate entire error object.
 *
 * Before iteration the iterator must be initialized by faux_error_iter()
 * function. Doesn't use faux_error_each() with uninitialized iterator.
 *
 * On each call function returns error string and modifies iterator.
 * Stop iteration when function returns NULL.
 *
 * @param [in,out] iter Iterator.
 * @return String with error description.
 * @sa faux_error_iter()
 */
const char *faux_error_each(faux_error_node_t **iter)
{
	return (const char *)faux_list_each((faux_list_node_t **)iter);
}


/** @brief Iterate entire error object in reverse order.
 *
 * Before iteration the iterator must be initialized by faux_error_iterr()
 * function. Doesn't use faux_error_each() with uninitialized iterator.
 *
 * On each call function returns error string and modifies iterator.
 * Stop iteration when function returns NULL.
 *
 * @param [in,out] iter Iterator.
 * @return String with error description.
 * @sa faux_error_iter()
 */
const char *faux_error_eachr(faux_error_node_t **iter)
{
	return (const char *)faux_list_eachr((faux_list_node_t **)iter);
}


/** @brief Print error stack.
 *
 * @param [in] error Allocated and initialized error object.
 * @param [in] handle File handler to write to.
 * @param [in] reverse Print errors in reverse order.
 * @param [in] hierarchy Print errors using hierarchy view (or peer view).
 * @return BOOL_TRUE - success, BOOL_FALSE - fail.
 */
static bool_t faux_error_show(const faux_error_t *error, FILE *handle,
	bool_t reverse, bool_t hierarchy)
{
	faux_error_node_t *iter = NULL;
	const char *str = NULL;
	int level = 0;

	if (!error)
		return BOOL_FALSE;

	iter = reverse ? faux_error_iterr(error) : faux_error_iter(error);
	while ((str = (reverse ? faux_error_eachr(&iter) : faux_error_each(&iter)))) {
		if ((hierarchy) && (level > 0))
			fprintf(handle, "%*c", level, ' ');
		fprintf(handle, "%s\n", str);
		level ++;
	}

	return BOOL_TRUE;
}


/** @brief Print error stack.
 *
 * @param [in] error Allocated and initialized error object.
 * @param [in] handle File handler to write to.
 * @return BOOL_TRUE - success, BOOL_FALSE - fail.
 */
bool_t faux_error_fprint(const faux_error_t *error, FILE *handle)
{
	return faux_error_show(error, handle, BOOL_FALSE, BOOL_FALSE);
}


/** @brief Print error stack.
 *
 * @param [in] error Allocated and initialized error object.
 * @return BOOL_TRUE - success, BOOL_FALSE - fail.
 */
bool_t faux_error_print(const faux_error_t *error)
{
	return faux_error_fprint(error, stderr);
}
