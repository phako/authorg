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

#include <unistd.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <gtk/gtkdialog.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkprogressbar.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktreemodel.h>

#include <video-store.h>

#include "spawn-dialog.h"
#include "util.h"

AuthorgDestFlags
authorg_util_dvd_dest_check (const gchar *folder, gint flags)
{
	GDir *dir;
	GtkWidget *dialog;

	if (!g_file_test (folder, G_FILE_TEST_EXISTS))
	{
		dialog = gtk_message_dialog_new (
				NULL,
				GTK_DIALOG_MODAL |
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				_("%s does not exist."),
				folder);

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
				"%s", 
				_("Please select another directory or create this directory first"));
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		return AUTHORG_DEST_NOT_OK;
	}
	
	/* should not be necessary, but better check it */
	if (!g_file_test (folder, G_FILE_TEST_IS_DIR))
	{
		dialog = gtk_message_dialog_new_with_markup (
				NULL,
				GTK_DIALOG_MODAL |
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				_("<b>%s is not a directory.</b>"),
				folder);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		return AUTHORG_DEST_NOT_OK;
	}

	dir = g_dir_open (folder, 0, NULL);

	/* is this directory empty? */
	if (g_dir_read_name (dir) != NULL)
	{
		gint res;

		g_dir_close (dir);
		dialog = gtk_message_dialog_new (
				NULL,
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING,
				GTK_BUTTONS_NONE,
				_("%s is not empty."),
				folder);
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
				"%s", 
				_("Do you want to delete its content?"));
		gtk_dialog_add_buttons (GTK_DIALOG (dialog),
				_("_Ignore"), GTK_RESPONSE_OK,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_DELETE, GTK_RESPONSE_YES,
				NULL);
		res = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		if (res == GTK_RESPONSE_OK)
			return AUTHORG_DEST_OK;

		if (res == GTK_RESPONSE_YES)
		{
			authorg_util_clear_tmp (folder, FALSE);

			return AUTHORG_DEST_OK;
		}

		return AUTHORG_DEST_NOT_OK;
	}

	return AUTHORG_DEST_OK;
}

static void
_dvdauthor_parse_progress (AuthorgSpawnDialog *dialog, gchar *line, guint *size)
{
	gchar *tmp;
	gchar **token;
	gdouble fraction = 0.0;
	static gdouble last_fraction = 0.0;
	/* dvdauthor has two steps. The VOBU stat step and the vobu fixing step. Only
	 * the second process returns a precentage */

	tmp = g_strstrip (line);

	if (g_str_has_suffix (tmp, "%)"))
	{
		if (g_str_has_prefix (tmp, "STAT: fixing VOBU at"))
		{
			token = g_strsplit (tmp, ", ", 2);
			if (token[1])
			{
				fraction = g_strtod (token[1], NULL) / 2 / 100 + 0.5;
			}
			g_strfreev (token);
		}
	}
	else
	{
		if (g_str_has_prefix (tmp, "STAT: VOBU"))
		{
			/* extract size */
			token = g_strsplit (tmp, "at", 2);
			if (token[1])
			{
				fraction = g_strtod (token[1], NULL) / 2 / *size;
			}
			g_strfreev (token);
		}
	}
	
	if (fraction > last_fraction && fraction <= 1.0)
	{
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (dialog->progress), fraction);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (dialog->progress), 
				tmp = g_strdup_printf ("%2.0f%%", fraction * 100));
		last_fraction = fraction;

		g_free (tmp);
	}
}

static void
_add_size (AuthorgVideoFile *file, guint *size)
{
	*size += (file->size / 1024 / 1024);
}

