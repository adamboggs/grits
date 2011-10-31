/*
 * Copyright (C) 2009-2011 Andy Spencer <andy753421@gmail.com>
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
 * SECTION:grits-wms
 * @short_description: Web Map Service
 *
 * Provides an API for accessing image tiles form a Web Map Service (WMS)
 * server. #GritsWms integrates closely with #GritsTile. The remote server must
 * support the EPSG:4326 cartographic projection.
 */

/*
 * Example WMS URLS
 * ----------------
 *
 * Metacarte Open Street Map:
 *   http://labs.metacarta.com/wms/vmap0?
 *   LAYERS=basic&
 *   SERVICE=WMS&
 *   VERSION=1.1.1&
 *   REQUEST=GetMap&
 *   STYLES=&
 *   EXCEPTIONS=application/vnd.ogc.se_inimage&
 *   FORMAT=image/jpeg&
 *   SRS=EPSG:4326&
 *   BBOX=0,-90,180,90&
 *   WIDTH=256&
 *   HEIGHT=256
 *
 * NASA Blue Marble Next Generation:
 *   http://www.nasa.network.com/elev?
 *   SERVICE=WMS&
 *   VERSION=1.1.0&
 *   REQUEST=GetMap&
 *   LAYERS=bmng200406&
 *   STYLES=&
 *   SRS=EPSG:4326&
 *   BBOX=-180,-90,180,90&
 *   FORMAT=image/jpeg&
 *   WIDTH=600&
 *   HEIGHT=300
 *
 * NASA Shuttle Radar Topography Mission:
 *   http://www.nasa.network.com/elev?
 *   SERVICE=WMS&
 *   VERSION=1.1.0&
 *   REQUEST=GetMap&
 *   LAYERS=srtm30&
 *   STYLES=&
 *   SRS=EPSG:4326&
 *   BBOX=-180,-90,180,90&
 *   FORMAT=application/bil32&
 *   WIDTH=600&
 *   HEIGHT=300
 */

#include <config.h>
#include <stdio.h>
#include <glib.h>
#include <locale.h>

#include "grits-wms.h"
#include "grits-http.h"

static gchar *g_strdup_printf_safe(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	setlocale(LC_ALL, "POSIX.UTF-8");
	char *str = g_strdup_vprintf(fmt, ap);
	setlocale(LC_ALL, "");
	va_end(ap);
	return str;
}

static gchar *_make_uri(GritsWms *wms, GritsTile *tile)
{
	return g_strdup_printf_safe(
		"%s?"
		"SERVICE=WMS&"
		"VERSION=1.1.0&"
		"REQUEST=GetMap&"
		"LAYERS=%s&"
		"STYLES=&"
		"SRS=EPSG:4326&"
		"FORMAT=%s&"
		"WIDTH=%d&"
		"HEIGHT=%d&"
		"BBOX=%f,%f,%f,%f",
		wms->uri_prefix,
		wms->uri_layer,
		wms->uri_format,
		wms->width,
		wms->height,
		tile->edge.w,
		tile->edge.s,
		tile->edge.e,
		tile->edge.n);
}

/**
 * grits_wms_fetch:
 * @wms:       the #GritsWms to fetch the data from 
 * @tile:      a #GritsTile representing the area to be fetched 
 * @mode:      the update type to use when fetching data
 * @callback:  callback to call when a chunk of data is received
 * @user_data: user data to pass to the callback
 *
 * Fetch a image coresponding to a #GritsTile from a WMS server. 
 *
 * Returns: the path to the local file.
 */
gchar *grits_wms_fetch(GritsWms *wms, GritsTile *tile, GritsCacheType mode,
		GritsChunkCallback callback, gpointer user_data)
{
	gchar *uri   = _make_uri(wms, tile);
	gchar *tilep = grits_tile_get_path(tile);
	gchar *local = g_strdup_printf("%s%s", tilep, wms->extension);
	gchar *path  = grits_http_fetch(wms->http, uri, local,
			mode, callback, user_data);
	g_free(uri);
	g_free(tilep);
	g_free(local);
	return path;
}

/**
 * grits_wms_new:
 * @uri_prefix: the base URL for the WMS server
 * @uri_layer:  the layer the images should be fetched from (wms LAYERS)
 * @uri_format: the format the images should be fetch in (wms FORMAT)
 * @prefix:     prefix to use for local files
 * @extension:  file extension for local files, should correspond to @uri_format
 * @width:      width in pixels for downloaded images (wms WIDTH)
 * @height:     height in pixels for downloaded images (wms HEIGHT)
 *
 * Creates a #GritsWms for some layer on a WMS server. The returned #GritsWms
 * stores information about the images so it does not need to be entered each
 * time a images is fetched.
 *
 * Returns: the new #GritsWms
 */
GritsWms *grits_wms_new(
	const gchar *uri_prefix, const gchar *uri_layer,
	const gchar *uri_format, const gchar *prefix,
	const gchar *extension, gint width, gint height)
{
	g_debug("GritsWms: new - %s", uri_prefix);
	GritsWms *wms = g_new0(GritsWms, 1);
	wms->http       = grits_http_new(prefix);
	wms->uri_prefix = g_strdup(uri_prefix);
	wms->uri_layer  = g_strdup(uri_layer);
	wms->uri_format = g_strdup(uri_format);
	wms->extension  = g_strdup(extension);
	wms->width      = width;
	wms->height     = height;
	return wms;
}

/**
 * grits_wms_free:
 * @wms: the #GritsWms to free
 *
 * Free resources used by @wms and cancel any pending requests.
 */
void grits_wms_free(GritsWms *wms)
{
	g_debug("GritsWms: free - %s", wms->uri_prefix);
	grits_http_free(wms->http);
	g_free(wms->uri_prefix);
	g_free(wms->uri_layer);
	g_free(wms->uri_format);
	g_free(wms->extension);
	g_free(wms);
}
