/** @file testc_helpers.h
 * @brief Testc helper functions.
 */

#ifndef _faux_testc_helpers_h
#define _faux_testc_helpers_h

#include <stddef.h>

#include <faux/faux.h>

#define FAUX_TESTC_TMPDIR_ENV "TESTC_TMPDIR"

C_DECL_BEGIN

ssize_t faux_testc_file_deploy(const char *fn, const void *buf, size_t len);
ssize_t faux_testc_file_deploy_str(const char *fn, const char *str);
char *faux_testc_tmpfile_deploy(const void *buf, size_t len);
char *faux_testc_tmpfile_deploy_str(const char *str);
int faux_testc_file_cmp(const char *first_file, const char *second_file);
bool_t faux_testc_fill_rnd(void *buf, size_t len);
char *faux_testc_rnd_buf(size_t len);

C_DECL_END

#endif				/* _faux_testc_helpers_h */
