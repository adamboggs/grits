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

#ifndef __GIS_MARKER_H__
#define __GIS_MARKER_H__

#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include "gis-object.h"

/* GisMarker */
#define GIS_TYPE_MARKER            (gis_marker_get_type())
#define GIS_MARKER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_MARKER, GisMarker))
#define GIS_IS_MARKER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_MARKER))
#define GIS_MARKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_MARKER, GisMarkerClass))
#define GIS_IS_MARKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_MARKER))
#define GIS_MARKER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_MARKER, GisMarkerClass))

typedef struct _GisMarker      GisMarker;
typedef struct _GisMarkerClass GisMarkerClass;

struct _GisMarker {
	GisObject  parent_instance;
	gint       xoff, yoff;
	gchar     *label;
	cairo_t   *cairo;
	guint      tex;
};

struct _GisMarkerClass {
	GisObjectClass parent_class;
};

GType gis_marker_get_type(void);

GisMarker *gis_marker_new(const gchar *label);

#endif
