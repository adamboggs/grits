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

#include <config.h>
#include "gis-object.h"

/* GisPoint */
GisPoint *gis_point_new()
{
	return g_new0(GisPoint, 1);
}

void gis_point_set_lle(GisPoint *self, gdouble lat, gdouble lon, gdouble elev)
{
	self->lat  = lat;
	self->lon  = lon;
	self->elev = elev;
}

void gis_point_free(GisPoint *self)
{
	g_free(self);
}


/* GisObject */
G_DEFINE_TYPE(GisObject, gis_object, G_TYPE_OBJECT);
static void gis_object_init(GisObject *self) { }
static void gis_object_class_init(GisObjectClass *klass) { }
