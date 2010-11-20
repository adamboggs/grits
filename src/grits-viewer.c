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
 * SECTION:gis-viewer
 * @short_description: Virtual globe base class
 *
 * #GisViewer is the base class for the virtual globe widget. It handles
 * everything not directly related to drawing the globe. Plugins and
 * applications using the viewer should normally talk to the viewer and not care
 * how it is implemented. 
 */

#include <config.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "grits-marshal.h"
#include "grits-viewer.h"

#include "grits-util.h"


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
	switch (event->keyval) {
	case GDK_Left:  case GDK_h: gis_viewer_pan(viewer,  0,  -pan, 0); break;
	case GDK_Down:  case GDK_j: gis_viewer_pan(viewer, -pan, 0,   0); break;
	case GDK_Up:    case GDK_k: gis_viewer_pan(viewer,  pan, 0,   0); break;
	case GDK_Right: case GDK_l: gis_viewer_pan(viewer,  0,   pan, 0); break;
	case GDK_minus: case GDK_o: gis_viewer_zoom(viewer, 10./9); break;
	case GDK_plus:  case GDK_i: gis_viewer_zoom(viewer, 9./10); break;
	case GDK_H: gis_viewer_rotate(viewer,  0, 0, -2); break;
	case GDK_J: gis_viewer_rotate(viewer,  2, 0,  0); break;
	case GDK_K: gis_viewer_rotate(viewer, -2, 0,  0); break;
	case GDK_L: gis_viewer_rotate(viewer,  0, 0,  2); break;
	}
	return FALSE;
}

static gboolean on_scroll(GisViewer *viewer, GdkEventScroll *event, gpointer _)
{
	switch (event->direction) {
	case GDK_SCROLL_DOWN: gis_viewer_zoom(viewer, 10./9); break;
	case GDK_SCROLL_UP:   gis_viewer_zoom(viewer, 9./10); break;
	default: break;
	}
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
	default: viewer->drag_mode = GIS_DRAG_NONE; break;
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
	gdouble x = viewer->drag_x - event->x;
	gdouble y = viewer->drag_y - event->y;
	gdouble lat, lon, elev, scale;
	gis_viewer_get_location(GIS_VIEWER(viewer), &lat, &lon, &elev);
	scale = elev/EARTH_R/15;
	switch (viewer->drag_mode) {
	case GIS_DRAG_PAN:  gis_viewer_pan(viewer, -y*scale, x*scale, 0); break;
	case GIS_DRAG_ZOOM: gis_viewer_zoom(viewer, pow(2, -y/500)); break;
	case GIS_DRAG_TILT: gis_viewer_rotate(viewer, y/10, 0, x/10); break;
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

static void on_realize(GisViewer *viewer)
{
	GdkCursor *cursor = gdk_cursor_new(GDK_FLEUR);
	GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(viewer));
	gdk_window_set_cursor(window, cursor);
}

/***********
 * Methods *
 ***********/
/**
 * gis_viewer_setup:
 * @viewer:  the viewer
 * @plugins: a plugins store
 * @prefs:   a prefs store
 *
 * This should be called by objects which implement GisViewer somewhere in their
 * constructor.
 */
void gis_viewer_setup(GisViewer *viewer, GisPlugins *plugins, GisPrefs *prefs)
{
	viewer->plugins = plugins;
	viewer->prefs   = prefs;
	viewer->offline = gis_prefs_get_boolean(prefs, "gis/offline", NULL);
}

/**
 * gis_viewer_set_time:
 * @viewer: the viewer
 * @time: the time to set the view to
 *
 * Set the current time for the view
 */
void gis_viewer_set_time(GisViewer *viewer, time_t time)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: set_time - time=%ld", time);
	viewer->time = time;
	_gis_viewer_emit_time_changed(viewer);
}

/**
 * gis_viewer_get_time:
 * @viewer: the viewer
 * 
 * Get the time that is being viewed
 *
 * Returns: the current time
 */
time_t gis_viewer_get_time(GisViewer *viewer)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: get_time");
	return viewer->time;
}

