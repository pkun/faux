#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <errno.h>

#if WITH_INTERNAL_GETOPT
#include "libc/getopt.h"
#else
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#endif

#if HAVE_LOCALE_H
#include <locale.h>
#endif
#if HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

#include <faux/faux.h>
#include <faux/str.h>
#include <faux/list.h>
#include <faux/file.h>

#ifndef VERSION
#define VERSION 1.0.0
#endif
#define QUOTE(t) #t
#define version(v) printf("%s\n", v)

// Command line options */
struct opts_s {
	bool_t debug;
	bool_t binary;
	faux_list_t *file_list;
};

typedef struct opts_s opts_t;

static opts_t *opts_parse(int argc, char *argv[]);
static void opts_free(opts_t *opts);
static void help(int status, const char *argv0);


int main(int argc, char *argv[]) {

	opts_t *opts = NULL; // Command line options
	faux_list_node_t *iter = NULL;
	char *fn = NULL; // Text file
	unsigned int total_errors = 0; // Sum of all errors
	unsigned int file_num = 0; // Number of file


#if HAVE_LOCALE_H
	// Set current locale
	setlocale(LC_ALL, "");
#endif

	// Parse command line options
	opts = opts_parse(argc, argv);
	if (!opts) {
		fprintf(stderr, "Error: Can't parse command line options\n");
		return -1;
	}

	// Main loop. Iterate through the list of shared objects
	iter = faux_list_head(opts->file_list);
	while ((fn = faux_list_each(&iter))) {
		faux_file_t *f = NULL;
		char *buf = NULL;
		bool_t eof = BOOL_FALSE;
		unsigned int line_num = 0;

		file_num++;
		f = faux_file_open(fn, O_RDONLY, 0);
		if (!f) {
			fprintf(stderr, "Error: Can't open file \"%s\"\n", fn);
			total_errors++;
			continue;
		}
		printf("\n");
		printf("// File \"%s\"\n", fn);
		printf("const char *txt%u =\n", file_num);

		while ((buf = faux_file_getline_raw(f))) {
			char *escaped_str = NULL;
			line_num++;
			escaped_str = faux_str_c_esc(buf);
			faux_str_free(buf);
			if (escaped_str)
				printf("\t\"%s\"\n", escaped_str);
			faux_str_free(escaped_str);
		}
		eof = faux_file_eof(f);
		if (!eof) { // File reading was interrupted before EOF
			fprintf(stderr, "Error: File \"%s\" reading was "
				"interrupted before EOF\n", fn);
			total_errors++;
		} // Continue normal operations

		if (0 == line_num) // Empty file is not error
				printf("\t\"\"\n");
		printf(";\n");
		faux_file_close(f);
	}

	opts_free(opts);

	if (total_errors > 0)
		return -1;

	return 0;
}


/** @brief Frees allocated opts_t structure
 *
 * @param [in] opts Allocated opts_t structure.
 */
static void opts_free(opts_t *opts) {

	assert(opts);
	if (!opts)
		return;

	faux_list_free(opts->file_list);
	faux_free(opts);
}


/** @brief Allocates new opts_t structure
 *
 * Allocates structure that stores parse command line options.
 *
 * @return Allocated and initialized opts_t structure.
 * @warning The returned opts_t structure must be freed later by opts_free().
 */
static opts_t *opts_new(void) {

	opts_t *opts = NULL;

	opts = faux_zmalloc(sizeof(*opts));
	assert(opts);
	if (!opts)
		return NULL;

	opts->debug = BOOL_FALSE;
	opts->binary = BOOL_FALSE;

	// Members of list are static strings from argv so don't free() it
	opts->file_list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_UNIQUE,
		(faux_list_cmp_fn)strcmp, NULL, NULL);
	if (!opts->file_list) {
		opts_free(opts);
		return NULL;
	}

	return opts;
}


/** @brief Parse command line options
 *
 * Function allocates opts_t structure, parses command line options and
 * fills opts_t structure.
 *
 * @param [in] argc Standard argc argument.
 * @param [in] argv Standard argv argument.
 * @return Filled opts_t structure with parsed command line options.
 * @warning The returned opts_t structure must be freed later by opts_free().
 */
static opts_t *opts_parse(int argc, char *argv[]) {

	opts_t *opts = NULL;

	static const char *shortopts = "hvdbt";
#ifdef HAVE_GETOPT_LONG
	static const struct option longopts[] = {
		{"help",	0, NULL, 'h'},
		{"version",	0, NULL, 'v'},
		{"debug",	0, NULL, 'd'},
		{"text",	0, NULL, 't'},
		{"binary",	0, NULL, 'b'},
		{NULL,		0, NULL, 0}
	};
#endif

	opts = opts_new();
	if (!opts)
		return NULL;

	optind = 1;
	while (1) {
		int opt;
#ifdef HAVE_GETOPT_LONG
		opt = getopt_long(argc, argv, shortopts, longopts, NULL);
#else
		opt = getopt(argc, argv, shortopts);
#endif
		if (-1 == opt)
			break;
		switch (opt) {
		case 'd':
			opts->debug = BOOL_TRUE;
			break;
		case 't':
			opts->binary = BOOL_FALSE;
			break;
		case 'b':
			opts->binary = BOOL_TRUE;
			break;
		case 'h':
			help(0, argv[0]);
			exit(0);
			break;
		case 'v':
			version(VERSION);
			exit(0);
			break;
		default:
			help(-1, argv[0]);
			exit(-1);
			break;
		}
	}

	if (optind < argc) {
		int i = 0;
		for (i = optind; i < argc; i++)
			faux_list_add(opts->file_list, argv[i]);
	} else {
		help(-1, argv[0]);
		exit(-1);
	}

	return opts;
}


/** @brief Prints help
 *
 * @param [in] status If status is not '0' consider help printing as a reaction
 * to error and print appropriate message. If status is '0' then print general
 * help information.
 * @param [in] argv0 The argv[0] argument i.e. programm name
 */
static void help(int status, const char *argv0) {

	const char *name = NULL;

	if (!argv0)
		return;

	// Find the basename
	name = strrchr(argv0, '/');
	if (name)
		name++;
	else
		name = argv0;

	if (status != 0) {
		fprintf(stderr, "Try `%s -h' for more information.\n",
			name);
	} else {
		printf("Usage: %s [options] <txt_file> [txt_file] ...\n", name);
		printf("Converts text/binary files to C-strings.\n");
		printf("Options:\n");
		printf("\t-v, --version\tPrint version.\n");
		printf("\t-h, --help\tPrint this help.\n");
		printf("\t-d, --debug\tDebug mode.\n");
		printf("\t-t, --text\tText mode conversion (Default).\n");
		printf("\t-d, --debug\tBinary mode conversion.\n");
	}
}
