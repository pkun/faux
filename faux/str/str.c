/** @file str.c
 * @brief String related functions
 *
 * This file implements some often used string functions.
 * Some functions are more portable versions of standard
 * functions but others are original ones.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "faux/ctype.h"
#include "faux/str.h"

/** @brief Free the memory allocated for the string.
 *
 * Safely free the memory allocated for the string. You can use NULL
 * pointer with this function. POSIX's free() checks for the NULL pointer
 * but not all systems do so.
 *
 * @param [in] str String to free
 */
void faux_str_free(char *str)
{
	faux_free(str);
}


/** @brief Duplicates the string.
 *
 * Duplicates the string. Same as standard strdup() function. Allocates
 * memory with malloc(). Checks for NULL pointer.
 *
 * @warning Resulting string must be freed by faux_str_free().
 *
 * @param [in] str String to duplicate.
 * @return Pointer to allocated string or NULL.
 */
char *faux_str_dup(const char *str)
{
	if (!str)
		return NULL;
	return strdup(str);
}


/** @brief Duplicates the first n bytes of the string.
 *
 * Duplicates at most n bytes of the string. Allocates
 * memory with malloc(). Checks for NULL pointer. Function will allocate
 * n + 1 bytes to store string and terminating null byte.
 *
 * @warning Resulting string must be freed by faux_str_free().
 *
 * @param [in] str String to duplicate.
 * @param [in] n Number of bytes to copy.
 * @return Pointer to allocated string or NULL.
 */
char *faux_str_dupn(const char *str, size_t n)
{
	char *res = NULL;
	size_t len = 0;

	if (!str)
		return NULL;
	// Search for terminating '\0' among first n bytes
	// Don't use strlen() because it can be not null-terminated.
	for (len = 0; len < n; len++)
		if ('\0' == str[len])
			break;
	len = (len < n) ? len : n;
	res = faux_zmalloc(len + 1);
	if (!res)
		return NULL;
	strncpy(res, str, len);
	res[len] = '\0';

	return res;
}


/** @brief Generates lowercase copy of input string.
 *
 * Allocates the copy of input string and convert that copy to lowercase.
 *
 * @warning Resulting string must be freed by faux_str_free().
 *
 * @param [in] str String to convert.
 * @return Pointer to lowercase string copy or NULL.
 */
char *faux_str_tolower(const char *str)
{
	char *res = faux_str_dup(str);
	char *p = res;

	if (!res)
		return NULL;

	while (*p) {
		*p = faux_ctype_tolower(*p);
		p++;
	}

	return res;
}


/** @brief Generates uppercase copy of input string.
 *
 * Allocates the copy of input string and convert that copy to uppercase.
 *
 * @warning Resulting string must be freed by faux_str_free().
 *
 * @param [in] str String to convert.
 * @return Pointer to lowercase string copy or NULL.
 */
char *faux_str_toupper(const char *str)
{
	char *res = faux_str_dup(str);
	char *p = res;

	if (!res)
		return NULL;

	while (*p) {
		*p = faux_ctype_toupper(*p);
		p++;
	}

	return res;
}


/** @brief Add n bytes of text to existent string.
 *
 * Concatenate two strings. Add n bytes of second string to the end of the
 * first one. The first argument is address of string pointer. The pointer
 * can be changed due to realloc() features. The first pointer can be NULL.
 * In this case the memory will be malloc()-ed and stored to the first pointer.
 *
 * @param [in,out] str Address of first string pointer.
 * @param [in] text Text to add to the first string.
 * @param [in] n Number of bytes to add.
 * @return Pointer to resulting string or NULL.
 */
