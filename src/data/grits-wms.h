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

#ifndef __GIS_WMS_H__
#define __GIS_WMS_H__

#include <glib.h>

#include "data/grits-http.h"
#include "objects/grits-tile.h"

typedef struct _GisWms {
	GisHttp *http;
	gchar *uri_prefix;
	gchar *uri_layer;
	gchar *uri_format;
	gchar *extension;
	gint   width;
	gint   height;
} GisWms;

GisWms *gis_wms_new(
	const gchar *uri_prefix, const gchar *uri_layer,
	const gchar *uri_format, const gchar *prefix,
	const gchar *extension, gint width, gint height);

gchar *gis_wms_fetch(GisWms *wms, GisTile *tile, GisCacheType mode,
		GisChunkCallback callback, gpointer user_data);

void gis_wms_free(GisWms *wms);

#endif
