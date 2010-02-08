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
static void _gis_viewer_fix_location(GisViewer *self)
{
	while (self->location[0] <  -90) self->location[0] += 180;
	while (self->location[0] >   90) self->location[0] -= 180;
	while (self->location[1] < -180) self->location[1] += 360;
	while (self->location[1] >  180) self->location[1] -= 360;
	self->location[2] = ABS(self->location[2]);
}

/* Signal helpers */
static void _gis_viewer_emit_location_changed(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_LOCATION_CHANGED], 0,
			self->location[0],
			self->location[1],
			self->location[2]);
}
static void _gis_viewer_emit_rotation_changed(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_ROTATION_CHANGED], 0,
			self->rotation[0],
			self->rotation[1],
			self->rotation[2]);
}
static void _gis_viewer_emit_time_changed(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_TIME_CHANGED], 0,
			self->time);
}
static void _gis_viewer_emit_refresh(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_REFRESH], 0);
}
static void _gis_viewer_emit_offline(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_OFFLINE], 0,
			self->offline);
}

/*************
 * Callbacks *
 *************/
static gboolean on_key_press(GisViewer *self, GdkEventKey *event, gpointer _)
{
	g_debug("GisViewer: on_key_press - key=%x, state=%x, plus=%x",
			event->keyval, event->state, GDK_plus);

	double lat, lon, elev, pan;
	gis_viewer_get_location(self, &lat, &lon, &elev);
	pan = MIN(elev/(EARTH_R/2), 30);
	guint kv = event->keyval;
	gdk_threads_leave();
	if      (kv == GDK_Left  || kv == GDK_h) gis_viewer_pan(self,  0,  -pan, 0);
	else if (kv == GDK_Down  || kv == GDK_j) gis_viewer_pan(self, -pan, 0,   0);
	else if (kv == GDK_Up    || kv == GDK_k) gis_viewer_pan(self,  pan, 0,   0);
	else if (kv == GDK_Right || kv == GDK_l) gis_viewer_pan(self,  0,   pan, 0);
	else if (kv == GDK_minus || kv == GDK_o) gis_viewer_zoom(self, 10./9);
	else if (kv == GDK_plus  || kv == GDK_i) gis_viewer_zoom(self, 9./10);
	else if (kv == GDK_H) gis_viewer_rotate(self,  0, 0, -2);
	else if (kv == GDK_J) gis_viewer_rotate(self,  2, 0,  0);
	else if (kv == GDK_K) gis_viewer_rotate(self, -2, 0,  0);
	else if (kv == GDK_L) gis_viewer_rotate(self,  0, 0,  2);
	return FALSE;
}

enum {
	GIS_DRAG_NONE,
	GIS_DRAG_PAN,
	GIS_DRAG_ZOOM,
	GIS_DRAG_TILT,
};

static gboolean on_button_press(GisViewer *self, GdkEventButton *event, gpointer _)
{
	g_debug("GisViewer: on_button_press - %d", event->button);
	gtk_widget_grab_focus(GTK_WIDGET(self));
	switch (event->button) {
	case 1:  self->drag_mode = GIS_DRAG_PAN;  break;
	case 2:  self->drag_mode = GIS_DRAG_ZOOM; break;
	case 3:  self->drag_mode = GIS_DRAG_TILT; break;
	defualt: self->drag_mode = GIS_DRAG_NONE; break;
	}
	self->drag_x = event->x;
	self->drag_y = event->y;
	return FALSE;
}

static gboolean on_button_release(GisViewer *self, GdkEventButton *event, gpointer _)
{
	g_debug("GisViewer: on_button_release");
	self->drag_mode = GIS_DRAG_NONE;
	return FALSE;
}

