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
#define GIS_TYPE_CALLBACK            (gis_callback_get_type())
#define GIS_CALLBACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_CALLBACK, GisCallback))
#define GIS_IS_CALLBACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_CALLBACK))
#define GIS_CALLBACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_CALLBACK, GisCallbackClass))
#define GIS_IS_CALLBACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_CALLBACK))
#define GIS_CALLBACK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_CALLBACK, GisCallbackClass))

typedef struct _GisCallback      GisCallback;
typedef struct _GisCallbackClass GisCallbackClass;

/**
 * GisCallbackFunc:
 * @callback:  the callback object to be drawn
 * @user_data: the user data associated with the callback 
 *
 * A function to be called when the callback object is being rendered
 */
typedef void (*GisCallbackFunc)(GisCallback *callback, GisOpenGL *opengl, gpointer user_data);

struct _GisCallback {
	GisObject       parent;
	GisCallbackFunc draw;
	gpointer        user_data;
};

struct _GisCallbackClass {
	GisObjectClass parent_class;
};

GType gis_callback_get_type(void);

GisCallback *gis_callback_new(GisCallbackFunc draw_cb, gpointer user_data);

#endif
