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

/**
 * SECTION:gis-object
 * @short_description: Base class for drawing operations
 *
 * Objects in libgis are things which can be added to the viewer and will be
 * displayed to the user. Each object has information such as it's location and
 * level of detail which are used by the viewer to determine which objects
 * should be drawn.
 *
 * Each #GisObject is also a #GObject, but not every GObject in libgis is a
 * GisObject. The "Object" part of the name is just coincidence.
 */

#include <config.h>
#include <math.h>
#include <GL/gl.h>

#include "gis-object.h"


/*************
 * GisObject *
 *************/
/**
 * gis_object_draw:
 * @object: the object
 * @opengl: the viewer the object is being displayed in
 *
 * Perform any OpenGL commands necessasairy to draw the object.
 *
 * The GL_PROJECTION and GL_MODELVIEW matricies and GL_ALL_ATTRIB_BITS will be
 * restored to the default state after the call to draw.
 */
void gis_object_draw(GisObject *object, GisOpenGL *opengl)
{
	GisObjectClass *klass = GIS_OBJECT_GET_CLASS(object);
	if (!klass->draw) {
		g_warning("GisObject: draw - Unimplemented");
		return;
	}

	/* Skip hidden objects */
	if (object->hidden)
		return;

	/* Skip out of range objects */
	if (object->lod > 0) {
		/* LOD test */
		gdouble eye[3], obj[3];
		gis_viewer_get_location(GIS_VIEWER(opengl), &eye[0], &eye[1], &eye[2]);
		gdouble elev = eye[2];
		lle2xyz(eye[0], eye[1], eye[2], &eye[0], &eye[1], &eye[2]);
		lle2xyz(object->center.lat, object->center.lon, object->center.elev,
			&obj[0], &obj[1], &obj[2]);
		gdouble dist = distd(obj, eye);
		if (object->lod < dist)
			return;

		/* Horizon testing */
		gdouble c = EARTH_R+elev;
		gdouble a = EARTH_R;
		gdouble horizon = sqrt(c*c - a*a);
		if (dist > horizon)
			return;
	}

	/* Save state, draw, restore state */
	g_mutex_lock(opengl->sphere_lock);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION); glPushMatrix();
	glMatrixMode(GL_MODELVIEW);  glPushMatrix();

	klass->draw(object, opengl);

	glPopAttrib();
	glMatrixMode(GL_PROJECTION); glPopMatrix();
	glMatrixMode(GL_MODELVIEW);  glPopMatrix();
	g_mutex_unlock(opengl->sphere_lock);
}

/* GObject stuff */
G_DEFINE_ABSTRACT_TYPE(GisObject, gis_object, G_TYPE_OBJECT);
static void gis_object_init(GisObject *object)
{
}

static void gis_object_class_init(GisObjectClass *klass)
{
}
