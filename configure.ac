m4_define([grits_release], [0.4.2])
m4_define([grits_version], [0:0:0])
m4_define([grits_subdir],  [grits0])

# Init and options
AC_INIT([grits], [grits_release], [andy753421@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror -Wno-portability foreign])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
#AM_SILENT_RULES([yes])

# For library versioning
LIB_VERSION=grits_version
GRITS_SUBDIR=grits_subdir
AC_SUBST(LIB_VERSION)
AC_SUBST(GRITS_SUBDIR)

# Check for required programs
AC_PROG_CC
AC_PROG_LIBTOOL
DOLT
PKG_PROG_PKG_CONFIG
GTK_DOC_CHECK(1.9)

# Check for required packages
PKG_CHECK_MODULES(GLIB,  glib-2.0 >= 2.14 gobject-2.0 gthread-2.0)
PKG_CHECK_MODULES(CAIRO, cairo)
PKG_CHECK_MODULES(GTK,   gtk+-2.0 >= 2.16 gtkglext-1.0)
PKG_CHECK_MODULES(SOUP,  libsoup-2.4 >= 2.26)

# Test for Windows vs. Unix
case "${host}" in
	*mingw32*) WIN32="yes" ;;
	*cygwin*)  WIN32="yes" ;;
	*)         WIN32="no"  ;;
esac
if test "$WIN32" = yes; then
	ac_default_prefix="/"
fi
AM_CONDITIONAL([WIN32],    test "$WIN32" = "yes")
AM_CONDITIONAL([NOTWIN32], test "$WIN32" = "no")

# Output 
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/data/Makefile
	src/objects/Makefile
	src/plugins/Makefile
	src/grits.pc
	docs/Makefile
	docs/api/Makefile
])
AC_OUTPUT
