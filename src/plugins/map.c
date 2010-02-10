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
 * SECTION:map
 * @short_description: Map plugin
 *
 * #GisPluginMap provides map overlays. Much of this data is obtained from the
 * OpenStreetMap project.
 */

#include <time.h>
#include <stdlib.h>
#include <glib/gstdio.h>
#include <GL/gl.h>

#include <gis.h>

#include "map.h"

#define MAX_RESOLUTION 500
#define TILE_WIDTH     1024
#define TILE_HEIGHT    512

static const guchar colormap[][2][4] = {
	{{0x73, 0x91, 0xad}, {0x73, 0x91, 0xad, 0x20}}, // Oceans
	{{0xf6, 0xee, 0xee}, {0xf6, 0xee, 0xee, 0x00}}, // Ground
	{{0xff, 0xff, 0xff}, {0xff, 0xff, 0xff, 0xff}}, // Borders
	{{0x73, 0x93, 0xad}, {0x73, 0x93, 0xad, 0x40}}, // Lakes
	{{0xff, 0xe1, 0x80}, {0xff, 0xe1, 0x80, 0x60}}, // Cities
};

struct _LoadTileData {
	GisPluginMap *map;
	GisTile      *tile;
	GdkPixbuf    *pixbuf;
};
static gboolean _load_tile_cb(gpointer _data)
{
	struct _LoadTileData *data = _data;
	GisPluginMap *map    = data->map;
	GisTile      *tile   = data->tile;
	GdkPixbuf    *pixbuf = data->pixbuf;
	g_free(data);

	/* Create Texture */
	g_debug("GisPluginMap: _load_tile_cb start");
	guchar   *pixels = gdk_pixbuf_get_pixels(pixbuf);
	gboolean  alpha  = gdk_pixbuf_get_has_alpha(pixbuf);
	gint      width  = gdk_pixbuf_get_width(pixbuf);
	gint      height = gdk_pixbuf_get_height(pixbuf);

	for (int i = 0; i < width*height; i++) {
		for (int j = 0; j < G_N_ELEMENTS(colormap); j++) {
			if (pixels[i*4+0] == colormap[j][0][0] &&
			    pixels[i*4+1] == colormap[j][0][1] &&
			    pixels[i*4+2] == colormap[j][0][2]) {
				pixels[i*4+0] = colormap[j][1][0];
				pixels[i*4+1] = colormap[j][1][1];
				pixels[i*4+2] = colormap[j][1][2];
				pixels[i*4+3] = colormap[j][1][3];
				break;
			}
		}
	}

	guint *tex = g_new0(guint, 1);
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
			(alpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, pixels);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glFlush();

	tile->data = tex;
	gtk_widget_queue_draw(GTK_WIDGET(map->viewer));
	g_object_unref(pixbuf);
	return FALSE;
}

static void _load_tile(GisTile *tile, gpointer _map)
{
	GisPluginMap *map = _map;
	g_debug("GisPluginMap: _load_tile start %p", g_thread_self());
	char *path = gis_wms_fetch(map->wms, tile, GIS_ONCE, NULL, NULL);
	struct _LoadTileData *data = g_new0(struct _LoadTileData, 1);
	data->map    = map;
	data->tile   = tile;
	data->pixbuf = gdk_pixbuf_new_from_file(path, NULL);
	if (data->pixbuf) {
		g_idle_add_full(G_PRIORITY_LOW, _load_tile_cb, data, NULL);
	} else {
		g_warning("GisPluginMap: _load_tile - Error loading pixbuf %s", path);
		g_remove(path);
	}
	g_free(path);
	g_debug("GisPluginMap: _load_tile end %p", g_thread_self());
}

static gboolean _free_tile_cb(gpointer data)
{
	glDeleteTextures(1, data);
	g_free(data);
	return FALSE;
}
static void _free_tile(GisTile *tile, gpointer _map)
{
	GisPluginMap *map = _map;
	g_debug("GisPluginMap: _free_tile: %p", tile->data);
	g_idle_add_full(G_PRIORITY_LOW, _free_tile_cb, tile->data, NULL);
}

