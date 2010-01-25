/*
 * Copyright (C) 2009 Andy Spencer <spenceal@rose-hulman.edu>
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
	RoamPoint *self = g_new0(RoamPoint, 1);
	self->lat  = lat;
	self->lon  = lon;
	self->elev = elev;
	/* For get_intersect */
	lle2xyz(lat, lon, elev, &self->x, &self->y, &self->z);
	return self;
}
RoamPoint *roam_point_dup(RoamPoint *self)
{
	RoamPoint *new = g_memdup(self, sizeof(RoamPoint));
	new->tris = 0;
	return new;
}
void roam_point_add_triangle(RoamPoint *self, RoamTriangle *triangle)
{
	for (int i = 0; i < 3; i++) {
		self->norm[i] *= self->tris;
		self->norm[i] += triangle->norm[i];
	}
	self->tris++;
	for (int i = 0; i < 3; i++)
		self->norm[i] /= self->tris;
}
void roam_point_remove_triangle(RoamPoint *self, RoamTriangle *triangle)
{
	for (int i = 0; i < 3; i++) {
		self->norm[i] *= self->tris;
		self->norm[i] -= triangle->norm[i];
	}
	self->tris--;
	if (self->tris)
		for (int i = 0; i < 3; i++)
			self->norm[i] /= self->tris;
}
void roam_point_update_height(RoamPoint *self)
{
	if (self->height_func) {
		gdouble elev = self->height_func(
				self->lat, self->lon, self->height_data);
		lle2xyz(self->lat, self->lon, elev,
				&self->x, &self->y, &self->z);
	}
}
void roam_point_update_projection(RoamPoint *self, RoamSphere *sphere)
{
	static int count   = 0;
	static int version = 0;
	if (version != sphere->view->version) {
		g_debug("Projected %d points", count);
		count   = 0;
		version = sphere->view->version;
	}

	if (self->pversion != sphere->view->version) {
		/* Cache projection */
		gluProject(self->x, self->y, self->z,
			sphere->view->model, sphere->view->proj, sphere->view->view,
			&self->px, &self->py, &self->pz);
		self->pversion = sphere->view->version;
		count++;
	}
}

/****************
 * RoamTriangle *
 ****************/
RoamTriangle *roam_triangle_new(RoamPoint *l, RoamPoint *m, RoamPoint *r)
{
	RoamTriangle *self = g_new0(RoamTriangle, 1);

	self->error = 0;
	self->p.l = l;
	self->p.m = m;
	self->p.r = r;
	self->split = roam_point_new(
		(l->lat + r->lat)/2,
		(ABS(l->lat) == 90 ? r->lon :
		 ABS(r->lat) == 90 ? l->lon :
		 lon_avg(l->lon, r->lon)),
		(l->elev + r->elev)/2);
	/* TODO: Move this back to sphere, or actually use the nesting */
	self->split->height_func = m->height_func;
	self->split->height_data = m->height_data;
	roam_point_update_height(self->split);
	//if ((float)self->split->lat > 44 && (float)self->split->lat < 46)
	//	g_debug("RoamTriangle: new - (l,m,r,split).lats = %7.2f %7.2f %7.2f %7.2f",
	//			l->lat, m->lat, r->lat, self->split->lat);
	//if ((float)l->lat == (float)r->lat &&
	//    (float)self->split->lat != (float)l->lat)
	//	g_warning("RoamTriangle: new - split.lat=%f != (l=r).lat=%f",
	//		self->split->lat, l->lat);

	/* Update normal */
	double pa[3];
	double pb[3];
	pa[0] = self->p.l->x - self->p.m->x;
	pa[1] = self->p.l->y - self->p.m->y;
	pa[2] = self->p.l->z - self->p.m->z;

	pb[0] = self->p.r->x - self->p.m->x;
	pb[1] = self->p.r->y - self->p.m->y;
	pb[2] = self->p.r->z - self->p.m->z;

	self->norm[0] = pa[1] * pb[2] - pa[2] * pb[1];
	self->norm[1] = pa[2] * pb[0] - pa[0] * pb[2];
	self->norm[2] = pa[0] * pb[1] - pa[1] * pb[0];

	double total = sqrt(self->norm[0] * self->norm[0] +
	                    self->norm[1] * self->norm[1] +
	                    self->norm[2] * self->norm[2]);

	self->norm[0] /= total;
	self->norm[1] /= total;
	self->norm[2] /= total;

	/* Store bounding box, for get_intersect */
	RoamPoint *p[] = {l,m,r};
	self->edge.n =  -90; self->edge.s =  90;
	self->edge.e = -180; self->edge.w = 180;
	gboolean maxed = FALSE;
	for (int i = 0; i < G_N_ELEMENTS(p); i++) {
		self->edge.n = MAX(self->edge.n, p[i]->lat);
		self->edge.s = MIN(self->edge.s, p[i]->lat);
		if (p[i]->lat == 90 || p[i]->lat == -90)
			continue;
		if (p[i]->lon == 180) {
			maxed = TRUE;
			continue;
		}
		self->edge.e = MAX(self->edge.e, p[i]->lon);
		self->edge.w = MIN(self->edge.w, p[i]->lon);
	}
	if (maxed) {
		if (self->edge.e < 0)
			self->edge.w = -180;
		else
			self->edge.e =  180;
	}

	//g_message("roam_triangle_new: %p", self);
	return self;
}

