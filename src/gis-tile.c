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
