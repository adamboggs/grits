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

#include <config.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gis-marshal.h"
#include "gis-viewer.h"

#include "gis-util.h"


/* Constants */
enum {
	SIG_TIME_CHANGED,
	SIG_LOCATION_CHANGED,
	SIG_ROTATION_CHANGED,
	SIG_REFRESH,
	SIG_OFFLINE,
	NUM_SIGNALS,
};
static guint signals[NUM_SIGNALS];


/***********
 * Helpers *
 ***********/
/* Misc helpers */
static void _gis_viewer_fix_location(GisViewer *viewer)
{
	while (viewer->location[0] <  -90) viewer->location[0] += 180;
	while (viewer->location[0] >   90) viewer->location[0] -= 180;
	while (viewer->location[1] < -180) viewer->location[1] += 360;
	while (viewer->location[1] >  180) viewer->location[1] -= 360;
	viewer->location[2] = ABS(viewer->location[2]);
}

/* Signal helpers */
static void _gis_viewer_emit_location_changed(GisViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_LOCATION_CHANGED], 0,
			viewer->location[0],
			viewer->location[1],
			viewer->location[2]);
}
static void _gis_viewer_emit_rotation_changed(GisViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_ROTATION_CHANGED], 0,
			viewer->rotation[0],
			viewer->rotation[1],
			viewer->rotation[2]);
}
static void _gis_viewer_emit_time_changed(GisViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_TIME_CHANGED], 0,
			viewer->time);
}
static void _gis_viewer_emit_refresh(GisViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_REFRESH], 0);
}
static void _gis_viewer_emit_offline(GisViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_OFFLINE], 0,
			viewer->offline);
}

/*************
 * Callbacks *
 *************/
static gboolean on_key_press(GisViewer *viewer, GdkEventKey *event, gpointer _)
{
	g_debug("GisViewer: on_key_press - key=%x, state=%x, plus=%x",
			event->keyval, event->state, GDK_plus);

	double lat, lon, elev, pan;
	gis_viewer_get_location(viewer, &lat, &lon, &elev);
	pan = MIN(elev/(EARTH_R/2), 30);
	guint kv = event->keyval;
	gdk_threads_leave();
	if      (kv == GDK_Left  || kv == GDK_h) gis_viewer_pan(viewer,  0,  -pan, 0);
	else if (kv == GDK_Down  || kv == GDK_j) gis_viewer_pan(viewer, -pan, 0,   0);
	else if (kv == GDK_Up    || kv == GDK_k) gis_viewer_pan(viewer,  pan, 0,   0);
	else if (kv == GDK_Right || kv == GDK_l) gis_viewer_pan(viewer,  0,   pan, 0);
	else if (kv == GDK_minus || kv == GDK_o) gis_viewer_zoom(viewer, 10./9);
	else if (kv == GDK_plus  || kv == GDK_i) gis_viewer_zoom(viewer, 9./10);
	else if (kv == GDK_H) gis_viewer_rotate(viewer,  0, 0, -2);
	else if (kv == GDK_J) gis_viewer_rotate(viewer,  2, 0,  0);
	else if (kv == GDK_K) gis_viewer_rotate(viewer, -2, 0,  0);
	else if (kv == GDK_L) gis_viewer_rotate(viewer,  0, 0,  2);
	return FALSE;
}

enum {
	GIS_DRAG_NONE,
	GIS_DRAG_PAN,
	GIS_DRAG_ZOOM,
	GIS_DRAG_TILT,
};

static gboolean on_button_press(GisViewer *viewer, GdkEventButton *event, gpointer _)
{
	g_debug("GisViewer: on_button_press - %d", event->button);
	gtk_widget_grab_focus(GTK_WIDGET(viewer));
	switch (event->button) {
	case 1:  viewer->drag_mode = GIS_DRAG_PAN;  break;
	case 2:  viewer->drag_mode = GIS_DRAG_ZOOM; break;
	case 3:  viewer->drag_mode = GIS_DRAG_TILT; break;
	defualt: viewer->drag_mode = GIS_DRAG_NONE; break;
	}
	viewer->drag_x = event->x;
	viewer->drag_y = event->y;
	return FALSE;
}

static gboolean on_button_release(GisViewer *viewer, GdkEventButton *event, gpointer _)
{
	g_debug("GisViewer: on_button_release");
	viewer->drag_mode = GIS_DRAG_NONE;
	return FALSE;
}

