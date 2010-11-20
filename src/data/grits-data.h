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

#ifndef __GRITS_DATA_H__
#define __GRITS_DATA_H__

#include <glib.h>

/**
 * GritsCacheType:
 * @GRITS_LOCAL:   Only return local files (for offline mode)
 * @GRITS_ONCE:    Download the file only if it does not exist
 * @GRITS_UPDATE:  Update the file to be like the server
 * @GRITS_REFRESH: Delete the existing file and fetch a new copy
 *
 * Various methods for caching data
 */
typedef enum {
	GRITS_LOCAL,
	GRITS_ONCE,
	GRITS_UPDATE,
	GRITS_REFRESH,
} GritsCacheType;

/**
 * GritsChunkCallback:
 * @file:      path to the file which is being fetched
 * @cur:       current offset in the file
 * @total:     total size of the file
 * @user_data: the user_data argument passed to the function
 *
 * Function called when part of a file is fetched
 * Used for updating progress bars, etc
 */
typedef void (*GritsChunkCallback)(gchar *file, goffset cur,
		goffset total, gpointer user_data);

FILE *fopen_p(const gchar *path, const gchar *mode);

#endif