gboolean
authorg_util_run_dvdauthor (const gchar *folder, AuthorgVideoStore *store, GError **p_error)
{
	gint fd;
	GtkWidget *spawn_dialog;
	gchar *name = NULL, *tmp = NULL;
	GError *error = NULL;
	guint size = 0;

	if (authorg_util_dvd_dest_check (folder, 0) == AUTHORG_DEST_OK)
	{
		/* dump dvdauthor template to temporary file */
		fd = g_file_open_tmp ("authorgXXXXXX", &name, NULL);
		authorg_video_store_to_dvdauthor_fd (store, fd);
		close (fd);

		spawn_dialog = authorg_spawn_dialog_new ();
		authorg_spawn_dialog_set_info (AUTHORG_SPAWN_DIALOG (spawn_dialog),
				_("Please wait while creating DVD structure"));
		/* get size of all videos in store */
		authorg_video_store_foreach (store, (GFunc)_add_size, &size);
		
		g_signal_connect (G_OBJECT (spawn_dialog), "line-read",
				G_CALLBACK (_dvdauthor_parse_progress), 
				&size);
		tmp = g_strconcat (
					DVDAUTHOR, 
					" -o ",
					folder,
					" -x ",
					name, NULL);

		g_print ("%s\n", tmp);

		if (!authorg_spawn_dialog_run_command_line (
				AUTHORG_SPAWN_DIALOG (spawn_dialog), tmp, &error))
		{
			gtk_widget_hide (spawn_dialog);
			if (!g_error_matches (error, AUTHORG_SPAWN_DIALOG_ERROR, AUTHORG_SPAWN_DIALOG_ERROR_CANCELED))
			{
				authorg_util_spawn_error ("dvdauthor", error);
			}
			authorg_util_clear_tmp (folder, TRUE);
			g_error_free (error);
		
			gtk_widget_destroy (spawn_dialog);
			g_unlink (name);
	
			g_free (tmp);

			return FALSE;
		}
		
		gtk_widget_destroy (spawn_dialog);
		g_unlink (name);

		g_free (tmp);
	}

	return TRUE;
}

static void
_mkisofs_parse_progress (AuthorgSpawnDialog *dialog, gchar *line, gpointer user_data)
{
	gchar *tmp; 
	gdouble fraction = 0.0;
	static gdouble last_fraction = 0.0; 

	fraction = g_strtod (line, NULL) / 100;

	if (fraction > last_fraction && fraction <= 1.0)
	{
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (dialog->progress), fraction);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (dialog->progress), 
				tmp = g_strdup_printf ("%2.0f%%", fraction * 100));
		last_fraction = fraction;

		g_free (tmp);
	}
}

gboolean
authorg_util_run_mkisofs (const gchar *tmpfolder, const gchar *isoname, GError **p_error)
{
	GtkWidget *spawn_dialog;
	gchar *tmp, *mkisofs;
	GError *error = NULL;
				
	mkisofs = g_find_program_in_path ("mkisofs");

	if (mkisofs == NULL)
	{
		/* TODO: error message */
		return FALSE;
	}

	spawn_dialog = authorg_spawn_dialog_new ();
	authorg_spawn_dialog_set_info (AUTHORG_SPAWN_DIALOG (spawn_dialog),
			_("Creating the DVD ISO file"));
	tmp = g_strconcat (
				mkisofs = g_find_program_in_path ("mkisofs"),
				" -gui -dvd-video -o ",
				isoname,
				" ",
				tmpfolder,
				NULL);

	g_free (mkisofs);

	g_signal_connect (G_OBJECT (spawn_dialog), "line-read",
			G_CALLBACK (_mkisofs_parse_progress), NULL);

	if (!authorg_spawn_dialog_run_command_line (
			AUTHORG_SPAWN_DIALOG (spawn_dialog), tmp, &error))
	{
			gtk_widget_hide (spawn_dialog);
			if (!g_error_matches (error, AUTHORG_SPAWN_DIALOG_ERROR, AUTHORG_SPAWN_DIALOG_ERROR_CANCELED))
			{
				/* canceling is no "real" error, only show error if it was real */
				authorg_util_spawn_error ("mkisofs", error);
			}
			authorg_util_clear_tmp (isoname, TRUE);
			g_error_free (error);
			gtk_widget_destroy (spawn_dialog);
			g_free (tmp);

			return FALSE;
	}

	gtk_widget_destroy (spawn_dialog);
	g_free (tmp);

	return TRUE;
}