/**
 * gis_viewer_set_location:
 * @viewer: the viewer
 * @lat:  the new latitude
 * @lon:  the new longitude
 * @elev: the new elevation
 *
 * Set the location for the camera
 */
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

/**
 * gis_viewer_get_location:
 * @viewer: the viewer
 * @lat:  the location to store the latitude
 * @lon:  the location to store the longitude
 * @elev: the location to store the elevation
 *
 * Get the location of the camera
 */
void gis_viewer_get_location(GisViewer *viewer, gdouble *lat, gdouble *lon, gdouble *elev)
{
	g_assert(GIS_IS_VIEWER(viewer));
	//g_debug("GisViewer: get_location");
	*lat  = viewer->location[0];
	*lon  = viewer->location[1];
	*elev = viewer->location[2];
}

/**
 * gis_viewer_pan:
 * @viewer: the viewer
 * @forward:  distance to move forward in meters
 * @right:    distance to move right in meters
 * @up:       distance to move up in meters
 *
 * Pan the location by a number of meters long the surface.
 *
 * Bugs: the distances are not in meters
 * Bugs: panning does not move in strait lines
 */
void gis_viewer_pan(GisViewer *viewer, gdouble forward, gdouble sideways, gdouble up)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: pan - forward=%8.3f, sideways=%8.3f, up=%8.3f",
			forward, sideways, up);
	gdouble dist   = sqrt(forward*forward + sideways*sideways);
	gdouble angle1 = deg2rad(viewer->rotation[2]);
	gdouble angle2 = atan2(sideways, forward);
	gdouble angle  = angle1 + angle2;
	/* This isn't accurate, but it's usable */
	viewer->location[0] += dist*cos(angle);
	viewer->location[1] += dist*sin(angle);
	viewer->location[2] += up;
	_gis_viewer_fix_location(viewer);
	_gis_viewer_emit_location_changed(viewer);
}

/**
 * gis_viewer_zoom:
 * @viewer: the viewer
 * @scale: the scale to multiple the elevation by
 *
 * Multiple the elevation by a scale.
 */
void gis_viewer_zoom(GisViewer *viewer, gdouble scale)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: zoom");
	viewer->location[2] *= scale;
	_gis_viewer_emit_location_changed(viewer);
}

/**
 * gis_viewer_set_rotation:
 * @viewer: the viewer
 * @x: rotation new around the x axes
 * @y: rotation new around the y axes
 * @z: rotation new around the z axes
 *
 * Set the rotations in degrees around the x, y, and z axes.
 */
void gis_viewer_set_rotation(GisViewer *viewer, gdouble x, gdouble y, gdouble z)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: set_rotation");
	viewer->rotation[0] = x;
	viewer->rotation[1] = y;
	viewer->rotation[2] = z;
	_gis_viewer_emit_rotation_changed(viewer);
}

/**
 * gis_viewer_get_rotation:
 * @viewer: the viewer
 * @x: rotation around the x axes
 * @y: rotation around the y axes
 * @z: rotation around the z axes
 *
 * Get the rotations in degrees around the x, y, and z axes.
 */
void gis_viewer_get_rotation(GisViewer *viewer, gdouble *x, gdouble *y, gdouble *z)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: get_rotation");
	*x = viewer->rotation[0];
	*y = viewer->rotation[1];
	*z = viewer->rotation[2];
}

/**
 * gis_viewer_rotate:
 * @viewer: the viewer
 * @x: rotation around the x axes
 * @y: rotation around the y axes
 * @z: rotation around the z axes
 *
 * Add to the rotation around the x, y, and z axes.
 */
void gis_viewer_rotate(GisViewer *viewer, gdouble x, gdouble y, gdouble z)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: rotate - x=%.0f, y=%.0f, z=%.0f", x, y, z);
	viewer->rotation[0] += x;
	viewer->rotation[1] += y;
	viewer->rotation[2] += z;
	_gis_viewer_emit_rotation_changed(viewer);
}

