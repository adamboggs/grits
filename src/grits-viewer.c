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
 * SECTION:grits-viewer
 * @short_description: Virtual globe base class
 *
 * #GritsViewer is the base class for the virtual globe widget. It handles
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
static void _grits_viewer_fix_location(GritsViewer *viewer)
{
	while (viewer->location[0] <  -90) viewer->location[0] += 180;
	while (viewer->location[0] >   90) viewer->location[0] -= 180;
	while (viewer->location[1] < -180) viewer->location[1] += 360;
	while (viewer->location[1] >  180) viewer->location[1] -= 360;
	viewer->location[2] = ABS(viewer->location[2]);
}

/* Signal helpers */
static void _grits_viewer_emit_location_changed(GritsViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_LOCATION_CHANGED], 0,
			viewer->location[0],
			viewer->location[1],
			viewer->location[2]);
}
static void _grits_viewer_emit_rotation_changed(GritsViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_ROTATION_CHANGED], 0,
			viewer->rotation[0],
			viewer->rotation[1],
			viewer->rotation[2]);
}
static void _grits_viewer_emit_time_changed(GritsViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_TIME_CHANGED], 0,
			viewer->time);
}
static void _grits_viewer_emit_refresh(GritsViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_REFRESH], 0);
}
static void _grits_viewer_emit_offline(GritsViewer *viewer)
{
	g_signal_emit(viewer, signals[SIG_OFFLINE], 0,
			viewer->offline);
}

/*************
 * Callbacks *
 *************/
static gboolean on_key_press(GritsViewer *viewer, GdkEventKey *event, gpointer _)
{
	g_debug("GritsViewer: on_key_press - key=%x, state=%x, plus=%x",
			event->keyval, event->state, GDK_plus);

	double lat, lon, elev, pan;
	grits_viewer_get_location(viewer, &lat, &lon, &elev);
	pan = MIN(elev/(EARTH_R/2), 30);
	switch (event->keyval) {
	case GDK_Left:  case GDK_h: grits_viewer_pan(viewer,  0,  -pan, 0); break;
	case GDK_Down:  case GDK_j: grits_viewer_pan(viewer, -pan, 0,   0); break;
	case GDK_Up:    case GDK_k: grits_viewer_pan(viewer,  pan, 0,   0); break;
	case GDK_Right: case GDK_l: grits_viewer_pan(viewer,  0,   pan, 0); break;
	case GDK_minus: case GDK_o: grits_viewer_zoom(viewer, 10./9); break;
	case GDK_plus:  case GDK_i: grits_viewer_zoom(viewer, 9./10); break;
	case GDK_H: grits_viewer_rotate(viewer,  0, 0, -2); break;
	case GDK_J: grits_viewer_rotate(viewer,  2, 0,  0); break;
	case GDK_K: grits_viewer_rotate(viewer, -2, 0,  0); break;
	case GDK_L: grits_viewer_rotate(viewer,  0, 0,  2); break;
	}
	return FALSE;
}

static gboolean on_scroll(GritsViewer *viewer, GdkEventScroll *event, gpointer _)
{
	switch (event->direction) {
	case GDK_SCROLL_DOWN: grits_viewer_zoom(viewer, 10./9); break;
	case GDK_SCROLL_UP:   grits_viewer_zoom(viewer, 9./10); break;
	default: break;
	}
	return FALSE;
}

enum {
	GRITS_DRAG_NONE,
	GRITS_DRAG_PAN,
	GRITS_DRAG_ZOOM,
	GRITS_DRAG_TILT,
};

static gboolean on_button_press(GritsViewer *viewer, GdkEventButton *event, gpointer _)
{
	g_debug("GritsViewer: on_button_press - %d", event->button);
	gtk_widget_grab_focus(GTK_WIDGET(viewer));
	switch (event->button) {
	case 1:  viewer->drag_mode = GRITS_DRAG_PAN;  break;
	case 2:  viewer->drag_mode = GRITS_DRAG_ZOOM; break;
	case 3:  viewer->drag_mode = GRITS_DRAG_TILT; break;
	default: viewer->drag_mode = GRITS_DRAG_NONE; break;
	}
	viewer->drag_x = event->x;
	viewer->drag_y = event->y;
	return FALSE;
}

