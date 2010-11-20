AM_CPPFLAGS = $(SOUP_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS)

# Gtk docs
GTKDOC_LIBS        = $(SOUP_LIBS) $(GLIB_LIBS) $(GTK_LIBS) \
		     $(top_srcdir)/src/grits.la \
		     $(top_srcdir)/src/plugins/*.la
DOC_MODULE         = grits
DOC_SOURCE_DIR     = $(top_srcdir)/src/
DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.sgml
MKDB_OPTIONS       = --sgml-mode --output-format=xml
include $(top_srcdir)/gtk-doc.make

# Man pages 
man_MANS = grits-demo.1
%.1: %.xml 
	xsltproc -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<
EXTRA_DIST += $(man_MANS)

# Remove auto-generated files
MAINTAINERCLEANFILES = \
	tmpl/* Makefile.in grits-overrides.txt grits-sections.txt grits.types grits-scan.*
