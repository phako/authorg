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

#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <glib.h>
#include <gtk/gtk.h>

#include <gconf/gconf-client.h>

#include <video-store.h>

G_BEGIN_DECLS
#define AUTHORG_TYPE_MAIN_WINDOW (authorg_main_window_get_type())
#define AUTHORG_MAIN_WINDOW(obj) (GTK_CHECK_CAST ((obj), AUTHORG_TYPE_MAIN_WINDOW, AuthorgMainWindow))
#define AUTHORG_MAIN_WINDOW_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), AUTHORG_TYPE_MAIN_WINDOW, AuthorgMainWindowClass))
#define AUTHORG_IS_MAIN_WINDOW(obj) (GTK_CHECK_TYPE ((obj), AUTHORG_TYPE_MAIN_WINDOW))
#define AUTHORG_IS_MAIN_WINDOW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), AUTHORG_TYPE_MAIN_WINDOW))

typedef struct AuthorgMainWindow AuthorgMainWindow;
typedef struct AuthorgMainWindowClass AuthorgMainWindowClass;
typedef struct AuthorgMainWindowPrivate AuthorgMainWindowPrivate;

struct AuthorgMainWindow {
		GtkWindow parent;

		/*<private>*/
		GtkUIManager *ui_manager;
		GtkActionGroup *action_group;
		GtkActionGroup *nonempty_action_group;
		GtkActionGroup *selected_action_group;
		AuthorgVideoStore *store;
		AuthorgMainWindowPrivate *_priv;
		GConfClient *gconf_client;
};

struct AuthorgMainWindowClass {
		GtkWindowClass parent;
};

GtkType authorg_main_window_get_type (void);
GtkWidget *authorg_main_window_new (void);
void authorg_main_window_append_file (AuthorgMainWindow *window, const gchar *file);

G_END_DECLS
#endif /* _MAIN_WINDOW_H */
