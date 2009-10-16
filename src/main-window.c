/*  Authorg - a GNOME DVD authoring tool
 *
 *  Copyright (C) 2005 Jens Georg <mail@jensge.org>
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

#include <glib.h>
#include <glib/gi18n.h>

#include <gdk/gdkkeysyms.h>

#include <gtk/gtk.h>

#include <gconf/gconf-client.h>

#ifdef HAVE_GNOME_VFS
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#endif

#include <video-file.h>
#include <video-file-mpeg.h>
#include <video-store.h>

#include "about.h"
#include "authorg.h"
#include "chapter-editor.h"
#include "main-window.h"
#include "spawn-dialog.h"
#include "util.h"

struct AuthorgMainWindowPrivate
{
	GtkWidget *tree_view;
    GConfClient *gc;
};

G_DEFINE_TYPE (AuthorgMainWindow, authorg_main_window, GTK_TYPE_WINDOW);

static void
authorg_main_window_drag_data_received (GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time,
                        gpointer userdata);

static void
authorg_main_window_init (AuthorgMainWindow *main_window)
{
	main_window->_priv = g_new0 (AuthorgMainWindowPrivate, 1);
}

static void
authorg_main_window_finalize (GObject *object)
{
	g_return_if_fail (object != NULL);

	if (G_OBJECT_CLASS (authorg_main_window_parent_class)->finalize != NULL) {
			(* G_OBJECT_CLASS (authorg_main_window_parent_class)->finalize) (object);
	}
}

static void
authorg_main_window_quit ()
{
	gtk_main_quit ();
}

static void
authorg_main_window_cb_create_iso (GtkWidget *w, AuthorgMainWindow *main_window)
{
	/* get directory to create into */
	GtkWidget *dialog;
	gchar *fname, *folder;

	dialog = gtk_file_chooser_dialog_new (_("Select name for ISO file"),
			GTK_WINDOW (main_window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			_("_Generate"), GTK_RESPONSE_OK,
			NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	{
		GError *error = NULL;

		gtk_widget_hide (dialog);
		fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		folder = gconf_client_get_string (main_window->gconf_client,
						AUTHORG_GCONF_DIR "/iso_dir", NULL);
		
		if (authorg_util_run_dvdauthor (
				folder, 
				AUTHORG_VIDEO_STORE (gtk_tree_view_get_model (
						GTK_TREE_VIEW (main_window->_priv->tree_view))), 
				&error))
		{
			if (authorg_util_run_mkisofs (folder, fname, &error))
			{
				GtkWidget *msg;
				
				msg = gtk_message_dialog_new (
						GTK_WINDOW (main_window),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_INFO,
						GTK_BUTTONS_NONE,
						_("Successfully created ISO image."));
				
				gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (msg),
						"%s", fname);
					
				gtk_dialog_add_buttons (GTK_DIALOG (msg),
						GTK_STOCK_HELP, GTK_RESPONSE_HELP,
						GTK_STOCK_OPEN, GTK_RESPONSE_YES,
						GTK_STOCK_OK, GTK_RESPONSE_OK,
						NULL);
				gtk_dialog_set_default_response (GTK_DIALOG (msg),
						GTK_RESPONSE_OK);

				/* TODO: Connect to response for help */
				
				if (gtk_dialog_run (GTK_DIALOG (msg)) == GTK_RESPONSE_YES)
				{
					gchar *dir;

					dir = g_path_get_dirname (fname);
					authorg_util_launch_file_manager (dir);
					g_free (dir);
				}
				gtk_widget_destroy (msg);
			}
			
		}
		g_free (folder);
		g_free (fname);
	}

	gtk_widget_destroy (dialog);
}

static void
authorg_main_window_cb_run_dvdauthor (GtkWidget *w, AuthorgMainWindow *main_window)
{
	/* get directory to create into */
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new (_("Select directory for DVD structure"),
			GTK_WINDOW (main_window),
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			_("_Generate"), GTK_RESPONSE_OK,
			NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	{
		GError *error = NULL;
		gchar *folder;

		gtk_widget_hide (dialog);

		folder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog));

		if (authorg_util_run_dvdauthor (
				folder, AUTHORG_VIDEO_STORE (gtk_tree_view_get_model(
							GTK_TREE_VIEW (main_window->_priv->tree_view))),
						&error))
		{
			GtkWidget *message = gtk_message_dialog_new (GTK_WINDOW (main_window),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_NONE,
					_("Successfully created a DVD structure in %s"),
					folder);

			gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (message),
					_("You may now examine the folder in the filemanager"));

			gtk_dialog_add_buttons (GTK_DIALOG (message),
					GTK_STOCK_HELP, GTK_RESPONSE_HELP,
					GTK_STOCK_OPEN, GTK_RESPONSE_YES,
					GTK_STOCK_OK, GTK_RESPONSE_OK, 
					NULL);

			gtk_dialog_set_default_response (GTK_DIALOG (dialog),
					GTK_RESPONSE_OK);
			
			if (gtk_dialog_run (GTK_DIALOG (message)) == GTK_RESPONSE_YES)
			{
				authorg_util_launch_file_manager (folder);
			}
			gtk_widget_destroy (message);
		}
		
	}

	gtk_widget_destroy (dialog);
}

