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
#include <gtk/gtk.h>

#include "main-window.h"

gboolean
authorg_main_quit()
{
	gtk_main_quit();
	return TRUE;
}

int
main(int argc, char **argv)
{
	GOptionContext *ctx;
	GtkWidget *main_window;
	int i;
	GError *error = NULL;

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init ();
#endif

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	ctx = g_option_context_new ("FILES");
	g_option_context_add_group (ctx, gtk_get_option_group (TRUE));
	if (g_option_context_parse (ctx, &argc, &argv, &error) == FALSE)
	{
		g_warning ("Error parsing options: %s", error->message);
	}

	main_window = authorg_main_window_new ();

	for (i = 1; i < argc; i++)
	{
		authorg_main_window_append_file (AUTHORG_MAIN_WINDOW (main_window), argv[i]);
	}
	gtk_window_set_default_size (GTK_WINDOW (main_window), 400, 500);
	g_signal_connect (G_OBJECT (main_window), "delete-event",
			G_CALLBACK (authorg_main_quit), NULL);
	gtk_widget_show (main_window);
	
	gtk_main ();

	return 0;
}
