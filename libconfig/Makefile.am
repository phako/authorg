AM_CFLAGS = -Wall -Werror

noinst_LIBRARIES = libconfig.a

libconfig_a_SOURCES = \
			tools-page.c tools-page.h \
			settings-page.c settings-page.h

INCLUDES = $(GLIB_CFLAGS) \
		   $(GNOME_CFLAGS) \
		   $(GNOME_VFS_CFLAGS) \
		   -I ../libvideofile
