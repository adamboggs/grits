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
 * SECTION:grits-opengl
 * @short_description: OpenGL based virtual globe
 *
 * #GritsOpenGL is the core rendering engine used by grits. Theoretically other
 * renderers could be writte, but they have not been. GritsOpenGL uses the ROAM
 * algorithm for updating surface mesh the planet. The only thing GritsOpenGL
 * can actually render on it's own is a wireframe of a sphere.
 *
 * GritsOpenGL requires (at least) OpenGL 2.0.
 */

#include <config.h>
#include <math.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "grits-opengl.h"
#include "grits-util.h"
#include "gtkgl.h"
#include "roam.h"

// #define ROAM_DEBUG

/* Tessellation, "finding intersecting triangles" */
/* http://research.microsoft.com/pubs/70307/tr-2006-81.pdf */
/* http://www.opengl.org/wiki/Alpha_Blending */

/* The unsorted/sroted GLists are blank head nodes,
 * This way us we can remove objects from the level just by fixing up links
 * I.e. we don't need to do a lookup to remove an object if we have its GList */
struct RenderLevel {
	GList unsorted;
	GList sorted;
};

/***********
 * Helpers *
 ***********/
static void _set_visuals(GritsOpenGL *opengl)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Camera 1 */
	double lat, lon, elev, rx, ry, rz;
	grits_viewer_get_location(GRITS_VIEWER(opengl), &lat, &lon, &elev);
	grits_viewer_get_rotation(GRITS_VIEWER(opengl), &rx, &ry, &rz);
	glRotatef(rx, 1, 0, 0);
	glRotatef(rz, 0, 0, 1);

	/* Lighting */
#ifdef ROAM_DEBUG
	float light_ambient[]  = {0.7f, 0.7f, 0.7f, 1.0f};
	float light_diffuse[]  = {2.0f, 2.0f, 2.0f, 1.0f};
#else
	float light_ambient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
	float light_diffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
