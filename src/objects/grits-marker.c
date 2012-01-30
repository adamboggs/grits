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
#include <assert.h>
#include <math.h>
#include "gtkgl.h"
#include "grits-marker.h"

static void render_all(GritsMarker *marker);
static void render_point(GritsMarker *marker);
static void render_label(GritsMarker *marker);
static void render_icon(GritsMarker *marker);

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
	GritsMarker *marker = g_object_new(GRITS_TYPE_MARKER, NULL);

	marker->display_mask = MARKER_DMASK_POINT | MARKER_DMASK_LABEL;

	//g_debug("GritsMarker: new - %s", label);
	/* specify size of point and size of surface */
	marker->outline =   2;
	marker->radius  =   3;
	marker->width   = 128;
	marker->height  =  32;

	marker->xoff  = marker->radius+marker->outline;
	marker->yoff  = marker->height-(marker->radius+marker->outline);
	marker->cairo = cairo_create(cairo_image_surface_create(
			CAIRO_FORMAT_ARGB32, marker->width, marker->height));

	marker->label = g_strdup(label);

	render_all(marker);

	return marker;
}

GritsMarker *grits_marker_icon_new(const gchar *label, const gchar *filename,
    guint angle, gboolean flip)
{
	GritsMarker *marker = g_object_new(GRITS_TYPE_MARKER, NULL);

	marker->filename = g_strdup(filename);
	marker->angle = angle;
	marker->flip = flip;
	marker->display_mask = MARKER_DMASK_ICON;

	marker->outline =   2;
	marker->radius  =   3;
	marker->width   = 256; /* This limits icon size, should autodetect */
	marker->height  = 256;

	marker->xoff  = marker->width/2;
	marker->yoff  = marker->height/2;
	marker->cairo = cairo_create(cairo_image_surface_create(
			CAIRO_FORMAT_ARGB32, marker->width, marker->height));

	marker->label = g_strdup(label);

	render_all(marker);

	return marker;
}

static void
render_all(GritsMarker *marker)
{
	assert(marker);
	if (marker->display_mask & MARKER_DMASK_ICON) {
	    assert(marker->filename != NULL);
	    render_icon(marker);
	}
	if (marker->display_mask & MARKER_DMASK_POINT) {
	    render_point(marker);
	}
	if (marker->display_mask & MARKER_DMASK_LABEL) {
	    assert(marker->label);
	    render_label(marker);
	}

	/* Load GL texture */
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &marker->tex);
	glBindTexture(GL_TEXTURE_2D, marker->tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, marker->width, marker->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			cairo_image_surface_get_data(cairo_get_target(marker->cairo)));
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}


static void
render_point(GritsMarker *marker)
{
	/* Draw outline */
	cairo_set_source_rgba(marker->cairo, 0, 0, 0, 1);
	cairo_set_line_width(marker->cairo, marker->outline*2);

	cairo_arc(marker->cairo, marker->xoff, marker->yoff, marker->radius, 0, 2*G_PI);
	cairo_stroke(marker->cairo);

	/* Draw filler */
	cairo_set_source_rgba(marker->cairo, 1, 1, 1, 1);

	cairo_arc(marker->cairo, marker->xoff, marker->yoff, marker->radius, 0, 2*G_PI);
	cairo_fill(marker->cairo);
}

static void
render_label(GritsMarker *marker)
{
	cairo_set_source_rgba(marker->cairo, 0, 0, 0, 1);
	cairo_select_font_face(marker->cairo, "sans-serif",
			CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(marker->cairo, 13);
	cairo_move_to(marker->cairo, marker->xoff+4, marker->yoff-8);
	cairo_text_path(marker->cairo, marker->label);
	cairo_stroke(marker->cairo);

	/* Draw filler */
	cairo_set_source_rgba(marker->cairo, 1, 1, 1, 1);

	cairo_move_to(marker->cairo, marker->xoff+4, marker->yoff-8);
	cairo_show_text(marker->cairo, marker->label);
}

static void
render_icon(GritsMarker *marker)
{
    cairo_surface_t *input_img;

    g_debug("Using marker image %s", marker->filename);
    input_img = cairo_image_surface_create_from_png(marker->filename);
    if (cairo_surface_status(input_img)) {
        g_warning("Error reading file %s", marker->filename);
        return;
    }

    /* This code assumes the icon is an image pointing toward 90 degrees
     * (ie. east or to the right).  So to point it north for example, it
     * needs to rotate the icon -90 degrees (left).  If marker->flip is
     * set, then it will rotate the icon appropriately then reflect it
     * across the vertical axis so it's never upside down.
     */
    double flip = 1.0;
    double angle = marker->angle % 360;
    if (marker->flip && (angle < 360 && angle > 180)) {
	/* if the icon rotates to the left side it will be upside down */
	flip = -1.0; /* flip horizontally */
    }

    cairo_save(marker->cairo);

    /* move to marker location */
    cairo_translate(marker->cairo, marker->xoff, marker->yoff);

    /* perform rotation and flip in one transformation */
    double C = cos(angle*(M_PI/180.0));
    double S = sin(angle*(M_PI/180.0));
    double fx = flip; 
    double fy = 1.0;
    double tx = 0.0;
    double ty = 0.0;
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix,
        fx*C, fx*S,
        -S*fy, C*fy,
        C*tx*(1-fx)-S*ty*(fy-1)+tx-C*tx+S*ty,
        S*tx*(1-fx)+C*ty*(fy-1)+ty-S*tx-C*ty);
    cairo_transform(marker->cairo, &matrix);

    /* center image */
    unsigned int width = cairo_image_surface_get_width(input_img);
    unsigned int height = cairo_image_surface_get_height(input_img);
    cairo_translate (marker->cairo, -0.5*width, -0.5*height);

    cairo_set_source_surface(marker->cairo, input_img, 0, 0);

    cairo_paint(marker->cairo);
    cairo_restore(marker->cairo);
    cairo_surface_destroy(input_img);
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
	g_free(marker->filename);
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
