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
 * SECTION:elev
 * @short_description: Elevation plugin
 *
 * #GritsPluginElev provides access to ground elevation. It does this in two ways:
 * First, it provides a height function used by the viewer when drawing the
 * world. Second, it can load the elevation data into an image and draw a
 * greyscale elevation overlay on the planets surface.
 */

#include <glib/gstdio.h>
#include <GL/gl.h>

#include <grits.h>

#include "elev.h"

#define MAX_RESOLUTION 50
#define TILE_WIDTH     1024
#define TILE_HEIGHT    512
#define TILE_SIZE      (TILE_WIDTH*TILE_HEIGHT*sizeof(guint16))

struct _TileData {
	/* OpenGL has to be first to make grits_opengl_render_tiles happy */
	guint      opengl;
	guint16   *bil;
};

static gdouble _height_func(gdouble lat, gdouble lon, gpointer _elev)
{
	GritsPluginElev *elev = _elev;
	if (!elev) return 0;

	GritsTile *tile = grits_tile_find(elev->tiles, lat, lon);
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

	//if (lon == 180 || lon == -180)
	//	g_message("lon=%f w=%d min=%f max=%f dist=%f x=%f rem=%f flr=%d",
	//	           lon,   w,  xmin,  xmax,  xdist,   x, x_rem, x_flr);

	/* TODO: Fix interpolation at edges:
	 *   - Pad these at the edges instead of wrapping/truncating
	 *   - Figure out which pixels to index (is 0,0 edge, center, etc) */
	gint16 px00 = bil[MIN((y_flr  ),h-1)*w + MIN((x_flr  ),w-1)];
	gint16 px10 = bil[MIN((y_flr  ),h-1)*w + MIN((x_flr+1),w-1)];
	gint16 px01 = bil[MIN((y_flr+1),h-1)*w + MIN((x_flr  ),w-1)];
	gint16 px11 = bil[MIN((y_flr+1),h-1)*w + MIN((x_flr+1),w-1)];

	return px00 * (1-x_rem) * (1-y_rem) +
	       px10 * (  x_rem) * (1-y_rem) +
	       px01 * (1-x_rem) * (  y_rem) +
	       px11 * (  x_rem) * (  y_rem);
}

/**********************
 * Loader and Freeers *
 **********************/
#define LOAD_BIL    TRUE
#define LOAD_OPENGL FALSE
struct _LoadTileData {
	GritsPluginElev    *elev;
	gchar            *path;
	GritsTile          *tile;
	GdkPixbuf        *pixbuf;
	struct _TileData *data;
};
static guint16 *_load_bil(gchar *path)
{
	gsize len;
	gchar *data = NULL;
	g_file_get_contents(path, &data, &len, NULL);
	g_debug("GritsPluginElev: load_bil %p", data);
	if (len != TILE_SIZE) {
		g_warning("GritsPluginElev: _load_bil - unexpected tile size %d, != %d",
				len, TILE_SIZE);
		g_free(data);
		return NULL;
	}
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
	g_debug("GritsPluginElev: load_pixbuf %p", pixbuf);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	g_debug("GritsPluginElev: load_opengl %d", opengl);
	return opengl;
}
static gboolean _load_tile_cb(gpointer _load)
{
	struct _LoadTileData *load = _load;
	g_debug("GritsPluginElev: _load_tile_cb: %s", load->path);
	GritsPluginElev    *elev   = load->elev;
	GritsTile          *tile   = load->tile;
	GdkPixbuf        *pixbuf = load->pixbuf;
	struct _TileData *data   = load->data;
	g_free(load->path);
	g_free(load);

	if (LOAD_OPENGL)
		data->opengl = _load_opengl(pixbuf);

	tile->data = data;

	/* Do necessasairy processing */
	/* TODO: Lock this and move to thread, can remove elev from _load then */
	if (LOAD_BIL)
		grits_viewer_set_height_func(elev->viewer, &tile->edge,
				_height_func, elev, TRUE);

	/* Cleanup unneeded things */
	if (!LOAD_BIL)
		g_free(data->bil);
	if (LOAD_OPENGL)
		g_object_unref(pixbuf);

	return FALSE;
}
static void _load_tile(GritsTile *tile, gpointer _elev)
{
	GritsPluginElev *elev = _elev;

	struct _LoadTileData *load = g_new0(struct _LoadTileData, 1);
	load->path = grits_wms_fetch(elev->wms, tile, GRITS_ONCE, NULL, NULL);
	if (!load->path) return; // Canceled/error
	g_debug("GritsPluginElev: _load_tile: %s", load->path);
	load->elev = elev;
	load->tile = tile;
	load->data = g_new0(struct _TileData, 1);
	if (LOAD_BIL || LOAD_OPENGL) {
		load->data->bil = _load_bil(load->path);
		if (!load->data->bil) {
			g_remove(load->path);
			g_free(load->data);
			g_free(load->path);
			g_free(load);
			return;
		}
	}
	if (LOAD_OPENGL) {
		load->pixbuf = _load_pixbuf(load->data->bil);
	}

	g_idle_add_full(G_PRIORITY_LOW, _load_tile_cb, load, NULL);
}

