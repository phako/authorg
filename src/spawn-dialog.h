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

#ifndef _SPAWN_DIALOG_H
#define _SPAWN_DIALOG_H

#define AUTHORG_TYPE_SPAWN_DIALOG (authorg_spawn_dialog_get_type())
#define AUTHORG_SPAWN_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTHORG_TYPE_SPAWN_DIALOG, AuthorgSpawnDialog))
#define AUTHORG_SPAWN_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), AUTHORG_TYPE_SPAWN_DIALOG, AuthorgSpawnDialogClass))
#define AUTHORG_IS_SPAWN_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTHORG_TYPE_SPAWN_DIALOG))
#define AUTHORG_IS_SPAWN_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), AUTHORG_TYPE_SPAWN_DIALOG))

typedef struct AuthorgSpawnDialog AuthorgSpawnDialog;
typedef struct AuthorgSpawnDialogClass AuthorgSpawnDialogClass;

typedef enum AuthorgSpawnErrors AuthorgSpawnErrors;

#include <gtk/gtktextbuffer.h>

struct AuthorgSpawnDialog {
	GtkDialog parent;

	GtkWidget *progress;
	GtkWidget *command;
	GtkWidget *details_view;
	GtkTextBuffer *details;

	/* <private> */
	GPid pid;
	guint timeout;
	guint io_watch;
	guint child_watch;
	GMainLoop *loop;
	GQuark quark;
	GError *error;
	GtkTextMark *end;
};

struct AuthorgSpawnDialogClass {
	GtkDialogClass parent;

	/* <signals> */
	void (*line_read)	(AuthorgSpawnDialog *dialog, gchar *line, gpointer user_data);
};

#define AUTHORG_SPAWN_DIALOG_ERROR authorg_spawn_dialog_error_quark()

enum AuthorgSpawnErrors {
	AUTHORG_SPAWN_DIALOG_ERROR_TERMINATED_WITH_ERROR = 0,
	AUTHORG_SPAWN_DIALOG_ERROR_CANCELED,
	AUTHORG_SPAWN_DIALOG_ERROR_EXTERNAL_SIGNAL,
	AUTHORG_SPAWN_DIALOG_ERROR_DIED_UNEXPECTED,
};

GType authorg_spawn_dialog_get_type (void);
GtkWidget *authorg_spawn_dialog_new (void);
void authorg_spawn_dialog_set_info (AuthorgSpawnDialog *dialog, const gchar *info);
gboolean authorg_spawn_dialog_run_command_line (AuthorgSpawnDialog *dialog, const gchar *command_line, GError **error);
GQuark authorg_spawn_dialog_error_quark (void);

#endif /* _SPAWN_DIALOG_H */
