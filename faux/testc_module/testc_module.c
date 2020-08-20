#include "faux/ini.h"

const unsigned char testc_version_major = 1;
const unsigned char testc_version_minor = 0;

const char *testc_module[][2] = {

	// Demo
//	{"testc_faux_ini_bad", "INI bad"}, // demo
//	{"testc_faux_ini_signal", "Interrupted by signal"}, // demo
//	{"testc_faux_ini_good", "INI subsystem good"}, // demo

	// str
	{"testc_faux_str_nextword", "Find next word (quotation)"},

	// ini
	{"testc_faux_ini_parse_file", "Complex test of INI file parsing"},

	// argv
	{"testc_faux_argv_parse", "Parse string to arguments"},
	{"testc_faux_argv_is_continuable", "Is line continuable"},

	// time
	{"testc_faux_nsec_timespec_conversion", "Converts nsec from/to struct timespec"},
	{"testc_faux_timespec_diff", "Diff beetween timespec structures"},
	{"testc_faux_timespec_sum", "Sum of timespec structures"},
	{"testc_faux_timespec_now", "Timespec now and before now functions"},

	// sched
	{"testc_faux_sched_once", "Schedule once event. Simple and delayed ones."},
	{"testc_faux_sched_periodic", "Schedule periodic event."},
	{"testc_faux_sched_infinite", "Schedule infinite number of events."},

	// log
	{"testc_faux_log_facility_id", "Converts syslog facility string to id"},
	{"testc_faux_log_facility_str", "Converts syslog facility id to string"},

	// vec
	{"testc_faux_vec", "Complex test of variable length vector"},


	// End of list
	{NULL, NULL}
	};
