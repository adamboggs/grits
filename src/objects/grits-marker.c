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
	marker->icon_width  = marker->radius * 2;
	marker->icon_height = marker->radius * 3;

	marker->xoff  = marker->radius+marker->outline;
	marker->yoff  = marker->height-(marker->radius+marker->outline);
	marker->cairo = cairo_create(cairo_image_surface_create(
			CAIRO_FORMAT_ARGB32, marker->width, marker->height));

	marker->label = g_strdup(label);

	render_all(marker);

	return marker;
}

/*
 * Create a new marker with a label, point, icon (png), or any
 * combination of the above.
 * label: The label to display if MARKER_DMASK_LABEL is set
 * filename: The filename of the icon
 * angle: The angle to rotate the icon (0 is north)
 * flip: Whether to flip the image so that it's never upside down.
 *       Useful for non-symmetric icons which have an "up".
 * display_mask: A bitmask which specifies which items to display.
 */
GritsMarker *grits_marker_icon_new(const gchar *label, const gchar *filename,
    guint angle, gboolean flip, guint display_mask)
{
	GritsMarker *marker = g_object_new(GRITS_TYPE_MARKER, NULL);

	marker->label = g_strdup(label);
	marker->angle = angle;
	marker->flip = flip;
	marker->display_mask = display_mask;

	if (display_mask & MARKER_DMASK_ICON) {
		g_assert(filename != NULL);
		g_debug("GritsMarker: marker_icon_new - marker image %s",
		        filename);
		marker->icon_img = cairo_image_surface_create_from_png(filename);
		if (cairo_surface_status(marker->icon_img)) {
			g_warning("GritsMarker: marker_icon_new - "
			          "error reading file %s", filename);
			/* convert it to a point, better than nothing */
			marker->display_mask &= ~MARKER_DMASK_ICON;
			marker->display_mask |= MARKER_DMASK_POINT;
			marker->icon_width = 4;
			marker->icon_height = 8;

		} else {
			marker->icon_width =
			    cairo_image_surface_get_width(marker->icon_img);
			marker->icon_height =
			    cairo_image_surface_get_height(marker->icon_img);
		}
	} else {
		marker->icon_img = NULL;
		marker->icon_width = 4;	/* room for the point if there is one */
		marker->icon_height = 8;
	}
	g_debug("GritsMarker: marker_icon_new - width = %d, height = %d",
		marker->icon_width, marker->icon_height);

	marker->outline =   2;
	marker->radius  =   3;
	/* this is the surface size, a guess really */
	marker->width   = marker->icon_width  + 128;
	marker->height  = marker->icon_height + 64;

	marker->xoff  = marker->width/2;
	marker->yoff  = marker->height/2;
	marker->cairo = cairo_create(cairo_image_surface_create(
			CAIRO_FORMAT_ARGB32, marker->width, marker->height));
	/* clear the surface just in case */
	cairo_set_operator(marker->cairo, CAIRO_OPERATOR_SOURCE);
	//cairo_set_source_rgba(marker->cairo, 1.0, 0.0, 0.0, 0.3); // debug
	cairo_set_source_rgba(marker->cairo, 1.0, 0.0, 0.0, 0.0);
	cairo_paint(marker->cairo);
	cairo_set_operator(marker->cairo, CAIRO_OPERATOR_OVER);

	render_all(marker);

	if (marker->icon_img) {
		cairo_surface_destroy(marker->icon_img);
	}

	return marker;
}

static void render_all(GritsMarker *marker)
{
	g_assert(marker);
	if (marker->display_mask & MARKER_DMASK_ICON) {
		render_icon(marker);
	}
	if (marker->display_mask & MARKER_DMASK_POINT) {
		render_point(marker);
	}
	if (marker->display_mask & MARKER_DMASK_LABEL) {
		render_label(marker);
	}

	/* Load GL texture */
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &marker->tex);
	glBindTexture(GL_TEXTURE_2D, marker->tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, marker->width, marker->height,
	        0, GL_BGRA, GL_UNSIGNED_BYTE,
	        cairo_image_surface_get_data(cairo_get_target(marker->cairo)));
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}


static void render_point(GritsMarker *marker)
{
	/* Draw outline */
	cairo_set_source_rgba(marker->cairo, 0, 0, 0, 1);
	cairo_set_line_width(marker->cairo, marker->outline*2);

	cairo_arc(marker->cairo, marker->xoff, marker->yoff, marker->radius,
	          0, 2*G_PI);
	cairo_stroke(marker->cairo);

	/* Draw filler */
	cairo_set_source_rgba(marker->cairo, 1, 1, 1, 1);

	cairo_arc(marker->cairo, marker->xoff, marker->yoff, marker->radius,
	          0, 2*G_PI);
	cairo_fill(marker->cairo);
}

static void render_label(GritsMarker *marker)
{
	g_assert(marker->label);

	cairo_set_source_rgba(marker->cairo, 0, 0, 0, 1);
	cairo_select_font_face(marker->cairo, "sans-serif",
			CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(marker->cairo, 13);
	cairo_move_to(marker->cairo, marker->xoff + (marker->icon_width  / 2),
	                             marker->yoff - (marker->icon_height / 2));
	cairo_text_path(marker->cairo, marker->label);
	cairo_stroke(marker->cairo);

	/* Draw filler */
	cairo_set_source_rgba(marker->cairo, 1, 1, 1, 1);
	cairo_move_to(marker->cairo, marker->xoff + (marker->icon_width  / 2),
	                             marker->yoff - (marker->icon_height / 2));
	cairo_show_text(marker->cairo, marker->label);
}

static void render_icon(GritsMarker *marker)
{
	g_assert(marker->icon_img != NULL);

	/* This code assumes the icon is an image pointing toward 0 degrees
	* (ie. north/up).  If marker->flip is set, then it will rotate the
	* icon appropriately then reflect it across the vertical axis so
	* it's never upside down.
	*/
	gdouble flip = 1.0;
	gdouble angle = marker->angle % 360;
	if (marker->flip && (angle < 360 && angle > 180)) {
		/* if icon rotates to the left half it will be upside down */
		flip = -1.0; /* flip horizontally */
	}

	cairo_save(marker->cairo);

	/* move to marker location */
	cairo_translate(marker->cairo, marker->xoff, marker->yoff);

	/* perform rotation and flip in one transformation */
	gdouble C = cos(angle*(M_PI/180.0));
	gdouble S = sin(angle*(M_PI/180.0));
	gdouble fx = flip; 
	gdouble fy = 1.0;
	gdouble tx = 0.0;
	gdouble ty = 0.0;
	cairo_matrix_t matrix;
	cairo_matrix_init(&matrix,
	        fx*C, fx*S,
	        -S*fy, C*fy,
	        C*tx*(1-fx)-S*ty*(fy-1)+tx-C*tx+S*ty,
	        S*tx*(1-fx)+C*ty*(fy-1)+ty-S*tx-C*ty);
	cairo_transform(marker->cairo, &matrix);

	/* center image */
	cairo_translate(marker->cairo, -marker->icon_width/2,
	                               -marker->icon_height/2);

	cairo_set_source_surface(marker->cairo, marker->icon_img, 0, 0);

	cairo_paint(marker->cairo);
	cairo_restore(marker->cairo);
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
