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

#include <config.h>
#include <glib.h>

#include "gis-world.h"
#include "gis-tile.h"

gchar *gis_tile_path_table[2][2] = {
	{".00", ".01"},
	{".10", ".11"},
};

GisTile *gis_tile_new(GisTile *parent,
	gdouble n, gdouble s, gdouble e, gdouble w)
{
	GisTile *self = g_new0(GisTile, 1);
	self->parent = parent;
	self->edge.n = n;
	self->edge.s = s;
	self->edge.e = e;
	self->edge.w = w;
	self->atime  = time(NULL);
	return self;
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
	for (; parts; parts = parts->next)
		g_string_append(path, parts->data);
	g_list_free(parts);
	return g_string_free(path, FALSE);
}

gdouble _gis_tile_get_min_dist(GisTile *self,
		gdouble lat, gdouble lon, gdouble elev)
{
	gdouble tlat  = lat > self->edge.n ? self->edge.n :
                        lat < self->edge.s ? self->edge.s : lat;
	gdouble tlon  = lon > self->edge.e ? self->edge.e :
	                lon < self->edge.w ? self->edge.w : lon;
	gdouble telev = 0; // TODO: elevation at rlat,rlon
	//if (lat == tlat && lon == tlon)
	//	return elev; /* Shortcut? */
	gdouble a[3], b[3];
	lle2xyz( lat,  lon,  elev, a+0, a+1, a+2);
	lle2xyz(tlat, tlon, telev, b+0, b+1, b+2);
	return distd(a, b);
}

gboolean _gis_tile_needs_split(GisTile *self,
		gdouble max_res, gint width, gint height,
		gdouble lat, gdouble lon, gdouble elev)
{
	gdouble lat_point = self->edge.n < 0 ? self->edge.n :
	                    self->edge.s > 0 ? self->edge.s : 0;
	gdouble min_dist  = _gis_tile_get_min_dist(self, lat, lon, elev);
	gdouble view_res  = MPPX(min_dist);
	gdouble lon_dist  = self->edge.e - self->edge.w;
	gdouble tile_res  = ll2m(lon_dist, lat_point)/width;

	/* This isn't really right, but it helps with memory since we don't (yet?) test if the tile
	 * would be drawn */
	gdouble scale = elev / min_dist;
	view_res /= scale;
	//g_message("tile=(%7.2f %7.2f %7.2f %7.2f) "
	//          "eye=(%9.1f %9.1f %9.1f) "
	//          "elev=%9.1f / dist=%9.1f = %f",
	//		self->edge.n, self->edge.s, self->edge.e, self->edge.w,
	//		lat, lon, elev,
	//		elev, min_dist, scale);

	if (tile_res < max_res)
		return FALSE;
	return view_res < tile_res;
}

void gis_tile_update(GisTile *self,
		gdouble res, gint width, gint height,
		gdouble lat, gdouble lon, gdouble elev,
		GisTileLoadFunc load_func, gpointer user_data)
{
	self->atime = time(NULL);
	//g_debug("GisTile: update - %p->atime = %u", self, (guint)self->atime);
	gdouble lat_dist = self->edge.n - self->edge.s;
	gdouble lon_dist = self->edge.e - self->edge.w;
	if (_gis_tile_needs_split(self, res, width, height, lat, lon, elev)) {
		gdouble lat_step = lat_dist / G_N_ELEMENTS(self->children);
		gdouble lon_step = lon_dist / G_N_ELEMENTS(self->children[0]);
		int x, y;
		gis_tile_foreach_index(self, x, y) {
			if (!self->children[x][y]) {
				self->children[x][y] = gis_tile_new(self,
						self->edge.n-(lat_step*(x+0)),
						self->edge.n-(lat_step*(x+1)),
						self->edge.w+(lon_step*(y+1)),
						self->edge.w+(lon_step*(y+0)));
				load_func(self->children[x][y], user_data);
			}
			gis_tile_update(self->children[x][y],
					res, width, height,
					lat, lon, elev,
					load_func, user_data);
		}
	}
}

GisTile *gis_tile_gc(GisTile *self, time_t atime,
		GisTileFreeFunc free_func, gpointer user_data)
{
	if (!self)
		return NULL;
	gboolean has_children = FALSE;
	int x, y;
	gis_tile_foreach_index(self, x, y) {
		self->children[x][y] = gis_tile_gc(
				self->children[x][y], atime,
				free_func, user_data);
		if (self->children[x][y])
			has_children = TRUE;
	}
	//g_debug("GisTile: gc - %p->atime=%u < atime=%u",
	//		self, (guint)self->atime, (guint)atime);
	if (!has_children && self->atime < atime && self->data) {
		free_func(self, user_data);
		g_free(self);
		return NULL;
	}
	return self;
}

void gis_tile_free(GisTile *self, GisTileFreeFunc free_func, gpointer user_data)
{
	if (!self)
		return;
	GisTile *child;
	gis_tile_foreach(self, child)
		gis_tile_free(child, free_func, user_data);
	if (free_func)
		free_func(self, user_data);
	g_free(self);
}
