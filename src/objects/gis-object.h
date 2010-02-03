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
#include <glib-object.h>

/* Take that GLib boilerplate! */
#define GOBJECT_HEAD( \
		MAM, BAR, \
		Mam, Bar, \
		mam, bar) \
GType mam##_##bar##_get_type(void); \
typedef struct _##Mam##Bar Mam##Bar; \
typedef struct _##Mam##Bar##Class Mam##Bar##Class; \
static inline Mam##Bar *MAM##_##BAR(gpointer obj) { \
	return G_TYPE_CHECK_INSTANCE_CAST(obj, MAM##_TYPE_##BAR, Mam##Bar); \
} \
static inline gboolean MAM##_IS_##BAR(gpointer obj) { \
	return G_TYPE_CHECK_INSTANCE_TYPE(obj, MAM##_TYPE_##BAR); \
} \
static inline Mam##Bar##Class *MAM##_##BAR##_CLASS(gpointer klass) { \
	return G_TYPE_CHECK_CLASS_CAST(klass, MAM##_TYPE_##BAR, Mam##Bar##Class); \
} \
static inline gboolean MAM##_IS_##BAR##_CLASS(gpointer klass) { \
	return G_TYPE_CHECK_CLASS_TYPE(klass, MAM##_TYPE_##BAR); \
} \
static inline Mam##Bar##Class *MAM##_##BAR##_GET_CLASS(gpointer obj) { \
	return G_TYPE_INSTANCE_GET_CLASS(obj, MAM##_TYPE_##BAR, Mam##Bar##Class); \
}

#define GOBJECT_BODY( \
		parent_type, \
		MAM, BAR, \
		Mam, Bar, \
		mam, bar) \
G_DEFINE_TYPE(Mam##Bar, mam##_##bar, parent_type); \
static void mam##_##bar##_init(Mam##Bar *self) { \
} \
static void mam##_##bar##_class_init(Mam##Bar##Class *klass) { \
} \
static Mam##Bar *mam##_##bar##_new() { \
	return g_object_new(MAM##_TYPE_##BAR, NULL); \
}


/* GisPoint */
typedef struct _GisPoint GisPoint;

struct _GisPoint {
	gdouble lat, lon, elev;
};

GisPoint *gis_point_new();
void gis_point_set_lle(GisPoint *point, gdouble lat, gdouble lon, gdouble elev);
void gis_point_free(GisPoint *point);


/* GisObject */
#define GIS_TYPE_OBJECT (gis_object_get_type())

GOBJECT_HEAD(
	GIS, OBJECT,
	Gis, Object,
	gis, object);

struct _GisObject {
	GObject parent_instance;
	GisPoint center;
	gdouble  lod;
};

struct _GisObjectClass {
	GObjectClass parent_class;
};

static inline GisPoint *gis_object_center(GisObject *object)
{
	return &GIS_OBJECT(object)->center;
}

#endif
