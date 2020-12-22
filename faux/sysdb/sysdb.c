/** @file sysdb.c
 * @brief Wrappers for system database functions like getpwnam(), getgrnam().
 */

// It must be here to include config.h before another headers
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <assert.h>

#include "faux/faux.h"
#include "faux/str.h"
#include "faux/sysdb.h"

#define DEFAULT_GETPW_R_SIZE_MAX 1024


/** @brief Wrapper for ugly getpwnam_r() function.
 *
 * Gets passwd structure by user name. Easy to use.
 *
 * @param [in] name User name.
 * @return Pointer to allocated passwd structure.
 * @warning The resulting pointer (return value) must be freed by faux_free().
 */
struct passwd *faux_sysdb_getpwnam(const char *name)
{
	long int size = 0;
	char *buf = NULL;
	struct passwd *pwbuf = NULL;
	struct passwd *pw = NULL;
	int res = 0;

#ifdef _SC_GETPW_R_SIZE_MAX
	if ((size = sysconf(_SC_GETPW_R_SIZE_MAX)) < 0)
		size = DEFAULT_GETPW_R_SIZE_MAX;
#else
	size = DEFAULT_GETPW_R_SIZE_MAX;
#endif
	pwbuf = faux_zmalloc(sizeof(*pwbuf) + size);
	if (!pwbuf)
		return NULL;
	buf = (char *)pwbuf + sizeof(*pwbuf);

	res = getpwnam_r(name, pwbuf, buf, size, &pw);
	if ((res != 0) || !pw) {
		faux_free(pwbuf);
		if (res != 0)
			errno = res;
		else
			errno = ENOENT;
		return NULL;
	}

	return pwbuf;
}


/** @brief Wrapper for ugly getpwuid_r() function.
 *
 * Gets passwd structure by UID. Easy to use.
 *
 * @param [in] uid UID.
 * @return Pointer to allocated passwd structure.
 * @warning The resulting pointer (return value) must be freed by faux_free().
 */
struct passwd *faux_sysdb_getpwuid(uid_t uid)
{
	long int size = 0;
	char *buf = NULL;
	struct passwd *pwbuf = NULL;
	struct passwd *pw = NULL;
	int res = 0;

#ifdef _SC_GETPW_R_SIZE_MAX
	if ((size = sysconf(_SC_GETPW_R_SIZE_MAX)) < 0)
		size = DEFAULT_GETPW_R_SIZE_MAX;
#else
	size = DEFAULT_GETPW_R_SIZE_MAX;
#endif
	pwbuf = faux_zmalloc(sizeof(*pwbuf) + size);
	if (!pwbuf)
		return NULL;
	buf = (char *)pwbuf + sizeof(*pwbuf);

	res = getpwuid_r(uid, pwbuf, buf, size, &pw);
	if (!pw) {
		faux_free(pwbuf);
		if (res != 0)
			errno = res;
		else
			errno = ENOENT;
		return NULL;
	}

	return pwbuf;
}


/** @brief Get UID by user name.
 *
 * @param [in] name User name.
 * @param [out] uid UID.
 * @return BOOL_TRUE - success, BOOL_FALSE on error.
 */
bool_t faux_sysdb_uid_by_name(const char *name, uid_t *uid)
{
	struct passwd *pw = NULL;

	assert(name);
	if (!name)
		return BOOL_FALSE;

	pw = faux_sysdb_getpwnam(name);
	if (!pw)
		return BOOL_FALSE; // Unknown user
	if (uid)
		*uid = pw->pw_uid;
	faux_free(pw);

	return BOOL_TRUE;
}


/** @brief Get user name by UID.
 *
 * @param [in] uid UID.
 * @return Allocated user name string, NULL on error.
 * @warning The resulting pointer (return value) must be freed by faux_str_free().
 */
char *faux_sysdb_name_by_uid(uid_t uid)
{
	struct passwd *pw = NULL;
	char *name = NULL;

	pw = faux_sysdb_getpwuid(uid);
	if (!pw)
		return NULL; // Unknown user
	name = faux_str_dup(pw->pw_name);
	faux_free(pw);

	return name;
}


/** @brief Wrapper for ugly getgrnam_r() function.
 *
 * Gets group structure by group name. Easy to use.
 *
 * @param [in] name Group name.
 * @return Pointer to allocated group structure.
 * @warning The resulting pointer (return value) must be freed by faux_free().
 */
struct group *faux_sysdb_getgrnam(const char *name)
{
	long int size;
	char *buf;
	struct group *grbuf;
	struct group *gr = NULL;
	int res = 0;

#ifdef _SC_GETGR_R_SIZE_MAX
	if ((size = sysconf(_SC_GETGR_R_SIZE_MAX)) < 0)
		size = DEFAULT_GETPW_R_SIZE_MAX;
#else
	size = DEFAULT_GETPW_R_SIZE_MAX;
#endif
	grbuf = faux_zmalloc(sizeof(*grbuf) + size);
	if (!grbuf)
		return NULL;
	buf = (char *)grbuf + sizeof(*grbuf);

	res = getgrnam_r(name, grbuf, buf, size, &gr);
	if (!gr) {
		faux_free(grbuf);
		if (res != 0)
			errno = res;
		else
			errno = ENOENT;
		return NULL;
	}

	return grbuf;
}


/** @brief Wrapper for ugly getgrgid_r() function.
 *
 * Gets group structure by GID. Easy to use.
 *
 * @param [in] gid GID.
 * @return Pointer to allocated group structure.
 * @warning The resulting pointer (return value) must be freed by faux_free().
 */
struct group *faux_sysdb_getgrgid(gid_t gid)
{
	long int size;
	char *buf;
	struct group *grbuf;
	struct group *gr = NULL;
	int res = 0;

#ifdef _SC_GETGR_R_SIZE_MAX
	if ((size = sysconf(_SC_GETGR_R_SIZE_MAX)) < 0)
		size = DEFAULT_GETPW_R_SIZE_MAX;
#else
	size = DEFAULT_GETPW_R_SIZE_MAX;
#endif
	grbuf = faux_zmalloc(sizeof(struct group) + size);
	if (!grbuf)
		return NULL;
	buf = (char *)grbuf + sizeof(struct group);

	res = getgrgid_r(gid, grbuf, buf, size, &gr);
	if (!gr) {
		faux_free(grbuf);
		if (res != 0)
			errno = res;
		else
			errno = ENOENT;
		return NULL;
	}

	return grbuf;
}


/** @brief Get GID by name.
 *
 * @param [in] name Group name.
 * @param [out] gid GID.
 * @return BOOL_TRUE - success, BOOL_FALSE on error.
 */
bool_t faux_sysdb_gid_by_name(const char *name, gid_t *gid)
{
	struct group *gr = NULL;

	assert(name);
	if (!name)
		return BOOL_FALSE;

	gr = faux_sysdb_getgrnam(name);
	if (!gr)
		return BOOL_FALSE; // Unknown group
	if (gid)
		*gid = gr->gr_gid;
	faux_free(gr);

	return BOOL_TRUE;
}


/** @brief Get group name by GID.
 *
 * @param [in] gid GID.
 * @return Allocated group name string, NULL on error.
 * @warning The resulting pointer (return value) must be freed by faux_str_free().
 */
char *faux_sysdb_name_by_gid(gid_t gid)
{
	struct group *gr = NULL;
	char *name = NULL;

	gr = faux_sysdb_getgrgid(gid);
	if (!gr)
		return NULL; // Unknown group
	name = faux_str_dup(gr->gr_name);
	faux_free(gr);

	return name;
}