static gboolean
_delete_referenced_row (GtkTreeRowReference *ref, AuthorgVideoStore *store)
{
	GtkTreePath *path;
	
	path = gtk_tree_row_reference_get_path (ref);

	if (path)
	{
		GtkTreeIter iter;
		
		gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);
		authorg_video_store_remove (store, &iter);
	}

	return FALSE;
}

static void
authorg_main_window_cb_remove_video (GtkMenuItem *item, AuthorgMainWindow *window)
{
	GtkTreeSelection *selection;
	GList *selected_rows, *rowrefs = NULL;
	GtkTreeModel *model;
	AuthorgVideoStore *store;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (window->_priv->tree_view));
	
	selected_rows = gtk_tree_selection_get_selected_rows (selection, &model);

	if (selected_rows == NULL)
		return;

	store = AUTHORG_VIDEO_STORE (model);

	authorg_util_pathlist_to_rowref (model, selected_rows, &rowrefs);

	/* free paths */
	g_list_foreach (selected_rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (selected_rows);

	/* delete selected rows */
	g_list_foreach (rowrefs, (GFunc)_delete_referenced_row, store);

	if (authorg_video_store_is_empty(store))
		gtk_action_group_set_sensitive (window->nonempty_action_group, FALSE);
}

static gboolean
_add_files_to_store (const gchar *filename, AuthorgVideoStore *store)
{
	gchar *mime = NULL;
	AuthorgVideoFile *file = NULL;

#ifdef HAVE_GNOME_VFS
	mime = gnome_vfs_get_mime_type (filename);
#else
	if (g_str_has_suffix (filename, ".mpg") || g_str_has_suffix (filename, ".mpeg"))
		mime = g_strdup ("video/mpeg");
	else
		mime = g_strdup ("unknown");
#endif

	/* for now, only add mpeg files */
	if (g_ascii_strcasecmp ("video/mpeg", mime) == 0)
	{
		file = authorg_video_file_mpeg_new (filename);
		g_free (mime);
	}

	if (file != NULL)
		authorg_video_store_append (store, file);
	
	return TRUE;
}

static void
authorg_main_window_cb_add_file (GtkWidget *w, AuthorgMainWindow *main)
{
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new (
			_("Select files to be added to the DVD"),
			GTK_WINDOW (main),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_ADD, GTK_RESPONSE_OK,
			NULL);

	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog),
			TRUE);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	{
		GSList *files;
		AuthorgVideoStore *store;

		gtk_widget_hide (dialog);

		store = main->store;
		files = gtk_file_chooser_get_filenames (
				GTK_FILE_CHOOSER (dialog));

		g_assert (files != NULL);
		
		g_slist_foreach (files, (GFunc)_add_files_to_store, store);
		g_slist_foreach (files, (GFunc)g_free, NULL);
		g_slist_free (files);

		if (!authorg_video_store_is_empty(store))
			gtk_action_group_set_sensitive (main->nonempty_action_group, TRUE);
	}

	gtk_widget_destroy (dialog);
}

static void
authorg_main_window_cb_write_dvdauthor (GtkWidget *w, AuthorgMainWindow *main)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new (
			_("Select filename for dvdauthor XML file"),
			GTK_WINDOW (main),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_OK,
			NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	{
		gchar *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_widget_hide (dialog);
		
		/* check if file exists */
		if (authorg_util_overwrite (filename, GTK_WIDGET (main)))
		{
			authorg_video_store_to_dvdauthor (main->store, filename);
			authorg_util_message (GTK_WIDGET (main),
					_("Successfully created XML file %s"), filename);
		}
	}
	gtk_widget_destroy (dialog);
			
}

static void
authorg_main_window_cb_select_all (GtkWidget *w, AuthorgMainWindow *main)
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (
			GTK_TREE_VIEW (main->_priv->tree_view));

	gtk_tree_selection_select_all (selection);
	gtk_action_group_set_sensitive (main->selected_action_group, TRUE);
}

