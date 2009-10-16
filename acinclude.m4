dnl check for gnome-vfs
AC_DEFUN([AC_AUTHORG_CHECK_GNOME_VFS],[
	ENABLE_GNOME_VFS=yes
	AC_ARG_ENABLE(gnome-vfs,
			   AC_HELP_STRING([--enable-gnome-vfs], [use gnome-vfs for MIME detection [[default=yes]]]),
			   ENABLE_GNOME_VFS=$enableval, ENABLE_GNOME_VFS=yes)

	if test x"$ENABLE_GNOME_VFS" = x"yes"; then
		PKG_CHECK_MODULES(GNOME_VFS, gnome-vfs-2.0 >= 2.0 gnome-vfs-module-2.0 >= 2.0, have_gnome_vfs=yes,have_gnome_vfs=no)
		AC_MSG_CHECKING([for gnome-vfs])
		AC_MSG_RESULT([$have_gnome_vfs])
		if test x"$have_gnome_vfs" = x"yes"; then
			AC_SUBST(GNOME_VFS_LIBS)
			AC_SUBST(GNOME_VFS_CFLAGS)
			AC_DEFINE(HAVE_GNOME_VFS, [], [do we have gnome-vfs])
		fi
	fi
])

dnl check for gnome, obey --enable-gnome
AC_DEFUN([AC_AUTHORG_CHECK_GNOME],[
	ENABLE_GNOME=yes
	AC_ARG_ENABLE(gnome,
				  AC_HELP_STRING([--enable-gnome], [use gnome libraries [[default=yes]]]),
				  ENABLE_GNOME=$enableval, ENABLE_GNOME=yes)

	if test x"$ENABLE_GNOME" = x"yes"; then
		PKG_CHECK_MODULES(GNOME, libgnome-2.0 >= 2.0 libgnomeui-2.0 >= 2.0, have_gnome=yes, have_gnome=no)
		AC_MSG_CHECKING([for gnome])
		AC_MSG_RESULT([$have_gnome])
		if test x"$have_gnome" = x"yes"; then
			AC_SUBST(GNOME_LIBS)
			AC_SUBST(GNOME_CFLAGS)
			AC_DEFINE(HAVE_GNOME, [], [do we have gnome])
		fi
	fi
])
