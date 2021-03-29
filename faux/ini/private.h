#include "faux/faux.h"
#include "faux/list.h"
#include "faux/ini.h"

struct faux_pair_s {
	char *name;
	char *value;
};

struct faux_ini_s {
	faux_list_t *list;
};

C_DECL_BEGIN

FAUX_HIDDEN faux_pair_t *faux_pair_new(const char *name, const char *value);
FAUX_HIDDEN void faux_pair_free(void *pair);

FAUX_HIDDEN const char *faux_pair_name(const faux_pair_t *pair);
FAUX_HIDDEN void faux_pair_set_name(faux_pair_t *pair, const char *name);
FAUX_HIDDEN const char *faux_pair_value(const faux_pair_t *pair);
FAUX_HIDDEN void faux_pair_set_value(faux_pair_t *pair, const char *value);

C_DECL_END
