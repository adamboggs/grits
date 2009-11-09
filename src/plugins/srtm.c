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

#include <gtk/gtkgl.h>
#include <GL/gl.h>

#include <gis.h>

#include "srtm.h"

#define MAX_RESOLUTION 500
#define TILE_WIDTH     1024
#define TILE_HEIGHT    512

struct _TileData {
	/* OpenGL has to be first to make gis_opengl_render_tiles happy */
	guint      opengl;
	guint16   *bil;
};

static gdouble _height_func(gdouble lat, gdouble lon, gpointer _self)
{
	GisPluginSrtm *self = _self;
	if (!self) return 0;

	GisTile *tile = gis_tile_find(self->tiles, lat, lon);
	if (!tile) return 0;

	struct _TileData *data = tile->data;
	if (!data) return 0;

	guint16 *bil  = data->bil;
	if (!bil)  return 0;

	gint w = TILE_WIDTH;
	gint h = TILE_HEIGHT;

	gdouble ymin  = tile->edge.s;
	gdouble ymax  = tile->edge.n;
	gdouble xmin  = tile->edge.w;
	gdouble xmax  = tile->edge.e;

	gdouble xdist = xmax - xmin;
	gdouble ydist = ymax - ymin;

	gdouble x =    (lon-xmin)/xdist  * w;
	gdouble y = (1-(lat-ymin)/ydist) * h;

	gdouble x_rem = x - (int)x;
	gdouble y_rem = y - (int)y;
	guint x_flr = (int)x;
	guint y_flr = (int)y;

	/* TODO: Fix interpolation at edges:
	 *   - Pad these at the edges instead of wrapping/truncating
	 *   - Figure out which pixels to index (is 0,0 edge, center, etc) */
	gint16 px00 = bil[MIN((y_flr  ),h-1)*w + MIN((x_flr  ),w-1)];
	gint16 px10 = bil[MIN((y_flr  ),h-1)*w + MIN((x_flr+1),w-1)];
	gint16 px01 = bil[MIN((y_flr+1),h-1)*w + MIN((x_flr  ),w-1)];
	gint16 px11 = bil[MIN((y_flr+1),h-1)*w + MIN((x_flr+1),w-1)];

	gdouble elev =
		px00 * (1-x_rem) * (1-y_rem) +
		px10 * (  x_rem) * (1-y_rem) +
		px01 * (1-x_rem) * (  y_rem) +
		px11 * (  x_rem) * (  y_rem);
	return elev;
}

/**********************
 * Loader and Freeers *
 **********************/
#define LOAD_BIL    TRUE
#define LOAD_OPENGL TRUE
static guint16 *_load_bil(gchar *path)
{
	gchar *data;
	g_file_get_contents(path, &data, NULL, NULL);
	g_debug("GisPluginSrtm: load_bil %p", data);
	return (guint16*)data;
}
static GdkPixbuf *_load_pixbuf(guint16 *bil)
{
	GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, TILE_WIDTH, TILE_HEIGHT);
	guchar    *pixels = gdk_pixbuf_get_pixels(pixbuf);
	gint       stride = gdk_pixbuf_get_rowstride(pixbuf);
	gint       nchan  = gdk_pixbuf_get_n_channels(pixbuf);

	for (int r = 0; r < TILE_HEIGHT; r++) {
		for (int c = 0; c < TILE_WIDTH; c++) {
			gint16 value = bil[r*TILE_WIDTH + c];
			//guchar color = (float)(MAX(value,0))/8848 * 255;
			guchar color = (float)value/8848 * 255;
			pixels[r*stride + c*nchan + 0] = color;
			pixels[r*stride + c*nchan + 1] = color;
			pixels[r*stride + c*nchan + 2] = color;
			if (nchan == 4)
				pixels[r*stride + c*nchan + 3] = 128;
		}
	}
	g_debug("GisPluginSrtm: load_pixbuf %p", pixbuf);
	return pixbuf;
}
static guint _load_opengl(GdkPixbuf *pixbuf)
{
	/* Load image */
	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
	gint    alpha  = gdk_pixbuf_get_has_alpha(pixbuf);
	gint    nchan  = 4; // gdk_pixbuf_get_n_channels(pixbuf);
	gint    width  = gdk_pixbuf_get_width(pixbuf);
	gint    height = gdk_pixbuf_get_height(pixbuf);

	/* Create Texture */
	guint opengl;
	glGenTextures(1, &opengl);
	glBindTexture(GL_TEXTURE_2D, opengl);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, nchan, width, height, 0,
			(alpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, pixels);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	g_debug("GisPluginSrtm: load_opengl %d", opengl);
	return opengl;
}
static void _load_tile(GisTile *tile, gpointer _self)
{
	GisPluginSrtm *self = _self;
	g_debug("GisPluginSrtm: _load_tile");

	struct _TileData *data = g_new0(struct _TileData, 1);
	gchar *path = gis_wms_make_local(self->wms, tile);
	if (LOAD_BIL || LOAD_OPENGL)
		data->bil = _load_bil(path);
	g_free(path);
	if (LOAD_OPENGL) {
		GdkPixbuf *pixbuf = _load_pixbuf(data->bil);
		data->opengl = _load_opengl(pixbuf);
		g_object_unref(pixbuf);
	}

	/* Do necessasairy processing */
	if (LOAD_BIL)
		gis_opengl_set_height_func(self->opengl, tile,
			_height_func, self, TRUE);

	/* Cleanup unneeded things */
	if (!LOAD_BIL)
		g_free(data->bil);

	tile->data = data;
}

