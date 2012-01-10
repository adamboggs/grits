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

/**
 * SECTION:test
 * @short_description: Testing plugin
 *
 * #GritsPluginTest is a testing plugin used during development and as an example
 * for how to create a plugin.
 */

#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <grits.h>

#include "test.h"

static void on_poly_enter(GritsPoly *poly)
{
	g_debug("on_poly_enter");
	poly->color[3] = 0.50;
	grits_object_queue_draw(GRITS_OBJECT(poly));
}

static void on_poly_leave(GritsPoly *poly)
{
	g_debug("on_poly_leave");
	poly->color[3] = 0.2;
	grits_object_queue_draw(GRITS_OBJECT(poly));
}

static void on_poly_button(GritsPoly *poly, GdkEventButton *event)
{
	g_debug("on_poly_button");
	static int i = 0;
	gdouble colors[][3] = {
		{1, 0, 0}, {1, 1, 0},
		{0, 1, 0}, {0, 1, 1},
		{0, 0, 1}, {1, 0, 1},
	};
	int idx = i++ % G_N_ELEMENTS(colors);
	memcpy(poly->color, colors[idx], sizeof(gdouble)*3);
	grits_object_queue_draw(GRITS_OBJECT(poly));
}

static void on_poly_key(GritsPoly *poly, GdkEventKey *event)
{
	g_debug("on_poly_key");
	gdouble colors[0xff][3] = {
		[GDK_r] {1, 0, 0},
		[GDK_g] {0, 1, 0},
		[GDK_b] {0, 0, 1},
	};
	int key = event->keyval;
	memcpy(poly->color, colors[key], sizeof(gdouble)*3);
	grits_object_queue_draw(GRITS_OBJECT(poly));
}

static void on_marker_enter(GritsMarker *marker, GritsViewer *viewer)
{
	g_debug("on_marker_enter");
	GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(viewer));
	GdkCursor *cursor = gdk_cursor_new(GDK_HAND1);
	gdk_window_set_cursor(window, cursor);
}

static void on_marker_leave(GritsMarker *marker, GritsViewer *viewer)
{
	g_debug("on_marker_leave");
	GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(viewer));
	gdk_window_set_cursor(window, NULL);
}

static void on_marker_button(GritsMarker *marker, GdkEventButton *event)
{
	g_debug("on_marker_button");
	GtkWidget *dialog = gtk_dialog_new_with_buttons(
			"St. Charles!", NULL, 0, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	gtk_dialog_run(GTK_DIALOG(dialog));
}

/***********
 * Methods *
 ***********/
void _load_marker(GritsPluginTest *test)
{
	test->marker = grits_marker_new("St. Charles");
	GRITS_OBJECT(test->marker)->center.lat  =  38.841847;
	GRITS_OBJECT(test->marker)->center.lon  = -90.491982;
	GRITS_OBJECT(test->marker)->center.elev =   0.0;
	GRITS_OBJECT(test->marker)->lod         = EARTH_R;
	grits_viewer_add(test->viewer, GRITS_OBJECT(test->marker), GRITS_LEVEL_OVERLAY, FALSE);
	/* These do not work on marker yet */
	//g_signal_connect(test->marker, "enter",        G_CALLBACK(on_marker_enter),  NULL);
	//g_signal_connect(test->marker, "leave",        G_CALLBACK(on_marker_leave),  NULL);
	//g_signal_connect(test->marker, "button-press", G_CALLBACK(on_marker_button), NULL);
	(void)on_marker_enter;
	(void)on_marker_leave;
	(void)on_marker_button;
}

void _load_poly(GritsPluginTest *test)
{
	test->poly = grits_poly_parse("35,-90 35,-110 45,-110 45,-90", "\t", " ", ",");
	test->poly->color[0]  = test->poly->border[0] = 1;
	test->poly->color[1]  = test->poly->border[1] = 0;
	test->poly->color[2]  = test->poly->border[2] = 0;
	test->poly->color[3]  = 0.2;
	test->poly->border[3] = 1;
	test->poly->width     = 6;
	grits_viewer_add(test->viewer, GRITS_OBJECT(test->poly),  GRITS_LEVEL_OVERLAY, TRUE);
	g_signal_connect(test->poly, "enter",        G_CALLBACK(on_poly_enter),  NULL);
	g_signal_connect(test->poly, "leave",        G_CALLBACK(on_poly_leave),  NULL);
	g_signal_connect(test->poly, "button-press", G_CALLBACK(on_poly_button), NULL);
	g_signal_connect(test->poly, "key-press",    G_CALLBACK(on_poly_key),    NULL);
}

void _load_line(GritsPluginTest *test)
{
	test->line = grits_line_parse("30,-80 30,-120 50,-120 50,-80", "\t", " ", ",");
	test->line->color[0]  = 1;
	test->line->color[1]  = 0;
	test->line->color[2]  = 0;
	test->line->color[3]  = 1;
	test->line->width     = 8;
	grits_viewer_add(test->viewer, GRITS_OBJECT(test->line),  GRITS_LEVEL_OVERLAY, TRUE);
	g_signal_connect(test->line, "enter",        G_CALLBACK(on_poly_enter),  NULL);
	g_signal_connect(test->line, "leave",        G_CALLBACK(on_poly_leave),  NULL);
	g_signal_connect(test->line, "button-press", G_CALLBACK(on_poly_button), NULL);
	g_signal_connect(test->line, "key-press",    G_CALLBACK(on_poly_key),    NULL);
}

/**
 * grits_plugin_test_new:
 * @viewer: the #GritsViewer to use for drawing
 *
 * Create a new instance of the testing plugin.
 *
 * Returns: the new #GritsPluginTest
 */
GritsPluginTest *grits_plugin_test_new(GritsViewer *viewer)
{
	g_debug("GritsPluginTest: new");
	GritsPluginTest *test = g_object_new(GRITS_TYPE_PLUGIN_TEST, NULL);
	test->viewer = g_object_ref(viewer);
	_load_marker(test);
	_load_poly(test);
	_load_line(test);
	return test;
}


/****************
 * GObject code *
 ****************/
/* Plugin init */
static void grits_plugin_test_plugin_init(GritsPluginInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GritsPluginTest, grits_plugin_test, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(GRITS_TYPE_PLUGIN,
			grits_plugin_test_plugin_init));
static void grits_plugin_test_plugin_init(GritsPluginInterface *iface)
{
	g_debug("GritsPluginTest: plugin_init");
	/* Add methods to the interface */
}
/* Class/Object init */
static void grits_plugin_test_init(GritsPluginTest *test)
{
	g_debug("GritsPluginTest: init");
}
static void grits_plugin_test_dispose(GObject *_test)
{
	g_debug("GritsPluginTest: dispose");
	GritsPluginTest *test = GRITS_PLUGIN_TEST(_test);
	if (test->viewer) {
		grits_viewer_remove(test->viewer, GRITS_OBJECT(test->marker));
		grits_viewer_remove(test->viewer, GRITS_OBJECT(test->poly));
		grits_viewer_remove(test->viewer, GRITS_OBJECT(test->line));
		g_object_unref(test->viewer);
		test->viewer = NULL;
	}
	G_OBJECT_CLASS(grits_plugin_test_parent_class)->dispose(_test);
}
static void grits_plugin_test_class_init(GritsPluginTestClass *klass)
{
	g_debug("GritsPluginTest: class_init");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = grits_plugin_test_dispose;
}
