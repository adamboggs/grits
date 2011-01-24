/*
 * Copyright (C) 2010 Andy Spencer <andy753421@gmail.com>
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
 * SECTION:grits-poly
 * @short_description: Single point polys
 *
 * Each #GritsPoly represents a 3 dimentional polygon.
 */

#include <config.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "grits-poly.h"

/* Drawing */
static void grits_poly_tess(gdouble (**points)[3])
{
	//g_debug("GritsPoly: tess");

	/* Tesselate */
	GLUtesselator *tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN,  glBegin);
	gluTessCallback(tess, GLU_TESS_VERTEX, glVertex3dv);
	gluTessCallback(tess, GLU_TESS_END,    glEnd);
	gluTessBeginPolygon(tess, NULL);
	for (int pi = 0; points[pi]; pi++) {
		gluTessBeginContour(tess);
	 	for (int ci = 0; points[pi][ci][0]; ci++) {
			gluTessVertex(tess,
				points[pi][ci],
				points[pi][ci]);
		}
		gluTessEndContour(tess);
	}
	gluTessEndPolygon(tess);
	gluDeleteTess(tess);

	/* Outline */
	for (int pi = 0; points[pi]; pi++) {
		glColor4d(1,1,1,0.2);
		glBegin(GL_LINE_LOOP);
	 	for (int ci = 0; points[pi][ci][0]; ci++)
			glVertex3dv(points[pi][ci]);
		glEnd();
	}
}
static gboolean grits_poly_genlist(gpointer _poly)
{
	//g_debug("GritsPoly: genlist");
	GritsPoly *poly = GRITS_POLY(_poly);
	guint list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	grits_poly_tess(poly->points);
	glEndList();
	poly->list = list;
	return FALSE;
}

static void grits_poly_draw(GritsObject *_poly, GritsOpenGL *opengl)
{
	//g_debug("GritsPoly: draw");
	GritsPoly *poly = GRITS_POLY(_poly);

	if (!poly->list)
		return;

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glColor4dv(poly->color);
	glCallList(poly->list);
	glPopAttrib();
}

/**
 * grits_poly_new:
 * @points:  TODO
 * @npoints: TODO
 *
 * Create a new GritsPoly which TODO.
 *
 * Returns: the new #GritsPoly.
 */
GritsPoly *grits_poly_new(gdouble (**points)[3])
{
	//g_debug("GritsPoly: new - %p", points);
	GritsPoly *poly = g_object_new(GRITS_TYPE_POLY, NULL);
	poly->points    = points;
	g_idle_add(grits_poly_genlist, poly);
	return poly;
}

/* GObject code */
G_DEFINE_TYPE(GritsPoly, grits_poly, GRITS_TYPE_OBJECT);
static void grits_poly_init(GritsPoly *poly)
{
}

static void grits_poly_finalize(GObject *_poly)
{
	g_debug("GritsPoly: finalize");
	GritsPoly *poly = GRITS_POLY(_poly);
	(void)poly;
	// TODO: free points
}

static void grits_poly_class_init(GritsPolyClass *klass)
{
	g_debug("GritsPoly: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->finalize = grits_poly_finalize;

	GritsObjectClass *object_class = GRITS_OBJECT_CLASS(klass);
	object_class->draw = grits_poly_draw;
}
