/** @file conv.c
 * @brief Functions to convert from string to integer.
 */

#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include "faux/conv.h"
#include "faux/str.h"


/** @brief Converts string to long int
 *
 * Converts string to long int and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atol(const char *str, long int *val, int base)
{
	char *endptr = NULL;
	long int res = 0;

	errno = 0; // man recommends to do so
	res = strtol(str, &endptr, base);
	// Check for overflow
	if (((LONG_MIN == res) || (LONG_MAX == res)) && (ERANGE == errno))
		return BOOL_FALSE;
	// No valid digits at all
	if ((0 == res) && ((endptr == str) || (errno != 0)))
		return BOOL_FALSE;
	*val = res;

	return BOOL_TRUE;
}


/** @brief Converts string to unsigned long int
 *
 * Converts string to unsigned long int and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atoul(const char *str, unsigned long int *val, int base)
{
	char *endptr = NULL;
	unsigned long int res = 0;

	errno = 0; // man recommends to do so
	res = strtoul(str, &endptr, base);
	// Check for overflow
	if ((ULONG_MAX == res) && (ERANGE == errno))
		return BOOL_FALSE;
	// No valid digits at all
	if ((0 == res) && ((endptr == str) || (errno != 0)))
		return BOOL_FALSE;
	*val = res;

	return BOOL_TRUE;
}


/** @brief Converts string to long long int
 *
 * Converts string to long long int and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atoll(const char *str, long long int *val, int base)
{
	char *endptr = NULL;
	long long int res = 0;

	errno = 0; // man recommends to do so
	res = strtoll(str, &endptr, base);
	// Check for overflow
	if (((LLONG_MIN == res) || (LLONG_MAX == res)) && (ERANGE == errno))
		return BOOL_FALSE;
	// No valid digits at all
	if ((0 == res) && ((endptr == str) || (errno != 0)))
		return BOOL_FALSE;
	*val = res;

	return BOOL_TRUE;
}


/** @brief Converts string to unsigned long long int
 *
 * Converts string to unsigned long long int and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atoull(const char *str, unsigned long long int *val, int base)
{
	char *endptr = NULL;
	unsigned long long int res = 0;

	errno = 0; // man recommends to do so
	res = strtoull(str, &endptr, base);
	// Check for overflow
	if ((ULLONG_MAX == res) && (ERANGE == errno))
		return BOOL_FALSE;
	// No valid digits at all
	if ((0 == res) && ((endptr == str) || (errno != 0)))
		return BOOL_FALSE;
	*val = res;

	return BOOL_TRUE;
}


/** @brief Converts string to int
 *
 * Converts string to int and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atoi(const char *str, int *val, int base)
{
	long int tmp = 0;

	// Use existent func. The long int is longer or equal to int.
	if (faux_conv_atol(str, &tmp, base) < 0)
		return BOOL_FALSE;
	if ((tmp < INT_MIN) || (tmp > INT_MAX)) // Overflow
		return BOOL_FALSE;
	*val = tmp;

	return BOOL_TRUE;
}


/** @brief Converts string to unsigned int
 *
 * Converts string to unsigned int and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atoui(const char *str, unsigned int *val, int base)
{
	unsigned long int tmp = 0;

	// Use existent func. The long int is longer or equal to int.
	if (faux_conv_atoul(str, &tmp, base) < 0)
		return BOOL_FALSE;
	if (tmp > UINT_MAX) // Overflow
		return BOOL_FALSE;
	*val = tmp;

	return BOOL_TRUE;
}


/** @brief Converts string to short
 *
 * Converts string to short and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atos(const char *str, short *val, int base)
{
	long int tmp = 0;

	if (faux_conv_atol(str, &tmp, base) < 0)
		return BOOL_FALSE;
	if ((tmp < SHRT_MIN) || (tmp > SHRT_MAX)) // Overflow
		return BOOL_FALSE;
	*val = tmp;

	return BOOL_TRUE;
}


/** @brief Converts string to unsigned short
 *
 * Converts string to unsigned short and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atous(const char *str, unsigned short *val, int base)
{
	unsigned long int tmp = 0;

	if (faux_conv_atoul(str, &tmp, base) < 0)
		return BOOL_FALSE;
	if (tmp > USHRT_MAX) // Overflow
		return BOOL_FALSE;
	*val = tmp;

	return BOOL_TRUE;
}


/** @brief Converts string to char
 *
 * Converts string to char and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atoc(const char *str, char *val, int base)
{
	long int tmp = 0;

	if (faux_conv_atol(str, &tmp, base) < 0)
		return BOOL_FALSE;
	if ((tmp < CHAR_MIN) || (tmp > CHAR_MAX)) // Overflow
		return BOOL_FALSE;
	*val = tmp;

	return BOOL_TRUE;
}


/** @brief Converts string to unsigned char
 *
 * Converts string to unsigned char and check for overflow and valid
 * input values. Function indicates error by return value. It
 * returns the convertion result by second argument.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @param [in] base Base to convert.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_atouc(const char *str, unsigned char *val, int base)
{
	unsigned long int tmp = 0;

	if (faux_conv_atoul(str, &tmp, base) < 0)
		return BOOL_FALSE;
	if (tmp > UCHAR_MAX) // Overflow
		return BOOL_FALSE;
	*val = tmp;

	return BOOL_TRUE;
}


/** @brief Converts string to bool_t
 *
 * Case insensitive.
 *
 * @param [in] str Input string to convert.
 * @param [out] val Pointer to result value.
 * @return BOOL_TRUE - success, BOOL_FALSE - error
 */
bool_t faux_conv_str2bool(const char *str, bool_t *val)
{
	if (!str)
		return BOOL_FALSE;

	if (faux_str_casecmp(str, "true") == 0) {
		if (val)
			*val = BOOL_TRUE;
		return BOOL_TRUE;
	}

	if (faux_str_casecmp(str, "false") == 0) {
		if (val)
			*val = BOOL_FALSE;
		return BOOL_TRUE;
	}

	return BOOL_FALSE;
}


/** @brief Converts bool_t to string
 *
 * @param [in] val Boolean value.
 * @return "true" or "false" strings
 */
const char *faux_conv_bool2str(bool_t *val)
{
	if (val)
		return "true";

	return "false";
}
