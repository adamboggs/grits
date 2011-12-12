MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS   = -Wall --std=gnu99 -I$(top_srcdir)/src
AM_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(SOUP_CFLAGS)
AM_LDFLAGS  = -shared -module -avoid-version
LIBS        = $(top_srcdir)/src/libgrits.la

if SYS_WIN
AM_LDFLAGS += -no-undefined
endif
if !SYS_MAC
AM_LDFLAGS += -Wl,--as-needed
endif

pluginsdir  = "$(libdir)/$(GRITS_SUBDIR)"

plugins_LTLIBRARIES = sat.la elev.la map.la env.la test.la
sat_la_SOURCES      = sat.c  sat.h
elev_la_SOURCES     = elev.c elev.h
map_la_SOURCES      = map.c  map.h
env_la_SOURCES      = env.c  env.h
test_la_SOURCES     = test.c test.h

if HAVE_GLUT
noinst_LTLIBRARIES  = teapot.la
teapot_la_SOURCES   = \
	$(top_srcdir)/examples/plugin/teapot.c \
	$(top_srcdir)/examples/plugin/teapot.h
teapot_la_LDFLAGS   = $(AM_LDFLAGS) -lglut -rpath $(libdir) # I hate you libtool
endif

test:
	( cd ../; make test )
