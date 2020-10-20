/** @file time.h
 * @brief Public interface for time service functions.
 */

#ifndef _faux_time_h
#define _faux_time_h

#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include <faux/faux.h>

C_DECL_BEGIN

// Operations for struct timespec
int faux_timespec_cmp(const struct timespec *val1, const struct timespec *val2);
int faux_timespec_diff(struct timespec *res,
	const struct timespec *val1, const struct timespec *val2);
int faux_timespec_sum(struct timespec *res,
	const struct timespec *val1, const struct timespec *val2);
int faux_timespec_now(struct timespec *now);
int faux_timespec_now_monotonic(struct timespec *now);
bool_t faux_timespec_before_now(const struct timespec *ts);

// Convertions of struct timespec
uint64_t faux_timespec_to_nsec(const struct timespec *ts);
void faux_nsec_to_timespec(struct timespec *ts, uint64_t nsec);

C_DECL_END

#endif /* _faux_time_h */
