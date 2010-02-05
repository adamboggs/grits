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

#include <config.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <libsoup/soup.h>

#include "gis-http.h"

GisHttp *gis_http_new(const gchar *prefix)
{
	GisHttp *http = g_new0(GisHttp, 1);
	http->prefix = g_strdup(prefix);
	http->soup = soup_session_sync_new();
	g_object_set(http->soup, "user-agent", PACKAGE_STRING, NULL);
	return http;
}

/* For passing data to the chunck callback */
struct _cache_info {
	FILE  *fp;
	gchar *path;
	GisChunkCallback callback;
	gpointer user_data;
};

/**
 * Append data to the file and call the users callback if they supplied one.
 */
static void _chunk_cb(SoupMessage *message, SoupBuffer *chunk, gpointer _info)
{
	struct _cache_info *info = _info;

	if (!SOUP_STATUS_IS_SUCCESSFUL(message->status_code)) {
		g_warning("GisHttp: _chunk_cb - soup failed with %d",
				message->status_code);
		return;
	}

	if (!fwrite(chunk->data, chunk->length, 1, info->fp))
		g_error("GisHttp: _chunk_cb - Unable to write data");

	if (info->callback) {
		goffset cur = ftell(info->fp);
		goffset st=0, end=0, total=0;
		soup_message_headers_get_content_range(message->response_headers,
				&st, &end, &total);
		info->callback(info->path, cur, total, info->user_data);
	}
}

/* TODO: use .part extentions and continue even when using GIS_ONCE */
gchar *gis_http_fetch(GisHttp *self, const gchar *uri, const char *local,
		GisCacheType mode, GisChunkCallback callback, gpointer user_data)
{
	g_debug("GisHttp: fetch - %s >> %s  mode=%d", uri, self->prefix, mode);

	gchar *path = g_build_filename(g_get_user_cache_dir(), PACKAGE,
			self->prefix, local, NULL);

	/* Unlink the file if we're refreshing it */
	if (mode == GIS_REFRESH)
		g_remove(path);

	/* Do the cache if necessasairy */
	if (!(mode == GIS_ONCE && g_file_test(path, G_FILE_TEST_EXISTS)) &&
			mode != GIS_LOCAL) {
		g_debug("GisHttp: do_cache - Caching file %s", local);

		/* Open the file for writting */
		FILE *fp = fopen_p(path, "a");

		/* Make temp data */
		struct _cache_info info = {
			.fp        = fp,
			.path      = path,
			.callback  = callback,
			.user_data = user_data,
		};

		/* Download the file */
		SoupMessage *message = soup_message_new("GET", uri);
		if (message == NULL)
			g_error("message is null, cannot parse uri");
		g_signal_connect(message, "got-chunk", G_CALLBACK(_chunk_cb), &info);
		soup_message_headers_set_range(message->request_headers, ftell(fp), -1);
		soup_session_send_message(self->soup, message);

		/* Finished */
		if (message->status_code == 416) {
			/* Range unsatisfiable, file already complete */
		} else if (!SOUP_STATUS_IS_SUCCESSFUL(message->status_code))
			g_warning("GisHttp: done_cb - error copying file, status=%d\n"
					"\tsrc=%s\n"
					"\tdst=%s",
					message->status_code, uri, path);
	}

	/* TODO: free everything.. */
	return path;
}