/**
 * gis_viewer_refresh:
 * @viewer: the viewer
 *
 * Trigger the refresh signal. This will cause any remote data to be checked for
 * updates. 
 */
void gis_viewer_refresh(GisViewer *viewer)
{
	g_debug("GisViewer: refresh");
	_gis_viewer_emit_refresh(viewer);
}

/**
 * gis_viewer_set_offline:
 * @viewer: the viewer
 * @offline: %TRUE to enter offline mode
 *
 * Set the offline mode. If @offline is %TRUE, only locally cached data will be
 * used.
 */
void gis_viewer_set_offline(GisViewer *viewer, gboolean offline)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: set_offline - %d", offline);
	gis_prefs_set_boolean(viewer->prefs, "gis/offline", offline);
	viewer->offline = offline;
	_gis_viewer_emit_offline(viewer);
}

/**
 * gis_viewer_get_offline:
 * @viewer: the viewer
 *
 * Check if the viewer is in offline mode.
 *
 * Returns: %TRUE if the viewer is in offline mode.
 */
gboolean gis_viewer_get_offline(GisViewer *viewer)
{
	g_assert(GIS_IS_VIEWER(viewer));
	g_debug("GisViewer: get_offline - %d", viewer->offline);
	return viewer->offline;
}

/***********************************
 * To be implemented by subclasses *
 ***********************************/
/**
 * gis_viewer_center_position:
 * @viewer: the viewer
 * @lat:  the latitude
 * @lon:  the longitude
 * @elev: the elevation
 *
 * Center the viewer on a point. This can be used before drawing operations to
 * center the items a particular location.
 */
void gis_viewer_center_position(GisViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->center_position)
		g_warning("GisViewer: center_position - Unimplemented");
	klass->center_position(viewer, lat, lon, elev);
}

/**
 * gis_viewer_project:
 * @viewer: the viewer
 * @lat:  the latitude
 * @lon:  the latitude
 * @elev: the latitude
 * @px:   the project x coordinate
 * @py:   the project y coordinate
 * @pz:   the project z coordinate
 *
 * Project a latitude, longitude, elevation point to to x, y, and z coordinates
 * in screen space. Useful for drawing orthographic data over a particular point
 * in space. E.g. #GisMarker.
 */
void gis_viewer_project(GisViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev,
		gdouble *px, gdouble *py, gdouble *pz)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->project)
		g_warning("GisViewer: project - Unimplemented");
	klass->project(viewer, lat, lon, elev, px, py, pz);
}

/**
 * gis_viewer_clear_height_func:
 * @viewer: the viewer
 *
 * Clears the height function for the entire viewer. Useful when an elevation
 * plugin is unloaded.
 */
void gis_viewer_clear_height_func(GisViewer *viewer)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->clear_height_func)
		g_warning("GisViewer: clear_height_func - Unimplemented");
	klass->clear_height_func(viewer);
}

/**
 * gis_viewer_set_height_func:
 * @viewer:      the viewer
 * @bounds:      the area to set the height function for
 * @height_func: the height function 
 * @user_data:   user data to pass to the height function
 * @update:      %TRUE if the heights inside the bounds should be updated.
 *
 * Set the height function to be used for a given part of the surface..
 */
void gis_viewer_set_height_func(GisViewer *viewer, GisBounds *bounds,
		GisHeightFunc height_func, gpointer user_data,
		gboolean update)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->set_height_func)
		g_warning("GisViewer: set_height_func - Unimplemented");
	klass->set_height_func(viewer, bounds, height_func, user_data, update);
}

/**
 * gis_viewer_add:
 * @viewer: the viewer
 * @object: the object to add
 * @level:  the level to add the object to
 * @sort:   %TRUE if the object should be depth-sorted prior to being drawn
 *
 * Objects which are added to the viewer will be drawn on subsequent renderings
 * if their level of details is adequate.
 *
 * The @level represents the order the object should be drawn in, this is
 * unrelated to the objects actual position in the world.
 *
 * Semi-transparent objects should set @sort to %TRUE so that they are rendered
 * correctly when they overlap other semi-transparent objects.
 *
 * The viewer steals the objects reference. Call g_object_ref if you plan on
 * holding a reference as well.
 *
 * Returns: a handle to be pass to gis_viewer_remove()
 */
