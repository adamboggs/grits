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

#include <config.h>
#include "gis-util.h"
#include "gis-tile.h"

gchar *gis_tile_path_table[2][2] = {
	{"00.", "01."},
	{"10.", "11."},
};

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

gchar *gis_tile_get_path(GisTile *child)
{
	/* This could be easily cached if necessasairy */
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

gdouble _gis_tile_get_min_dist(GisTile *tile,
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

gboolean _gis_tile_needs_split(GisTile *tile,
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

void gis_tile_update(GisTile *tile,
		gdouble res, gint width, gint height,
		gdouble lat, gdouble lon, gdouble elev,
		GisTileLoadFunc load_func, gpointer user_data)
{
	tile->atime = time(NULL);
	//g_debug("GisTile: update - %p->atime = %u", tile, (guint)tile->atime);
	gdouble lat_dist = tile->edge.n - tile->edge.s;
	gdouble lon_dist = tile->edge.e - tile->edge.w;
	if (_gis_tile_needs_split(tile, res, width, height, lat, lon, elev)) {
		gdouble lat_step = lat_dist / G_N_ELEMENTS(tile->children);
		gdouble lon_step = lon_dist / G_N_ELEMENTS(tile->children[0]);
		int x, y;
		gis_tile_foreach_index(tile, x, y) {
			if (!tile->children[x][y]) {
				tile->children[x][y] = gis_tile_new(tile,
						tile->edge.n-(lat_step*(x+0)),
						tile->edge.n-(lat_step*(x+1)),
						tile->edge.w+(lon_step*(y+1)),
						tile->edge.w+(lon_step*(y+0)));
				load_func(tile->children[x][y], user_data);
			}
			gis_tile_update(tile->children[x][y],
					res, width, height,
					lat, lon, elev,
					load_func, user_data);
		}
	}
}

GisTile *gis_tile_find(GisTile *tile, gdouble lat, gdouble lon)
{
	gint    rows = G_N_ELEMENTS(tile->children);
	gint    cols = G_N_ELEMENTS(tile->children[0]);

	gdouble lat_step = (tile->edge.n - tile->edge.s) / rows;
	gdouble lon_step = (tile->edge.e - tile->edge.w) / cols;

	gdouble lat_offset = tile->edge.n - lat;;
	gdouble lon_offset = lon - tile->edge.w;

	gint    row = lat_offset / lat_step;
	gint    col = lon_offset / lon_step;

	if (lon == 180) col--;
	if (lat == -90) row--;

	//if (lon == 180 || lon == -180)
	//	g_message("lat=%f,lon=%f step=%f,%f off=%f,%f row=%d/%d,col=%d/%d",
	//		lat,lon, lat_step,lon_step, lat_offset,lon_offset, row,rows,col,cols);

	if (row < 0 || row >= rows || col < 0 || col >= cols)
		return NULL;
	else if (tile->children[row][col] && tile->children[row][col]->data)
		return gis_tile_find(tile->children[row][col], lat, lon);
	else
		return tile;
}

GisTile *gis_tile_gc(GisTile *tile, time_t atime,
		GisTileFreeFunc free_func, gpointer user_data)
{
	if (!tile)
		return NULL;
	gboolean has_children = FALSE;
	int x, y;
	gis_tile_foreach_index(tile, x, y) {
		tile->children[x][y] = gis_tile_gc(
				tile->children[x][y], atime,
				free_func, user_data);
		if (tile->children[x][y])
			has_children = TRUE;
	}
	//g_debug("GisTile: gc - %p->atime=%u < atime=%u",
	//		tile, (guint)tile->atime, (guint)atime);
	if (!has_children && tile->atime < atime && tile->data) {
		free_func(tile, user_data);
		g_object_unref(tile);
		return NULL;
	}
	return tile;
}

/* Use GObject for this */
void gis_tile_free(GisTile *tile, GisTileFreeFunc free_func, gpointer user_data)
{
	if (!tile)
		return;
	GisTile *child;
	gis_tile_foreach(tile, child)
		gis_tile_free(child, free_func, user_data);
	if (free_func)
		free_func(tile, user_data);
	g_object_unref(tile);
}

/* GObject code */
G_DEFINE_TYPE(GisTile, gis_tile, GIS_TYPE_OBJECT);
static void gis_tile_init(GisTile *tile) { }
static void gis_tile_class_init(GisTileClass *klass) { }
