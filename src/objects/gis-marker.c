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
#include "gis-marker.h"

/* GisMarker */
GisMarker *gis_marker_new(const gchar *label)
{
	//g_debug("GisMarker: new - %s", label);
	static const int RADIUS =   4;
	static const int WIDTH  = 100;
	static const int HEIGHT =  20;

	GisMarker *self = g_object_new(GIS_TYPE_MARKER, NULL);
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

G_DEFINE_TYPE(GisMarker, gis_marker, GIS_TYPE_OBJECT);
static void gis_marker_init(GisMarker *self) { }

static void gis_marker_finalize(GObject *_self)
{
	GisMarker *self = GIS_MARKER(_self);
	//g_debug("GisMarker: finalize - %s", self->label);
	cairo_surface_t *surface = cairo_get_target(self->cairo);
	cairo_surface_destroy(surface);
	cairo_destroy(self->cairo);
	g_free(self->label);
}

static void gis_marker_class_init(GisMarkerClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = gis_marker_finalize;
}
