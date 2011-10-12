/*
 * Copyright (C) 2010 Andy Spencer <andy753421@gmail.com>
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

#ifndef __GRITS_POLY_H__
#define __GRITS_POLY_H__

#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include <objects/grits-object.h>

/* GritsPoly */
#define GRITS_TYPE_POLY            (grits_poly_get_type())
#define GRITS_POLY(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_POLY, GritsPoly))
#define GRITS_IS_POLY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_POLY))
#define GRITS_POLY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_POLY, GritsPolyClass))
#define GRITS_IS_POLY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_POLY))
#define GRITS_POLY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_POLY, GritsPolyClass))

typedef struct _GritsPoly      GritsPoly;
typedef struct _GritsPolyClass GritsPolyClass;

struct _GritsPoly {
	GritsObject  parent_instance;
	gdouble   (**points)[3];
	gdouble      color[4];
	gdouble      border[4];
	gdouble      width;
	guint        list;
};

struct _GritsPolyClass {
	GritsObjectClass parent_class;
};

GType grits_poly_get_type(void);

GritsPoly *grits_poly_new(gdouble (**points)[3]);

GritsPoly *grits_poly_parse(gchar *str,
		gchar *poly_sep, gchar *point_sep, gchar *coord_sep);

#endif
