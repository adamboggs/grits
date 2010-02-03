SUBDIRS = . plugins

AM_CFLAGS   = -Wall --std=gnu99
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
AM_LDADD    = $(GLIB_LIBS) $(GTK_LIBS) $(SOUP_LIBS)

# Library
BUILT_SOURCES = gis-marshal.c gis-marshal.h
lib_LTLIBRARIES  = libgis.la
gis_includedir = $(includedir)/gis
gis_include_HEADERS = \
	gis.h         \
	gis-viewer.h  \
	gis-prefs.h   \
	gis-opengl.h  \
	gis-plugin.h  \
	gis-object.h  \
	gis-util.h    \
	gis-tile.h    \
	gis-wms.h     \
	gis-data.h    \
	gpqueue.h     \
	roam.h
libgis_la_SOURCES = \
	gis-viewer.c  gis-viewer.h  \
	gis-prefs.c   gis-prefs.h   \
	gis-opengl.c  gis-opengl.h  \
	gis-plugin.c  gis-plugin.h  \
	gis-marshal.c gis-marshal.h \
	gis-util.c    gis-util.h    \
	gis-tile.c    gis-tile.h    \
	gis-wms.c     gis-wms.h     \
	gis-data.c    gis-data.h    \
	gis-object.c  gis-object.h  \
	roam.c        roam.h        \
	gpqueue.c     gpqueue.h
libgis_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-DPKGDATADIR="\"$(datadir)/gis\"" -DPLUGINSDIR="\"$(libdir)/gis\""
libgis_la_LIBADD = $(AM_LDADD)
libgis_la_LDFLAGS = -no-undefined

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgis.pc

# Test programs
bin_PROGRAMS = gis-demo tile-test

gis_demo_SOURCES  = gis-demo.c gis.h
gis_demo_LDADD    = $(AM_LDADD) libgis.la
gis_demo_LDFLAGS  = -static

tile_test_SOURCES = tile-test.c   \
	gis-util.c   gis-util.h   \
	gis-tile.c   gis-tile.h   \
	gis-object.c gis-object.h \
	gis-wms.c    gis-wms.h
tile_test_LDADD   = $(AM_LDADD)

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
