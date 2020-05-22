---
title: Unit testing framework `testc`
author: Sergey Kalichev &lt;serj.kalichev(at)gmail.com&gt;
date: 2020
...


# About

The `testc` is a unit testing framework for code written in C. The `testc` is a part of `faux` project (library of aux functions). The utility executes a set of unit tests, gets a results (success/fail) and generates a report. Each test is a C-function. The source of tests is a shared object - executable binary or shared library. The tested shared object must contain the symbol with prefixed name. This symbol references the list of test functions. So the shared object can contain production code and testing code simultaneously. Also testing code can has its own shared object that linked to tested libraries.

The goal of `testc` framework is simplicity and integration of testing to the development process.


## Links

* GIT repository <https://src.libcode.org/pkun/faux>
* Download <http://libcode.org/projects/faux>
* Mailing list <http://groups.google.com/group/libfaux>


# The `testc` utility

The `testc` utility gets a list of need to be tested shared objects from command line, executes all the test functions from them. It generates a report while tests execution.

The utility executes fork() for each test. So broken test interrupted by signal can't break `testc` itself. The utility prints a return value for each test. It can be `success`, `failed`, `interrupted by signal`. For failed tests their output (stdout, stderr) is printed to report for debugging purposes. The successfull tests are silent.

The utility's return value will be `0` if all the tests are successful. Single failed test will lead to non-null return value.

The `testc` utility has no dependencies excluding standard `libc` library. This fact allows to use `LD_LIBRARY_PATH` environment variable to define path to tested shared objects and tests the software in place without installing it.

User can define tested files in command line using absolute path, relative path or library name only. If path is not specified then utility will search for the library within `LD_LIBRARY_PATH` and standard system paths.

There are examples of `testc` utility execution with defining relative path, absolute path, search within `LD_LIBRARY_PATH`, search within system paths accordingly.

```
$ testc ./.libs/libfaux.so.1.0.0
$ testc /home/pkun/faux/.libs/libfaux.so.1.0.0
$ testc libfaux.so
$ LD_LIBRARY_PATH=/home/pkun/faux/.libs testc libfaux.so
```


## Utility options

* `-v`, `--version` - Show utility version.
* `-h`, `--help` - Show help.
* `-d`, `--debug` - Show output for all tests. Not for failed tests only.
* `-t`, `--preserve-tmp` - Preserve test's temporary files. It's useful for debug purposes. Since version `faux-1.1.0`.



## Report example

```
$ LD_LIBRARY_PATH=.libs/ testc libfaux.so absent.so libsecond.so
--------------------------------------------------------------------------------
Processing module "libfaux.so" v1.0 ...
Test #001 testc_faux_ini_good() INI subsystem good: success
(!) Test #002 testc_faux_ini_bad() INI bad: failed (-1)
Some debug information here
[!] Test #003 testc_faux_ini_signal() Interrupted by signal: terminated (11)
Module tests: 3
Module errors: 2
--------------------------------------------------------------------------------
Error: Can't open module "absent.so"... Skipped
--------------------------------------------------------------------------------
Processing module "libsecond.so" v1.0 ...
Test #001 testc_faux_ini_good() INI subsystem good: success
(!) Test #002 testc_faux_ini_bad() INI bad: failed (-1)
Some debug information here
[!] Test #003 testc_faux_ini_signal() Interrupted by signal: terminated (11)
Module tests: 3
Module errors: 2
================================================================================
Total modules: 2
Total tests: 6
Total errors: 5
```


# How to write tests

The tests source code doesn't use any special headers and test binaries don't need to be linked to any special testing library. The only necessary thing is to define three special symbols within shared object and define testing functions accordingly to predefined prototype. These three special symbols are API version (major version byte, minor testing byte) and list of testing functions.

The testing function prototype is:

```
int testc_my_func(void) {
	...
}
```

The function name is arbitrary. It's recommended to start name with `testc_` to differ testing functions from the other ones.

The testing function returns `0` on success or any other value on error. The function can output any debug information to stdout or stderr. Only failed function's output will appear in report. The successful function is silent.


## Testing functions examples

The following functions are simple examples of tests.

The following function are always successful. If test needs any external data for processing the environment variables can help. This example prints nothing because it always returns `0` i.e. function is successful.

