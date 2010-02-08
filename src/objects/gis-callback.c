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
 * SECTION:gis-callback
 * @short_description: Custom callback objects
 *
 * #GisCallback objects are used for custom drawing functions. A common example
 * of this would be to render something which does not easily fit into a normal
 * object. For instance, a Heads-Up-Display overlay.
 */

#include <config.h>
#include "gis-callback.h"

/* GisCallback */
G_DEFINE_TYPE(GisCallback, gis_callback, GIS_TYPE_OBJECT);
static void gis_callback_init(GisCallback *cb)
{
}

static void gis_callback_class_init(GisCallbackClass *klass)
{
}

/**
 * gis_callback_new:
 * @callback:  the function to call to draw the object
 * @user_data: user data to pass to the drawing function
 *
 * Create a #GisCallback object with an associated function and user data.
 *
 * Returns: the new #GisCallback
 */
GisCallback *gis_callback_new(GisCallbackFunc callback, gpointer user_data)
{
	GisCallback *cb = g_object_new(GIS_TYPE_CALLBACK, NULL);
	cb->callback  = callback;
	cb->user_data = user_data;
	return cb;
}
