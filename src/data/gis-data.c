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
 * SECTION:gis-data
 * @short_description: Miscellaneous utilities for data access
 * @include: glib/gstdio.h
 *
 * Various support routines for data access,
 * these are mostly related to disk caching.
 */

#include <config.h>
#include <stdio.h>
#include <glib.h>

#include "gis-data.h"

/**
 * fopen_p:
 * @path: the path to the file to be opened.
 * @mode: mode to open the file, see <function>fopen</function> for details
 *
 * Open a file, creating parent directories if needed
 *
 * Returns: the opened file descriptor
 */
FILE *fopen_p(const gchar *path, const gchar *mode)
{
	gchar *parent = g_path_get_dirname(path);
	if (!g_file_test(parent, G_FILE_TEST_EXISTS))
		g_mkdir_with_parents(parent, 0755);
	g_free(parent);
	return fopen(path, mode);
}
