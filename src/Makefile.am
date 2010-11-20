SUBDIRS = data objects . plugins

AM_CFLAGS   = -Wall --std=gnu99 -I$(top_srcdir)/src
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
AM_LDADD    = $(GLIB_LIBS) $(GTK_LIBS) $(SOUP_LIBS)
AM_LDFLAGS  = --as-needed -no-undefined

BUILT_SOURCES = grits-marshal.c grits-marshal.h


# Headers
grits_includedir = $(includedir)/grits
grits_include_HEADERS = \
	grits.h         \
	grits-viewer.h  \
	grits-prefs.h   \
	grits-opengl.h  \
	grits-plugin.h  \
	grits-util.h    \
	gpqueue.h       \
	roam.h

# Pkg-config
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = grits.pc

# Library
lib_LTLIBRARIES = libgrits.la

libgrits_la_SOURCES = grits.h \
	grits-viewer.c  grits-viewer.h  \
	grits-prefs.c   grits-prefs.h   \
	grits-opengl.c  grits-opengl.h  \
	grits-plugin.c  grits-plugin.h  \
	grits-marshal.c grits-marshal.h \
	grits-util.c    grits-util.h    \
	roam.c          roam.h          \
	gpqueue.c       gpqueue.h
libgrits_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-DPKGDATADIR="\"$(dots)$(datadir)/$(GRITS_SUBDIR)\"" \
	-DPLUGINSDIR="\"$(dots)$(libdir)/$(GRITS_SUBDIR)\""
libgrits_la_LIBADD  = $(AM_LDADD) \
	objects/libgrits-objects.la \
	data/libgrits-data.la
libgrits_la_LDFLAGS = $(AM_LDFLAGS) \
	-version-info $(LIB_VERSION)

# Demo program
if WIN32
AM_LDFLAGS += -mwindows
dots        = ..
else
BINLDFLAGS  = -static
endif

bin_PROGRAMS = grits-demo

grits_demo_SOURCES = grits-demo.c
grits_demo_LDADD   = $(AM_LDADD) libgrits.la
grits_demo_LDFLAGS = $(BINLDFLAGS)

# Test programs
noinst_PROGRAMS = grits-test tile-test

grits_test_SOURCES = grits-test.c
grits_test_LDADD   = $(AM_LDADD) libgrits.la
grits_test_LDFLAGS = $(BINLDFLAGS)

tile_test_SOURCES = tile-test.c
tile_test_LDADD   = $(AM_LDADD) libgrits.la
tile_test_LDFLAGS = $(BINLDFLAGS)


MAINTAINERCLEANFILES = Makefile.in

.list.c:
	glib-genmarshal --prefix=grits_cclosure_marshal --body   $< > $@
.list.h:
	glib-genmarshal --prefix=grits_cclosure_marshal --header $< > $@

test: all .libs/$(GRITS_SUBDIR)
	./grits-test

gdb: all .libs/$(GRITS_SUBDIR)
	gdb ./grits-test

ddd: all .libs/$(GRITS_SUBDIR)
	ddd ./grits-test

.libs/$(GRITS_SUBDIR): plugins/.libs
	ln -sf ../plugins/.libs .libs/$(GRITS_SUBDIR)

memcheck: all
	G_SLICE=always-malloc                   \
	G_DEBUG=gc-friendly,resident-modules    \
	valgrind --track-origins=yes            \
	         --leak-check=full              \
	         --leak-resolution=high         \
	         --num-callers=50               \
	         --suppressions=gtk.suppression \
	         ./grits-test                   \
	2> valgrind.out
