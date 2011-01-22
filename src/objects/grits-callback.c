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
 * SECTION:grits-callback
 * @short_description: Custom callback objects
 *
 * #GritsCallback objects are used for custom drawing functions. A common example
 * of this would be to render something which does not easily fit into a normal
 * object. For instance, a Heads-Up-Display overlay.
 *
 * Callbacks are an alternate to extending GritsObject with a new class and
 * should be used when only once instance of the object will be needed.
 */

#include <config.h>
#include "grits-callback.h"

/**
 * grits_callback_new:
 * @callback:  the function to call to draw the object
 * @user_data: user data to pass to the drawing function
 *
 * Create a #GritsCallback object with an associated function and user data.
 *
 * Returns: the new #GritsCallback
 */
GritsCallback *grits_callback_new(GritsCallbackFunc draw_cb, gpointer user_data)
{
	GritsCallback *cb = g_object_new(GRITS_TYPE_CALLBACK, NULL);
	cb->draw      = draw_cb;
	cb->user_data = user_data;
	return cb;
}

/* Proxy class methods to per-object methods */
static void proxy_draw(GritsObject *_cb, GritsOpenGL *opengl)
{
	GritsCallback *cb = GRITS_CALLBACK(_cb);
	if (cb->draw)
		cb->draw(cb, opengl, cb->user_data);
}

/* GritsCallback */
G_DEFINE_TYPE(GritsCallback, grits_callback, GRITS_TYPE_OBJECT);
static void grits_callback_finalize(GObject *cb)
{
	g_debug("GritsCallback: finalize");
}
static void grits_callback_init(GritsCallback *cb)
{
	g_debug("GritsCallback: init");
}

static void grits_callback_class_init(GritsCallbackClass *klass)
{
	GritsObjectClass *grits_class  = GRITS_OBJECT_CLASS(klass);
	GObjectClass     *object_class = G_OBJECT_CLASS(klass);
	grits_class->draw      = proxy_draw;
	object_class->finalize = grits_callback_finalize;
}
