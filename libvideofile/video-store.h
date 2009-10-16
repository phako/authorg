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


#ifndef _VIDEO_STORE_H
#define _VIDEO_STORE_H
#define AUTHORG_TYPE_VIDEO_STORE (authorg_video_store_get_type())
#define AUTHORG_VIDEO_STORE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTHORG_TYPE_VIDEO_STORE, AuthorgVideoStore))
#define AUTHORG_VIDEO_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), AUTHORG_TYPE_VIDEO_STORE, AuthorgVideoStoreClass))
#define AUTHORG_IS_VIDEO_STORE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTHORG_TYPE_VIDEO_STORE))
#define AUTHORG_IS_VIDEO_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), AUTHORG_TYPE_VIDEO_STORE))
#define AUTHORG_VIDEO_STORE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), AUTHORG_TYPE_VIDEO_STORE, AuthorgVideoStoreClass))

#include <gtk/gtktreemodel.h>

#include "video-file.h"

typedef struct AuthorgVideoStore AuthorgVideoStore;
typedef struct AuthorgVideoStoreClass AuthorgVideoStoreClass;
typedef struct AuthorgVideoStorePrivate AuthorgVideoStorePrivate;

enum AUTHORG_COLUMN_TYPES
{
	AUTHORG_VIDEO_COLUMN_THUMBNAIL = 0,
	AUTHORG_VIDEO_COLUMN_MARKUP,
	AUTHORG_VIDEO_COLUMN_DATA,
	AUTHORG_VIDEO_COLUMN_COUNT,
};

struct AuthorgVideoStore {
	GObject parent;
	guint num_rows;
	
	gint n_columns;
	GType column_types[AUTHORG_VIDEO_COLUMN_COUNT];

	gint stamp;
	AuthorgVideoStorePrivate *_priv;
};

struct AuthorgVideoStoreClass {
	GObjectClass parent;
};

GType authorg_video_store_get_type (void);
AuthorgVideoStore *authorg_video_store_new (void);
AuthorgVideoFile *authorg_video_store_nth (AuthorgVideoStore *store, gint n);
gint authorg_video_store_index (AuthorgVideoStore *store, AuthorgVideoFile *file);
AuthorgVideoFile *authorg_video_store_next (AuthorgVideoStore *store, AuthorgVideoFile *file);
void authorg_video_store_append (AuthorgVideoStore *store, AuthorgVideoFile *file);
void authorg_video_store_to_dvdauthor_fd (AuthorgVideoStore *store, gint fd);
void authorg_video_store_to_dvdauthor (AuthorgVideoStore *store, const gchar *filename);
void authorg_video_store_foreach (AuthorgVideoStore *store, GFunc func, gpointer user_data);
void authorg_video_store_remove (AuthorgVideoStore *store, GtkTreeIter *iter);
gboolean authorg_video_store_is_empty (AuthorgVideoStore *store);
void authorg_video_store_insert_after (AuthorgVideoStore *store, AuthorgVideoFile *dest, AuthorgVideoFile *src);
void authorg_video_store_prepend (AuthorgVideoStore *store, AuthorgVideoFile *file);
#endif /* _VIDEO_STORE_H */
