AC_INIT([authorg],[0.1])
AC_CONFIG_SRCDIR(src/authorg.c)
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE([enable])

IT_PROG_INTLTOOL([0.21])
AC_PROG_AWK
AC_PROG_LN_S
AC_PROG_CC
AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AM_SILENT_RULES([yes])

GNOME_COMPILE_WARNINGS

ALL_LINGUAS="de"

GETTEXT_PACKAGE=authorg
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",
                                   [definition of GETTEXT_PACKAGE])
AM_GLIB_DEFINE_LOCALEDIR(GNOMELOCALEDIR)

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.6.0 gtk+-3.0 >= 3.0 gio-2.0 >= 2.20 gconf-2.0)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

GTK_DOC_CHECK(1.0)

dnl *****************************************************************
dnl gconf stuff
AC_ARG_ENABLE(schemas-install,
			  AS_HELP_STRING([--disable-schemas-install],[Disbale installation of gconf schemas]))

AM_CONDITIONAL(SCHEMAS_INSTALL, test "x$enable_schemas_install" != "xno")

AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
if test x"$GCONFTOOL" = xno; then
	AC_MSG_ERROR([gconftool-2 executable not found in your path - it should be installed with GConf])
fi
AM_GCONF_SOURCE_2

dnl *****************************************************************
dnl check for ffmpeg
PKG_CHECK_MODULES(AVCODEC, libavcodec >= 52.20)
AC_SUBST(AVCODEC_LIBS)
AC_SUBST(AVCODEC_CFLAGS)

PKG_CHECK_MODULES(AVFORMAT, libavformat >= 52.31)
AC_SUBST(AVFORMAT_LIBS)
AC_SUBST(AVFORMAT_CFLAGS)

dnl *****************************************************************
dnl check for dvdauthor
AC_PATH_PROG(PROG_DVDAUTHOR, dvdauthor)
AC_DEFINE_UNQUOTED(DVDAUTHOR, "$PROG_DVDAUTHOR", [location of the dvdauthor binary])

dnl *****************************************************************
dnl check for totem-video-thumbnailer
AC_PATH_PROG(PROG_TOTEM_VIDEO_THUMBNAILER, totem-video-thumbnailer)
AC_DEFINE_UNQUOTED(TOTEM_VIDEO_THUMBNAILER, "$PROG_TOTEM_VIDEO_THUMBNAILER", [location of the totem-video-thumbnailer binary])

dnl *****************************************************************
dnl check for mkisofs
AC_PATH_PROG(PROG_MKISOFS, mkisofs)
AC_DEFINE_UNQUOTED(MKISOFS, "$PROG_MKISOFS", [location of the mkisofs binary])

AC_CONFIG_FILES([
				 Makefile
				 data/Makefile
				 src/Makefile
				 libvideofile/Makefile
				 libconfig/Makefile
				 po/Makefile.in
				 doc/Makefile
				 doc/reference/Makefile
				 doc/reference/libvideofile/Makefile
				 ])
AC_OUTPUT