```
int testc_faux_ini_good(void) {

	char *path = NULL;

	path = getenv("FAUX_INI_PATH");
	if (path)
		printf("Env var is [%s]\n", path);
	return 0;
}
```

The following function is always failed. The report will contain the printed text string.

```
int testc_faux_ini_bad(void) {

	printf("Some debug information here\n");
	return -1;
}
```

The following function leads to `Segmentation fault` and test will be interrupted by signal.

```
int testc_faux_ini_signal(void) {

	char *p = NULL;

	printf("%s\n", p);
	return -1;
}

```

The corresponding report (execution of these three example functions) you can find in `Report example` section.


## API version

The testing function prototype can be changed in future and format of functions list can be changed too. The `testc` utility must know about it. For this purpose the tested object must contain the following special symbols:

```
const unsigned char testc_version_major = 1;
const unsigned char testc_version_minor = 0;
```

It's API version (major and minor parts). The module declares API version that it uses. The names of symbols is fixed. Now the single API version `1.0` exists but it can be changed in future. Frankly the API version declaration is not mandatory but it's strongly recommended. If version is not specified the `testc` utility will suggest the most modern API version.


## Testing functions list

Each testing function must be referenced from testing functions list. It allows `testc` utility to find all the tests. The symbol that contain a list of testing functions has a special name and type. It's an array of pairs of text strings.

```
const char *testc_module[][2] = {
	{"testc_faux_ini_good", "INI subsystem good"},
	{"testc_faux_ini_bad", "INI bad"},
	{"testc_faux_ini_signal", "Interrupted by signal"},
	{NULL, NULL}
	};
```

Each text string pair describes one testing function. The first text string is a name testing function. The `testc` utility will use this name to find corresponding symbol within shared object. The second text string is a description of the test. It will be used within report to identify tests.

The testing functions list must be terminated by mandatory NULL-pair `{NULL, NULL}`. Without it the `testc` utility doesn't know where the list ends.


## Ways to integrate tests


### Tests fully separated from production code

All the testing functions can be contained by separated shared library. This library can be separated from tested project.


### Tests in separate files

Testing function can be part of tested project but be isolated in testing-purpose files. These files can be compiled or not compiled depending on the build flags.

```
# Makefile.am
...
if TESTC
include $(top_srcdir)/testc_module/Makefile.am
endif
...
```


### Tests integrated into production files

The production source code files can contain testing functions. These functions can be compiled or not compiled depending on the build flags.

```
# Makefile.am
if TESTC
libfaux_la_CFLAGS = -DTESTC
endif
```

```
int foo(int y) {
...
}

#ifdef TESTC
int testc_foo(void) {
...
	if (foo(7)) ...
}
#endif
```

In this case testing function can test local static functions but not library public interface only.

## Temporary files

Some complex tests need files to work with. Such feature is available since `faux-1.1.0`. The `testc` utility creates temporary directory for each test, sets environment variable `TESTC_TMPDIR` to the name of this temporary directory. The testing function can get environment variable to find out its temporary directory. The test can use this directory in any way. The temporary dir is individual for each test so nobody creates file within this directory excluding test itself. The test doesn't need to care about duplicate file names. After test execution the `testc` utility removes temporary dir with all content. So the cleaning of temporary files is not mandatory task for test. To get temporary directory path use the following command:

```
const char *tmpdir = getenv("TESTC_TMPDIR");
```

Sometimes temporary files are needed for debugging purposes. Use `--preserve-tmp` flag while `testc` utility execution to preserve all temporary files. The names of temporary directories you can find out from test report. Temporary files can be removed manually later.


## The `testc` helpers library

The `testc_helpers` sub-library is a set of functions that help to write tests. The sub-library is a part of `faux` library. Available since `faux-1.1.0`. The using of `testc_helpers` library is not mandatory for test writing. It's not necessary to link tests to this library or to use `faux/testc_helpers.h` header. It's a simply helpers. The test developer can use library or not, as he prefer. The sub-library contains header, set of functions and service utilities.


### The `faux-file2c` utility

