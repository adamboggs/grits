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
 * SECTION:grits-plugin
 * @short_description: Plugin support
 *
 * A plugin in grits is a GObject which implements the GritsPlugin interface.
 * Additionally, each plugin is compiled to a separate shared object and loaded
 * conditionally at runtime when the plugin is enabled. Each such shared object
 * should define a GritsPluginConstructor() function named
 * grits_plugin_NAME_new which will be called when loading the plugin.
 *
 * Almost all grits functionality is provided by a set of plugins. Each plugin
 * can how however much it likes. The interface between plugins and the rest of
 * grits is intentionally very thin. Since grits is the library, plugins must
 * manually do everything. For instance, to draw something in the world, the
 * plugin must add an object to the viewer. Likewise, plugins need to
 * register callbacks on the viewer in order to receive updates, very little
 * happens automagically.
 *
 * That being said, one thing that plugins do do automagically, is provide a
 * configuration area.  Since the plugin doesn't know what application is is
 * being loaded form, it is better for the application to ask the plugin for
 * it's confirmation area, not the other way around.
 */

#include <glib.h>
#include <gmodule.h>

#include <string.h>

#include "grits-plugin.h"

/********************
 * Plugin interface *
 ********************/
static void grits_plugin_base_init(gpointer g_class)
{
	static gboolean is_initialized = FALSE;
	if (!is_initialized) {
		/* add properties and signals to the interface here */
		is_initialized = TRUE;
	}
}

GType grits_plugin_get_type()
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof(GritsPluginInterface),
			grits_plugin_base_init,
			NULL,
		};
		type = g_type_register_static(G_TYPE_INTERFACE,
				"GritsPlugin", &info, 0);
	}
	return type;
}

/**
 * grits_plugin_get_name:
 * @plugin: the plugin
 *
 * Get a short human readable name for a plugin, this is not necessarily the
 * same as the name of the shared object.
 *
 * Returns: a short name for the plugin
 */
const gchar *grits_plugin_get_name(GritsPlugin *plugin)
{
	if (!GRITS_IS_PLUGIN(plugin))
		return NULL;
	return GRITS_PLUGIN_GET_INTERFACE(plugin)->name;
}

/**
 * grits_plugin_get_description:
 * @plugin: the plugin
 *
 * Get a description of a plugin
 *
 * Returns: a description of the plugin
 */
const gchar *grits_plugin_get_description(GritsPlugin *plugin)
{
	if (!GRITS_IS_PLUGIN(plugin))
		return NULL;
	return GRITS_PLUGIN_GET_INTERFACE(plugin)->description;
}

/**
 * grits_plugin_get_config:
 * @plugin: the plugin
 *
 * Each plugin can provide a configuration area. Applications using grits
 * should display this configuration area to the user so they can modify the
 * behavior of the plugin.
 *
 * Returns: a configuration widget for the plugin
 */
GtkWidget *grits_plugin_get_config(GritsPlugin *plugin)
{
	if (!GRITS_IS_PLUGIN(plugin))
		return NULL;
	GritsPluginInterface *iface = GRITS_PLUGIN_GET_INTERFACE(plugin);
	return iface->get_config ? iface->get_config(plugin) : NULL;
}


/***************
 * Plugins API *
 ***************/
typedef struct {
	gchar       *name;
	GritsPlugin *plugin;
	GModule     *module;
} GritsPluginStore;

/**
 * grits_plugins_new:
 * @dir:   the directory to search for plugins in
 * @prefs: a #GritsPrefs to save the state of plugins, or NULL
 *
 * Create a new plugin source. If @prefs is not %NULL, the state of the plugins
 * will be saved when they are either enabled or disabled.
 *
 * Returns: the new plugin source
 */
GritsPlugins *grits_plugins_new(const gchar *dir, GritsPrefs *prefs)
{
	g_debug("GritsPlugins: new - dir=%s", dir);
	GritsPlugins *plugins = g_new0(GritsPlugins, 1);
	plugins->prefs = prefs;
	if (dir)
		plugins->dir = g_strdup(dir);
	return plugins;
}

static void grits_plugins_free_store(GritsPluginStore *store)
{
	g_object_unref(store->plugin);
	//g_module_close(store->module);
	g_free(store->name);
	g_free(store);
}

/**
 * grits_plugins_free:
 * @plugins: the #GritsPlugins to free
 *
 * Free data used by a plugin source
 */
void grits_plugins_free(GritsPlugins *plugins)
{
	g_debug("GritsPlugins: free");
	for (GList *cur = plugins->plugins; cur; cur = cur->next) {
		GritsPluginStore *store = cur->data;
		g_debug("GritsPlugin: freeing %s refs=%d->%d", store->name,
			G_OBJECT(store->plugin)->ref_count,
			G_OBJECT(store->plugin)->ref_count-1);
		grits_plugins_free_store(store);
	}
	g_list_free(plugins->plugins);
	if (plugins->dir)
		g_free(plugins->dir);
	g_free(plugins);
}

/**
 * grits_plugins_available:
 * @plugins: the plugin source
 *
 * Search the plugin directory for shared objects which can be loaded as
 * plugins.
 *
 * Returns: the list of available plugins
 */
GList *grits_plugins_available(GritsPlugins *plugins)
{
	g_debug("GritsPlugins: available");
	GList *list = NULL;
	gchar *dirs[] = {plugins->dir, PLUGINSDIR};
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
	list = g_list_sort(list, (GCompareFunc)strcmp);
	for (GList *cur = list; cur; cur = cur->next)
		while (cur->next && g_str_equal(cur->data,cur->next->data)) {
			GList *tmp = cur->next;
			list = g_list_remove_link(list, cur);
			cur = tmp;
		}
	return list;
}

