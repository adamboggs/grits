SUBDIRS = plugins

AM_CFLAGS   = -Wall --std=gnu99 
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
AM_LDADD    = $(GLIB_LIBS) $(GTK_LIBS) $(SOUP_LIBS)

# Library
BUILT_SOURCES = gis-marshal.c gis-marshal.h
lib_LTLIBRARIES  = libgis.la
gis_includedir = $(includedir)/gis
gis_include_HEADERS = \
	gis.h         \
	gis-world.h   \
	gis-view.h    \
	gis-prefs.h   \
	gis-opengl.h  \
	gis-plugin.h  \
	gis-data.h    \
	gis-tile.h    \
	gis-wms.h     \
	gpqueue.h     \
	roam.h
libgis_la_SOURCES = \
	gis-world.c   gis-world.h   \
	gis-view.c    gis-view.h    \
	gis-prefs.c   gis-prefs.h   \
	gis-opengl.c  gis-opengl.h  \
	gis-plugin.c  gis-plugin.h  \
	gis-marshal.c gis-marshal.h \
	gis-data.c    gis-data.h    \
	gis-tile.c    gis-tile.h    \
	gis-wms.c     gis-wms.h     \
	roam.c        roam.h        \
	gpqueue.c     gpqueue.h
libgis_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-DPKGDATADIR="\"$(datadir)/gis\"" -DPLUGINSDIR="\"$(libdir)/gis\""
libgis_la_LIBADD = $(AM_LDADD)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgis.pc

# Test programs
bin_PROGRAMS = gis_test wms_test

gis_test_SOURCES  = gis_test.c gis.h
gis_test_LDFLAGS  = -static
gis_test_LDADD    = $(AM_LDADD) libgis.la

wms_test_SOURCES  = wms_test.c gis-world.c gis-world.h gis-wms.c gis-wms.h gis-tile.c gis-tile.h
wms_test_LDADD    = $(AM_LDADD)

MAINTAINERCLEANFILES = Makefile.in

.list.c:
	glib-genmarshal --prefix=gis_cclosure_marshal --body   $< > $@
.list.h:
	glib-genmarshal --prefix=gis_cclosure_marshal --header $< > $@

test: all .libs/gis
	./gis_test

gdb: all .libs/gis
	gdb ./gis_test

ddd: all .libs/gis
	ddd ./gis_test

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
	         ./gis_test                     \
	2> valgrind.out
