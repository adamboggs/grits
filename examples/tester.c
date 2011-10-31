/*
 * Copyright (C) 2009-2011 Andy Spencer <andy753421@gmail.com>
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

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <math.h>

#include "tester.h"

/*************
 * Callbacks *
 *************/
static gboolean on_expose(GritsTester *tester, GdkEventExpose *event, gpointer _)
{
	g_debug("GritsTester: on_expose");

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.0);
	glEnable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glDisable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	/* Lighting */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	float light_ambient[]  = {0.1f, 0.1f, 0.1f, 1.0f};
	float light_diffuse[]  = {0.6f, 0.6f, 0.8f, 1.0f};
	float light_position[] = {-40.0f, 80.0f, 40.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glPopMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

	/* Draw */
	gdk_gl_draw_cube(FALSE, 2);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	for (GList *cur = tester->objects; cur; cur = cur->next)
		grits_object_draw(cur->data, NULL);
	if (tester->wireframe) {
		glDisable(GL_LIGHTING);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-1,0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (GList *cur = tester->objects; cur; cur = cur->next)
			grits_object_draw(cur->data, NULL);
	}

	/* Flush */
	GdkGLDrawable *gldrawable = gdk_gl_drawable_get_current();
	gdk_gl_drawable_swap_buffers(gldrawable);
	return FALSE;
}

static gboolean on_key_press(GritsTester *tester, GdkEventKey *event, gpointer _)
{
	g_debug("GritsTester: on_key_press");
	guint kv = event->keyval;
	gdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	     if (kv == GDK_q) gtk_main_quit();
        else if (kv == GDK_w) tester->wireframe = !tester->wireframe;
	else if (kv == GDK_h) glTranslated( 0.1,  0,  0);
	else if (kv == GDK_l) glTranslated(-0.1,  0,  0);
	else if (kv == GDK_j) glTranslated( 0,  0.1,  0);
	else if (kv == GDK_k) glTranslated( 0, -0.1,  0);
	else if (kv == GDK_i) glTranslated( 0,  0,  0.1);
	else if (kv == GDK_o) glTranslated( 0,  0, -0.1);
	else if (kv == GDK_H) glRotated(-5, 0,1,0);
	else if (kv == GDK_L) glRotated( 5, 0,1,0);
	else if (kv == GDK_J) glRotated( 5, 1,0,0);
	else if (kv == GDK_K) glRotated(-5, 1,0,0);
	else if (kv == GDK_I) glRotated(-5, 0,0,1);
	else if (kv == GDK_O) glRotated( 5, 0,0,1);
	glMultMatrixd(model);

	gtk_widget_queue_draw(GTK_WIDGET(tester));
	return FALSE;
}

static gboolean on_configure(GritsTester *tester, GdkEventConfigure *event, gpointer _)
{
	g_debug("GritsTester: on_configure");

	double width  = GTK_WIDGET(tester)->allocation.width;
	double height = GTK_WIDGET(tester)->allocation.height;

	/* Setup OpenGL Window */
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double ang = atan(height/FOV_DIST);
	gluPerspective(rad2deg(ang)*2, width/height, 0.001, 100);

	return FALSE;
}

static void on_realize(GritsTester *tester, gpointer _)
{
	g_debug("GritsTester: on_realize");

	/* Start OpenGL */
	GdkGLContext   *glcontext  = gtk_widget_get_gl_context(GTK_WIDGET(tester));
	GdkGLDrawable  *gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(tester));
	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		g_assert_not_reached();

	/* Connect signals and idle functions now that opengl is fully initialized */
	g_object_set(tester, "can-focus", TRUE, NULL);
	gtk_widget_add_events(GTK_WIDGET(tester), GDK_KEY_PRESS_MASK);
	g_signal_connect(tester, "configure-event", G_CALLBACK(on_configure), NULL);
	g_signal_connect(tester, "expose-event",    G_CALLBACK(on_expose),    NULL);
	g_signal_connect(tester, "key-press-event", G_CALLBACK(on_key_press), NULL);

	/* Re-queue resize incase configure was triggered before realize */
	gtk_widget_queue_resize(GTK_WIDGET(tester));

	/* Setup model view matrix */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0, 0, -2);
}

/***********
 * Methods *
 ***********/
gpointer grits_tester_add(GritsTester *tester, GritsObject *object)
{
	g_debug("GritsTester: add");
	gtk_widget_queue_draw(GTK_WIDGET(tester));
	object->viewer = (void*)tester;
	return tester->objects = g_list_prepend(tester->objects, object);
}

GritsObject *grits_tester_remove(GritsTester *tester, gpointer _ref)
{
	g_debug("GritsTester: remove");
	GList *ref = _ref;
	GritsObject *object = ref->data;
	tester->objects = g_list_remove_link(tester->objects, ref);
	return object;
}

GritsTester *grits_tester_new()
{
	g_debug("GritsTester: new");
	GritsTester *tester = g_object_new(GRITS_TYPE_TESTER, NULL);
	return tester;
}

/****************
 * GObject code *
 ****************/
G_DEFINE_TYPE(GritsTester, grits_tester, GTK_TYPE_DRAWING_AREA);
static void grits_tester_init(GritsTester *tester)
{
	g_debug("GritsTester: init");
	tester->objects = NULL;

	/* Set OpenGL before "realize" */
	GdkGLConfig *glconfig = gdk_gl_config_new_by_mode(
			GDK_GL_MODE_RGBA   | GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_DOUBLE | GDK_GL_MODE_ALPHA);
	if (!glconfig)
		g_error("Failed to create glconfig");
	if (!gtk_widget_set_gl_capability(GTK_WIDGET(tester),
				glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE))
		g_error("GL lacks required capabilities");
	g_object_unref(glconfig);

	/* Finish OpenGL init after it's realized */
	g_signal_connect(tester, "realize", G_CALLBACK(on_realize), NULL);
}
static void grits_tester_class_init(GritsTesterClass *klass)
{
	g_debug("GritsTester: class_init");
}
