m4_define([grits_release], [0.6.2])
m4_define([grits_version], [2:1:0])
m4_define([grits_subdir],  [grits2])

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

# Fix broken libtool
lt_cv_sys_lib_dlsearch_path_spec="/lib /lib32 /lib64 /usr/lib /usr/lib32 /usr/lib64"

# Check for required programs
AC_PROG_CC
AC_PROG_LIBTOOL
DOLT
PKG_PROG_PKG_CONFIG
GTK_DOC_CHECK(1.9)

# Check for required packages
PKG_CHECK_MODULES(GLIB,  glib-2.0 >= 2.14 gobject-2.0 gthread-2.0)
PKG_CHECK_MODULES(CAIRO, cairo)
PKG_CHECK_MODULES(GTK,   gtk+-2.0 >= 2.16)
PKG_CHECK_MODULES(SOUP,  libsoup-2.4 >= 2.26)

# Check for glut for example plugin
AC_CHECK_LIB(glut, glutSolidTeapot, GLUT_LIBS="-lglut")
AM_CONDITIONAL(HAVE_GLUT, test "$GLUT_LIBS" != "")
AC_SUBST(GLUT_LIBS)

# Test for windowing system
case "${host}" in
	*mingw32*) SYS="WIN" ;;
	*cygwin*)  SYS="WIN" ;;
	*apple*)   SYS="MAC" ;;
	*)         SYS="X11" ;;
esac
AM_CONDITIONAL([SYS_WIN], test "$SYS" = "WIN")
AM_CONDITIONAL([SYS_MAC], test "$SYS" = "MAC")
AM_CONDITIONAL([SYS_X11], test "$SYS" = "X11")

# OpenGL flags
GL_CFLAGS="-DSYS_$SYS"
case "$SYS" in
	"WIN") GL_LIBS="-lglu32 -lopengl32" ;;
	"MAC") GL_LIBS="-framework OpenGL" ;;
	"X11") GL_LIBS="-lGL -lGLU" ;;
esac
AC_SUBST([GL_CFLAGS])
AC_SUBST([GL_LIBS])

# Check for relative build
AC_ARG_ENABLE(relative, [AS_HELP_STRING([--enable-relative],
	[enable runtime search paths @<:@default=no@:>@])],
	[DOTS=".."; ac_default_prefix="/"])
AC_SUBST([DOTS])

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
