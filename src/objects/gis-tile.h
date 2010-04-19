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
#include <glib-object.h>
#include "gis-object.h"

#define GIS_TYPE_TILE            (gis_tile_get_type())
#define GIS_TILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_TILE, GisTile))
#define GIS_IS_TILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_TILE))
#define GIS_TILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_TILE, GisTileClass))
#define GIS_IS_TILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_TILE))
#define GIS_TILE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_TILE, GisTileClass))

typedef struct _GisTile      GisTile;
typedef struct _GisTileClass GisTileClass;

struct _GisTile {
	GisObject  parent_instance;

	/* Pointer to the tile data */
	gpointer data;

	/* Pointer to the tile data */
	gint zindex;

	/* North,South,East,West limits */
	GisBBox edge;

	/* Pointers to parent/child nodes */
	GisTile *parent;
	GisTile *children[2][2];

	/* Last access time (for garbage collection) */
	time_t atime;
};

struct _GisTileClass {
	GisObjectClass parent_class;
};

/**
 * GisTileLoadFunc:
 * @tile:      the tile to load
 * @user_data: data paseed to the function
 *
 * Used to load the image data associated with a tile. For GisOpenGL, this
 * function should store the OpenGL texture number in the tiles data field.
 */
typedef void (*GisTileLoadFunc)(GisTile *tile, gpointer user_data);

/**
 * GisTileFreeFunc:
 * @tile:      the tile to free
 * @user_data: data paseed to the function
 *
 * Used to free the image data associated with a tile
 */
typedef void (*GisTileFreeFunc)(GisTile *tile, gpointer user_data);

/* Forech functions */
/**
 * gis_tile_foreach:
 * @parent: the #GisTile to iterate over
 * @child:  a pointer to a #GisTile to store the current subtile 
 *
 * Iterate over each imediate subtile of @parent. 
 */
#define gis_tile_foreach(parent, child) \
	for (int _x = 0; _x < G_N_ELEMENTS(parent->children); _x++) \
	for (int _y = 0; child = parent->children[_x][_y], \
		_y < G_N_ELEMENTS(parent->children[_x]); _y++)

/**
 * gis_tile_foreach_index:
 * @parent: the #GisTile to iterate over
 * @x:      integer to store the x index of the current subtile
 * @y:      integer to store the y index of the current subtile
 *
 * Iterate over each imediate subtile of @parent. 
 */
#define gis_tile_foreach_index(parent, x, y) \
	for (x = 0; x < G_N_ELEMENTS(parent->children); x++) \
	for (y = 0; y < G_N_ELEMENTS(parent->children[x]); y++)

/* Path to string table, keep in sync with tile->children */
extern gchar *gis_tile_path_table[2][2];

GType gis_tile_get_type(void);

/* Allocate a new Tile */
GisTile *gis_tile_new(GisTile *parent,
	gdouble n, gdouble s, gdouble e, gdouble w);

/* Return a string representation of the tile's path */
gchar *gis_tile_get_path(GisTile *child);

/* Update a root tile */
/* Based on eye distance */
void gis_tile_update(GisTile *root, GisPoint *eye,
		gdouble res, gint width, gint height,
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
