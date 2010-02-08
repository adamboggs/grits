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
 * SECTION:gis-marker
 * @short_description: Single point markers
 *
 * Each #GisMarker represents some point on the earth with some form of
 * content. Commonly this is used to mark geographic features such as cities or
 * states.
 * 
 * While markers represent a place in three dimensions somewhere on, below, or
 * above the surface of the earth, they are drawn in 2 dimensions so that they
 * look normal and readable by the user. Due to this, GisObjects should almost
 * always be added to the GIS_LEVEL_OVERLAY level so they are drawn "above" the
 * actual earth.
 */

#include <config.h>
#include "gis-marker.h"

/*************
 * GisMarker *
 *************/
/**
 * gis_marker_new:
 * @label: a short description of the marker
 *
 * Create a new GisMarker which shows the given label when drawn.
 *
 * Returns: the new #GisMarker.
 */
GisMarker *gis_marker_new(const gchar *label)
{
	//g_debug("GisMarker: new - %s", label);
	static const int RADIUS =   4;
	static const int WIDTH  = 100;
	static const int HEIGHT =  20;

	GisMarker *marker = g_object_new(GIS_TYPE_MARKER, NULL);
	marker->xoff  = RADIUS;
	marker->yoff  = HEIGHT-RADIUS;
	marker->label = g_strdup(label);
	marker->cairo = cairo_create(cairo_image_surface_create(
				CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT));
	cairo_set_source_rgba(marker->cairo, 1, 1, 1, 1);
	cairo_arc(marker->cairo, marker->xoff, marker->yoff, RADIUS, 0, 2*G_PI);
	cairo_fill(marker->cairo);
	cairo_move_to(marker->cairo, marker->xoff+4, marker->yoff-8);
	cairo_set_font_size(marker->cairo, 10);
	cairo_show_text(marker->cairo, marker->label);
	return marker;
}

G_DEFINE_TYPE(GisMarker, gis_marker, GIS_TYPE_OBJECT);
static void gis_marker_init(GisMarker *marker)
{
}

static void gis_marker_finalize(GObject *_marker)
{
	GisMarker *marker = GIS_MARKER(_marker);
	//g_debug("GisMarker: finalize - %s", marker->label);
	cairo_surface_t *surface = cairo_get_target(marker->cairo);
	cairo_surface_destroy(surface);
	cairo_destroy(marker->cairo);
	g_free(marker->label);
}

static void gis_marker_class_init(GisMarkerClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = gis_marker_finalize;
}
