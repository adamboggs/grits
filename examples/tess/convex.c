/*
 * Copyright (C) 2011 Andy Spencer <andy753421@gmail.com>
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
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glu.h>

guint tex, texl, texr;

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer _)
{
	if (event->keyval == GDK_q)
		gtk_main_quit();
	return FALSE;
}

gboolean on_expose(GtkWidget *drawing, GdkEventExpose *event, gpointer _)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,1, -1,1, 10,-10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -5);

	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);

	/* Create vertexes */
	double verts[20][3];
	for (int i = 0; i < G_N_ELEMENTS(verts); i++) {
		float ang = 2*G_PI * i / G_N_ELEMENTS(verts);
		verts[i][0] = sin(ang) * (i%2+0.5) * 0.6;
		verts[i][1] = cos(ang) * (i%2+0.5) * 0.6;
		verts[i][2] = 0;
	}

	/* Draw raw polygon */
	glColor4f(0.5, 0.0, 0.0, 1.0);
	GLUtesselator *tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN,  glBegin);
	gluTessCallback(tess, GLU_TESS_VERTEX, glVertex3dv);
	gluTessCallback(tess, GLU_TESS_END,    glEnd);
	gluTessBeginPolygon(tess, NULL);
	gluTessBeginContour(tess);
	for (int i = 0; i < G_N_ELEMENTS(verts); i++)
		gluTessVertex(tess, verts[i], verts[i]);
	gluTessEndContour(tess);
	gluTessEndPolygon(tess);
	gluDeleteTess(tess);

	/* Draw tesselated polygon */
	//glColor4f(0.0, 0.0, 0.5, 1.0);
	//glBegin(GL_POLYGON);
	//for (int i = 0; i < G_N_ELEMENTS(verts); i++)
	//	glVertex3dv(verts[i]);
	//glEnd();

	/* Draw outline */
	glColor4f(0.8, 0.8, 0.8, 1.0);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < G_N_ELEMENTS(verts); i++)
		glVertex3dv(verts[i]);
	glEnd();

	/* Flush */
	GdkGLDrawable *gldrawable = gdk_gl_drawable_get_current();
	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();
	return FALSE;
}
gboolean on_configure(GtkWidget *drawing, GdkEventConfigure *event, gpointer _)
{
	glViewport(0, 0,
		drawing->allocation.width,
		drawing->allocation.height);
	return FALSE;
}

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);

	GtkWidget   *window   = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget   *drawing  = gtk_drawing_area_new();
	GdkGLConfig *glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)(
			GDK_GL_MODE_RGBA   | GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_DOUBLE | GDK_GL_MODE_ALPHA));
	g_signal_connect(window,  "destroy",         G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window,  "key-press-event", G_CALLBACK(on_key_press),  NULL);
	g_signal_connect(drawing, "expose-event",    G_CALLBACK(on_expose),     NULL);
	g_signal_connect(drawing, "configure-event", G_CALLBACK(on_configure),  NULL);
	gtk_widget_set_gl_capability(drawing, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
	gtk_container_add(GTK_CONTAINER(window), drawing);
	gtk_widget_show_all(window);

	/* OpenGL setup */
	GdkGLContext  *glcontext  = gtk_widget_get_gl_context(GTK_WIDGET(drawing));
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(drawing));
	gdk_gl_drawable_gl_begin(gldrawable, glcontext);

	/* Go */
	gtk_main();
	gdk_gl_drawable_gl_end(gldrawable);
}
