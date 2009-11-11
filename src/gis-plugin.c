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
#include <gmodule.h>

#include "gis-plugin.h"

/********************
 * Plugin interface *
 ********************/
static void gis_plugin_base_init(gpointer g_class)
{
	static gboolean is_initialized = FALSE;
	if (!is_initialized) {
		/* add properties and signals to the interface here */
		is_initialized = TRUE;
	}
}

GType gis_plugin_get_type()
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof(GisPluginInterface),
			gis_plugin_base_init,
			NULL,
		};
		type = g_type_register_static(G_TYPE_INTERFACE,
				"GisPlugin", &info, 0);
	}
	return type;
}

void gis_plugin_expose(GisPlugin *self)
{
	g_return_if_fail(GIS_IS_PLUGIN(self));
	GIS_PLUGIN_GET_INTERFACE(self)->expose(self);
}

GtkWidget *gis_plugin_get_config(GisPlugin *self)
{
	g_return_val_if_fail(GIS_IS_PLUGIN(self), NULL);
	GisPluginInterface *iface = GIS_PLUGIN_GET_INTERFACE(self);
	return iface->get_config ? iface->get_config (self) : NULL;
}


/***************
 * Plugins API *
 ***************/
typedef struct {
	gchar *name;
	GisPlugin *plugin;
} GisPluginStore;

GisPlugins *gis_plugins_new(gchar *dir)
{
	g_debug("GisPlugins: new - dir=%s", dir);
	GisPlugins *plugins = g_new0(GisPlugins, 1);
	if (dir)
		plugins->dir = g_strdup(dir);
	plugins->plugins = g_ptr_array_new();
	return plugins;
}

void gis_plugins_free(GisPlugins *self)
{
	g_debug("GisPlugins: free");
	for (int i = 0; i < self->plugins->len; i++) {
		GisPluginStore *store = g_ptr_array_index(self->plugins, i);
		g_object_unref(store->plugin);
		g_free(store->name);
		g_free(store);
		g_ptr_array_remove_index(self->plugins, i);
	}
	g_ptr_array_free(self->plugins, TRUE);
	if (self->dir)
		g_free(self->dir);
	g_free(self);
}

GList *gis_plugins_available(GisPlugins *self)
{
	g_debug("GisPlugins: available");
	GList *list = NULL;
	gchar *dirs[] = {self->dir, PLUGINSDIR};
	g_debug("pluginsdir=%s", PLUGINSDIR);
	for (int i = 0; i<2; i++) {
		GDir *dir = g_dir_open(dirs[i], 0, NULL);
		if (dir == NULL)
			continue;
		g_debug("            checking %s", dirs[i]);
		const gchar *name;
		while ((name = g_dir_read_name(dir))) {
			if (g_pattern_match_simple("*." G_MODULE_SUFFIX, name)) {
				gchar **parts = g_strsplit(name, ".", 2);
				list = g_list_prepend(list, g_strdup(parts[0]));
				g_strfreev(parts);
			}
		}
	}
	return list;
}

GisPlugin *gis_plugins_load(GisPlugins *self, const char *name,
		GisWorld *world, GisView *view, GisOpenGL *opengl, GisPrefs *prefs)
{
	g_debug("GisPlugins: load %s", name);
	gchar *path = g_strdup_printf("%s/%s.%s", self->dir, name, G_MODULE_SUFFIX);
	g_debug("GisPlugins: load - trying %s", path);
	if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
		g_free(path);
		path = g_strdup_printf("%s/%s.%s", PLUGINSDIR, name, G_MODULE_SUFFIX);
	}
	g_debug("GisPlugins: load - trying %s", path);
	if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
		g_warning("Module %s not found", name);
		g_free(path);
		return NULL;
	}
	GModule *module = g_module_open(path, G_MODULE_BIND_LAZY);
	g_free(path);
	if (module == NULL) {
		g_warning("Unable to load module %s: %s", name, g_module_error());
		return NULL;
	}

	gpointer constructor_ptr; // GCC 4.1 fix?
	gchar *constructor_str = g_strconcat("gis_plugin_", name, "_new", NULL);
	if (!g_module_symbol(module, constructor_str, &constructor_ptr)) {
		g_warning("Unable to load symbol %s from %s: %s",
				constructor_str, name, g_module_error());
		g_module_close(module);
		g_free(constructor_str);
		return NULL;
	}
	g_free(constructor_str);
	GisPluginConstructor constructor = constructor_ptr;

	GisPluginStore *store = g_new0(GisPluginStore, 1);
	store->name = g_strdup(name);
	store->plugin = constructor(world, view, opengl, prefs);
	g_ptr_array_add(self->plugins, store);
	return store->plugin;
}

gboolean gis_plugins_unload(GisPlugins *self, const char *name)
{
	g_debug("GisPlugins: unload %s", name);
	for (int i = 0; i < self->plugins->len; i++) {
		GisPluginStore *store = g_ptr_array_index(self->plugins, i);
		if (g_str_equal(store->name, name)) {
			g_object_unref(store->plugin);
			g_free(store->name);
			g_free(store);
			g_ptr_array_remove_index(self->plugins, i);
		}
	}
	return FALSE;
}
void gis_plugins_foreach(GisPlugins *self, GCallback _callback, gpointer user_data)
{
	g_debug("GisPlugins: foreach");
	typedef void (*CBFunc)(GisPlugin *, const gchar *, gpointer);
	CBFunc callback = (CBFunc)_callback;
	for (int i = 0; i < self->plugins->len; i++) {
		GisPluginStore *store = g_ptr_array_index(self->plugins, i);
		callback(store->plugin, store->name, user_data);
	}
}
