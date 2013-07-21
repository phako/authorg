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

#include <unistd.h>
#include <fcntl.h>

#include <glib/gi18n.h>
#include <glib-object.h>
#include <glib/gstdio.h>

#include <gtk/gtktreemodel.h>
#include <gtk/gtktreednd.h>

#include "video-file.h"
#include "video-store.h"

struct AuthorgVideoStorePrivate
{
	GList *data;
};


/* local functions */
static void authorg_tree_model_init (GtkTreeModelIface *iface);
static void authorg_video_store_finalize (GObject *object);

/* TreeModel interface stuff */
static GtkTreeModelFlags authorg_video_store_get_flags  (GtkTreeModel      *tree_model);
static gint authorg_video_store_get_n_columns (GtkTreeModel *tree_model);
static GType authorg_video_store_get_column_type (GtkTreeModel *tree_model, gint index);
static gboolean authorg_video_store_get_iter (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path);
static GtkTreePath *authorg_video_store_get_path (GtkTreeModel *tree_model, GtkTreeIter *iter);
static void authorg_video_store_get_value (GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, GValue *value);
static gboolean authorg_video_store_iter_next (GtkTreeModel *tree_model, GtkTreeIter *iter);
static gboolean authorg_video_store_iter_children (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent);
static gboolean authorg_video_store_iter_has_child (GtkTreeModel *tree_model, GtkTreeIter *iter);
static gint authorg_video_store_iter_n_children (GtkTreeModel *tree_model, GtkTreeIter *iter);
static gboolean authorg_video_store_iter_nth_child (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent, gint n);
static gboolean authorg_video_store_iter_parent (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *child);

/* forward declaration of TreeDragDest iface */
static void authorg_tree_drag_dest_init (GtkTreeDragDestIface *iface);
static gboolean authorg_video_store_drag_data_received (GtkTreeDragDest *drag_dest, GtkTreePath *dest, GtkSelectionData *selection_data);
static gboolean authorg_video_store_row_drop_possible (GtkTreeDragDest *drag_dest, GtkTreePath *dest, GtkSelectionData *selection_data);

/*static void authorg_tree_drag_source_init (GtkTreeDragSourceIface *iface); */

G_DEFINE_TYPE_EXTENDED (AuthorgVideoStore,
				authorg_video_store,
				G_TYPE_OBJECT,
				0,
				G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL, authorg_tree_model_init)
				G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_DRAG_DEST, authorg_tree_drag_dest_init)
/*				G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_DRAG_SOURCE, authorg_tree_drag_source_init)*/
				);

static void
authorg_video_store_finalize (GObject *object)
{
	AuthorgVideoStore *video_store = AUTHORG_VIDEO_STORE (object);

	g_return_if_fail (object != NULL);

	if (video_store->_priv->data)
	{
		g_list_foreach (video_store->_priv->data, (GFunc)g_object_unref, NULL);
		g_list_free (video_store->_priv->data);
	}

	g_free (video_store->_priv);

	if (G_OBJECT_CLASS (authorg_video_store_parent_class)->finalize)
		G_OBJECT_CLASS (authorg_video_store_parent_class)->finalize (object);
}

static void
authorg_video_store_class_init (AuthorgVideoStoreClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = authorg_video_store_finalize;
}

/* TreeModel interface stuff */

static void
authorg_tree_model_init (GtkTreeModelIface *iface)
{
	iface->get_flags = authorg_video_store_get_flags;
	iface->get_n_columns = authorg_video_store_get_n_columns;
	iface->get_column_type = authorg_video_store_get_column_type;
	iface->get_iter        = authorg_video_store_get_iter;
	iface->get_path        = authorg_video_store_get_path;
	iface->get_value       = authorg_video_store_get_value;
	iface->iter_next       = authorg_video_store_iter_next;
	iface->iter_children   = authorg_video_store_iter_children;
	iface->iter_has_child  = authorg_video_store_iter_has_child;
	iface->iter_n_children = authorg_video_store_iter_n_children;
	iface->iter_nth_child  = authorg_video_store_iter_nth_child;
	iface->iter_parent     = authorg_video_store_iter_parent;
}

