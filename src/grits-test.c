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
static void load_plugin(GisPlugins *plugins, gchar *name,
		GisViewer *viewer, GisPrefs *prefs, GtkNotebook *notebook)
{
	GisPlugin *plugin = gis_plugins_load(plugins, name, viewer, prefs);
	GtkWidget *config = gis_plugin_get_config(plugin);
	gtk_notebook_append_page(notebook, config, gtk_label_new(name));
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
	GtkWidget *vbox   = gtk_vbox_new(FALSE, 0);
	GtkWidget *config = gtk_notebook_new();
	g_signal_connect(window, "delete-event",    G_CALLBACK(on_delete),    NULL);
	g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(config), GTK_POS_BOTTOM);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(viewer), TRUE,  TRUE,  0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(config), FALSE, FALSE, 0);
	gtk_widget_show_all(window);

	/* Configurable stuff */
	gis_viewer_set_offline(viewer, TRUE);
	load_plugin(plugins, "elev",  viewer, prefs, GTK_NOTEBOOK(config));
	load_plugin(plugins, "env",   viewer, prefs, GTK_NOTEBOOK(config));
	//load_plugin(plugins, "map",   viewer, prefs, GTK_NOTEBOOK(config));
	load_plugin(plugins, "sat",   viewer, prefs, GTK_NOTEBOOK(config));
	load_plugin(plugins, "test",  viewer, prefs, GTK_NOTEBOOK(config));
	//load_plugin(plugins, "radar", viewer, prefs, GTK_NOTEBOOK(config));

	gtk_widget_show_all(config);
	gtk_main();
	gdk_threads_leave();

	gdk_display_close(gdk_display_get_default());
	return 0;
}
