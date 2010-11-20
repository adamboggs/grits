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

#ifndef __GRITS_MARKER_H__
#define __GRITS_MARKER_H__

#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include "grits-object.h"

/* GritsMarker */
#define GRITS_TYPE_MARKER            (grits_marker_get_type())
#define GRITS_MARKER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_MARKER, GritsMarker))
#define GRITS_IS_MARKER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_MARKER))
#define GRITS_MARKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_MARKER, GritsMarkerClass))
#define GRITS_IS_MARKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_MARKER))
#define GRITS_MARKER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_MARKER, GritsMarkerClass))

typedef struct _GritsMarker      GritsMarker;
typedef struct _GritsMarkerClass GritsMarkerClass;

struct _GritsMarker {
	GritsObject  parent_instance;
	gint       xoff, yoff;
	gchar     *label;
	cairo_t   *cairo;
	guint      tex;
};

struct _GritsMarkerClass {
	GritsObjectClass parent_class;
};

GType grits_marker_get_type(void);

GritsMarker *grits_marker_new(const gchar *label);

#endif
