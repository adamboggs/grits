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

const gchar *gis_plugin_get_name(GisPlugin *self)
{
	if (!GIS_IS_PLUGIN(self))
		return NULL;
	return GIS_PLUGIN_GET_INTERFACE(self)->name;
}

const gchar *gis_plugin_get_description(GisPlugin *self)
{
	if (!GIS_IS_PLUGIN(self))
		return NULL;
	return GIS_PLUGIN_GET_INTERFACE(self)->description;
}

GtkWidget *gis_plugin_get_config(GisPlugin *self)
{
	if (!GIS_IS_PLUGIN(self))
		return NULL;
	GisPluginInterface *iface = GIS_PLUGIN_GET_INTERFACE(self);
	return iface->get_config ? iface->get_config(self) : NULL;
}


/***************
 * Plugins API *
 ***************/
typedef struct {
	gchar *name;
	GisPlugin *plugin;
} GisPluginStore;

GisPlugins *gis_plugins_new(const gchar *dir, GisPrefs *prefs)
{
	g_debug("GisPlugins: new - dir=%s", dir);
	GisPlugins *self = g_new0(GisPlugins, 1);
	self->prefs = prefs;
	if (dir)
		self->dir = g_strdup(dir);
	return self;
}

void gis_plugins_free(GisPlugins *self)
{
	g_debug("GisPlugins: free");
	for (GList *cur = self->plugins; cur; cur = cur->next) {
		GisPluginStore *store = cur->data;
		g_debug("GisPlugin: freeing %s refs=%d->%d", store->name,
			G_OBJECT(store->plugin)->ref_count,
			G_OBJECT(store->plugin)->ref_count-1);
		g_object_unref(store->plugin);
		g_free(store->name);
		g_free(store);
	}
	g_list_free(self->plugins);
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
		if (dirs[i] == NULL)
			continue;
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
		g_dir_close(dir);
	}
	return list;
}

GisPlugin *gis_plugins_load(GisPlugins *self, const char *name,
		GisViewer *viewer, GisPrefs *prefs)
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
	store->plugin = constructor(viewer, prefs);
	self->plugins = g_list_prepend(self->plugins, store);
	return store->plugin;
}

GisPlugin *gis_plugins_enable(GisPlugins *self, const char *name,
		GisViewer *viewer, GisPrefs *prefs)
{
	GisPlugin *plugin = gis_plugins_load(self, name, viewer, prefs);
	gis_prefs_set_boolean_v(self->prefs, "plugins", name, TRUE);
	return plugin;
}

GList *gis_plugins_load_enabled(GisPlugins *self,
		GisViewer *viewer, GisPrefs *prefs)
{
	GList *loaded = NULL;
	for (GList *cur = gis_plugins_available(self); cur; cur = cur->next) {
		gchar *name = cur->data;
		if (gis_prefs_get_boolean_v(self->prefs, "plugins", name, NULL)) {
			GisPlugin *plugin = gis_plugins_load(self, name, viewer, prefs);
			loaded = g_list_prepend(loaded, plugin);
		}
	}
	return loaded;
}

gboolean gis_plugins_unload(GisPlugins *self, const char *name)
{
	g_debug("GisPlugins: unload %s", name);
	for (GList *cur = self->plugins; cur; cur = cur->next) {
		GisPluginStore *store = cur->data;
		if (g_str_equal(store->name, name)) {
			g_object_unref(store->plugin);
			g_free(store->name);
			g_free(store);
			self->plugins = g_list_delete_link(self->plugins, cur);
		}
	}
	return FALSE;
}

gboolean gis_plugins_disable(GisPlugins *self, const char *name)
{
	gis_prefs_set_boolean_v(self->prefs, "plugins", name, FALSE);
	gis_plugins_unload(self, name);
	return FALSE;
}

void gis_plugins_foreach(GisPlugins *self, GCallback _callback, gpointer user_data)
{
	g_debug("GisPlugins: foreach");
	if (self == NULL)
		return;
	typedef void (*CBFunc)(GisPlugin *, const gchar *, gpointer);
	CBFunc callback = (CBFunc)_callback;
	for (GList *cur = self->plugins; cur; cur = cur->next) {
		GisPluginStore *store = cur->data;
		callback(store->plugin, store->name, user_data);
	}
}
