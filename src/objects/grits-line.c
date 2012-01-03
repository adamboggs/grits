/*
 * Copyright (C) 2012 Andy Spencer <andy753421@gmail.com>
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
 * SECTION:grits-line
 * @short_description: Single point lines
 *
 * Each #GritsLine represents a 3 dimentional line.
 */

#include <config.h>
#include "gtkgl.h"
#include "grits-line.h"

/* Drawing */
static void grits_line_trace(guint mode, gdouble (**points)[3])
{
	//g_debug("GritsLine: outline");
	for (int pi = 0; points[pi]; pi++) {
		glBegin(mode);
	 	for (int ci = 0; points[pi][ci][0] &&
	 	                 points[pi][ci][1] &&
	 	                 points[pi][ci][2]; ci++)
			glVertex3dv(points[pi][ci]);
		glEnd();
	}
}

static void grits_line_draw(GritsObject *_poly, GritsOpenGL *opengl)
{
	//g_debug("GritsLine: draw");
	GritsLine *line = GRITS_LINE(_poly);

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT |
			GL_POINT_BIT | GL_LINE_BIT | GL_POLYGON_BIT);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

	glColor4dv(line->color);

	glPointSize(line->width);
	glLineWidth(line->width);

	if (line->width > 1)
		grits_line_trace(GL_POINTS, line->points);
	grits_line_trace(GL_LINE_STRIP, line->points);

	glPopAttrib();
}

/**
 * grits_line_new:
 * @points:  TODO
 * @npoints: TODO
 *
 * Create a new GritsLine which TODO.
 *
 * Returns: the new #GritsLine.
 */
GritsLine *grits_line_new(gdouble (**points)[3])
{
	//g_debug("GritsLine: new - %p", points);
	GritsLine *line = g_object_new(GRITS_TYPE_LINE, NULL);
	line->points    = points;
	return line;
}

GritsLine *grits_line_parse(const gchar *str,
		const gchar *line_sep, const gchar *point_sep, const gchar *coord_sep)
{
	GritsPoint center;
	gdouble (**lines)[3] = parse_points(str,
			line_sep, point_sep, coord_sep, NULL, &center);

	GritsLine *line = grits_line_new(lines);
	GRITS_OBJECT(line)->center = center;
	GRITS_OBJECT(line)->skip   = GRITS_SKIP_CENTER;
	g_object_weak_ref(G_OBJECT(line), (GWeakNotify)free_points, lines);
	return line;
}

/* GObject code */
G_DEFINE_TYPE(GritsLine, grits_line, GRITS_TYPE_OBJECT);
static void grits_line_init(GritsLine *line)
{
	line->color[0] = 1;
	line->color[1] = 1;
	line->color[2] = 1;
	line->color[3] = 0.2;
	line->width    = 1;
	GRITS_OBJECT(line)->skip = GRITS_SKIP_STATE;
}

static void grits_line_finalize(GObject *_line)
{
	//g_debug("GritsLine: finalize");
	GritsLine *line = GRITS_LINE(_line);
	(void)line;
}

static void grits_line_class_init(GritsLineClass *klass)
{
	g_debug("GritsLine: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->finalize = grits_line_finalize;

	GritsObjectClass *object_class = GRITS_OBJECT_CLASS(klass);
	object_class->draw = grits_line_draw;
	//object_class->pick = grits_line_pick;
}
