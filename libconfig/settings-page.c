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

#include <config.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>

#include <gtk/gtkentry.h>

#include <gconf/gconf-client.h>

#include "settings-page.h"

struct AuthorgSettingsPagePrivate
{
  gchar *gconf_key;
  GConfClient *gconf_client;
};

G_DEFINE_TYPE (AuthorgSettingsPage, authorg_settings_page, GTK_TYPE_VBOX);

static void
authorg_settings_page_init (AuthorgSettingsPage *self)
{
  self->_priv = g_new0(AuthorgSettingsPagePrivate, 1);
}

static void
authorg_settings_page_finalize (GObject *object)
{
  AuthorgSettingsPage *settings_page = AUTHORG_SETTINGS_PAGE (object);

  g_return_if_fail (object != NULL);

  if (settings_page->_priv != NULL)
  {
    if (settings_page->_priv->gconf_key != NULL)
    {
      g_free (settings_page->_priv->gconf_key);
    }

    if (settings_page->_priv->gconf_client != NULL)
    {
      g_object_unref (settings_page->_priv->gconf_client);
    }

    g_free (settings_page->_priv);
    settings_page->_priv = NULL;
  }

  if (G_OBJECT_CLASS (authorg_settings_page_parent_class)->finalize)
    G_OBJECT_CLASS (authorg_settings_page_parent_class)->finalize (object);
}

static void
authorg_settings_page_class_init (AuthorgSettingsPageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = authorg_settings_page_finalize;
}

void
authorg_settings_page_construct (AuthorgSettingsPage *page, const gchar *gconf_key)
{
  g_return_if_fail (AUTHORG_IS_SETTINGS_PAGE (page));

  page->_priv->gconf_key = g_strdup (gconf_key);
  page->_priv->gconf_client = gconf_client_get_default ();
}

/*
 * Behind the scenes function to provide more simple convenience
 * functions for specific widgets
 */
static guint
authorg_settings_page_bind_widget_to_gconf_full (
    AuthorgSettingsPage *page,
    GtkWidget *widget,
    const gchar *key,
    GConfClientNotifyFunc notify_func,
    GCallback set_func,
    const gchar *signal
    )
{
  guint cxn_id = 0;
  gchar *tmp = NULL;
  GError *error = NULL;

  if (NULL != notify_func)
  {
    tmp = g_strconcat (page->_priv->gconf_key, "/", key, NULL);
    cxn_id = gconf_client_notify_add (page->_priv->gconf_client,
        tmp, notify_func, widget, NULL, &error);
    g_free (tmp);
  }

  if (NULL != set_func)
  {
    g_object_set_data (G_OBJECT (widget),
        "authorg_gconf_key", g_strdup (key));

    if (signal != NULL)
    {
      g_signal_connect (G_OBJECT (widget), signal,
          set_func, page);
    }

    g_signal_connect (G_OBJECT (widget),
        "focus-out-event", set_func, page);
  }

  return cxn_id;
}

static void
__set_entry_from_gconf (GConfClient *client,
    guint cxn_id,
    GConfEntry *entry,
    gpointer user_data)
{
  GtkWidget *tentry = user_data;


  if (gconf_entry_get_value (entry) == NULL)
  {
    gtk_entry_set_text (GTK_ENTRY (tentry), "");
  }
  else if (gconf_entry_get_value (entry)->type == GCONF_VALUE_STRING)
  {
    const gchar *tmp = gconf_value_get_string (gconf_entry_get_value (entry));

    gtk_entry_set_text (GTK_ENTRY (tentry), tmp);
  }
}

static void
__set_gconf_from_entry (GtkWidget *entry, gpointer user_data)
{
  GError *error = NULL;
  gchar *key, *path;
  AuthorgSettingsPage *page = AUTHORG_SETTINGS_PAGE (user_data);

  key = g_object_get_data (G_OBJECT (entry), "key");

  path = g_strconcat (page->_priv->gconf_key, "/", key, NULL);
  gconf_client_set_string (
      page->_priv->gconf_client,
      path,
      gtk_entry_get_text (GTK_ENTRY (entry)),
      &error);
  g_free (path);
}

guint 
authorg_settings_page_bind_entry_to_gconf (
    AuthorgSettingsPage *page,
    GtkWidget *entry,
    const gchar *key)
{
  return authorg_settings_page_bind_widget_to_gconf_full (
      page,
      entry,
      key,
      (GConfClientNotifyFunc)(__set_entry_from_gconf),
      G_CALLBACK (__set_gconf_from_entry),
      "activate");
}

/* vim:ts=2 sw=2 et tw=76
 */
