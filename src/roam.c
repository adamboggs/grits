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

#include <glib.h>
#include <math.h>
#include <string.h>
#include "gpqueue.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "gis-util.h"
#include "roam.h"

/**
 * TODO:
 *   - Optimize for memory consumption
 *   - Profile for computation speed
 *   - Target polygon count/detail
 */

/* For GPQueue comparators */
static gint tri_cmp(RoamTriangle *a, RoamTriangle *b, gpointer data)
{
	if      (a->error < b->error) return  1;
	else if (a->error > b->error) return -1;
	else                          return  0;
}
static gint dia_cmp(RoamDiamond *a, RoamDiamond *b, gpointer data)
{
	if      (a->error < b->error) return -1;
	else if (a->error > b->error) return  1;
	else                          return  0;
}


/*************
 * RoamPoint *
 *************/
RoamPoint *roam_point_new(gdouble lat, gdouble lon, gdouble elev)
{
	RoamPoint *point = g_new0(RoamPoint, 1);
	point->lat  = lat;
	point->lon  = lon;
	point->elev = elev;
	/* For get_intersect */
	lle2xyz(lat, lon, elev, &point->x, &point->y, &point->z);
	return point;
}
RoamPoint *roam_point_dup(RoamPoint *point)
{
	RoamPoint *new = g_memdup(point, sizeof(RoamPoint));
	new->tris = 0;
	return new;
}
void roam_point_add_triangle(RoamPoint *point, RoamTriangle *triangle)
{
	for (int i = 0; i < 3; i++) {
		point->norm[i] *= point->tris;
		point->norm[i] += triangle->norm[i];
	}
	point->tris++;
	for (int i = 0; i < 3; i++)
		point->norm[i] /= point->tris;
}
void roam_point_remove_triangle(RoamPoint *point, RoamTriangle *triangle)
{
	for (int i = 0; i < 3; i++) {
		point->norm[i] *= point->tris;
		point->norm[i] -= triangle->norm[i];
	}
	point->tris--;
	if (point->tris)
		for (int i = 0; i < 3; i++)
			point->norm[i] /= point->tris;
}
void roam_point_update_height(RoamPoint *point)
{
	if (point->height_func) {
		gdouble elev = point->height_func(
				point->lat, point->lon, point->height_data);
		lle2xyz(point->lat, point->lon, elev,
				&point->x, &point->y, &point->z);
	}
}
void roam_point_update_projection(RoamPoint *point, RoamSphere *sphere)
{
	static int count   = 0;
	static int version = 0;
	if (version != sphere->view->version) {
		g_debug("RoamPoint: Projected %d points", count);
		count   = 0;
		version = sphere->view->version;
	}

	if (point->pversion != sphere->view->version) {
		/* Cache projection */
		gluProject(point->x, point->y, point->z,
			sphere->view->model, sphere->view->proj, sphere->view->view,
			&point->px, &point->py, &point->pz);
		point->pversion = sphere->view->version;
		count++;
	}
}

/****************
 * RoamTriangle *
 ****************/
