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

#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define COUNT(arr) (sizeof(arr)/sizeof(arr[0]))

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(250,250);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Hello World!");


	/* Query max size */
	GLint size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	printf("\nGL_MAX_TEXTURE_SIZE = %d\n", size);


	/* Test extensions */
	const char *exts[] = {
		"GL_ARB_texture_rectangle",
		"GL_ARB_does_not_exist",
	};
	printf("\nChecking some extensions...\n");
	const char *avail  = (char*)glGetString(GL_EXTENSIONS);
	for (int i = 0; i < COUNT(exts); i++)
		printf("\t%s: %s\n", exts[i],
			strstr(avail,exts[i]) ? "OK" : "Error");


	/* Test sample image */
	GLint sizes[][2] = {
		{3400, 1600},
		{1024, 1024},
		{4096, 4096},
		{8192, 8192},
	};
	printf("\nTrying some sizes...\n");
	for (int i = 0; i < COUNT(sizes); i++) {
		GLint width  = sizes[i][0];
		GLint height = sizes[i][1];
		printf("\t%dx%d: ", width, height);
		glTexImage2D(GL_PROXY_TEXTURE_2D, 0, 4, width, height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &width);
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		printf("%s\n", width && height ? "OK" : "Error");
	}


	/* Wait */
	printf("\nPress any key to exit..\n");
	getchar();
}
