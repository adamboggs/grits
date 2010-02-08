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

#include <gis/gis.h>

#include "teapot.h"

/***********
 * Helpers *
 ***********/
static gboolean rotate(gpointer _teapot)
{
	GisPluginTeapot *teapot = _teapot;
	if (gtk_toggle_button_get_active(teapot->button)) {
		teapot->rotation += 1.0;
		gtk_widget_queue_draw(GTK_WIDGET(teapot->viewer));
	}
	return TRUE;
}

static void expose(GisCallback *callback, gpointer _teapot)
{
	GisPluginTeapot *teapot = GIS_PLUGIN_TEAPOT(_teapot);
	g_debug("GisPluginTeapot: expose");

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(1,-1, -1,1, -10,10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float light_ambient[]  = {0.1f, 0.1f, 0.0f, 1.0f};
	float light_diffuse[]  = {0.9f, 0.9f, 0.9f, 1.0f};
	float light_position[] = {-30.0f, 50.0f, 40.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_COLOR_MATERIAL);

	glTranslatef(-0.5, -0.5, -2);
	glRotatef(teapot->rotation, 1, 1, 0);
	glColor4f(0.9, 0.9, 0.7, 1.0);
	glDisable(GL_CULL_FACE);
	gdk_gl_draw_teapot(TRUE, 0.25);
}


/***********
 * Methods *
 ***********/
GisPluginTeapot *gis_plugin_teapot_new(GisViewer *viewer, GisPrefs *prefs)
{
	g_debug("GisPluginTeapot: new");
	GisPluginTeapot *teapot = g_object_new(GIS_TYPE_PLUGIN_TEAPOT, NULL);
	teapot->viewer = viewer;

	/* Add renderers */
	GisCallback *callback = gis_callback_new(expose, teapot);
	gis_viewer_add(viewer, GIS_OBJECT(callback), GIS_LEVEL_WORLD, 0);

	return teapot;
}

static GtkWidget *gis_plugin_teapot_get_config(GisPlugin *_teapot)
{
	GisPluginTeapot *teapot = GIS_PLUGIN_TEAPOT(_teapot);
	return GTK_WIDGET(teapot->button);
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void gis_plugin_teapot_plugin_init(GisPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GisPluginTeapot, gis_plugin_teapot, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GIS_TYPE_PLUGIN,
			gis_plugin_teapot_plugin_init));
static void gis_plugin_teapot_plugin_init(GisPluginInterface *iface)
{
	g_debug("GisPluginTeapot: plugin_init");
	/* Add methods to the interface */
	iface->get_config = gis_plugin_teapot_get_config;
}
/* Class/Object init */
static void gis_plugin_teapot_init(GisPluginTeapot *teapot)
{
	g_debug("GisPluginTeapot: init");
	/* Set defaults */
	teapot->button    = GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_label("Rotate"));
	teapot->rotate_id = g_timeout_add(1000/60, rotate, teapot);
	teapot->rotation  = 30.0;
}
static void gis_plugin_teapot_dispose(GObject *gobject)
{
	g_debug("GisPluginTeapot: dispose");
	GisPluginTeapot *teapot = GIS_PLUGIN_TEAPOT(gobject);
	g_source_remove(teapot->rotate_id);
	/* Drop references */
	G_OBJECT_CLASS(gis_plugin_teapot_parent_class)->dispose(gobject);
}
static void gis_plugin_teapot_class_init(GisPluginTeapotClass *klass)
{
	g_debug("GisPluginTeapot: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose  = gis_plugin_teapot_dispose;
}
