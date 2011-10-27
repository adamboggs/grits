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
 * SECTION:sat
 * @short_description: Satellite plugin
 *
 * #GritsPluginSat provides overlays using satellite imagery. This is mostly
 * provided by NASA's Blue Marble Next Generation.
 */

#include <time.h>
#include <glib/gstdio.h>
#include <GL/gl.h>

#include <grits.h>

#include "sat.h"

#define MAX_RESOLUTION 500
#define TILE_WIDTH     1024
#define TILE_HEIGHT    512

struct _LoadTileData {
	GritsPluginSat *sat;
	GritsTile      *tile;
	guint8         *pixels;
	gboolean        alpha;
	gint            width;
	gint            height;
};
static gboolean _load_tile_cb(gpointer _data)
{
	struct _LoadTileData *data = _data;
	g_debug("GritsPluginSat: _load_tile_cb start");

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
	gtk_widget_queue_draw(GTK_WIDGET(data->sat->viewer));
	g_free(data->pixels);
	g_free(data);
	return FALSE;
}

static void _load_tile(GritsTile *tile, gpointer _sat)
{
	GritsPluginSat *sat = _sat;
	g_debug("GritsPluginSat: _load_tile start %p", g_thread_self());
	if (sat->aborted) {
		g_debug("GritsPluginSat: _load_tile - aborted");
		return;
	}

	/* Download tile */
	gchar *path = grits_wms_fetch(sat->wms, tile, GRITS_ONCE, NULL, NULL);
	if (!path) return; // Canceled/error

	/* Load pixbuf */
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
	if (!pixbuf) {
		g_warning("GritsPluginSat: _load_tile - Error loading pixbuf %s", path);
		g_remove(path);
		g_free(path);
		return;
	}
	g_free(path);

	/* Copy pixbuf data for callback */
	struct _LoadTileData *data = g_new0(struct _LoadTileData, 1);
	data->sat    = sat;
	data->tile   = tile;
	data->pixels = gdk_pixbuf_get_pixels(pixbuf);
	data->alpha  = gdk_pixbuf_get_has_alpha(pixbuf);
	data->width  = gdk_pixbuf_get_width(pixbuf);
	data->height = gdk_pixbuf_get_height(pixbuf);
	data->pixels = g_memdup(data->pixels,
			data->width * data->height * (data->alpha ? 4 : 3));
	g_object_unref(pixbuf);

	/* Draw a border */
	//gint border = 10;
	//gint stride = gdk_pixbuf_get_rowstride(pixbuf);
	//for (int i = 0; i < border; i++) {
	//	memset(&pixels[(       i)*stride], 0xff, stride);
	//	memset(&pixels[(height-i)*stride], 0xff, stride);
	//}
	//for (int i = 0; i < height; i++) {
	//	memset(&pixels[(i*stride)], 0xff, border*4);
	//	memset(&pixels[(i*stride)+((width-border)*4)], 0xff, border*4);
	//}

	/* Load the GL texture from the main thread */
	g_idle_add_full(G_PRIORITY_LOW, _load_tile_cb, data, NULL);
	g_debug("GritsPluginSat: _load_tile end %p", g_thread_self());
}

static gboolean _free_tile_cb(gpointer data)
{
	glDeleteTextures(1, data);
	g_free(data);
	return FALSE;
}
static void _free_tile(GritsTile *tile, gpointer _sat)
{
	g_debug("GritsPluginSat: _free_tile: %p", tile->data);
	if (tile->data)
		g_idle_add_full(G_PRIORITY_LOW, _free_tile_cb, tile->data, NULL);
}

static void _update_tiles(gpointer _, gpointer _sat)
{
	g_debug("GritsPluginSat: _update_tiles");
	GritsPluginSat *sat = _sat;
	GritsPoint eye;
	grits_viewer_get_location(sat->viewer, &eye.lat, &eye.lon, &eye.elev);
	grits_tile_update(sat->tiles, &eye,
			MAX_RESOLUTION, TILE_WIDTH, TILE_WIDTH,
			_load_tile, sat);
	grits_tile_gc(sat->tiles, time(NULL)-10,
			_free_tile, sat);
}

