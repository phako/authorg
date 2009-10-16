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

#include <signal.h>
#include <sys/wait.h>

#include <glib/gi18n.h>
#include <glib/gmain.h>
#include <glib/gquark.h>
#include <glib/gerror.h>

#include <gtk/gtkbox.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkprogressbar.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktextview.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkmain.h>

#include <pango/pango.h>

#include "spawn-dialog.h"

enum signals
{
	LINE_READ = 0,
	LAST_SIGNAL,
};

static guint authorg_spawn_dialog_signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE (AuthorgSpawnDialog, authorg_spawn_dialog, GTK_TYPE_DIALOG);

static void
authorg_spawn_dialog_cb_response (AuthorgSpawnDialog *dialog, gint id, gpointer user_data);

static void
authorg_spawn_dialog_init (AuthorgSpawnDialog *dialog)
{
	GtkWidget *vbox;
	GtkWidget *expander;
	GtkWidget *scrolled;
	GtkTextIter iter;
	PangoFontDescription *desc;

	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_widget_set_size_request (GTK_WIDGET (dialog), 220, -1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
	gtk_dialog_add_button (GTK_DIALOG (dialog),
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

	vbox = gtk_bin_get_child (GTK_BIN (dialog));
	gtk_box_set_spacing (GTK_BOX (vbox), 12);

	dialog->command = gtk_label_new ("");
	gtk_label_set_use_markup (GTK_LABEL (dialog->command), TRUE);
	gtk_widget_show (dialog->command);
	gtk_box_pack_start (GTK_BOX (vbox), dialog->command, FALSE, FALSE, 0);

	dialog->progress = gtk_progress_bar_new ();
	gtk_widget_show (dialog->progress);
	gtk_box_pack_start (GTK_BOX (vbox), dialog->progress, FALSE, FALSE, 0);

	expander = gtk_expander_new_with_mnemonic (_("_Details:"));
	gtk_box_pack_end (GTK_BOX (vbox), expander, TRUE, TRUE, 0);
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (expander), scrolled);
	dialog->details = gtk_text_buffer_new (NULL);
	gtk_text_buffer_get_end_iter (dialog->details, &iter);
	dialog->end = gtk_text_buffer_create_mark (dialog->details, 
			"end", 
			&iter,
			FALSE);
	dialog->details_view = gtk_text_view_new_with_buffer (dialog->details);
	desc = pango_font_description_from_string ("Mono");
	gtk_widget_modify_font (dialog->details_view, desc);
	pango_font_description_free (desc);

	gtk_container_add (GTK_CONTAINER (scrolled), dialog->details_view);
	gtk_widget_show (expander);
	gtk_widget_show (scrolled);
	gtk_widget_show (dialog->details_view);

	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	g_signal_connect (G_OBJECT (dialog), "response",
			G_CALLBACK (authorg_spawn_dialog_cb_response), NULL);

}

static void
authorg_spawn_dialog_finalize (GObject *object)
{
	AuthorgSpawnDialog *spawn_dialog = AUTHORG_SPAWN_DIALOG (object);

	g_return_if_fail (object != NULL);

	if (spawn_dialog->timeout != 0)
		g_source_remove (spawn_dialog->timeout);

	if (spawn_dialog->io_watch != 0)
		g_source_remove (spawn_dialog->io_watch);

	if (spawn_dialog->child_watch != 0)
		g_source_remove (spawn_dialog->child_watch);

	if (G_OBJECT_CLASS (authorg_spawn_dialog_parent_class)->finalize)
		G_OBJECT_CLASS (authorg_spawn_dialog_parent_class)->finalize (object);
}

static void
authorg_spawn_dialog_class_init (AuthorgSpawnDialogClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = authorg_spawn_dialog_finalize;

	/* setup signals */
	authorg_spawn_dialog_signals[LINE_READ] =
		g_signal_new ("line-read",
				G_TYPE_FROM_CLASS (klass),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (AuthorgSpawnDialogClass, line_read),
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING,
				G_TYPE_NONE,
				1, G_TYPE_STRING);
}

static void
authorg_spawn_dialog_cb_response (AuthorgSpawnDialog *dialog, gint id, gpointer user_data)
{
	switch (id)
	{
		case GTK_RESPONSE_NONE:
		case GTK_RESPONSE_CANCEL:
			/* fill error */
			dialog->error = NULL;
			g_set_error (&(dialog->error),
					AUTHORG_SPAWN_DIALOG_ERROR,
					AUTHORG_SPAWN_DIALOG_ERROR_CANCELED,
					_("User canceled program execution"));
			/* shutdown spawned program - 
			 * no need to care about cleanup; this happens in child
			 * watch, which takes care about the signals */
			kill (dialog->pid, SIGTERM);
			break;
		case GTK_RESPONSE_HELP:
			/* do nothing for now */
			break;
	}
}

GtkWidget *
authorg_spawn_dialog_new ()
{
	AuthorgSpawnDialog *dialog;

	dialog = AUTHORG_SPAWN_DIALOG (g_object_new (AUTHORG_TYPE_SPAWN_DIALOG, NULL));

	return GTK_WIDGET (dialog);
}

static void
authorg_spawn_dialog_child_watch (GPid pid, gint status, gpointer data)
{
	AuthorgSpawnDialog *dialog = AUTHORG_SPAWN_DIALOG (data);

	if (dialog->io_watch != 0)
	{
		g_source_remove (dialog->io_watch);
		dialog->io_watch = 0;
	}

	if (dialog->child_watch != 0)
	{
		g_source_remove (dialog->child_watch);
		dialog->child_watch = 0;
	}

	if (WIFEXITED (status) && WEXITSTATUS (status))
	{
		dialog->error = NULL;
		g_set_error (&(dialog->error),
				dialog->quark,
				AUTHORG_SPAWN_DIALOG_ERROR_TERMINATED_WITH_ERROR,
				_("Command failed with error %d"),
				WEXITSTATUS (status));
	}

	if (WIFSIGNALED (status))
	{
		if (dialog->error == NULL)
		{
			/* We didn't kill it ourself; in that case 
			 * the error would be already set
			 */
			g_set_error (&(dialog->error),
					dialog->quark,
					AUTHORG_SPAWN_DIALOG_ERROR_EXTERNAL_SIGNAL,
					_("Program got terminated by external signal %d\n"),
					WTERMSIG (status));
		}
	}

	if (!WIFSIGNALED (status) && ! WIFEXITED (status))
	{
		if (dialog->error == NULL)
		{
			g_set_error (&(dialog->error),
					dialog->quark,
					AUTHORG_SPAWN_DIALOG_ERROR_DIED_UNEXPECTED,
					_("Program died unexpected..."));
		}
	}

	g_spawn_close_pid (pid);

	/* unblock dialog */
	if (g_main_loop_is_running (dialog->loop))
		g_main_loop_quit (dialog->loop);
}

void
authorg_spawn_dialog_set_info (AuthorgSpawnDialog *dialog, const gchar *info)
{
	gchar *markup = NULL;

	markup = g_markup_printf_escaped ("<b>%s</b>", info);
	gtk_label_set_markup (GTK_LABEL (dialog->command), markup);
	g_free (markup);
}

static void
authorg_spawn_dialog_emit_line_read (GIOChannel *source, GIOCondition condition, gpointer user_data)
{
	AuthorgSpawnDialog *dialog = AUTHORG_SPAWN_DIALOG (user_data);
	gchar *line;
	
	g_io_channel_read_line (source, &line, NULL, NULL, NULL);

	gtk_text_buffer_insert_at_cursor (dialog->details, line, -1);
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (dialog->details_view), 
			dialog->end,
			0.25,
			TRUE,
			0.0,
			1.0);
			

	g_signal_emit (G_OBJECT (dialog),
			authorg_spawn_dialog_signals[LINE_READ],
			0,
			line);

	/* clear up pending events as signal handler could 
	 * have taken a long time to process */
	while (gtk_events_pending ())
		gtk_main_iteration ();

	g_free (line);
}