RoamTriangle *roam_triangle_new(RoamPoint *l, RoamPoint *m, RoamPoint *r)
{
	RoamTriangle *triangle = g_new0(RoamTriangle, 1);

	triangle->error = 0;
	triangle->p.l = l;
	triangle->p.m = m;
	triangle->p.r = r;
	triangle->split = roam_point_new(
		(l->lat + r->lat)/2,
		(ABS(l->lat) == 90 ? r->lon :
		 ABS(r->lat) == 90 ? l->lon :
		 lon_avg(l->lon, r->lon)),
		(l->elev + r->elev)/2);
	/* TODO: Move this back to sphere, or actually use the nesting */
	triangle->split->height_func = m->height_func;
	triangle->split->height_data = m->height_data;
	roam_point_update_height(triangle->split);
	//if ((float)triangle->split->lat > 44 && (float)triangle->split->lat < 46)
	//	g_debug("RoamTriangle: new - (l,m,r,split).lats = %7.2f %7.2f %7.2f %7.2f",
	//			l->lat, m->lat, r->lat, triangle->split->lat);
	//if ((float)l->lat == (float)r->lat &&
	//    (float)triangle->split->lat != (float)l->lat)
	//	g_warning("RoamTriangle: new - split.lat=%f != (l=r).lat=%f",
	//		triangle->split->lat, l->lat);

	/* Update normal */
	double pa[3];
	double pb[3];
	pa[0] = triangle->p.l->x - triangle->p.m->x;
	pa[1] = triangle->p.l->y - triangle->p.m->y;
	pa[2] = triangle->p.l->z - triangle->p.m->z;

	pb[0] = triangle->p.r->x - triangle->p.m->x;
	pb[1] = triangle->p.r->y - triangle->p.m->y;
	pb[2] = triangle->p.r->z - triangle->p.m->z;

	triangle->norm[0] = pa[1] * pb[2] - pa[2] * pb[1];
	triangle->norm[1] = pa[2] * pb[0] - pa[0] * pb[2];
	triangle->norm[2] = pa[0] * pb[1] - pa[1] * pb[0];

	double total = sqrt(triangle->norm[0] * triangle->norm[0] +
	                    triangle->norm[1] * triangle->norm[1] +
	                    triangle->norm[2] * triangle->norm[2]);

	triangle->norm[0] /= total;
	triangle->norm[1] /= total;
	triangle->norm[2] /= total;

	/* Store bounding box, for get_intersect */
	RoamPoint *p[] = {l,m,r};
	triangle->edge.n =  -90; triangle->edge.s =  90;
	triangle->edge.e = -180; triangle->edge.w = 180;
	gboolean maxed = FALSE;
	for (int i = 0; i < G_N_ELEMENTS(p); i++) {
		triangle->edge.n = MAX(triangle->edge.n, p[i]->lat);
		triangle->edge.s = MIN(triangle->edge.s, p[i]->lat);
		if (p[i]->lat == 90 || p[i]->lat == -90)
			continue;
		if (p[i]->lon == 180) {
			maxed = TRUE;
			continue;
		}
		triangle->edge.e = MAX(triangle->edge.e, p[i]->lon);
		triangle->edge.w = MIN(triangle->edge.w, p[i]->lon);
	}
	if (maxed) {
		if (triangle->edge.e < 0)
			triangle->edge.w = -180;
		else
			triangle->edge.e =  180;
	}

	//g_message("roam_triangle_new: %p", triangle);
	return triangle;
}

void roam_triangle_free(RoamTriangle *triangle)
{
	g_free(triangle->split);
	g_free(triangle);
}

void roam_triangle_add(RoamTriangle *triangle,
		RoamTriangle *left, RoamTriangle *base, RoamTriangle *right,
		RoamSphere *sphere)
{
	triangle->t.l = left;
	triangle->t.b = base;
	triangle->t.r = right;

	roam_point_add_triangle(triangle->p.l, triangle);
	roam_point_add_triangle(triangle->p.m, triangle);
	roam_point_add_triangle(triangle->p.r, triangle);

	if (sphere->view)
		roam_triangle_update_errors(triangle, sphere);

	triangle->handle = g_pqueue_push(sphere->triangles, triangle);
}

void roam_triangle_remove(RoamTriangle *triangle, RoamSphere *sphere)
{
	/* Update vertex normals */
	roam_point_remove_triangle(triangle->p.l, triangle);
	roam_point_remove_triangle(triangle->p.m, triangle);
	roam_point_remove_triangle(triangle->p.r, triangle);

	g_pqueue_remove(sphere->triangles, triangle->handle);
}

void roam_triangle_sync_neighbors(RoamTriangle *new, RoamTriangle *old, RoamTriangle *neigh)
{
	if      (neigh->t.l == old) neigh->t.l = new;
	else if (neigh->t.b == old) neigh->t.b = new;
	else if (neigh->t.r == old) neigh->t.r = new;
	else g_assert_not_reached();
}

