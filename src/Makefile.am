SUBDIRS = objects . plugins

AM_CFLAGS   = -Wall --std=gnu99 -I$(top_srcdir)/src
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
AM_LDADD    = $(GLIB_LIBS) $(GTK_LIBS) $(SOUP_LIBS)
AM_LDFLAGS  = --as-needed -no-undefined

BUILT_SOURCES = gis-marshal.c gis-marshal.h


# Headers
gis_includedir = $(includedir)/gis
gis_include_HEADERS = \
	gis.h         \
	gis-viewer.h  \
	gis-prefs.h   \
	gis-opengl.h  \
	gis-plugin.h  \
	gis-util.h    \
	gis-wms.h     \
	gis-data.h    \
	gpqueue.h     \
	roam.h

# Pkg-config
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgis.pc

# Library
lib_LTLIBRARIES = libgis.la

libgis_la_SOURCES = gis.h \
	gis-viewer.c  gis-viewer.h  \
	gis-prefs.c   gis-prefs.h   \
	gis-opengl.c  gis-opengl.h  \
	gis-plugin.c  gis-plugin.h  \
	gis-marshal.c gis-marshal.h \
	gis-util.c    gis-util.h    \
	gis-wms.c     gis-wms.h     \
	gis-data.c    gis-data.h    \
	roam.c        roam.h        \
	gpqueue.c     gpqueue.h
libgis_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-DPKGDATADIR="\"$(datadir)/gis\"" \
	-DPLUGINSDIR="\"$(libdir)/gis\""
libgis_la_LIBADD  = $(AM_LDADD) \
	objects/libgis-objects.la

# Test programs
bin_PROGRAMS = gis-demo tile-test

gis_demo_SOURCES  = gis-demo.c
gis_demo_LDADD    = $(AM_LDADD) libgis.la
gis_demo_LDFLAGS  = $(AM_LDFLAGS) -static

tile_test_SOURCES = tile-test.c
tile_test_LDADD   = $(AM_LDADD) libgis.la
tile_test_LDFLAGS = $(AM_LDFLAGS) -static


MAINTAINERCLEANFILES = Makefile.in

.list.c:
	glib-genmarshal --prefix=gis_cclosure_marshal --body   $< > $@
.list.h:
	glib-genmarshal --prefix=gis_cclosure_marshal --header $< > $@

test: all .libs/gis
	./gis-demo

gdb: all .libs/gis
	gdb ./gis-demo

ddd: all .libs/gis
	ddd ./gis-demo

.libs/gis: plugins/.libs
	ln -sf ../plugins/.libs .libs/gis

memcheck: all
	G_SLICE=always-malloc                   \
	G_DEBUG=gc-friendly,resident-modules    \
	valgrind --track-origins=yes            \
	         --leak-check=full              \
	         --leak-resolution=high         \
	         --num-callers=50               \
	         --suppressions=gtk.suppression \
	         ./gis-demo                     \
	2> valgrind.out
