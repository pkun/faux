#include "faux/faux.h"
#include "faux/list.h"
#include "faux/error.h"

struct faux_error_s {
	faux_list_t *list;
};
