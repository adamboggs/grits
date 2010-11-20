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

/**
 * SECTION:test
 * @short_description: Testing plugin
 *
 * #GritsPluginTest is a testing plugin used during development and as an example
 * for how to create a plugin.
 */

#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <grits.h>

#include "test.h"

/***********
 * Methods *
 ***********/
gboolean _load_marker(gpointer _test)
{
	GritsPluginTest *test = _test;
	GritsMarker *marker = grits_marker_new("St. Charles");
	grits_point_set_lle(grits_object_center(marker), 38.841847, -90.491982, 0);
	GRITS_OBJECT(marker)->lod = EARTH_R;
	test->marker = grits_viewer_add(test->viewer, GRITS_OBJECT(marker), GRITS_LEVEL_OVERLAY, 0);
	return FALSE;
}
/**
 * grits_plugin_test_new:
 * @viewer: the #GritsViewer to use for drawing
 *
 * Create a new instance of the testing plugin.
 *
 * Returns: the new #GritsPluginTest
 */
GritsPluginTest *grits_plugin_test_new(GritsViewer *viewer)
{
	g_debug("GritsPluginTest: new");
	GritsPluginTest *test = g_object_new(GRITS_TYPE_PLUGIN_TEST, NULL);
	test->viewer = g_object_ref(viewer);
	g_idle_add(_load_marker, test);
	return test;
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void grits_plugin_test_plugin_init(GritsPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GritsPluginTest, grits_plugin_test, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GRITS_TYPE_PLUGIN,
			grits_plugin_test_plugin_init));
static void grits_plugin_test_plugin_init(GritsPluginInterface *iface)
{
	g_debug("GritsPluginTest: plugin_init");
	/* Add methods to the interface */
}
/* Class/Object init */
static void grits_plugin_test_init(GritsPluginTest *test)
{
	g_debug("GritsPluginTest: init");
}
static void grits_plugin_test_dispose(GObject *_test)
{
	g_debug("GritsPluginTest: dispose");
	GritsPluginTest *test = GRITS_PLUGIN_TEST(_test);
	if (test->viewer) {
		grits_viewer_remove(test->viewer, test->marker);
		g_object_unref(test->viewer);
		test->viewer = NULL;
	}
	G_OBJECT_CLASS(grits_plugin_test_parent_class)->finalize(_test);
}
static void grits_plugin_test_class_init(GritsPluginTestClass *klass)
{
	g_debug("GritsPluginTest: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = grits_plugin_test_dispose;
}