void roam_triangle_free(RoamTriangle *self)
{
	g_free(self->split);
	g_free(self);
}

void roam_triangle_add(RoamTriangle *self,
		RoamTriangle *left, RoamTriangle *base, RoamTriangle *right,
		RoamSphere *sphere)
{
	self->t.l = left;
	self->t.b = base;
	self->t.r = right;

	roam_point_add_triangle(self->p.l, self);
	roam_point_add_triangle(self->p.m, self);
	roam_point_add_triangle(self->p.r, self);

	if (sphere->view)
		roam_triangle_update_errors(self, sphere);

	self->handle = g_pqueue_push(sphere->triangles, self);
}

void roam_triangle_remove(RoamTriangle *self, RoamSphere *sphere)
{
	/* Update vertex normals */
	roam_point_remove_triangle(self->p.l, self);
	roam_point_remove_triangle(self->p.m, self);
	roam_point_remove_triangle(self->p.r, self);

	g_pqueue_remove(sphere->triangles, self->handle);
}

void roam_triangle_sync_neighbors(RoamTriangle *new, RoamTriangle *old, RoamTriangle *neigh)
{
	if      (neigh->t.l == old) neigh->t.l = new;
	else if (neigh->t.b == old) neigh->t.b = new;
	else if (neigh->t.r == old) neigh->t.r = new;
	else g_assert_not_reached();
}

gboolean roam_point_visible(RoamPoint *self, RoamSphere *sphere)
{
	gint *view = sphere->view->view;
	return self->px > view[0] && self->px < view[2] &&
	       self->py > view[1] && self->py < view[3] &&
	       self->pz > 0       && self->pz < 1;
}
gboolean roam_triangle_visible(RoamTriangle *self, RoamSphere *sphere)
{
	/* Do this with a bounding box */
	return roam_point_visible(self->p.l, sphere) ||
	       roam_point_visible(self->p.m, sphere) ||
	       roam_point_visible(self->p.r, sphere);
}

