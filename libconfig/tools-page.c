/*  Authorg - a GNOME DVD authoring tool
 *
 *  Copyright (C) 2005 Jens Georg <mail@jensge.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as 
 *  published by the Free Software Foundation.
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

#include <glib.h>
#include <glib-object.h>
 
#include "tools-page.h"

struct AuthorgToolsPagePrivate
{
};

G_DEFINE_TYPE (AuthorgToolsPage, authorg_tools_page, AUTHORG_TYPE_SETTINGS_PAGE);

static void
authorg_tools_page_init (AuthorgToolsPage *self)
{
	self->_priv = g_new0(AuthorgToolsPagePrivate, 1);
}

static void
authorg_tools_page_finalize (GObject *object)
{
	AuthorgToolsPage *tools_page = AUTHORG_TOOLS_PAGE (object);

	g_return_if_fail (object != NULL);

	if (tools_page->_priv != NULL)
	{
		g_free (tools_page->_priv);
		tools_page->_priv = NULL;
	}
	
	if (G_OBJECT_CLASS (authorg_tools_page_parent_class)->finalize)
		G_OBJECT_CLASS (authorg_tools_page_parent_class)->finalize (object);
}

static void
authorg_tools_page_class_init (AuthorgToolsPageClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = authorg_tools_page_finalize;
}

GtkWidget *
authorg_tools_page_new (const gchar *gconf_key)
{
	AuthorgSettingsPage *tools_page;

	tools_page = AUTHORG_SETTINGS_PAGE (
			g_object_new (AUTHORG_TYPE_TOOLS_PAGE, NULL));

	authorg_settings_page_construct (tools_page, gconf_key);

	return GTK_WIDGET(tools_page);
}
