AM_CFLAGS  = -Wall --std=gnu99 -I$(top_srcdir)/src
AM_CFLAGS += $(GLIB_CFLAGS) $(GTK_CFLAGS) $(CAIRO_CFLAGS)
if NOTWIN32
AM_CFLAGS += -fPIC
endif

gis_objects_includedir = $(includedir)/gis/objects
gis_objects_include_HEADERS = \
	gis-object.h \
	gis-marker.h \
	gis-callback.h \
	gis-tile.h

noinst_LTLIBRARIES = libgis-objects.la
libgis_objects_la_SOURCES = \
	gis-object.c   gis-object.h \
	gis-marker.c   gis-marker.h \
	gis-callback.c gis-callback.h \
	gis-tile.c     gis-tile.h
libgis_objects_la_LDFLAGS = -static

MAINTAINERCLEANFILES = Makefile.in
