#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>

typedef struct {
	gfloat xyz[3];
	gfloat color[4];
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
	glFeedbackBuffer(size, GL_3D_COLOR, data);
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
	glLoadIdentity();
	glOrtho(view[0],view[2], view[1],view[3], -10,10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);

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
				//g_print("\t%f, %f, %f\n",
				//	verts[j].xyz[0],
				//	verts[j].xyz[1],
				//	verts[j].xyz[2]);
				glColor4fv(verts[j].color);
				glVertex3fv(verts[j].xyz);
			}
			glEnd();

			/* Draw line */
			glColor4f(1,1,1,1);
			glBegin(GL_LINE_LOOP);
			for (int j = 0; j < n; j++)
				glVertex3fv(verts[j].xyz);
			glEnd();
		} else {
			g_error("Unknown token: %f\n", token);
		}
	}
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

	/* Set up projection */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(1,-1, -0.7,0.7, -10,10);

	/* Draw teapots */
	glMatrixMode(GL_MODELVIEW);

	gfloat *data = sort_start();
	glLoadIdentity();
	glTranslatef(0.05, 0.05, -2);
	glRotatef(30, 1, -1, -0.2);
	glColor4f(1.0, 0.2, 0.2, 0.6);
	gdk_gl_draw_teapot(TRUE, 0.5);

	glLoadIdentity();
	glTranslatef(-0.2, -0.05, -2);
	glRotatef(30, 1, -1, -0.2);
	glColor4f(0.2, 0.2, 1.0, 0.6);
	gdk_gl_draw_teapot(TRUE, 0.5);
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
	glViewport(0, 0,
		drawing->allocation.width,
		drawing->allocation.height);
	return FALSE;
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer _)
{
	if (event->keyval == GDK_q)
		gtk_main_quit();
	return FALSE;
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

	gtk_main();

	gdk_gl_drawable_gl_end(gldrawable);
}
