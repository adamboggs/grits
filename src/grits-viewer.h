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
 * Hack alert: grits-opengl.h needs to be included before grits-viewer
 *   - GritsViewer depends on GritsObject for add/remove functions
 *   - GritsObject depends on GritsOpenGL for load/unload functions
 *   - GritsOpenGL depends on GritsViewer for inheritance
 *
 * The problem here is that GritsOpenGL needs the GritsViewer definition but
 * GritsViewer only needs the typedefs (through GritsObject), so GritsViewer
 * needs to be included after the GritsOpenGL typedefs but before the
 * GritsOpenGL definition. This is handled internally by grits-opengl.h
 *
 * This should probably be fixed, but making a GritsGLObject interface seems
 * like too much work. Merging GritsViewer and GritsOpenGL would also work, but
 * I like the separate that that's provided by having two.
 */
#include "grits-opengl.h"

#ifndef __GRITS_VIEWER_H__
#define __GRITS_VIEWER_H__

#include <gtk/gtk.h>
#include <glib-object.h>

/* Rendering levels */
/**
 * GRITS_LEVEL_BACKGROUND: 
 *
 * The level used to draw background objects (stars, atmosphere, etc).
 */
#define GRITS_LEVEL_BACKGROUND -100

/**
 * GRITS_LEVEL_WORLD: 
 *
 * The level used to draw world objects. This is for both surface data as well
 * as things in the air or underground. Most objects should use
 * %GRITS_LEVEL_WORLD;
 */
#define GRITS_LEVEL_WORLD         0

/**
 * GRITS_LEVEL_OVERLAY: 
 *
 * The level used to draw screen overlays. These will be drawn in front of most
 * of ther objects. Text and markers should use %GRITS_LEVEL_OVERLAY.
 */
#define GRITS_LEVEL_OVERLAY     100

/**
 * GRITS_LEVEL_HUD: 
 *
 * The level used to draw the Heads Up Display. This is for things that are not
 * anchored at all the the world. They should be drawn in front of everything
 * else.
 */
#define GRITS_LEVEL_HUD         200

/* Type macros */
#define GRITS_TYPE_VIEWER            (grits_viewer_get_type())
#define GRITS_VIEWER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_VIEWER, GritsViewer))
#define GRITS_IS_VIEWER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_VIEWER))
#define GRITS_VIEWER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_VIEWER, GritsViewerClass))
#define GRITS_IS_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_VIEWER))
#define GRITS_VIEWER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_VIEWER, GritsViewerClass))

typedef struct _GritsViewer      GritsViewer;
typedef struct _GritsViewerClass GritsViewerClass;

/**
 * GritsHeightFunc:
 * @lat:       the target latitude
 * @lon:       the target longitude
 * @user_data: user data passed to the function
 *
 * Determine the surface elevation (ground level) at a given point.
 *
 * Returns: the elevation in meters above sea level
 */
typedef gdouble (*GritsHeightFunc)(gdouble lat, gdouble lon, gpointer user_data);

#include "grits-plugin.h"
#include "grits-prefs.h"
#include "objects/grits-object.h"

struct _GritsViewer {
	GtkDrawingArea parent_instance;

	/* instance members */
	GritsPlugins *plugins;
	GritsPrefs   *prefs;
	time_t      time;
	gdouble     location[3];
	gdouble     rotation[3];
	gboolean    offline;

	/* For dragging */
	gint    drag_mode;
	gdouble drag_x, drag_y;
};

struct _GritsViewerClass {
	GtkDrawingAreaClass parent_class;

	/* class members */
	void (*center_position)  (GritsViewer *viewer,
	                          gdouble lat, gdouble lon, gdouble elev);

	void (*project)          (GritsViewer *viewer,
	                          gdouble lat, gdouble lon, gdouble elev,
	                          gdouble *px, gdouble *py, gdouble *pz);

	void (*clear_height_func)(GritsViewer *viewer);
	void (*set_height_func)  (GritsViewer *viewer, GritsBounds *bounds,
	                          GritsHeightFunc height_func, gpointer user_data,
	                          gboolean update);

	gpointer (*add)          (GritsViewer *viewer, GritsObject *object,
	                          gint level, gboolean sort);
	GritsObject *(*remove)   (GritsViewer *viewer, gpointer ref);
};

GType grits_viewer_get_type(void);

/* Methods */
void grits_viewer_setup(GritsViewer *viewer, GritsPlugins *plugins, GritsPrefs *prefs);

void grits_viewer_set_time(GritsViewer *viewer, time_t time);
time_t grits_viewer_get_time(GritsViewer *viewer);

void grits_viewer_set_location(GritsViewer *viewer, gdouble  lat, gdouble  lon, gdouble  elev);
void grits_viewer_get_location(GritsViewer *viewer, gdouble *lat, gdouble *lon, gdouble *elev);
void grits_viewer_pan(GritsViewer *viewer, gdouble forward, gdouble right, gdouble up);
void grits_viewer_zoom(GritsViewer *viewer, gdouble  scale);

void grits_viewer_set_rotation(GritsViewer *viewer, gdouble  x, gdouble  y, gdouble  z);
void grits_viewer_get_rotation(GritsViewer *viewer, gdouble *x, gdouble *y, gdouble *z);
void grits_viewer_rotate      (GritsViewer *viewer, gdouble  x, gdouble  y, gdouble  z);

void grits_viewer_refresh(GritsViewer *viewer);

void grits_viewer_set_offline(GritsViewer *viewer, gboolean offline);
gboolean grits_viewer_get_offline(GritsViewer *viewer);

/* To be implemented by subclasses */
void grits_viewer_center_position(GritsViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev);

void grits_viewer_project(GritsViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev,
		gdouble *px, gdouble *py, gdouble *pz);

void grits_viewer_clear_height_func(GritsViewer *viewer);
void grits_viewer_set_height_func(GritsViewer *viewer, GritsBounds *bounds,
		GritsHeightFunc height_func, gpointer user_data,
		gboolean update);

gpointer grits_viewer_add(GritsViewer *viewer, GritsObject *object,
		gint level, gboolean sort);
GritsObject *grits_viewer_remove(GritsViewer *viewer, gpointer ref);

#endif
