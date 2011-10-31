/*
 * Copyright (C) 2009-2011 Andy Spencer <andy753421@gmail.com>
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
 * SECTION:grits-marker
 * @short_description: Single point markers
 *
 * Each #GritsMarker represents some point on the earth with some form of
 * content. Commonly this is used to mark geographic features such as cities or
 * states.
 * 
 * While markers represent a place in three dimensions somewhere on, below, or
 * above the surface of the earth, they are drawn in 2 dimensions so that they
 * look normal and readable by the user. Due to this, GritsObjects should almost
 * always be added to the GRITS_LEVEL_OVERLAY level so they are drawn "above" the
 * actual earth.
 */

#include <config.h>
#include <GL/gl.h>
#include "grits-marker.h"

/**
 * grits_marker_new:
 * @label: a short description of the marker
 *
 * Create a new GritsMarker which shows the given label when drawn.
 *
 * Returns: the new #GritsMarker.
 */
GritsMarker *grits_marker_new(const gchar *label)
{
	//g_debug("GritsMarker: new - %s", label);
	static const gdouble OUTLINE =   2;
	static const gdouble RADIUS  =   3;
	static const gdouble WIDTH   = 128;
	static const gdouble HEIGHT  =  32;

	GritsMarker *marker = g_object_new(GRITS_TYPE_MARKER, NULL);
	marker->xoff  = RADIUS+OUTLINE;
	marker->yoff  = HEIGHT-(RADIUS+OUTLINE);
	marker->label = g_strdup(label);
	marker->cairo = cairo_create(cairo_image_surface_create(
			CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT));

	cairo_select_font_face(marker->cairo, "sans-serif",
			CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(marker->cairo, 13);

	/* Draw outline */
	cairo_set_source_rgba(marker->cairo, 0, 0, 0, 1);
	cairo_set_line_width(marker->cairo, OUTLINE*2);

	cairo_arc(marker->cairo, marker->xoff, marker->yoff, RADIUS, 0, 2*G_PI);
	cairo_stroke(marker->cairo);

	cairo_move_to(marker->cairo, marker->xoff+4, marker->yoff-8);
	cairo_text_path(marker->cairo, marker->label);
	cairo_stroke(marker->cairo);

	/* Draw filler */
	cairo_set_source_rgba(marker->cairo, 1, 1, 1, 1);

	cairo_arc(marker->cairo, marker->xoff, marker->yoff, RADIUS, 0, 2*G_PI);
	cairo_fill(marker->cairo);

	cairo_move_to(marker->cairo, marker->xoff+4, marker->yoff-8);
	cairo_show_text(marker->cairo, marker->label);

	/* Load GL texture */
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &marker->tex);
	glBindTexture(GL_TEXTURE_2D, marker->tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			cairo_image_surface_get_data(cairo_get_target(marker->cairo)));
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	return marker;
}

/* Drawing */
static void grits_marker_draw(GritsObject *_marker, GritsOpenGL *opengl)
{
	GritsMarker *marker = GRITS_MARKER(_marker);
	GritsPoint  *point  = grits_object_center(marker);
	gdouble px, py, pz;
	grits_viewer_project(GRITS_VIEWER(opengl),
			point->lat, point->lon, point->elev,
			&px, &py, &pz);

	gint win_width  = GTK_WIDGET(opengl)->allocation.width;
	gint win_height = GTK_WIDGET(opengl)->allocation.height;
	py = win_height - py;
	if (pz > 1)
		return;

	//g_debug("GritsOpenGL: draw_marker - %s pz=%f ", marker->label, pz);

	cairo_surface_t *surface = cairo_get_target(marker->cairo);
	gdouble width  = cairo_image_surface_get_width(surface);
	gdouble height = cairo_image_surface_get_height(surface);

	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
	glOrtho(0, win_width, win_height, 0, -1, 1);
	glTranslated(px - marker->xoff,
	             py - marker->yoff, 0);

	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, marker->tex);
	glDisable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	glTexCoord2f(1, 0); glVertex3f(width, 0     , 0);
	glTexCoord2f(1, 1); glVertex3f(width, height, 0);
	glTexCoord2f(0, 1); glVertex3f(0    , height, 0);
	glTexCoord2f(0, 0); glVertex3f(0    , 0     , 0);
	glEnd();
}

/* GObject code */
G_DEFINE_TYPE(GritsMarker, grits_marker, GRITS_TYPE_OBJECT);
static void grits_marker_init(GritsMarker *marker)
{
}

static void grits_marker_finalize(GObject *_marker)
{
	//g_debug("GritsMarker: finalize - %s", marker->label);
	GritsMarker *marker = GRITS_MARKER(_marker);
	glDeleteTextures(1, &marker->tex);
	cairo_surface_t *surface = cairo_get_target(marker->cairo);
	cairo_surface_destroy(surface);
	cairo_destroy(marker->cairo);
	g_free(marker->label);
	glDeleteTextures(1, &marker->tex);
}

static void grits_marker_class_init(GritsMarkerClass *klass)
{
	g_debug("GritsMarker: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->finalize = grits_marker_finalize;

	GritsObjectClass *object_class = GRITS_OBJECT_CLASS(klass);
	object_class->draw = grits_marker_draw;
}
