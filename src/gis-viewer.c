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

#include "gis-marshal.h"
#include "gis-viewer.h"

#include "gis-util.h"


/* Constants */
enum {
	PROP_0,
	PROP_TIME,
	PROP_SITE,
	PROP_OFFLINE,
};
enum {
	SIG_TIME_CHANGED,
	SIG_SITE_CHANGED,
	SIG_LOCATION_CHANGED,
	SIG_ROTATION_CHANGED,
	SIG_REFRESH,
	SIG_OFFLINE,
	NUM_SIGNALS,
};
static guint signals[NUM_SIGNALS];


/* Misc helpers */
static void _gis_viewer_fix_location(GisViewer *self)
{
	while (self->location[0] <  -90) self->location[0] += 180;
	while (self->location[0] >   90) self->location[0] -= 180;
	while (self->location[1] < -180) self->location[1] += 360;
	while (self->location[1] >  180) self->location[1] -= 360;
	self->location[2] = ABS(self->location[2]);
}

/* Signal helpers */
static void _gis_viewer_emit_location_changed(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_LOCATION_CHANGED], 0,
			self->location[0],
			self->location[1],
			self->location[2]);
}
static void _gis_viewer_emit_rotation_changed(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_ROTATION_CHANGED], 0,
			self->rotation[0],
			self->rotation[1],
			self->rotation[2]);
}
static void _gis_viewer_emit_time_changed(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_TIME_CHANGED], 0,
			self->time);
}
static void _gis_viewer_emit_site_changed(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_SITE_CHANGED], 0,
			self->site);
}
static void _gis_viewer_emit_refresh(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_REFRESH], 0);
}
static void _gis_viewer_emit_offline(GisViewer *self)
{
	g_signal_emit(self, signals[SIG_OFFLINE], 0,
			self->offline);
}


/***********
 * Methods *
 ***********/
GisViewer *gis_viewer_new()
{
	g_debug("GisViewer: new");
	return g_object_new(GIS_TYPE_VIEWER, NULL);
}

void gis_viewer_set_time(GisViewer *self, const char *time)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: set_time - time=%s", time);
	g_free(self->time);
	self->time = g_strdup(time);
	_gis_viewer_emit_time_changed(self);
}

gchar *gis_viewer_get_time(GisViewer *self)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: get_time");
	return self->time;
}

void gis_viewer_set_location(GisViewer *self, gdouble lat, gdouble lon, gdouble elev)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: set_location");
	self->location[0] = lat;
	self->location[1] = lon;
	self->location[2] = elev;
	_gis_viewer_fix_location(self);
	_gis_viewer_emit_location_changed(self);
}

void gis_viewer_get_location(GisViewer *self, gdouble *lat, gdouble *lon, gdouble *elev)
{
	g_assert(GIS_IS_VIEWER(self));
	//g_debug("GisViewer: get_location");
	*lat  = self->location[0];
	*lon  = self->location[1];
	*elev = self->location[2];
}

void gis_viewer_pan(GisViewer *self, gdouble lat, gdouble lon, gdouble elev)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: pan - lat=%8.3f, lon=%8.3f, elev=%8.3f", lat, lon, elev);
	self->location[0] += lat;
	self->location[1] += lon;
	self->location[2] += elev;
	_gis_viewer_fix_location(self);
	_gis_viewer_emit_location_changed(self);
}

void gis_viewer_zoom(GisViewer *self, gdouble scale)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: zoom");
	self->location[2] *= scale;
	_gis_viewer_emit_location_changed(self);
}

void gis_viewer_set_rotation(GisViewer *self, gdouble x, gdouble y, gdouble z)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: set_rotation");
	self->rotation[0] = x;
	self->rotation[1] = y;
	self->rotation[2] = z;
	_gis_viewer_emit_rotation_changed(self);
}

void gis_viewer_get_rotation(GisViewer *self, gdouble *x, gdouble *y, gdouble *z)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: get_rotation");
	*x = self->rotation[0];
	*y = self->rotation[1];
	*z = self->rotation[2];
}

void gis_viewer_rotate(GisViewer *self, gdouble x, gdouble y, gdouble z)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: rotate - x=%.0f, y=%.0f, z=%.0f", x, y, z);
	self->rotation[0] += x;
	self->rotation[1] += y;
	self->rotation[2] += z;
	_gis_viewer_emit_rotation_changed(self);
}

/* To be deprecated, use {get,set}_location */
void gis_viewer_set_site(GisViewer *self, const gchar *site)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: set_site");
	g_free(self->site);
	self->site = g_strdup(site);
	_gis_viewer_emit_site_changed(self);
}

gchar *gis_viewer_get_site(GisViewer *self)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: get_site - %s", self->site);
	return self->site;
}

void gis_viewer_refresh(GisViewer *self)
{
	g_debug("GisViewer: refresh");
	_gis_viewer_emit_refresh(self);
}