char *faux_str_catn(char **str, const char *text, size_t n)
{
	size_t str_len = 0;
	size_t text_len = 0;
	char *res = NULL;
	char *p = NULL;

	if (!text)
		return *str;

	str_len = (*str) ? strlen(*str) : 0;
	text_len = strlen(text);
	text_len = (text_len < n) ? text_len : n;

	res = realloc(*str, str_len + text_len + 1);
	if (!res)
		return NULL;
	p = res + str_len;
	strncpy(p, text, text_len);
	p[text_len] = '\0';
	*str = res;

	return res;
}


/** @brief Add some text to existent string.
 *
 * Concatenate two strings. Add second string to the end of the first one.
 * The first argument is address of string pointer. The pointer can be
 * changed due to realloc() features. The first pointer can be NULL. In this
 * case the memory will be malloc()-ed and stored to the first pointer.
 *
 * @param [in,out] str Address of first string pointer.
 * @param [in] text Text to add to the first string.
 * @return Pointer to resulting string or NULL.
 */
char *faux_str_cat(char **str, const char *text)
{
	size_t len = 0;

	if (!text)
		return *str;
	len = strlen(text);

	return faux_str_catn(str, text, len);
}

/** @brief Add multiply text strings to existent string.
 *
 * Concatenate multiply strings. Add next string to the end of the previous one.
 * The first argument is address of string pointer. The pointer can be
 * changed due to realloc() features. The first pointer can be NULL. In this
 * case the memory will be malloc()-ed and stored to the first pointer.
 * The last argument must be 'NULL'. It marks the last argument within
 * variable arguments list.
 *
 * @warning If last argument is not 'NULL' then behaviour is undefined.
 *
 * @param [in,out] str Address of first string pointer.
 * @param [in] text Text to add to the first string.
 * @return Pointer to resulting string or NULL.
 */
char *faux_str_mcat(char **str, ...)
{
	va_list ap;
	const char *arg = NULL;
	char *retval = *str;

	va_start(ap, str);
	while ((arg = va_arg(ap, const char *))) {
		retval = faux_str_cat(str, arg);
	}
	va_end(ap);

	return retval;
}


/** @brief Allocates memory and vsprintf() to it.
 *
 * Function tries to find out necessary amount of memory for specified format
 * string and arguments. Format is same as for vsprintf() function. Then
 * function allocates memory for resulting string and vsprintf() to it. So
 * user doesn't need to allocate buffer himself. Function returns allocated
 * string that need to be freed by faux_str_free() function later.
 *
 * @warning The returned pointer must be free by faux_str_free().
 *
 * @param [in] fmt Format string like the sprintf()'s fmt.
 * @param [in] ap The va_list argument.
 * @return Allocated resulting string or NULL on error.
 */
char *faux_str_vsprintf(const char *fmt, va_list ap)
{
	int size = 1;
	char calc_buf[1] = "";
	char *line = NULL;
	va_list ap2;

	// Calculate buffer size
	va_copy(ap2, ap);
	size = vsnprintf(calc_buf, size, fmt, ap2);
	va_end(ap2);
	// The snprintf() prior to 2.0.6 glibc version returns -1 if string
	// was truncated. The later glibc returns required buffer size.
	// The calc_buf can be NULL and size can be 0 for recent glibc but
	// probably some exotic implementations can break on it. So use
	// minimal buffer with length = 1.
	if (size < 0)
		return NULL;

	size++; // Additional byte for '\0'
	line = faux_zmalloc(size);
	if (!line) // Memory problems
		return NULL;

	// Format real string
	size = vsnprintf(line, size, fmt, ap);
	if (size < 0) { // Some problems
		faux_str_free(line);
		return NULL;
	}

	return line;
}


/** @brief Allocates memory and sprintf() to it.
 *
 * Function tries to find out necessary amount of memory for specified format
 * string and arguments. Format is same as for sprintf() function. Then
 * function allocates memory for resulting string and sprintf() to it. So
 * user doesn't need to allocate buffer himself. Function returns allocated
 * string that need to be freed by faux_str_free() function later.
 *
 * @warning The returned pointer must be free by faux_str_free().
 *
 * @param [in] fmt Format string like the sprintf()'s fmt.
 * @param [in] arg Number of arguments.
 * @return Allocated resulting string or NULL on error.
 */
