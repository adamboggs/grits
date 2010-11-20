/*
 * Copyright (C) 2009-2010 Andy Spencer <andy753421@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GRITS_PLUGIN_H__
#define __GRITS_PLUGIN_H__

#include <glib-object.h>
#include <gtk/gtk.h>

/********************
 * Plugin interface *
 ********************/
#define GRITS_TYPE_PLUGIN                (grits_plugin_get_type())
#define GRITS_PLUGIN(obj)                (G_TYPE_CHECK_INSTANCE_CAST   ((obj),  GRITS_TYPE_PLUGIN, GritsPlugin))
#define GRITS_IS_PLUGIN(obj)             (G_TYPE_CHECK_INSTANCE_TYPE   ((obj),  GRITS_TYPE_PLUGIN))
#define GRITS_PLUGIN_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), GRITS_TYPE_PLUGIN, GritsPluginInterface))

typedef struct _GritsPlugin          GritsPlugin;
typedef struct _GritsPluginInterface GritsPluginInterface;

struct _GritsPluginInterface
{
	GTypeInterface parent_iface;

	/* Virtual data */
	const gchar *name;
	const gchar *description;

	/* Virtual functions */
	GtkWidget *(*get_config)(GritsPlugin *plugin);
};

GType grits_plugin_get_type();

/* Methods */
const gchar *grits_plugin_get_name(GritsPlugin *plugin);

const gchar *grits_plugin_get_description(GritsPlugin *plugin);

GtkWidget *grits_plugin_get_config(GritsPlugin *plugin);

/***************
 * Plugins API *
 ***************/
typedef struct _GritsPlugins GritsPlugins;

#include "grits-viewer.h"
#include "grits-prefs.h"

/**
 * GritsPluginConstructor:
 * @viewer: the viewer the plugin is associated with
 * @prefs:  preferences the plugin can use for storing informtion
 *
 * Create a new instance of a plugin. Each plugin should supply a constructor named
 * grits_plugin_NAME_new in it's shared object.
 *
 * Returns: the new plugin
 */
typedef GritsPlugin *(*GritsPluginConstructor)(GritsViewer *viewer, GritsPrefs *prefs);

struct _GritsPlugins {
	gchar      *dir;
	GList      *plugins;
	GritsPrefs *prefs;
};

GritsPlugins *grits_plugins_new(const gchar *dir, GritsPrefs *prefs);

void grits_plugins_free();

GList *grits_plugins_available(GritsPlugins *plugins);

GritsPlugin *grits_plugins_load(GritsPlugins *plugins, const char *name,
		GritsViewer *viewer, GritsPrefs *prefs);

GritsPlugin *grits_plugins_enable(GritsPlugins *plugins, const char *name,
		GritsViewer *viewer, GritsPrefs *prefs);

GList *grits_plugins_load_enabled(GritsPlugins *plugins,
		GritsViewer *viewer, GritsPrefs *prefs);

gboolean grits_plugins_disable(GritsPlugins *plugins, const char *name);

gboolean grits_plugins_unload(GritsPlugins *plugins, const char *name);

void grits_plugins_foreach(GritsPlugins *plugins, GCallback callback, gpointer user_data);

#endif