static gboolean on_motion_notify(GisViewer *self, GdkEventMotion *event, gpointer _)
{
	gdouble x_dist = self->drag_x - event->x;
	gdouble y_dist = self->drag_y - event->y;
	gdouble lat, lon, elev, scale;
	gis_viewer_get_location(GIS_VIEWER(self), &lat, &lon, &elev);
	scale = elev/EARTH_R/15;
	switch (self->drag_mode) {
	case GIS_DRAG_PAN:
		gis_viewer_pan(self, -y_dist*scale, x_dist*scale, 0);
		break;
	case GIS_DRAG_ZOOM:
		gis_viewer_zoom(self, pow(2, -y_dist/500));
		break;
	case GIS_DRAG_TILT:
		gis_viewer_rotate(self, y_dist/10, 0, x_dist/10);
		break;
	}
	self->drag_x = event->x;
	self->drag_y = event->y;
	return FALSE;
}

static void on_view_changed(GisViewer *self,
		gdouble _1, gdouble _2, gdouble _3)
{
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

/***********
 * Methods *
 ***********/
void gis_viewer_setup(GisViewer *self, GisPlugins *plugins, GisPrefs *prefs)
{
	self->plugins = plugins;
	self->prefs   = prefs;
	self->offline = gis_prefs_get_boolean(prefs, "gis/offline", NULL);
}

void gis_viewer_set_time(GisViewer *self, const char *time)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: set_time - time=%s", time);
	g_free(self->time);
	self->time = g_strdup(time);
	_gis_viewer_emit_time_changed(self);
}

gchar *gis_viewer_get_time(GisViewer *self)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: get_time");
	return self->time;
}

void gis_viewer_set_location(GisViewer *self, gdouble lat, gdouble lon, gdouble elev)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: set_location");
	self->location[0] = lat;
	self->location[1] = lon;
	self->location[2] = elev;
	_gis_viewer_fix_location(self);
	_gis_viewer_emit_location_changed(self);
}

void gis_viewer_get_location(GisViewer *self, gdouble *lat, gdouble *lon, gdouble *elev)
{
	g_assert(GIS_IS_VIEWER(self));
	//g_debug("GisViewer: get_location");
	*lat  = self->location[0];
	*lon  = self->location[1];
	*elev = self->location[2];
}

void gis_viewer_pan(GisViewer *self, gdouble forward, gdouble sideways, gdouble up)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: pan - forward=%8.3f, sideways=%8.3f, up=%8.3f",
			forward, sideways, up);
	gdouble dist   = sqrt(forward*forward + sideways*sideways);
	gdouble angle1 = deg2rad(self->rotation[2]);
	gdouble angle2 = atan2(sideways, forward);
	gdouble angle  = angle1 + angle2;
	g_message("pan: dist=%f, angle=%f+%f=%f move=%f,%f",
			dist, angle1, angle2, angle,
			dist*cos(angle),
			dist*sin(angle));
	/* This isn't accurate, but it's usable */
	self->location[0] += dist*cos(angle);
	self->location[1] += dist*sin(angle);
	self->location[2] += up;
	_gis_viewer_fix_location(self);
	_gis_viewer_emit_location_changed(self);
}

void gis_viewer_zoom(GisViewer *self, gdouble scale)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: zoom");
	self->location[2] *= scale;
	_gis_viewer_emit_location_changed(self);
}

void gis_viewer_set_rotation(GisViewer *self, gdouble x, gdouble y, gdouble z)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: set_rotation");
	self->rotation[0] = x;
	self->rotation[1] = y;
	self->rotation[2] = z;
	_gis_viewer_emit_rotation_changed(self);
}

void gis_viewer_get_rotation(GisViewer *self, gdouble *x, gdouble *y, gdouble *z)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: get_rotation");
	*x = self->rotation[0];
	*y = self->rotation[1];
	*z = self->rotation[2];
}

void gis_viewer_rotate(GisViewer *self, gdouble x, gdouble y, gdouble z)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: rotate - x=%.0f, y=%.0f, z=%.0f", x, y, z);
	self->rotation[0] += x;
	self->rotation[1] += y;
	self->rotation[2] += z;
	_gis_viewer_emit_rotation_changed(self);
}

