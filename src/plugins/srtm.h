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

#ifndef __SRTM_H__
#define __SRTM_H__

#include <glib-object.h>

#define GIS_TYPE_PLUGIN_SRTM            (gis_plugin_srtm_get_type ())
#define GIS_PLUGIN_SRTM(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_PLUGIN_SRTM, GisPluginSrtm))
#define GIS_IS_PLUGIN_SRTM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_PLUGIN_SRTM))
#define GIS_PLUGIN_SRTM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_PLUGIN_SRTM, GisPluginSrtmClass))
#define GIS_IS_PLUGIN_SRTM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_PLUGIN_SRTM))
#define GIS_PLUGIN_SRTM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_PLUGIN_SRTM, GisPluginSrtmClass))

typedef struct _GisPluginSrtm      GisPluginSrtm;
typedef struct _GisPluginSrtmClass GisPluginSrtmClass;

struct _GisPluginSrtm {
	GObject parent_instance;

	/* instance members */
	GisViewer *viewer;
	GisTile   *tiles;
	GisWms    *wms;
	GMutex    *mutex;
	gulong     sigid;
};

struct _GisPluginSrtmClass {
	GObjectClass parent_class;
};

GType gis_plugin_srtm_get_type();

/* Methods */
GisPluginSrtm *gis_plugin_srtm_new(GisViewer *viewer);

#endif
