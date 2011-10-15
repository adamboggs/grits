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
	GLUtesselator *tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN,  (_GLUfuncptr)glBegin);
	gluTessCallback(tess, GLU_TESS_VERTEX, (_GLUfuncptr)glVertex3dv);
	gluTessCallback(tess, GLU_TESS_END,    (_GLUfuncptr)glEnd);
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
}

static void grits_poly_outline(gdouble (**points)[3])
{
	//g_debug("GritsPoly: outline");
	for (int pi = 0; points[pi]; pi++) {
		glBegin(GL_LINE_LOOP);
	 	for (int ci = 0; points[pi][ci][0] &&
	 	                 points[pi][ci][1] &&
	 	                 points[pi][ci][2]; ci++)
			glVertex3dv(points[pi][ci]);
		glEnd();
		glBegin(GL_POINTS);
	 	for (int ci = 0; points[pi][ci][0] &&
	 	                 points[pi][ci][1] &&
	 	                 points[pi][ci][2]; ci++)
			glVertex3dv(points[pi][ci]);
		glEnd();
	}
}
static gboolean grits_poly_genlist(gpointer _poly)
{
	//g_debug("GritsPoly: genlist");
	GritsPoly *poly = GRITS_POLY(_poly);
	guint list = glGenLists(2);
	glNewList(list+0, GL_COMPILE);
	grits_poly_tess(poly->points);
	glEndList();
	glNewList(list+1, GL_COMPILE);
	grits_poly_outline(poly->points);
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
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	if (poly->color[3]) {
		glColor4dv(poly->color);
		glCallList(poly->list+0);
	}
	if (poly->border[3]) {
		glPointSize(poly->width);
		glLineWidth(poly->width);
		glColor4dv(poly->border);
		glCallList(poly->list+1);
	}
	glPopAttrib();
}

static void grits_poly_pick(GritsObject *_poly, GritsOpenGL *opengl)
{
	//g_debug("GritsPoly: pick");
	GritsPoly *poly = GRITS_POLY(_poly);
	if (!poly->list)
		return;
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CULL_FACE);
	glCallList(poly->list+0);
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

GritsPoly *grits_poly_parse(const gchar *str,
		const gchar *poly_sep, const gchar *point_sep, const gchar *coord_sep)
{
	/* Split and count polygons */
	gchar **spolys = g_strsplit(str, poly_sep, -1);
	int     npolys = g_strv_length(spolys);

	GritsBounds bounds = {-90, 90, -180, 180};
	gdouble (**polys)[3] = (gpointer)g_new0(double*, npolys+1);
	for (int pi = 0; pi < npolys; pi++) {
		/* Split and count coordinates */
		gchar **scoords = g_strsplit(spolys[pi], point_sep, -1);
		int     ncoords = g_strv_length(scoords);

		/* Create binary coords */
		gdouble (*coords)[3] = (gpointer)g_new0(gdouble, 3*(ncoords+1));
		for (int ci = 0; ci < ncoords; ci++) {
			gdouble lat, lon;
			sscanf(scoords[ci], "%lf,%lf", &lat, &lon);
			if (lat > bounds.n) bounds.n = lat;
			if (lat < bounds.s) bounds.s = lat;
			if (lon > bounds.e) bounds.e = lon;
			if (lon < bounds.w) bounds.w = lon;
			lle2xyz(lat, lon, 0,
					&coords[ci][0],
					&coords[ci][1],
					&coords[ci][2]);
		}

		/* Insert coords into poly array */
		polys[pi] = coords;
		g_strfreev(scoords);
	}

	/* Create GritsPoly */
	GritsPoly *poly = grits_poly_new(polys);
	GRITS_OBJECT(poly)->center.lat  = (bounds.n + bounds.s)/2;
	GRITS_OBJECT(poly)->center.lon  = lon_avg(bounds.e, bounds.w);
	GRITS_OBJECT(poly)->center.elev = 0;
	GRITS_OBJECT(poly)->skip        = GRITS_SKIP_CENTER;
	return poly;
}

/* GObject code */
G_DEFINE_TYPE(GritsPoly, grits_poly, GRITS_TYPE_OBJECT);
static void grits_poly_init(GritsPoly *poly)
{
	poly->border[0] = 1;
	poly->border[1] = 1;
	poly->border[2] = 1;
	poly->border[3] = 0.2;
	poly->width     = 1;
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
	object_class->pick = grits_poly_pick;
}
