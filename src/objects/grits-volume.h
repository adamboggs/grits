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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GRITS_VOLUME_H__
#define __GRITS_VOLUME_H__

#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include "grits-object.h"
#include "marching.h"

/***************
 * GritsVolume *
 ***************/
#define GRITS_TYPE_VOLUME            (grits_volume_get_type())
#define GRITS_VOLUME(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_VOLUME, GritsVolume))
#define GRITS_IS_VOLUME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_VOLUME))
#define GRITS_VOLUME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_VOLUME, GritsVolumeClass))
#define GRITS_IS_VOLUME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_VOLUME))
#define GRITS_VOLUME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_VOLUME, GritsVolumeClass))

typedef enum {
	GRITS_VOLUME_CARTESIAN,
	GRITS_VOLUME_SPHERICAL,
} GritsVolumeProj;

typedef enum {
	GRITS_VOLUME_SURFACE,
	GRITS_VOLUME_POINTS,
} GritsVolumeDisp;

typedef struct _GritsVolume      GritsVolume;
typedef struct _GritsVolumeClass GritsVolumeClass;

struct _GritsVolume {
	GritsObject parent_instance;

	/* Instance members */
	GritsVolumeProj proj; // projection
	GritsVolumeDisp disp; // display mode

	/* Internal */
	VolGrid *grid;
	GList   *tris;
	gdouble  level;
	guint8   color[4];
	gint     update_id;
};

struct _GritsVolumeClass {
	GritsObjectClass parent_class;
};

GType grits_volume_get_type(void);

/* Methods */
void grits_volume_set_level(GritsVolume *volume, gdouble level);

GritsVolume *grits_volume_new(VolGrid *grid);

#endif
