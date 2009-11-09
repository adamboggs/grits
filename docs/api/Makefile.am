AM_CPPFLAGS=$(SOUP_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS)

GTKDOC_LIBS        = $(SOUP_LIBS) $(GLIB_LIBS) $(GTK_LIBS) $(top_srcdir)/src/libgis.la $(top_srcdir)/src/plugins/*.la
DOC_MODULE         = libgis
DOC_SOURCE_DIR     = $(top_srcdir)/src/
DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.sgml
MKDB_OPTIONS       = --sgml-mode --output-format=xml
include $(top_srcdir)/gtk-doc.make

CLEANFILES += libgis-sections.txt

MAINTAINERCLEANFILES=\
	tmpl/* Makefile.in libgis-overrides.txt libgis.types libgis-scan.*
