/*
 * Copyright (C) 2009-2011 Andy Spencer <andy753421@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUReOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GRITS_TESTER_H__
#define __GRITS_TESTER_H__

#include <gtk/gtk.h>
#include <glib-object.h>

#include <objects/grits-object.h>

/* Type macros */
#define GRITS_TYPE_TESTER            (grits_tester_get_type())
#define GRITS_TESTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_TESTER, GritsTester))
#define GRITS_IS_TESTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_TESTER))
#define GRITS_TESTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_TESTER, GritsTesterClass))
#define GRITS_IS_TESTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_TESTER))
#define GRITS_TESTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_TESTER, GritsTesterClass))

typedef struct _GritsTester      GritsTester;
typedef struct _GritsTesterClass GritsTesterClass;

struct _GritsTester {
	GtkDrawingArea parent_instance;

	/* instance members */
	gboolean  wireframe;
	GList    *objects;
};

struct _GritsTesterClass {
	GtkDrawingAreaClass parent_class;
};

GType grits_tester_get_type(void);

/* Methods */
GritsTester *grits_tester_new();
gpointer grits_tester_add(GritsTester *tester, GritsObject *object);
GritsObject *grits_tester_remove(GritsTester *tester, gpointer ref);

#endif
