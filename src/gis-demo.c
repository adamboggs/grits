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
#include <gdk/gdkkeysyms.h>

#include "gis.h"


static GisPrefs   *prefs;
static GisPlugins *plugins;
static GisViewer  *viewer;


/*************
 * Callbacks *
 *************/
static gboolean on_delete(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_main_quit();
	return TRUE;
}

static void on_offline(GtkToggleAction *action, gpointer _)
{
	gboolean active = gtk_toggle_action_get_active(action);
	gis_viewer_set_offline(viewer, active);
}

static void on_plugin(GtkToggleAction *action, GtkWidget *notebook)
{
	const gchar *name = gtk_action_get_name(GTK_ACTION(action));
	gboolean active = gtk_toggle_action_get_active(action);
	if (active) {
		GisPlugin *plugin = gis_plugins_enable(plugins, name,
				GIS_VIEWER(viewer), prefs);
		GtkWidget *config = gis_plugin_get_config(plugin);
		if (config) {
			gtk_notebook_append_page(GTK_NOTEBOOK(notebook), config,
					gtk_label_new(name));
			gtk_widget_show_all(config);
		}
	} else {
		gis_plugins_disable(plugins, name);
		guint n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
		for (int i = 0; i < n_pages; i++) {
			GtkWidget *body = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), i);
			if (!body) continue;
			GtkWidget *tab = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook), body);
			if (!tab) continue;
			const gchar *tab_name = gtk_label_get_text(GTK_LABEL(tab));
			if (tab_name && g_str_equal(name, tab_name))
				gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), i);
		}
		gtk_widget_queue_draw(GTK_WIDGET(viewer));
	}
}


/******************
 * Static UI Data *
 ******************/
static const gchar menu_xml[] =
	"<ui>"
	"  <menubar name='Menu' >"
	"    <menu name='File'          action='File' >"
	"      <menuitem name='Offline' action='Offline' />"
	"      <menuitem name='Quit'    action='Quit' />"
	"    </menu>"
	"    <menu name='Plugins'       action='Plugins' >"
	"      <placeholder name='PluginsItems' />"
	"    </menu>"
	"  </menubar>"
	"</ui>"
;

static GtkActionEntry action_data[] =
{
	/* name, stock id, label, accel, tooltip, callback */
	{"File",    NULL, "_File"},
	{"Plugins", NULL, "_Plugins"},
	{"Quit", GTK_STOCK_QUIT, "_Quit", "q", NULL,
		G_CALLBACK(gtk_main_quit)},
};
static GtkToggleActionEntry toggle_action_data[] =
{
	/* name, stock id, label, accel, tooltip, callback, is_active */
	{"Offline", GTK_STOCK_DISCONNECT, "_Offline", NULL, NULL,
		G_CALLBACK(on_offline), FALSE},
};


/***********
 * Helpers *
 ***********/
static GtkUIManager *setup_actions()
{
	GtkUIManager   *manager = gtk_ui_manager_new();
	GtkActionGroup *actions = gtk_action_group_new("Actions");
	gtk_action_group_add_actions(actions, action_data,
			G_N_ELEMENTS(action_data), NULL);
	gtk_action_group_add_toggle_actions(actions, toggle_action_data,
			G_N_ELEMENTS(toggle_action_data), NULL);
	gtk_ui_manager_insert_action_group(manager, actions, 0);
	gtk_ui_manager_add_ui_from_string(manager, menu_xml, sizeof(menu_xml)-1, NULL);
	g_object_unref(actions);
	return manager;
}

static GtkWidget *setup_window(GtkUIManager *manager, GtkWidget **_notebook)
{
	GtkWidget *window   = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *menu     = gtk_ui_manager_get_widget(manager, "/Menu");
	GtkWidget *notebook = gtk_notebook_new();
	GtkWidget *vbox     = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_box_pack_start(GTK_BOX(vbox), menu,               FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(viewer), TRUE,  TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), notebook,           FALSE, TRUE, 0);
	g_signal_connect(window, "delete-event", G_CALLBACK(on_delete), NULL);
	gtk_window_add_accel_group(GTK_WINDOW(window),
			gtk_ui_manager_get_accel_group(manager));
	*_notebook = notebook;
	return window;
}

static void setup_plugins(GtkUIManager *manager, GtkNotebook *notebook)
{
	GtkActionGroup *actions = gtk_action_group_new("Plugins");
	gtk_ui_manager_insert_action_group(manager, actions, 1);
	guint merge_id = gtk_ui_manager_new_merge_id(manager);
	for (GList *cur = gis_plugins_available(plugins); cur; cur = cur->next) {
		gchar *name = cur->data;
		GtkToggleAction *action = gtk_toggle_action_new(name, name, NULL, NULL);
		g_signal_connect(action, "toggled", G_CALLBACK(on_plugin), notebook);
		gtk_action_group_add_action(actions, GTK_ACTION(action));
		gtk_ui_manager_add_ui(manager, merge_id, "/Menu/Plugins", name, name,
				GTK_UI_MANAGER_AUTO, TRUE);
		if (gis_prefs_get_boolean_v(prefs, "plugins", name, NULL))
			gtk_toggle_action_set_active(action, TRUE);
	}
}

static void restore_states(GtkUIManager *manager)
{
	GtkAction *action = gtk_ui_manager_get_action(manager, "/Menu/File/Offline");
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),
			gis_viewer_get_offline(viewer));
}

int main(int argc, char **argv)
{
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);

	prefs   = gis_prefs_new(NULL, NULL);
	plugins = gis_plugins_new(g_getenv("GIS_PLUGIN_PATH"), prefs);
	viewer  = gis_opengl_new(plugins, prefs);

	gdk_threads_enter();

	GtkWidget    *notebook = NULL;
	GtkUIManager *manager  = setup_actions();
	GtkWidget    *window   = setup_window(manager, &notebook);
	gtk_widget_show_all(window);
	setup_plugins(manager, GTK_NOTEBOOK(notebook));
	restore_states(manager);
	gtk_ui_manager_ensure_update(manager);

	gtk_main();

	gis_plugins_free(plugins);
	g_object_unref(prefs);

	gdk_threads_leave();

	g_debug("GisDemo: main - refs=%d,%d",
			G_OBJECT(manager)->ref_count,
			G_OBJECT(window)->ref_count);
	g_object_unref(manager);
	gtk_widget_destroy(window);

	prefs   = NULL;
	plugins = NULL;
	viewer  = NULL;

	gdk_display_close(gdk_display_get_default());

	return 0;
}
