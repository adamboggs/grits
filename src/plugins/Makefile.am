MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS   = -Wall --std=gnu99 -I$(top_srcdir)/src
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
AM_LDFLAGS  = -module -avoid-version -no-undefined
LIBS        = $(top_srcdir)/src/libgis.la

pluginsdir  = "$(libdir)/$(GIS_SUBDIR)"

plugins_LTLIBRARIES = sat.la elev.la map.la env.la test.la
sat_la_SOURCES      = sat.c  sat.h
elev_la_SOURCES     = elev.c elev.h
map_la_SOURCES      = map.c  map.h
env_la_SOURCES      = env.c  env.h
test_la_SOURCES     = test.c test.h

noinst_LTLIBRARIES  = teapot.la
teapot_la_SOURCES   = \
	$(top_srcdir)/examples/plugin/teapot.c \
	$(top_srcdir)/examples/plugin/teapot.h

test:
	( cd ../; make test )