char *faux_str_sprintf(const char *fmt, ...)
{
	char *line = NULL;
	va_list ap;

	va_start(ap, fmt);
	line = faux_str_vsprintf(fmt, ap);
	va_end(ap);

	return line;
}


/** @brief Service function to compare to chars in right way.
 *
 * The problem is char type can be signed or unsigned on different
 * platforms. So stright comparision can return different results.
 *
 * @param [in] char1 First char
 * @param [in] char2 Second char
 * @return
 * < 0 if char1 < char2
 * = 0 if char1 = char2
 * > 0 if char1 > char2
 */
static int faux_str_cmp_chars(char char1, char char2)
{
	unsigned char ch1 = (unsigned char)char1;
	unsigned char ch2 = (unsigned char)char2;

	return (int)ch1 - (int)ch2;
}


/** @brief Compare n first characters of two strings ignoring case.
 *
 * The difference beetween this function an standard strncasecmp() is
 * faux function uses faux ctype functions. It can be important for
 * portability.
 *
 * @param [in] str1 First string to compare.
 * @param [in] str2 Second string to compare.
 * @param [in] n Number of characters to compare.
 * @return < 0, 0, > 0, see the strcasecmp().
 */
int faux_str_casecmpn(const char *str1, const char *str2, size_t n)
{
	const char *p1 = str1;
	const char *p2 = str2;
	size_t num = n;

	while (*p1 != '\0' && *p2 != '\0' && num != 0) {
		int res = faux_str_cmp_chars(
			faux_ctype_tolower(*p1), faux_ctype_tolower(*p2));
		if (res != 0)
			return res;
		p1++;
		p2++;
		num--;
	}

	if (0 == n) // It means n first characters are equal.
		return 0;

	return faux_str_cmp_chars(
		faux_ctype_tolower(*p1), faux_ctype_tolower(*p2));
}


/** @brief Compare two strings ignoring case.
 *
 * The difference beetween this function an standard strcasecmp() is
 * faux function uses faux ctype functions. It can be important for
 * portability.
 *
 * @param [in] str1 First string to compare.
 * @param [in] str2 Second string to compare.
 * @return < 0, 0, > 0, see the strcasecmp().
 */
int faux_str_casecmp(const char *str1, const char *str2)
{
	const char *p1 = str1;
	const char *p2 = str2;

	if (!p1 && !p2) // Empty strings are equal
		return 0;

	if (!p1) // Consider NULL string to be less then empty string
		return -1;

	if (!p2) // Consider NULL string to be less then empty string
		return 1;

	while (*p1 != '\0' && *p2 != '\0') {
		int res = faux_str_cmp_chars(
			faux_ctype_tolower(*p1), faux_ctype_tolower(*p2));
		if (res != 0)
			return res;
		p1++;
		p2++;
	}

	return faux_str_cmp_chars(
		faux_ctype_tolower(*p1), faux_ctype_tolower(*p2));
}


/** @brief Finds the first occurrence of the substring in the string
 *
 * Function is a faux version of strcasestr() function.
 *
 * @param [in] haystack String to find substring in it.
 * @param [in] needle Substring to find.
 * @return
 * Pointer to first occurence of substring in the string.
 * NULL on error
 */
char *faux_str_casestr(const char *haystack, const char *needle)
{
	const char *ptr = haystack;
	size_t ptr_len = 0;
	size_t needle_len = 0;

	assert(haystack);
	assert(needle);
	if (!haystack || !needle)
		return NULL;

	ptr_len = strlen(haystack);
	needle_len = strlen(needle);

	while ((*ptr != '\0') && (ptr_len >= needle_len)) {
		int res = faux_str_casecmpn(ptr, needle, needle_len);
		if (0 == res)
			return (char *)ptr;
		ptr++;
		ptr_len--;
	}

	return NULL; // Not found
}


