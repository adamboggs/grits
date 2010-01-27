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

#ifndef __TEST_H__
#define __TEST_H__

#include <glib-object.h>

#define GIS_TYPE_PLUGIN_TEST            (gis_plugin_test_get_type ())
#define GIS_PLUGIN_TEST(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_PLUGIN_TEST, GisPluginTest))
#define GIS_IS_PLUGIN_TEST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_PLUGIN_TEST))
#define GIS_PLUGIN_TEST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_PLUGIN_TEST, GisPluginTestClass))
#define GIS_IS_PLUGIN_TEST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_PLUGIN_TEST))
#define GIS_PLUGIN_TEST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_PLUGIN_TEST, GisPluginTestClass))

typedef struct _GisPluginTest      GisPluginTest;
typedef struct _GisPluginTestClass GisPluginTestClass;

struct _GisPluginTest {
	GObject parent_instance;

	/* instance members */
	GisViewer *viewer;
};

struct _GisPluginTestClass {
	GObjectClass parent_class;
};

GType gis_plugin_test_get_type();

/* Methods */
GisPluginTest *gis_plugin_test_new(GisViewer *viewer);

#endif
