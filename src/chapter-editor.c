/* 	Authorg - a GNOME DVD authoring tool
 *
 * 	Copyright (C) 2005 Jens Georg <mail@jensge.org>
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
#include <glib-object.h>
#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "chapter-editor.h"

G_DEFINE_TYPE (AuthorgChapterEditor, authorg_chapter_editor, GTK_TYPE_DIALOG);

static void
authorg_chapter_editor_init (AuthorgChapterEditor *chapter_editor)
{
}

static void
authorg_chapter_editor_finalize (GObject *object)
{
	/*AuthorgChapterEditor *chapter_editor = AUTHORG_CHAPTER_EDITOR (object); */

	g_return_if_fail (object != NULL);

	if (G_OBJECT_CLASS (authorg_chapter_editor_parent_class)->finalize)
		G_OBJECT_CLASS (authorg_chapter_editor_parent_class)->finalize (object);
}

static void
authorg_chapter_editor_class_init (AuthorgChapterEditorClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = authorg_chapter_editor_finalize;
}

static void
authorg_video_chapter_cb_spin_button (GtkWidget *w, AuthorgChapterEditor *editor)
{
	editor->timecode = (int)(gtk_spin_button_get_value (GTK_SPIN_BUTTON (w)) * 60.0);
}

/* public functions */
GtkWidget *
authorg_chapter_editor_new ()
{
	AuthorgChapterEditor *editor;
	GtkWidget *vbox, *hbox, *label, *spinbutton;

	editor = AUTHORG_CHAPTER_EDITOR (g_object_new (AUTHORG_TYPE_CHAPTER_EDITOR, NULL));

	gtk_dialog_add_button (GTK_DIALOG (editor),
					GTK_STOCK_OK, GTK_RESPONSE_OK);

	gtk_container_set_border_width (GTK_CONTAINER (editor), 6);

	vbox = gtk_bin_get_child (GTK_BIN (editor));
	gtk_box_set_spacing (GTK_BOX (vbox), 2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);

	label = gtk_label_new (_("Add chapter every"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0); 
	gtk_widget_show (label);

	spinbutton = gtk_spin_button_new_with_range (5.0, 90.0, 1.0);
	gtk_widget_show (spinbutton);
	gtk_box_pack_start (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);

	/* set timecode to 5 minutes */
	editor->timecode = 300;
	g_signal_connect (G_OBJECT (spinbutton), "value-changed",
			G_CALLBACK (authorg_video_chapter_cb_spin_button), editor);
	
	label = gtk_label_new (_("minutes"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0); 
	gtk_widget_show (label);
	
	return GTK_WIDGET (editor);
}
