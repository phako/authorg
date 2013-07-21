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

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <gdk/gdk.h>


#include "video-file.h"
#include "empty.xpm"

enum signals
{
	GOT_ICON,
	LAST_SIGNAL
};

static guint authorg_video_file_signals[LAST_SIGNAL] = {0};

struct AuthorgVideoFilePrivate
{
	GdkPixbuf *thumbnail;
	gchar *tmpthumb;
	gint tmpfd;
};

G_DEFINE_TYPE (AuthorgVideoFile, authorg_video_file, G_TYPE_OBJECT);

static void
authorg_video_file_init (AuthorgVideoFile *video)
{
	video->_priv = g_new0 (AuthorgVideoFilePrivate, 1);
}

static void
authorg_video_file_finalize (GObject *object)
{
	AuthorgVideoFile *video = AUTHORG_VIDEO_FILE (object);

	g_return_if_fail (object != NULL);

	if (video->filename)
		g_free (video->filename);

	if (video->chapters)
		g_list_free (video->chapters);

	if (video->_priv->thumbnail)
		g_object_unref (video->_priv->thumbnail);

	g_free (video->_priv);

	if (G_OBJECT_CLASS (authorg_video_file_parent_class)->finalize)
		G_OBJECT_CLASS (authorg_video_file_parent_class)->finalize (object);
}

static void
base_authorg_video_file_info (AuthorgVideoFile *file)
{
	struct stat statbuf;

	/* Get size via stat */
	g_stat (file->filename, &statbuf);

	file->size = statbuf.st_size;
}

static void
authorg_video_file_class_init (AuthorgVideoFileClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = authorg_video_file_finalize;

	klass->info = base_authorg_video_file_info;

	/* setup signals */
	authorg_video_file_signals[GOT_ICON] = 
			g_signal_new ("got-icon",
							G_TYPE_FROM_CLASS (klass),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET (AuthorgVideoFileClass, got_icon),
							NULL, NULL,
							g_cclosure_marshal_VOID__POINTER,
							G_TYPE_NONE,
							1, G_TYPE_POINTER);
}

void
authorg_video_file_read_thumbnail (GPid pid, gint status, gpointer data)
{
	AuthorgVideoFile *file = AUTHORG_VIDEO_FILE (data);

	if (WIFEXITED(status) && !WEXITSTATUS(status))
	{
		/* success */

		if (file->_priv->thumbnail)
			g_object_unref (file->_priv->thumbnail);

		file->_priv->thumbnail = gdk_pixbuf_new_from_file (
				file->_priv->tmpthumb, NULL);

		g_object_ref (file->_priv->thumbnail);

		g_signal_emit (G_OBJECT (file),
				authorg_video_file_signals[GOT_ICON],
				0,
				file->_priv->thumbnail);
	}
	close (file->_priv->tmpfd);
	g_unlink (file->_priv->tmpthumb);

	g_spawn_close_pid (pid);
}

void
authorg_video_file_create_tumbnail(AuthorgVideoFile *file)
{
	GPid pid;
	char *argv[4];
	GError *error = NULL;
	gint fd;
	char *name;

	/* create temporary file */
	fd = g_file_open_tmp ("authorgXXXXXX", &name, NULL);
	
	/* spawn process */
	argv[0] = "/bin/sh";
	argv[1] = "-c";
	argv[2] = g_strdup_printf ("%s -s 64 %s %s",
					TOTEM_VIDEO_THUMBNAILER,
					g_shell_quote (file->filename),
					g_shell_quote (name));
	argv[3] = NULL; 
	
	if (g_spawn_async (g_get_tmp_dir(),
			argv,
			NULL,
			G_SPAWN_DO_NOT_REAP_CHILD | 
			G_SPAWN_STDOUT_TO_DEV_NULL | 
			G_SPAWN_STDERR_TO_DEV_NULL,
			NULL,
			NULL,
			&pid,
			&error))
	{
		file->_priv->tmpthumb = name;
		file->_priv->tmpfd = fd;
		g_child_watch_add (pid, authorg_video_file_read_thumbnail, file);
	}
	else
	{
		g_print("Could not spawn : %s\n", error->message);
	}

	g_free (argv[2]);
}