/*************
 * Callbacks *
 *************/
static void _on_location_changed(GritsViewer *viewer,
		gdouble lat, gdouble lon, gdouble elev, GritsPluginSat *sat)
{
	g_thread_pool_push(sat->threads, NULL+1, NULL);
}

/***********
 * Methods *
 ***********/
/**
 * grits_plugin_sat_new:
 * @viewer: the #GritsViewer to use for drawing
 *
 * Create a new instance of the satellite plugin.
 *
 * Returns: the new #GritsPluginSat
 */
GritsPluginSat *grits_plugin_sat_new(GritsViewer *viewer)
{
	g_debug("GritsPluginSat: new");
	GritsPluginSat *sat = g_object_new(GRITS_TYPE_PLUGIN_SAT, NULL);
	sat->viewer = g_object_ref(viewer);

	/* Load initial tiles */
	_load_tile(sat->tiles, sat);
	_update_tiles(NULL, sat);

	/* Connect signals */
	sat->sigid = g_signal_connect(sat->viewer, "location-changed",
			G_CALLBACK(_on_location_changed), sat);

	/* Add renderers */
	grits_viewer_add(viewer, GRITS_OBJECT(sat->tiles), GRITS_LEVEL_WORLD, FALSE);

	return sat;
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void grits_plugin_sat_plugin_init(GritsPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GritsPluginSat, grits_plugin_sat, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GRITS_TYPE_PLUGIN,
			grits_plugin_sat_plugin_init));
static void grits_plugin_sat_plugin_init(GritsPluginInterface *iface)
{
	g_debug("GritsPluginSat: plugin_init");
	/* Add methods to the interface */
}
/* Class/Object init */
static void grits_plugin_sat_init(GritsPluginSat *sat)
{
	g_debug("GritsPluginSat: init");
	/* Set defaults */
	sat->threads = g_thread_pool_new(_update_tiles, sat, 1, FALSE, NULL);
	sat->tiles = grits_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	sat->wms   = grits_wms_new(
		"http://www.nasa.network.com/wms", "bmng200406", "image/jpeg",
		"bmng/", "jpg", TILE_WIDTH, TILE_HEIGHT);
	g_object_ref(sat->tiles);
}
static void grits_plugin_sat_dispose(GObject *gobject)
{
	g_debug("GritsPluginSat: dispose");
	GritsPluginSat *sat = GRITS_PLUGIN_SAT(gobject);
	sat->aborted = TRUE;
	/* Drop references */
	if (sat->viewer) {
		g_signal_handler_disconnect(sat->viewer, sat->sigid);
		grits_viewer_remove(sat->viewer, GRITS_OBJECT(sat->tiles));
		soup_session_abort(sat->wms->http->soup);
		g_thread_pool_free(sat->threads, TRUE, TRUE);
		while (gtk_events_pending())
			gtk_main_iteration();
		g_object_unref(sat->viewer);
		sat->viewer = NULL;
	}
	G_OBJECT_CLASS(grits_plugin_sat_parent_class)->dispose(gobject);
}
static void grits_plugin_sat_finalize(GObject *gobject)
{
	g_debug("GritsPluginSat: finalize");
	GritsPluginSat *sat = GRITS_PLUGIN_SAT(gobject);
	/* Free data */
	grits_wms_free(sat->wms);
	grits_tile_free(sat->tiles, _free_tile, sat);
	G_OBJECT_CLASS(grits_plugin_sat_parent_class)->finalize(gobject);

}
static void grits_plugin_sat_class_init(GritsPluginSatClass *klass)
{
	g_debug("GritsPluginSat: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose  = grits_plugin_sat_dispose;
	gobject_class->finalize = grits_plugin_sat_finalize;
}