static void
authorg_video_store_init (AuthorgVideoStore *store)
{
	store->_priv = g_new0(AuthorgVideoStorePrivate, 1);

	store->n_columns = AUTHORG_VIDEO_COLUMN_COUNT;

	store->column_types[AUTHORG_VIDEO_COLUMN_THUMBNAIL] = GDK_TYPE_PIXBUF;
	store->column_types[AUTHORG_VIDEO_COLUMN_MARKUP] = G_TYPE_STRING;
	store->column_types[AUTHORG_VIDEO_COLUMN_DATA] = AUTHORG_TYPE_VIDEO_FILE;

	store->num_rows = 0;
	store->_priv->data = NULL;
	store->stamp = g_random_int ();
}

static GtkTreeModelFlags
authorg_video_store_get_flags (GtkTreeModel *tree_model)
{
	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (tree_model), 0);

	return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
}

static gint
authorg_video_store_get_n_columns (GtkTreeModel *tree_model)
{
	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (tree_model), 0);

	return (AUTHORG_VIDEO_STORE (tree_model)->n_columns);
}

static GType
authorg_video_store_get_column_type (GtkTreeModel *tree_model, gint index)
{
	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE(tree_model), G_TYPE_INVALID);
	g_return_val_if_fail (index < AUTHORG_VIDEO_STORE(tree_model)->n_columns && index >= 0, G_TYPE_INVALID);

	return AUTHORG_VIDEO_STORE(tree_model)->column_types[index];
}

static gboolean
authorg_video_store_get_iter (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path)
{
	AuthorgVideoStore *store;
	gint *indices, n, depth;
	AuthorgVideoFile *video_file = NULL;

	g_assert (AUTHORG_IS_VIDEO_STORE (tree_model));
	g_assert (path != NULL);

	store = AUTHORG_VIDEO_STORE (tree_model);
	indices = gtk_tree_path_get_indices (path);
	depth = gtk_tree_path_get_depth (path);

	g_assert (depth == 1);

	n = indices[0];

	if (n >= store->num_rows || n < 0)
		return FALSE;

	video_file = authorg_video_store_nth (store, n);

	g_assert (video_file != NULL);

	iter->stamp = store->stamp;
	iter->user_data = video_file;
	iter->user_data2 = NULL;
	iter->user_data3 = NULL;
	
	return TRUE;
}

static GtkTreePath *
authorg_video_store_get_path (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	GtkTreePath *path;
	AuthorgVideoStore *store;
	
	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (tree_model), NULL);
	g_return_val_if_fail (iter != NULL, NULL);
	g_return_val_if_fail (iter->user_data != NULL, NULL);

	store = AUTHORG_VIDEO_STORE (tree_model);

	path = gtk_tree_path_new ();
	gtk_tree_path_append_index (path,
			authorg_video_store_index (store, AUTHORG_VIDEO_FILE (iter->user_data)));

	return path;
}

static void
authorg_video_store_get_value (GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, GValue *value)
{
	AuthorgVideoStore *store;
	AuthorgVideoFile *file;

	g_return_if_fail (AUTHORG_IS_VIDEO_STORE (tree_model));
	g_return_if_fail (iter != NULL);

	store = AUTHORG_VIDEO_STORE (tree_model);
	g_return_if_fail (column < store->n_columns);

	g_value_init (value, store->column_types[column]);
	
	file = iter->user_data;

	g_return_if_fail (iter != NULL);

	if (authorg_video_store_index (store, file) >= store->num_rows)
		g_return_if_reached ();

	switch (column)
	{
		case AUTHORG_VIDEO_COLUMN_THUMBNAIL:
			g_value_set_object (value, authorg_video_file_get_thumbnail(file));
			break;

		case AUTHORG_VIDEO_COLUMN_MARKUP:
			g_value_set_string (value, authorg_video_file_get_info_as_markup(file));
			break;

		case AUTHORG_VIDEO_COLUMN_DATA:
			g_value_set_object (value, file);
			break;
	}
}