void
authorg_util_spawn_error (const gchar *command, GError *error)
{
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			_("Execution of %s failed."), command);

	gtk_message_dialog_format_secondary_text (
			GTK_MESSAGE_DIALOG (dialog),
			_("%s"), error->message);

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

void 
authorg_util_clear_tmp (const gchar *file, gboolean ask)
{
	gchar *argv[4];
	GError *error = NULL;
	gint res;
	
	if (ask)
	{
		GtkWidget *dialog;
		
		dialog = gtk_message_dialog_new (NULL,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			_("Remove %s?"), file);

		gtk_message_dialog_format_secondary_text (
				GTK_MESSAGE_DIALOG (dialog),
				_("There have already been created some files. It should be safe to delete them."));
		res = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}
	else
	{
		res = GTK_RESPONSE_YES;
	}

	if (res == GTK_RESPONSE_YES)
	{
		argv[0] = "rm";
		argv[1] = "-rf";
		argv[2] = g_strdup (file);
		argv[3] = NULL;

		if (!g_spawn_sync (g_get_tmp_dir(),
				argv,
				NULL,
				G_SPAWN_SEARCH_PATH,
				NULL, NULL,
				NULL, NULL, NULL,
				&error))
		{
			authorg_util_spawn_error ("rm", error);
			g_error_free (error);
		}
	}
}

void
authorg_util_pathlist_to_rowref (GtkTreeModel *model, GList *pathlist, GList **rowrefs)
{
	GList *tmp;
	GtkTreeRowReference *ref;
	
	g_return_if_fail (pathlist != NULL);
	g_return_if_fail (rowrefs != NULL && *rowrefs == NULL);

	for (tmp = pathlist; tmp != NULL; tmp = tmp->next)
	{
		ref = gtk_tree_row_reference_new (model, (GtkTreePath *)(tmp->data));
		*rowrefs = g_list_append (*rowrefs, ref);
	}
}

/**
 * authorg_util_overwrite:
 * 
 * Checks if given file already exists and displays
 * an Overwrite? Yes/No dialog if so
 */
gboolean
authorg_util_overwrite (const gchar *filename, GtkWidget *parent)
{
	GtkWidget *dialog;
	gboolean res;

	if (!g_file_test (filename, G_FILE_TEST_EXISTS))
		return TRUE;

	dialog = gtk_message_dialog_new (GTK_WINDOW (parent),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			_("Replace existing file %s?"), filename);

	gtk_message_dialog_format_secondary_text (
			GTK_MESSAGE_DIALOG (dialog),
			_("The file you have selected already exists.\nSelecting no will cancel the saving and leave your file untouched."));

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_NO);

	res = gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES;

	gtk_widget_destroy (dialog);

	return res;
}

/* launch konqueror or nautilus, copied from totem */
void
authorg_util_launch_file_manager (const gchar *filename)
{
	GError *err = NULL;
	char *cmd, *filemanager;

	if ((g_getenv ("KDE_STARTUP_ENV") || g_getenv("KDE_FULL_SESSION")) && (filemanager = g_find_program_in_path("konqueror"))) {
		cmd = g_strconcat (filemanager, " ", filename, NULL);
		g_free (filemanager);
	} else {
		cmd = g_strdup_printf ("nautilus --no-default-window -no-desktop %s", filename);
	}

	if (g_spawn_command_line_async (cmd, &err) == FALSE)
	{
		authorg_util_spawn_error ("filemanager", err);
		g_error_free (err);
	}

	g_free (cmd);
}

void
authorg_util_message (GtkWidget *parent, const gchar *message, const gchar *secondary)
{
	GtkWidget *dialog;
	gchar *tmp;

	if (secondary == NULL)
	{
		dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (parent),
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO,
				GTK_BUTTONS_OK,
                "%s",
				tmp = g_strdup_printf ("<b>%s</b>", message));
		g_free (tmp);
	}
	else
	{
		dialog = gtk_message_dialog_new (GTK_WINDOW (parent),
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO,
				GTK_BUTTONS_OK,
				"%s", message);

		gtk_message_dialog_format_secondary_text (
				GTK_MESSAGE_DIALOG (dialog),
				"%s", secondary);
	}

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);
}

