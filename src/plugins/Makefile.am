MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS   = -Wall --std=gnu99  -I../
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
AM_LDFLAGS  = -module -avoid-version -no-undefined
LIBS        = $(top_srcdir)/src/libgis.la

pluginsdir  = "$(libdir)/gis"

plugins_LTLIBRARIES = bmng.la srtm.la env.la test.la
bmng_la_SOURCES     = bmng.c bmng.h
srtm_la_SOURCES     = srtm.c srtm.h
env_la_SOURCES      = env.c  env.h
test_la_SOURCES     = test.c test.h

noinst_LTLIBRARIES  = teapot.la
teapot_la_SOURCES   = \
	$(top_srcdir)/examples/plugin/teapot.c
	$(top_srcdir)/examples/plugin/teapot.h

test:
	( cd ../; make test )