static gboolean
authorg_video_store_iter_next (GtkTreeModel *model, GtkTreeIter *iter)
{
	AuthorgVideoStore *store;
	AuthorgVideoFile *file, *newfile;
	
	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (model), FALSE);

	if (iter == NULL || iter->user_data == NULL)
		return FALSE;

	store = AUTHORG_VIDEO_STORE (model);

	file = iter->user_data;
	newfile = authorg_video_store_next (store, file);
	if (newfile == NULL)
		return FALSE;

	iter->stamp = store->stamp;
	iter->user_data = newfile;

	return TRUE;
}

static gboolean
authorg_video_store_iter_children (GtkTreeModel *tree_model, GtkTreeIter  *iter, GtkTreeIter  *parent)
{
	AuthorgVideoStore *store;

	g_return_val_if_fail (parent == NULL || parent->user_data != NULL, FALSE);

	if (parent)
		return FALSE;

	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (tree_model), FALSE);

	store = AUTHORG_VIDEO_STORE (tree_model);

	if (store->num_rows == 0)
		return FALSE;

	iter->stamp = store->stamp;
	iter->user_data = authorg_video_store_nth (store, 0);

	return TRUE;
}

static gboolean
authorg_video_store_iter_has_child (GtkTreeModel *model, GtkTreeIter *iter)
{
	return FALSE;
}

static gint
authorg_video_store_iter_n_children (GtkTreeModel *model, GtkTreeIter *iter)
{
	AuthorgVideoStore *store;

	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (model), -1);
	g_return_val_if_fail (iter == NULL || iter->user_data != NULL, -1);

	store = AUTHORG_VIDEO_STORE (model);

	if (!iter)
		return store->num_rows;

	return 0;
}

static gboolean
authorg_video_store_iter_nth_child (GtkTreeModel *model, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	AuthorgVideoStore *store;
	AuthorgVideoFile *file;

	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (model), FALSE);

	store = AUTHORG_VIDEO_STORE (model);

	if (parent)
		return FALSE;

	if (n >= store->num_rows)
		return FALSE;

	file = authorg_video_store_nth (store, 0);

	g_assert (file != NULL);

	iter->stamp = store->stamp;
	iter->user_data = file;

	return TRUE;
}

static gboolean
authorg_video_store_iter_parent (GtkTreeModel *m, GtkTreeIter *i, GtkTreeIter *c)
{
	return FALSE;
}

/* TreeDragDest interface */
static void
authorg_tree_drag_dest_init (GtkTreeDragDestIface *iface)
{
	iface->drag_data_received = authorg_video_store_drag_data_received;
	iface->row_drop_possible = authorg_video_store_row_drop_possible;
}


static gboolean 
authorg_video_store_drag_data_received (GtkTreeDragDest *drag_dest, GtkTreePath *dest, GtkSelectionData *selection_data)
{
	GtkTreeModel *tree_model;
	AuthorgVideoStore *video_store;
	GtkTreeModel *src_model = NULL;
	GtkTreePath *src_path = NULL;
	gboolean retval = FALSE;

	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (drag_dest), FALSE);

	tree_model = GTK_TREE_MODEL (drag_dest);
	video_store = AUTHORG_VIDEO_STORE (drag_dest);

	if (gtk_tree_get_row_drag_data (selection_data,
				&src_model, &src_path) && src_model == tree_model)
	{
		GtkTreeIter src_iter;
		GtkTreeIter dest_iter;
		GtkTreePath *prev;

		if (!gtk_tree_model_get_iter (src_model,
					&src_iter, src_path))
		{
			goto out;
		}

		prev = gtk_tree_path_copy (dest);

		if (! gtk_tree_path_prev (prev))
		{
			authorg_video_store_prepend (video_store,
					AUTHORG_VIDEO_FILE (src_iter.user_data));
			retval = TRUE;
		}
		else
		{
			if (gtk_tree_model_get_iter (tree_model, &dest_iter, prev))
			{
				authorg_video_store_insert_after (video_store,
						AUTHORG_VIDEO_FILE (dest_iter.user_data),
						AUTHORG_VIDEO_FILE (src_iter.user_data));
				retval = TRUE;
			}
		}

		gtk_tree_path_free (prev);
		
	}
