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

/* Tessellation, "finding intersecting triangles" */
/* http://research.microsoft.com/pubs/70307/tr-2006-81.pdf */
/* http://www.opengl.org/wiki/Alpha_Blending */

#include <config.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "gis-opengl.h"
#include "roam.h"
#include "wms.h"

#define FOV_DIST   2000.0
#define MPPX(dist) (4*dist/FOV_DIST)

// #define ROAM_DEBUG

/*************
 * ROAM Code *
 *************/
static void set_visuals(GisOpenGL *self)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Camera 1 */
	double lat, lon, elev, rx, ry, rz;
	gis_view_get_location(self->view, &lat, &lon, &elev);
	gis_view_get_rotation(self->view, &rx, &ry, &rz);
	glRotatef(rx, 1, 0, 0);
	glRotatef(rz, 0, 0, 1);

	/* Lighting */
#ifdef ROAM_DEBUG
	float light_ambient[]  = {0.7f, 0.7f, 0.7f, 1.0f};
	float light_diffuse[]  = {2.0f, 2.0f, 2.0f, 1.0f};
#else
	float light_ambient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
	float light_diffuse[]  = {5.0f, 5.0f, 5.0f, 1.0f};
#endif
	float light_position[] = {-13*EARTH_R, 1*EARTH_R, 3*EARTH_R, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	float material_ambient[]  = {0.2, 0.2, 0.2, 1.0};
	float material_diffuse[]  = {0.8, 0.8, 0.8, 1.0};
	float material_specular[] = {0.0, 0.0, 0.0, 1.0};
	float material_emission[] = {0.0, 0.0, 0.0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  material_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  material_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_emission);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);

	/* Camera 2 */
	glTranslatef(0, 0, -elev2rad(elev));
	glRotatef(lat, 1, 0, 0);
	glRotatef(-lon, 0, 1, 0);

	/* Misc */
	gdouble rg   = MAX(0, 1-(elev/20000));
	gdouble blue = MAX(0, 1-(elev/50000));
	glClearColor(MIN(0.65,rg), MIN(0.65,rg), MIN(1,blue), 1.0f);

	glDisable(GL_ALPHA_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

#ifndef ROAM_DEBUG
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
#endif

	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LINE_SMOOTH);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glShadeModel(GL_FLAT);
}


/*************
 * Callbacks *
 *************/
static void on_realize(GisOpenGL *self, gpointer _)
{
	g_debug("GisOpenGL: on_realize");
	set_visuals(self);
	roam_sphere_update_errors(self->sphere);
}
static gboolean on_configure(GisOpenGL *self, GdkEventConfigure *event, gpointer _)
{
	g_debug("GisOpenGL: on_configure");
	gis_opengl_begin(self);

	double width  = GTK_WIDGET(self)->allocation.width;
	double height = GTK_WIDGET(self)->allocation.height;
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double ang = atan(height/FOV_DIST);
	gluPerspective(rad2deg(ang)*2, width/height, 1, 20*EARTH_R);

#ifndef ROAM_DEBUG
	roam_sphere_update_errors(self->sphere);
#endif

	gis_opengl_end(self);
	return FALSE;
}

static void on_expose_plugin(GisPlugin *plugin, gchar *name, GisOpenGL *self)
{
	set_visuals(self);
	glMatrixMode(GL_PROJECTION); glPushMatrix();
	glMatrixMode(GL_MODELVIEW);  glPushMatrix();
	gis_plugin_expose(plugin);
	glMatrixMode(GL_PROJECTION); glPopMatrix();
	glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}
static gboolean on_expose(GisOpenGL *self, GdkEventExpose *event, gpointer _)
{
	g_debug("GisOpenGL: on_expose - begin");
	gis_opengl_begin(self);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifndef ROAM_DEBUG
	gis_plugins_foreach(self->plugins, G_CALLBACK(on_expose_plugin), self);

	if (self->wireframe) {
		set_visuals(self);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		roam_sphere_draw(self->sphere);
	}
#else
	set_visuals(self);
	glColor4f(0.0, 0.0, 9.0, 0.6);
	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	roam_sphere_draw(self->sphere);

	//roam_sphere_draw_normals(self->sphere);
#endif

	set_visuals(self);
	gis_opengl_end(self);
	gis_opengl_flush(self);
	g_debug("GisOpenGL: on_expose - end\n");
	return FALSE;
}

