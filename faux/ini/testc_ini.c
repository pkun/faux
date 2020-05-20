#include <stdlib.h>
#include <stdio.h>

#include "faux/ini.h"
#include "faux/testc_helpers.h"

int testc_faux_ini_good(void) {

	char *path = NULL;

	path = getenv("FAUX_INI_PATH");
	if (path)
		printf("Env var is [%s]\n", path);
	return 0;
}


int testc_faux_ini_bad(void) {

	printf("Some debug information here\n");
	return -1;
}

int testc_faux_ini_signal(void) {

	char *p = NULL;

	printf("%s\n", p);
	return -1;
}

int testc_faux_ini_parse(void) {

	// Source INI file
	const char *src_file =
		"# Comment\n"
		"DISTRIB_ID=Ubuntu\n"
		"DISTRIB_RELEASE=18.04\n"
		"DISTRIB_CODENAME=bionic\n"
		"DISTRIB_DESCRIPTION=\"Ubuntu 18.04.4 LTS\"\n"
		"COMPLEX_VAR=\"  Ubuntu\t\t1818 \"\n"
		"WO_QUOTES_VAR = qwerty\n"
		"WO_QUOTES_VAR2 = qwerty 98989898\n"
		"EMPTY_VAR3 = \n"
		"EMPTY_VAR4 =\n"
		"     EMPTY_VAR5 = \"\"\t   \n"
		"     ANOTHER_VAR6 = \"Normal var\"\t   \n"
		"\tTABBED_VAR = \"Normal tabbed var\"\t   \n"
		"# Another comment\n"
		"  # Yet another comment\n"
		"\t# Tabbed comment\n"
		"VAR_WITHOUT_EOL=zxcvbnm"
	;

// Etalon file
	const char *etalon_file =
		"ANOTHER_VAR6=\"Normal var\"\n"
		"COMPLEX_VAR=\"  Ubuntu\t\t1818 \"\n"
		"DISTRIB_CODENAME=bionic\n"
		"DISTRIB_DESCRIPTION=\"Ubuntu 18.04.4 LTS\"\n"
		"DISTRIB_ID=Ubuntu\n"
		"DISTRIB_RELEASE=18.04\n"
		"TABBED_VAR=\"Normal tabbed var\"\n"
		"VAR_WITHOUT_EOL=zxcvbnm\n"
		"WO_QUOTES_VAR=qwerty\n"
		"WO_QUOTES_VAR2=qwerty\n"
		"\"test space\"=\"lk lk lk \"\n"
	;

	faux_ini_t *ini = NULL;
	faux_ini_node_t *iter = NULL;
	const faux_pair_t *pair = NULL;
	const char *src_fn = "/tmp/src12";
	const char *dst_fn = "/tmp/dst12";
	const char *etalon_fn = "/tmp/etalon12";
	unsigned num_entries = 0;
	ssize_t r = 0;

	// Prepare files
	r = faux_testc_file_deploy(src_fn, src_file);
	if (r < 0) {
		fprintf(stderr, "Can't create test file %s\n", src_fn);
	}
	faux_testc_file_deploy(etalon_fn, etalon_file);

	ini = faux_ini_new();
	faux_ini_parse_file(ini, src_fn);
	iter = faux_ini_iter(ini);
	while ((pair = faux_ini_each(&iter))) {
		num_entries++;
		printf("[%s] = [%s]\n", faux_pair_name(pair), faux_pair_value(pair));
	}
	if (10 != num_entries) {
		fprintf(stderr, "Wrong number of entries %u\n", num_entries);
		faux_ini_free(ini);
		return -1;
	}

	faux_ini_set(ini, "test space", "lk lk lk ");
	faux_ini_write_file(ini, dst_fn);

	faux_ini_free(ini);

	return 0;

}