gboolean roam_point_visible(RoamPoint *triangle, RoamSphere *sphere)
{
	gint *view = sphere->view->view;
	return triangle->px > view[0] && triangle->px < view[2] &&
	       triangle->py > view[1] && triangle->py < view[3] &&
	       triangle->pz > 0       && triangle->pz < 1;
}
gboolean roam_triangle_visible(RoamTriangle *triangle, RoamSphere *sphere)
{
	/* Do this with a bounding box */
	return roam_point_visible(triangle->p.l, sphere) ||
	       roam_point_visible(triangle->p.m, sphere) ||
	       roam_point_visible(triangle->p.r, sphere);
}

void roam_triangle_update_errors(RoamTriangle *triangle, RoamSphere *sphere)
{
	/* Update points */
	roam_point_update_projection(triangle->p.l, sphere);
	roam_point_update_projection(triangle->p.m, sphere);
	roam_point_update_projection(triangle->p.r, sphere);

	/* Not exactly correct, could be out on both sides (middle in) */
	if (!roam_triangle_visible(triangle, sphere)) {
		triangle->error = -1;
	} else {
		roam_point_update_projection(triangle->split, sphere);
		RoamPoint *l = triangle->p.l;
		RoamPoint *m = triangle->p.m;
		RoamPoint *r = triangle->p.r;
		RoamPoint *split = triangle->split;

		/*               l-r midpoint        projected l-r midpoint */
		gdouble pxdist = (l->px + r->px)/2 - split->px;
		gdouble pydist = (l->py + r->py)/2 - split->py;

		triangle->error = sqrt(pxdist*pxdist + pydist*pydist);

		/* Multiply by size of triangle */
		double size = -( l->px * (m->py - r->py) +
		                 m->px * (r->py - l->py) +
		                 r->px * (l->py - m->py) ) / 2.0;

		/* Size < 0 == backface */
		//if (size < 0)
		//	triangle->error *= -1;
		triangle->error *= size;
	}
}

void roam_triangle_split(RoamTriangle *triangle, RoamSphere *sphere)
{
	//g_message("roam_triangle_split: %p, e=%f\n", triangle, triangle->error);

	sphere->polys += 2;

	if (triangle != triangle->t.b->t.b)
		roam_triangle_split(triangle->t.b, sphere);
	if (triangle != triangle->t.b->t.b)
		g_assert_not_reached();

	RoamTriangle *base = triangle->t.b;

	/* Add new triangles */
	RoamPoint *mid = triangle->split;
	RoamTriangle *sl = triangle->kids[0] = roam_triangle_new(triangle->p.m, mid, triangle->p.l); // triangle Left
	RoamTriangle *sr = triangle->kids[1] = roam_triangle_new(triangle->p.r, mid, triangle->p.m); // triangle Right
	RoamTriangle *bl = base->kids[0] = roam_triangle_new(base->p.m, mid, base->p.l); // Base Left
	RoamTriangle *br = base->kids[1] = roam_triangle_new(base->p.r, mid, base->p.m); // Base Right

	/*                triangle,l,  base,      r,  sphere */
	roam_triangle_add(sl, sr, triangle->t.l, br, sphere);
	roam_triangle_add(sr, bl, triangle->t.r, sl, sphere);
	roam_triangle_add(bl, br, base->t.l, sr, sphere);
	roam_triangle_add(br, sl, base->t.r, bl, sphere);

	roam_triangle_sync_neighbors(sl, triangle, triangle->t.l);
	roam_triangle_sync_neighbors(sr, triangle, triangle->t.r);
	roam_triangle_sync_neighbors(bl, base, base->t.l);
	roam_triangle_sync_neighbors(br, base, base->t.r);

	/* Remove old triangles */
	roam_triangle_remove(triangle, sphere);
	roam_triangle_remove(base, sphere);

	/* Add/Remove diamonds */
	RoamDiamond *diamond = roam_diamond_new(triangle, base, sl, sr, bl, br);
	roam_diamond_update_errors(diamond, sphere);
	roam_diamond_add(diamond, sphere);
	roam_diamond_remove(triangle->parent, sphere);
	roam_diamond_remove(base->parent, sphere);
}

