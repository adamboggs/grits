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

#include <math.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>

#include <gis.h>

#include "env.h"

/***********
 * Helpers *
 ***********/
static gpointer expose(GisCallback *callback, gpointer _self)
{
	GisPluginEnv *self = GIS_PLUGIN_ENV(_self);
	g_debug("GisPluginEnv: expose");

	gdouble lat, lon, elev;
	gis_viewer_get_location(self->viewer, &lat, &lon, &elev);

	/* Misc */
	gdouble rg   = MAX(0, 1-(elev/20000));
	gdouble blue = MAX(0, 1-(elev/50000));
	glClearColor(MIN(0.65,rg), MIN(0.65,rg), MIN(1,blue), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Attempt to render an atmosphere */
	/*
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	glBlendFunc(GL_ONE, GL_ONE);

	glMatrixMode(GL_MODELVIEW);

	elev = -EARTH_R;
	for (elev = -EARTH_R; elev < 0; elev += EARTH_R/10) {
		glPushMatrix();
		glColor4f(0.3, 0.3, 1.0, 0.2);
		gis_viewer_center_position(self->viewer, lat, lon, elev);

		glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0, 0, 0);
		for (gdouble i = 0; i <= 2*G_PI; i += G_PI/10) {
			gint rad = 1*EARTH_R + 300000;
			glVertex3f(rad*sin(i), rad*cos(i), 0);
			g_message("%f %f %f", 3*EARTH_R*sin(i), 3*EARTH_R*cos(i), 0.);
		}
		glEnd();
		glPopMatrix();
	}
	*/

	return NULL;
}


/***********
 * Methods *
 ***********/
GisPluginEnv *gis_plugin_env_new(GisViewer *viewer, GisPrefs *prefs)
{
	g_debug("GisPluginEnv: new");
	GisPluginEnv *self = g_object_new(GIS_TYPE_PLUGIN_ENV, NULL);
	self->viewer = g_object_ref(viewer);

	/* Create objects */
	GisCallback *callback   = gis_callback_new(expose, self);
	GisTile     *background = gis_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	glGenTextures(1, &self->tex);
	background->data = &self->tex;

	/* Add renderers */
	gpointer ref1, ref2;
	ref1 = gis_viewer_add(viewer, GIS_OBJECT(callback),   GIS_LEVEL_BACKGROUND, FALSE);
	ref2 = gis_viewer_add(viewer, GIS_OBJECT(background), GIS_LEVEL_BACKGROUND, FALSE);
	self->refs = g_list_prepend(self->refs, ref1);
	self->refs = g_list_prepend(self->refs, ref2);

	return self;
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void gis_plugin_env_plugin_init(GisPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GisPluginEnv, gis_plugin_env, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GIS_TYPE_PLUGIN,
			gis_plugin_env_plugin_init));
static void gis_plugin_env_plugin_init(GisPluginInterface *iface)
{
	g_debug("GisPluginEnv: plugin_init");
	/* Add methods to the interface */
}
/* Class/Object init */
static void gis_plugin_env_init(GisPluginEnv *self)
{
	g_debug("GisPluginEnv: init");
	/* Set defaults */
}
static void gis_plugin_env_dispose(GObject *gobject)
{
	g_debug("GisPluginEnv: dispose");
	GisPluginEnv *self = GIS_PLUGIN_ENV(gobject);
	/* Drop references */
	if (self->viewer) {
		for (GList *cur = self->refs; cur; cur = cur->next)
			gis_viewer_remove(self->viewer, cur->data);
		g_list_free(self->refs);
		g_object_unref(self->viewer);
		glDeleteTextures(1, &self->tex);
		self->viewer = NULL;
	}
	G_OBJECT_CLASS(gis_plugin_env_parent_class)->dispose(gobject);
}
static void gis_plugin_env_class_init(GisPluginEnvClass *klass)
{
	g_debug("GisPluginEnv: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose = gis_plugin_env_dispose;
}
