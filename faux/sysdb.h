/** @file sysdb.h
 * @brief Public interface for faux system database (passwd, group etc) functions.
 */

#ifndef _faux_sysdb_h
#define _faux_sysdb_h

#include <stddef.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "faux/faux.h"

C_DECL_BEGIN

// Wrappers for ugly getpwnam_r()-like functions
struct passwd *faux_sysdb_getpwnam(const char *name);
struct passwd *faux_sysdb_getpwuid(uid_t uid);
struct group *faux_sysdb_getgrnam(const char *name);
struct group *faux_sysdb_getgrgid(gid_t gid);

C_DECL_END

#endif