void roam_triangle_draw(RoamTriangle *triangle)
{
	glBegin(GL_TRIANGLES);
	glNormal3dv(triangle->p.r->norm); glVertex3dv((double*)triangle->p.r);
	glNormal3dv(triangle->p.m->norm); glVertex3dv((double*)triangle->p.m);
	glNormal3dv(triangle->p.l->norm); glVertex3dv((double*)triangle->p.l);
	glEnd();
	return;
}

void roam_triangle_draw_normal(RoamTriangle *triangle)
{
	double center[] = {
		(triangle->p.l->x + triangle->p.m->x + triangle->p.r->x)/3.0,
		(triangle->p.l->y + triangle->p.m->y + triangle->p.r->y)/3.0,
		(triangle->p.l->z + triangle->p.m->z + triangle->p.r->z)/3.0,
	};
	double end[] = {
		center[0]+triangle->norm[0]*2000000,
		center[1]+triangle->norm[1]*2000000,
		center[2]+triangle->norm[2]*2000000,
	};
	glBegin(GL_LINES);
	glVertex3dv(center);
	glVertex3dv(end);
	glEnd();
}

/***************
 * RoamDiamond *
 ***************/
RoamDiamond *roam_diamond_new(
		RoamTriangle *parent0, RoamTriangle *parent1,
		RoamTriangle *kid0, RoamTriangle *kid1,
		RoamTriangle *kid2, RoamTriangle *kid3)
{
	RoamDiamond *diamond = g_new0(RoamDiamond, 1);

	diamond->kids[0] = kid0;
	diamond->kids[1] = kid1;
	diamond->kids[2] = kid2;
	diamond->kids[3] = kid3;

	kid0->parent = diamond;
	kid1->parent = diamond;
	kid2->parent = diamond;
	kid3->parent = diamond;

	diamond->parents[0] = parent0;
	diamond->parents[1] = parent1;

	return diamond;
}
void roam_diamond_add(RoamDiamond *diamond, RoamSphere *sphere)
{
	diamond->active = TRUE;
	diamond->error  = MAX(diamond->parents[0]->error, diamond->parents[1]->error);
	diamond->handle = g_pqueue_push(sphere->diamonds, diamond);
}
void roam_diamond_remove(RoamDiamond *diamond, RoamSphere *sphere)
{
	if (diamond && diamond->active) {
		diamond->active = FALSE;
		g_pqueue_remove(sphere->diamonds, diamond->handle);
	}
}
void roam_diamond_merge(RoamDiamond *diamond, RoamSphere *sphere)
{
	//g_message("roam_diamond_merge: %p, e=%f\n", diamond, diamond->error);

	sphere->polys -= 2;

	/* TODO: pick the best split */
	RoamTriangle **kids = diamond->kids;

	/* Create triangles */
	RoamTriangle *triangle  = diamond->parents[0];
	RoamTriangle *base = diamond->parents[1];

	roam_triangle_add(triangle,  kids[0]->t.b, base, kids[1]->t.b, sphere);
	roam_triangle_add(base, kids[2]->t.b, triangle,  kids[3]->t.b, sphere);

	roam_triangle_sync_neighbors(triangle,  kids[0], kids[0]->t.b);
	roam_triangle_sync_neighbors(triangle,  kids[1], kids[1]->t.b);
	roam_triangle_sync_neighbors(base, kids[2], kids[2]->t.b);
	roam_triangle_sync_neighbors(base, kids[3], kids[3]->t.b);

	/* Remove triangles */
	roam_triangle_remove(kids[0], sphere);
	roam_triangle_remove(kids[1], sphere);
	roam_triangle_remove(kids[2], sphere);
	roam_triangle_remove(kids[3], sphere);

	/* Clear kids */
	triangle->kids[0]  = triangle->kids[1]  = NULL;
	base->kids[0] = base->kids[1] = NULL;

	/* Add/Remove triangles */
	if (triangle->t.l->t.l == triangle->t.r->t.r &&
	    triangle->t.l->t.l != triangle && triangle->parent) {
		roam_diamond_update_errors(triangle->parent, sphere);
		roam_diamond_add(triangle->parent, sphere);
	}

	if (base->t.l->t.l == base->t.r->t.r &&
	    base->t.l->t.l != base && base->parent) {
		roam_diamond_update_errors(base->parent, sphere);
		roam_diamond_add(base->parent, sphere);
	}

	/* Remove and free diamond and child triangles */
	roam_diamond_remove(diamond, sphere);
	g_assert(diamond->kids[0]->p.m == diamond->kids[1]->p.m &&
	         diamond->kids[1]->p.m == diamond->kids[2]->p.m &&
	         diamond->kids[2]->p.m == diamond->kids[3]->p.m);
	g_assert(diamond->kids[0]->p.m->tris == 0);
	roam_triangle_free(diamond->kids[0]);
	roam_triangle_free(diamond->kids[1]);
	roam_triangle_free(diamond->kids[2]);
	roam_triangle_free(diamond->kids[3]);
	g_free(diamond);
}
void roam_diamond_update_errors(RoamDiamond *diamond, RoamSphere *sphere)
{
	roam_triangle_update_errors(diamond->parents[0], sphere);
	roam_triangle_update_errors(diamond->parents[1], sphere);
	diamond->error = MAX(diamond->parents[0]->error, diamond->parents[1]->error);
}

