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
 * #GritsPluginMap provides map overlays. Much of this data is obtained from the
 * OpenStreetMap project.
 */

#include <time.h>
#include <glib/gstdio.h>
#include <GL/gl.h>

#include <grits.h>

#include "map.h"

#define MAX_RESOLUTION 100
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
	GritsPluginMap *map;
	GritsTile      *tile;
	guint8         *pixels;
	gboolean        alpha;
	gint            width;
	gint            height;
};
static gboolean _load_tile_cb(gpointer _data)
{
	struct _LoadTileData *data = _data;
	g_debug("GritsPluginMap: _load_tile_cb start");

	guint *tex = g_new0(guint, 1);
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, data->width, data->height, 0,
			(data->alpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data->pixels);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glFlush();

	data->tile->data = tex;
	gtk_widget_queue_draw(GTK_WIDGET(data->map->viewer));
	g_free(data->pixels);
	g_free(data);
	return FALSE;
}

static void _load_tile(GritsTile *tile, gpointer _map)
{
	GritsPluginMap *map = _map;
	g_debug("GritsPluginMap: _load_tile start %p", g_thread_self());
	if (map->aborted) {
		g_debug("GritsPluginMap: _load_tile - aborted");
		return;
	}

	/* Download tile */
	gchar *path = grits_wms_fetch(map->wms, tile, GRITS_ONCE, NULL, NULL);
	if (!path) return; // Canceled/error

	/* Load pixbuf */
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
	if (!pixbuf) {
		g_warning("GritsPluginMap: _load_tile - Error loading pixbuf %s", path);
		g_remove(path);
		g_free(path);
		return;
	}
	g_free(path);

	/* Copy pixbuf data for callback */
	struct _LoadTileData *data = g_new0(struct _LoadTileData, 1);
	data->map    = map;
	data->tile   = tile;
	data->pixels = gdk_pixbuf_get_pixels(pixbuf);
	data->alpha  = gdk_pixbuf_get_has_alpha(pixbuf);
	data->width  = gdk_pixbuf_get_width(pixbuf);
	data->height = gdk_pixbuf_get_height(pixbuf);
	data->pixels = g_memdup(data->pixels,
			data->width * data->height * (data->alpha ? 4 : 3));
	g_object_unref(pixbuf);

	/* Map texture colors, if needed */
	for (int i = 0; i < data->width * data->height; i++) {
		for (int j = 0; j < G_N_ELEMENTS(colormap); j++) {
			if (data->pixels[i*4+0] == colormap[j][0][0] &&
			    data->pixels[i*4+1] == colormap[j][0][1] &&
			    data->pixels[i*4+2] == colormap[j][0][2]) {
				data->pixels[i*4+0] = colormap[j][1][0];
				data->pixels[i*4+1] = colormap[j][1][1];
				data->pixels[i*4+2] = colormap[j][1][2];
				data->pixels[i*4+3] = colormap[j][1][3];
				break;
			}
		}
	}

	/* Load the GL texture from the main thread */
	g_idle_add_full(G_PRIORITY_LOW, _load_tile_cb, data, NULL);
	g_debug("GritsPluginMap: _load_tile end %p", g_thread_self());
}

static gboolean _free_tile_cb(gpointer data)
{
	glDeleteTextures(1, data);
	g_free(data);
	return FALSE;
}
static void _free_tile(GritsTile *tile, gpointer _map)
{
	g_debug("GritsPluginMap: _free_tile: %p", tile->data);
	if (tile->data)
		g_idle_add_full(G_PRIORITY_LOW, _free_tile_cb, tile->data, NULL);
}

static void _update_tiles(gpointer _, gpointer _map)
{
	g_debug("GritsPluginMap: _update_tiles");
	GritsPluginMap *map = _map;
	GritsPoint eye;
	grits_viewer_get_location(map->viewer, &eye.lat, &eye.lon, &eye.elev);
	grits_tile_update(map->tiles, &eye,
			MAX_RESOLUTION, TILE_WIDTH, TILE_WIDTH,
			_load_tile, map);
	grits_tile_gc(map->tiles, time(NULL)-10,
			_free_tile, map);
}

/*************
 * Callbacks *
 *************/
static void _on_location_changed(GritsViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev, GritsPluginMap *map)
{
	g_thread_pool_push(map->threads, NULL+1, NULL);
}

/***********
 * Methods *
 ***********/
/**
 * grits_plugin_map_new:
 * @viewer: the #GritsViewer to use for drawing
 *
 * Create a new instance of the map plugin.
 *
 * Returns: the new #GritsPluginMap
 */
GritsPluginMap *grits_plugin_map_new(GritsViewer *viewer)
{
	g_debug("GritsPluginMap: new");
	GritsPluginMap *map = g_object_new(GRITS_TYPE_PLUGIN_MAP, NULL);
	map->viewer = g_object_ref(viewer);

	/* Load initial tiles */
	_load_tile(map->tiles, map);
	_update_tiles(NULL, map);

	/* Connect signals */
	map->sigid = g_signal_connect(map->viewer, "location-changed",
			G_CALLBACK(_on_location_changed), map);

	/* Add renderers */
	grits_viewer_add(viewer, GRITS_OBJECT(map->tiles), GRITS_LEVEL_OVERLAY-1, 0);

	return map;
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void grits_plugin_map_plugin_init(GritsPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GritsPluginMap, grits_plugin_map, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GRITS_TYPE_PLUGIN,
			grits_plugin_map_plugin_init));
static void grits_plugin_map_plugin_init(GritsPluginInterface *iface)
{
	g_debug("GritsPluginMap: plugin_init");
	/* Add methods to the interface */
}
/* Class/Object init */
static void grits_plugin_map_init(GritsPluginMap *map)
{
	g_debug("GritsPluginMap: init");
	/* Set defaults */
	map->threads = g_thread_pool_new(_update_tiles, map, 1, FALSE, NULL);
	map->tiles = grits_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	map->wms   = grits_wms_new(
		"http://vmap0.tiles.osgeo.org/wms/vmap0",
		"basic,priroad,secroad,depthcontour,clabel,statelabel",
		 "image/png", "osm/", "png", TILE_WIDTH, TILE_HEIGHT);
	g_object_ref(map->tiles);
}
static void grits_plugin_map_dispose(GObject *gobject)
{
	g_debug("GritsPluginMap: dispose");
	GritsPluginMap *map = GRITS_PLUGIN_MAP(gobject);
	map->aborted = TRUE;
	/* Drop references */
	if (map->viewer) {
		g_signal_handler_disconnect(map->viewer, map->sigid);
		grits_viewer_remove(map->viewer, GRITS_OBJECT(map->tiles));
		soup_session_abort(map->wms->http->soup);
		g_thread_pool_free(map->threads, TRUE, TRUE);
		while (gtk_events_pending())
			gtk_main_iteration();
		g_object_unref(map->viewer);
		map->viewer = NULL;
	}
	G_OBJECT_CLASS(grits_plugin_map_parent_class)->dispose(gobject);
}
static void grits_plugin_map_finalize(GObject *gobject)
{
	g_debug("GritsPluginMap: finalize");
	GritsPluginMap *map = GRITS_PLUGIN_MAP(gobject);
	/* Free data */
	grits_wms_free(map->wms);
	grits_tile_free(map->tiles, _free_tile, map);
	G_OBJECT_CLASS(grits_plugin_map_parent_class)->finalize(gobject);

}
static void grits_plugin_map_class_init(GritsPluginMapClass *klass)
{
	g_debug("GritsPluginMap: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose  = grits_plugin_map_dispose;
	gobject_class->finalize = grits_plugin_map_finalize;
}