/** Prepare string for embedding to C-code (make escaping).
 *
 * @warning The returned pointer must be freed by faux_str_free().
 * @param [in] src String for escaping.
 * @return Escaped string or NULL on error.
 */
char *faux_str_c_esc(const char *src)
{
	const char *src_ptr = src;
	char *dst = NULL;
	char *dst_ptr = NULL;
	char *escaped = NULL;
	size_t src_len = 0;
	size_t dst_len = 0;

	assert(src);
	if (!src)
		return NULL;

	src_len = strlen(src);
	// Calculate max destination string size.
	// The worst case is when each src character will be replaced by
	// something like '\xff'. So it's 4 dst chars for 1 src one.
	dst_len = (src_len * 4) + 1; // one byte for '\0'
	dst = faux_zmalloc(dst_len);
	assert(dst);
	if (!dst)
		return NULL;
	dst_ptr = dst;

	while (*src_ptr != '\0') {
		char *esc = NULL; // escaped replacement
		char buf[5]; // longest 'char' (4 bytes) + '\0'
		size_t len = 0;

		switch (*src_ptr) {
		case '\n':
			esc = "\\n";
			break;
		case '\"':
			esc = "\\\"";
			break;
		case '\\':
			esc = "\\\\";
			break;
		case '\'':
			esc = "\\\'";
			break;
		case '\r':
			esc = "\\r";
			break;
		case '\t':
			esc = "\\t";
			break;
		default:
			// Check is the symbol control character. Control
			// characters has codes from 0x00 to 0x1f.
			if (((unsigned char)*src_ptr & 0xe0) == 0) { // control
				snprintf(buf, sizeof(buf), "\\x%02x",
					(unsigned char)*src_ptr);
				buf[4] = '\0'; // for safety
			} else {
				buf[0] = *src_ptr; // Common character
				buf[1] = '\0';
			}
			esc = buf;
			break;
		}

		len = strlen(esc);
		memcpy(dst_ptr, esc, len); // zmalloc() nullify the rest
		dst_ptr += len;
		src_ptr++;
	}

	escaped = faux_str_dup(dst); // Free some memory
	faux_str_free(dst); // 'dst' size >= 'escaped' size

	return escaped;
}


#define BYTE_CONV_LEN 4 // Length of one byte converted to string

/** Prepare binary block for embedding to C-code.
 *
 * @warning The returned pointer must be freed by faux_str_free().
 * @param [in] src Binary block for conversion.
 * @return C-string or NULL on error.
 */
char *faux_str_c_bin(const char *src, size_t n)
{
	const char *src_ptr = src;
	char *dst = NULL;
	char *dst_ptr = NULL;
	size_t dst_len = 0;

	assert(src);
	if (!src)
		return NULL;

	// Calculate destination string size.
	// Each src character will be replaced by
	// something like '\xff'. So it's 4 dst chars for 1 src char.
	dst_len = (n * BYTE_CONV_LEN) + 1; // one byte for '\0'
	dst = faux_zmalloc(dst_len);
	assert(dst);
	if (!dst)
		return NULL;
	dst_ptr = dst;

	while (src_ptr < (src + n)) {
		char buf[BYTE_CONV_LEN + 1]; // longest 'char' (4 bytes) + '\0'

		snprintf(buf, sizeof(buf), "\\x%02x", (unsigned char)*src_ptr);
		memcpy(dst_ptr, buf, BYTE_CONV_LEN); // zmalloc() nullify the rest
		dst_ptr += BYTE_CONV_LEN;
		src_ptr++;
	}

	return dst;
}


