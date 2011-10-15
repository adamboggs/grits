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

#ifndef __GRITS_OBJECT_H__
#define __GRITS_OBJECT_H__

#include <glib.h>
#include <glib-object.h>
#include "grits-util.h"

/* GritsObject */
#define GRITS_TYPE_OBJECT            (grits_object_get_type())
#define GRITS_OBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_OBJECT, GritsObject))
#define GRITS_IS_OBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_OBJECT))
#define GRITS_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_OBJECT, GritsObjectClass))
#define GRITS_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_OBJECT))
#define GRITS_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_OBJECT, GritsObjectClass))

/* Bitmask of things to skip while drawing the object */
#define GRITS_SKIP_LOD     (1<<0)
#define GRITS_SKIP_HORIZON (1<<1)
#define GRITS_SKIP_CENTER  (1<<2)
#define GRITS_SKIP_STATE   (1<<3)

/* Picking states */
typedef struct {
	guint picked   : 1;
	guint selected : 1;
} GritsState;

typedef struct _GritsObject      GritsObject;
typedef struct _GritsObjectClass GritsObjectClass;

#include "grits-opengl.h"
struct _GritsObject {
	GObject      parent_instance;
	GritsViewer *viewer; // The viewer the object was added to
	gpointer     ref;    // Reference for objects that have been added
	GritsPoint   center; // Center of the object
	gboolean     hidden; // If true, the object will not be drawn
	gdouble      lod;    // Level of detail, used to hide small objects
	guint32      skip;   // Bit mask of safe operations

	GritsState   state;  // Internal, used for picking
};

struct _GritsObjectClass {
	GObjectClass parent_class;

	/* Move some of these to GObject? */
	void (*draw) (GritsObject *object, GritsOpenGL *opengl);
	void (*pick) (GritsObject *object, GritsOpenGL *opengl);
	void (*hide) (GritsObject *object, gboolean hidden);
};

GType grits_object_get_type(void);

/* Implemented by sub-classes */
void grits_object_draw(GritsObject *object, GritsOpenGL *opengl);

void grits_object_hide(GritsObject *object, gboolean hidden);

/* Interal, used by grits_opengl */
void grits_object_pick_begin(GritsObject *object, GritsOpenGL *opengl);
void grits_object_pick_pointer(GritsObject *object, double x, double y);
void grits_object_pick_end(GritsObject *object);
void grits_object_event(GritsObject *object, GdkEvent *event);

/**
 * grits_object_queue_draw:
 * @object: The #GritsObject that needs drawing
 * 
 * Cause the widget to be redrawn on the screen at some later point
 */
void grits_object_queue_draw(GritsObject *object);

/**
 * grits_object_center:
 * @object: The #GritsObject to get the center of
 * 
 * Get the #GritsPoint representing the center of an object
 *
 * Returns: the center point
 */
#define grits_object_center(object) \
	(&GRITS_OBJECT(object)->center)

#endif