void gis_viewer_set_offline(GisViewer *self, gboolean offline)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: set_offline - %d", offline);
	self->offline = offline;
	_gis_viewer_emit_offline(self);
}

gboolean gis_viewer_get_offline(GisViewer *self)
{
	g_assert(GIS_IS_VIEWER(self));
	g_debug("GisViewer: get_offline - %d", self->offline);
	return self->offline;
}


/****************
 * GObject code *
 ****************/
G_DEFINE_TYPE(GisViewer, gis_viewer, G_TYPE_OBJECT);
static void gis_viewer_init(GisViewer *self)
{
	g_debug("GisViewer: init");
	/* Default values */
	self->time = g_strdup("");
	self->site = g_strdup("");
	self->location[0] = 40;
	self->location[1] = -100;
	self->location[2] = 1.5*EARTH_R;
	self->rotation[0] = 0;
	self->rotation[1] = 0;
	self->rotation[2] = 0;
}
static void gis_viewer_dispose(GObject *gobject)
{
	g_debug("GisViewer: dispose");
	/* Drop references to other GObjects */
	G_OBJECT_CLASS(gis_viewer_parent_class)->dispose(gobject);
}
static void gis_viewer_finalize(GObject *gobject)
{
	g_debug("GisViewer: finalize");
	GisViewer *self = GIS_VIEWER(gobject);
	g_free(self->time);
	g_free(self->site);
	G_OBJECT_CLASS(gis_viewer_parent_class)->finalize(gobject);
}
static void gis_viewer_set_property(GObject *object, guint property_id,
		const GValue *value, GParamSpec *pspec)
{
	g_debug("GisViewer: set_property");
	GisViewer *self = GIS_VIEWER(object);
	switch (property_id) {
	case PROP_TIME:    gis_viewer_set_time   (self, g_value_get_string (value)); break;
	case PROP_SITE:    gis_viewer_set_site   (self, g_value_get_string (value)); break;
	case PROP_OFFLINE: gis_viewer_set_offline(self, g_value_get_boolean(value)); break;
	default:           G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}
static void gis_viewer_get_property(GObject *object, guint property_id,
		GValue *value, GParamSpec *pspec)
{
	g_debug("GisViewer: get_property");
	GisViewer *self = GIS_VIEWER(object);
	switch (property_id) {
	case PROP_TIME:    g_value_set_string (value, gis_viewer_get_time   (self)); break;
	case PROP_SITE:    g_value_set_string (value, gis_viewer_get_site   (self)); break;
	case PROP_OFFLINE: g_value_set_boolean(value, gis_viewer_get_offline(self)); break;
	default:           G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}
static void gis_viewer_class_init(GisViewerClass *klass)
{
	g_debug("GisViewer: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose      = gis_viewer_dispose;
	gobject_class->finalize     = gis_viewer_finalize;
	gobject_class->get_property = gis_viewer_get_property;
	gobject_class->set_property = gis_viewer_set_property;
	g_object_class_install_property(gobject_class, PROP_TIME,
		g_param_spec_pointer(
			"time",
			"time of the current frame",
			"(format unknown)",
			G_PARAM_READWRITE));
	g_object_class_install_property(gobject_class, PROP_SITE,
		g_param_spec_pointer(
			"site",
			"site seen by the viewerport",
			"Site of the viewerport. "
			"Currently this is the name of the radar site.",
			G_PARAM_READWRITE));
	g_object_class_install_property(gobject_class, PROP_OFFLINE,
		g_param_spec_pointer(
			"offline",
			"whether the viewer should access the network",
			"Offline state of the viewer. "
			"If set to true, the viewer will not access the network",
			G_PARAM_READWRITE));
	signals[SIG_TIME_CHANGED] = g_signal_new(
			"time-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__STRING,
			G_TYPE_NONE,
			1,
			G_TYPE_STRING);
	signals[SIG_SITE_CHANGED] = g_signal_new(
			"site-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__STRING,
			G_TYPE_NONE,
			1,
			G_TYPE_STRING);
	signals[SIG_LOCATION_CHANGED] = g_signal_new(
			"location-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			gis_cclosure_marshal_VOID__DOUBLE_DOUBLE_DOUBLE,
			G_TYPE_NONE,
			3,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE);
	signals[SIG_ROTATION_CHANGED] = g_signal_new(
			"rotation-changed",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			gis_cclosure_marshal_VOID__DOUBLE_DOUBLE_DOUBLE,
			G_TYPE_NONE,
			3,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE,
			G_TYPE_DOUBLE);
	signals[SIG_REFRESH] = g_signal_new(
			"refresh",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			0);
	signals[SIG_OFFLINE] = g_signal_new(
			"offline",
			G_TYPE_FROM_CLASS(gobject_class),
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__BOOLEAN,
			G_TYPE_NONE,
			1,
			G_TYPE_BOOLEAN);
}
