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

#include <config.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gis.h"

/*************
 * Callbacks *
 *************/
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event,
		gpointer _window)
{
	g_debug("GisTest: on_key_press - key=%x, state=%x",
			event->keyval, event->state);
	GtkWidget *window = _window;
	switch (event->keyval) {
	case GDK_q:
		gtk_widget_destroy(window);
		return TRUE;
	}
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

	GisPrefs   *prefs   = gis_prefs_new(NULL, NULL);
	GisPlugins *plugins = gis_plugins_new(NULL);
	GisViewer  *viewer  = gis_opengl_new(plugins);

	gdk_threads_enter();
	GtkWidget  *window  = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window,  "destroy",         G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window,  "key-press-event", G_CALLBACK(on_key_press),  window);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(viewer));
	gtk_widget_show_all(window);
	gdk_threads_leave();

	gis_plugins_load(plugins, "bmng", viewer, prefs);
	gis_plugins_load(plugins, "srtm", viewer, prefs);
	gis_plugins_load(plugins, "test", viewer, prefs);

	gdk_threads_enter();
	gtk_main();

	gis_plugins_free(plugins);
	g_object_unref(prefs);
	gdk_threads_leave();
	return 0;
}
