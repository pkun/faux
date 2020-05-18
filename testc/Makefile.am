## Process this file with automake to produce Makefile.in
bin_PROGRAMS += \
	testc/testc

testc_testc_SOURCES = \
	testc/testc.c

# The testc binary must be maximally static because LD_LIBRARY_PATH may be used
# to test not-installed libraries. This LD_LIBRARY_PATH can influence testc
# itself. So build necessary parts of faux library statically.
testc_testc_SOURCES += \
	testc/base/mem.c \
	testc/base/io.c \
	testc/ctype/ctype.c \
	testc/str/str.c \
	testc/list/list.c \
	testc/list/private.h