/**************
 * RoamSphere *
 **************/
RoamSphere *roam_sphere_new()
{
	RoamSphere *sphere = g_new0(RoamSphere, 1);
	sphere->polys       = 8;
	sphere->triangles   = g_pqueue_new((GCompareDataFunc)tri_cmp, NULL);
	sphere->diamonds    = g_pqueue_new((GCompareDataFunc)dia_cmp, NULL);

	RoamPoint *vertexes[] = {
		roam_point_new( 90,   0,  0), // 0 (North)
		roam_point_new(-90,   0,  0), // 1 (South)
		roam_point_new(  0,   0,  0), // 2 (Europe/Africa)
		roam_point_new(  0,  90,  0), // 3 (Asia,East)
		roam_point_new(  0, 180,  0), // 4 (Pacific)
		roam_point_new(  0, -90,  0), // 5 (Americas,West)
	};
	int _triangles[][2][3] = {
		/*lv mv rv   ln, bn, rn */
		{{2,0,3}, {3, 4, 1}}, // 0
		{{3,0,4}, {0, 5, 2}}, // 1
		{{4,0,5}, {1, 6, 3}}, // 2
		{{5,0,2}, {2, 7, 0}}, // 3
		{{3,1,2}, {5, 0, 7}}, // 4
		{{4,1,3}, {6, 1, 4}}, // 5
		{{5,1,4}, {7, 2, 5}}, // 6
		{{2,1,5}, {4, 3, 6}}, // 7
	};

	for (int i = 0; i < 6; i++)
		roam_point_update_height(vertexes[i]);
	for (int i = 0; i < 8; i++)
		sphere->roots[i] = roam_triangle_new(
			vertexes[_triangles[i][0][0]],
			vertexes[_triangles[i][0][1]],
			vertexes[_triangles[i][0][2]]);
	for (int i = 0; i < 8; i++)
		roam_triangle_add(sphere->roots[i],
			sphere->roots[_triangles[i][1][0]],
			sphere->roots[_triangles[i][1][1]],
			sphere->roots[_triangles[i][1][2]],
			sphere);
	for (int i = 0; i < 8; i++)
		g_debug("RoamSphere: new - %p edge=%f,%f,%f,%f", sphere->roots[i],
				sphere->roots[i]->edge.n,
				sphere->roots[i]->edge.s,
				sphere->roots[i]->edge.e,
				sphere->roots[i]->edge.w);

	return sphere;
}
void roam_sphere_update_view(RoamSphere *sphere)
{
	if (!sphere->view)
		sphere->view = g_new0(RoamView, 1);
	glGetDoublev (GL_MODELVIEW_MATRIX,  sphere->view->model);
	glGetDoublev (GL_PROJECTION_MATRIX, sphere->view->proj);
	glGetIntegerv(GL_VIEWPORT,          sphere->view->view);
	sphere->view->version++;
}
void roam_sphere_update_errors(RoamSphere *sphere)
{
	g_debug("RoamSphere: update_errors - polys=%d", sphere->polys);
	GPtrArray *tris = g_pqueue_get_array(sphere->triangles);
	GPtrArray *dias = g_pqueue_get_array(sphere->diamonds);

	roam_sphere_update_view(sphere);

	for (int i = 0; i < tris->len; i++) {
		RoamTriangle *triangle = tris->pdata[i];
		roam_triangle_update_errors(triangle, sphere);
		g_pqueue_priority_changed(sphere->triangles, triangle->handle);
	}

	for (int i = 0; i < dias->len; i++) {
		RoamDiamond *diamond = dias->pdata[i];
		roam_diamond_update_errors(diamond, sphere);
		g_pqueue_priority_changed(sphere->diamonds, diamond->handle);
	}

	g_ptr_array_free(tris, TRUE);
	g_ptr_array_free(dias, TRUE);
}