static gboolean _free_tile_cb(gpointer _data)
{
	struct _TileData *data = _data;
	if (LOAD_BIL)
		g_free(data->bil);
	if (LOAD_OPENGL)
		glDeleteTextures(1, &data->opengl);
	g_free(data);
	return FALSE;
}
static void _free_tile(GritsTile *tile, gpointer _elev)
{
	g_debug("GritsPluginElev: _free_tile: %p", tile->data);
	if (tile->data)
		g_idle_add_full(G_PRIORITY_LOW, _free_tile_cb, tile->data, NULL);
}

static gpointer _update_tiles(gpointer _elev)
{
	GritsPluginElev *elev = _elev;
	if (!g_mutex_trylock(elev->mutex))
		return NULL;
	GritsPoint eye;
	grits_viewer_get_location(elev->viewer, &eye.lat, &eye.lon, &eye.elev);
	grits_tile_update(elev->tiles, &eye,
			MAX_RESOLUTION, TILE_WIDTH, TILE_WIDTH,
			_load_tile, elev);
	grits_tile_gc(elev->tiles, time(NULL)-10,
			_free_tile, elev);
	g_mutex_unlock(elev->mutex);
	return NULL;
}

/*************
 * Callbacks *
 *************/
static void _on_location_changed(GritsViewer *viewer,
		gdouble lat, gdouble lon, gdouble elevation, GritsPluginElev *elev)
{
	g_thread_create(_update_tiles, elev, FALSE, NULL);
}

static gpointer _threaded_init(GritsPluginElev *elev)
{
	_load_tile(elev->tiles, elev);
	_update_tiles(elev);
	return NULL;
}

/***********
 * Methods *
 ***********/
/**
 * grits_plugin_elev_new:
 * @viewer: the #GritsViewer to use for drawing
 *
 * Create a new instance of the elevation plugin.
 *
 * Returns: the new #GritsPluginElev
 */
GritsPluginElev *grits_plugin_elev_new(GritsViewer *viewer)
{
	g_debug("GritsPluginElev: new");
	GritsPluginElev *elev = g_object_new(GRITS_TYPE_PLUGIN_ELEV, NULL);
	elev->viewer = g_object_ref(viewer);

	/* Load initial tiles */
	g_thread_create((GThreadFunc)_threaded_init, elev, FALSE, NULL);

	/* Connect signals */
	elev->sigid = g_signal_connect(elev->viewer, "location-changed",
			G_CALLBACK(_on_location_changed), elev);

	/* Add renderers */
	if (LOAD_OPENGL)
		grits_viewer_add(viewer, GRITS_OBJECT(elev->tiles), GRITS_LEVEL_WORLD, 0);

	return elev;
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void grits_plugin_elev_plugin_init(GritsPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GritsPluginElev, grits_plugin_elev, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GRITS_TYPE_PLUGIN,
			grits_plugin_elev_plugin_init));
static void grits_plugin_elev_plugin_init(GritsPluginInterface *iface)
{
	g_debug("GritsPluginElev: plugin_init");
	/* Add methods to the interface */
}
/* Class/Object init */
static void grits_plugin_elev_init(GritsPluginElev *elev)
{
	g_debug("GritsPluginElev: init");
	/* Set defaults */
	elev->mutex = g_mutex_new();
	elev->tiles = grits_tile_new(NULL, NORTH, SOUTH, EAST, WEST);
	elev->wms   = grits_wms_new(
		"http://www.nasa.network.com/elev", "mergedSrtm", "application/bil",
		"srtm/", "bil", TILE_WIDTH, TILE_HEIGHT);
}
static void grits_plugin_elev_dispose(GObject *gobject)
{
	g_debug("GritsPluginElev: dispose");
	GritsPluginElev *elev = GRITS_PLUGIN_ELEV(gobject);
	/* Drop references */
	if (elev->viewer) {
		if (LOAD_BIL)
			grits_viewer_clear_height_func(elev->viewer);
		if (LOAD_OPENGL)
			grits_viewer_remove(elev->viewer, elev->tiles);
		g_signal_handler_disconnect(elev->viewer, elev->sigid);
		g_object_unref(elev->viewer);
		elev->viewer = NULL;
	}
	G_OBJECT_CLASS(grits_plugin_elev_parent_class)->dispose(gobject);
}
static void grits_plugin_elev_finalize(GObject *gobject)
{
	g_debug("GritsPluginElev: finalize");
	GritsPluginElev *elev = GRITS_PLUGIN_ELEV(gobject);
	/* Free data */
	grits_tile_free(elev->tiles, _free_tile, elev);
	grits_wms_free(elev->wms);
	g_mutex_lock(elev->mutex);
	g_mutex_unlock(elev->mutex);
	g_mutex_free(elev->mutex);
	G_OBJECT_CLASS(grits_plugin_elev_parent_class)->finalize(gobject);

}
static void grits_plugin_elev_class_init(GritsPluginElevClass *klass)
{
	g_debug("GritsPluginElev: class_init");
	GObjectClass *gobject_class = (GObjectClass*)klass;
	gobject_class->dispose  = grits_plugin_elev_dispose;
	gobject_class->finalize = grits_plugin_elev_finalize;
}
