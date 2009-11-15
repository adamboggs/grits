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

#ifndef __GIS_OBJECT_H__
#define __GIS_OBJECT_H__

#include <glib.h>

/* Base types */
typedef struct _GisProjection GisProjection;
typedef struct _GisPoint      GisPoint;

struct _GisProjection {
	gdouble model[16];
	gdouble proj[16];
	gint    view[4];
};
struct _GisPoint {
	union {
		gdouble lle[3];
		struct { gdouble lat, lon, elev; };
	};
	union {
		gdouble xyz[3];
		struct { gdouble x, y, z; };
	};
	union {
		gdouble proj[3];
		struct { gdouble px, py, pz; };
	};
	union {
		gdouble norm[3];
		struct { gdouble nx, ny, nz; };
	};
	union {
		gdouble coords[2];
		struct { gdouble cx, cy, xz; };
	};
	gint refs;
};

/* Objects */
#define GIS_OBJECT(object)     ((GisObject  *)object)
#define GIS_TRIANGLE(triangle) ((GisTriangle*)triangle)
#define GIS_QUAD(quad)         ((GisQuad    *)quad)
#define GIS_CALLBACK(callback) ((GisCallback*)callback)
#define GIS_MARKER(marker)     ((GisMarker  *)marker)

typedef enum {
	GIS_TYPE_TRIANGLE,
	GIS_TYPE_QUAD,
	GIS_TYPE_CALLBACK,
	GIS_TYPE_MARKER,
	GIS_NUM_TYPES,
} GisObjectType;

typedef struct _GisObject   GisObject;
typedef struct _GisTriangle GisTriangle;
typedef struct _GisQuad     GisQuad;
typedef struct _GisCallback GisCallback;
typedef struct _GisMarker   GisMarker;

typedef gpointer (*GisCallbackFunc)(GisCallback *callback, gpointer user_data);

struct _GisObject {
	GisObjectType  type;
	GisPoint       center;
	GisProjection *proj;
	gdouble        lod;
};
struct _GisTriangle {
	GisObject  parent;
	GisPoint  *verts[3];
	guint      tex;
};
struct _GisQuad {
	GisObject  parent;
	GisPoint  *verts[4];
	guint     tex;
};
struct _GisCallback {
	GisObject       parent;
	GisCallbackFunc callback;
	gpointer        user_data;
};
struct _GisMarker   {
	GisObject  parent;
	gchar     *label;
};

/* Support functions */
#define gis_object_center(object) \
	(&GIS_OBJECT(object)->center)

GisPoint *gis_point_new();
void gis_point_set_lle(GisPoint *point, gdouble lat, gdouble lon, gdouble elev);
void gis_point_set_xyz(GisPoint *point, gdouble x, gdouble y, gdouble z);
void gis_point_set_coords(GisPoint *point, gdouble x, gdouble y);
void gis_point_project(GisPoint *point, GisProjection *proj);
GisPoint *gis_point_ref(GisPoint *point);
void gis_point_unref(GisPoint *point);

GisTriangle *gis_triangle_new(GisPoint *a, GisPoint *b, GisPoint *c, guint tex);
void gis_triangle_free(GisTriangle *tri);

GisQuad *gis_quad_new(GisPoint *a, GisPoint *b, GisPoint *c, GisPoint *d, guint tex);
void gis_quad_free(GisQuad *quad);

GisCallback *gis_callback_new(GisCallbackFunc callback, gpointer user_data);
void gis_callback_free(GisCallback *cb);

GisMarker *gis_marker_new(const gchar *label);
void gis_marker_free(GisMarker *marker);

#endif