struct _chapter
{
	guint timecode;
	AuthorgVideoStore *store;
};

static void
_add_chapter (GtkTreePath *path, struct _chapter *helper)
{	
	GtkTreeIter iter;
	AuthorgVideoFile *file;
	
	gtk_tree_model_get_iter (GTK_TREE_MODEL (helper->store), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL (helper->store), &iter,
			AUTHORG_VIDEO_COLUMN_DATA, &file, -1);

	authorg_video_file_create_chapters (file, helper->timecode);
	
	gtk_tree_path_free (path);
}

static void
authorg_main_window_cb_edit_chapters (GtkWidget *w, AuthorgMainWindow *main)
{
	GtkWidget *editor;

	editor = authorg_chapter_editor_new ();

	if (gtk_dialog_run (GTK_DIALOG (editor)) == GTK_RESPONSE_OK)
	{
		GtkTreeSelection *selection;
		GList *selected_rows;
		struct _chapter help;
		GtkTreeModel *model;

		gtk_widget_hide (editor);
	
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (main->_priv->tree_view));
		selected_rows = gtk_tree_selection_get_selected_rows (selection, &model);

		help.timecode = AUTHORG_CHAPTER_EDITOR (editor)->timecode;
		help.store = AUTHORG_VIDEO_STORE (model);

		g_list_foreach (selected_rows, (GFunc)_add_chapter, &help);

		/* paths are already freed in previous foreach */
		g_list_free (selected_rows);
	}

	gtk_widget_destroy (editor);
}

static void
authorg_main_window_cb_about (GtkWidget *w, AuthorgMainWindow *win)
{
	authorg_about (GTK_WIDGET (win));
}

/* DND stuff */
enum
{
	TARGET_URL,
};

static GtkTargetEntry dnd_entries[] =
{
	{"text/uri-list", 0, TARGET_URL, },
};

/* ui manager stuff */
static GtkActionEntry entries[] =
{
	{"FileMenu", NULL, N_("_File"), },
	{"EditMenu", NULL, N_("_Edit"), },
	{"HelpMenu", NULL, N_("_Help"), },
	
	{"AddFile", GTK_STOCK_OPEN, N_("_Add file"), NULL, NULL, 
			G_CALLBACK (authorg_main_window_cb_add_file)},
	{"Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q", N_("Exit authorg"), 
			G_CALLBACK (authorg_main_window_quit)},

	/* edit menu */
	{"Preferences", GTK_STOCK_PREFERENCES, N_("_Preferences"), NULL, NULL },

	/* help menu */
	{"About", GTK_STOCK_ABOUT, N_("_About authorg"), NULL, NULL, 
		G_CALLBACK (authorg_main_window_cb_about), },
};

/* ActionEntries which depend on a non-empty list */
static GtkActionEntry nonempty_entries[] =
{
	{"SelectAll", NULL, N_("Select _all"), "<control>A", NULL,
			G_CALLBACK (authorg_main_window_cb_select_all) },
	{"ExportList", NULL, N_("_Export"), },
	{"WriteDVDAuthor", GTK_STOCK_SAVE, N_("_Write dvdauthor XML file"), NULL,
		NULL, G_CALLBACK (authorg_main_window_cb_write_dvdauthor) },
	{"RunDVDAuthor", NULL, N_("_Create DVD structure"), NULL, NULL, 
			G_CALLBACK (authorg_main_window_cb_run_dvdauthor) },
	{"CreateISO", GTK_STOCK_CDROM, N_("_Create ISO image"), NULL, NULL, 
			G_CALLBACK (authorg_main_window_cb_create_iso) },

};

/* ActionEntries which depend on a selected item */
static GtkActionEntry selected_entries[] =
{
	{"EditChapters", NULL, N_("_Edit chapters"), NULL, NULL, 
			G_CALLBACK (authorg_main_window_cb_edit_chapters), },
	{"DeleteVideo", GTK_STOCK_DELETE, N_("_Remove video from list"), NULL, NULL,
			G_CALLBACK (authorg_main_window_cb_remove_video), },
};

