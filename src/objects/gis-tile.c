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

/**
 * SECTION:gis-tile
 * @short_description: Latitude/longitude overlays
 *
 * Each #GisTile corresponds to a latitude/longitude box on the surface of the
 * earth. When drawn, the #GisTile renders an images associated with it to the
 * surface of the earth. This is primarily used to draw ground overlays.
 *
 * Each GisTile can be split into subtiles in order to draw higher resolution
 * overlays. Pointers to subtitles are stored in the parent tile and a parent
 * pointer is stored in each child.
 *
 * Each #GisTile has a data filed which must be set by the user in order for
 * the tile to be drawn. When used with GisOpenGL the data must be an integer
 * representing the OpenGL texture to use when drawing the tile.
 */

#include <config.h>
#include "gis-util.h"
#include "gis-tile.h"

gchar *gis_tile_path_table[2][2] = {
	{"00.", "01."},
	{"10.", "11."},
};

/**
 * gis_tile_new:
 * @parent: the parent for the tile, or NULL
 * @n:      the northern border of the tile
 * @s:      the southern border of the tile
 * @e:      the eastern border of the tile
 * @w:      the western border of the tile
 *
 * Create a tile associated with a particular latitude/longitude box.
 *
 * Returns: the new #GisTile
 */
GisTile *gis_tile_new(GisTile *parent,
	gdouble n, gdouble s, gdouble e, gdouble w)
{
	GisTile *tile = g_object_new(GIS_TYPE_TILE, NULL);
	tile->parent = parent;
	tile->edge.n = n;
	tile->edge.s = s;
	tile->edge.e = e;
	tile->edge.w = w;
	tile->atime  = time(NULL);
	return tile;
}

/**
 * gis_tile_get_path:
 * @child: the tile to generate a path for
 *
 * Generate a string representation of a tiles location in a group of nested
 * tiles. The string returned consists of groups of two digits separated by a
 * delimiter. Each group of digits the tiles location with respect to it's
 * parent tile.
 *
 * Returns: the path representing the tiles's location
 */
gchar *gis_tile_get_path(GisTile *child)
{
	/* This could be easily cached if necessary */
	int x, y;
	GList *parts = NULL;
	for (GisTile *parent = child->parent; parent; child = parent, parent = child->parent)
		gis_tile_foreach_index(child, x, y)
			if (parent->children[x][y] == child)
				parts = g_list_prepend(parts, gis_tile_path_table[x][y]);
	GString *path = g_string_new("");
	for (GList *cur = parts; cur; cur = cur->next)
		g_string_append(path, cur->data);
	g_list_free(parts);
	return g_string_free(path, FALSE);
}

static gdouble _gis_tile_get_min_dist(GisTile *tile,
		gdouble lat, gdouble lon, gdouble elev)
{
	gdouble tlat  = lat > tile->edge.n ? tile->edge.n :
                        lat < tile->edge.s ? tile->edge.s : lat;
	gdouble tlon  = lon > tile->edge.e ? tile->edge.e :
	                lon < tile->edge.w ? tile->edge.w : lon;
	gdouble telev = 0; // TODO: elevation at rlat,rlon
	//if (lat == tlat && lon == tlon)
	//	return elev; /* Shortcut? */
	gdouble a[3], b[3];
	lle2xyz( lat,  lon,  elev, a+0, a+1, a+2);
	lle2xyz(tlat, tlon, telev, b+0, b+1, b+2);
	return distd(a, b);
}

static gboolean _gis_tile_needs_split(GisTile *tile,
		gdouble max_res, gint width, gint height,
		gdouble lat, gdouble lon, gdouble elev)
{
	gdouble lat_point = tile->edge.n < 0 ? tile->edge.n :
	                    tile->edge.s > 0 ? tile->edge.s : 0;
	gdouble min_dist  = _gis_tile_get_min_dist(tile, lat, lon, elev);
	gdouble view_res  = MPPX(min_dist);
	gdouble lon_dist  = tile->edge.e - tile->edge.w;
	gdouble tile_res  = ll2m(lon_dist, lat_point)/width;

	/* This isn't really right, but it helps with memory since we don't (yet?) test if the tile
	 * would be drawn */
	gdouble scale = elev / min_dist;
	view_res /= scale;
	//g_message("tile=(%7.2f %7.2f %7.2f %7.2f) "
	//          "eye=(%9.1f %9.1f %9.1f) "
	//          "elev=%9.1f / dist=%9.1f = %f",
	//		tile->edge.n, tile->edge.s, tile->edge.e, tile->edge.w,
	//		lat, lon, elev,
	//		elev, min_dist, scale);

	if (tile_res < max_res)
		return FALSE;
	return view_res < tile_res;
}

