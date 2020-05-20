/** @file testc_helpers.h
 * @brief Testc helper functions.
 */

#ifndef _faux_testc_helpers_h
#define _faux_testc_helpers_h

#include <stddef.h>

#include "faux/faux.h"

C_DECL_BEGIN

ssize_t faux_testc_file_deploy(const char *fn, const char *str);

C_DECL_END

#endif				/* _faux_testc_helpers_h */
