/*
 * command.h
 */

#include "clish/command.h"

struct clish_command_s {
	lub_bintree_node_t bt_node;
	char *name;
	char *text;
	clish_paramv_t *paramv;
	clish_action_t *action;
	clish_config_t *config;
	char *viewname;
	char *viewid;
	char *detail;
	char *escape_chars;
	char *regex_chars;
	char *access;
	clish_param_t *args;
	const struct clish_command_s *link;
	char *alias_view;
	char *alias;
	clish_view_t *pview;
#ifdef LEGACY
	bool_t lock;
	bool_t interrupt;
#endif
	bool_t dynamic; /* Is command dynamically created */
	bool_t internal; /* Is command internal? Like the "startup" */
};
