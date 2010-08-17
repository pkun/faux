/*
 * conf.c
 *
 * This file provides the implementation of a conf class
 */
#include "private.h"
#include "lub/argv.h"
#include "lub/string.h"
#include "lub/ctype.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*---------------------------------------------------------
 * PRIVATE META FUNCTIONS
 *--------------------------------------------------------- */
int konf_tree_bt_compare(const void *clientnode, const void *clientkey)
{
	const konf_tree_t *this = clientnode;
	unsigned short *pri = (unsigned short *)clientkey;
	char *line = ((char *)clientkey + sizeof(unsigned short));

/*	printf("COMPARE: node_pri=%d node_line=[%s] key_pri=%d key_line=[%s]\n",
	konf_tree__get_priority(this), this->line, *pri, line);
*/
	if (konf_tree__get_priority(this) == *pri)
		return lub_string_nocasecmp(this->line, line);

	return (konf_tree__get_priority(this) - *pri);
}

/*-------------------------------------------------------- */
static void konf_tree_key(lub_bintree_key_t * key,
	unsigned short priority, const char *text)
{
	unsigned short *pri = (unsigned short *)key;
	char *line = ((char *)key + sizeof(unsigned short));

	/* fill out the opaque key */
	*pri = priority;
	strcpy(line, text);
}

/*-------------------------------------------------------- */
void konf_tree_bt_getkey(const void *clientnode, lub_bintree_key_t * key)
{
	const konf_tree_t *this = clientnode;

	konf_tree_key(key, konf_tree__get_priority(this), this->line);
}

/*---------------------------------------------------------
 * PRIVATE METHODS
 *--------------------------------------------------------- */
static void
konf_tree_init(konf_tree_t * this, const char *line, unsigned short priority)
{
	/* set up defaults */
	this->line = lub_string_dup(line);
	this->priority = priority;
	this->splitter = BOOL_TRUE;

	/* Be a good binary tree citizen */
	lub_bintree_node_init(&this->bt_node);

	/* initialise the tree of commands for this conf */
	lub_bintree_init(&this->tree,
		konf_tree_bt_offset(),
		 konf_tree_bt_compare, konf_tree_bt_getkey);
}

/*--------------------------------------------------------- */
static void konf_tree_fini(konf_tree_t * this)
{
	konf_tree_t *conf;

	/* delete each conf held by this conf */
	while ((conf = lub_bintree_findfirst(&this->tree))) {
		/* remove the conf from the tree */
		lub_bintree_remove(&this->tree, conf);
		/* release the instance */
		konf_tree_delete(conf);
	}

	/* free our memory */
	lub_string_free(this->line);
	this->line = NULL;
}

/*---------------------------------------------------------
 * PUBLIC META FUNCTIONS
 *--------------------------------------------------------- */
size_t konf_tree_bt_offset(void)
{
	return offsetof(konf_tree_t, bt_node);
}

/*--------------------------------------------------------- */
konf_tree_t *konf_tree_new(const char *line, unsigned short priority)
{
	konf_tree_t *this = malloc(sizeof(konf_tree_t));

	if (this) {
		konf_tree_init(this, line, priority);
	}

	return this;
}

/*---------------------------------------------------------
 * PUBLIC METHODS
 *--------------------------------------------------------- */
void konf_tree_delete(konf_tree_t * this)
{
	konf_tree_fini(this);
	free(this);
}

void konf_tree_fprintf(konf_tree_t * this, FILE * stream,
		const char *pattern, int depth,
		unsigned char prev_pri_hi)
{
	konf_tree_t *conf;
	lub_bintree_iterator_t iter;
	unsigned char pri = 0;

	if (this->line && *(this->line) != '\0') {
		char *space = NULL;

		if (depth > 0) {
			space = malloc(depth + 1);
			memset(space, ' ', depth);
			space[depth] = '\0';
		}
		if ((0 == depth) &&
			(this->splitter ||
			(konf_tree__get_priority_hi(this) != prev_pri_hi)))
			fprintf(stream, "!\n");
		fprintf(stream, "%s%s\n", space ? space : "", this->line);
		free(space);
	}

	/* iterate child elements */
	if (!(conf = lub_bintree_findfirst(&this->tree)))
		return;

	for(lub_bintree_iterator_init(&iter, &this->tree, conf);
		conf; conf = lub_bintree_iterator_next(&iter)) {
		if (pattern &&
			(lub_string_nocasestr(conf->line, pattern) != conf->line))
			continue;
		konf_tree_fprintf(conf, stream, NULL, depth + 1, pri);
		pri = konf_tree__get_priority_hi(conf);
	}
}

/*--------------------------------------------------------- */
konf_tree_t *konf_tree_new_conf(konf_tree_t * this,
					const char *line, unsigned short priority)
{
	/* allocate the memory for a new child element */
	konf_tree_t *conf = konf_tree_new(line, priority);
	assert(conf);

	/* ...insert it into the binary tree for this conf */
	if (-1 == lub_bintree_insert(&this->tree, conf)) {
		/* inserting a duplicate command is bad */
		konf_tree_delete(conf);
		conf = NULL;
	}

	return conf;
}

/*--------------------------------------------------------- */
konf_tree_t *konf_tree_find_conf(konf_tree_t * this,
	const char *line, unsigned short priority)
{
	konf_tree_t *conf;
	lub_bintree_key_t key;
	lub_bintree_iterator_t iter;

	if (0 != priority) {
		konf_tree_key(&key, priority, line);
		return lub_bintree_find(&this->tree, &key);
	}

	/* If tree is empty */
	if (!(conf = lub_bintree_findfirst(&this->tree)))
		return NULL;

	/* Iterate non-empty tree */
	lub_bintree_iterator_init(&iter, &this->tree, conf);
	do {
		if (0 == lub_string_nocasecmp(conf->line, line))
			return conf;
	} while ((conf = lub_bintree_iterator_next(&iter)));

	return NULL;
}

/*--------------------------------------------------------- */
void konf_tree_del_pattern(konf_tree_t *this,
	const char *pattern)
{
	konf_tree_t *conf;
	lub_bintree_iterator_t iter;

	/* Empty tree */
	if (!(conf = lub_bintree_findfirst(&this->tree)))
		return;

	lub_bintree_iterator_init(&iter, &this->tree, conf);
	do {
		if (lub_string_nocasestr(conf->line, pattern) == conf->line) {
			lub_bintree_remove(&this->tree, conf);
			konf_tree_delete(conf);
		}
	} while ((conf = lub_bintree_iterator_next(&iter)));
}

/*--------------------------------------------------------- */
unsigned short konf_tree__get_priority(const konf_tree_t * this)
{
	return this->priority;
}

/*--------------------------------------------------------- */
unsigned char konf_tree__get_priority_hi(const konf_tree_t * this)
{
	return (unsigned char)(this->priority >> 8);
}

/*--------------------------------------------------------- */
unsigned char konf_tree__get_priority_lo(const konf_tree_t * this)
{
	return (unsigned char)(this->priority & 0xff);
}

/*--------------------------------------------------------- */
bool_t konf_tree__get_splitter(const konf_tree_t * this)
{
	return this->splitter;
}

/*--------------------------------------------------------- */
void konf_tree__set_splitter(konf_tree_t *this, bool_t splitter)
{
	this->splitter = splitter;
}