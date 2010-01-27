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

#ifndef __GIS_TILE_H__
#define __GIS_TILE_H__

#include <glib.h>

typedef struct _GisTile        GisTile;

#define gis_tile_foreach(tile, child) \
	for (int _x = 0; _x < G_N_ELEMENTS(tile->children); _x++) \
	for (int _y = 0; child = tile->children[_x][_y], \
		_y < G_N_ELEMENTS(tile->children[_x]); _y++) \

#define gis_tile_foreach_index(tile, x, y) \
	for (x = 0; x < G_N_ELEMENTS(tile->children); x++) \
	for (y = 0; y < G_N_ELEMENTS(tile->children[x]); y++)

typedef void (*GisTileLoadFunc)(GisTile *tile, gpointer user_data);
typedef void (*GisTileFreeFunc)(GisTile *tile, gpointer user_data);

struct _GisTile {
	/* Pointer to the tile data */
	gpointer data;

	/* North,South,East,West limits */
	struct {
		gdouble n,s,e,w;
	} edge;

	/* Pointers to parent/child nodes */
	GisTile *parent;
	GisTile *children[2][2];

	/* Last access time (for garbage collection) */
	time_t atime;
};

/* Path to string table, keep in sync with tile->children */ 
extern gchar *gis_tile_path_table[2][2];

/* Allocate a new Tile */
GisTile *gis_tile_new(GisTile *parent,
	gdouble n, gdouble s, gdouble e, gdouble w);

/* Return a string representation of the tile's path */
gchar *gis_tile_get_path(GisTile *child);

/* Update a root tile */
/* Based on eye distance (lat,lon,elev) */
void gis_tile_update(GisTile *root,
		gdouble res, gint width, gint height,
		gdouble lat, gdouble lon, gdouble elev,
		GisTileLoadFunc load_func, gpointer user_data);

/* Find the leaf tile containing lat-lon */
GisTile *gis_tile_find(GisTile *root, gdouble lat, gdouble lon);

/* Delete nodes that haven't been accessed since atime */
GisTile *gis_tile_gc(GisTile *root, time_t atime,
		GisTileFreeFunc free_func, gpointer user_data);

/* Free a tile and all it's children */
void gis_tile_free(GisTile *root,
		GisTileFreeFunc free_func, gpointer user_data);

#endif