/**
 * grits_plugins_load:
 * @plugins: the plugins source
 * @name:    the name of the plugin to load
 * @viewer:  a #GritsViewer to pass to the plugins constructor
 * @prefs:   a #GritsPrefs to pass to the plugins constructor
 *
 * @name should be the name of the shared object without the file extension.
 * This is the same as what is returned by grits_plugins_available().
 *
 * When loading plugins, the @prefs argument is used, not the #GritsPrefs stored
 * in @plugins.
 *
 * Returns: the new plugin
 */
GritsPlugin *grits_plugins_load(GritsPlugins *plugins, const char *name,
		GritsViewer *viewer, GritsPrefs *prefs)
{
	g_debug("GritsPlugins: load %s", name);
	gchar *path = g_strdup_printf("%s/%s.%s", plugins->dir, name, G_MODULE_SUFFIX);
	g_debug("GritsPlugins: load - trying %s", path);
	if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
		g_free(path);
		path = g_strdup_printf("%s/%s.%s", PLUGINSDIR, name, G_MODULE_SUFFIX);
	}
	g_debug("GritsPlugins: load - trying %s", path);
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
	gchar *constructor_str = g_strconcat("grits_plugin_", name, "_new", NULL);
	if (!g_module_symbol(module, constructor_str, &constructor_ptr)) {
		g_warning("Unable to load symbol %s from %s: %s",
				constructor_str, name, g_module_error());
		g_module_close(module);
		g_free(constructor_str);
		return NULL;
	}
	g_free(constructor_str);
	GritsPluginConstructor constructor = constructor_ptr;

	GritsPluginStore *store = g_new0(GritsPluginStore, 1);
	store->name = g_strdup(name);
	store->plugin = constructor(viewer, prefs);
	store->module = module;
	plugins->plugins = g_list_prepend(plugins->plugins, store);
	return store->plugin;
}

/**
 * grits_plugins_enable:
 * @plugins: the plugins source
 * @name:    the name of the plugin to load
 * @viewer:  a #GritsViewer to pass to the plugins constructor
 * @prefs:   a #GritsPrefs to pass to the plugins constructor
 *
 * Load a plugin and save it's loaded/unloaded state in the #GritsPrefs stored in
 * #plugins.
 *
 * See also: grits_plugins_load()
 *
 * Returns: the new plugin
 */
GritsPlugin *grits_plugins_enable(GritsPlugins *plugins, const char *name,
		GritsViewer *viewer, GritsPrefs *prefs)
{
	GritsPlugin *plugin = grits_plugins_load(plugins, name, viewer, prefs);
	grits_prefs_set_boolean_v(plugins->prefs, "plugins", name, TRUE);
	return plugin;
}

/**
 * grits_plugins_load_enabled:
 * @plugins: the plugins source
 * @viewer:  a #GritsViewer to pass to the plugins constructor
 * @prefs:   a #GritsPrefs to pass to the plugins constructor
 *
 * Load all enabled which have previously been enabled.
 *
 * See also: grits_plugins_load()
 *
 * Returns: a list of all loaded plugins
 */
GList *grits_plugins_load_enabled(GritsPlugins *plugins,
		GritsViewer *viewer, GritsPrefs *prefs)
{
	GList *loaded = NULL;
	for (GList *cur = grits_plugins_available(plugins); cur; cur = cur->next) {
		gchar *name = cur->data;
		if (grits_prefs_get_boolean_v(plugins->prefs, "plugins", name, NULL)) {
			GritsPlugin *plugin = grits_plugins_load(plugins, name, viewer, prefs);
			loaded = g_list_prepend(loaded, plugin);
		}
	}
	return loaded;
}

/**
 * grits_plugins_unload:
 * @plugins: the plugins source
 * @name:    the name of the plugin to unload
 *
 * Unload a plugin and free any associated data.
 *
 * Returns: %FALSE
 */
gboolean grits_plugins_unload(GritsPlugins *plugins, const char *name)
{
	g_debug("GritsPlugins: unload %s", name);
	for (GList *cur = plugins->plugins; cur; cur = cur->next) {
		GritsPluginStore *store = cur->data;
		if (g_str_equal(store->name, name)) {
			plugins->plugins = g_list_delete_link(plugins->plugins, cur);
			grits_plugins_free_store(store);
		}
	}
	return FALSE;
}

/**
 * grits_plugins_disable:
 * @plugins: the plugins source
 * @name:    the name of the plugin to unload
 *
 * Unload a plugin and save it's loaded/unloaded state in the #GritsPrefs stored
 * in #plugins.
 *
 * See also: grits_plugins_unload()
 *
 * Returns: %FALSE
 */
gboolean grits_plugins_disable(GritsPlugins *plugins, const char *name)
{
	grits_prefs_set_boolean_v(plugins->prefs, "plugins", name, FALSE);
	grits_plugins_unload(plugins, name);
	return FALSE;
}

/**
 * grits_plugins_foreach:
 * @plugins:   the plugins source
 * @callback:  a function to call on each plugin
 * @user_data: user data to pass to the function
 *
 * Iterate over all plugins loaded by the plugins source
 */
void grits_plugins_foreach(GritsPlugins *plugins, GCallback _callback, gpointer user_data)
{
	g_debug("GritsPlugins: foreach");
	if (plugins == NULL)
		return;
	typedef void (*CBFunc)(GritsPlugin *, const gchar *, gpointer);
	CBFunc callback = (CBFunc)_callback;
	for (GList *cur = plugins->plugins; cur; cur = cur->next) {
		GritsPluginStore *store = cur->data;
		callback(store->plugin, store->name, user_data);
	}
}
