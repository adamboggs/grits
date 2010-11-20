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
#include <GL/gl.h>
#include "grits-tile.h"

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
	tile->atime  = time(NULL);
	gis_bounds_set_bounds(&tile->coords, 0, 1, 1, 0);
	gis_bounds_set_bounds(&tile->edge, n, s, e, w);
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

static gdouble _gis_tile_get_min_dist(GisPoint *eye, GisBounds *bounds)
{
	GisPoint pos = {};
	pos.lat = eye->lat > bounds->n ? bounds->n :
	          eye->lat < bounds->s ? bounds->s : eye->lat;
	pos.lon = eye->lon > bounds->e ? bounds->e :
	          eye->lon < bounds->w ? bounds->w : eye->lon;
	//if (eye->lat == pos.lat && eye->lon == pos.lon)
	//	return elev; /* Shortcut? */
	gdouble a[3], b[3];
	lle2xyz(eye->lat, eye->lon, eye->elev, a+0, a+1, a+2);
	lle2xyz(pos.lat,  pos.lon,  pos.elev,  b+0, b+1, b+2);
	return distd(a, b);
}

static gboolean _gis_tile_precise(GisPoint *eye, GisBounds *bounds,
		gdouble max_res, gint width, gint height)
{
	gdouble min_dist  = _gis_tile_get_min_dist(eye, bounds);
	gdouble view_res  = MPPX(min_dist);

	gdouble lat_point = bounds->n < 0 ? bounds->n :
	                    bounds->s > 0 ? bounds->s : 0;
	gdouble lon_dist  = bounds->e - bounds->w;
	gdouble tile_res  = ll2m(lon_dist, lat_point)/width;

	/* This isn't really right, but it helps with memory since we don't (yet?) test if the tile
	 * would be drawn */
	gdouble scale = eye->elev / min_dist;
	view_res /= scale;
	//view_res /= 1.4; /* make it a little nicer, not sure why this is needed */
	//g_message("tile=(%7.2f %7.2f %7.2f %7.2f) "
	//          "eye=(%9.1f %9.1f %9.1f) "
	//          "elev=%9.1f / dist=%9.1f = %f",
	//		tile->edge.n, tile->edge.s, tile->edge.e, tile->edge.w,
	//		lat, lon, elev,
	//		elev, min_dist, scale);

	return tile_res < max_res ||
	       tile_res < view_res;
}

/**
 * gis_tile_update:
 * @root:      the root tile to split
 * @eye:       the point the tile is viewed from, for calculating distances
 * @res:       a maximum resolution in meters per pixel to split tiles to
 * @width:     width in pixels of the image associated with the tile
 * @height:    height in pixels of the image associated with the tile
 * @load_func: function used to load the image when a new tile is created
 * @user_data: user data to past to the load function
 *
 * Recursively split a tile into children of appropriate detail. The resolution
 * of the tile in pixels per meter is compared to the resolution which the tile
 * is being drawn at on the screen. If the screen resolution is insufficient
 * the tile is recursively subdivided until a sufficient resolution is
 * achieved.
 */
