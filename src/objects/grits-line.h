/*
 * Copyright (C) 2012 Andy Spencer <andy753421@gmail.com>
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

#ifndef __GRITS_LINE_H__
#define __GRITS_LINE_H__

#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include <objects/grits-object.h>

/* GritsLine */
#define GRITS_TYPE_LINE            (grits_line_get_type())
#define GRITS_LINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_LINE, GritsLine))
#define GRITS_IS_LINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_LINE))
#define GRITS_LINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_LINE, GritsLineClass))
#define GRITS_IS_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_LINE))
#define GRITS_LINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_LINE, GritsLineClass))

typedef struct _GritsLine      GritsLine;
typedef struct _GritsLineClass GritsLineClass;

struct _GritsLine {
	GritsObject  parent_instance;
	gdouble   (**points)[3];
	gdouble      color[4];
	gdouble      width;
};

struct _GritsLineClass {
	GritsObjectClass parent_class;
};

GType grits_line_get_type(void);

GritsLine *grits_line_new(gdouble (**points)[3]);

GritsLine *grits_line_parse(const gchar *str,
		const gchar *line_sep, const gchar *point_sep, const gchar *coord_sep);

#endif
