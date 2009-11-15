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

#include <glib.h>
#include <GL/glu.h>

#include "gis-object.h"
#include "gis-util.h"

/* TODO
 *   - Manage normals for GisPoint
 */

/* GisPoint */
GisPoint *gis_point_new()
{
	return g_new0(GisPoint, 1);
}

void gis_point_set_lle(GisPoint *self, gdouble lat, gdouble lon, gdouble elev)
{
	self->lat  = lat;
	self->lon  = lon;
	self->elev = elev;
	lle2xyz(self->lat, self->lon, self->elev,
			&self->x, &self->y, &self->z);
}

void gis_point_set_xyz(GisPoint *self, gdouble x, gdouble y, gdouble z)
{
	self->x = x;
	self->y = y;
	self->z = z;
}

void gis_point_set_coords(GisPoint *self, gdouble x, gdouble y)
{
	self->cx = x;
	self->cy = y;
}

void gis_point_project(GisPoint *self, GisProjection *proj)
{
	gluProject(self->x, self->y, self->z,
	           proj->model, proj->proj, proj->view,
	           &self->px, &self->py, &self->pz);
}

GisPoint *gis_point_ref(GisPoint *self)
{
	self->refs++;
	return self;
}

void gis_point_unref(GisPoint *self)
{
	self->refs--;
	if (self->refs <= 0)
		g_free(self);
}


/* GisTriangle */
GisTriangle *gis_triangle_new(GisPoint *a, GisPoint *b, GisPoint *c, guint tex)
{
	GisTriangle *self = g_new0(GisTriangle, 1);
	GIS_OBJECT(self)->type = GIS_TYPE_TRIANGLE;
	gis_point_set_xyz(&GIS_OBJECT(self)->center,
		(a->x + b->x + c->x)/3,
		(a->y + b->y + c->y)/3,
		(a->z + b->z + c->z)/3);
	self->verts[0] = gis_point_ref(a);
	self->verts[1] = gis_point_ref(b);
	self->verts[2] = gis_point_ref(c);
	self->tex      = tex;
	return self;
}

void gis_triangle_free(GisTriangle *self)
{
	gis_point_unref(self->verts[0]);
	gis_point_unref(self->verts[1]);
	gis_point_unref(self->verts[2]);
	g_free(self);
}


/* GisQuad */
GisQuad *gis_quad_new(GisPoint *a, GisPoint *b, GisPoint *c, GisPoint *d, guint tex)
{
	GisQuad *self = g_new0(GisQuad, 1);
	GIS_OBJECT(self)->type = GIS_TYPE_QUAD;
	gis_point_set_xyz(&GIS_OBJECT(self)->center,
		(a->x + b->x + c->x + d->x)/4,
		(a->y + b->y + c->y + d->y)/4,
		(a->z + b->z + c->z + d->z)/4);
	self->verts[0] = gis_point_ref(a);
	self->verts[1] = gis_point_ref(b);
	self->verts[2] = gis_point_ref(c);
	self->verts[3] = gis_point_ref(d);
	self->tex      = tex;
	return self;
}

void gis_quad_free(GisQuad *self)
{
	gis_point_unref(self->verts[0]);
	gis_point_unref(self->verts[1]);
	gis_point_unref(self->verts[2]);
	gis_point_unref(self->verts[3]);
	g_free(self);
}


/* GisCallback */
GisCallback *gis_callback_new(GisCallbackFunc callback, gpointer user_data)
{
	GisCallback *self = g_new0(GisCallback, 1);
	GIS_OBJECT(self)->type = GIS_TYPE_CALLBACK;
	self->callback  = callback;
	self->user_data = user_data;
	return self;
}

void gis_callback_free(GisCallback *self)
{
	g_free(self);
}


/* GisCallback */
GisMarker *gis_marker_new(const gchar *label)
{
	GisMarker *self = g_new0(GisMarker, 1);
	GIS_OBJECT(self)->type = GIS_TYPE_MARKER;
	self->label = g_strdup(label);;
	return self;
}

void gis_marker_free(GisMarker *self)
{
	g_free(self->label);
	g_free(self);
}