#endif
	float light_position[] = {-13*EARTH_R, 1*EARTH_R, 3*EARTH_R, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	float material_ambient[]  = {1.0, 1.0, 1.0, 1.0};
	float material_diffuse[]  = {1.0, 1.0, 1.0, 1.0};
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

	glDisable(GL_ALPHA_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

#ifndef ROAM_DEBUG
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
#endif

	glEnable(GL_LINE_SMOOTH);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glShadeModel(GL_FLAT);

	g_mutex_lock(opengl->sphere_lock);
	roam_sphere_update_view(opengl->sphere);
	g_mutex_unlock(opengl->sphere_lock);
}

static gboolean _foreach_object_cb(gpointer key, gpointer value, gpointer pointers)
{
	struct RenderLevel *level = value;
	GFunc    user_func = ((gpointer*)pointers)[0];
	gpointer user_data = ((gpointer*)pointers)[1];
	for (GList *cur = level->unsorted.next; cur; cur = cur->next)
		user_func(cur->data, user_data);
	for (GList *cur = level->sorted.next;   cur; cur = cur->next)
		user_func(cur->data, user_data);
	return FALSE;
}

static void _foreach_object(GritsOpenGL *opengl, GFunc func, gpointer user_data)
{
	gpointer pointers[2] = {func, user_data};
	g_tree_foreach(opengl->objects, _foreach_object_cb, pointers);
}

/*************
 * Callbacks *
 *************/
static gboolean on_configure(GritsOpenGL *opengl, GdkEventConfigure *event, gpointer _)
{
	g_debug("GritsOpenGL: on_configure");

	double width  = GTK_WIDGET(opengl)->allocation.width;
	double height = GTK_WIDGET(opengl)->allocation.height;

	/* Setup OpenGL Window */
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double ang = atan(height/FOV_DIST);
	gluPerspective(rad2deg(ang)*2, width/height, 10, 100*EARTH_R);

#ifndef ROAM_DEBUG
	g_mutex_lock(opengl->sphere_lock);
	roam_sphere_update_errors(opengl->sphere);
	g_mutex_unlock(opengl->sphere_lock);
#endif

	return FALSE;
}

static gboolean _draw_level(gpointer key, gpointer value, gpointer user_data)
{
	g_debug("GritsOpenGL: _draw_level - level=%-4d", (int)key);
	GritsOpenGL *opengl = user_data;
	struct RenderLevel *level = value;
	int nsorted = 0, nunsorted = 0;
	GList *cur = NULL;

	/* Draw opaque objects without sorting */
	glDepthMask(TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	for (cur = level->unsorted.next; cur; cur = cur->next, nunsorted++)
		grits_object_draw(GRITS_OBJECT(cur->data), opengl);

	/* Freeze depth buffer and draw transparent objects sorted */
	/* TODO: sorting */
	//glDepthMask(FALSE);
	glAlphaFunc(GL_GREATER, 0.1);
	for (cur = level->sorted.next; cur; cur = cur->next, nsorted++)
		grits_object_draw(GRITS_OBJECT(cur->data), opengl);

	/* TODO: Prune empty levels */

	g_debug("GritsOpenGL: _draw_level - drew %d,%d objects",
			nunsorted, nsorted);
	return FALSE;
}

static gboolean on_expose(GritsOpenGL *opengl, GdkEventExpose *event, gpointer _)
{
	g_debug("GritsOpenGL: on_expose - begin");

	glClear(GL_COLOR_BUFFER_BIT);

	_set_visuals(opengl);
#ifdef ROAM_DEBUG
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glLineWidth(2);
	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	roam_sphere_draw(opengl->sphere);
	(void)_draw_level;
	//roam_sphere_draw_normals(opengl->sphere);
#else
	g_mutex_lock(opengl->objects_lock);
	g_tree_foreach(opengl->objects, _draw_level, opengl);
	if (opengl->wireframe) {
		glClear(GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		roam_sphere_draw(opengl->sphere);
		g_tree_foreach(opengl->objects, _draw_level, opengl);
	}
	g_mutex_unlock(opengl->objects_lock);
#endif

	gtk_gl_end(GTK_WIDGET(opengl));

	g_debug("GritsOpenGL: on_expose - end\n");
	return FALSE;
}

static gboolean on_motion_notify(GritsOpenGL *opengl, GdkEventMotion *event, gpointer _)
{
	gdouble height = GTK_WIDGET(opengl)->allocation.height;
	gdouble gl_x   = event->x;
	gdouble gl_y   = height - event->y;

	/* Configure view */
	gint viewport[4];
	gdouble projection[16];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(gl_x, gl_y, 2, 2, viewport);
	glMultMatrixd(projection);

	/* Prepare for picking */
	guint buffer[100][4] = {};
	glSelectBuffer(G_N_ELEMENTS(buffer), (guint*)buffer);
	glRenderMode(GL_SELECT);
	glInitNames();

	/* Run picking */
	g_mutex_lock(opengl->objects_lock);
	_foreach_object(opengl, (GFunc)grits_object_pick_begin, opengl);
	int hits = glRenderMode(GL_RENDER);
	g_debug("GritsOpenGL: on_motion_notify - hits=%d ev=%.0lf,%.0lf",
			hits, gl_x, gl_y);
	for (int i = 0; i < hits; i++) {
		//g_debug("\tHit: %d",     i);
		//g_debug("\t\tcount: %d", buffer[i][0]);
		//g_debug("\t\tz1:    %f", (float)buffer[i][1]/0x7fffffff);
		//g_debug("\t\tz2:    %f", (float)buffer[i][2]/0x7fffffff);
		//g_debug("\t\tname:  %p", (gpointer)buffer[i][3]);
		GritsObject *object = GRITS_OBJECT(buffer[i][3]);
		grits_object_pick_pointer(object, gl_x, gl_y);
	}
	_foreach_object(opengl, (GFunc)grits_object_pick_end, NULL);
	g_mutex_unlock(opengl->objects_lock);

	/* Cleanup */
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	return FALSE;
}

static gboolean on_key_press(GritsOpenGL *opengl, GdkEventKey *event, gpointer _)
{
	g_debug("GritsOpenGL: on_key_press - key=%x, state=%x, plus=%x",
			event->keyval, event->state, GDK_plus);

	guint kv = event->keyval;
	/* Testing */
	if (kv == GDK_w) {
		opengl->wireframe = !opengl->wireframe;
		gtk_widget_queue_draw(GTK_WIDGET(opengl));
	}
#ifdef ROAM_DEBUG
	else if (kv == GDK_n) roam_sphere_split_one(opengl->sphere);
	else if (kv == GDK_p) roam_sphere_merge_one(opengl->sphere);
	else if (kv == GDK_r) roam_sphere_split_merge(opengl->sphere);
	else if (kv == GDK_u) roam_sphere_update_errors(opengl->sphere);
	gtk_widget_queue_draw(GTK_WIDGET(opengl));
#endif
	return FALSE;
}

static gboolean on_chained_event(GritsOpenGL *opengl, GdkEvent *event, gpointer _)
{
	_foreach_object(opengl, (GFunc)grits_object_event, event);
	return FALSE;
}

static gboolean _update_errors_cb(gpointer _opengl)
{
	GritsOpenGL *opengl = _opengl;
	g_mutex_lock(opengl->sphere_lock);
	roam_sphere_update_errors(opengl->sphere);
	g_mutex_unlock(opengl->sphere_lock);
	opengl->ue_source = 0;
	return FALSE;
}
static void on_view_changed(GritsOpenGL *opengl,
		gdouble _1, gdouble _2, gdouble _3)
{
	g_debug("GritsOpenGL: on_view_changed");
	_set_visuals(opengl);
#ifndef ROAM_DEBUG
	if (!opengl->ue_source)
		opengl->ue_source = g_idle_add_full(G_PRIORITY_HIGH_IDLE+30,
				_update_errors_cb, opengl, NULL);
	//roam_sphere_update_errors(opengl->sphere);
#else
	(void)_update_errors_cb;
#endif
}

static gboolean on_idle(GritsOpenGL *opengl)
{
	//g_debug("GritsOpenGL: on_idle");
	g_mutex_lock(opengl->sphere_lock);
	if (roam_sphere_split_merge(opengl->sphere))
		gtk_widget_queue_draw(GTK_WIDGET(opengl));
	g_mutex_unlock(opengl->sphere_lock);
	return TRUE;
}

static void on_realize(GritsOpenGL *opengl, gpointer _)
{
	g_debug("GritsOpenGL: on_realize");
	gtk_gl_begin(GTK_WIDGET(opengl));

	/* Connect signals and idle functions now that opengl is fully initialized */
	gtk_widget_add_events(GTK_WIDGET(opengl), GDK_KEY_PRESS_MASK);
	g_signal_connect(opengl, "configure-event",  G_CALLBACK(on_configure),    NULL);
	g_signal_connect(opengl, "expose-event",     G_CALLBACK(on_expose),       NULL);

	g_signal_connect(opengl, "key-press-event",  G_CALLBACK(on_key_press),    NULL);

	g_signal_connect(opengl, "location-changed", G_CALLBACK(on_view_changed), NULL);
	g_signal_connect(opengl, "rotation-changed", G_CALLBACK(on_view_changed), NULL);

	g_signal_connect(opengl, "motion-notify-event", G_CALLBACK(on_motion_notify), NULL);
	g_signal_connect_after(opengl, "key-press-event",      G_CALLBACK(on_chained_event), NULL);
	g_signal_connect_after(opengl, "key-release-event",    G_CALLBACK(on_chained_event), NULL);
	g_signal_connect_after(opengl, "button-press-event",   G_CALLBACK(on_chained_event), NULL);
	g_signal_connect_after(opengl, "button-release-event", G_CALLBACK(on_chained_event), NULL);
	g_signal_connect_after(opengl, "motion-notify-event",  G_CALLBACK(on_chained_event), NULL);

#ifndef ROAM_DEBUG
	opengl->sm_source[0] = g_timeout_add_full(G_PRIORITY_HIGH_IDLE+30, 33,  (GSourceFunc)on_idle, opengl, NULL);
	opengl->sm_source[1] = g_timeout_add_full(G_PRIORITY_HIGH_IDLE+10, 500, (GSourceFunc)on_idle, opengl, NULL);
#else
	(void)on_idle;
	(void)_update_errors_cb;
#endif

	/* Re-queue resize incase configure was triggered before realize */
	gtk_widget_queue_resize(GTK_WIDGET(opengl));
}

/*********************
 * GritsViewer methods *
 *********************/
/**
 * grits_opengl_new:
 * @plugins: the plugins store to use
 * @prefs:   the preferences object to use
 *
 * Create a new OpenGL renderer.
 *
 * Returns: the new #GritsOpenGL
 */
GritsViewer *grits_opengl_new(GritsPlugins *plugins, GritsPrefs *prefs)
{
	g_debug("GritsOpenGL: new");
	GritsViewer *opengl = g_object_new(GRITS_TYPE_OPENGL, NULL);
	grits_viewer_setup(opengl, plugins, prefs);
	return opengl;
}

static void grits_opengl_center_position(GritsViewer *_opengl, gdouble lat, gdouble lon, gdouble elev)
{
	glRotatef(lon, 0, 1, 0);
	glRotatef(-lat, 1, 0, 0);
	glTranslatef(0, 0, elev2rad(elev));
}

static void grits_opengl_project(GritsViewer *_opengl,
		gdouble lat, gdouble lon, gdouble elev,
		gdouble *px, gdouble *py, gdouble *pz)
{
	GritsOpenGL *opengl = GRITS_OPENGL(_opengl);
	gdouble x, y, z;
	lle2xyz(lat, lon, elev, &x, &y, &z);
	gluProject(x, y, z,
		opengl->sphere->view->model,
		opengl->sphere->view->proj,
		opengl->sphere->view->view,
		px, py, pz);
}

static void grits_opengl_set_height_func(GritsViewer *_opengl, GritsBounds *bounds,
		RoamHeightFunc height_func, gpointer user_data, gboolean update)
{
	GritsOpenGL *opengl = GRITS_OPENGL(_opengl);
	/* TODO: get points? */
	g_mutex_lock(opengl->sphere_lock);
	GList *triangles = roam_sphere_get_intersect(opengl->sphere, TRUE,
			bounds->n, bounds->s, bounds->e, bounds->w);
	for (GList *cur = triangles; cur; cur = cur->next) {
		RoamTriangle *tri = cur->data;
		RoamPoint *points[] = {tri->p.l, tri->p.m, tri->p.r, tri->split};
		for (int i = 0; i < G_N_ELEMENTS(points); i++) {
			if (bounds->n >= points[i]->lat && points[i]->lat >= bounds->s &&
			    bounds->e >= points[i]->lon && points[i]->lon >= bounds->w) {
				points[i]->height_func = height_func;
				points[i]->height_data = user_data;
				roam_point_update_height(points[i]);
			}
		}
	}
	g_list_free(triangles);
	g_mutex_unlock(opengl->sphere_lock);
}

static void _grits_opengl_clear_height_func_rec(RoamTriangle *root)
{
	if (!root)
		return;
	RoamPoint *points[] = {root->p.l, root->p.m, root->p.r, root->split};
	for (int i = 0; i < G_N_ELEMENTS(points); i++) {
		points[i]->height_func = NULL;
		points[i]->height_data = NULL;
		roam_point_update_height(points[i]);
	}
	_grits_opengl_clear_height_func_rec(root->kids[0]);
	_grits_opengl_clear_height_func_rec(root->kids[1]);
}

static void grits_opengl_clear_height_func(GritsViewer *_opengl)
{
	GritsOpenGL *opengl = GRITS_OPENGL(_opengl);
	for (int i = 0; i < G_N_ELEMENTS(opengl->sphere->roots); i++)
		_grits_opengl_clear_height_func_rec(opengl->sphere->roots[i]);
}

static gpointer grits_opengl_add(GritsViewer *_opengl, GritsObject *object,
		gint key, gboolean sort)
{
	g_assert(GRITS_IS_OPENGL(_opengl));
	GritsOpenGL *opengl = GRITS_OPENGL(_opengl);
	g_mutex_lock(opengl->objects_lock);
	struct RenderLevel *level = g_tree_lookup(opengl->objects, (gpointer)key);
	if (!level) {
		level = g_new0(struct RenderLevel, 1);
		g_tree_insert(opengl->objects, (gpointer)key, level);
	}
	GList *list = sort ? &level->sorted : &level->unsorted;
	/* Put the link in the list */
	GList *link = g_new0(GList, 1);
	link->data = object;
	link->prev = list;
	link->next = list->next;
	if (list->next)
		list->next->prev = link;
	list->next = link;
	g_mutex_unlock(opengl->objects_lock);
	return link;
}

static GritsObject *grits_opengl_remove(GritsViewer *_opengl, GritsObject *object)
{
	g_assert(GRITS_IS_OPENGL(_opengl));
	GritsOpenGL *opengl = GRITS_OPENGL(_opengl);
	GList *link = object->ref;
	g_mutex_lock(opengl->objects_lock);
	/* Just unlink and free it, link->prev is assured */
	link->prev->next = link->next;
	if (link->next)
		link->next->prev = link->prev;
	g_mutex_unlock(opengl->objects_lock);
	object->ref    = NULL;
	object->viewer = NULL;
	g_free(link);
	g_object_unref(object);
	return object;
}

/****************
 * GObject code *
 ****************/
static int _objects_cmp(gconstpointer _a, gconstpointer _b, gpointer _)
{
	gint a = (int)_a, b = (int)_b;
	return a < b ? -1 :
	       a > b ?  1 : 0;
}
static void _objects_free(gpointer value)
{
	struct RenderLevel *level = value;
	if (level->sorted.next)
		g_list_free(level->sorted.next);
	if (level->unsorted.next)
		g_list_free(level->unsorted.next);
	g_free(level);
}

G_DEFINE_TYPE(GritsOpenGL, grits_opengl, GRITS_TYPE_VIEWER);
static void grits_opengl_init(GritsOpenGL *opengl)
{
	g_debug("GritsOpenGL: init");
	opengl->objects      = g_tree_new_full(_objects_cmp, NULL, NULL, _objects_free);
	opengl->objects_lock = g_mutex_new();
	opengl->sphere       = roam_sphere_new(opengl);
	opengl->sphere_lock  = g_mutex_new();
	gtk_gl_enable(GTK_WIDGET(opengl));
	g_signal_connect(opengl, "realize", G_CALLBACK(on_realize), NULL);
}
static void grits_opengl_dispose(GObject *_opengl)
{
	g_debug("GritsOpenGL: dispose");
	GritsOpenGL *opengl = GRITS_OPENGL(_opengl);
	if (opengl->sm_source[0]) {
		g_source_remove(opengl->sm_source[0]);
		opengl->sm_source[0] = 0;
	}
	if (opengl->sm_source[1]) {
		g_source_remove(opengl->sm_source[1]);
		opengl->sm_source[1] = 0;
	}
	if (opengl->ue_source) {
		g_source_remove(opengl->ue_source);
		opengl->ue_source = 0;
	}
	G_OBJECT_CLASS(grits_opengl_parent_class)->dispose(_opengl);
}
static void grits_opengl_finalize(GObject *_opengl)
{
	g_debug("GritsOpenGL: finalize");
	GritsOpenGL *opengl = GRITS_OPENGL(_opengl);
	roam_sphere_free(opengl->sphere);
	g_tree_destroy(opengl->objects);
	g_mutex_free(opengl->objects_lock);
	g_mutex_free(opengl->sphere_lock);
	G_OBJECT_CLASS(grits_opengl_parent_class)->finalize(_opengl);
}
static void grits_opengl_class_init(GritsOpenGLClass *klass)
{
	g_debug("GritsOpenGL: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->finalize = grits_opengl_finalize;
	gobject_class->dispose = grits_opengl_dispose;

	GritsViewerClass *viewer_class = GRITS_VIEWER_CLASS(klass);
	viewer_class->center_position   = grits_opengl_center_position;
	viewer_class->project           = grits_opengl_project;
	viewer_class->clear_height_func = grits_opengl_clear_height_func;
	viewer_class->set_height_func   = grits_opengl_set_height_func;
	viewer_class->add               = grits_opengl_add;
	viewer_class->remove            = grits_opengl_remove;
}
