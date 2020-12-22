#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "faux/log.h"
#include "faux/str.h"

int testc_faux_log_facility_id(void)
{
	const char *str = "daemon";
	int e = LOG_DAEMON;
	int r = 0;

	if (!faux_log_facility_id(str, &r)) {
		printf("Can't get id by string\n");
		return -1;
	}
	if (r != e) {
		printf("Etalon: %d, Result: %d\n", e, r);
		return -1;
	}

	return 0;
}

int testc_faux_log_facility_str(void)
{
	int id = LOG_KERN;
	const char *e = "kern";
	const char *r = NULL;

	r = faux_log_facility_str(id);
	if (strcmp(r, e) != 0) {
		printf("Etalon: %s, Result: %s\n", e, r);
		return -1;
	}

	return 0;
}