static const gchar *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='AddFile' />"
"      <menu action='ExportList'>"
"        <menuitem action='WriteDVDAuthor' />"
"        <menuitem action='RunDVDAuthor' />"
"        <menuitem action='CreateISO' />"
"      </menu>"
"      <separator />"
"      <menuitem action='Quit' />"
"    </menu>"
"    <menu action='EditMenu' >"
"      <menuitem action='DeleteVideo' />"
"      <separator />"
"      <menuitem action='EditChapters' />"
"      <separator />"
"      <menuitem action='SelectAll' />"
"      <separator />"
"      <menuitem action='Preferences' />"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='About' />"
"    </menu>"
"  </menubar>"
"  <toolbar action='Toolbar'>"
"    <toolitem action='CreateISO' />"
"      <separator />"
"    <toolitem action='Quit' />"
"  </toolbar>"
"  <popup action='VideoPopup'>"
"    <menuitem action='EditChapters' />"
"    <separator />"
"    <menuitem action='DeleteVideo' />"
"  </popup>"
"</ui>";

/*
 * This function creates a treeview with one column
 * which contains a thumbnail and information about the clip
 */
static GtkWidget *
main_window_setup_tree_view ()
{
	GtkWidget *tree_view;	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	AuthorgVideoStore *store;

	renderer = gtk_cell_renderer_pixbuf_new ();
	
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN (column),
			renderer, FALSE);
	gtk_tree_view_column_add_attribute (GTK_TREE_VIEW_COLUMN (column),
			renderer, "pixbuf", AUTHORG_VIDEO_COLUMN_THUMBNAIL);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN (column),
			renderer, TRUE);
	gtk_tree_view_column_add_attribute (GTK_TREE_VIEW_COLUMN (column),
			renderer, "markup", AUTHORG_VIDEO_COLUMN_MARKUP);

	store = authorg_video_store_new ();
	
	tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
			column);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view),
			FALSE);
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
				GTK_SELECTION_MULTIPLE);

	/* Treeview is searchable, search in column markup */
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tree_view), TRUE);
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view), AUTHORG_VIDEO_COLUMN_MARKUP);
	gtk_widget_show (tree_view);

	return tree_view;
}

static void
authorg_main_window_drag_data_received (GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time,
                        gpointer userdata)
{
	gchar **uris = NULL, 
		  *unescaped_uri = NULL,
		  *mime = NULL;
	gint i;
	AuthorgVideoStore *store = AUTHORG_MAIN_WINDOW (userdata)->store;

	uris = g_strsplit ((gchar *)seldata->data, "\r\n", -1);

	for (i = 0; uris[i] != NULL; i++)
	{
		if (!g_str_has_prefix (uris[i], "file://"))
			continue;

#ifdef HAVE_GNOME_VFS
		mime = gnome_vfs_get_mime_type (uris[i]);
		unescaped_uri = gnome_vfs_get_local_path_from_uri (uris[i]);
#else
		if (g_str_has_suffix (uris[i], ".mpg") || 
				g_str_has_suffix (uris[i], ".mpeg"))
			mime = g_strdup ("video/mpeg");
		else
			mime = g_strdup ("unknown");
		unescaped_uri = g_strdup ((uris[i])+7);
#endif

		if (g_ascii_strcasecmp ("video/mpeg", mime) == 0)
		{
			authorg_video_store_append (store,
					authorg_video_file_mpeg_new (unescaped_uri));
		}
		g_free (mime);
		g_free (unescaped_uri);
	}
	if (!authorg_video_store_is_empty (store))
		gtk_action_group_set_sensitive (AUTHORG_MAIN_WINDOW(userdata)->nonempty_action_group, TRUE);

	g_strfreev (uris);

	/* stop default handler because we use a custom model */
//	g_signal_stop_emission_by_name(G_OBJECT (wgt), "drag_data_received");
}

static gboolean
authorg_main_window_treeview_button_press_cb (GtkWidget *widget, GdkEventButton *event, AuthorgMainWindow *window)
{
	if (event->button == 3)
	{
		gtk_menu_popup (
				GTK_MENU (gtk_ui_manager_get_widget (window->ui_manager, "/VideoPopup")),
				NULL, NULL, NULL, NULL,
				event->button, event->time);
		return TRUE;
	}

	return FALSE;
}

static void
authorg_main_window_cb_selection_changed (GtkTreeSelection *selection, AuthorgMainWindow *window)
{
	gtk_action_group_set_sensitive (window->selected_action_group, 
			(gtk_tree_selection_count_selected_rows (selection) > 0));
}