static gboolean on_button_press(GisOpenGL *self, GdkEventButton *event, gpointer _)
{
	g_debug("GisOpenGL: on_button_press - Grabbing focus");
	gtk_widget_grab_focus(GTK_WIDGET(self));
	return TRUE;
}

static gboolean on_key_press(GisOpenGL *self, GdkEventKey *event, gpointer _)
{
	g_debug("GisOpenGL: on_key_press - key=%x, state=%x, plus=%x",
			event->keyval, event->state, GDK_plus);

	double lat, lon, elev, pan;
	gis_view_get_location(self->view, &lat, &lon, &elev);
	pan = MIN(elev/(EARTH_R/2), 30);
	guint kv = event->keyval;
	if      (kv == GDK_Left  || kv == GDK_h) gis_view_pan(self->view,  0,  -pan, 0);
	else if (kv == GDK_Down  || kv == GDK_j) gis_view_pan(self->view, -pan, 0,   0);
	else if (kv == GDK_Up    || kv == GDK_k) gis_view_pan(self->view,  pan, 0,   0);
	else if (kv == GDK_Right || kv == GDK_l) gis_view_pan(self->view,  0,   pan, 0);
	else if (kv == GDK_minus || kv == GDK_o) gis_view_zoom(self->view, 10./9);
	else if (kv == GDK_plus  || kv == GDK_i) gis_view_zoom(self->view, 9./10);
	else if (kv == GDK_H) gis_view_rotate(self->view,  0, 0, -2);
	else if (kv == GDK_J) gis_view_rotate(self->view,  2, 0,  0);
	else if (kv == GDK_K) gis_view_rotate(self->view, -2, 0,  0);
	else if (kv == GDK_L) gis_view_rotate(self->view,  0, 0,  2);

	/* Testing */
	else if (kv == GDK_w) {self->wireframe = !self->wireframe; gtk_widget_queue_draw(GTK_WIDGET(self));}
#ifdef ROAM_DEBUG
	else if (kv == GDK_n) roam_sphere_split_one(self->sphere);
	else if (kv == GDK_p) roam_sphere_merge_one(self->sphere);
	else if (kv == GDK_r) roam_sphere_split_merge(self->sphere);
	else if (kv == GDK_u) roam_sphere_update_errors(self->sphere);
	gtk_widget_queue_draw(GTK_WIDGET(self));
#endif

	return TRUE;
}

static void on_view_changed(GisView *view,
		gdouble _1, gdouble _2, gdouble _3, GisOpenGL *self)
{
	g_debug("GisOpenGL: on_view_changed");
	gis_opengl_begin(self);
	set_visuals(self);
#ifndef ROAM_DEBUG
	roam_sphere_update_errors(self->sphere);
#endif
	gis_opengl_redraw(self);
	gis_opengl_end(self);
}

static gboolean on_idle(GisOpenGL *self)
{
	//g_debug("GisOpenGL: on_idle");
	gis_opengl_begin(self);
	if (roam_sphere_split_merge(self->sphere))
		gis_opengl_redraw(self);
	gis_opengl_end(self);
	return TRUE;
}


/***********
 * Methods *
 ***********/
GisOpenGL *gis_opengl_new(GisWorld *world, GisView *view, GisPlugins *plugins)
{
	g_debug("GisOpenGL: new");
	GisOpenGL *self = g_object_new(GIS_TYPE_OPENGL, NULL);
	self->world   = world;
	self->view    = view;
	self->plugins = plugins;
	g_object_ref(world);
	g_object_ref(view);

	g_signal_connect(self->view, "location-changed", G_CALLBACK(on_view_changed), self);
	g_signal_connect(self->view, "rotation-changed", G_CALLBACK(on_view_changed), self);

	self->sphere = roam_sphere_new(self);

	return g_object_ref(self);
}

void gis_opengl_center_position(GisOpenGL *self, gdouble lat, gdouble lon, gdouble elev)
{
	glRotatef(lon, 0, 1, 0);
	glRotatef(-lat, 1, 0, 0);
	glTranslatef(0, 0, elev2rad(elev));
}

