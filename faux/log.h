/** @file log.h
 * @brief Public interface for faux log functions.
 */

#ifndef _faux_log_h
#define _faux_log_h

#include <syslog.h>

#include "faux/faux.h"

C_DECL_BEGIN

int faux_log_facility_id(const char *str, int *facility);
const char *faux_log_facility_str(int facility_id);

C_DECL_END

#endif
