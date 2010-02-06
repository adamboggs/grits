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

#ifndef __ENV_H__
#define __ENV_H__

#include <glib-object.h>

#define GIS_TYPE_PLUGIN_ENV            (gis_plugin_env_get_type ())
#define GIS_PLUGIN_ENV(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_PLUGIN_ENV, GisPluginEnv))
#define GIS_IS_PLUGIN_ENV(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_PLUGIN_ENV))
#define GIS_PLUGIN_ENV_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_PLUGIN_ENV, GisPluginEnvClass))
#define GIS_IS_PLUGIN_ENV_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_PLUGIN_ENV))
#define GIS_PLUGIN_ENV_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_PLUGIN_ENV, GisPluginEnvClass))

typedef struct _GisPluginEnv      GisPluginEnv;
typedef struct _GisPluginEnvClass GisPluginEnvClass;

struct _GisPluginEnv {
	GObject parent_instance;

	/* instance members */
	GisViewer *viewer;
	guint      tex;
	GisTile   *background;
	GList     *refs;
};

struct _GisPluginEnvClass {
	GObjectClass parent_class;
};

GType gis_plugin_env_get_type();

/* Methods */
GisPluginEnv *gis_plugin_env_new(GisViewer *viewer, GisPrefs *prefs);

#endif