static void _free_tile(GisTile *tile, gpointer _self)
{
	GisPluginSrtm *self = _self;
	struct _TileData *data = tile->data;
	g_debug("GisPluginSrtm: _free_tile: %p=%d", data, data->opengl);
	if (LOAD_BIL)
		g_free(data->bil);
	if (LOAD_OPENGL)
		glDeleteTextures(1, &data->opengl);
	g_free(data);
}

static gpointer _update_tiles(gpointer _self)
{
	GisPluginSrtm *self = _self;
	gdouble lat, lon, elev;
	gis_view_get_location(self->view, &lat, &lon, &elev);
	gis_tile_update(self->tiles,
			MAX_RESOLUTION, TILE_WIDTH, TILE_WIDTH,
			lat, lon, elev,
			_load_tile, self);
	gis_tile_gc(self->tiles, time(NULL)-10,
			_free_tile, self);
	return NULL;
}

/*************
 * Callbacks *
 *************/
static void _on_location_changed(GisView *view, gdouble lat, gdouble lon, gdouble elev,
		GisPluginSrtm *self)
{
	_update_tiles(self);
}

/***********
 * Methods *
 ***********/
GisPluginSrtm *gis_plugin_srtm_new(GisWorld *world, GisView *view, GisOpenGL *opengl)
{
	g_debug("GisPluginSrtm: new");
	GisPluginSrtm *self = g_object_new(GIS_TYPE_PLUGIN_SRTM, NULL);
	self->view   = view;
	self->opengl = opengl;

	/* Load initial tiles */
	_load_tile(self->tiles, self);
	_update_tiles(self);

	/* Connect signals */
	g_signal_connect(view, "location-changed", G_CALLBACK(_on_location_changed), self);

	return self;
}

static void gis_plugin_srtm_expose(GisPlugin *_self)
{
	GisPluginSrtm *self = GIS_PLUGIN_SRTM(_self);
	g_debug("GisPluginSrtm: expose tiles=%p data=%p",
		self->tiles, self->tiles->data);
	if (LOAD_OPENGL)
		gis_opengl_render_tiles(self->opengl, self->tiles);
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void gis_plugin_srtm_plugin_init(GisPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GisPluginSrtm, gis_plugin_srtm, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GIS_TYPE_PLUGIN,
			gis_plugin_srtm_plugin_init));
static void gis_plugin_srtm_plugin_init(GisPluginInterface *iface)
{
	g_debug("GisPluginSrtm: plugin_init");
	/* Add methods to the interface */
	iface->expose = gis_plugin_srtm_expose;
}
/* Class/Object init */
static void gis_plugin_srtm_init(GisPluginSrtm *self)
{
	g_debug("GisPluginSrtm: init");
	/* Set defaults */
	self->tiles = gis_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	self->wms   = gis_wms_new(
		"http://www.nasa.network.com/elev", "srtm30", "application/bil",
		"srtm", ".bil", TILE_WIDTH, TILE_HEIGHT);
}
static void gis_plugin_srtm_dispose(GObject *gobject)
{
	g_debug("GisPluginSrtm: dispose");
	GisPluginSrtm *self = GIS_PLUGIN_SRTM(gobject);
	/* Drop references */
	G_OBJECT_CLASS(gis_plugin_srtm_parent_class)->dispose(gobject);
}
static void gis_plugin_srtm_finalize(GObject *gobject)
{
	g_debug("GisPluginSrtm: finalize");
	GisPluginSrtm *self = GIS_PLUGIN_SRTM(gobject);
	/* Free data */
	gis_tile_free(self->tiles, _free_tile, self);
	gis_wms_free(self->wms);
	G_OBJECT_CLASS(gis_plugin_srtm_parent_class)->finalize(gobject);

}
static void gis_plugin_srtm_class_init(GisPluginSrtmClass *klass)
{
	g_debug("GisPluginSrtm: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose  = gis_plugin_srtm_dispose;
	gobject_class->finalize = gis_plugin_srtm_finalize;
}
