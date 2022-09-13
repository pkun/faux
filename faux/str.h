/** @file str.h
 * @brief Public interface for faux string functions.
 */

#ifndef _faux_str_h
#define _faux_str_h

#include <stddef.h>
#include <stdarg.h>

#include <faux/faux.h>

#define UTF8_MASK 0xC0
#define UTF8_7BIT_MASK 0x80 // One byte or multibyte
#define UTF8_11   0xC0 // First UTF8 byte
#define UTF8_10   0x80 // Next UTF8 bytes

C_DECL_BEGIN

void faux_str_free(char *str);

char *faux_str_dupn(const char *str, size_t n);
char *faux_str_dup(const char *str);

char *faux_str_catn(char **str, const char *text, size_t n);
char *faux_str_cat(char **str, const char *text);
char *faux_str_mcat(char **str, ...);
char *faux_str_vsprintf(const char *fmt, va_list ap);
char *faux_str_sprintf(const char *fmt, ...);

char *faux_str_tolower(const char *str);
char *faux_str_toupper(const char *str);

int faux_str_cmpn(const char *str1, const char *str2, size_t n);
int faux_str_cmp(const char *str1, const char *str2);
int faux_str_casecmpn(const char *str1, const char *str2, size_t n);
int faux_str_casecmp(const char *str1, const char *str2);
int faux_str_numcmp(const char *str1, const char *str2);
char *faux_str_casestr(const char *haystack, const char *needle);
char *faux_str_charsn(const char *str, const char *chars_to_search, size_t n);
char *faux_str_chars(const char *str, const char *chars_to_search);
bool_t faux_str_is_empty(const char *str);

char *faux_str_c_esc(const char *src);
char *faux_str_c_bin(const char *src, size_t n);

char *faux_str_nextword(const char *str, const char **saveptr,
	const char *alt_quotes, bool_t *qclosed);
char *faux_str_getline(const char *str, const char **saveptr);

C_DECL_END

#endif				/* _faux_str_h */
