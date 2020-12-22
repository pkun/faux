/** @brief Some usefull function to work with time structs.
 */

#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>

#include "faux/time.h"


/** @brief Compares two time (struct timespec) values.
 *
 * @param [in] val1 First timespec struct value.
 * @param [in] val2 Second timespec struct value.
 * @return 0 if val1==val2, 1 if val1>val2, -1 if val1<val2.
*/
int faux_timespec_cmp(const struct timespec *val1, const struct timespec *val2)
{
	assert(val1);
	assert(val2);
	if (!val1 && !val2)
		return 0;
	if (val1 && !val2)
		return 1;
	if (!val1 && val2)
		return -1;

	if (val1->tv_sec > val2->tv_sec)
		return 1;
	if (val1->tv_sec < val2->tv_sec)
		return -1;
	// Seconds are equal

	if (val1->tv_nsec > val2->tv_nsec)
		return 1;
	if (val1->tv_nsec < val2->tv_nsec)
		return -1;
	// Nanoseconds are equal too

	return 0;
}


/** @brief Calculates difference between two time (struct timespec) values.
 *
 * It implements "res = val1 - val2" function.
 *
 * @param [out] res Result of operation.
 * @param [in] val1 First struct timespec value.
 * @param [in] val2 Second struct timespec value.
 * @return 0 - BOOL_TRUE, BOOL_FALSE on error.
 * @exception EINVAL Invalid arguments value.
 * @exception EOVERFLOW If val2>val1.
 */
bool_t faux_timespec_diff(struct timespec *res,
	const struct timespec *val1, const struct timespec *val2)
{
	assert(res);
	assert(val1);
	assert(val2);
	if (!res || !val1 || !val2) {
		errno = EINVAL;
		return BOOL_FALSE;
	}

	if (faux_timespec_cmp(val1, val2) < 0) {
		errno = EOVERFLOW;
		return BOOL_FALSE;
	}

	res->tv_sec = val1->tv_sec - val2->tv_sec;
	if (val1->tv_nsec < val2->tv_nsec) {
		res->tv_sec -= 1;
		res->tv_nsec = 1000000000l - val2->tv_nsec + val1->tv_nsec;
	} else {
		res->tv_nsec = val1->tv_nsec - val2->tv_nsec;
	}

	return BOOL_TRUE;
}

/** @brief Sum of two time (struct timespec) values.
 *
 * Function implements "res = val1 + val2" operation.
 *
 * @param [out] res Result of operation.
 * @param [in] val1 First time value.
 * @param [in] val2 Second time value.
 * @return BOOL_TRUE - success, BOOL_FALSE on error.
 * @exception EINVAL Invalid arguments value.
 */
bool_t faux_timespec_sum(struct timespec *res,
	const struct timespec *val1, const struct timespec *val2)
{
	assert(res);
	assert(val1);
	assert(val2);
	if (!res || !val1 || !val2) {
		errno = EINVAL;
		return BOOL_FALSE;
	}

	res->tv_sec = val1->tv_sec + val2->tv_sec + ((val1->tv_nsec + val2->tv_nsec) / 1000000000l);
	res->tv_nsec = (val1->tv_nsec + val2->tv_nsec) % 1000000000l;

	return BOOL_TRUE;
}

/** @brief Converts struct timespec value to nanoseconds.
 *
 * @param [in] ts Struct timespec to convert.
 * @return Number of nanoseconds or 0 if argument is invalid.
 */
uint64_t faux_timespec_to_nsec(const struct timespec *ts)
{
	assert(ts);
	if (!ts)
		return 0;

	return ((uint64_t)ts->tv_sec * 1000000000l) + (uint64_t)ts->tv_nsec;
}

/** @brief Converts nanoseconds to struct timespec value.
 *
 * @param [out] ts Struct timespec pointer to save result of conversion.
 * @param [in] nsec Time in nanoseconds to convert.
 */
void faux_nsec_to_timespec(struct timespec *ts, uint64_t nsec)
{
	assert(ts);
	if (!ts)
		return;

	ts->tv_sec = (time_t)(nsec / 1000000000l);
	ts->tv_nsec = (long)(nsec % 1000000000l);
}


/** @brief Returns current time (now).
 *
 * @param [out] now The struct timespec to save current time.
 * @return 0 - success, < 0 on error.
 */
int faux_timespec_now(struct timespec *now)
{
	assert(now);
	if (!now)
		return -1;

	clock_gettime(CLOCK_REALTIME, now);

	return 0;
}


/** @brief Returns current time using CLOCK_MONOTONIC (now).
 *
 * CLOCK_MONOTONIC is not ajustable by NTP and etc. But it represents
 * time from system up but not since Epoch.
 *
 * @param [out] now The struct timespec to save current time.
 * @return 0 - success, < 0 on error.
 */
int faux_timespec_now_monotonic(struct timespec *now)
{
	assert(now);
	if (!now)
		return -1;

	clock_gettime(CLOCK_MONOTONIC, now);

	return 0;
}


/** @brief Indicates if specified struct timespec is before now.
 *
 * The equality to current time (now) is considered as already
 * coming time.
 *
 * @param [in] ts The struct timespec to compare.
 * @return BOOL_TRUE if timespec is before now else BOOL_FALSE.
 */
bool_t faux_timespec_before_now(const struct timespec *ts)
{
	struct timespec now = {};

	assert(ts);
	if (!ts)
		return BOOL_FALSE;

	faux_timespec_now(&now);
	if (faux_timespec_cmp(&now, ts) >= 0) // Already happend
		return BOOL_TRUE;

	return BOOL_FALSE;
}