static void
authorg_video_file_create_chapter_thumb (AuthorgVideoFile *file, AuthorgVideoChapter *chapter)
{
}

/* public functions */

void
authorg_video_file_construct (AuthorgVideoFile *file, const gchar *filename)
{
	file->filename = g_strdup (filename);
	file->_priv->thumbnail = gdk_pixbuf_new_from_xpm_data ((const char **)empty);

	/* gather information from file */
	authorg_video_file_info (file);

	/* spawn thumbnail creation */
	authorg_video_file_create_tumbnail (file);
}

gchar *
authorg_video_file_get_info_as_markup(AuthorgVideoFile *file)
{
	gchar *res, *tmp;

	guint h, m, s;

	s = file->length;
	m = s / 60;
	s %= 60;
	h = m / 60;
	m %= 60;

	res = g_strdup_printf (
			_("<b>%s</b>\n<i>Length: %02d:%02d:%02d</i>"),
			tmp = g_path_get_basename (file->filename),
			h,m,s);

	g_free (tmp);
	return res;
}

GdkPixbuf *
authorg_video_file_get_thumbnail(AuthorgVideoFile *file)
{
	if (file->_priv->thumbnail)
	{
		g_object_ref (file->_priv->thumbnail);
		return file->_priv->thumbnail;
	}
	else
		return NULL;
}

void
authorg_video_file_info (AuthorgVideoFile *file)
{
	AUTHORG_VIDEO_FILE_GET_CLASS (G_OBJECT (file))->info (file);
}

gchar *
authorg_video_file_get_name (AuthorgVideoFile *file)
{
	return g_strdup (file->filename);
}

/**
 * authorg_video_file_create_chapters:
 * 
 * Adds a chapter to the file every @interval seconds
 */
void
authorg_video_file_create_chapters (AuthorgVideoFile *file, guint interval)
{
	GList *chapters = NULL;
	guint c = 0;
	AuthorgVideoChapter *chapter;

	g_return_if_fail (AUTHORG_IS_VIDEO_FILE (file));

	if (file->chapters)
	{
		g_list_free (file->chapters);
		file->chapters = NULL;
	}
	

	for (c = 0; c < file->length; c += interval)
	{
		chapter = g_new0(AuthorgVideoChapter, 1);
		chapter->timecode = c;
	//	authorg_video_file_create_chapter_thumb (file, chapter);
		chapters = g_list_prepend (chapters, chapter);
	}

	chapters = g_list_reverse (chapters);

	file->chapters = chapters;
}

void
authorg_video_file_add_chapter (AuthorgVideoFile *file, guint chapter_timecode)
{
	AuthorgVideoChapter *chapter;

	g_return_if_fail (AUTHORG_IS_VIDEO_FILE (file));

	if (chapter_timecode > file->length)
		return;

	chapter = g_new0(AuthorgVideoChapter, 1);
	chapter->timecode = chapter_timecode;
	authorg_video_file_create_chapter_thumb (file, chapter);
//	file->chapters = g_list_insert_sorted (file->chapters, GINT_TO_POINTER (chapter));
}

void
authorg_video_file_remove_chapter (AuthorgVideoFile *file, guint chapter)
{
	file->chapters = g_list_remove (file->chapters, GINT_TO_POINTER (chapter));
}

void
authorg_video_file_chapters_foreach (AuthorgVideoFile *file, GFunc func, gpointer user_data)
{
	if (file->chapters)
		g_list_foreach (file->chapters, func, user_data);
}

gboolean
authorg_video_file_has_chapters (AuthorgVideoFile *file)
{
	return g_list_length (file->chapters) != 0;
}