static gboolean on_motion_notify(GisViewer *viewer, GdkEventMotion *event, gpointer _)
{
	gdouble x_dist = viewer->drag_x - event->x;
	gdouble y_dist = viewer->drag_y - event->y;
	gdouble lat, lon, elev, scale;
	gis_viewer_get_location(GIS_VIEWER(viewer), &lat, &lon, &elev);
	scale = elev/EARTH_R/15;
	switch (viewer->drag_mode) {
	case GIS_DRAG_PAN:
		gis_viewer_pan(viewer, -y_dist*scale, x_dist*scale, 0);
		break;
	case GIS_DRAG_ZOOM:
		gis_viewer_zoom(viewer, pow(2, -y_dist/500));
		break;
	case GIS_DRAG_TILT:
		gis_viewer_rotate(viewer, y_dist/10, 0, x_dist/10);
		break;
	}
	viewer->drag_x = event->x;
	viewer->drag_y = event->y;
	return FALSE;
}

static void on_view_changed(GisViewer *viewer,
		gdouble _1, gdouble _2, gdouble _3)
{
	gtk_widget_queue_draw(GTK_WIDGET(viewer));
}

/***********
 * Methods *
 ***********/
void gis_viewer_setup(GisViewer *viewer, GisPlugins *plugins, GisPrefs *prefs)
{
	viewer->plugins = plugins;
	viewer->prefs   = prefs;
	viewer->offline = gis_prefs_get_boolean(prefs, "gis/offline", NULL);
}

void gis_viewer_set_time(GisViewer *viewer, const char *time)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: set_time - time=%s", time);
	g_free(viewer->time);
	viewer->time = g_strdup(time);
	_gis_viewer_emit_time_changed(viewer);
}

gchar *gis_viewer_get_time(GisViewer *viewer)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: get_time");
	return viewer->time;
}

void gis_viewer_set_location(GisViewer *viewer, gdouble lat, gdouble lon, gdouble elev)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: set_location");
	viewer->location[0] = lat;
	viewer->location[1] = lon;
	viewer->location[2] = elev;
	_gis_viewer_fix_location(viewer);
	_gis_viewer_emit_location_changed(viewer);
}

void gis_viewer_get_location(GisViewer *viewer, gdouble *lat, gdouble *lon, gdouble *elev)
{
	g_assert(GIS_IS_VIEWER(viewer));
	//g_debug("GisViewer: get_location");
	*lat  = viewer->location[0];
	*lon  = viewer->location[1];
	*elev = viewer->location[2];
}

void gis_viewer_pan(GisViewer *viewer, gdouble forward, gdouble sideways, gdouble up)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: pan - forward=%8.3f, sideways=%8.3f, up=%8.3f",
			forward, sideways, up);
	gdouble dist   = sqrt(forward*forward + sideways*sideways);
	gdouble angle1 = deg2rad(viewer->rotation[2]);
	gdouble angle2 = atan2(sideways, forward);
	gdouble angle  = angle1 + angle2;
	g_message("pan: dist=%f, angle=%f+%f=%f move=%f,%f",
			dist, angle1, angle2, angle,
			dist*cos(angle),
			dist*sin(angle));
	/* This isn't accurate, but it's usable */
	viewer->location[0] += dist*cos(angle);
	viewer->location[1] += dist*sin(angle);
	viewer->location[2] += up;
	_gis_viewer_fix_location(viewer);
	_gis_viewer_emit_location_changed(viewer);
}

void gis_viewer_zoom(GisViewer *viewer, gdouble scale)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: zoom");
	viewer->location[2] *= scale;
	_gis_viewer_emit_location_changed(viewer);
}

void gis_viewer_set_rotation(GisViewer *viewer, gdouble x, gdouble y, gdouble z)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: set_rotation");
	viewer->rotation[0] = x;
	viewer->rotation[1] = y;
	viewer->rotation[2] = z;
	_gis_viewer_emit_rotation_changed(viewer);
}

void gis_viewer_get_rotation(GisViewer *viewer, gdouble *x, gdouble *y, gdouble *z)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: get_rotation");
	*x = viewer->rotation[0];
	*y = viewer->rotation[1];
	*z = viewer->rotation[2];
}

void gis_viewer_rotate(GisViewer *viewer, gdouble x, gdouble y, gdouble z)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: rotate - x=%.0f, y=%.0f, z=%.0f", x, y, z);
	viewer->rotation[0] += x;
	viewer->rotation[1] += y;
	viewer->rotation[2] += z;
	_gis_viewer_emit_rotation_changed(viewer);
}

void gis_viewer_refresh(GisViewer *viewer)
{
	g_debug("GisViewer: refresh");
	_gis_viewer_emit_refresh(viewer);
}