out:
	if (src_path)
		gtk_tree_path_free (src_path);

	return retval;
}

static gboolean
authorg_video_store_row_drop_possible (GtkTreeDragDest *drag_dest, GtkTreePath *dest, GtkSelectionData *selection_data)
{
	return TRUE;
}

/* TreeDragSource interface */
/*static void
authorg_video_store_tree_drag_source_init (GtkTreeDragSourceIface *iface)
{
}
*/
/* public functions */

/**
 * authorg_video_store_new:
 *
 * Creates a new storage for video files.
 *
 * Returns: a newly created #AuthorgVideoStore
 */
AuthorgVideoStore *
authorg_video_store_new ()
{
	AuthorgVideoStore *store;

	store = AUTHORG_VIDEO_STORE (g_object_new (AUTHORG_TYPE_VIDEO_STORE, NULL));

	g_assert (store != NULL);

	return store;
}

/**
 * authorg_video_store_nth:
 * @store: the #AuthorgVideoStore to query
 * @n: Index of element to retrieve
 *
 * Retrieves the nth element of an #AuthorgVideoStore
 *
 * Returns: the nth stored #AuthorgVideoFile or NULL, if n is too small or too
 * large
 */
AuthorgVideoFile *
authorg_video_store_nth (AuthorgVideoStore *store, gint n)
{
	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (store), NULL);
	g_return_val_if_fail (store->_priv->data != NULL, NULL);
	g_return_val_if_fail (n >= 0 && n <= store->num_rows, NULL);

	return AUTHORG_VIDEO_FILE (g_list_nth_data (store->_priv->data, n));
}

gint
authorg_video_store_index (AuthorgVideoStore *store, AuthorgVideoFile *file)
{
	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (store), -1);
	g_return_val_if_fail (AUTHORG_IS_VIDEO_FILE (file), -1);
	g_return_val_if_fail (store->_priv->data != NULL, -1);

	return g_list_index (store->_priv->data, file);
}

AuthorgVideoFile *
authorg_video_store_next (AuthorgVideoStore *store, AuthorgVideoFile *file)
{
	GList *tmp;
	
	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (store), NULL);
	g_return_val_if_fail (AUTHORG_IS_VIDEO_FILE (file), NULL);
	g_return_val_if_fail (store->_priv->data != NULL, NULL);

	tmp = g_list_find (store->_priv->data, file);

	if (tmp == NULL || tmp->next == NULL)
		return NULL;

	return AUTHORG_VIDEO_FILE (tmp->next->data);
}

static void 
_emit_changed (AuthorgVideoFile *file, GdkPixbuf *thumbnail, AuthorgVideoStore *store)
{
	gint row;
	GtkTreePath *path;
	GtkTreeIter iter;
	
	row = authorg_video_store_index (store, file);

	path = gtk_tree_path_new ();
	gtk_tree_path_append_index (path, row);
	iter.stamp = store->stamp;
	iter.user_data = file;
	
	gtk_tree_model_row_changed (GTK_TREE_MODEL (store), path, &iter);
	gtk_tree_path_free (path);
}

void
authorg_video_store_append (AuthorgVideoStore *store, AuthorgVideoFile *file)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	
	g_return_if_fail (AUTHORG_IS_VIDEO_STORE (store));
	g_return_if_fail (AUTHORG_IS_VIDEO_FILE (file));

	store->_priv->data = g_list_append (store->_priv->data, file);

	g_signal_connect (G_OBJECT (file), "got-icon",
			G_CALLBACK (_emit_changed), store);
	
	path = gtk_tree_path_new ();
	gtk_tree_path_append_index (path, store->num_rows);
	iter.stamp = store->stamp;
	iter.user_data = file;

	store->num_rows++;

	gtk_tree_model_row_inserted (GTK_TREE_MODEL (store), path, &iter);
	gtk_tree_path_free (path);
}