static gboolean on_button_release(GritsViewer *viewer, GdkEventButton *event, gpointer _)
{
	g_debug("GritsViewer: on_button_release");
	viewer->drag_mode = GRITS_DRAG_NONE;
	return FALSE;
}

static gboolean on_motion_notify(GritsViewer *viewer, GdkEventMotion *event, gpointer _)
{
	gdouble x = viewer->drag_x - event->x;
	gdouble y = viewer->drag_y - event->y;
	gdouble lat, lon, elev, scale, rx, ry, rz;
	grits_viewer_get_location(GRITS_VIEWER(viewer), &lat, &lon, &elev);
	grits_viewer_get_rotation(GRITS_VIEWER(viewer), &rx,  &ry,  &rz);
	scale = (elev/EARTH_R/15) * (sin(deg2rad(ABS(rx)))*4+1);
	switch (viewer->drag_mode) {
	case GRITS_DRAG_PAN:  grits_viewer_pan(viewer, -y*scale, x*scale, 0); break;
	case GRITS_DRAG_ZOOM: grits_viewer_zoom(viewer, pow(2, -y/500)); break;
	case GRITS_DRAG_TILT: grits_viewer_rotate(viewer, y/10, 0, x/10); break;
	}
	viewer->drag_x = event->x;
	viewer->drag_y = event->y;
	return FALSE;
}

static void on_view_changed(GritsViewer *viewer,
		gdouble _1, gdouble _2, gdouble _3)
{
	gtk_widget_queue_draw(GTK_WIDGET(viewer));
}

static void on_realize(GritsViewer *viewer)
{
	GdkCursor *cursor = gdk_cursor_new(GDK_FLEUR);
	GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(viewer));
	gdk_window_set_cursor(window, cursor);
}

/***********
 * Methods *
 ***********/
/**
 * grits_viewer_setup:
 * @viewer:  the viewer
 * @plugins: a plugins store
 * @prefs:   a prefs store
 *
 * This should be called by objects which implement GritsViewer somewhere in their
 * constructor.
 */
void grits_viewer_setup(GritsViewer *viewer, GritsPlugins *plugins, GritsPrefs *prefs)
{
	viewer->plugins = plugins;
	viewer->prefs   = prefs;
	viewer->offline = grits_prefs_get_boolean(prefs, "grits/offline", NULL);
}

/**
 * grits_viewer_set_time:
 * @viewer: the viewer
 * @time: the time to set the view to
 *
 * Set the current time for the view
 */
void grits_viewer_set_time(GritsViewer *viewer, time_t time)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: set_time - time=%ld", time);
	viewer->time = time;
	_grits_viewer_emit_time_changed(viewer);
}

/**
 * grits_viewer_get_time:
 * @viewer: the viewer
 * 
 * Get the time that is being viewed
 *
 * Returns: the current time
 */
time_t grits_viewer_get_time(GritsViewer *viewer)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: get_time");
	return viewer->time;
}

/**
 * grits_viewer_set_location:
 * @viewer: the viewer
 * @lat:  the new latitude
 * @lon:  the new longitude
 * @elev: the new elevation
 *
 * Set the location for the camera
 */
void grits_viewer_set_location(GritsViewer *viewer, gdouble lat, gdouble lon, gdouble elev)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: set_location");
	viewer->location[0] = lat;
	viewer->location[1] = lon;
	viewer->location[2] = elev;
	_grits_viewer_fix_location(viewer);
	_grits_viewer_emit_location_changed(viewer);
}

/**
 * grits_viewer_get_location:
 * @viewer: the viewer
 * @lat:  the location to store the latitude
 * @lon:  the location to store the longitude
 * @elev: the location to store the elevation
 *
 * Get the location of the camera
 */
void grits_viewer_get_location(GritsViewer *viewer, gdouble *lat, gdouble *lon, gdouble *elev)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	//g_debug("GritsViewer: get_location");
	*lat  = viewer->location[0];
	*lon  = viewer->location[1];
	*elev = viewer->location[2];
}