void roam_triangle_update_errors(RoamTriangle *self, RoamSphere *sphere)
{
	/* Update points */
	roam_point_update_projection(self->p.l, sphere);
	roam_point_update_projection(self->p.m, sphere);
	roam_point_update_projection(self->p.r, sphere);

	/* Not exactly correct, could be out on both sides (middle in) */
	if (!roam_triangle_visible(self, sphere)) {
		self->error = -1;
	} else {
		roam_point_update_projection(self->split, sphere);
		RoamPoint *l = self->p.l;
		RoamPoint *m = self->p.m;
		RoamPoint *r = self->p.r;
		RoamPoint *split = self->split;

		/*               l-r midpoint        projected l-r midpoint */
		gdouble pxdist = (l->px + r->px)/2 - split->px;
		gdouble pydist = (l->py + r->py)/2 - split->py;

		self->error = sqrt(pxdist*pxdist + pydist*pydist);

		/* Multiply by size of triangle */
		double size = -( l->px * (m->py - r->py) +
		                 m->px * (r->py - l->py) +
		                 r->px * (l->py - m->py) ) / 2.0;

		/* Size < 0 == backface */
		//if (size < 0)
		//	self->error *= -1;
		self->error *= size;
	}
}

void roam_triangle_split(RoamTriangle *self, RoamSphere *sphere)
{
	//g_message("roam_triangle_split: %p, e=%f\n", self, self->error);

	sphere->polys += 2;

	if (self != self->t.b->t.b)
		roam_triangle_split(self->t.b, sphere);
	if (self != self->t.b->t.b)
		g_assert_not_reached();

	RoamTriangle *base = self->t.b;

	/* Add new triangles */
	RoamPoint *mid = self->split;
	RoamTriangle *sl = self->kids[0] = roam_triangle_new(self->p.m, mid, self->p.l); // Self Left
	RoamTriangle *sr = self->kids[1] = roam_triangle_new(self->p.r, mid, self->p.m); // Self Right
	RoamTriangle *bl = base->kids[0] = roam_triangle_new(base->p.m, mid, base->p.l); // Base Left
	RoamTriangle *br = base->kids[1] = roam_triangle_new(base->p.r, mid, base->p.m); // Base Right

	/*                tri,l,  base,      r,  sphere */
	roam_triangle_add(sl, sr, self->t.l, br, sphere);
	roam_triangle_add(sr, bl, self->t.r, sl, sphere);
	roam_triangle_add(bl, br, base->t.l, sr, sphere);
	roam_triangle_add(br, sl, base->t.r, bl, sphere);

	roam_triangle_sync_neighbors(sl, self, self->t.l);
	roam_triangle_sync_neighbors(sr, self, self->t.r);
	roam_triangle_sync_neighbors(bl, base, base->t.l);
	roam_triangle_sync_neighbors(br, base, base->t.r);

	/* Remove old triangles */
	roam_triangle_remove(self, sphere);
	roam_triangle_remove(base, sphere);

	/* Add/Remove diamonds */
	RoamDiamond *diamond = roam_diamond_new(self, base, sl, sr, bl, br);
	roam_diamond_update_errors(diamond, sphere);
	roam_diamond_add(diamond, sphere);
	roam_diamond_remove(self->parent, sphere);
	roam_diamond_remove(base->parent, sphere);
}

void roam_triangle_draw(RoamTriangle *self)
{
	glBegin(GL_TRIANGLES);
	glNormal3dv(self->p.r->norm); glVertex3dv((double*)self->p.r);
	glNormal3dv(self->p.m->norm); glVertex3dv((double*)self->p.m);
	glNormal3dv(self->p.l->norm); glVertex3dv((double*)self->p.l);
	glEnd();
	return;
}

