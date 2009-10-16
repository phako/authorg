#ifndef _FFMPEG_UTIL_H
#define _FFMPEG_UTIL_H

#include <glib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#define AUTHORG_FFMPEG_ERROR authorg_ffmpeg_util_quark()

enum
{
	AUTHORG_FFMPEG_ERROR_OPEN = 0,
	AUTHORG_FFMPEG_ERROR_INFO,
	AUTHORG_FFMPEG_ERROR_NO_VIDEO,
	AUTHORG_FFMPEG_ERROR_NO_CODEC,
	AUTHORG_FFMPEG_ERROR_CODEC_OPEN,
	AUTHORG_FFMPEG_ERROR_NO_FRAME,
} AuthorgFfmpegError;

GdkPixbuf *authorg_ffmpeg_grab_pixbuf (const gchar *filename, guint timecode, GError **error);
GQuark authorg_ffmpeg_util_quark ();
gchar *authorg_ffmpeg_util_error_to_string (gint error);
#endif
