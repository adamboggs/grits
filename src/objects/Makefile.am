AM_CFLAGS  = -Wall --std=gnu99 -I$(top_srcdir)/src
AM_CFLAGS += $(GLIB_CFLAGS) $(GTK_CFLAGS) $(CAIRO_CFLAGS)
if NOTWIN32
AM_CFLAGS += -fPIC
endif

grits_objects_includedir = $(includedir)/grits/objects
grits_objects_include_HEADERS = \
	grits-object.h   \
	grits-marker.h   \
	grits-callback.h \
	grits-tile.h     \
	grits-volume.h   \
	marching.h

noinst_LTLIBRARIES = libgrits-objects.la
libgrits_objects_la_SOURCES = \
	grits-object.c   grits-object.h   \
	grits-marker.c   grits-marker.h   \
	grits-callback.c grits-callback.h \
	grits-tile.c     grits-tile.h     \
	grits-volume.c   grits-volume.h   \
	marching.c       marching.h
libgrits_objects_la_LDFLAGS = -static

MAINTAINERCLEANFILES = Makefile.in
