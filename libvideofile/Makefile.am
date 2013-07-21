AM_CFLAGS = -Wall -Werror -Wno-deprecated-declarations

noinst_LIBRARIES = libvideofile.a

libvideofile_a_SOURCES = \
		 ffmpeg-util.c ffmpeg-util.h \
		 video-file.c video-file.h \
		 video-file-mpeg.c video-file-mpeg.h \
		 video-store.c video-store.h

INCLUDES = $(GLIB_CFLAGS) \
		   $(AVCODEC_CFLAGS) \
		   $(AVFORMAT_CFLAGS)
