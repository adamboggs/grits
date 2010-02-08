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
 * SECTION:gis-object
 * @short_description: Base classes for drawing operations
 *
 * Objects in libgis are things which can be added to the viewer and will be
 * displayed to the user. Each object has information such as it's location and
 * level of detail which are used by the viewer to determine which objects
 * should be drawn.
 *
 * Each #GisObject is also a #GObject, but not every GObject in libgis is a
 * GisObject. The "Object" part of the name is just coincidence.
 */

#include <config.h>
#include "gis-object.h"

/************
 * GisPoint *
 ************/
/**
 * gis_point_new:
 *
 * Create a new #GisPoint
 *
 * Returns: the new point
 */
GisPoint *gis_point_new()
{
	return g_new0(GisPoint, 1);
}

/**
 * gis_point_set_lle:
 * @point: the point to modify
 * @lat:   the new latitude
 * @lon:   the new longitude
 * @elev:  the new elevation
 *
 * Set the latitude, longitude, and elevation for a point.
 */
void gis_point_set_lle(GisPoint *point, gdouble lat, gdouble lon, gdouble elev)
{
	point->lat  = lat;
	point->lon  = lon;
	point->elev = elev;
}

/**
 * gis_point_free:
 * @point: The point to free
 *
 * Free data used by a #GisPoint
 */
void gis_point_free(GisPoint *point)
{
	g_free(point);
}


/*************
 * GisObject *
 *************/
/* GObject stuff */
G_DEFINE_ABSTRACT_TYPE(GisObject, gis_object, G_TYPE_OBJECT);
static void gis_object_init(GisObject *object)
{
}

static void gis_object_class_init(GisObjectClass *klass)
{
}
