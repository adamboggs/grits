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

#ifndef __GIS_CALLBACK_H__
#define __GIS_CALLBACK_H__

#include <glib.h>
#include <glib-object.h>
#include "gis-object.h"

/* GisCallback */
#define GIS_TYPE_CALLBACK (gis_callback_get_type())

GOBJECT_HEAD(
	GIS, CALLBACK,
	Gis, Callback,
	gis, callback);

typedef gpointer (*GisCallbackFunc)(GisCallback *callback, gpointer user_data);

struct _GisCallback {
	GisObject       parent;
	GisCallbackFunc callback;
	gpointer        user_data;
};

struct _GisCallbackClass {
	GisObjectClass parent_class;
};

GisCallback *gis_callback_new(GisCallbackFunc callback, gpointer user_data);

#endif