void gis_viewer_refresh(GisViewer *self)
{
	g_debug("GisViewer: refresh");
	_gis_viewer_emit_refresh(self);
}

void gis_viewer_set_offline(GisViewer *self, gboolean offline)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: set_offline - %d", offline);
	gis_prefs_set_boolean(self->prefs, "gis/offline", offline);
	self->offline = offline;
	_gis_viewer_emit_offline(self);
}

gboolean gis_viewer_get_offline(GisViewer *self)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: get_offline - %d", self->offline);
	return self->offline;
}

/* To be implemented by subclasses */
void gis_viewer_center_position(GisViewer *self,
		gdouble lat, gdouble lon, gdouble elev)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(self);
	if (!klass->center_position)
		g_warning("GisViewer: center_position - Unimplemented");
	klass->center_position(self, lat, lon, elev);
}

void gis_viewer_project(GisViewer *self,
		gdouble lat, gdouble lon, gdouble elev,
		gdouble *px, gdouble *py, gdouble *pz)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(self);
	if (!klass->project)
		g_warning("GisViewer: project - Unimplemented");
	klass->project(self, lat, lon, elev, px, py, pz);
}

void gis_viewer_clear_height_func(GisViewer *self)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(self);
	if (!klass->clear_height_func)
		g_warning("GisViewer: clear_height_func - Unimplemented");
	klass->clear_height_func(self);
}

void gis_viewer_set_height_func(GisViewer *self, GisTile *tile,
		GisHeightFunc height_func, gpointer user_data,
		gboolean update)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(self);
	if (!klass->set_height_func)
		g_warning("GisViewer: set_height_func - Unimplemented");
	klass->set_height_func(self, tile, height_func, user_data, update);
}

gpointer gis_viewer_add(GisViewer *self, GisObject *object,
		gint level, gboolean sort)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(self);
	if (!klass->add)
		g_warning("GisViewer: add - Unimplemented");
	return klass->add(self, object, level, sort);
}

GisObject *gis_viewer_remove(GisViewer *self, gpointer ref)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(self);
	if (!klass->remove)
		g_warning("GisViewer: remove - Unimplemented");
	return klass->remove(self, ref);
}

/****************
 * GObject code *
 ****************/
G_DEFINE_ABSTRACT_TYPE(GisViewer, gis_viewer, GTK_TYPE_DRAWING_AREA);
static void gis_viewer_init(GisViewer *self)
{
	g_debug("GisViewer: init");
	/* Default values */
	self->time = g_strdup("");
	self->location[0] = 40;
	self->location[1] = -100;
	self->location[2] = 1.5*EARTH_R;
	self->rotation[0] = 0;
	self->rotation[1] = 0;
	self->rotation[2] = 0;

	g_object_set(self, "can-focus", TRUE, NULL);
	gtk_widget_add_events(GTK_WIDGET(self),
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK |
			GDK_KEY_PRESS_MASK);

	g_signal_connect(self, "key-press-event",      G_CALLBACK(on_key_press),      NULL);

	g_signal_connect(self, "button-press-event",   G_CALLBACK(on_button_press),   NULL);
	g_signal_connect(self, "button-release-event", G_CALLBACK(on_button_release), NULL);
	g_signal_connect(self, "motion-notify-event",  G_CALLBACK(on_motion_notify),  NULL);

	g_signal_connect(self, "location-changed",     G_CALLBACK(on_view_changed),   NULL);
	g_signal_connect(self, "rotation-changed",     G_CALLBACK(on_view_changed),   NULL);
}
static void gis_viewer_finalize(GObject *gobject)
{
	g_debug("GisViewer: finalize");
	GisViewer *self = GIS_VIEWER(gobject);
	g_free(self->time);
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
