#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "faux/time.h"


#define TNUM1 3
int testc_faux_nsec_timespec_conversion(void)
{
	uint64_t n[TNUM1] = {
		123456789l,
		880123456789l,
		789000000000l
		};
	struct timespec e[TNUM1] = {
		{.tv_sec = 0, .tv_nsec = 123456789l},
		{.tv_sec = 880, .tv_nsec = 123456789l},
		{.tv_sec = 789, .tv_nsec = 0l}
		};
	unsigned int i = 0;
	int ret = 0;

	// From nsec to struct timespec
	for (i = 0; i < TNUM1; i++) {
		struct timespec res = {};
		faux_nsec_to_timespec(&res, n[i]);
		if (faux_timespec_cmp(&res, &e[i]) != 0) {
			printf("nsec_to_timespec: Test %u failed\n", i);
			ret = -1;
		}
	}

	// From struct timespec to nsec
	for (i = 0; i < TNUM1; i++) {
		uint64_t res = 0;
		res = faux_timespec_to_nsec(&e[i]);
		if (res != n[i]) {
			printf("timespec_to_nsec: Test %u failed\n", i);
			ret = -1;
		}
	}

	return ret;
}


#define TNUM2 2
int testc_faux_timespec_diff(void)
{
	struct timespec val1[TNUM2] = {
		{.tv_sec = 0, .tv_nsec = 123456789l},
		{.tv_sec = 880, .tv_nsec = 2l},
		};
	struct timespec val2[TNUM2] = {
		{.tv_sec = 1, .tv_nsec = 123456789l},
		{.tv_sec = 770, .tv_nsec = 3l},
		};
	struct timespec e[TNUM2] = {
		{.tv_sec = 0, .tv_nsec = 0l},
		{.tv_sec = 109, .tv_nsec = 999999999l},
		};
	bool_t eretval[TNUM2] = {
		BOOL_FALSE,
		BOOL_TRUE
		};
	int eerrno[TNUM2] = {
		EOVERFLOW,
		0
		};
	int ret = 0;
	int i = 0;

	// Diff
	for (i = 0; i < TNUM2; i++) {
		struct timespec res = {};
		bool_t retval = 0;
		int err = 0;
		printf("Test %u:\n", i);
		printf("val1=%ld:%ld, val2=%ld:%ld\n",
			val1[i].tv_sec, val1[i].tv_nsec,
			val2[i].tv_sec, val2[i].tv_nsec);
		retval = faux_timespec_diff(&res, &val1[i], &val2[i]);
		err = errno;
		printf("diff=%ld:%ld, etalon=%ld:%ld\n",
			res.tv_sec, res.tv_nsec,
			e[i].tv_sec, e[i].tv_nsec);
		if (faux_timespec_cmp(&res, &e[i]) != 0) {
			printf("Test %u timespec cmp failed\n", i);
			ret = -1;
		}
		if (retval != eretval[i]) {
			printf("Test %u retval failed. Actual: %d. Need: %d.\n",
				i, retval, eretval[i]);
			ret = -1;
		}
		if (!retval && (err != eerrno[i])) {
			printf("Test %u errno failed\n", i);
			ret = -1;
		}
	}

	return ret;
}


#define TNUM3 2
int testc_faux_timespec_sum(void)
{
	struct timespec val1[TNUM3] = {
		{.tv_sec = 0, .tv_nsec = 123456789l},
		{.tv_sec = 880, .tv_nsec = 2l},
		};
	struct timespec val2[TNUM3] = {
		{.tv_sec = 1, .tv_nsec = 910000000l},
		{.tv_sec = 710, .tv_nsec = 8l},
		};
	struct timespec e[TNUM3] = {
		{.tv_sec = 2, .tv_nsec = 33456789l},
		{.tv_sec = 1590, .tv_nsec = 10l},
		};
	int ret = 0;
	int i = 0;

	// Sum
	for (i = 0; i < TNUM3; i++) {
		struct timespec res = {};
		printf("Test %u:\n", i);
		printf("val1=%ld:%ld, val2=%ld:%ld\n",
			val1[i].tv_sec, val1[i].tv_nsec,
			val2[i].tv_sec, val2[i].tv_nsec);
		if (faux_timespec_sum(&res, &val1[i], &val2[i]) < 0) {
			printf("Test %u retval failed\n", i);
			ret = -1;
		}
		printf("sum=%ld:%ld, etalon=%ld:%ld\n",
			res.tv_sec, res.tv_nsec,
			e[i].tv_sec, e[i].tv_nsec);
		if (faux_timespec_cmp(&res, &e[i]) != 0) {
			printf("Test %u timespec cmp failed\n", i);
			ret = -1;
		}
	}

	return ret;
}


int testc_faux_timespec_now(void)
{
	int ret = 0;
	struct timespec before = {};
	struct timespec now = {};
	struct timespec after = {};
	struct timespec interval = {};

	faux_nsec_to_timespec(&interval, 1000000l);
	faux_timespec_now(&now);
	faux_timespec_diff(&before, &now, &interval);
	faux_timespec_sum(&after, &now, &interval);

	if (!faux_timespec_before_now(&before))
		ret = -1;
	if (!faux_timespec_before_now(&now)) // Formally now is before now
		ret = -1;
	if (faux_timespec_before_now(&after))
		ret = -1;

	return ret;
}
