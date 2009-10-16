#include <config.h>

#include <glib/gi18n.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <gtk/gtkaboutdialog.h>
#include <gtk/gtkwidget.h>

#include "about.h"

const gchar *authors[] = {
	"Jens Georg",
	NULL,
};

void
authorg_about(GtkWidget *parent)
{
	GdkPixbuf *logo;
	gchar *path;
	
	path = g_build_filename (DATADIR, "authorg", "authorg.png", NULL);
	logo = gdk_pixbuf_new_from_file (path, NULL);
	g_free (path);

	gtk_show_about_dialog (GTK_WINDOW (parent),
			"name", "Authorg",
			"version", VERSION,
			"copyright", "(C) 2005 Jens Georg",
			"comments", _("Simple DVD authoring program"),
			"authors", authors,
			"logo", logo,
			NULL);

	g_object_unref (logo);
}
