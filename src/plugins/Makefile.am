MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS       = -Wall --std=gnu99  -I../
AM_CPPFLAGS     = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
AM_LDFLAGS      = -module -avoid-version
LIBS            = $(top_srcdir)/src/libgis.la

pluginsdir  = $(pkglibdir)

plugins_LTLIBRARIES = bmng.la srtm.la
bmng_la_SOURCES     = bmng.c bmng.h
srtm_la_SOURCES     = srtm.c srtm.h

test:
	( cd ../; make test )

# Fixme...
bmng_la_DEPENDENCIES = $(top_srcdir)/src/libgis.la
srtm_la_DEPENDENCIES = $(top_srcdir)/src/libgis.la
$(top_srcdir)/src/libgis.la: $(wildcard ../*.[ch])
	( cd ../; make libgis.la )
