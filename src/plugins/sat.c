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

#include <time.h>
#include <glib/gstdio.h>
#include <GL/gl.h>

#include <gis.h>

#include "sat.h"

#define MAX_RESOLUTION 500
#define TILE_WIDTH     1024
#define TILE_HEIGHT    512

struct _LoadTileData {
	GisPluginSat *sat;
	GisTile      *tile;
	GdkPixbuf    *pixbuf;
};
static gboolean _load_tile_cb(gpointer _data)
{
	struct _LoadTileData *data = _data;
	GisPluginSat *sat    = data->sat;
	GisTile      *tile   = data->tile;
	GdkPixbuf    *pixbuf = data->pixbuf;
	g_free(data);

	/* Create Texture */
	g_debug("GisPluginSat: _load_tile_cb start");
	guchar   *pixels = gdk_pixbuf_get_pixels(pixbuf);
	gboolean  alpha  = gdk_pixbuf_get_has_alpha(pixbuf);
	gint      width  = gdk_pixbuf_get_width(pixbuf);
	gint      height = gdk_pixbuf_get_height(pixbuf);

	guint *tex = g_new0(guint, 1);
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
			(alpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, pixels);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFlush();

	tile->data = tex;
	gtk_widget_queue_draw(GTK_WIDGET(sat->viewer));
	g_object_unref(pixbuf);
	return FALSE;
}

static void _load_tile(GisTile *tile, gpointer _sat)
{
	GisPluginSat *sat = _sat;
	g_debug("GisPluginSat: _load_tile start %p", g_thread_self());
	char *path = gis_wms_fetch(sat->wms, tile, GIS_ONCE, NULL, NULL);
	struct _LoadTileData *data = g_new0(struct _LoadTileData, 1);
	data->sat   = sat;
	data->tile   = tile;
	data->pixbuf = gdk_pixbuf_new_from_file(path, NULL);
	if (data->pixbuf) {
		g_idle_add_full(G_PRIORITY_LOW, _load_tile_cb, data, NULL);
	} else {
		g_warning("GisPluginSat: _load_tile - Error loading pixbuf %s", path);
		g_free(data);
		g_remove(path);
	}
	g_free(path);
	g_debug("GisPluginSat: _load_tile end %p", g_thread_self());
}

static gboolean _free_tile_cb(gpointer data)
{
	glDeleteTextures(1, data);
	g_free(data);
	return FALSE;
}
static void _free_tile(GisTile *tile, gpointer _sat)
{
	GisPluginSat *sat = _sat;
	g_debug("GisPluginSat: _free_tile: %p=%d", tile->data, *(guint*)tile->data);
	g_idle_add_full(G_PRIORITY_LOW, _free_tile_cb, tile->data, NULL);
}

static gpointer _update_tiles(gpointer _sat)
{
	g_debug("GisPluginSat: _update_tiles");
	GisPluginSat *sat = _sat;
	g_mutex_lock(sat->mutex);
	gdouble lat, lon, elev;
	gis_viewer_get_location(sat->viewer, &lat, &lon, &elev);
	gis_tile_update(sat->tiles,
			MAX_RESOLUTION, TILE_WIDTH, TILE_WIDTH,
			lat, lon, elev,
			_load_tile, sat);
	gis_tile_gc(sat->tiles, time(NULL)-10,
			_free_tile, sat);
	g_mutex_unlock(sat->mutex);
	return NULL;
}

/*************
 * Callbacks *
 *************/
static void _on_location_changed(GisViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev, GisPluginSat *sat)
{
	g_thread_create(_update_tiles, sat, FALSE, NULL);
}

/***********
 * Methods *
 ***********/
GisPluginSat *gis_plugin_sat_new(GisViewer *viewer)
{
	g_debug("GisPluginSat: new");
	GisPluginSat *sat = g_object_new(GIS_TYPE_PLUGIN_SAT, NULL);
	sat->viewer = g_object_ref(viewer);

	/* Load initial tiles */
	_load_tile(sat->tiles, sat);
	g_thread_create(_update_tiles, sat, FALSE, NULL);

	/* Connect signals */
	sat->sigid = g_signal_connect(sat->viewer, "location-changed",
			G_CALLBACK(_on_location_changed), sat);

	/* Add renderers */
	gis_viewer_add(viewer, GIS_OBJECT(sat->tiles), GIS_LEVEL_WORLD, 0);

	return sat;
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void gis_plugin_sat_plugin_init(GisPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GisPluginSat, gis_plugin_sat, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GIS_TYPE_PLUGIN,
			gis_plugin_sat_plugin_init));
static void gis_plugin_sat_plugin_init(GisPluginInterface *iface)
{
	g_debug("GisPluginSat: plugin_init");
	/* Add methods to the interface */
}
/* Class/Object init */
static void gis_plugin_sat_init(GisPluginSat *sat)
{
	g_debug("GisPluginSat: init");
	/* Set defaults */
	sat->mutex = g_mutex_new();
	sat->tiles = gis_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	sat->wms   = gis_wms_new(
		"http://www.nasa.network.com/wms", "bmng200406", "image/jpeg",
		"bmng/", "jpg", TILE_WIDTH, TILE_HEIGHT);
}
static void gis_plugin_sat_dispose(GObject *gobject)
{
	g_debug("GisPluginSat: dispose");
	GisPluginSat *sat = GIS_PLUGIN_SAT(gobject);
	/* Drop references */
	if (sat->viewer) {
		g_signal_handler_disconnect(sat->viewer, sat->sigid);
		g_object_unref(sat->viewer);
		sat->viewer = NULL;
	}
	G_OBJECT_CLASS(gis_plugin_sat_parent_class)->dispose(gobject);
}
static void gis_plugin_sat_finalize(GObject *gobject)
{
	g_debug("GisPluginSat: finalize");
	GisPluginSat *sat = GIS_PLUGIN_SAT(gobject);
	/* Free data */
	gis_tile_free(sat->tiles, _free_tile, sat);
	gis_wms_free(sat->wms);
	g_mutex_free(sat->mutex);
	G_OBJECT_CLASS(gis_plugin_sat_parent_class)->finalize(gobject);

}
static void gis_plugin_sat_class_init(GisPluginSatClass *klass)
{
	g_debug("GisPluginSat: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose  = gis_plugin_sat_dispose;
	gobject_class->finalize = gis_plugin_sat_finalize;
}
