MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS   = -Wall --std=gnu99 -I$(top_srcdir)/src
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
AM_LDFLAGS  = -shared -module -avoid-version -Wl,--as-needed
LIBS        = $(top_srcdir)/src/libgrits.la

pluginsdir  = "$(libdir)/$(GRITS_SUBDIR)"

plugins_LTLIBRARIES = sat.la elev.la map.la env.la test.la
sat_la_SOURCES      = sat.c  sat.h
elev_la_SOURCES     = elev.c elev.h
map_la_SOURCES      = map.c  map.h
env_la_SOURCES      = env.c  env.h
test_la_SOURCES     = test.c test.h

if WIN32
AM_LDFLAGS += -no-undefined
endif

if HAVE_GLUT
noinst_LTLIBRARIES  = teapot.la
teapot_la_SOURCES   = \
	$(top_srcdir)/examples/plugin/teapot.c \
	$(top_srcdir)/examples/plugin/teapot.h
endif

test:
	( cd ../; make test )
