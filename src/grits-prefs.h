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

#ifndef __GRITS_PREFS_H__
#define __GRITS_PREFS_H__

#include <glib-object.h>

/* Type macros */
#define GRITS_TYPE_PREFS            (grits_prefs_get_type())
#define GRITS_PREFS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),   GRITS_TYPE_PREFS, GritsPrefs))
#define GRITS_IS_PREFS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),   GRITS_TYPE_PREFS))
#define GRITS_PREFS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST   ((klass), GRITS_TYPE_PREFS, GritsPrefsClass))
#define GRITS_IS_PREFS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE   ((klass), GRITS_TYPE_PREFS))
#define GRITS_PREFS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),   GRITS_TYPE_PREFS, GritsPrefsClass))

typedef struct _GritsPrefs      GritsPrefs;
typedef struct _GritsPrefsClass GritsPrefsClass;

struct _GritsPrefs {
	GObject parent_instance;

	/* instance members */
	gchar    *key_path;
	GKeyFile *key_file;
};

struct _GritsPrefsClass {
	GObjectClass parent_class;
	
	/* class members */
};

GType grits_prefs_get_type(void);

/* Methods */
GritsPrefs *grits_prefs_new(const gchar *config, const gchar *defaults);

gchar    *grits_prefs_get_string   (GritsPrefs *prefs, const gchar *key, GError **error);
gboolean  grits_prefs_get_boolean  (GritsPrefs *prefs, const gchar *key, GError **error);
gint      grits_prefs_get_integer  (GritsPrefs *prefs, const gchar *key, GError **error);
gdouble   grits_prefs_get_double   (GritsPrefs *prefs, const gchar *key, GError **error);

gchar    *grits_prefs_get_string_v (GritsPrefs *prefs, const gchar *group, const gchar *key, GError **error);
gboolean  grits_prefs_get_boolean_v(GritsPrefs *prefs, const gchar *group, const gchar *key, GError **error);
gint      grits_prefs_get_integer_v(GritsPrefs *prefs, const gchar *group, const gchar *key, GError **error);
gdouble   grits_prefs_get_double_v (GritsPrefs *prefs, const gchar *group, const gchar *key, GError **error);

void      grits_prefs_set_string   (GritsPrefs *prefs, const gchar *key, const gchar *string);
void      grits_prefs_set_boolean  (GritsPrefs *prefs, const gchar *key, gboolean value);
void      grits_prefs_set_integer  (GritsPrefs *prefs, const gchar *key, gint value);
void      grits_prefs_set_double   (GritsPrefs *prefs, const gchar *key, gdouble value);

void      grits_prefs_set_string_v (GritsPrefs *prefs, const gchar *group, const gchar *key, const gchar *string);
void      grits_prefs_set_boolean_v(GritsPrefs *prefs, const gchar *group, const gchar *key, gboolean value);
void      grits_prefs_set_integer_v(GritsPrefs *prefs, const gchar *group, const gchar *key, gint value);
void      grits_prefs_set_double_v (GritsPrefs *prefs, const gchar *group, const gchar *key, gdouble value);
#endif
