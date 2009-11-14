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
	return self;
}

static void gis_plugin_test_expose(GisPlugin *_self)
{
	GisPluginTest *self = GIS_PLUGIN_TEST(_self);
	g_debug("GisPluginTest: expose");

	double width  = GTK_WIDGET(self->viewer)->allocation.width;
	double height = GTK_WIDGET(self->viewer)->allocation.height;

	// St. Charles
	// lat =  38.841847
	// lon = -90.491982
	gdouble px, py, pz;
	gis_viewer_project(self->viewer,
		38.841847, -90.491982, 0, &px, &py, &pz);
	py = height-py;

	//cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	//cairo_t *cairo = cairo_create(surface);
	//cairo_set_source_rgba(cairo, 1, 1, 1, 1);
	//cairo_arc(cairo, px, py, 4, 0, 2*G_PI);
	//cairo_fill(cairo);
	//cairo_move_to(cairo, px+4, py-8);
	//cairo_set_font_size(cairo, 10);
	//cairo_show_text(cairo, "Marker!");

	//guint tex;
	//glEnable(GL_TEXTURE_2D);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glPixelStorei(GL_PACK_ALIGNMENT, 1);
	//glGenTextures(1, &tex);
	//glBindTexture(GL_TEXTURE_2D, tex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
	//		cairo_image_surface_get_data(surface));
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	//glDisable(GL_COLOR_MATERIAL);
	//glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_LIGHTING);
	//glBegin(GL_QUADS);
	//glTexCoord2d(0, 0); glVertex3f(-1,  1, 1);
	//glTexCoord2d(1, 0); glVertex3f( 1,  1, 1);
	//glTexCoord2d(1, 1); glVertex3f( 1, -1, 1);
	//glTexCoord2d(0, 1); glVertex3f(-1, -1, 1);
	//glEnd();
	//glDeleteTextures(1, &tex);
	//cairo_destroy(cairo);
	//cairo_surface_destroy(surface);
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
	iface->expose = gis_plugin_test_expose;
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
