AM_CFLAGS  = -Wall --std=gnu99 -I$(top_srcdir)/src
AM_CFLAGS += $(GLIB_CFLAGS) $(GTK_CFLAGS) $(SOUP_CFLAGS)
if NOTWIN32
AM_CFLAGS += -fPIC
endif

grits_data_includedir = $(includedir)/grits/data
grits_data_include_HEADERS = \
	grits-data.h \
	grits-http.h \
	grits-wms.h

noinst_LTLIBRARIES = libgrits-data.la
libgrits_data_la_SOURCES = \
	grits-data.c grits-data.h \
	grits-http.c grits-http.h \
	grits-wms.c  grits-wms.h
libgrits_data_la_LDFLAGS = -static

MAINTAINERCLEANFILES = Makefile.in