static gpointer _update_tiles(gpointer _map)
{
	g_debug("GisPluginMap: _update_tiles");
	GisPluginMap *map = _map;
	if (!g_mutex_trylock(map->mutex))
		return NULL;
	GisPoint eye;
	gis_viewer_get_location(map->viewer, &eye.lat, &eye.lon, &eye.elev);
	gis_tile_update(map->tiles, &eye,
			MAX_RESOLUTION, TILE_WIDTH, TILE_WIDTH,
			_load_tile, map);
	gis_tile_gc(map->tiles, time(NULL)-10,
			_free_tile, map);
	g_mutex_unlock(map->mutex);
	return NULL;
}

/*************
 * Callbacks *
 *************/
static void _on_location_changed(GisViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev, GisPluginMap *map)
{
	g_thread_create(_update_tiles, map, FALSE, NULL);
}

static gpointer _threaded_init(GisPluginMap *map)
{
	_load_tile(map->tiles, map);
	_update_tiles(map);
	return NULL;
}

/***********
 * Methods *
 ***********/
/**
 * gis_plugin_map_new:
 * @viewer: the #GisViewer to use for drawing
 *
 * Create a new instance of the map plugin.
 *
 * Returns: the new #GisPluginMap
 */
GisPluginMap *gis_plugin_map_new(GisViewer *viewer)
{
	g_debug("GisPluginMap: new");
	GisPluginMap *map = g_object_new(GIS_TYPE_PLUGIN_MAP, NULL);
	map->viewer = g_object_ref(viewer);

	/* Load initial tiles */
	g_thread_create((GThreadFunc)_threaded_init, map, FALSE, NULL);

	/* Connect signals */
	map->sigid = g_signal_connect(map->viewer, "location-changed",
			G_CALLBACK(_on_location_changed), map);

	/* Add renderers */
	gis_viewer_add(viewer, GIS_OBJECT(map->tiles), GIS_LEVEL_OVERLAY-1, 0);

	return map;
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void gis_plugin_map_plugin_init(GisPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GisPluginMap, gis_plugin_map, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GIS_TYPE_PLUGIN,
			gis_plugin_map_plugin_init));
static void gis_plugin_map_plugin_init(GisPluginInterface *iface)
{
	g_debug("GisPluginMap: plugin_init");
	/* Add methods to the interface */
}
/* Class/Object init */
static void gis_plugin_map_init(GisPluginMap *map)
{
	g_debug("GisPluginMap: init");
	/* Set defaults */
	map->mutex  = g_mutex_new();
	map->tiles  = gis_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	map->wms    = gis_wms_new(
		"http://labs.metacarta.com/wms/vmap0", "basic", "image/png",
		"osm/", "png", TILE_WIDTH, TILE_HEIGHT);
}
static void gis_plugin_map_dispose(GObject *gobject)
{
	g_debug("GisPluginMap: dispose");
	GisPluginMap *map = GIS_PLUGIN_MAP(gobject);
	/* Drop references */
	if (map->viewer) {
		g_signal_handler_disconnect(map->viewer, map->sigid);
		g_object_unref(map->viewer);
		map->viewer = NULL;
	}
	G_OBJECT_CLASS(gis_plugin_map_parent_class)->dispose(gobject);
}
static void gis_plugin_map_finalize(GObject *gobject)
{
	g_debug("GisPluginMap: finalize");
	GisPluginMap *map = GIS_PLUGIN_MAP(gobject);
	/* Free data */
	gis_tile_free(map->tiles, _free_tile, map);
	gis_wms_free(map->wms);
	g_mutex_free(map->mutex);
	G_OBJECT_CLASS(gis_plugin_map_parent_class)->finalize(gobject);

}
static void gis_plugin_map_class_init(GisPluginMapClass *klass)
{
	g_debug("GisPluginMap: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose  = gis_plugin_map_dispose;
	gobject_class->finalize = gis_plugin_map_finalize;
}
