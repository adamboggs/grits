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

#ifndef __GRITS_OPENGL_H__
#define __GRITS_OPENGL_H__

#include <glib-object.h>

/* Type macros */
#define GRITS_TYPE_OPENGL            (grits_opengl_get_type())
#define GRITS_OPENGL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_OPENGL, GritsOpenGL))
#define GRITS_IS_OPENGL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_OPENGL))
#define GRITS_OPENGL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_OPENGL, GritsOpenGLClass))
#define GRITS_IS_OPENGL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_OPENGL))
#define GRITS_OPENGL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_OPENGL, GritsOpenGLClass))

typedef struct _GritsOpenGL      GritsOpenGL;
typedef struct _GritsOpenGLClass GritsOpenGLClass;

#include "grits-viewer.h"
#include "roam.h"

struct _GritsOpenGL {
	GritsViewer parent_instance;

	/* instance members */
	GTree      *objects;
	GMutex     *objects_lock;
	RoamSphere *sphere;
	GMutex     *sphere_lock;
	guint       sm_source[2];
	guint       ue_source;

	/* for testing */
	gboolean    wireframe;
};

struct _GritsOpenGLClass {
	GritsViewerClass parent_class;

	/* class members */
};

GType grits_opengl_get_type(void);

/* Methods */
GritsViewer *grits_opengl_new(GritsPlugins *plugins, GritsPrefs *prefs);

#endif
