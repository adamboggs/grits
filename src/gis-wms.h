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

#ifndef __GIS_WMS_H__
#define __GIS_WMS_H__

#include <glib.h>
#include <libsoup/soup.h>

#include "gis-tile.h"

typedef struct _GisWms GisWms;

struct _GisWms {
	gchar *uri_prefix;
	gchar *uri_layer;
	gchar *uri_format;
	gchar *cache_prefix;
	gchar *cache_ext;
	gint   width;
	gint   height;
	SoupSession  *soup;
};

char *gis_wms_make_local(GisWms *wms, GisTile *tile);

GisWms *gis_wms_new(
	gchar *uri_prefix, gchar *uri_layer, gchar *uri_format,
	gchar *cache_prefix, gchar *cache_ext,
	gint width, gint height);

void gis_wms_free(GisWms *self);

#endif
