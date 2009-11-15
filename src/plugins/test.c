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
	GisPluginTest *self = g_object_new(GIS_TYPE_PLUGIN_TEST, NULL);
	self->viewer = viewer;

	GisMarker *marker = gis_marker_new("St. Charles");
	gis_point_set_lle(gis_object_center(marker), 38.841847, -90.491982, 0);
	gis_viewer_add_object(self->viewer, GIS_OBJECT(marker));

	return self;
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
static void gis_plugin_test_init(GisPluginTest *self)
{
	g_debug("GisPluginTest: init");
}
static void gis_plugin_test_class_init(GisPluginTestClass *klass)
{
	g_debug("GisPluginTest: class_init");
}
