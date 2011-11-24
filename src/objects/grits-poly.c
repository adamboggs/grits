/*
 * Copyright (C) 2010-2011 Andy Spencer <andy753421@gmail.com>
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
	for (int pi = 0; points[pi]; pi++) {
		gluTessBeginPolygon(tess, NULL);
		gluTessBeginContour(tess);
	 	for (int ci = 0; points[pi][ci][0]; ci++) {
			gluTessVertex(tess,
				points[pi][ci],
				points[pi][ci]);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
	}
	gluDeleteTess(tess);
}

static void grits_poly_outline(gdouble (**points)[3])
{
	//g_debug("GritsPoly: outline");
	for (int pi = 0; points[pi]; pi++) {
		glBegin(GL_POLYGON);
	 	for (int ci = 0; points[pi][ci][0] &&
	 	                 points[pi][ci][1] &&
	 	                 points[pi][ci][2]; ci++)
			glVertex3dv(points[pi][ci]);
		glEnd();
	}
}

static gboolean grits_poly_runlist(GritsPoly *poly, int i,
		void (*render)(gdouble(**)[3]))
{
	//g_debug("GritsPoly: genlist");
	if (poly->list[i]) {
		glCallList(poly->list[i]);
	} else {
		guint list = glGenLists(1);
		glNewList(list, GL_COMPILE_AND_EXECUTE);
		render(poly->points);
		glEndList();
		poly->list[i] = list;
	}
	return FALSE;
}

static void grits_poly_draw(GritsObject *_poly, GritsOpenGL *opengl)
{
	//g_debug("GritsPoly: draw");
	GritsPoly *poly = GRITS_POLY(_poly);

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT |
			GL_POINT_BIT | GL_LINE_BIT | GL_POLYGON_BIT);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glEnable(GL_POLYGON_OFFSET_POINT);

	if (poly->color[3]) {
		/* Draw background farthest back */
		glPolygonOffset(3, 3);
		glColor4dv(poly->color);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		grits_poly_runlist(poly, 0, grits_poly_tess);
	}

	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

	if (!poly->color[3] && poly->border[3] && poly->width > 1) {
		/* Draw line border in the middle */
		glColor4d(0,0,0,1);

		glPointSize(poly->width*2);
		glLineWidth(poly->width*2);

		glPolygonOffset(2, 2);

		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		grits_poly_runlist(poly, 1, grits_poly_outline);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		grits_poly_runlist(poly, 1, grits_poly_outline);
	}

	if (poly->border[3]) {
		/* Draw border front-most */
		glColor4dv(poly->border);

		glPointSize(poly->width);
		glLineWidth(poly->width);

		glPolygonOffset(1, 1);
		if (poly->width > 1) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			grits_poly_runlist(poly, 1, grits_poly_outline);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		grits_poly_runlist(poly, 1, grits_poly_outline);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopAttrib();
}

static void grits_poly_pick(GritsObject *_poly, GritsOpenGL *opengl)
{
	//g_debug("GritsPoly: pick");
	GritsPoly *poly = GRITS_POLY(_poly);
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CULL_FACE);
	grits_poly_runlist(poly, 0, grits_poly_tess);
	glPopAttrib();
}

static gboolean grits_poly_delete(gpointer list)
{
	glDeleteLists((guintptr)list, 1);
	return FALSE;
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
	return poly;
}

static void _free_points(gdouble (**points)[3])
{
	for (int i = 0; points[i]; i++)
		g_free(points[i]);
	g_free(points);
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
	g_strfreev(spolys);

	/* Create GritsPoly */
	GritsPoly *poly = grits_poly_new(polys);
	GRITS_OBJECT(poly)->center.lat  = (bounds.n + bounds.s)/2;
	GRITS_OBJECT(poly)->center.lon  = lon_avg(bounds.e, bounds.w);
	GRITS_OBJECT(poly)->center.elev = 0;
	GRITS_OBJECT(poly)->skip        = GRITS_SKIP_CENTER;
	g_object_weak_ref(G_OBJECT(poly), (GWeakNotify)_free_points, polys);
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
	GRITS_OBJECT(poly)->skip = GRITS_SKIP_STATE;
}

static void grits_poly_finalize(GObject *_poly)
{
	//g_debug("GritsPoly: finalize");
	GritsPoly *poly = GRITS_POLY(_poly);
	if (poly->list[0]) g_idle_add(grits_poly_delete, (gpointer)(guintptr)poly->list[0]);
	if (poly->list[1]) g_idle_add(grits_poly_delete, (gpointer)(guintptr)poly->list[1]);
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
