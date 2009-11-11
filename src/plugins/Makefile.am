MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS   = -Wall --std=gnu99  -I../
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
AM_LDFLAGS  = -module -avoid-version -no-undefined
LIBS        = $(top_srcdir)/src/libgis.la

pluginsdir  = "$(libdir)/gis"

plugins_LTLIBRARIES = bmng.la srtm.la
bmng_la_SOURCES     = bmng.c bmng.h
srtm_la_SOURCES     = srtm.c srtm.h

test:
	( cd ../; make test )