GtkWidget *
authorg_main_window_new ()
{
    AuthorgMainWindow *main_window;
    GtkWidget *vbox, *hbox, *scrolled;
	GtkAccelGroup *ui_manager_accel_group, *accel_group;
	GClosure *closure;

    main_window = AUTHORG_MAIN_WINDOW (g_object_new (AUTHORG_TYPE_MAIN_WINDOW, NULL));
    vbox = gtk_vbox_new (0, FALSE);
	gtk_widget_show (vbox);

    gtk_container_add (GTK_CONTAINER (main_window), vbox);

	/* Setup toolbar and menu */

	/* ActionGroup for default actions */
	main_window->action_group = gtk_action_group_new ("DefaultActions");
	gtk_action_group_set_translation_domain (main_window->action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (main_window->action_group, entries,  G_N_ELEMENTS (entries), main_window);

	/* ActionGroup for actions that have to have a non-empty list */
	main_window->nonempty_action_group = gtk_action_group_new ("NonemptyActions");
	gtk_action_group_set_translation_domain (main_window->nonempty_action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (main_window->nonempty_action_group, nonempty_entries,  G_N_ELEMENTS (nonempty_entries), main_window);
	gtk_action_group_set_sensitive (main_window->nonempty_action_group, FALSE);

	/* ActionGroup for actions that have to have an list entry selected */
	main_window->selected_action_group = gtk_action_group_new ("SelectedActions");
	gtk_action_group_set_translation_domain (main_window->selected_action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (main_window->selected_action_group, selected_entries, G_N_ELEMENTS (selected_entries), main_window);
	gtk_action_group_set_sensitive (main_window->selected_action_group, FALSE);
	
	main_window->ui_manager = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (main_window->ui_manager, main_window->action_group, 0);
	gtk_ui_manager_insert_action_group (main_window->ui_manager, main_window->nonempty_action_group, 1);
	gtk_ui_manager_insert_action_group (main_window->ui_manager, main_window->selected_action_group, 2);

	ui_manager_accel_group = gtk_ui_manager_get_accel_group (main_window->ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (main_window), ui_manager_accel_group);

	/* own accel group */
	accel_group = gtk_accel_group_new ();
	closure = g_cclosure_new (G_CALLBACK (authorg_main_window_cb_remove_video), main_window, NULL);
	gtk_accel_group_connect (accel_group, GDK_Delete, 0, GTK_ACCEL_VISIBLE, closure); 

	gtk_window_add_accel_group (GTK_WINDOW (main_window), accel_group);
	
	gtk_ui_manager_add_ui_from_string (main_window->ui_manager, ui_description, -1, NULL);
	
    gtk_box_pack_start (GTK_BOX (vbox), 
			gtk_ui_manager_get_widget (main_window->ui_manager, "/MainMenu"), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox),
			gtk_ui_manager_get_widget (main_window->ui_manager, "/Toolbar"), FALSE, FALSE, 0);

    hbox = gtk_hbox_new (0, TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);
    
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
			GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled), GTK_SHADOW_IN);

    main_window->_priv->tree_view = main_window_setup_tree_view ();
	gtk_widget_show (main_window->_priv->tree_view);

	/* enable treeview as drop target */
	gtk_drag_dest_set(main_window->_priv->tree_view, GTK_DEST_DEFAULT_ALL, dnd_entries, 1,
			GDK_ACTION_COPY|GDK_ACTION_MOVE|GDK_ACTION_LINK);

	g_signal_connect(main_window->_priv->tree_view, "drag_data_received",
			G_CALLBACK(authorg_main_window_drag_data_received), main_window);

	main_window->store = AUTHORG_VIDEO_STORE (gtk_tree_view_get_model (
			GTK_TREE_VIEW (main_window->_priv->tree_view)));

	g_signal_connect (main_window->_priv->tree_view, "button-press-event",
			G_CALLBACK (authorg_main_window_treeview_button_press_cb), main_window);

	g_signal_connect (gtk_tree_view_get_selection (
				GTK_TREE_VIEW (main_window->_priv->tree_view)), "changed",
			G_CALLBACK (authorg_main_window_cb_selection_changed), main_window);

	gtk_container_add (GTK_CONTAINER (scrolled), main_window->_priv->tree_view);
	gtk_widget_show (scrolled);

    gtk_box_pack_start(GTK_BOX (hbox), scrolled, TRUE, TRUE, 0);

	/* get gconf client and add main directory */
	main_window->gconf_client = gconf_client_get_default ();
	gconf_client_add_dir (main_window->gconf_client,
					AUTHORG_GCONF_DIR,
					GCONF_CLIENT_PRELOAD_NONE,
					NULL);
	
    return GTK_WIDGET (main_window);
}

void
authorg_main_window_append_file (AuthorgMainWindow *window, const gchar *file)
{
	_add_files_to_store (file, window->store);
	gtk_action_group_set_sensitive (window->nonempty_action_group, 
			!authorg_video_store_is_empty(window->store));
}

static void
authorg_main_window_class_init (AuthorgMainWindowClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = authorg_main_window_finalize;
}
