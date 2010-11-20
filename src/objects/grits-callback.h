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

#ifndef __GRITS_CALLBACK_H__
#define __GRITS_CALLBACK_H__

#include <glib.h>
#include <glib-object.h>
#include "grits-object.h"

/* GritsCallback */
#define GRITS_TYPE_CALLBACK            (grits_callback_get_type())
#define GRITS_CALLBACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_CALLBACK, GritsCallback))
#define GRITS_IS_CALLBACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_CALLBACK))
#define GRITS_CALLBACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_CALLBACK, GritsCallbackClass))
#define GRITS_IS_CALLBACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_CALLBACK))
#define GRITS_CALLBACK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_CALLBACK, GritsCallbackClass))

typedef struct _GritsCallback      GritsCallback;
typedef struct _GritsCallbackClass GritsCallbackClass;

/**
 * GritsCallbackFunc:
 * @callback:  the callback object to be drawn
 * @user_data: the user data associated with the callback 
 *
 * A function to be called when the callback object is being rendered
 */
typedef void (*GritsCallbackFunc)(GritsCallback *callback, GritsOpenGL *opengl, gpointer user_data);

struct _GritsCallback {
	GritsObject       parent;
	GritsCallbackFunc draw;
	gpointer          user_data;
};

struct _GritsCallbackClass {
	GritsObjectClass parent_class;
};

GType grits_callback_get_type(void);

GritsCallback *grits_callback_new(GritsCallbackFunc draw_cb, gpointer user_data);

#endif
