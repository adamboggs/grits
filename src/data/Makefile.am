AM_CFLAGS  = -Wall --std=gnu99 -I$(top_srcdir)/src
AM_CFLAGS += $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
if NOTWIN32
AM_CFLAGS += -fPIC
endif

gis_data_includedir = $(includedir)/gis/data
gis_data_include_HEADERS = \
	gis-data.h \
	gis-http.h \
	gis-wms.h

noinst_LTLIBRARIES = libgis-data.la
libgis_data_la_SOURCES = \
	gis-data.c gis-data.h \
	gis-http.c gis-http.h \
	gis-wms.c  gis-wms.h
libgis_data_la_LDFLAGS = -static

MAINTAINERCLEANFILES = Makefile.in
