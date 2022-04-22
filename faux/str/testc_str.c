#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "faux/str.h"


int testc_faux_str_nextword(void)
{
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
	const char *saveptr = line;
	bool_t closed_quotes = BOOL_FALSE;

	printf("Line   : [%s]\n", line);

	for (i = 0; etalon[i]; i++) {
		int r = -1;
		char *res = NULL;
		printf("Etalon %d : [%s]\n", i, etalon[i]);
		res = faux_str_nextword(saveptr, &saveptr, "`", &closed_quotes);
		if (!res) {
			printf("The faux_str_nextword() return value is NULL\n");
			break;
		} else {
			printf("Result %d : [%s]\n", i, res);
		}
		r = strcmp(etalon[i], res);
		if (r < 0) {
			printf("Not equal %d\n", i);
			retval = -1;
		}
		faux_str_free(res);
	}
	// Last quote is unclosed
	if (closed_quotes) {
		printf("Closed quotes flag is wrong\n");
		retval = -1;
	} else {
		printf("Really unclosed quotes\n");
	}

	return retval;
}


int testc_faux_str_getline(void)
{
	const char* line = "arg 0\narg 1\narg 2";
	const char* etalon[] = {
		"arg 0",
		"arg 1",
		"arg 2",
		NULL
		 };
	ssize_t num_etalon = 3;
	size_t index = 0;
	char *str = NULL;
	const char *saveptr = NULL;

	printf("Line   : [%s]\n", line);

	saveptr = line;
	while ((str = faux_str_getline(saveptr, &saveptr)) && (index < num_etalon)) {
		int r = -1;
		const char *res = NULL;
		printf("Etalon %ld : [%s]\n", index, etalon[index]);
		r = strcmp(etalon[index], str);
		if (r < 0) {
			printf("Not equal %ld [%s]\n", index, str);
			return -1;
		}
		faux_str_free(str);
		index++;
	}
	if (index != num_etalon) {
		printf("Number of args is not equal real=%ld etalon=%ld\n", index, num_etalon);
		return -1;
	}

	return 0;
}
