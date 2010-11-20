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

#ifndef __TEAPOT_H__
#define __TEAPOT_H__

#include <glib-object.h>

#define GRITS_TYPE_PLUGIN_TEAPOT            (grits_plugin_teapot_get_type ())
#define GRITS_PLUGIN_TEAPOT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_PLUGIN_TEAPOT, GritsPluginTeapot))
#define GRITS_IS_PLUGIN_TEAPOT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_PLUGIN_TEAPOT))
#define GRITS_PLUGIN_TEAPOT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_PLUGIN_TEAPOT, GritsPluginTeapotClass))
#define GRITS_IS_PLUGIN_TEAPOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_PLUGIN_TEAPOT))
#define GRITS_PLUGIN_TEAPOT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_PLUGIN_TEAPOT, GritsPluginTeapotClass))

typedef struct _GritsPluginTeapot      GritsPluginTeapot;
typedef struct _GritsPluginTeapotClass GritsPluginTeapotClass;

struct _GritsPluginTeapot {
	GObject parent_instance;

	/* instance members */
	GritsViewer     *viewer;
	GtkToggleButton *button;
	guint            rotate_id;
	float            rotation;
};

struct _GritsPluginTeapotClass {
	GObjectClass parent_class;
};

GType grits_plugin_teapot_get_type();

/* Methods */
GritsPluginTeapot *grits_plugin_teapot_new(GritsViewer *viewer, GritsPrefs *prefs);

#endif
