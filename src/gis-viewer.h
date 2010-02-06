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

#ifndef __GIS_VIEWER_H__
#define __GIS_VIEWER_H__

#include <gtk/gtk.h>
#include <glib-object.h>

/* Rendering levels */
#define GIS_LEVEL_BACKGROUND -100
#define GIS_LEVEL_WORLD         0
#define GIS_LEVEL_OVERLAY     100
#define GIS_LEVEL_HUD         200

/* Type macros */
#define GIS_TYPE_VIEWER            (gis_viewer_get_type())
#define GIS_VIEWER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_VIEWER, GisViewer))
#define GIS_IS_VIEWER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_VIEWER))
#define GIS_VIEWER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_VIEWER, GisViewerClass))
#define GIS_IS_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_VIEWER))
#define GIS_VIEWER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_VIEWER, GisViewerClass))

typedef struct _GisViewer      GisViewer;
typedef struct _GisViewerClass GisViewerClass;

typedef gdouble (*GisHeightFunc)(gdouble lat, gdouble lon, gpointer user_data);

#include "gis-plugin.h"
#include "gis-prefs.h"
#include "objects/gis-object.h"
#include "objects/gis-tile.h"

struct _GisViewer {
	GtkDrawingArea parent_instance;

	/* instance members */
	GisPlugins *plugins;
	GisPrefs   *prefs;
	gchar      *time;
	gdouble     location[3];
	gdouble     rotation[3];
	gboolean    offline;
};

struct _GisViewerClass {
	GtkDrawingAreaClass parent_class;

	/* class members */
	void (*center_position)  (GisViewer *viewer,
	                          gdouble lat, gdouble lon, gdouble elev);

	void (*project)          (GisViewer *viewer,
	                          gdouble lat, gdouble lon, gdouble elev,
	                          gdouble *px, gdouble *py, gdouble *pz);

	void (*clear_height_func)(GisViewer *self);
	void (*set_height_func)  (GisViewer *self, GisTile *tile,
	                          GisHeightFunc height_func, gpointer user_data,
	                          gboolean update);

	gpointer (*add)          (GisViewer *viewer, GisObject *object,
	                          gint level, gboolean sort);
	void (*remove)           (GisViewer *viewer, gpointer ref);
};

GType gis_viewer_get_type(void);

/* Methods */
void gis_viewer_setup(GisViewer *viewer, GisPlugins *plugins, GisPrefs *prefs);

void gis_viewer_set_time(GisViewer *viewer, const gchar *time);
gchar *gis_viewer_get_time(GisViewer *viewer);

void gis_viewer_set_location(GisViewer *viewer, gdouble  lat, gdouble  lon, gdouble  elev);
void gis_viewer_get_location(GisViewer *viewer, gdouble *lat, gdouble *lon, gdouble *elev);
void gis_viewer_pan         (GisViewer *viewer, gdouble  lat, gdouble  lon, gdouble  elev);
void gis_viewer_zoom        (GisViewer *viewer, gdouble  scale);

void gis_viewer_set_rotation(GisViewer *viewer, gdouble  x, gdouble  y, gdouble  z);
void gis_viewer_get_rotation(GisViewer *viewer, gdouble *x, gdouble *y, gdouble *z);
void gis_viewer_rotate      (GisViewer *viewer, gdouble  x, gdouble  y, gdouble  z);

void gis_viewer_refresh(GisViewer *viewer);

void gis_viewer_set_offline(GisViewer *viewer, gboolean offline);
gboolean gis_viewer_get_offline(GisViewer *viewer);

/* To be implemented by subclasses */
void gis_viewer_center_position(GisViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev);

void gis_viewer_project(GisViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev,
		gdouble *px, gdouble *py, gdouble *pz);

void gis_viewer_clear_height_func(GisViewer *self);
void gis_viewer_set_height_func(GisViewer *self, GisTile *tile,
		GisHeightFunc height_func, gpointer user_data,
		gboolean update);

gpointer gis_viewer_add(GisViewer *self, GisObject *object,
		gint level, gboolean sort);
void gis_viewer_remove(GisViewer *self, gpointer ref);

#endif
