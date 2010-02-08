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

#ifndef __ROAM_H__
#define __ROAM_H__

#include "gpqueue.h"

/* Roam */
typedef struct _RoamView     RoamView;
typedef struct _RoamPoint    RoamPoint;
typedef struct _RoamTriangle RoamTriangle;
typedef struct _RoamDiamond  RoamDiamond;
typedef struct _RoamSphere   RoamSphere;
typedef gdouble (*RoamHeightFunc)(gdouble lat, gdouble lon, gpointer user_data);

/* Misc */
struct _RoamView {
	gdouble model[16];
	gdouble proj[16];
	gint view[4];
	gint version;
};

/*************
 * RoamPoint *
 *************/
struct _RoamPoint {
	/*< private >*/
	gdouble  x, y, z;    /* Model coordinates */
	gdouble  px, py, pz; /* Projected coordinates */
	gint     pversion;   /* Version of cached projection */

	gint     tris;       /* Count of associated triangles */
	gdouble  norm[3];    /* Vertex normal */

	/* For get_intersect */
	gdouble  lat, lon, elev;

	/* For terrain */
	RoamHeightFunc height_func;
	gpointer       height_data;
};
RoamPoint *roam_point_new(double lat, double lon, double elev);
void roam_point_add_triangle(RoamPoint *point, RoamTriangle *triangle);
void roam_point_remove_triangle(RoamPoint *point, RoamTriangle *triangle);
void roam_point_update_height(RoamPoint *point);
void roam_point_update_projection(RoamPoint *point, RoamView *view);

/****************
 * RoamTriangle *
 ****************/
struct _RoamTriangle {
	/*< private >*/
	/* Left, middle and right vertices */
	struct { RoamPoint    *l,*m,*r; } p;

	/* Left, base, and right neighbor triangles */
	struct { RoamTriangle *l,*b,*r; } t;

	RoamPoint *split;     /* Split point */
	RoamDiamond *parent;  /* Parent diamond */
	double norm[3];       /* Surface normal */
	double error;         /* Screen space error */
	GPQueueHandle handle;

	/* For get_intersect */
	struct { gdouble n,s,e,w; } edge;
	RoamTriangle *kids[2];
};
RoamTriangle *roam_triangle_new(RoamPoint *l, RoamPoint *m, RoamPoint *r);
void roam_triangle_free(RoamTriangle *triangle);
void roam_triangle_add(RoamTriangle *triangle,
		RoamTriangle *left, RoamTriangle *base, RoamTriangle *right,
		RoamSphere *sphere);
void roam_triangle_remove(RoamTriangle *triangle, RoamSphere *sphere);
void roam_triangle_update_errors(RoamTriangle *triangle, RoamSphere *sphere);
void roam_triangle_split(RoamTriangle *triangle, RoamSphere *sphere);
void roam_triangle_draw(RoamTriangle *triangle);
void roam_triangle_draw_normal(RoamTriangle *triangle);

/***************
 * RoamDiamond *
 ***************/
struct _RoamDiamond {
	/*< private >*/
	RoamTriangle *kids[4];    /* Child triangles */
	RoamTriangle *parents[2]; /* Parent triangles */
	double error;             /* Screen space error */
	gboolean active;          /* For internal use */
	GPQueueHandle handle;
};
RoamDiamond *roam_diamond_new(
		RoamTriangle *parent0, RoamTriangle *parent1,
		RoamTriangle *kid0, RoamTriangle *kid1,
		RoamTriangle *kid2, RoamTriangle *kid3);
void roam_diamond_add(RoamDiamond *diamond, RoamSphere *sphere);
void roam_diamond_remove(RoamDiamond *diamond, RoamSphere *sphere);
void roam_diamond_merge(RoamDiamond *diamond, RoamSphere *sphere);
void roam_diamond_update_errors(RoamDiamond *diamond, RoamSphere *sphere);

/**************
 * RoamSphere *
 **************/
struct _RoamSphere {
	/*< private >*/
	GPQueue *triangles; /* List of triangles */
	GPQueue *diamonds;  /* List of diamonds */
	RoamView *view;     /* Current projection */
	gint polys;         /* Polygon count */

	/* For get_intersect */
	RoamTriangle *roots[8]; /* Original 8 triangles */
};
RoamSphere *roam_sphere_new();
void roam_sphere_update_view(RoamSphere *sphere);
void roam_sphere_update_errors(RoamSphere *sphere);
void roam_sphere_split_one(RoamSphere *sphere);
void roam_sphere_merge_one(RoamSphere *sphere);
gint roam_sphere_split_merge(RoamSphere *sphere);
void roam_sphere_draw(RoamSphere *sphere);
void roam_sphere_draw_normals(RoamSphere *sphere);
GList *roam_sphere_get_intersect(RoamSphere *sphere, gboolean all,
		gdouble n, gdouble s, gdouble e, gdouble w);
void roam_sphere_free(RoamSphere *sphere);

#endif
