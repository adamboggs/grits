AM_CFLAGS  = -Wall --std=gnu99 -fPIC -I../
AM_CFLAGS += $(GLIB_CFLAGS) $(CAIRO_CFLAGS)

gis_includedir = $(includedir)/gis
gis_include_HEADERS = \
	gis-object.h  \
	gis-tile.h

noinst_LTLIBRARIES = libgis-objects.la
libgis_objects_la_SOURCES = \
	gis-object.c gis-object.h \
	gis-tile.c   gis-tile.h
libgis_objects_la_LDFLAGS = -static

MAINTAINERCLEANFILES = Makefile.in
