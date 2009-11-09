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

#ifndef __GIS_OPENGL_H__
#define __GIS_OPENGL_H__

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <glib-object.h>

/* Type macros */
#define GIS_TYPE_OPENGL            (gis_opengl_get_type())
#define GIS_OPENGL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GIS_TYPE_OPENGL, GisOpenGL))
#define GIS_IS_OPENGL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GIS_TYPE_OPENGL))
#define GIS_OPENGL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GIS_TYPE_OPENGL, GisOpenGLClass))
#define GIS_IS_OPENGL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GIS_TYPE_OPENGL))
#define GIS_OPENGL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GIS_TYPE_OPENGL, GisOpenGLClass))

typedef struct _GisOpenGL      GisOpenGL;
typedef struct _GisOpenGLClass GisOpenGLClass;

#include "gis-view.h"
#include "gis-world.h"
#include "gis-plugin.h"
#include "roam.h"

struct _GisOpenGL {
	GtkDrawingArea parent_instance;

	/* instance members */
	GisWorld   *world;
	GisView    *view;
	GisPlugins *plugins;
	RoamSphere *sphere;
	guint       sm_source;

	/* for testing */
	gboolean    wireframe;
};

struct _GisOpenGLClass {
	GtkDrawingAreaClass parent_class;

	/* class members */
};

GType gis_opengl_get_type(void);

/* Methods */
GisOpenGL *gis_opengl_new(GisWorld *world, GisView *view, GisPlugins *plugins);

void gis_opengl_center_position(GisOpenGL *opengl,
		gdouble lat, gdouble lon, gdouble elev);

void gis_opengl_redraw(GisOpenGL *opengl);

void gis_opengl_begin(GisOpenGL *opengl);
void gis_opengl_end(GisOpenGL *opengl);
void gis_opengl_flush(GisOpenGL *opengl);

#endif
