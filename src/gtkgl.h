/*
 * Copyright (C) 2011 Andy Spencer <andy753421@gmail.com>
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

#ifndef __GTK_GL_H__
#define __GTK_GL_H__

#include <gtk/gtk.h>

/* Platform dependant OpenGL includes */
#ifdef SYS_MAC
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

/* Call before widget is realized */
void gtk_gl_enable(GtkWidget *widget);

/* Call at the start of "expose" */
void gtk_gl_begin(GtkWidget *widget);

/* Call at the end of "expose-event" */
void gtk_gl_end(GtkWidget *widget);

/* Call when done to cleanup data */
void gtk_gl_disable(GtkWidget *widget);

#endif