void roam_triangle_draw_normal(RoamTriangle *self)
{
	double center[] = {
		(self->p.l->x + self->p.m->x + self->p.r->x)/3.0,
		(self->p.l->y + self->p.m->y + self->p.r->y)/3.0,
		(self->p.l->z + self->p.m->z + self->p.r->z)/3.0,
	};
	double end[] = {
		center[0]+self->norm[0]*2000000,
		center[1]+self->norm[1]*2000000,
		center[2]+self->norm[2]*2000000,
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
	RoamDiamond *self = g_new0(RoamDiamond, 1);

	self->kids[0] = kid0;
	self->kids[1] = kid1;
	self->kids[2] = kid2;
	self->kids[3] = kid3;

	kid0->parent = self;
	kid1->parent = self;
	kid2->parent = self;
	kid3->parent = self;

	self->parents[0] = parent0;
	self->parents[1] = parent1;

	return self;
}
void roam_diamond_add(RoamDiamond *self, RoamSphere *sphere)
{
	self->active = TRUE;
	self->error  = MAX(self->parents[0]->error, self->parents[1]->error);
	self->handle = g_pqueue_push(sphere->diamonds, self);
}
void roam_diamond_remove(RoamDiamond *self, RoamSphere *sphere)
{
	if (self && self->active) {
		self->active = FALSE;
		g_pqueue_remove(sphere->diamonds, self->handle);
	}
}
void roam_diamond_merge(RoamDiamond *self, RoamSphere *sphere)
{
	//g_message("roam_diamond_merge: %p, e=%f\n", self, self->error);

	sphere->polys -= 2;

	/* TODO: pick the best split */
	RoamTriangle **kids = self->kids;

	/* Create triangles */
	RoamTriangle *tri  = self->parents[0];
	RoamTriangle *base = self->parents[1];

	roam_triangle_add(tri,  kids[0]->t.b, base, kids[1]->t.b, sphere);
	roam_triangle_add(base, kids[2]->t.b, tri,  kids[3]->t.b, sphere);

	roam_triangle_sync_neighbors(tri,  kids[0], kids[0]->t.b);
	roam_triangle_sync_neighbors(tri,  kids[1], kids[1]->t.b);
	roam_triangle_sync_neighbors(base, kids[2], kids[2]->t.b);
	roam_triangle_sync_neighbors(base, kids[3], kids[3]->t.b);

	/* Remove triangles */
	roam_triangle_remove(kids[0], sphere);
	roam_triangle_remove(kids[1], sphere);
	roam_triangle_remove(kids[2], sphere);
	roam_triangle_remove(kids[3], sphere);

	/* Clear kids */
	tri->kids[0]  = tri->kids[1]  = NULL;
	base->kids[0] = base->kids[1] = NULL;

	/* Add/Remove triangles */
	if (tri->t.l->t.l == tri->t.r->t.r &&
	    tri->t.l->t.l != tri && tri->parent) {
		roam_diamond_update_errors(tri->parent, sphere);
		roam_diamond_add(tri->parent, sphere);
	}

	if (base->t.l->t.l == base->t.r->t.r &&
	    base->t.l->t.l != base && base->parent) {
		roam_diamond_update_errors(base->parent, sphere);
		roam_diamond_add(base->parent, sphere);
	}

	/* Remove and free diamond and child triangles */
	roam_diamond_remove(self, sphere);
	g_assert(self->kids[0]->p.m == self->kids[1]->p.m &&
	         self->kids[1]->p.m == self->kids[2]->p.m &&
	         self->kids[2]->p.m == self->kids[3]->p.m);
	g_assert(self->kids[0]->p.m->tris == 0);
	roam_triangle_free(self->kids[0]);
	roam_triangle_free(self->kids[1]);
	roam_triangle_free(self->kids[2]);
	roam_triangle_free(self->kids[3]);
	g_free(self);
}
void roam_diamond_update_errors(RoamDiamond *self, RoamSphere *sphere)
{
	roam_triangle_update_errors(self->parents[0], sphere);
	roam_triangle_update_errors(self->parents[1], sphere);
	self->error = MAX(self->parents[0]->error, self->parents[1]->error);
}

/**************
 * RoamSphere *
 **************/
RoamSphere *roam_sphere_new()
{
	RoamSphere *self = g_new0(RoamSphere, 1);
	self->polys       = 8;
	self->triangles   = g_pqueue_new((GCompareDataFunc)tri_cmp, NULL);
	self->diamonds    = g_pqueue_new((GCompareDataFunc)dia_cmp, NULL);

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
		self->roots[i] = roam_triangle_new(
			vertexes[_triangles[i][0][0]],
			vertexes[_triangles[i][0][1]],
			vertexes[_triangles[i][0][2]]);
	for (int i = 0; i < 8; i++)
		roam_triangle_add(self->roots[i],
			self->roots[_triangles[i][1][0]],
			self->roots[_triangles[i][1][1]],
			self->roots[_triangles[i][1][2]],
			self);
	for (int i = 0; i < 8; i++)
		g_debug("RoamSphere: new - %p edge=%f,%f,%f,%f", self->roots[i],
				self->roots[i]->edge.n,
				self->roots[i]->edge.s,
				self->roots[i]->edge.e,
				self->roots[i]->edge.w);

	return self;
}
void roam_sphere_update_view(RoamSphere *self)
{
	if (!self->view)
		self->view = g_new0(RoamView, 1);
	glGetDoublev (GL_MODELVIEW_MATRIX,  self->view->model);
	glGetDoublev (GL_PROJECTION_MATRIX, self->view->proj);
	glGetIntegerv(GL_VIEWPORT,          self->view->view);
	self->view->version++;
}
void roam_sphere_update_errors(RoamSphere *self)
{
	g_debug("RoamSphere: update_errors - polys=%d", self->polys);
	GPtrArray *tris = g_pqueue_get_array(self->triangles);
	GPtrArray *dias = g_pqueue_get_array(self->diamonds);

	roam_sphere_update_view(self);

	for (int i = 0; i < tris->len; i++) {
		RoamTriangle *tri = tris->pdata[i];
		roam_triangle_update_errors(tri, self);
		g_pqueue_priority_changed(self->triangles, tri->handle);
	}

	for (int i = 0; i < dias->len; i++) {
		RoamDiamond *dia = dias->pdata[i];
		roam_diamond_update_errors(dia, self);
		g_pqueue_priority_changed(self->diamonds, dia->handle);
	}

	g_ptr_array_free(tris, TRUE);
	g_ptr_array_free(dias, TRUE);
}

void roam_sphere_split_one(RoamSphere *self)
{
	RoamTriangle *to_split = g_pqueue_peek(self->triangles);
	if (!to_split) return;
	roam_triangle_split(to_split, self);
}
void roam_sphere_merge_one(RoamSphere *self)
{
	RoamDiamond *to_merge = g_pqueue_peek(self->diamonds);
	if (!to_merge) return;
	roam_diamond_merge(to_merge, self);
}
gint roam_sphere_split_merge(RoamSphere *self)
{
	gint iters = 0, max_iters = 500;
	//gint target = 4000;
	gint target = 2000;
	//gint target = 500;

	if (!self->view)
		return 0;

	if (target - self->polys > 100) {
		//g_debug("RoamSphere: split_merge - Splitting %d - %d > 100", target, self->polys);
		while (self->polys < target && iters++ < max_iters)
			roam_sphere_split_one(self);
	}

	if (self->polys - target > 100) {
		//g_debug("RoamSphere: split_merge - Merging %d - %d > 100", self->polys, target);
		while (self->polys > target && iters++ < max_iters)
			roam_sphere_merge_one(self);
	}

	while (((RoamTriangle*)g_pqueue_peek(self->triangles))->error >
	       ((RoamDiamond *)g_pqueue_peek(self->diamonds ))->error &&
	       iters++ < max_iters) {
		//g_debug("RoamSphere: split_merge - Fixing 1 %f > %f && %d < %d",
		//		((RoamTriangle*)g_pqueue_peek(self->triangles))->error,
		//		((RoamDiamond *)g_pqueue_peek(self->diamonds ))->error,
		//		iters-1, max_iters);
		roam_sphere_merge_one(self);
		roam_sphere_split_one(self);
	}

	return iters;
}
void roam_sphere_draw(RoamSphere *self)
{
	g_debug("RoamSphere: draw");
	g_pqueue_foreach(self->triangles, (GFunc)roam_triangle_draw, NULL);
}
void roam_sphere_draw_normals(RoamSphere *self)
{
	g_debug("RoamSphere: draw_normal");
	g_pqueue_foreach(self->triangles, (GFunc)roam_triangle_draw_normal, NULL);
}
static GList *_roam_sphere_get_leaves(RoamTriangle *tri, GList *list, gboolean all)
{
	if (tri->kids[0] && tri->kids[1]) {
		if (all) list = g_list_prepend(list, tri);
		list = _roam_sphere_get_leaves(tri->kids[0], list, all);
		list = _roam_sphere_get_leaves(tri->kids[1], list, all);
		return list;
	} else {
		return g_list_prepend(list, tri);
	}
}
static GList *_roam_sphere_get_intersect_rec(RoamTriangle *tri, GList *list,
		gboolean all, gdouble n, gdouble s, gdouble e, gdouble w)
{
	gdouble tn = tri->edge.n;
	gdouble ts = tri->edge.s;
	gdouble te = tri->edge.e;
	gdouble tw = tri->edge.w;

	gboolean debug = FALSE  &&
		n==90 && s==45 && e==-90 && w==-180 &&
		ts > 44 && ts < 46;

	if (debug)
		g_message("t=%p: %f < %f || %f > %f || %f < %f || %f > %f",
		            tri, tn,   s,   ts,   n,   te,   w,   tw,   e);
	if (tn < s || ts > n || te < w || tw > e) {
		/* No intersect */
		if (debug) g_message("no intersect");
		return list;
	} else if (tn <= n && ts >= s && te <= e && tw >= w) {
		/* Triangle is completely contained */
		if (debug) g_message("contained");
		if (all) list = g_list_prepend(list, tri);
		return _roam_sphere_get_leaves(tri, list, all);
	} else if (tri->kids[0] && tri->kids[1]) {
		/* Paritial intersect with children */
		if (debug) g_message("edge w/ child");
		if (all) list = g_list_prepend(list, tri);
		list = _roam_sphere_get_intersect_rec(tri->kids[0], list, all, n, s, e, w);
		list = _roam_sphere_get_intersect_rec(tri->kids[1], list, all, n, s, e, w);
		return list;
	} else {
		/* This triangle is an edge case */
		if (debug) g_message("edge");
		return g_list_prepend(list, tri);
	}
}
/* Warning: This grabs pointers to triangles which can be changed by other
 * calls, e.g. split_merge. If you use this, you need to do some locking to
 * prevent the returned list from becomming stale. */
GList *roam_sphere_get_intersect(RoamSphere *self, gboolean all,
		gdouble n, gdouble s, gdouble e, gdouble w)
{
	/* I think this is correct..
	 * i_cost = cost for triangle-rectagnle intersect test
	 * time = n_tiles * 2*tris_per_tile * i_cost
	 * time = 30      * 2*333           * i_cost = 20000 * i_cost */
	GList *list = NULL;
	for (int i = 0; i < G_N_ELEMENTS(self->roots); i++)
		list = _roam_sphere_get_intersect_rec(self->roots[i],
				list, all, n, s, e, w);
	return list;
}
void roam_sphere_free_tri(RoamTriangle *tri)
{
	if (--tri->p.l->tris == 0) g_free(tri->p.l);
	if (--tri->p.m->tris == 0) g_free(tri->p.m);
	if (--tri->p.r->tris == 0) g_free(tri->p.r);
	roam_triangle_free(tri);
}
void roam_sphere_free(RoamSphere *self)
{
	/* Slow method, but it should work */
	while (self->polys > 8)
		roam_sphere_merge_one(self);
	/* TODO: free points */
	g_pqueue_foreach(self->triangles, (GFunc)roam_sphere_free_tri, NULL);
	g_pqueue_free(self->triangles);
	g_pqueue_free(self->diamonds);
	g_free(self->view);
	g_free(self);
}