/**
 * authorg_spawn_dialog_run_command_line:
 * @dialog: the spawn dialog
 * @command_line: the commandline to run
 * @error: a GError
 *
 * runs a command line with no progress
 * Returns: TRUE on success, FALSE else.
 */
gboolean 
authorg_spawn_dialog_run_command_line (AuthorgSpawnDialog *dialog, 
		const gchar *command_line, 
		GError **error)
{
	gchar **argv;
	gint argc;
	GIOChannel *channel;
	gint output_fd;
	gboolean res = TRUE;

	gtk_widget_show (GTK_WIDGET (dialog));
	
	if (!g_shell_parse_argv (command_line,
			&argc, &argv, error))
	{
		return FALSE;
	}

	if (g_spawn_async_with_pipes (
				g_get_current_dir (),			/* working directory */
				argv, NULL,						/* args & environment */
				G_SPAWN_DO_NOT_REAP_CHILD,		/* flags */
				NULL, NULL,						/* ChildSetup */
				&dialog->pid,					/* Pid */
				NULL, NULL, &output_fd,			/* File descriptors */
				error))							/* Error */
	{
		dialog->child_watch = g_child_watch_add (dialog->pid,
				authorg_spawn_dialog_child_watch, dialog);
		
		if (dialog->child_watch != 0)
		{
			dialog->loop = g_main_loop_new (NULL, FALSE);
			channel = g_io_channel_unix_new (output_fd);
			dialog->io_watch = g_io_add_watch (
					channel,
					G_IO_IN | G_IO_PRI,
					(GIOFunc) authorg_spawn_dialog_emit_line_read,
					dialog);
		
			g_main_loop_run (dialog->loop);

			g_io_channel_unref (channel);

			/* user canceled */
			if (dialog->error != NULL)
			{
				g_propagate_error (error, dialog->error);
				res = FALSE;
			}
		}
	}
	else
	{
		res = FALSE;
	}

	gtk_widget_hide (GTK_WIDGET (dialog));

	return res;
}

GQuark
authorg_spawn_dialog_error_quark (void)
{
	static GQuark q;

	if (q == 0)
		q = g_quark_from_static_string ("authorg-spawn-dialog-error-quark");

	return q;
}
