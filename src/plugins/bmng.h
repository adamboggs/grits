/*
 * Copyright (C) 2009 Andy Spencer <spenceal@rose-hulman.edu>
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

#ifndef __BMNG_H__
#define __BMNG_H__

#include <glib-object.h>

#define GIS_TYPE_PLUGIN_BMNG            (gis_plugin_bmng_get_type ())
#define GIS_PLUGIN_BMNG(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_PLUGIN_BMNG, GisPluginBmng))
#define GIS_IS_PLUGIN_BMNG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_PLUGIN_BMNG))
#define GIS_PLUGIN_BMNG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_PLUGIN_BMNG, GisPluginBmngClass))
#define GIS_IS_PLUGIN_BMNG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_PLUGIN_BMNG))
#define GIS_PLUGIN_BMNG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_PLUGIN_BMNG, GisPluginBmngClass))

typedef struct _GisPluginBmng      GisPluginBmng;
typedef struct _GisPluginBmngClass GisPluginBmngClass;

struct _GisPluginBmng {
	GObject parent_instance;

	/* instance members */
	GisViewer *viewer;
	GisOpenGL *opengl;
	GisTile   *tiles;
	GisWms    *wms;
	GMutex    *mutex;
	gulong     sigid;
};

struct _GisPluginBmngClass {
	GObjectClass parent_class;
};

GType gis_plugin_bmng_get_type();

/* Methods */
GisPluginBmng *gis_plugin_bmng_new(GisViewer *viewer, GisOpenGL *opengl);

#endif
