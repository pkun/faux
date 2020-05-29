#include "faux/faux.h"
#include "faux/list.h"
#include "faux/argv.h"

struct faux_argv_s {
	faux_list_t *list;
	char *quotes; // List of possible quotes chars
};
