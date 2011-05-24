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

#ifndef __SAT_H__
#define __SAT_H__

#include <glib-object.h>

#define GRITS_TYPE_PLUGIN_SAT            (grits_plugin_sat_get_type ())
#define GRITS_PLUGIN_SAT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_PLUGIN_SAT, GritsPluginSat))
#define GRITS_IS_PLUGIN_SAT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_PLUGIN_SAT))
#define GRITS_PLUGIN_SAT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_PLUGIN_SAT, GritsPluginSatClass))
#define GRITS_IS_PLUGIN_SAT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_PLUGIN_SAT))
#define GRITS_PLUGIN_SAT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_PLUGIN_SAT, GritsPluginSatClass))

typedef struct _GritsPluginSat      GritsPluginSat;
typedef struct _GritsPluginSatClass GritsPluginSatClass;

struct _GritsPluginSat {
	GObject parent_instance;

	/* instance members */
	GritsViewer *viewer;
	GritsTile   *tiles;
	GritsWms    *wms;
	GThreadPool *threads;
	gulong       sigid;
	gboolean     aborted;
};

struct _GritsPluginSatClass {
	GObjectClass parent_class;
};

GType grits_plugin_sat_get_type();

/* Methods */
GritsPluginSat *grits_plugin_sat_new(GritsViewer *viewer);

#endif
