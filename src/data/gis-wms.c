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
 * Metacarte
 * ---------
 * http://labs.metacarta.com/wms/vmap0?
 * LAYERS=basic&
 * SERVICE=WMS&
 * VERSION=1.1.1&
 * REQUEST=GetMap&
 * STYLES=&
 * EXCEPTIONS=application/vnd.ogc.se_inimage&
 * FORMAT=image/jpeg&
 * SRS=EPSG:4326&
 * BBOX=0,-90,180,90&
 * WIDTH=256&
 * HEIGHT=256
 */

/**
 * http://www.nasa.network.com/elev?
 * SERVICE=WMS&
 * VERSION=1.1.0&
 * REQUEST=GetMap&
 * LAYERS=bmng200406&
 * STYLES=&
 * SRS=EPSG:4326&
 * BBOX=-180,-90,180,90&
 * FORMAT=image/jpeg&
 * WIDTH=600&
 * HEIGHT=300
 *
 * http://www.nasa.network.com/elev?
 * SERVICE=WMS&
 * VERSION=1.1.0&
 * REQUEST=GetMap&
 * LAYERS=srtm30&
 * STYLES=&
 * SRS=EPSG:4326&
 * BBOX=-180,-90,180,90&
 * FORMAT=application/bil32&
 * WIDTH=600&
 * HEIGHT=300
 */

#include <config.h>
#include <stdio.h>
#include <glib.h>

#include "gis-wms.h"
#include "gis-http.h"

static gchar *_make_uri(GisWms *wms, GisTile *tile)
{
	return g_strdup_printf(
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

gchar *gis_wms_fetch(GisWms *wms, GisTile *tile, GisCacheType mode,
		GisChunkCallback callback, gpointer user_data)
{
	gchar *uri   = _make_uri(wms, tile);
	gchar *tilep = gis_tile_get_path(tile);
	gchar *local = g_strdup_printf("%s%s", tilep, wms->extension);
	mode = GIS_ONCE;
	gchar *path  = gis_http_fetch(wms->http, uri, local,
			mode, callback, user_data);
	g_free(uri);
	g_free(tilep);
	g_free(local);
	return path;
}

GisWms *gis_wms_new(
	const gchar *uri_prefix, const gchar *uri_layer,
	const gchar *uri_format, const gchar *prefix,
	const gchar *extension, gint width, gint height)
{
	g_debug("GisWms: new - %s", uri_prefix);
	GisWms *wms = g_new0(GisWms, 1);
	wms->http         = gis_http_new(prefix);
	wms->uri_prefix   = g_strdup(uri_prefix);
	wms->uri_layer    = g_strdup(uri_layer);
	wms->uri_format   = g_strdup(uri_format);
	wms->extension    = g_strdup(extension);
	wms->width        = width;
	wms->height       = height;
	return wms;
}

void gis_wms_free(GisWms *wms)
{
	g_debug("GisWms: free - %s", wms->uri_prefix);
	gis_http_free(wms->http);
	g_free(wms->uri_prefix);
	g_free(wms->uri_layer);
	g_free(wms->uri_format);
	g_free(wms->extension);
	g_free(wms);
}
