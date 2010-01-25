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

#include <glib.h>
#include <GL/glu.h>

#include "gis-object.h"
#include "gis-util.h"

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


/* GisMarker */
GisMarker *gis_marker_new(const gchar *label)
{
	static const int RADIUS =   4;
	static const int WIDTH  = 100;
	static const int HEIGHT =  20;

	GisMarker *self = g_new0(GisMarker, 1);
	GIS_OBJECT(self)->type = GIS_TYPE_MARKER;
	self->xoff  = RADIUS;
	self->yoff  = HEIGHT-RADIUS;
	self->label = g_strdup(label);
	self->cairo = cairo_create(cairo_image_surface_create(
				CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT));
	cairo_set_source_rgba(self->cairo, 1, 1, 1, 1);
	cairo_arc(self->cairo, self->xoff, self->yoff, RADIUS, 0, 2*G_PI);
	cairo_fill(self->cairo);
	cairo_move_to(self->cairo, self->xoff+4, self->yoff-8);
	cairo_set_font_size(self->cairo, 10);
	cairo_show_text(self->cairo, self->label);
	return self;
}

void gis_marker_free(GisMarker *self)
{
	cairo_surface_destroy(cairo_get_target(self->cairo));
	cairo_destroy(self->cairo);
	g_free(self->label);
	g_free(self);
}


/* GisCallback */
GisCallback *gis_callback_new(GisCallbackFunc callback, gpointer user_data)
{
	GisCallback *self = g_new0(GisCallback, 1);
	GIS_OBJECT(self)->type = GIS_TYPE_CALLBACK;
	self->callback  = callback;
	self->user_data = user_data;
	return self;
}

void gis_callback_free(GisCallback *self)
{
	g_free(self);
}
