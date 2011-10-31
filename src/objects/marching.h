/*
 * Copyright (C) 2002 Jamie Zawinski <jwz@jwz.org>
 * Copyright (C) 2009-2011 Andy Spencer <andy753421@gmail.com>
 *
 * Marching cubes implementation based on code from from the xscreensaver
 * package.
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

#ifndef __VOLUME_H__
#define __VOLUME_H__

#include <glib.h>

/* VolCoord */
typedef struct {
	double x, y, z;
} VolCoord;

/* VolPoint */
typedef struct {
	VolCoord c;     /* Coordinate of the point */
	double   value; /* Function value at point */
} VolPoint;

/* VolVertex */
typedef struct {
	VolCoord c;    /* Coordinate of the vertex */
	gint     tris; /* Count of associated triangles */
	VolCoord norm; /* Vertex normal */
} VolVertex;

/* VolTriangle */
typedef struct {
	VolVertex *v[3]; /* Vertices */
	VolCoord   norm; /* Surface normal */
} VolTriangle;

/* VolCell */
typedef struct {
	VolPoint  *corner[8]; /* Corners */
	VolVertex *edge[12];  /* Points along the edges (for caching) */
} VolCell;

/* VolGrid */
typedef struct {
	int xs, ys, zs; /* Dimensions of points */
	gpointer data;  /* 3-D grid of points */
} VolGrid;

/* Find triangles on an isosurface that pass through the cell */
GList *march_one_cube(VolCell *cell, double level);

/* Find triangles along an isosurface in a grid of points */
GList *marching_cubes(VolGrid *grid, double level);

/* Free/unref data for triangle */
void vol_triangle_free(VolTriangle *tri);

/* Grid functions */
static inline VolPoint *vol_grid_get(VolGrid *grid, int x, int y, int z)
{
	VolPoint (*points)[grid->ys][grid->zs] = grid->data;
	return &points[x][y][z];
}
VolGrid *vol_grid_new(int xs, int ys, int zs);
void vol_grid_free(VolGrid *grid);


#endif
