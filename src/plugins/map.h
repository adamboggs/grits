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

#ifndef __MAP_H__
#define __MAP_H__

#include <glib-object.h>

#define GIS_TYPE_PLUGIN_MAP            (gis_plugin_map_get_type ())
#define GIS_PLUGIN_MAP(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_PLUGIN_MAP, GisPluginMap))
#define GIS_IS_PLUGIN_MAP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_PLUGIN_MAP))
#define GIS_PLUGIN_MAP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_PLUGIN_MAP, GisPluginMapClass))
#define GIS_IS_PLUGIN_MAP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_PLUGIN_MAP))
#define GIS_PLUGIN_MAP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_PLUGIN_MAP, GisPluginMapClass))

typedef struct _GisPluginMap      GisPluginMap;
typedef struct _GisPluginMapClass GisPluginMapClass;

struct _GisPluginMap {
	GObject parent_instance;

	/* instance members */
	GisViewer *viewer;
	GisTile   *tiles;
	GisWms    *wms;
	GMutex    *mutex;
	gulong     sigid;
};

struct _GisPluginMapClass {
	GObjectClass parent_class;
};

GType gis_plugin_map_get_type();

/* Methods */
GisPluginMap *gis_plugin_map_new(GisViewer *viewer);

#endif
