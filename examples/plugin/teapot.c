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

#include <grits.h>

#include "teapot.h"

/***********
 * Helpers *
 ***********/
static gboolean rotate(gpointer _teapot)
{
	GritsPluginTeapot *teapot = _teapot;
	if (gtk_toggle_button_get_active(teapot->button)) {
		teapot->rotation += 1.0;
		gtk_widget_queue_draw(GTK_WIDGET(teapot->viewer));
	}
	return TRUE;
}

static void expose(GritsCallback *callback, GritsOpenGL *opengl, gpointer _teapot)
{
	GritsPluginTeapot *teapot = GRITS_PLUGIN_TEAPOT(_teapot);
	g_debug("GritsPluginTeapot: expose");

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
GritsPluginTeapot *grits_plugin_teapot_new(GritsViewer *viewer, GritsPrefs *prefs)
{
	g_debug("GritsPluginTeapot: new");
	GritsPluginTeapot *teapot = g_object_new(GRITS_TYPE_PLUGIN_TEAPOT, NULL);
	teapot->viewer = viewer;

	/* Add renderers */
	GritsCallback *callback = grits_callback_new(expose, teapot);
	grits_viewer_add(viewer, GRITS_OBJECT(callback), GRITS_LEVEL_OVERLAY+1, 0);

	return teapot;
}

static GtkWidget *grits_plugin_teapot_get_config(GritsPlugin *_teapot)
{
	GritsPluginTeapot *teapot = GRITS_PLUGIN_TEAPOT(_teapot);
	return GTK_WIDGET(teapot->button);
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void grits_plugin_teapot_plugin_init(GritsPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GritsPluginTeapot, grits_plugin_teapot, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GRITS_TYPE_PLUGIN,
			grits_plugin_teapot_plugin_init));
static void grits_plugin_teapot_plugin_init(GritsPluginInterface *iface)
{
	g_debug("GritsPluginTeapot: plugin_init");
	/* Add methods to the interface */
	iface->get_config = grits_plugin_teapot_get_config;
}
/* Class/Object init */
static void grits_plugin_teapot_init(GritsPluginTeapot *teapot)
{
	g_debug("GritsPluginTeapot: init");
	/* Set defaults */
	teapot->button    = GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_label("Rotate"));
	teapot->rotate_id = g_timeout_add(1000/60, rotate, teapot);
	teapot->rotation  = 30.0;
}
static void grits_plugin_teapot_dispose(GObject *gobject)
{
	g_debug("GritsPluginTeapot: dispose");
	GritsPluginTeapot *teapot = GRITS_PLUGIN_TEAPOT(gobject);
	g_source_remove(teapot->rotate_id);
	/* Drop references */
	G_OBJECT_CLASS(grits_plugin_teapot_parent_class)->dispose(gobject);
}
static void grits_plugin_teapot_class_init(GritsPluginTeapotClass *klass)
{
	g_debug("GritsPluginTeapot: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose  = grits_plugin_teapot_dispose;
}
