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

#ifndef __GIS_DATA_H__
#define __GIS_DATA_H__

#include <glib.h>

/**
 * Various ways to cach a file
 */
typedef enum {
	GIS_LOCAL,   // Only return local files (for offline mode)
	GIS_ONCE,    // Download the file only if it does not exist
	GIS_UPDATE,  // Update the file to be like the server
	GIS_REFRESH, // Delete the existing file and fetch a new copy
} GisCacheType;

/**
 * Function called when part of a file is fetched
 * Used for updating progress bars, etc
 */
typedef void (*GisChunkCallback)(gchar *file, goffset cur,
		goffset total, gpointer user_data);

/**
 * Open a file and create the parent directory if necessasairy
 */
FILE *fopen_p(const gchar *path, const gchar *mode);

#endif
