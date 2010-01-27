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

static void _soup_chunk_cb(SoupMessage *message, SoupBuffer *chunk, gpointer _file)
{
	FILE *file = _file;
	if (!SOUP_STATUS_IS_SUCCESSFUL(message->status_code)) {
		g_warning("GisWms: soup_chunk_cb - soup failed with %d", message->status_code);
		return;
	}
	goffset total = soup_message_headers_get_content_length(message->response_headers);
	if (fwrite(chunk->data, chunk->length, 1, file) != 1)
		g_warning("GisWms: soup_chunk_cb - eror writing data");
}

char *gis_wms_make_local(GisWms *self, GisTile *tile)
{
	/* Get file path */
	gchar *tile_path = gis_tile_get_path(tile);
	gchar *path = g_strdup_printf("%s/%s/%s%s%s",
		g_get_user_cache_dir(), PACKAGE,
		self->cache_prefix, tile_path, self->cache_ext);
	g_free(tile_path);

	/* Return if it already exists */
	if (g_file_test(path, G_FILE_TEST_EXISTS))
		return path;

	/* Open temp file for writing */
	gchar *tmp_path = g_strconcat(path, ".part", NULL);
	gchar *dirname = g_path_get_dirname(tmp_path);
	g_mkdir_with_parents(dirname, 0755);
	g_free(dirname);
	FILE *file = fopen(tmp_path, "a");

	/* Download file */
	gchar *uri = _make_uri(self, tile);
	g_debug("GisWms: make_local - fetching %s", uri);
	SoupMessage *message = soup_message_new("GET", uri);
	g_signal_connect(message, "got-chunk", G_CALLBACK(_soup_chunk_cb), file);
	soup_message_headers_set_range(message->request_headers, ftell(file), -1);
	int status = soup_session_send_message(self->soup, message);
	if (!SOUP_STATUS_IS_SUCCESSFUL(message->status_code))
		g_warning("GisWms: make_local - soup failed with %d", message->status_code);
	g_free(uri);

	/* Clean up */
	fclose(file);
	rename(tmp_path, path);
	g_free(tmp_path);
	return path;
}

GisWms *gis_wms_new(
	gchar *uri_prefix, gchar *uri_layer, gchar *uri_format,
	gchar *cache_prefix, gchar *cache_ext,
	gint width, gint height)
{
	GisWms *self = g_new0(GisWms, 1);
	self->uri_prefix   = uri_prefix;
	self->uri_layer    = uri_layer;
	self->uri_format   = uri_format;
	self->cache_prefix = cache_prefix;
	self->cache_ext    = cache_ext;
	self->width        = width;
	self->height       = height;
	self->soup         = soup_session_sync_new();
	return self;
}

void gis_wms_free(GisWms *self)
{
	g_object_unref(self->soup);
	g_free(self);
}