/**
 * grits_viewer_pan:
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
void grits_viewer_pan(GritsViewer *viewer, gdouble forward, gdouble sideways, gdouble up)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: pan - forward=%8.3f, sideways=%8.3f, up=%8.3f",
			forward, sideways, up);
	gdouble dist   = sqrt(forward*forward + sideways*sideways);
	gdouble angle1 = deg2rad(viewer->rotation[2]);
	gdouble angle2 = atan2(sideways, forward);
	gdouble angle  = angle1 + angle2;
	/* This isn't accurate, but it's usable */
	viewer->location[0] += dist*cos(angle);
	viewer->location[1] += dist*sin(angle);
	viewer->location[2] += up;
	_grits_viewer_fix_location(viewer);
	_grits_viewer_emit_location_changed(viewer);
}

/**
 * grits_viewer_zoom:
 * @viewer: the viewer
 * @scale: the scale to multiple the elevation by
 *
 * Multiple the elevation by a scale.
 */
void grits_viewer_zoom(GritsViewer *viewer, gdouble scale)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: zoom");
	viewer->location[2] *= scale;
	_grits_viewer_emit_location_changed(viewer);
}

/**
 * grits_viewer_set_rotation:
 * @viewer: the viewer
 * @x: rotation new around the x axes
 * @y: rotation new around the y axes
 * @z: rotation new around the z axes
 *
 * Set the rotations in degrees around the x, y, and z axes.
 */
void grits_viewer_set_rotation(GritsViewer *viewer, gdouble x, gdouble y, gdouble z)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: set_rotation");
	viewer->rotation[0] = x;
	viewer->rotation[1] = y;
	viewer->rotation[2] = z;
	_grits_viewer_emit_rotation_changed(viewer);
}

/**
 * grits_viewer_get_rotation:
 * @viewer: the viewer
 * @x: rotation around the x axes
 * @y: rotation around the y axes
 * @z: rotation around the z axes
 *
 * Get the rotations in degrees around the x, y, and z axes.
 */
void grits_viewer_get_rotation(GritsViewer *viewer, gdouble *x, gdouble *y, gdouble *z)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: get_rotation");
	*x = viewer->rotation[0];
	*y = viewer->rotation[1];
	*z = viewer->rotation[2];
}

/**
 * grits_viewer_rotate:
 * @viewer: the viewer
 * @x: rotation around the x axes
 * @y: rotation around the y axes
 * @z: rotation around the z axes
 *
 * Add to the rotation around the x, y, and z axes.
 */
void grits_viewer_rotate(GritsViewer *viewer, gdouble x, gdouble y, gdouble z)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: rotate - x=%.0f, y=%.0f, z=%.0f", x, y, z);
	viewer->rotation[0] += x;
	viewer->rotation[1] += y;
	viewer->rotation[2] += z;
	_grits_viewer_emit_rotation_changed(viewer);
}

/**
 * grits_viewer_refresh:
 * @viewer: the viewer
 *
 * Trigger the refresh signal. This will cause any remote data to be checked for
 * updates. 
 */
void grits_viewer_refresh(GritsViewer *viewer)
{
	g_debug("GritsViewer: refresh");
	_grits_viewer_emit_refresh(viewer);
}

/**
 * grits_viewer_set_offline:
 * @viewer: the viewer
 * @offline: %TRUE to enter offline mode
 *
 * Set the offline mode. If @offline is %TRUE, only locally cached data will be
 * used.
 */
void grits_viewer_set_offline(GritsViewer *viewer, gboolean offline)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: set_offline - %d", offline);
	grits_prefs_set_boolean(viewer->prefs, "grits/offline", offline);
	viewer->offline = offline;
	_grits_viewer_emit_offline(viewer);
}

/**
 * grits_viewer_get_offline:
 * @viewer: the viewer
 *
 * Check if the viewer is in offline mode.
 *
 * Returns: %TRUE if the viewer is in offline mode.
 */
gboolean grits_viewer_get_offline(GritsViewer *viewer)
{
	g_assert(GRITS_IS_VIEWER(viewer));
	g_debug("GritsViewer: get_offline - %d", viewer->offline);
	return viewer->offline;
}

/***********************************
 * To be implemented by subclasses *
 ***********************************/
/**
 * grits_viewer_center_position:
 * @viewer: the viewer
 * @lat:  the latitude
 * @lon:  the longitude
 * @elev: the elevation
 *
 * Center the viewer on a point. This can be used before drawing operations to
 * center the items a particular location.
 */
