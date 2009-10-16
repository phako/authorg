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

#ifndef _VIDEO_FILE_MPEG_H
#define _VIDEO_FILE_MPEG_H

#include <glib-object.h>
#include <glib.h>

#include "video-file.h"

#define AUTHORG_TYPE_VIDEO_FILE_MPEG (authorg_video_file_mpeg_get_type())
#define AUTHORG_VIDEO_FILE_MPEG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTHORG_TYPE_VIDEO_FILE_MPEG, AuthorgVideoFileMpeg))
#define AUTHORG_VIDEO_FILE_MPEG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), AUTHORG_TYPE_VIDEO_FILE_MPEG, AuthorgVideoFileMpegClass))
#define AUTHORG_IS_VIDEO_FILE_MPEG(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTHORG_TYPE_VIDEO_FILE_MPEG))
#define AUTHORG_IS_VIDEO_FILE_MPEG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), AUTHORG_TYPE_VIDEO_FILE_MPEG))
#define AUTHORG_VIDEO_FILE_MPEG_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), AUTHORG_TYPE_VIDEO_FILE_MPEG, VideoFileMpegClass))

typedef struct AuthorgVideoFileMpeg AuthorgVideoFileMpeg;
typedef struct AuthorgVideoFileMpegClass AuthorgVideoFileMpegClass;
typedef struct AuthorgVideoFileMpegPrivate AuthorgVideoFileMpegPrivate;

struct AuthorgVideoFileMpeg {
		AuthorgVideoFile parent;
};

struct AuthorgVideoFileMpegClass {
		AuthorgVideoFileClass parent;
};

GType authorg_video_file_mpeg_get_type (void);
AuthorgVideoFile *authorg_video_file_mpeg_new (const gchar *filename);
#endif /* _VIDEO_FILE_MPEG_H */