void gis_opengl_redraw(GisOpenGL *self)
{
	g_debug("GisOpenGL: redraw");
	gtk_widget_queue_draw(GTK_WIDGET(self));
}
void gis_opengl_begin(GisOpenGL *self)
{
	g_assert(GIS_IS_OPENGL(self));

	GdkGLContext   *glcontext  = gtk_widget_get_gl_context(GTK_WIDGET(self));
	GdkGLDrawable  *gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(self));

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		g_assert_not_reached();
}
void gis_opengl_end(GisOpenGL *self)
{
	g_assert(GIS_IS_OPENGL(self));
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(self));
	gdk_gl_drawable_gl_end(gldrawable);
}
void gis_opengl_flush(GisOpenGL *self)
{
	g_assert(GIS_IS_OPENGL(self));
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(self));
	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();
	gdk_gl_drawable_gl_end(gldrawable);
}


/****************
 * GObject code *
 ****************/
G_DEFINE_TYPE(GisOpenGL, gis_opengl, GTK_TYPE_DRAWING_AREA);
static void gis_opengl_init(GisOpenGL *self)
{
	g_debug("GisOpenGL: init");
	/* OpenGL setup */
	GdkGLConfig *glconfig = gdk_gl_config_new_by_mode(
			GDK_GL_MODE_RGBA   | GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_DOUBLE | GDK_GL_MODE_ALPHA);
	if (!glconfig)
		g_error("Failed to create glconfig");
	if (!gtk_widget_set_gl_capability(GTK_WIDGET(self),
				glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE))
		g_error("GL lacks required capabilities");
	g_object_unref(glconfig);

	gtk_widget_set_size_request(GTK_WIDGET(self), 600, 550);
	gtk_widget_set_events(GTK_WIDGET(self),
			GDK_BUTTON_PRESS_MASK |
			GDK_ENTER_NOTIFY_MASK |
			GDK_KEY_PRESS_MASK);
	g_object_set(self, "can-focus", TRUE, NULL);

#ifndef ROAM_DEBUG
	self->sm_source = g_timeout_add(10, (GSourceFunc)on_idle, self);
#endif

	g_signal_connect(self, "realize",            G_CALLBACK(on_realize),      NULL);
	g_signal_connect(self, "configure-event",    G_CALLBACK(on_configure),    NULL);
	g_signal_connect(self, "expose-event",       G_CALLBACK(on_expose),       NULL);

	g_signal_connect(self, "button-press-event", G_CALLBACK(on_button_press), NULL);
	g_signal_connect(self, "enter-notify-event", G_CALLBACK(on_button_press), NULL);
	g_signal_connect(self, "key-press-event",    G_CALLBACK(on_key_press),    NULL);
}
static GObject *gis_opengl_constructor(GType gtype, guint n_properties,
		GObjectConstructParam *properties)
{
	g_debug("GisOpengl: constructor");
	GObjectClass *parent_class = G_OBJECT_CLASS(gis_opengl_parent_class);
	return parent_class->constructor(gtype, n_properties, properties);
}
static void gis_opengl_dispose(GObject *_self)
{
	g_debug("GisOpenGL: dispose");
	GisOpenGL *self = GIS_OPENGL(_self);
	if (self->sm_source) {
		g_source_remove(self->sm_source);
		self->sm_source = 0;
	}
	if (self->sphere) {
		roam_sphere_free(self->sphere);
		self->sphere = NULL;
	}
	if (self->world) {
		g_object_unref(self->world);
		self->world = NULL;
	}
	if (self->view) {
		g_object_unref(self->view);
		self->view = NULL;
	}
	G_OBJECT_CLASS(gis_opengl_parent_class)->dispose(_self);
}
static void gis_opengl_finalize(GObject *_self)
{
	g_debug("GisOpenGL: finalize");
	GisOpenGL *self = GIS_OPENGL(_self);
	G_OBJECT_CLASS(gis_opengl_parent_class)->finalize(_self);
}
static void gis_opengl_class_init(GisOpenGLClass *klass)
{
	g_debug("GisOpenGL: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->constructor  = gis_opengl_constructor;
	gobject_class->dispose      = gis_opengl_dispose;
	gobject_class->finalize     = gis_opengl_finalize;
}
