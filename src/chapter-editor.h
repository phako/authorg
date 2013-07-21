
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

#ifndef _CHAPTER_EDITOR_H
#define _CHAPTER_EDITOR_H

#include <gtk/gtkdialog.h>

#define AUTHORG_TYPE_CHAPTER_EDITOR (authorg_chapter_editor_get_type())
#define AUTHORG_CHAPTER_EDITOR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTHORG_TYPE_CHAPTER_EDITOR, AuthorgChapterEditor))
#define AUTHORG_CHAPTER_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), AUTHORG_TYPE_CHAPTER_EDITOR, AuthorgChapterEditorClass))
#define AUTHORG_IS_CHAPTER_EDITOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTHORG_TYPE_CHAPTER_EDITOR))
#define AUTHORG_IS_CHAPTER_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), AUTHORG_TYPE_CHAPTER_EDITOR))

typedef struct AuthorgChapterEditor AuthorgChapterEditor;
typedef struct AuthorgChapterEditorClass AuthorgChapterEditorClass;

struct AuthorgChapterEditor {
		GtkDialog parent;
		guint timecode;
};

struct AuthorgChapterEditorClass {
		GtkDialogClass parent;
};

GType authorg_chapter_editor_get_type (void);
GtkWidget *authorg_chapter_editor_new (void);
#endif /* _CHAPTER_EDITOR_H */
