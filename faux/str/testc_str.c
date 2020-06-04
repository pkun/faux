#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "faux/str.h"


int testc_faux_str_nextword(void)
{
	const char* line = "asd\"\\\"\"mmm   lll";
	const char* etalon = "asd\"mmm";
	char *res = NULL;
	int retval = -1;

	printf("Line   : [%s]\n", line);
	printf("Etalon : [%s]\n", etalon);

	res = faux_str_nextword(line, NULL, NULL);
	if (!res)
		printf("The faux_str_nextword() return value is NULL\n");
	else
		printf("Result : [%s]\n", res);
	retval = strcmp(etalon, res);
	faux_str_free(res);

	return retval;
}