void grits_viewer_center_position(GritsViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev)
{
	GritsViewerClass *klass = GRITS_VIEWER_GET_CLASS(viewer);
	if (!klass->center_position)
		g_warning("GritsViewer: center_position - Unimplemented");
	klass->center_position(viewer, lat, lon, elev);
}

/**
 * grits_viewer_project:
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
 * in space. E.g. #GritsMarker.
 */
void grits_viewer_project(GritsViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev,
		gdouble *px, gdouble *py, gdouble *pz)
{
	GritsViewerClass *klass = GRITS_VIEWER_GET_CLASS(viewer);
	if (!klass->project)
		g_warning("GritsViewer: project - Unimplemented");
	klass->project(viewer, lat, lon, elev, px, py, pz);
}

/**
 * grits_viewer_clear_height_func:
 * @viewer: the viewer
 *
 * Clears the height function for the entire viewer. Useful when an elevation
 * plugin is unloaded.
 */
void grits_viewer_clear_height_func(GritsViewer *viewer)
{
	GritsViewerClass *klass = GRITS_VIEWER_GET_CLASS(viewer);
	if (!klass->clear_height_func)
		g_warning("GritsViewer: clear_height_func - Unimplemented");
	klass->clear_height_func(viewer);
}

/**
 * grits_viewer_set_height_func:
 * @viewer:      the viewer
 * @bounds:      the area to set the height function for
 * @height_func: the height function 
 * @user_data:   user data to pass to the height function
 * @update:      %TRUE if the heights inside the bounds should be updated.
 *
 * Set the height function to be used for a given part of the surface..
 */
void grits_viewer_set_height_func(GritsViewer *viewer, GritsBounds *bounds,
		GritsHeightFunc height_func, gpointer user_data,
		gboolean update)
{
	GritsViewerClass *klass = GRITS_VIEWER_GET_CLASS(viewer);
	if (!klass->set_height_func)
		g_warning("GritsViewer: set_height_func - Unimplemented");
	klass->set_height_func(viewer, bounds, height_func, user_data, update);
}

/**
 * grits_viewer_add:
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
 * Returns: a handle to be pass to grits_viewer_remove()
 */
gpointer grits_viewer_add(GritsViewer *viewer, GritsObject *object,
		gint level, gboolean sort)
{
	GritsViewerClass *klass = GRITS_VIEWER_GET_CLASS(viewer);
	if (!klass->add)
		g_warning("GritsViewer: add - Unimplemented");
	object->ref    = klass->add(viewer, object, level, sort);
	object->viewer = viewer;
	return object;
}

/**
 * grits_viewer_remove:
 * @viewer: the viewer
 * @ref:    the handle obtained from grits_viewer_add()
 *
 * Remove an object from the viewer. The objects reference count is decremented
 * prior to being removed.
 *
 * Returns: the #GritsObject referenced by the handle
 */
GritsObject *grits_viewer_remove(GritsViewer *viewer, gpointer _object)
{
	GritsObject *object = _object;
	GritsViewerClass *klass = GRITS_VIEWER_GET_CLASS(viewer);
	if (!klass->remove)
		g_warning("GritsViewer: remove - Unimplemented");
	klass->remove(viewer, object->ref);
	object->ref    = NULL;
	object->viewer = NULL;
	return object;
}

/****************
 * GObject code *
 ****************/
G_DEFINE_ABSTRACT_TYPE(GritsViewer, grits_viewer, GTK_TYPE_DRAWING_AREA);
static void grits_viewer_init(GritsViewer *viewer)
{
	g_debug("GritsViewer: init");
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
static void grits_viewer_finalize(GObject *gobject)
{
	g_debug("GritsViewer: finalize");
	G_OBJECT_CLASS(grits_viewer_parent_class)->finalize(gobject);
	g_debug("GritsViewer: finalize - done");
}
static void grits_viewer_class_init(GritsViewerClass *klass)
{
	g_debug("GritsViewer: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->finalize     = grits_viewer_finalize;

	/**
	 * GritsViewer::time-changed:
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
	 * GritsViewer::location-changed:
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
	 * GritsViewer::rotation-changed:
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
	 * GritsViewer::refresh:
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
	 * GritsViewer::offline:
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