/** @brief Search the n-th chars of string for one of the specified chars.
 *
 * The function search for any of specified characters within string.
 * The search is limited to first n characters of the string. If
 * terminating '\0' is before n-th character then search will stop on
 * it. Can be used with raw memory block.
 *
 * @param [in] str String (or memory block) to search in.
 * @param [in] chars_to_string Chars enumeration to search for.
 * @param [in] n Maximum number of bytes to search within.
 * @return Pointer to the first occurence of one of specified chars.
 * NULL on error.
 */
char *faux_str_charsn(const char *str, const char *chars_to_search, size_t n)
{
	const char *current_char = str;
	size_t len = n;

	assert(str);
	assert(chars_to_search);
	if (!str || !chars_to_search)
		return NULL;

	while ((*current_char != '\0') && (len > 0)) {
		if (strchr(chars_to_search, *current_char))
			return (char *)current_char;
		current_char++;
		len--;
	}

	return NULL;
}


/** @brief Search string for one of the specified chars.
 *
 * The function search for any of specified characters within string.
 *
 * @param [in] str String to search in.
 * @param [in] chars_to_string Chars enumeration to search for.
 * @return Pointer to the first occurence of one of specified chars.
 * NULL on error.
 */
char *faux_str_chars(const char *str, const char *chars_to_search)
{
	assert(str);
	if (!str)
		return NULL;

	return faux_str_charsn(str, chars_to_search, strlen(str));
}


/** @brief Remove escaping. Convert string to internal view.
 *
 * Find backslashes (before escaped symbols) and remove it. Escaped symbol
 * will not be analyzed so `\\` will lead to `\`.
 *
 * @param [in] string Escaped string.
 * @param [in] len Length of string to de-escape.
 * @return Allocated de-escaped string
 * @warning Returned value must be freed by faux_str_free() later.
 */
static char *faux_str_deesc(const char *string, size_t len)
{
	const char *s = string;
	char *res = NULL;
	char *p = NULL;
	bool_t escaped = BOOL_FALSE;

	assert(string);
	if (!string)
		return NULL;
	if (0 == len)
		return NULL;

	res = faux_zmalloc(len + 1);
	assert(res);
	if (!res)
		return NULL;
	p = res;

	while ((*s != '\0') && (s < (string +len))) {
		if (('\\' == *s) && !escaped) {
			escaped = BOOL_TRUE;
			s++;
			continue;
		}
		escaped = BOOL_FALSE;
		*p = *s;
		s++;
		p++;
	}
	*p = '\0';

	return res;
}


/*--------------------------------------------------------- */
/** @brief Find next word or quoted substring within string
 *
 * The quotation can be of several different kinds.
 *
 * The first kind is standard double quoting. In this case the internal (within
 * quotation) `"` and `\` symbols must be escaped. But symbols will be deescaped
 * before writing to internal buffers.
 *
 * The second kind of quotation is alternative quotation. Any symbol can become
 * quote sign. For example "`" and "'" can be considered as a quotes. To use
 * some symbols as a quote them must be specified by `alt_quotes` function
 * parameter. The single symbol can be considered as a start of quotation or
 * a sequence of the same symbols can be considered as a start of quotation. In
 * this case the end of quotation is a sequence of the same symbols. The same
 * symbol can appear inside quotation but number of symbols (sequence) must be
 * less than opening quote sequence. The example of alternatively quoted string
 * is ```some text``and anothe`r```. The backslash has no special meaning inside
 * quoted string.
 *
 * The substring can be unquoted string without spaces. The space, backslash and
 * quote can be escaped by backslash.
 *
 * Parts of text with different quotes can be glued together to get single
 * substring like this: aaa"inside dbl quote"bbb``alt quote"`here``ccc.
 *
 * @param [in] str String to parse.
 * @param [out] saveptr Pointer to first symbol after found substring.
 * @param [in] alt_quotes Possible alternative quotes.
 * @param [out] qclosed Flag is quote closed.
 * @return Allocated buffer with found substring (without quotes).
 * @warning Returned alocated buffer must be freed later by faux_str_free()
 */