/**
 * gis_tile_update:
 * @root:      the root tile to split
 * @res:       a maximum resolution in meters per pixel to split tiles to
 * @width:     width in pixels of the image associated with the tile
 * @height:    height in pixels of the image associated with the tile
 * @lat:       latitude of the eye point
 * @lon:       longitude of the eye point
 * @elev:      elevation of the eye point
 * @load_func: function used to load the image when a new tile is created
 * @user_data: user data to past to the load function
 *
 * Recursively split a tile into children of appropriate detail. The resolution
 * of the tile in pixels per meter is compared to the resolution which the tile
 * is being drawn at on the screen. If the screen resolution is insufficient
 * the tile is recursively subdivided until a sufficient resolution is
 * achieved.
 */
void gis_tile_update(GisTile *root,
		gdouble res, gint width, gint height,
		gdouble lat, gdouble lon, gdouble elev,
		GisTileLoadFunc load_func, gpointer user_data)
{
	root->atime = time(NULL);
	//g_debug("GisTile: update - %p->atime = %u", root, (guint)root->atime);
	gdouble lat_dist = root->edge.n - root->edge.s;
	gdouble lon_dist = root->edge.e - root->edge.w;
	if (_gis_tile_needs_split(root, res, width, height, lat, lon, elev)) {
		gdouble lat_step = lat_dist / G_N_ELEMENTS(root->children);
		gdouble lon_step = lon_dist / G_N_ELEMENTS(root->children[0]);
		int x, y;
		gis_tile_foreach_index(root, x, y) {
			if (!root->children[x][y]) {
				root->children[x][y] = gis_tile_new(root,
						root->edge.n-(lat_step*(x+0)),
						root->edge.n-(lat_step*(x+1)),
						root->edge.w+(lon_step*(y+1)),
						root->edge.w+(lon_step*(y+0)));
				load_func(root->children[x][y], user_data);
			}
			gis_tile_update(root->children[x][y],
					res, width, height,
					lat, lon, elev,
					load_func, user_data);
		}
	}
}

/**
 * gis_tile_find:
 * @root: the root tile to search from
 * @lat:  target latitude
 * @lon:  target longitude
 *
 * Locate the subtile with the highest resolution which contains the given
 * lat/lon point.
 * 
 * Returns: the child tile
 */
GisTile *gis_tile_find(GisTile *root, gdouble lat, gdouble lon)
{
	gint    rows = G_N_ELEMENTS(root->children);
	gint    cols = G_N_ELEMENTS(root->children[0]);

	gdouble lat_step = (root->edge.n - root->edge.s) / rows;
	gdouble lon_step = (root->edge.e - root->edge.w) / cols;

	gdouble lat_offset = root->edge.n - lat;;
	gdouble lon_offset = lon - root->edge.w;

	gint    row = lat_offset / lat_step;
	gint    col = lon_offset / lon_step;

	if (lon == 180) col--;
	if (lat == -90) row--;

	//if (lon == 180 || lon == -180)
	//	g_message("lat=%f,lon=%f step=%f,%f off=%f,%f row=%d/%d,col=%d/%d",
	//		lat,lon, lat_step,lon_step, lat_offset,lon_offset, row,rows,col,cols);

	if (row < 0 || row >= rows || col < 0 || col >= cols)
		return NULL;
	else if (root->children[row][col] && root->children[row][col]->data)
		return gis_tile_find(root->children[row][col], lat, lon);
	else
		return root;
}

/**
 * gis_tile_gc:
 * @root:      the root tile to start garbage collection at
 * @atime:     most recent time at which tiles will be kept
 * @free_func: function used to free the image when a new tile is collected
 * @user_data: user data to past to the free function
 *
 * Garbage collect old tiles. This removes and deallocate tiles that have not
 * been used since before @atime.
 *
 * Returns: a pointer to the original tile, or NULL if it was garbage collected
 */
GisTile *gis_tile_gc(GisTile *root, time_t atime,
		GisTileFreeFunc free_func, gpointer user_data)
{
	if (!root)
		return NULL;
	gboolean has_children = FALSE;
	int x, y;
	gis_tile_foreach_index(root, x, y) {
		root->children[x][y] = gis_tile_gc(
				root->children[x][y], atime,
				free_func, user_data);
		if (root->children[x][y])
			has_children = TRUE;
	}
	//g_debug("GisTile: gc - %p->atime=%u < atime=%u",
	//		root, (guint)root->atime, (guint)atime);
	if (!has_children && root->atime < atime && root->data) {
		free_func(root, user_data);
		g_object_unref(root);
		return NULL;
	}
	return root;
}

/* Use GObject for this */
/**
 * gis_tile_free:
 * @root:      the root tile to free
 * @free_func: function used to free the image when a new tile is collected
 * @user_data: user data to past to the free function
 *
 * Recursively free a tile and all it's children.
 */
void gis_tile_free(GisTile *root, GisTileFreeFunc free_func, gpointer user_data)
{
	if (!root)
		return;
	GisTile *child;
	gis_tile_foreach(root, child)
		gis_tile_free(child, free_func, user_data);
	if (free_func)
		free_func(root, user_data);
	g_object_unref(root);
}

/* GObject code */
G_DEFINE_TYPE(GisTile, gis_tile, GIS_TYPE_OBJECT);
static void gis_tile_init(GisTile *tile)
{
}

static void gis_tile_class_init(GisTileClass *klass)
{
}
