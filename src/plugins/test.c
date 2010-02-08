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

#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <gis.h>

#include "test.h"

/***********
 * Methods *
 ***********/
GisPluginTest *gis_plugin_test_new(GisViewer *viewer)
{
	g_debug("GisPluginTest: new");
	GisPluginTest *test = g_object_new(GIS_TYPE_PLUGIN_TEST, NULL);
	test->viewer = g_object_ref(viewer);

	GisMarker *marker = gis_marker_new("St. Charles");
	gis_point_set_lle(gis_object_center(marker), 38.841847, -90.491982, 0);
	GIS_OBJECT(marker)->lod = EARTH_R/4;
	test->marker = gis_viewer_add(test->viewer, GIS_OBJECT(marker), GIS_LEVEL_OVERLAY, 0);

	return test;
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void gis_plugin_test_plugin_init(GisPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GisPluginTest, gis_plugin_test, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GIS_TYPE_PLUGIN,
			gis_plugin_test_plugin_init));
static void gis_plugin_test_plugin_init(GisPluginInterface *iface)
{
	g_debug("GisPluginTest: plugin_init");
	/* Add methods to the interface */
}
/* Class/Object init */
static void gis_plugin_test_init(GisPluginTest *test)
{
	g_debug("GisPluginTest: init");
}
static void gis_plugin_test_dispose(GObject *_test)
{
	g_debug("GisPluginTest: dispose");
	GisPluginTest *test = GIS_PLUGIN_TEST(_test);
	if (test->viewer) {
		gis_viewer_remove(test->viewer, test->marker);
		g_object_unref(test->viewer);
		test->viewer = NULL;
	}
	G_OBJECT_CLASS(gis_plugin_test_parent_class)->finalize(_test);
}
static void gis_plugin_test_class_init(GisPluginTestClass *klass)
{
	g_debug("GisPluginTest: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = gis_plugin_test_dispose;
}
