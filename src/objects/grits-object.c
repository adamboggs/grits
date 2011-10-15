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

/* Constants */
enum {
	SIG_ENTER,
	SIG_LEAVE,
	SIG_BUTTON_PRESS,
	SIG_BUTTON_RELEASE,
	SIG_KEY_PRESS,
	SIG_KEY_RELEASE,
	SIG_MOTION,
	NUM_SIGNALS,
};
static guint signals[NUM_SIGNALS];

void grits_object_pickdraw(GritsObject *object, GritsOpenGL *opengl, gboolean pick)
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

	if (pick && klass->pick)
		klass->pick(object, opengl);
	else
		klass->draw(object, opengl);

	if (!(object->skip & GRITS_SKIP_STATE)) {
		glPopAttrib();
		glMatrixMode(GL_PROJECTION); glPopMatrix();
		glMatrixMode(GL_MODELVIEW);  glPopMatrix();
	}
	g_mutex_unlock(opengl->sphere_lock);
}

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
	grits_object_pickdraw(object, opengl, FALSE);
}

void grits_object_hide(GritsObject *object, gboolean hidden)
{
	GritsObjectClass *klass = GRITS_OBJECT_GET_CLASS(object);
	object->hidden = hidden;
	if (klass->hide)
		klass->hide(object, hidden);
}

void grits_object_queue_draw(GritsObject *object)
{
	if (object->viewer)
		gtk_widget_queue_draw(GTK_WIDGET(object->viewer));
}

/* Event handling */
void grits_object_pick_begin(GritsObject *object, GritsOpenGL *opengl)
{
	object->state.picked = FALSE;

	/* Check for connected signals */
	for (int i = 0; i < NUM_SIGNALS; i++) {
		if (g_signal_has_handler_pending(object, signals[i], 0, FALSE)) {
			/* Someone is watching, render the object _once_ */
			glPushName((guint)object);
			grits_object_pickdraw(object, opengl, TRUE);
			glPopName();
			return;
		}
	}
}

void grits_object_pick_pointer(GritsObject *object, double x, double y)
{
	object->state.picked = TRUE;
}

void grits_object_pick_end(GritsObject *object)
{
	if (object->state.picked) {
		if (!object->state.selected)
			g_signal_emit(object, signals[SIG_ENTER], 0);
		object->state.selected = TRUE;
	} else {
		if (object->state.selected)
			g_signal_emit(object, signals[SIG_LEAVE], 0);
		object->state.selected = FALSE;
	}
}

void grits_object_event(GritsObject *object, GdkEvent *event)
{
	const int map[GDK_EVENT_LAST] = {
		[GDK_BUTTON_PRESS  ] SIG_BUTTON_PRESS,
		[GDK_2BUTTON_PRESS ] SIG_BUTTON_PRESS,
		[GDK_3BUTTON_PRESS ] SIG_BUTTON_PRESS,
		[GDK_BUTTON_RELEASE] SIG_BUTTON_RELEASE,
		[GDK_KEY_PRESS     ] SIG_KEY_PRESS,
		[GDK_KEY_RELEASE   ] SIG_KEY_RELEASE,
		[GDK_MOTION_NOTIFY ] SIG_MOTION,
	};
	if (!object->state.selected)
		return;
	guint sig = signals[map[event->type]];
	if (!g_signal_has_handler_pending(object, sig, 0, FALSE))
		return;
	g_signal_emit(object, sig, 0, event);
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
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	/**
	 * GritsObject::enter:
	 * @object: the object.
	 *
	 * The ::enter signal is emitted when the pointer moves over the object
	 */
	signals[SIG_ENTER] = g_signal_new(
			"enter",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			0);

	/**
	 * GritsViewer::leave:
	 * @object: the object.
	 *
	 * The ::leave signal is emitted when the pointer moves away from the
	 * object
	 */
	signals[SIG_LEAVE] = g_signal_new(
			"leave",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			0);

	/**
	 * GritsViewer::button-press:
	 * @object: the object.
	 * @event:  the GdkEventButton which triggered this signal
	 *
	 * The ::button-press signal is emitted when a button (typically from a
	 * mouse) is pressed.
	 */
	signals[SIG_BUTTON_PRESS] = g_signal_new(
			"button-press",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER);

	/**
	 * GritsViewer::button-release:
	 * @object: the object.
	 * @event:  the GdkEventButton which triggered this signal
	 *
	 * The ::button-release signal is emitted when a button (typically from
	 * a mouse) is released.
	 */
	signals[SIG_BUTTON_RELEASE] = g_signal_new(
			"button-release",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER);

	/**
	 * GritsViewer::key-press:
	 * @object: the object.
	 * @event:  the GdkEventKey which triggered this signal
	 *
	 * The ::key-press signal is emitted when a key is pressed.
	 */
	signals[SIG_KEY_PRESS] = g_signal_new(
			"key-press",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER);

	/**
	 * GritsViewer::key-release:
	 * @object: the object.
	 * @event:  the GdkEventKey which triggered this signal
	 *
	 * The ::key-release signal is emitted when a key is released.
	 */
	signals[SIG_KEY_RELEASE] = g_signal_new(
			"key-release",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER);

	/**
	 * GritsViewer::motion:
	 * @object: the object.
	 * @event:  the GdkEventMotion which triggered this signal
	 *
	 * The ::motion signal is emitted the pointer moves over the object
	 */
	signals[SIG_MOTION] = g_signal_new(
			"motion",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER);
}
