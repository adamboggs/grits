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

#ifndef __GRITS_TILE_H__
#define __GRITS_TILE_H__

#include <glib.h>
#include <glib-object.h>
#include "grits-object.h"

#define GRITS_TYPE_TILE            (grits_tile_get_type())
#define GRITS_TILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_TILE, GritsTile))
#define GRITS_IS_TILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_TILE))
#define GRITS_TILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_TILE, GritsTileClass))
#define GRITS_IS_TILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_TILE))
#define GRITS_TILE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_TILE, GritsTileClass))

typedef struct _GritsTile      GritsTile;
typedef struct _GritsTileClass GritsTileClass;

struct _GritsTile {
	GritsObject  parent_instance;

	/* Pointer to the tile data */
	gpointer data;

	/* Drawing order */
	gint zindex;

	/* North,South,East,West limits */
	GritsBounds edge;

	/* Texture mapping coordinates */
	GritsBounds coords;

	/* Pointers to parent/child nodes */
	GritsTile *parent;
	GritsTile *children[2][2];

	/* Last access time (for garbage collection) */
	time_t atime;
};

struct _GritsTileClass {
	GritsObjectClass parent_class;
};

/**
 * GritsTileLoadFunc:
 * @tile:      the tile to load
 * @user_data: data paseed to the function
 *
 * Used to load the image data associated with a tile. For GritsOpenGL, this
 * function should store the OpenGL texture number in the tiles data field.
 */
typedef void (*GritsTileLoadFunc)(GritsTile *tile, gpointer user_data);

/**
 * GritsTileFreeFunc:
 * @tile:      the tile to free
 * @user_data: data paseed to the function
 *
 * Used to free the image data associated with a tile
 */
typedef void (*GritsTileFreeFunc)(GritsTile *tile, gpointer user_data);

/* Forech functions */
/**
 * grits_tile_foreach:
 * @parent: the #GritsTile to iterate over
 * @child:  a pointer to a #GritsTile to store the current subtile 
 *
 * Iterate over each imediate subtile of @parent. 
 */
#define grits_tile_foreach(parent, child) \
	for (int _x = 0; _x < G_N_ELEMENTS(parent->children); _x++) \
	for (int _y = 0; child = parent->children[_x][_y], \
		_y < G_N_ELEMENTS(parent->children[_x]); _y++)

/**
 * grits_tile_foreach_index:
 * @parent: the #GritsTile to iterate over
 * @x:      integer to store the x index of the current subtile
 * @y:      integer to store the y index of the current subtile
 *
 * Iterate over each imediate subtile of @parent. 
 */
#define grits_tile_foreach_index(parent, x, y) \
	for (x = 0; x < G_N_ELEMENTS(parent->children); x++) \
	for (y = 0; y < G_N_ELEMENTS(parent->children[x]); y++)

/* Path to string table, keep in sync with tile->children */
extern gchar *grits_tile_path_table[2][2];

GType grits_tile_get_type(void);

/* Allocate a new Tile */
GritsTile *grits_tile_new(GritsTile *parent,
	gdouble n, gdouble s, gdouble e, gdouble w);

/* Return a string representation of the tile's path */
gchar *grits_tile_get_path(GritsTile *child);

/* Update a root tile */
/* Based on eye distance */
void grits_tile_update(GritsTile *root, GritsPoint *eye,
		gdouble res, gint width, gint height,
		GritsTileLoadFunc load_func, gpointer user_data);

/* Find the leaf tile containing lat-lon */
GritsTile *grits_tile_find(GritsTile *root, gdouble lat, gdouble lon);

/* Delete nodes that haven't been accessed since atime */
GritsTile *grits_tile_gc(GritsTile *root, time_t atime,
		GritsTileFreeFunc free_func, gpointer user_data);

/* Free a tile and all it's children */
void grits_tile_free(GritsTile *root,
		GritsTileFreeFunc free_func, gpointer user_data);

#endif
