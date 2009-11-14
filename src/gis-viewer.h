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

#ifndef __GIS_VIEWER_H__
#define __GIS_VIEWER_H__

#include <glib-object.h>

/* Type macros */
#define GIS_TYPE_VIEWER            (gis_viewer_get_type())
#define GIS_VIEWER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_VIEWER, GisViewer))
#define GIS_IS_VIEWER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_VIEWER))
#define GIS_VIEWER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_VIEWER, GisViewerClass))
#define GIS_IS_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_VIEWER))
#define GIS_VIEWER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_VIEWER, GisViewerClass))

typedef struct _GisViewer      GisViewer;
typedef struct _GisViewerClass GisViewerClass;

struct _GisViewer {
	GObject parent_instance;

	/* instance members */
	gchar   *time;
	gchar   *site;
	gdouble  location[3];
	gdouble  rotation[3];
	gboolean offline;
};

struct _GisViewerClass {
	GObjectClass parent_class;

	/* class members */
};

GType gis_viewer_get_type(void);

/* Methods */
GisViewer *gis_viewer_new();

void gis_viewer_set_time(GisViewer *viewer, const gchar *time);
gchar *gis_viewer_get_time(GisViewer *viewer);

void gis_viewer_set_location(GisViewer *viewer, gdouble  lat, gdouble  lon, gdouble  elev);
void gis_viewer_get_location(GisViewer *viewer, gdouble *lat, gdouble *lon, gdouble *elev);
void gis_viewer_pan         (GisViewer *viewer, gdouble  lat, gdouble  lon, gdouble  elev);
void gis_viewer_zoom        (GisViewer *viewer, gdouble  scale);

void gis_viewer_set_rotation(GisViewer *viewer, gdouble  x, gdouble  y, gdouble  z);
void gis_viewer_get_rotation(GisViewer *viewer, gdouble *x, gdouble *y, gdouble *z);
void gis_viewer_rotate      (GisViewer *viewer, gdouble  x, gdouble  y, gdouble  z);

/* To be deprecated, use {get,set}_location */
void gis_viewer_set_site(GisViewer *viewer, const gchar *site);
gchar *gis_viewer_get_site(GisViewer *viewer);

void gis_viewer_refresh(GisViewer *viewer);

void gis_viewer_set_offline(GisViewer *viewer, gboolean offline);
gboolean gis_viewer_get_offline(GisViewer *viewer);

#endif
