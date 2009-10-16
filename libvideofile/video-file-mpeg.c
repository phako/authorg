/* Authorg - a GNOME DVD authoring tool
 *
 * Copyright (C) 2005 Jens Georg <mail@jensge.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "video-file-mpeg.h"

G_DEFINE_TYPE (AuthorgVideoFileMpeg, authorg_video_file_mpeg, AUTHORG_TYPE_VIDEO_FILE);

static void
authorg_video_file_mpeg_info (AuthorgVideoFile *file)
{
	AVFormatContext *pFormatCtx;
	
	av_register_all();
	av_open_input_file (&pFormatCtx, file->filename,
			NULL, 0, NULL);

	av_find_stream_info (pFormatCtx);

	file->length = pFormatCtx->duration / AV_TIME_BASE;

	av_close_input_file (pFormatCtx);

	/* chain up */
	AUTHORG_VIDEO_FILE_CLASS (authorg_video_file_mpeg_parent_class)->info (file);
}

static void
authorg_video_file_mpeg_init (AuthorgVideoFileMpeg *video_file_mpeg)
{
	/* nothing to do */
}

static void
authorg_video_file_mpeg_finalize (GObject *object)
{
	g_return_if_fail (object != NULL);

	if (G_OBJECT_CLASS (authorg_video_file_mpeg_parent_class)->finalize)
		G_OBJECT_CLASS (authorg_video_file_mpeg_parent_class)->finalize (object);
}

static void
authorg_video_file_mpeg_class_init (AuthorgVideoFileMpegClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AuthorgVideoFileClass *video_file_class = AUTHORG_VIDEO_FILE_CLASS (klass);

	gobject_class->finalize = authorg_video_file_mpeg_finalize;

	video_file_class->info = authorg_video_file_mpeg_info;
}

AuthorgVideoFile *
authorg_video_file_mpeg_new (const gchar *filename)
{
	AuthorgVideoFile *file;

	file = AUTHORG_VIDEO_FILE (g_object_new (AUTHORG_TYPE_VIDEO_FILE_MPEG, NULL));

	authorg_video_file_construct (file, filename);

	return file;
}