void
authorg_video_store_remove (AuthorgVideoStore *store, GtkTreeIter *iter)
{
	GtkTreePath *path;
	
	g_return_if_fail (AUTHORG_IS_VIDEO_STORE (store));
	g_return_if_fail (iter != NULL && iter->user_data != NULL);
	g_return_if_fail (AUTHORG_IS_VIDEO_FILE (iter->user_data));
	
	path = authorg_video_store_get_path (GTK_TREE_MODEL (store), iter);
	store->_priv->data = g_list_remove (store->_priv->data, iter->user_data);
	store->num_rows--;

	gtk_tree_model_row_deleted (GTK_TREE_MODEL (store), path);
}

static void
_vf_chapter_list (AuthorgVideoChapter *chapter, GString *author)
{
	gchar *chapter_str;
	
	chapter_str = g_strdup_printf ("%02d:%02d:%02d,", 
			chapter->timecode / 60 / 60, 
			(chapter->timecode / 60) % 60, 
			chapter->timecode % 60);
	g_print ("Adding chapter %s\n", chapter_str);

	g_string_append (author, chapter_str);
	g_free (chapter_str);
}

static void
_vf_to_pgc (AuthorgVideoFile *file, GString *author)
{
	g_string_append (author, "<pgc><vob file=\"");
	g_string_append (author, file->filename);
	if (authorg_video_file_has_chapters (file))
	{
		g_string_append (author, "\" chapters=\"");
		authorg_video_file_chapters_foreach (file, (GFunc)_vf_chapter_list, author);

		/* remove last "," */
		*author = *(g_string_truncate (author, author->len -1));
	}
	g_string_append (author, "\" /></pgc>");
}

/**
 * authorg_video_store_to_dvdauthor_fd:
 * @store: The #AuthorgVideoStore to dump
 * @fd: an open and valid writeable file desctriptor.
 *
 * Writes an XML file suitable to be processed by dvdauthor to a given
 * file descriptor. It is meant to be used with g_file_open_tmp.
 * Note that this function does not close the file.
 */
void
authorg_video_store_to_dvdauthor_fd (AuthorgVideoStore *store, gint fd)
{
	GString *dvdauthor;
	int res;

	dvdauthor = g_string_new ("<dvdauthor>");
	g_string_append (dvdauthor, "<vmgm /><titleset><titles>");

	authorg_video_store_foreach (store,
			(GFunc)_vf_to_pgc, dvdauthor);

	g_string_append (dvdauthor, "</titles></titleset></dvdauthor>");

	res = write (fd, dvdauthor->str, dvdauthor->len);
	if (res != 0) {
		g_warning ("Failed to write to file");
	}

	g_string_free (dvdauthor, TRUE);
}

void
authorg_video_store_to_dvdauthor (AuthorgVideoStore *store, const gchar *filename)
{
	gint fd;
	
	if (!g_file_test (filename, G_FILE_TEST_EXISTS))
	{
		fd = g_open (filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		authorg_video_store_to_dvdauthor_fd (store, fd);
		close (fd);
	}
}

void
authorg_video_store_foreach (AuthorgVideoStore *store, GFunc func, gpointer user_data)
{
	g_return_if_fail (AUTHORG_IS_VIDEO_STORE (store));

	g_list_foreach (store->_priv->data, func, user_data);
}

gboolean
authorg_video_store_is_empty (AuthorgVideoStore *store)
{
	g_return_val_if_fail (AUTHORG_IS_VIDEO_STORE (store), TRUE);

	return store->num_rows < 1;
}

void
authorg_video_store_prepend (AuthorgVideoStore *store, AuthorgVideoFile *file)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	store->_priv->data = g_list_prepend (store->_priv->data, file);
	
	path = gtk_tree_path_new ();

	gtk_tree_path_append_index (path, 0);

	gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);

	gtk_tree_model_row_inserted (GTK_TREE_MODEL (store), path, &iter);
}

void
authorg_video_store_insert_after (AuthorgVideoStore *store,
		AuthorgVideoFile *dest, AuthorgVideoFile *src)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *sibling;

	sibling = g_list_next (g_list_find (store->_priv->data, dest));
	store->_priv->data = g_list_insert_before (store->_priv->data, sibling, src);
	iter.stamp = store->stamp;
	iter.user_data = src;

	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &iter);
	gtk_tree_model_row_inserted (GTK_TREE_MODEL (store), path, &iter);

	gtk_tree_path_free (path);
}
