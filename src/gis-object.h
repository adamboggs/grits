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

#ifndef __GIS_OBJECT_H__
#define __GIS_OBJECT_H__

#include <glib.h>
#include <cairo.h>

/* GisPoint */
typedef struct _GisPoint      GisPoint;

struct _GisPoint {
	gdouble lat, lon, elev;
};

GisPoint *gis_point_new();
void gis_point_set_lle(GisPoint *point, gdouble lat, gdouble lon, gdouble elev);
void gis_point_free(GisPoint *point);


/* GisObject */
#define GIS_OBJECT(object)     ((GisObject  *)object)

typedef enum {
	GIS_TYPE_CALLBACK,
	GIS_TYPE_MARKER,
	GIS_NUM_TYPES,
} GisObjectType;

typedef struct _GisObject GisObject;

struct _GisObject {
	GisObjectType type;
	GisPoint      center;
	gdouble       lod;
};

static inline GisPoint *gis_object_center(GisObject *object)
{
	return &GIS_OBJECT(object)->center;
}


/* GisMarker */
#define GIS_MARKER(marker) ((GisMarker  *)marker)

typedef struct _GisMarker GisMarker;

struct _GisMarker   {
	GisObject  parent;
	gint       xoff, yoff;
	gchar     *label;
	cairo_t   *cairo;
	guint      tex;
};

GisMarker *gis_marker_new(const gchar *label);
void gis_marker_free(GisMarker *marker);


/* GisCallback */
#define GIS_CALLBACK(callback) ((GisCallback*)callback)

typedef struct _GisCallback GisCallback;
typedef gpointer (*GisCallbackFunc)(GisCallback *callback, gpointer user_data);

struct _GisCallback {
	GisObject       parent;
	GisCallbackFunc callback;
	gpointer        user_data;
};

GisCallback *gis_callback_new(GisCallbackFunc callback, gpointer user_data);
void gis_callback_free(GisCallback *cb);


#endif
