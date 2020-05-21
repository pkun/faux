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
#include <sys/stat.h>
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

#include "faux/faux.h"
#include "faux/str.h"
#include "faux/list.h"
#include "faux/testc_helpers.h"

#ifndef VERSION
#define VERSION 1.0.0
#endif
#define QUOTE(t) #t
#define version(v) printf("%s\n", v)

// Version of testc API (not version of programm)
#define TESTC_VERSION_MAJOR_DEFAULT 1
#define TESTC_VERSION_MINOR_DEFAULT 0
#define SYM_TESTC_VERSION_MAJOR "testc_version_major"
#define SYM_TESTC_VERSION_MINOR "testc_version_minor"
#define SYM_TESTC_MODULE "testc_module"

#define CHUNK_SIZE 1024
#define TEST_OUTPUT_LIMIT 1024 * CHUNK_SIZE

// Command line options */
struct opts_s {
	bool_t debug;
	bool_t preserve_tmp;
	faux_list_t *so_list;
};

typedef struct opts_s opts_t;

static opts_t *opts_parse(int argc, char *argv[]);
static void opts_free(opts_t *opts);
static void help(int status, const char *argv0);
static int exec_test(int (*test_sym)(void), faux_list_t **buf_list);
static void print_test_output(faux_list_t *buf_list);


