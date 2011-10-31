/*
 * Copyright (C) 2009-2011 Andy Spencer <andy753421@gmail.com>
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

#define GL_GLEXT_PROTOTYPES
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>

gdouble pos[3];
gdouble rot[3];
guint tex;

typedef struct {
	gfloat xyz[4];
	gfloat color[4];
	gfloat texture[4];
} __attribute__ ((packed)) vert_t;

static int sort_cmp(const void *a, const void *b)
{
	vert_t *a_verts = (vert_t*)(((gfloat*)a)+2);
	vert_t *b_verts = (vert_t*)(((gfloat*)b)+2);
	gfloat a_sum = a_verts[0].xyz[2] + a_verts[1].xyz[2] + a_verts[2].xyz[2];
	gfloat b_sum = b_verts[0].xyz[2] + b_verts[1].xyz[2] + b_verts[2].xyz[2];
	return a_sum == b_sum ? 0 :
	       a_sum <  b_sum ? 1 : -1;
}

static gfloat *sort_start()
{
	int size = 1000000;
	gfloat *data = g_new0(gfloat, size);
	glFeedbackBuffer(size, GL_4D_COLOR_TEXTURE, data);
	glRenderMode(GL_FEEDBACK);
	g_print("1st = %f\n", data[0]);
	return data;
}

static void sort_end(gfloat *data)
{
	int vertsize = sizeof(vert_t)/sizeof(gfloat);
	int nvals = glRenderMode(GL_RENDER);

	/* Set up screen coords */
	gint view[4];
	glGetIntegerv(GL_VIEWPORT, view);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(view[0],view[2], view[1],view[3], -100,100);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	//glTranslatef(pos[0], pos[1], pos[2]);
	//glTranslatef(0.05, 0.1, -2);
	//glRotatef(0.03, 1, 1, 0);

	//gluPerspective(90, 0.8, 0.1, 10);


	/* Sort the vertexes (this only works with all-triangles */
	int trisize = 2*sizeof(gfloat) + 3*sizeof(vert_t);
	int ntris   = nvals*sizeof(gfloat) / trisize;
	g_print("%d, %d, %d\n", sizeof(gfloat), trisize, ntris);
	qsort(data, ntris, trisize, sort_cmp);

	/* Draw the data */
	for (int i = 0; i < nvals;) {
		gfloat token = data[i++];
		if (token == GL_POLYGON_TOKEN) {
			gfloat n = data[i++];
			vert_t *verts = (vert_t*)&data[i];
			i += n*vertsize;
			//g_print("GL_POLYGON_TOKEN: %f\n", n);

			/* Draw triangle */
			glBegin(GL_TRIANGLES);
			for (int j = 0; j < n; j++) {
				g_print("\t%7.2f, %6.2f, %6.2f %6.2f - "
				        "%5.2f, %5.2f, %5.2f, %5.2f\n",
					verts[j].xyz[0],
					verts[j].xyz[1],
					verts[j].xyz[2],
					verts[j].xyz[3],
					verts[j].texture[0],
					verts[j].texture[1],
					verts[j].texture[2],
					verts[j].texture[3]);
				glTexCoord4f(
					verts[j].texture[0],
					verts[j].texture[1],
					verts[j].texture[2],
					verts[j].texture[3]);
				glColor4f(
					verts[j].color[0],
					verts[j].color[1],
					verts[j].color[2],
					verts[j].color[3]);
				if (j == 2)
					verts[j].xyz[2] = -100;
				glVertex3f(
					verts[j].xyz[0],
					verts[j].xyz[1],
					verts[j].xyz[2]);
			}
			glEnd();

			/* Draw line */
			//glDisable(GL_TEXTURE_2D);
			//glBegin(GL_LINE_LOOP);
			//glColor4f(1,1,1,1);
			//for (int j = 0; j < n; j++)
			//	glVertex3fv(verts[j].xyz);
			//glEnd();
		} else {
			g_error("Unknown token: %f\n", token);
		}
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	g_free(data);
}

static gboolean on_expose(GtkWidget *drawing, GdkEventExpose *event, gpointer _)
{
	//glClearColor(0.5, 0.5, 1.0, 1.0);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);

	/* Blend, but broken sorting */
	//glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* No sorting, just add */
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);

	/* Lighting */
	float light_ambient[]  = {0.1f, 0.1f, 0.0f, 1.0f};
	float light_diffuse[]  = {0.9f, 0.9f, 0.9f, 1.0f};
	float light_position[] = {-30.0f, 50.0f, 40.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	/* Draw teapots */
	g_print("%f,%f,%f\n", pos[0], pos[1], pos[2]);
	g_print("%f,%f,%f\n", rot[0], rot[1], rot[2]);

	glMatrixMode(GL_MODELVIEW);

	gfloat *data = sort_start();
	glLoadIdentity();
	glTranslatef(pos[0], pos[1], pos[2]);
	glTranslatef(0.05, 0.1, -2);
	glRotatef(60, 1, -1, -0.2);
	glColor4f(1,1,1,1);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-0.8,  0.5, 0.0);
	glTexCoord2f(0, 1); glVertex3f(-0.8, -0.5, 0.0);
	glTexCoord2f(1, 1); glVertex3f( 0.8, -0.5, 0.0);
	glTexCoord2f(1, 0); glVertex3f( 0.8,  0.5, 0.0);
	glEnd();

	//glLoadIdentity();
	//glTranslatef(pos[0], pos[1], pos[2]);
	//glTranslatef(0.05, 0.1, -2);
	//glRotatef(30, 1, -1, -0.2);
	//glColor4f(1.0, 0.2, 0.2, 0.5);
	////gdk_gl_draw_teapot(TRUE, 0.5);
	//gdk_gl_draw_cube(TRUE, 0.5);

	//glLoadIdentity();
	//glTranslatef(pos[0], pos[1], pos[2]);
	//glTranslatef(-0.2, 0, -2);
	//glRotatef(30, 1, -1, -0.2);
	//glColor4f(0.2, 0.2, 1.0, 0.5);
	////gdk_gl_draw_teapot(TRUE, 0.5);
	//gdk_gl_draw_cube(TRUE, 0.5);
	sort_end(data);

	/* Flush */
	GdkGLDrawable *gldrawable = gdk_gl_drawable_get_current();
	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();
	return FALSE;
}

