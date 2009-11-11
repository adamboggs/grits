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

#include <time.h>
#include <GL/gl.h>

#include <gis.h>

#include "bmng.h"

#define MAX_RESOLUTION 500
#define TILE_WIDTH     1024
#define TILE_HEIGHT    512

struct _LoadTileData {
	GisPluginBmng *self;
	GisTile       *tile;
	GdkPixbuf     *pixbuf;
};
static gboolean _load_tile_cb(gpointer _data)
{
	struct _LoadTileData *data = _data;
	GisPluginBmng *self   = data->self;
	GisTile       *tile   = data->tile;
	GdkPixbuf     *pixbuf = data->pixbuf;
	g_free(data);

	/* Create Texture */
	g_debug("GisPluginBmng: _load_tile_cb start");
	guchar   *pixels = gdk_pixbuf_get_pixels(pixbuf);
	gboolean  alpha  = gdk_pixbuf_get_has_alpha(pixbuf);
	gint      width  = gdk_pixbuf_get_width(pixbuf);
	gint      height = gdk_pixbuf_get_height(pixbuf);

	guint *tex = g_new0(guint, 1);
	gis_opengl_begin(self->opengl);
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
	gis_opengl_end(self->opengl);

	tile->data = tex;
	gis_opengl_redraw(self->opengl);
	g_object_unref(pixbuf);
	return FALSE;
}

static void _load_tile(GisTile *tile, gpointer _self)
{
	GisPluginBmng *self = _self;
	g_debug("GisPluginBmng: _load_tile start %p", g_thread_self());
	char *path = gis_wms_make_local(self->wms, tile);
	struct _LoadTileData *data = g_new0(struct _LoadTileData, 1);
	data->self   = self;
	data->tile   = tile;
	data->pixbuf = gdk_pixbuf_new_from_file(path, NULL);
	if (!data->pixbuf)
		g_warning("GisPluginBmng: _load_tile - Error loading pixbuf %s", path);
	g_free(path);
	g_idle_add_full(G_PRIORITY_LOW, _load_tile_cb, data, NULL);
	g_debug("GisPluginBmng: _load_tile end %p", g_thread_self());
}

static gboolean _free_tile_cb(gpointer data)
{
	glDeleteTextures(1, data);
	g_free(data);
	return FALSE;
}
static void _free_tile(GisTile *tile, gpointer _self)
{
	GisPluginBmng *self = _self;
	g_debug("GisPluginBmng: _free_tile: %p=%d", tile->data, *(guint*)tile->data);
	g_idle_add_full(G_PRIORITY_LOW, _free_tile_cb, tile->data, NULL);
}

static gpointer _update_tiles(gpointer _self)
{
	g_debug("GisPluginBmng: _update_tiles");
	GisPluginBmng *self = _self;
	g_mutex_lock(self->mutex);
	gdouble lat, lon, elev;
	gis_view_get_location(self->view, &lat, &lon, &elev);
	gis_tile_update(self->tiles,
			MAX_RESOLUTION, TILE_WIDTH, TILE_WIDTH,
			lat, lon, elev,
			_load_tile, self);
	gis_tile_gc(self->tiles, time(NULL)-10,
			_free_tile, self);
	g_mutex_unlock(self->mutex);
	return NULL;
}

/*************
 * Callbacks *
 *************/
static void _on_location_changed(GisView *view, gdouble lat, gdouble lon, gdouble elev,
		GisPluginBmng *self)
{
	g_thread_create(_update_tiles, self, FALSE, NULL);
}

/***********
 * Methods *
 ***********/
GisPluginBmng *gis_plugin_bmng_new(GisWorld *world, GisView *view, GisOpenGL *opengl)
{
	g_debug("GisPluginBmng: new");
	GisPluginBmng *self = g_object_new(GIS_TYPE_PLUGIN_BMNG, NULL);
	self->view   = view;
	self->opengl = opengl;

	/* Load initial tiles */
	_load_tile(self->tiles, self);
	g_thread_create(_update_tiles, self, FALSE, NULL);

	/* Connect signals */
	self->sigid = g_signal_connect(self->view, "location-changed",
			G_CALLBACK(_on_location_changed), self);

	return self;
}

static void gis_plugin_bmng_expose(GisPlugin *_self)
{
	GisPluginBmng *self = GIS_PLUGIN_BMNG(_self);
	g_debug("GisPluginBmng: expose opengl=%p tiles=%p,%p",
			self->opengl, self->tiles, self->tiles->data);
	gis_opengl_render_tiles(self->opengl, self->tiles);
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void gis_plugin_bmng_plugin_init(GisPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GisPluginBmng, gis_plugin_bmng, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GIS_TYPE_PLUGIN,
			gis_plugin_bmng_plugin_init));
static void gis_plugin_bmng_plugin_init(GisPluginInterface *iface)
{
	g_debug("GisPluginBmng: plugin_init");
	/* Add methods to the interface */
	iface->expose = gis_plugin_bmng_expose;
}
/* Class/Object init */
static void gis_plugin_bmng_init(GisPluginBmng *self)
{
	g_debug("GisPluginBmng: init");
	/* Set defaults */
	self->mutex = g_mutex_new();
	self->tiles = gis_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	self->wms   = gis_wms_new(
		"http://www.nasa.network.com/wms", "bmng200406", "image/jpeg",
		"bmng/", "jpg", TILE_WIDTH, TILE_HEIGHT);
}
static void gis_plugin_bmng_dispose(GObject *gobject)
{
	g_debug("GisPluginBmng: dispose");
	GisPluginBmng *self = GIS_PLUGIN_BMNG(gobject);
	/* Drop references */
	g_signal_handler_disconnect(self->view, self->sigid);
	G_OBJECT_CLASS(gis_plugin_bmng_parent_class)->dispose(gobject);
}
static void gis_plugin_bmng_finalize(GObject *gobject)
{
	g_debug("GisPluginBmng: finalize");
	GisPluginBmng *self = GIS_PLUGIN_BMNG(gobject);
	/* Free data */
	gis_tile_free(self->tiles, _free_tile, self);
	gis_wms_free(self->wms);
	g_mutex_free(self->mutex);
	G_OBJECT_CLASS(gis_plugin_bmng_parent_class)->finalize(gobject);

}
static void gis_plugin_bmng_class_init(GisPluginBmngClass *klass)
{
	g_debug("GisPluginBmng: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose  = gis_plugin_bmng_dispose;
	gobject_class->finalize = gis_plugin_bmng_finalize;
}