void roam_sphere_split_one(RoamSphere *sphere)
{
	RoamTriangle *to_split = g_pqueue_peek(sphere->triangles);
	if (!to_split) return;
	roam_triangle_split(to_split, sphere);
}
void roam_sphere_merge_one(RoamSphere *sphere)
{
	RoamDiamond *to_merge = g_pqueue_peek(sphere->diamonds);
	if (!to_merge) return;
	roam_diamond_merge(to_merge, sphere);
}
gint roam_sphere_split_merge(RoamSphere *sphere)
{
	gint iters = 0, max_iters = 500;
	//gint target = 4000;
	gint target = 2000;
	//gint target = 500;

	if (!sphere->view)
		return 0;

	if (target - sphere->polys > 100) {
		//g_debug("RoamSphere: split_merge - Splitting %d - %d > 100", target, sphere->polys);
		while (sphere->polys < target && iters++ < max_iters)
			roam_sphere_split_one(sphere);
	}

	if (sphere->polys - target > 100) {
		//g_debug("RoamSphere: split_merge - Merging %d - %d > 100", sphere->polys, target);
		while (sphere->polys > target && iters++ < max_iters)
			roam_sphere_merge_one(sphere);
	}

	while (((RoamTriangle*)g_pqueue_peek(sphere->triangles))->error >
	       ((RoamDiamond *)g_pqueue_peek(sphere->diamonds ))->error &&
	       iters++ < max_iters) {
		//g_debug("RoamSphere: split_merge - Fixing 1 %f > %f && %d < %d",
		//		((RoamTriangle*)g_pqueue_peek(sphere->triangles))->error,
		//		((RoamDiamond *)g_pqueue_peek(sphere->diamonds ))->error,
		//		iters-1, max_iters);
		roam_sphere_merge_one(sphere);
		roam_sphere_split_one(sphere);
	}

	return iters;
}
void roam_sphere_draw(RoamSphere *sphere)
{
	g_debug("RoamSphere: draw");
	g_pqueue_foreach(sphere->triangles, (GFunc)roam_triangle_draw, NULL);
}
void roam_sphere_draw_normals(RoamSphere *sphere)
{
	g_debug("RoamSphere: draw_normal");
	g_pqueue_foreach(sphere->triangles, (GFunc)roam_triangle_draw_normal, NULL);
}
static GList *_roam_sphere_get_leaves(RoamTriangle *triangle, GList *list, gboolean all)
{
	if (triangle->kids[0] && triangle->kids[1]) {
		if (all) list = g_list_prepend(list, triangle);
		list = _roam_sphere_get_leaves(triangle->kids[0], list, all);
		list = _roam_sphere_get_leaves(triangle->kids[1], list, all);
		return list;
	} else {
		return g_list_prepend(list, triangle);
	}
}
static GList *_roam_sphere_get_intersect_rec(RoamTriangle *triangle, GList *list,
		gboolean all, gdouble n, gdouble s, gdouble e, gdouble w)
{
	gdouble tn = triangle->edge.n;
	gdouble ts = triangle->edge.s;
	gdouble te = triangle->edge.e;
	gdouble tw = triangle->edge.w;

	gboolean debug = FALSE  &&
		n==90 && s==45 && e==-90 && w==-180 &&
		ts > 44 && ts < 46;

	if (debug)
		g_message("t=%p: %f < %f || %f > %f || %f < %f || %f > %f",
		            triangle, tn,   s,   ts,   n,   te,   w,   tw,   e);
	if (tn < s || ts > n || te < w || tw > e) {
		/* No intersect */
		if (debug) g_message("no intersect");
		return list;
	} else if (tn <= n && ts >= s && te <= e && tw >= w) {
		/* Triangle is completely contained */
		if (debug) g_message("contained");
		if (all) list = g_list_prepend(list, triangle);
		return _roam_sphere_get_leaves(triangle, list, all);
	} else if (triangle->kids[0] && triangle->kids[1]) {
		/* Paritial intersect with children */
		if (debug) g_message("edge w/ child");
		if (all) list = g_list_prepend(list, triangle);
		list = _roam_sphere_get_intersect_rec(triangle->kids[0], list, all, n, s, e, w);
		list = _roam_sphere_get_intersect_rec(triangle->kids[1], list, all, n, s, e, w);
		return list;
	} else {
		/* This triangle is an edge case */
		if (debug) g_message("edge");
		return g_list_prepend(list, triangle);
	}
}
/* Warning: This grabs pointers to triangles which can be changed by other
 * calls, e.g. split_merge. If you use this, you need to do some locking to
 * prevent the returned list from becomming stale. */