gpointer gis_viewer_add(GisViewer *viewer, GisObject *object,
		gint level, gboolean sort)
{
	GisViewerClass *klass = GIS_VIEWER_GET_CLASS(viewer);
	if (!klass->add)
		g_warning("GisViewer: add - Unimplemented");
	return klass->add(viewer, object, level, sort);
}

/**
 * gis_viewer_remove:
 * @viewer: the viewer
 * @ref:    the handle obtained from gis_viewer_add()
 *
 * Remove an object from the viewer. The objects reference count is decremented
 * prior to being removed.
 *
 * Returns: the #GisObject referenced by the handle
 */
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
	viewer->time        = time(NULL);
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
	g_signal_connect(viewer, "scroll-event",         G_CALLBACK(on_scroll),         NULL);

	g_signal_connect(viewer, "button-press-event",   G_CALLBACK(on_button_press),   NULL);
	g_signal_connect(viewer, "button-release-event", G_CALLBACK(on_button_release), NULL);
	g_signal_connect(viewer, "motion-notify-event",  G_CALLBACK(on_motion_notify),  NULL);

	g_signal_connect(viewer, "location-changed",     G_CALLBACK(on_view_changed),   NULL);
	g_signal_connect(viewer, "rotation-changed",     G_CALLBACK(on_view_changed),   NULL);

	g_signal_connect(viewer, "realize",              G_CALLBACK(on_realize),        NULL);
}
static void gis_viewer_finalize(GObject *gobject)
{
	g_debug("GisViewer: finalize");
	G_OBJECT_CLASS(gis_viewer_parent_class)->finalize(gobject);
}
static void gis_viewer_class_init(GisViewerClass *klass)
{
	g_debug("GisViewer: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->finalize     = gis_viewer_finalize;

	/**
	 * GisViewer::time-changed:
	 * @viewer: the viewer.
	 * @time:   the new time.
	 *
	 * The ::time-changed signal is emitted when the viewers current time
	 * changers.
	 */
	signals[SIG_TIME_CHANGED] = g_signal_new(
			"time-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__LONG,
			G_TYPE_NONE,
			1,
			G_TYPE_LONG);

	/**
	 * GisViewer::location-changed:
	 * @viewer: the viewer.
	 * @lat:    the new latitude.
	 * @lon:    the new longitude.
	 * @elev:   the new elevation.
	 *
	 * The ::location-changed signal is emitted when the viewers camera
	 * location changes.
	 */
	signals[SIG_LOCATION_CHANGED] = g_signal_new(
			"location-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			grits_cclosure_marshal_VOID__DOUBLE_DOUBLE_DOUBLE,
			G_TYPE_NONE,
			3,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE);

	/**
	 * GisViewer::rotation-changed:
	 * @viewer: the viewer.
	 * @x: rotation new around the x axes.
	 * @y: rotation new around the y axes.
	 * @z: rotation new around the z axes.
	 *
	 * The ::rotation-changed signal is emitted when the viewers cameras
	 * rotation changes.
	 */
	signals[SIG_ROTATION_CHANGED] = g_signal_new(
			"rotation-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			grits_cclosure_marshal_VOID__DOUBLE_DOUBLE_DOUBLE,
			G_TYPE_NONE,
			3,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE);

	/**
	 * GisViewer::refresh:
	 * @viewer: the viewer.
	 *
	 * The ::refresh signal is emitted when a refresh is needed. If you are
	 * using real-time data from a remote server, you should connect to the
	 * refresh signal and update the data when necessary.
	 */
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

	/**
	 * GisViewer::offline:
	 * @viewer:  the viewer.
	 * @offline: %TRUE if the viewer going offline.
	 *
	 * The ::offline signal is emitted when the viewers offline mode
	 * changes.
	 */
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
