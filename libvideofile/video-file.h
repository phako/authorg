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


#ifndef _VIDEO_FILE_H
#define _VIDEO_FILE_H

#include <glib-object.h>
#include <glib.h>
#include <gdk/gdkpixbuf.h>

#define AUTHORG_TYPE_VIDEO_FILE (authorg_video_file_get_type())
#define AUTHORG_VIDEO_FILE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTHORG_TYPE_VIDEO_FILE, AuthorgVideoFile))
#define AUTHORG_VIDEO_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), AUTHORG_TYPE_VIDEO_FILE, AuthorgVideoFileClass))
#define AUTHORG_IS_VIDEO_FILE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTHORG_TYPE_VIDEO_FILE))
#define AUTHORG_IS_VIDEO_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), AUTHORG_TYPE_VIDEO_FILE))
#define AUTHORG_VIDEO_FILE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), AUTHORG_TYPE_VIDEO_FILE, AuthorgVideoFileClass))

typedef struct AuthorgVideoFile AuthorgVideoFile;
typedef struct AuthorgVideoChapter AuthorgVideoChapter;
typedef struct AuthorgVideoFileClass AuthorgVideoFileClass;
typedef struct AuthorgVideoFilePrivate AuthorgVideoFilePrivate;

struct AuthorgVideoChapter {
	guint timecode;				/* time in seconds */
	GdkPixbuf *thumbnail;		/* thumbnail of the chapter */
};

struct AuthorgVideoFile {
	GObject parent;

	/*<protected>*/
	gchar *filename;
	guint length;
	GList *chapters;
	guint size;

	AuthorgVideoFilePrivate *_priv;
};

struct AuthorgVideoFileClass {
	GObjectClass parent;

	/*<virtual functions>*/
	void	(*info)		(AuthorgVideoFile *file);

	/*<signals>*/
	void	(*got_icon)	(AuthorgVideoFile *file, GdkPixbuf *icon, gpointer user_data);
//	void	(*got_size) (AuthorgVideoFile *file, gpointer user_data);
};

GType authorg_video_file_get_type (void);

void authorg_video_file_construct (AuthorgVideoFile *file, const gchar *filename);
gchar *authorg_video_file_get_info_as_markup (AuthorgVideoFile *file);
GdkPixbuf *authorg_video_file_get_thumbnail (AuthorgVideoFile *file);
void authorg_video_file_info (AuthorgVideoFile *file);
gchar *authorg_video_file_get_name (AuthorgVideoFile *file);
void authorg_video_file_create_chapters (AuthorgVideoFile *file, guint interval);
void authorg_video_file_remove_chapter (AuthorgVideoFile *file, guint chapter);
void authorg_video_file_chapters_foreach (AuthorgVideoFile *file, GFunc func, gpointer user_data);
gboolean authorg_video_file_has_chapters (AuthorgVideoFile *file);
#endif /* _VIDEO_FILE_H */