char *faux_str_nextword(const char *str, const char **saveptr,
	const char *alt_quotes, bool_t *qclosed)
{
	const char *string = str;
	const char *word = NULL;
	size_t len = 0;
	const char dbl_quote = '"';
	bool_t dbl_quoted = BOOL_FALSE;
	char alt_quote = '\0';
	unsigned int alt_quote_num = 0; // Number of opening alt quotes
	bool_t alt_quoted = BOOL_FALSE;
	char *result = NULL;

	// Find the start of a word (not including an opening quote)
	while (*string && isspace(*string))
		string++;

	word = string; // Suppose not quoted string

	while (*string != '\0') {

		// Standard double quotation
		if (dbl_quoted) {
			// End of word
			if (*string == dbl_quote) {
				if (len > 0) {
					char *s = faux_str_deesc(word, len);
					faux_str_cat(&result, s);
					faux_str_free(s);
				}
				dbl_quoted = BOOL_FALSE;
				string++;
				word = string;
				len = 0;
			// Escaping
			} else if (*string == '\\') {
				// Skip escaping
				string++;
				len++;
				// Skip escaped symbol
				if (*string) {
					string++;
					len++;
				}
			} else {
				string++;
				len++;
			}

		// Alternative multi quotation
		} else if (alt_quoted) {
			unsigned int qnum = alt_quote_num;
			while (string && (*string == alt_quote) && qnum) {
				string++;
				len++;
				qnum--;
			}
			if (0 == qnum) { // End of word was found
				// Quotes themselfs are not a part of a word
				len -= alt_quote_num;
				if (len > 0)
					faux_str_catn(&result, word, len);
				alt_quoted = BOOL_FALSE;
				word = string;
				len = 0;
			} else if (qnum == alt_quote_num) { // No quote syms
				string++;
				len++;
			}

		// Not quoted
		} else {
			// Start of a double quoted string
			if (*string == dbl_quote) {
				if (len > 0) {
					char *s = faux_str_deesc(word, len);
					faux_str_cat(&result, s);
					faux_str_free(s);
				}
				dbl_quoted = BOOL_TRUE;
				string++;
				word = string;
				len = 0;
			// Start of alt quoted string
			} else if (alt_quotes && strchr(alt_quotes, *string)) {
				if (len > 0) {
					char *s = faux_str_deesc(word, len);
					faux_str_cat(&result, s);
					faux_str_free(s);
				}
				alt_quoted = BOOL_TRUE;
				alt_quote = *string;
				alt_quote_num = 0;
				while (string && (*string == alt_quote)) {
					string++;
					alt_quote_num++; // Count starting quotes
				}
				word = string;
				len = 0;
			// End of word
			} else if (isspace(*string)) {
				if (len > 0) {
					char *s = faux_str_deesc(word, len);
					faux_str_cat(&result, s);
					faux_str_free(s);
				}
				word = string;
				len = 0;
				break;
			// Escaping
			} else if (*string == '\\') {
				// Skip escaping
				string++;
				len++;
				// Skip escaped symbol
				if (*string) {
					string++;
					len++;
				}
			} else {
				string++;
				len++;
			}
		}
	}

	if (len > 0) {
		if (alt_quoted) {
			faux_str_catn(&result, word, len);
		} else {
			char *s = faux_str_deesc(word, len);
			faux_str_cat(&result, s);
			faux_str_free(s);
		}
	}

	if (saveptr)
		*saveptr = string;
	if (qclosed)
		*qclosed = ! (dbl_quoted || alt_quoted);

	return result;
}


/** @brief Indicates is string is empty.
 *
 * @param [in] str String to analyze.
 * @return BOOL_TRUE if pointer is NULL or empty, BOOL_FALSE if not empty.
 */
bool_t faux_str_is_empty(const char *str)
{
	if (!str)
		return BOOL_TRUE;
	if ('\0' == *str)
		return BOOL_TRUE;

	return BOOL_FALSE;
}
