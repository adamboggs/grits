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

#include <config.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>

#include "gis.h"

GisPrefs   *prefs   = NULL;
GisPlugins *plugins = NULL;
GisViewer  *viewer  = NULL;

/*************
 * Callbacks *
 *************/
static gboolean gis_shutdown(GtkWidget *window)
{
	gis_plugins_free(plugins);
	g_object_unref(prefs);
	gtk_widget_destroy(window);

	while (gtk_events_pending())
		  gtk_main_iteration();

	gtk_main_quit();
	return TRUE;
}
static gboolean on_delete(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	return gis_shutdown(widget);
}
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event,
		gpointer window)
{
	if (event->keyval == GDK_q)
		return gis_shutdown(widget);
	return FALSE;
}

/***********
 * Methods *
 ***********/
int main(int argc, char **argv)
{
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);

	prefs   = gis_prefs_new(NULL, NULL);
	plugins = gis_plugins_new(g_getenv("GIS_PLUGIN_PATH"), prefs);
	viewer  = gis_opengl_new(plugins, prefs);

	gdk_threads_enter();
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "delete-event",    G_CALLBACK(on_delete),    NULL);
	g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(viewer));
	gtk_widget_show_all(window);


	/* elev env map sat test */
	gis_plugins_load(plugins, "elev",  viewer, prefs);
	gis_plugins_load(plugins, "env",   viewer, prefs);
	gis_plugins_load(plugins, "map",   viewer, prefs);
	gis_plugins_load(plugins, "sat",   viewer, prefs);
	gis_plugins_load(plugins, "test",  viewer, prefs);

	gtk_main();

	gdk_threads_leave();
	return 0;
}
