AM_CFLAGS = -Wall -Werror
bin_PROGRAMS = authorg

DEFS=-DDATADIR=\"$(datadir)\"

AM_CPPFLAGS = $(GLIB_CFLAGS) \
		   -I ../libvideofile

authorg_SOURCES = \
				  about.c about.h \
				  authorg.c \
				  chapter-editor.c chapter-editor.h \
				  main-window.c main-window.h \
				  spawn-dialog.c spawn-dialog.h \
				  util.c util.h

authorg_LDADD = $(GLIB_LIBS) \
				../libvideofile/libvideofile.a \
				$(AVFORMAT_LIBS) \
				$(AVFORMAT_CFLAGS)