Some tests need files for processing. Them can be files with input data or etalon data to compare result with. It's not obvious where to store such files while the test function is a part of shared object (shared library or executable). The project's sourcecode tree can be inaccessible while test execution. One of the possible solutions is to embed data into C-code. The `faux-file2c` helps to embed data files into C-files.

The input parameters for `faux-file2c` utility are names of files needed to be embed to C-code. The output is a C-code fragment with `const char *` variables initialized with text-formatted content of data files. Utility has two modes - text mode and binary mode. The text mode is for text files. The file is embedded to C-code line-by-line. Special characters are in a form of hex codes, but widely-used special characters are escaped only and printable symbols preserve simple forms. So such representation in C-code is readable by human. In binary mode all the bytes are converted to hex codes so it's not readable. See the following examples.

```
$ cat tmpfile
# Comment
DISTRIB_ID=Ubuntu
DISTRIB_RELEASE=18.04
DISTRIB_CODENAME=bionic
DISTRIB_DESCRIPTION="Ubuntu 18.04.4 LTS"
COMPLEX_VAR="  Ubuntu           1818 "
WO_QUOTES_VAR = qwerty
WO_QUOTES_VAR2 = qwerty 98989898
EMPTY_VAR3 = 
EMPTY_VAR4 =
     EMPTY_VAR5 = ""       
     ANOTHER_VAR6 = "Normal var"           
        TABBED_VAR = "Normal tabbed var"           
# Another comment
  # Yet another comment
        # Tabbed comment
VAR_WITHOUT_EOL=zxcvbnm
```

Text mode (default):

```
$ faux-file2c tmpfile

// File "tmpfile"
const char *txt1 =
        "# Comment\n"
        "DISTRIB_ID=Ubuntu\n"
        "DISTRIB_RELEASE=18.04\n"
        "DISTRIB_CODENAME=bionic\n"
        "DISTRIB_DESCRIPTION=\"Ubuntu 18.04.4 LTS\"\n"
        "COMPLEX_VAR=\"  Ubuntu\t\t1818 \"\n"
        "WO_QUOTES_VAR = qwerty\n"
        "WO_QUOTES_VAR2 = qwerty 98989898\n"
        "EMPTY_VAR3 = \n"
        "EMPTY_VAR4 =\n"
        "     EMPTY_VAR5 = \"\"\t   \n"
        "     ANOTHER_VAR6 = \"Normal var\"\t   \n"
        "\tTABBED_VAR = \"Normal tabbed var\"\t   \n"
        "# Another comment\n"
        "  # Yet another comment\n"
        "\t# Tabbed comment\n"
        "VAR_WITHOUT_EOL=zxcvbnm"
;

```

Binary mode:

```
// File "tmpfile"
const char *bin1 =
        "\x23\x20\x43\x6f\x6d\x6d\x65\x6e\x74\x0a\x44\x49\x53\x54\x52\x49\x42\x5f\x49\x44"
        "\x3d\x55\x62\x75\x6e\x74\x75\x0a\x44\x49\x53\x54\x52\x49\x42\x5f\x52\x45\x4c\x45"
        "\x41\x53\x45\x3d\x31\x38\x2e\x30\x34\x0a\x44\x49\x53\x54\x52\x49\x42\x5f\x43\x4f"
        "\x44\x45\x4e\x41\x4d\x45\x3d\x62\x69\x6f\x6e\x69\x63\x0a\x44\x49\x53\x54\x52\x49"
        "\x42\x5f\x44\x45\x53\x43\x52\x49\x50\x54\x49\x4f\x4e\x3d\x22\x55\x62\x75\x6e\x74"
        "\x75\x20\x31\x38\x2e\x30\x34\x2e\x34\x20\x4c\x54\x53\x22\x0a\x43\x4f\x4d\x50\x4c"
        "\x45\x58\x5f\x56\x41\x52\x3d\x22\x20\x20\x55\x62\x75\x6e\x74\x75\x09\x09\x31\x38"
        "\x31\x38\x20\x22\x0a\x57\x4f\x5f\x51\x55\x4f\x54\x45\x53\x5f\x56\x41\x52\x20\x3d"
        "\x20\x71\x77\x65\x72\x74\x79\x0a\x57\x4f\x5f\x51\x55\x4f\x54\x45\x53\x5f\x56\x41"
        "\x52\x32\x20\x3d\x20\x71\x77\x65\x72\x74\x79\x20\x39\x38\x39\x38\x39\x38\x39\x38"
        "\x0a\x45\x4d\x50\x54\x59\x5f\x56\x41\x52\x33\x20\x3d\x20\x0a\x45\x4d\x50\x54\x59"
        "\x5f\x56\x41\x52\x34\x20\x3d\x0a\x20\x20\x20\x20\x20\x45\x4d\x50\x54\x59\x5f\x56"
        "\x41\x52\x35\x20\x3d\x20\x22\x22\x09\x20\x20\x20\x0a\x20\x20\x20\x20\x20\x41\x4e"
        "\x4f\x54\x48\x45\x52\x5f\x56\x41\x52\x36\x20\x3d\x20\x22\x4e\x6f\x72\x6d\x61\x6c"
        "\x20\x76\x61\x72\x22\x09\x20\x20\x20\x0a\x09\x54\x41\x42\x42\x45\x44\x5f\x56\x41"
        "\x52\x20\x3d\x20\x22\x4e\x6f\x72\x6d\x61\x6c\x20\x74\x61\x62\x62\x65\x64\x20\x76"
        "\x61\x72\x22\x09\x20\x20\x20\x0a\x23\x20\x41\x6e\x6f\x74\x68\x65\x72\x20\x63\x6f"
        "\x6d\x6d\x65\x6e\x74\x0a\x20\x20\x23\x20\x59\x65\x74\x20\x61\x6e\x6f\x74\x68\x65"
        "\x72\x20\x63\x6f\x6d\x6d\x65\x6e\x74\x0a\x09\x23\x20\x54\x61\x62\x62\x65\x64\x20"
        "\x63\x6f\x6d\x6d\x65\x6e\x74\x0a\x56\x41\x52\x5f\x57\x49\x54\x48\x4f\x55\x54\x5f"
        "\x45\x4f\x4c\x3d\x7a\x78\x63\x76\x62\x6e\x6d"
;

```

These fragments can be embedded to C-code. The `testc_helpers` library contains special functions that allow to create real files using embedded data representation. Then test can work with real files on disk.

#### Utility options

* `-v`, `--version` - Show version.
* `-h`, `--help` - Show help.
* `-b`, `--binary` - Binary mode.
* `-t`, `--text` - Text mode (default).


### Header file

The header `faux/testc_helpers.h` contains declarations of all helper functions. Additionally it contains macro `FAUX_TESTC_TMPDIR_ENV`. It holds name of environment variable that is set to test's temporary directory path.


### Helper functions


#### The faux_testc_file_deploy() function

The function creates file with specified name (first argument) and writes string buffer (ends with `'\0'`) data (second argument) to it. The buffer can contain embedded data generated by `faux-file2c` utility.

```
// Etalon file
const char *etalon_file =
	"ANOTHER_VAR6=\"Normal var\"\n"
	"COMPLEX_VAR=\"  Ubuntu\t\t1818 \"\n"
;
char *etalon_fn = NULL;
int ret = 0;
etalon_fn = str_faux_sprintf("%s/%s", getenv(FAUX_TESTC_TMPDIR_VAR), "etalon.txt");
ret = faux_testc_file_deploy(etalon_fn, etalon_file);
...
faux_str_free(etalon_fn);
```

#### The faux_testc_tmpfile_deploy() function

The function is the same as `faux_testc_file_deploy()`, but it generates unique filename itself. Function returns generated name. It uses `TESTC_TMPDIR` environment variable to get temporary directory path.

```
// Etalon file
const char *etalon_file =
	"ANOTHER_VAR6=\"Normal var\"\n"
	"COMPLEX_VAR=\"  Ubuntu\t\t1818 \"\n"
;
char *etalon_fn = NULL;
etalon_fn = faux_testc_tmpfile_deploy(etalon_file);
...
faux_str_free(etalon_fn);
```

#### The faux_testc_file_cmp() function

The function compares two specifed files byte-by-byte. It returns `0` if files are equal.

```
if (faux_testc_file_cmp(dst_fn, etalon_fn) != 0) {
	fprintf(stderr, "Generated file is not equal to etalon.\n");
	...
}
```