GList *roam_sphere_get_intersect(RoamSphere *sphere, gboolean all,
		gdouble n, gdouble s, gdouble e, gdouble w)
{
	/* I think this is correct..
	 * i_cost = cost for triangle-rectagnle intersect test
	 * time = n_tiles * 2*tris_per_tile * i_cost
	 * time = 30      * 2*333           * i_cost = 20000 * i_cost */
	GList *list = NULL;
	for (int i = 0; i < G_N_ELEMENTS(sphere->roots); i++)
		list = _roam_sphere_get_intersect_rec(sphere->roots[i],
				list, all, n, s, e, w);
	return list;
}
void roam_sphere_free_tri(RoamTriangle *triangle)
{
	if (--triangle->p.l->tris == 0) g_free(triangle->p.l);
	if (--triangle->p.m->tris == 0) g_free(triangle->p.m);
	if (--triangle->p.r->tris == 0) g_free(triangle->p.r);
	roam_triangle_free(triangle);
}
void roam_sphere_free(RoamSphere *sphere)
{
	g_debug("RoamSphere: free");
	/* Slow method, but it should work */
	while (sphere->polys > 8)
		roam_sphere_merge_one(sphere);
	/* TODO: free points */
	g_pqueue_foreach(sphere->triangles, (GFunc)roam_sphere_free_tri, NULL);
	g_pqueue_free(sphere->triangles);
	g_pqueue_free(sphere->diamonds);
	g_free(sphere->view);
	g_free(sphere);
}