static gboolean on_configure(GtkWidget *drawing, GdkEventConfigure *event, gpointer _)
{
	gdouble width  = drawing->allocation.width;
	gdouble height = drawing->allocation.height;
	glViewport(0, 0, width, height);

	/* Set up projection */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(90, width/height, 0.1, 10);
	gluPerspective(90, 0.8, 0.1, 10);
	//glOrtho(1,-1, -0.7,0.7, -10,10);
	return FALSE;
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer _)
{
	guint kv = event->keyval;
	if      (kv == GDK_q) gtk_main_quit();
	else if (kv == GDK_h) pos[0] -= 0.02;
	else if (kv == GDK_j) pos[1] += 0.02;
	else if (kv == GDK_k) pos[1] -= 0.02;
	else if (kv == GDK_l) pos[0] += 0.02;
	else if (kv == GDK_o) pos[2] -= 0.02;
	else if (kv == GDK_i) pos[2] += 0.02;
	else if (kv == GDK_H) rot[2] -= 2.0;
	else if (kv == GDK_J) rot[0] += 2.0;
	else if (kv == GDK_K) rot[0] -= 2.0;
	else if (kv == GDK_L) rot[2] += 2.0;
	gtk_widget_queue_draw(widget);
	return FALSE;
}

static guint load_tex(gchar *filename)
{
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	guchar    *pixels = gdk_pixbuf_get_pixels(pixbuf);
	int        width  = gdk_pixbuf_get_width(pixbuf);
	int        height = gdk_pixbuf_get_height(pixbuf);
	int        alpha  = gdk_pixbuf_get_has_alpha(pixbuf);
	guint      tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
			(alpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, pixels);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	g_object_unref(pixbuf);
	return tex;
}

gchar *gl_program_log(guint program, int *_len)
{
	gchar *buf = NULL;
	int len = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		buf = g_malloc(len);
		glGetProgramInfoLog(program, len, &len, buf);
	}
	if (_len)
		*_len = len;
	return buf;
}

static void load_shader(gchar *filename)
{
	gchar *source;
	gboolean status = g_file_get_contents(filename, &source, NULL, NULL);
	if (!status)
		g_error("Failed to load shader");

	guint program = glCreateProgram();
	if (!program)
		g_error("Error creating program");

	guint shader = glCreateShader(GL_VERTEX_SHADER_ARB);
	if (!shader)
		g_error("Error creating shader");

	glShaderSource(shader, 1, (const gchar**)&source, NULL);
	if (glGetError())
		g_error("Error setting shader source");

	glCompileShader(shader);
	if (glGetError())
		g_error("Error compiling shader");

	glAttachShader(program, shader);
	if (glGetError())
		g_error("Error attaching shader");

	glLinkProgram(program);
	if (glGetError())
		g_error("Error linking program");

	glUseProgram(program);
	if (glGetError())
		g_error("Error using program:\n%s", gl_program_log(program, NULL));
}

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);

	GtkWidget   *window   = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget   *drawing  = gtk_drawing_area_new();
	GdkGLConfig *glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)(
			GDK_GL_MODE_RGBA   | GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_DOUBLE | GDK_GL_MODE_ALPHA));
	g_signal_connect(window,  "destroy",         G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window,  "key-press-event", G_CALLBACK(on_key_press),  NULL);
	g_signal_connect(drawing, "expose-event",    G_CALLBACK(on_expose),     NULL);
	g_signal_connect(drawing, "configure-event", G_CALLBACK(on_configure),  NULL);
	gtk_widget_set_gl_capability(drawing, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
	gtk_container_add(GTK_CONTAINER(window), drawing);
	gtk_widget_show_all(window);

	/* OpenGL setup */
	GdkGLContext  *glcontext  = gtk_widget_get_gl_context(GTK_WIDGET(drawing));
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(drawing));
	gdk_gl_drawable_gl_begin(gldrawable, glcontext);

	/* Load texture */
	tex = load_tex("flag.png");

	/* Load shader */
	load_shader("sort.glsl");

	gtk_main();

	gdk_gl_drawable_gl_end(gldrawable);
}
