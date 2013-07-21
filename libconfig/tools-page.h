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

#ifndef _TOOLS_PAGE_H
#define _TOOLS_PAGE_H

#include "settings-page.h"

#define AUTHORG_TYPE_TOOLS_PAGE (authorg_tools_page_get_type())
#define AUTHORG_TOOLS_PAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTHORG_TYPE_TOOLS_PAGE, AuthorgToolsPage))
#define AUTHORG_TOOLS_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), AUTHORG_TYPE_TOOLS_PAGE, AuthorgToolsPageClass))
#define AUTHORG_IS_TOOLS_PAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTHORG_TYPE_TOOLS_PAGE))
#define AUTHORG_IS_TOOLS_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), AUTHORG_TYPE_TOOLS_PAGE))

typedef struct AuthorgToolsPage AuthorgToolsPage;
typedef struct AuthorgToolsPageClass AuthorgToolsPageClass;
typedef struct AuthorgToolsPagePrivate AuthorgToolsPagePrivate;

struct AuthorgToolsPage {
		AuthorgSettingsPage parent;
		AuthorgToolsPagePrivate *_priv;
};

struct AuthorgToolsPageClass {
		AuthorgSettingsPageClass parent;
};

GType authorg_tools_page_get_type (void);
GtkWidget *authorg_tools_page_new (const gchar *gconf_key);
#endif /* _TOOLS_PAGE_H */