int main(int argc, char *argv[]) {

	opts_t *opts = NULL; // Command line options
	faux_list_node_t *iter = NULL;
	char *so = NULL; // Shared object name

	// Return value will be negative on any error or failed test.
	// It doesn't mean that any error will break the processing.
	// The following vars are error statistics.
	unsigned int total_modules = 0; // Number of processed shared objects
	unsigned int total_broken_modules = 0; // Module processing errors
	unsigned int total_tests = 0; // Total number of tests
	unsigned int total_broken_tests = 0; // Something is wrong with test
	unsigned int total_failed_tests = 0; // Total number of failed tests
	unsigned int total_interrupted_tests = 0; // Number of interrupted tests
	unsigned int total_errors = 0; // Sum of all errors

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
	iter = faux_list_head(opts->so_list);
	while ((so = faux_list_each(&iter))) {

		void *so_handle = NULL;
		char testc_tmpdir[] = "/tmp/testc-XXXXXX";

		// Module symbols
		unsigned char testc_version_major = TESTC_VERSION_MAJOR_DEFAULT;
		unsigned char testc_version_minor = TESTC_VERSION_MINOR_DEFAULT;
		unsigned char *testc_version = NULL;
		const char *(*testc_module)[2] = NULL;

		// Module statistics
		unsigned int module_tests = 0;
		unsigned int module_broken_tests = 0;
		unsigned int module_failed_tests = 0;
		unsigned int module_interrupted_tests = 0;
		unsigned int module_errors = 0; // Sum of all errors

		total_modules++; // Statistics
		printf("--------------------------------------------------------------------------------\n");

		// Open shared object
		so_handle = dlopen(so, RTLD_LAZY | RTLD_LOCAL);
		if (!so_handle) {
			fprintf(stderr, "Error: "
				"Can't open module \"%s\"... "
				"Skipped\n", so);
			total_broken_modules++; // Statistics
			continue;
		}

		// Get testc API version from module
		testc_version = dlsym(so_handle, SYM_TESTC_VERSION_MAJOR);
		if (!testc_version) {
			fprintf(stderr, "Warning: "
				"Can't get API version for module \"%s\"... "
				"Use defaults\n", so);
		} else {
			testc_version_major = *testc_version;
			testc_version = dlsym(so_handle,
				SYM_TESTC_VERSION_MINOR);
			if (!testc_version) {
				fprintf(stderr, "Warning: "
					"Can't get API minor version"
					" for module \"%s\"... "
					"Use '0'\n", so);
				testc_version_minor = 0;
			} else {
				testc_version_minor = *testc_version;
			}
		}
		if ((testc_version_major > TESTC_VERSION_MAJOR_DEFAULT) ||
			((testc_version_major == TESTC_VERSION_MAJOR_DEFAULT) &&
			(testc_version_minor >TESTC_VERSION_MINOR_DEFAULT))) {
			fprintf(stderr, "Error: "
				"Unsupported API v%u.%u for module \"%s\"... "
				"Skipped\n",
				testc_version_major, testc_version_minor, so);
			total_broken_modules++; // Statistics
			continue;
		}

		// Get testing functions list from module
		testc_module = dlsym(so_handle, SYM_TESTC_MODULE);
		if (!testc_module) {
			fprintf(stderr, "Error: "
				"Can't get test list for module \"%s\"... "
				"Skipped\n", so);
			total_broken_modules++; // Statistics
			continue;
		}

		printf("Processing module \"%s\" v%u.%u ...\n", so,
			testc_version_major, testc_version_minor);

		// Create tmpdir for current shared object
		if (!mkdtemp(testc_tmpdir)) {
			fprintf(stderr, "Warning: "
				"Can't create tmp dir for module \"%s\"... "
				"Ignored\n", so);
		}
		if (opts->preserve_tmp) {
			fprintf(stderr, "Warning: "
				"Temp dir \"%s\" will be preserved\n",
				testc_tmpdir);
		}

		// Iterate through testing functions list
		while ((*testc_module)[0]) {

			const char *test_name = NULL;
			const char *test_desc = NULL;
			int (*test_sym)(void);
			int wstatus = 0; // Test's retval
			char *result_str = NULL;
			char *attention_str = NULL;
			faux_list_t *buf_list = NULL;
			char *tmpdir = NULL; // tmp dir for current test

			// Get name and description of testing function
			test_name = (*testc_module)[0];
			test_desc = (*testc_module)[1];
			if (!test_desc) // Description can be NULL
				test_desc = "";
			testc_module++; // Next test

			module_tests++; // Statistics

			// Get address of testing function by symbol name
			test_sym = (int (*)(void))dlsym(so_handle, test_name);
			if (!test_sym) {
				fprintf(stderr, "Error: "
					"Can't find symbol \"%s\"... "
					"Skipped\n", test_name);
				module_broken_tests++; // Statistics
				continue;
			}

			// Create tmp dir for current test and set TESTC_TMPDIR
			tmpdir = faux_str_sprintf("%s/test%03u",
				testc_tmpdir, module_tests);
			if (tmpdir) {
				if (mkdir(tmpdir, 0755) < 0) {
					fprintf(stderr, "Warning: "
						"Can't create temp dir \"%s\": %s\n",
						tmpdir, strerror(errno));
				}
				setenv(FAUX_TESTC_TMPDIR_ENV, tmpdir, 1);
			} else {
				fprintf(stderr, "Warning: "
					"Can't generate name for temp dir\n");
			}

			// Execute testing function
			wstatus = exec_test(test_sym, &buf_list);

			// Analyze testing function return code

			// Normal exit
			if (WIFEXITED(wstatus)) {

				// Success
				if (WEXITSTATUS(wstatus) == 0) {
					result_str = faux_str_dup("success");
					attention_str = faux_str_dup("");

				// Failed
				} else {
					result_str = faux_str_sprintf(
						"failed (%d)",
						(int)((signed char)((unsigned char)WEXITSTATUS(wstatus))));
					attention_str = faux_str_dup("(!) ");
					module_failed_tests++; // Statistics
				}

			// Terminated by signal
			} else if (WIFSIGNALED(wstatus)) {
				result_str = faux_str_sprintf("terminated (%d)",
					WTERMSIG(wstatus));
				attention_str = faux_str_dup("[!] ");
				module_interrupted_tests++; // Statistics

			// Stopped by unknown conditions
			} else {
				result_str = faux_str_dup("unknown");
				attention_str = faux_str_dup("[!] ");
				module_broken_tests++; // Statistics
			}

			// Print test execution report
			printf("%sTest #%03u %s() %s: %s\n",
				attention_str, module_tests,
				test_name, test_desc, result_str);
			faux_str_free(result_str);
			faux_str_free(attention_str);

			// Print test output if error or debug
			if (!WIFEXITED(wstatus) ||
				WEXITSTATUS(wstatus) != 0 ||
				opts->debug) {
				if (opts->preserve_tmp) {
					fprintf(stderr, "Info: "
						"Test's temp dir is \"%s\"\n",
						tmpdir);
				}
				if (faux_list_len(buf_list) > 0)
					printf("~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ \n");
				print_test_output(buf_list);
				if (faux_list_len(buf_list) > 0)
					printf("~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ \n");
			}
			faux_list_free(buf_list);

			// Remove test's tmp dir
			if (!opts->preserve_tmp)
				faux_rm(tmpdir);
			faux_str_free(tmpdir);
		}

		dlclose(so_handle);
		so_handle = NULL;

		// Remove module's tmp dir
		if (!opts->preserve_tmp)
			faux_rm(testc_tmpdir);

		// Report module statistics
		printf("Module tests: %u\n", module_tests);
		printf("Module broken tests: %u\n", module_broken_tests);
		printf("Module failed tests: %u\n", module_failed_tests);
		printf("Module interrupted tests: %u\n", module_interrupted_tests);
		module_errors =
			module_broken_tests +
			module_failed_tests +
			module_interrupted_tests;
		printf("Module errors: %u\n", module_errors);

		// Gather total statistics
		total_tests += module_tests;
		total_broken_tests += module_broken_tests;
		total_failed_tests += module_failed_tests;
		total_interrupted_tests += module_interrupted_tests;

	}

	opts_free(opts);

	// Report total statistics
	printf("================================================================================\n");
	printf("Total modules: %u\n", total_modules);
	printf("Total broken modules: %u\n", total_broken_modules);
	printf("Total tests: %u\n", total_tests);
	printf("Total broken_tests: %u\n", total_broken_tests);
	printf("Total failed tests: %u\n", total_failed_tests);
	printf("Total interrupted tests: %u\n", total_interrupted_tests);
	total_errors =
		total_broken_modules +
		total_broken_tests +
		total_failed_tests +
		total_interrupted_tests;
	printf("Total errors: %u\n", total_errors);

	if (total_errors > 0)
		return -1;

	return 0;
}


