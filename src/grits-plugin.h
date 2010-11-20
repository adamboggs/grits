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

#ifndef __GIS_PLUGIN_H__
#define __GIS_PLUGIN_H__

#include <glib-object.h>
#include <gtk/gtk.h>

/********************
 * Plugin interface *
 ********************/
#define GIS_TYPE_PLUGIN                (gis_plugin_get_type())
#define GIS_PLUGIN(obj)                (G_TYPE_CHECK_INSTANCE_CAST   ((obj),  GIS_TYPE_PLUGIN, GisPlugin))
#define GIS_IS_PLUGIN(obj)             (G_TYPE_CHECK_INSTANCE_TYPE   ((obj),  GIS_TYPE_PLUGIN))
#define GIS_PLUGIN_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), GIS_TYPE_PLUGIN, GisPluginInterface))

typedef struct _GisPlugin          GisPlugin;
typedef struct _GisPluginInterface GisPluginInterface;

struct _GisPluginInterface
{
	GTypeInterface parent_iface;

	/* Virtual data */
	const gchar *name;
	const gchar *description;

	/* Virtual functions */
	GtkWidget *(*get_config)(GisPlugin *plugin);
};

GType gis_plugin_get_type();

/* Methods */
const gchar *gis_plugin_get_name(GisPlugin *plugin);

const gchar *gis_plugin_get_description(GisPlugin *plugin);

GtkWidget *gis_plugin_get_config(GisPlugin *plugin);

/***************
 * Plugins API *
 ***************/
typedef struct _GisPlugins GisPlugins;

#include "grits-viewer.h"
#include "grits-prefs.h"

/**
 * GisPluginConstructor:
 * @viewer: the viewer the plugin is associated with
 * @prefs:  preferences the plugin can use for storing informtion
 *
 * Create a new instance of a plugin. Each plugin should supply a constructor named
 * gis_plugin_NAME_new in it's shared object.
 *
 * Returns: the new plugin
 */
typedef GisPlugin *(*GisPluginConstructor)(GisViewer *viewer, GisPrefs *prefs);

struct _GisPlugins {
	gchar    *dir;
	GList    *plugins;
	GisPrefs *prefs;
};

GisPlugins *gis_plugins_new(const gchar *dir, GisPrefs *prefs);

void gis_plugins_free();

GList *gis_plugins_available(GisPlugins *plugins);

GisPlugin *gis_plugins_load(GisPlugins *plugins, const char *name,
		GisViewer *viewer, GisPrefs *prefs);

GisPlugin *gis_plugins_enable(GisPlugins *plugins, const char *name,
		GisViewer *viewer, GisPrefs *prefs);

GList *gis_plugins_load_enabled(GisPlugins *plugins,
		GisViewer *viewer, GisPrefs *prefs);

gboolean gis_plugins_disable(GisPlugins *plugins, const char *name);

gboolean gis_plugins_unload(GisPlugins *plugins, const char *name);

void gis_plugins_foreach(GisPlugins *plugins, GCallback callback, gpointer user_data);

#endif
