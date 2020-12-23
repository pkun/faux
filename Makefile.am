## Process this file with automake to generate Makefile.in
AUTOMAKE_OPTIONS = foreign nostdinc
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I. -I$(top_srcdir)
AM_LD = $(CC)
VERSION_INFO = -version-info $(SONAME_CURRENT):$(SONAME_REVISION):$(SONAME_AGE)

if DEBUG
DEBUG_CFLAGS = -DDEBUG
endif

AM_CFLAGS = -Wall $(DEBUG_CFLAGS)

bin_PROGRAMS =
lib_LTLIBRARIES =
lib_LIBRARIES =
nobase_include_HEADERS =

EXTRA_DIST = \
	faux/Makefile.am \
	utils/Makefile.am \
	testc/Makefile.am

include $(top_srcdir)/faux/Makefile.am
include $(top_srcdir)/utils/Makefile.am
include $(top_srcdir)/testc/Makefile.am

define CONTROL
PACKAGE: faux
Version: $(PACKAGE_VERSION)
Architecture: amd64
Maintainer: pkun
Description: Various auxiliary functions
Homepage: https://src.libcode.org/pkun/faux
endef

export CONTROL
deb: all
	mkdir -p deb/DEBIAN
	echo "$$CONTROL" > deb/DEBIAN/control
	DESTDIR=$(shell pwd)/deb make install
	dpkg-deb --build deb/ faux_$(PACKAGE_VERSION).deb
