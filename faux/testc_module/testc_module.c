#include "faux/ini.h"

const unsigned char testc_version_major = 1;
const unsigned char testc_version_minor = 0;

const char *testc_module[][2] = {

	// Demo
//	{"testc_faux_ini_bad", "INI bad"}, // demo
//	{"testc_faux_ini_signal", "Interrupted by signal"}, // demo
//	{"testc_faux_ini_good", "INI subsystem good"}, // demo

	// Str
	{"testc_faux_str_nextword", "Find next word (quotation)"},

	// INI
	{"testc_faux_ini_parse_file", "Complex test of INI file parsing"},

	// argv
	{"testc_faux_argv_parse", "Parse string to arguments"},

	// End of list
	{NULL, NULL}
	};
