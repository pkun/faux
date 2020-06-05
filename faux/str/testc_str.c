#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "faux/str.h"


int testc_faux_str_nextword(void)
{
	const char* line = "asd\"\\\"\"mmm  `ll\"l\\p\\\\m```j`j`` ```kk``pp``` ll\\ l  \"aaa\"bbb`ccc```ddd``eee ``lk\\\"";
	const char* etalon[] = {
		"asd\"mmm",
		"ll\"l\\p\\\\mj`j",
		"kk``pp",
		"ll l",
		"aaabbbcccdddeee",
		"lk\\\"", // Unclosed quotes
		NULL
		 };
	int retval = 0;
	int i = 0;
	const char *saveptr = line;

	printf("Line   : [%s]\n", line);

	for (i = 0; etalon[i]; i++) {
		int r = -1;
		char *res = NULL;
		printf("Etalon %d : [%s]\n", i, etalon[i]);
		res = faux_str_nextword(saveptr, &saveptr, "`");
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

	return retval;
}
