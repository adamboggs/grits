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
#include "gis-util.h"

/* GisObject */
#define GIS_TYPE_OBJECT            (gis_object_get_type())
#define GIS_OBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_OBJECT, GisObject))
#define GIS_IS_OBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_OBJECT))
#define GIS_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_OBJECT, GisObjectClass))
#define GIS_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_OBJECT))
#define GIS_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_OBJECT, GisObjectClass))

typedef struct _GisObject      GisObject;
typedef struct _GisObjectClass GisObjectClass;

struct _GisObject {
	GObject  parent_instance;
	GisPoint center;
	gdouble  lod;
};

struct _GisObjectClass {
	GObjectClass parent_class;
};

GType gis_object_get_type(void);

/**
 * gis_object_center:
 * @object: The #GisObject to get the center of
 * 
 * Get the #GisPoint representing the center of an object
 *
 * Returns: the center point
 */
#define gis_object_center(object) \
	(&GIS_OBJECT(object)->center)

#endif
