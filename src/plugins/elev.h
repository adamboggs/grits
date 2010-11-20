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

#ifndef __ELEV_H__
#define __ELEV_H__

#include <glib-object.h>

#define GRITS_TYPE_PLUGIN_ELEV            (grits_plugin_elev_get_type ())
#define GRITS_PLUGIN_ELEV(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_PLUGIN_ELEV, GritsPluginElev))
#define GRITS_IS_PLUGIN_ELEV(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_PLUGIN_ELEV))
#define GRITS_PLUGIN_ELEV_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_PLUGIN_ELEV, GritsPluginElevClass))
#define GRITS_IS_PLUGIN_ELEV_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_PLUGIN_ELEV))
#define GRITS_PLUGIN_ELEV_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_PLUGIN_ELEV, GritsPluginElevClass))

typedef struct _GritsPluginElev      GritsPluginElev;
typedef struct _GritsPluginElevClass GritsPluginElevClass;

struct _GritsPluginElev {
	GObject parent_instance;

	/* instance members */
	GritsViewer *viewer;
	GritsTile   *tiles;
	GritsWms    *wms;
	GMutex      *mutex;
	gulong       sigid;
};

struct _GritsPluginElevClass {
	GObjectClass parent_class;
};

GType grits_plugin_elev_get_type();

/* Methods */
GritsPluginElev *grits_plugin_elev_new(GritsViewer *viewer);

#endif