void gis_tile_update(GisTile *root, GisPoint *eye,
		gdouble res, gint width, gint height,
		GisTileLoadFunc load_func, gpointer user_data)
{
	root->atime = time(NULL);
	//g_debug("GisTile: update - %p->atime = %u",
	//		root, (guint)root->atime);
	const gdouble rows = G_N_ELEMENTS(root->children);
	const gdouble cols = G_N_ELEMENTS(root->children[0]);
	const gdouble lat_dist = root->edge.n - root->edge.s;
	const gdouble lon_dist = root->edge.e - root->edge.w;
	const gdouble lat_step = lat_dist / rows;
	const gdouble lon_step = lon_dist / cols;
	int row, col;
	gis_tile_foreach_index(root, row, col) {
		GisTile **child = &root->children[row][col];
		GisBounds edge;
		edge.n = root->edge.n-(lat_step*(row+0));
		edge.s = root->edge.n-(lat_step*(row+1));
		edge.e = root->edge.w+(lon_step*(col+1));
		edge.w = root->edge.w+(lon_step*(col+0));
		if (!_gis_tile_precise(eye, &edge, res,
					width/cols, height/rows)) {
			if (!*child) {
				*child = gis_tile_new(root, edge.n, edge.s,
						edge.e, edge.w);
				load_func(*child, user_data);
			}
			gis_tile_update(*child, eye,
					res, width, height,
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

/* Draw a single tile */
static void gis_tile_draw_one(GisTile *tile, GisOpenGL *opengl, GList *triangles)
{
	if (!tile || !tile->data)
		return;
	if (!triangles)
		g_warning("GisOpenGL: _draw_tiles - No triangles to draw: edges=%f,%f,%f,%f",
			tile->edge.n, tile->edge.s, tile->edge.e, tile->edge.w);
	//g_message("drawing %4d triangles for tile edges=%7.2f,%7.2f,%7.2f,%7.2f",
	//		g_list_length(triangles), tile->edge.n, tile->edge.s, tile->edge.e, tile->edge.w);
	gdouble n = tile->edge.n;
	gdouble s = tile->edge.s;
	gdouble e = tile->edge.e;
	gdouble w = tile->edge.w;

	gdouble londist = e - w;
	gdouble latdist = n - s;

	gdouble xscale = tile->coords.e - tile->coords.w;
	gdouble yscale = tile->coords.s - tile->coords.n;

	for (GList *cur = triangles; cur; cur = cur->next) {
		RoamTriangle *tri = cur->data;

		gdouble lat[3] = {tri->p.r->lat, tri->p.m->lat, tri->p.l->lat};
		gdouble lon[3] = {tri->p.r->lon, tri->p.m->lon, tri->p.l->lon};

		if (lon[0] < -90 || lon[1] < -90 || lon[2] < -90) {
			if (lon[0] > 90) lon[0] -= 360;
			if (lon[1] > 90) lon[1] -= 360;
			if (lon[2] > 90) lon[2] -= 360;
		}

		gdouble xy[3][2] = {
			{(lon[0]-w)/londist, 1-(lat[0]-s)/latdist},
			{(lon[1]-w)/londist, 1-(lat[1]-s)/latdist},
			{(lon[2]-w)/londist, 1-(lat[2]-s)/latdist},
		};

		//if ((lat[0] == 90 && (xy[0][0] < 0 || xy[0][0] > 1)) ||
		//    (lat[1] == 90 && (xy[1][0] < 0 || xy[1][0] > 1)) ||
		//    (lat[2] == 90 && (xy[2][0] < 0 || xy[2][0] > 1)))
		//	g_message("w,e=%4.f,%4.f   "
		//	          "lat,lon,x,y="
		//	          "%4.1f,%4.0f,%4.2f,%4.2f   "
		//	          "%4.1f,%4.0f,%4.2f,%4.2f   "
		//	          "%4.1f,%4.0f,%4.2f,%4.2f   ",
		//		w,e,
		//		lat[0], lon[0], xy[0][0], xy[0][1],
		//		lat[1], lon[1], xy[1][0], xy[1][1],
		//		lat[2], lon[2], xy[2][0], xy[2][1]);

		/* Fix poles */
		if (lat[0] == 90 || lat[0] == -90) xy[0][0] = 0.5;
		if (lat[1] == 90 || lat[1] == -90) xy[1][0] = 0.5;
		if (lat[2] == 90 || lat[2] == -90) xy[2][0] = 0.5;

		/* Scale to tile coords */
		for (int i = 0; i < 3; i++) {
			xy[i][0] = tile->coords.w + xy[i][0]*xscale;
			xy[i][1] = tile->coords.n + xy[i][1]*yscale;
		}

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glBindTexture(GL_TEXTURE_2D, *(guint*)tile->data);
		glPolygonOffset(0, -tile->zindex);
		glBegin(GL_TRIANGLES);
		glNormal3dv(tri->p.r->norm); glTexCoord2dv(xy[0]); glVertex3dv((double*)tri->p.r);
		glNormal3dv(tri->p.m->norm); glTexCoord2dv(xy[1]); glVertex3dv((double*)tri->p.m);
		glNormal3dv(tri->p.l->norm); glTexCoord2dv(xy[2]); glVertex3dv((double*)tri->p.l);
		glEnd();
	}
}

/* Draw the tile */
static void gis_tile_draw_rec(GisTile *tile, GisOpenGL *opengl)
{
	/* Only draw children if possible */
	gboolean has_children = FALSE;
	GisTile *child;
	gis_tile_foreach(tile, child)
		if (child && child->data)
			has_children = TRUE;

	GList *triangles = NULL;
	if (has_children) {
		/* TODO: simplify this */
		const gdouble rows = G_N_ELEMENTS(tile->children);
		const gdouble cols = G_N_ELEMENTS(tile->children[0]);
		const gdouble lat_dist = tile->edge.n - tile->edge.s;
		const gdouble lon_dist = tile->edge.e - tile->edge.w;
		const gdouble lat_step = lat_dist / rows;
		const gdouble lon_step = lon_dist / cols;
		int row, col;
		gis_tile_foreach_index(tile, row, col) {
			GisTile *child = tile->children[row][col];
			if (child && child->data) {
				gis_tile_draw_rec(child, opengl);
			} else {
				const gdouble n = tile->edge.n-(lat_step*(row+0));
				const gdouble s = tile->edge.n-(lat_step*(row+1));
				const gdouble e = tile->edge.w+(lon_step*(col+1));
				const gdouble w = tile->edge.w+(lon_step*(col+0));
				GList *these = roam_sphere_get_intersect(opengl->sphere,
						FALSE, n, s, e, w);
				triangles = g_list_concat(triangles, these);
			}
		}
	} else {
		triangles = roam_sphere_get_intersect(opengl->sphere, FALSE,
				tile->edge.n, tile->edge.s, tile->edge.e, tile->edge.w);
	}
	if (triangles)
		gis_tile_draw_one(tile, opengl, triangles);
	g_list_free(triangles);
}

static void gis_tile_draw(GisObject *tile, GisOpenGL *opengl)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	gis_tile_draw_rec(GIS_TILE(tile), opengl);
}


/* GObject code */
G_DEFINE_TYPE(GisTile, gis_tile, GIS_TYPE_OBJECT);
static void gis_tile_init(GisTile *tile)
{
}

static void gis_tile_class_init(GisTileClass *klass)
{
	g_debug("GisTile: class_init");
	GisObjectClass *object_class = GIS_OBJECT_CLASS(klass);
	object_class->draw = gis_tile_draw;
}
