#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "faux/argv.h"


int testc_faux_argv_parse(void)
{
	faux_argv_t *fargv = NULL;
	const char* line = "asd\"\\\"\"mmm \"``\" `ll\"l\\p\\\\m```j`j`` ```kk``pp``` ll\\ l jj\\\"kk ll\\\\nn  \"aaa\"bbb`ccc```ddd``eee ``lk\\\"";
	const char* etalon[] = {
		"asd\"mmm",
		"``",
		"ll\"l\\p\\\\mj`j",
		"kk``pp",
		"ll l",
		"jj\"kk",
		"ll\\nn",
		"aaabbbcccdddeee",
		"lk\\\"", // Unclosed quotes
		NULL
		 };
	int retval = 0;
	int i = 0;
	ssize_t num = 0;
	ssize_t num_etalon = 9;
	faux_argv_node_t *iter = NULL;

	printf("Line   : [%s]\n", line);

	fargv = faux_argv_new();
	faux_argv_quotes(fargv, "`");
	num = faux_argv_parse(fargv, line);
	if (num < 0) {
		printf("Error: Can't parse line\n");
		faux_argv_free(fargv);
		return -1;
	}
	if (num != num_etalon) {
		printf("Error: Wrong argument number\n");
		printf("Real number : %ld\n", num);
		printf("Etalon number : %ld\n", num_etalon);
		faux_argv_free(fargv);
		retval = -1;
	}

	iter = faux_argv_iter(fargv);
	for (i = 0; etalon[i]; i++) {
		int r = -1;
		const char *res = NULL;
		printf("Etalon %d : [%s]\n", i, etalon[i]);
		res = faux_argv_each(&iter);
		if (!res) {
			printf("The faux_argv_each() return value is NULL\n");
			break;
		} else {
			printf("Result %d : [%s]\n", i, res);
		}
		r = strcmp(etalon[i], res);
		if (r < 0) {
			printf("Not equal %d\n", i);
			retval = -1;
		}
	}
	// Last quote is unclosed
	if (!faux_argv_is_continuable(fargv)) {
		printf("Continuable flag is wrong\n");
		retval = -1;
	} else {
		printf("Continuable flag is on\n");
	}

	return retval;
}

int testc_faux_argv_is_continuable(void)
{
	faux_argv_t *fargv = NULL;
	const char* line = "asd\"\\\"\"mmm \"``\" `ll\"l\\p\\\\m```j`j`` ```kk``pp``` ll\\ l jj\\\"kk ll\\\\nn  \"aaa\"bbb`ccc```ddd``eee ``lk\\\" ";
	int retval = 0;
	ssize_t num = 0;

	printf("Line   : [%s]\n", line);

	fargv = faux_argv_new();
	faux_argv_quotes(fargv, "`");
	num = faux_argv_parse(fargv, line);
	if (num < 0) {
		printf("Error: Can't parse line\n");
		faux_argv_free(fargv);
		return -1;
	}
	// Not continuable
	if (faux_argv_is_continuable(fargv)) {
		printf("Continuable flag is wrong\n");
		retval = -1;
	} else {
		printf("Line is not continuable\n");
	}

	return retval;
}
