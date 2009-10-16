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

#ifndef _AUTHORG_UTIL_H
#define _AUTHORG_UTIL_H

#include <glib.h>
#include <glib/gi18n.h>


typedef enum 
{
	AUTHORG_DEST_OK = 0,
	AUTHORG_DEST_NOT_OK,
} AuthorgDestFlags;

AuthorgDestFlags authorg_util_dvd_dest_check (const gchar *folder, gint flags);
gboolean authorg_util_run_dvdauthor (const gchar *folder, AuthorgVideoStore *store, GError **error);
gboolean authorg_util_run_mkisofs (const gchar *tmpfolder, const gchar *isoname, GError **error);
void authorg_util_spawn_error (const gchar *command, GError *error);
void authorg_util_clear_tmp (const gchar *file, gboolean ask);
void authorg_util_pathlist_to_rowref (GtkTreeModel *model, GList *pathlist, GList **rowrefs);
gboolean authorg_util_overwrite (const gchar *filename, GtkWidget *parent);
void authorg_util_launch_file_manager (const gchar *filename);
void authorg_util_message (GtkWidget *parent, const gchar *message, const gchar *secondary);
#endif
