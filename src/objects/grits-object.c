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
 * SECTION:grits-object
 * @short_description: Base class for drawing operations
 *
 * Objects in grits are things which can be added to the viewer and will be
 * displayed to the user. Each object has information such as it's location and
 * level of detail which are used by the viewer to determine which objects
 * should be drawn.
 *
 * Each #GritsObject is also a #GObject, but not every GObject in grits is a
 * GritsObject. The "Object" part of the name is just coincidence.
 */

#include <config.h>
#include <math.h>
#include <GL/gl.h>

#include "grits-object.h"


/**
 * grits_object_draw:
 * @object: the object
 * @opengl: the viewer the object is being displayed in
 *
 * Perform any OpenGL commands necessasairy to draw the object.
 *
 * The GL_PROJECTION and GL_MODELVIEW matricies and GL_ALL_ATTRIB_BITS will be
 * restored to the default state after the call to draw.
 */
void grits_object_draw(GritsObject *object, GritsOpenGL *opengl)
{
	GritsObjectClass *klass = GRITS_OBJECT_GET_CLASS(object);
	if (!klass->draw) {
		g_warning("GritsObject: draw - Unimplemented");
		return;
	}

	/* Skip hidden objects */
	if (object->hidden)
		return;

	/* Support GritsTester */
	if (!GRITS_IS_OPENGL(opengl)) {
		g_debug("GritsObject: draw - drawing raw object");
		klass->draw(object, opengl);
		return;
	}

	/* Calculae distance for LOD and horizon tests */
	GritsPoint *center = &object->center;
	if ((!(object->skip & GRITS_SKIP_LOD) ||
	     !(object->skip & GRITS_SKIP_HORIZON)) &&
	    (center->elev != -EARTH_R)) {
		/* LOD test */
		gdouble eye[3], obj[3];
		grits_viewer_get_location(GRITS_VIEWER(opengl),
				&eye[0], &eye[1], &eye[2]);
		gdouble elev = eye[2];
		lle2xyz(eye[0], eye[1], eye[2],
				&eye[0], &eye[1], &eye[2]);
		lle2xyz(center->lat, center->lon, center->elev,
				&obj[0], &obj[1], &obj[2]);
		gdouble dist = distd(obj, eye);

		/* Level of detail test */
		if (!(object->skip & GRITS_SKIP_LOD)
				&& object->lod > 0) {
			if (object->lod < dist)
				return;
		}

		/* Horizon test */
		if (!(object->skip & GRITS_SKIP_HORIZON)) {
			gdouble c = EARTH_R+elev;
			gdouble a = EARTH_R;
			gdouble horizon = sqrt(c*c - a*a);
			if (dist > horizon)
				return;
		}
	}

	/* Save state, draw, restore state */
	g_mutex_lock(opengl->sphere_lock);
	if (!(object->skip & GRITS_SKIP_STATE)) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glMatrixMode(GL_PROJECTION); glPushMatrix();
		glMatrixMode(GL_MODELVIEW);  glPushMatrix();
	}

	if (!(object->skip & GRITS_SKIP_CENTER))
		grits_viewer_center_position(GRITS_VIEWER(opengl),
				object->center.lat,
				object->center.lon,
				object->center.elev);

	klass->draw(object, opengl);

	if (!(object->skip & GRITS_SKIP_STATE)) {
		glPopAttrib();
		glMatrixMode(GL_PROJECTION); glPopMatrix();
		glMatrixMode(GL_MODELVIEW);  glPopMatrix();
	}
	g_mutex_unlock(opengl->sphere_lock);
}

void grits_object_queue_draw(GritsObject *object)
{
	if (object->viewer)
		gtk_widget_queue_draw(GTK_WIDGET(object->viewer));
}

/* GObject stuff */
G_DEFINE_ABSTRACT_TYPE(GritsObject, grits_object, G_TYPE_OBJECT);
static void grits_object_init(GritsObject *object)
{
	object->center.lat  =  0;
	object->center.lon  =  0;
	object->center.elev = -EARTH_R;
}

static void grits_object_class_init(GritsObjectClass *klass)
{
}