static void free_iov(struct iovec *iov) {

	faux_free(iov->iov_base);
	faux_free(iov);
}


static faux_list_t *read_test_output(int fd, size_t limit) {

	struct iovec *iov = NULL;
	size_t total_len = 0;
	faux_list_t *buf_list = NULL; // Buffer list

	buf_list = faux_list_new(
		FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, (void (*)(void *))free_iov);

	do {
		ssize_t bytes_readed = 0;

		iov = faux_zmalloc(sizeof(*iov));
		assert(iov);
		iov->iov_len = CHUNK_SIZE;
		iov->iov_base = faux_malloc(iov->iov_len);
		assert(iov->iov_base);

		do {
			bytes_readed = readv(fd, iov, 1);
		} while ((bytes_readed < 0) && (errno == EINTR));
		if (bytes_readed <= 0) { /* Error or EOF */
			free_iov(iov);
			break;
		}

		iov->iov_len = bytes_readed;
		faux_list_add(buf_list, iov);
		total_len += iov->iov_len;

	} while (total_len < limit);

	return buf_list;
}


static void print_test_output(faux_list_t *buf_list) {

	faux_list_node_t *iter = NULL;
	struct iovec *iov = NULL;

	iter = faux_list_head(buf_list);
	while ((iov = faux_list_each(&iter))) {
		faux_write_block(STDOUT_FILENO, iov->iov_base, iov->iov_len);
	}
}

/** Executes testing function
 *
 * Function fork() and executes testing function.
 *
 * @param [in] test_sym Testing function.
 * @param [in] buf_list
 * @return Testing function return value
 */
static int exec_test(int (*test_sym)(void), faux_list_t **buf_list) {

	pid_t pid = -1;
	int wstatus = -1;
	int pipefd[2];
	faux_list_t *blist = NULL;

	if (pipe(pipefd))
		return -1;

	pid = fork();
	assert(pid != -1);
	if (pid == -1)
		return -1;

	// Child
	if (pid == 0) {
		dup2(pipefd[1], 1);
		dup2(pipefd[1], 2);
		close(pipefd[0]);
		close(pipefd[1]);
		_exit(test_sym());
	}

	// Parent
	close(pipefd[1]);
	blist = read_test_output(pipefd[0], TEST_OUTPUT_LIMIT);
	// The pipe closing can lead to test interruption when output length
	// limit is exceeded. But it's ok because it saves us from iternal
	// loops. It doesn't saves from silent iternal loops.
	close(pipefd[0]);
	if (blist)
		*buf_list = blist;

	while (waitpid(pid, &wstatus, 0) != pid);

	return wstatus;
}


/** @brief Frees allocated opts_t structure
 *
 * @param [in] opts Allocated opts_t structure.
 */
static void opts_free(opts_t *opts) {

	assert(opts);
	if (!opts)
		return;

	faux_list_free(opts->so_list);
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
	opts->preserve_tmp = BOOL_FALSE;

	// Members of list are static strings from argv so don't free() it
	opts->so_list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_UNIQUE,
		(faux_list_cmp_fn)strcmp, NULL, NULL);
	if (!opts->so_list) {
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

	static const char *shortopts = "hvdt";
#ifdef HAVE_GETOPT_LONG
	static const struct option longopts[] = {
		{"help",		0, NULL, 'h'},
		{"version",		0, NULL, 'v'},
		{"debug",		0, NULL, 'd'},
		{"preserve-tmp",	0, NULL, 't'},
		{NULL,			0, NULL, 0}
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
			opts->preserve_tmp = BOOL_TRUE;
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
			faux_list_add(opts->so_list, argv[i]);
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
		printf("Usage: %s [options] <so_object> [so_object] ...\n", name);
		printf("Unit test helper for C code.\n");
		printf("Options:\n");
		printf("\t-v, --version\tPrint version.\n");
		printf("\t-h, --help\tPrint this help.\n");
		printf("\t-d, --debug\tDebug mode. Show output for all tests.\n");
		printf("\t-t, --preserve-tmp\tPreserve test's tmp files.\n");
	}
}
