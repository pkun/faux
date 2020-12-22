#include <stdlib.h>
#include <stdio.h>

#include "faux/str.h"
#include "faux/ini.h"
#include "faux/testc_helpers.h"


int testc_faux_ini_parse_file(void)
{
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

	int ret = -1; // Pessimistic return value
	faux_ini_t *ini = NULL;
	faux_ini_node_t *iter = NULL;
	const faux_pair_t *pair = NULL;
	char *src_fn = NULL;
	char *dst_fn = NULL;
	char *etalon_fn = NULL;

	// Prepare files
	src_fn = faux_testc_tmpfile_deploy(src_file);
	etalon_fn = faux_testc_tmpfile_deploy(etalon_file);
	dst_fn = faux_str_sprintf("%s/dst", getenv(FAUX_TESTC_TMPDIR_ENV));

	ini = faux_ini_new();
	if (!faux_ini_parse_file(ini, src_fn)) {
		fprintf(stderr, "Can't parse INI file %s\n", src_fn);
		goto parse_error;
	}

	iter = faux_ini_iter(ini);
	while ((pair = faux_ini_each(&iter))) {
		printf("[%s] = [%s]\n", faux_pair_name(pair), faux_pair_value(pair));
	}

	faux_ini_set(ini, "test space", "lk lk lk ");
	if (!faux_ini_write_file(ini, dst_fn)) {
		fprintf(stderr, "Can't write INI file %s\n", dst_fn);
		goto parse_error;
	}

	if (faux_testc_file_cmp(dst_fn, etalon_fn) != 0) {
		fprintf(stderr, "Generated file %s is not equal to etalon %s\n",
		dst_fn, etalon_fn);
		goto parse_error;
	}

	ret = 0; // success

parse_error:
	faux_ini_free(ini);
	faux_str_free(dst_fn);
	faux_str_free(src_fn);
	faux_str_free(etalon_fn);

	return ret;
}