void gis_viewer_set_offline(GisViewer *viewer, gboolean offline)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: set_offline - %d", offline);
	gis_prefs_set_boolean(viewer->prefs, "gis/offline", offline);
	viewer->offline = offline;
	_gis_viewer_emit_offline(viewer);
}

gboolean gis_viewer_get_offline(GisViewer *viewer)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: get_offline - %d", viewer->offline);
	return viewer->offline;
}

/* To be implemented by subclasses */
void gis_viewer_center_position(GisViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->center_position)
		g_warning("GisViewer: center_position - Unimplemented");
	klass->center_position(viewer, lat, lon, elev);
}

void gis_viewer_project(GisViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev,
		gdouble *px, gdouble *py, gdouble *pz)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->project)
		g_warning("GisViewer: project - Unimplemented");
	klass->project(viewer, lat, lon, elev, px, py, pz);
}

void gis_viewer_clear_height_func(GisViewer *viewer)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->clear_height_func)
		g_warning("GisViewer: clear_height_func - Unimplemented");
	klass->clear_height_func(viewer);
}

void gis_viewer_set_height_func(GisViewer *viewer, GisTile *tile,
		GisHeightFunc height_func, gpointer user_data,
		gboolean update)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->set_height_func)
		g_warning("GisViewer: set_height_func - Unimplemented");
	klass->set_height_func(viewer, tile, height_func, user_data, update);
}

gpointer gis_viewer_add(GisViewer *viewer, GisObject *object,
		gint level, gboolean sort)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->add)
		g_warning("GisViewer: add - Unimplemented");
	return klass->add(viewer, object, level, sort);
}

GisObject *gis_viewer_remove(GisViewer *viewer, gpointer ref)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->remove)
		g_warning("GisViewer: remove - Unimplemented");
	return klass->remove(viewer, ref);
}

/****************
 * GObject code *
 ****************/
G_DEFINE_ABSTRACT_TYPE(GisViewer, gis_viewer, GTK_TYPE_DRAWING_AREA);
static void gis_viewer_init(GisViewer *viewer)
{
	g_debug("GisViewer: init");
	/* Default values */
	viewer->time = g_strdup("");
	viewer->location[0] = 40;
	viewer->location[1] = -100;
	viewer->location[2] = 1.5*EARTH_R;
	viewer->rotation[0] = 0;
	viewer->rotation[1] = 0;
	viewer->rotation[2] = 0;

	g_object_set(viewer, "can-focus", TRUE, NULL);
	gtk_widget_add_events(GTK_WIDGET(viewer),
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK |
			GDK_KEY_PRESS_MASK);

	g_signal_connect(viewer, "key-press-event",      G_CALLBACK(on_key_press),      NULL);

	g_signal_connect(viewer, "button-press-event",   G_CALLBACK(on_button_press),   NULL);
	g_signal_connect(viewer, "button-release-event", G_CALLBACK(on_button_release), NULL);
	g_signal_connect(viewer, "motion-notify-event",  G_CALLBACK(on_motion_notify),  NULL);

	g_signal_connect(viewer, "location-changed",     G_CALLBACK(on_view_changed),   NULL);
	g_signal_connect(viewer, "rotation-changed",     G_CALLBACK(on_view_changed),   NULL);
}
static void gis_viewer_finalize(GObject *gobject)
{
	g_debug("GisViewer: finalize");
	GisViewer *viewer = GIS_VIEWER(gobject);
	g_free(viewer->time);
	G_OBJECT_CLASS(gis_viewer_parent_class)->finalize(gobject);
}
static void gis_viewer_class_init(GisViewerClass *klass)
{
	g_debug("GisViewer: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->finalize     = gis_viewer_finalize;
	signals[SIG_TIME_CHANGED] = g_signal_new(
			"time-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__STRING,
			G_TYPE_NONE,
			1,
			G_TYPE_STRING);
	signals[SIG_LOCATION_CHANGED] = g_signal_new(
			"location-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			gis_cclosure_marshal_VOID__DOUBLE_DOUBLE_DOUBLE,
			G_TYPE_NONE,
			3,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE);
	signals[SIG_ROTATION_CHANGED] = g_signal_new(
			"rotation-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			gis_cclosure_marshal_VOID__DOUBLE_DOUBLE_DOUBLE,
			G_TYPE_NONE,
			3,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE);
	signals[SIG_REFRESH] = g_signal_new(
			"refresh",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			0);
	signals[SIG_OFFLINE] = g_signal_new(
			"offline",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__BOOLEAN,
			G_TYPE_NONE,
			1,
			G_TYPE_BOOLEAN);
}
