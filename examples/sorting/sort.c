#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glu.h>

guint tex, texl, texr;

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer _)
{
	if (event->keyval == GDK_q)
		gtk_main_quit();
	return FALSE;
}

gboolean on_expose(GtkWidget *drawing, GdkEventExpose *event, gpointer _)
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
	glOrtho(1,-1, -1,1, -10,10);

	/* Draw teapots */
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glTranslatef(-0.2, -0.2, -2);
	glRotatef(30, 1, -1, -0.2);
	glColor4f(0.6, 0.6, 0.4, 0.5);
	gdk_gl_draw_teapot(TRUE, 0.5);

	glLoadIdentity();
	glTranslatef(0.2, 0.2, -2);
	glRotatef(30, 1, -1, -0.2);
	glColor4f(0.6, 0.6, 0.4, 0.5);
	gdk_gl_draw_teapot(TRUE, 0.5);

	/* Flush */
	GdkGLDrawable *gldrawable = gdk_gl_drawable_get_current();
	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush();
	return FALSE;
}

gboolean on_configure(GtkWidget *drawing, GdkEventConfigure *event, gpointer _)
{
	glViewport(0, 0,
		drawing->allocation.width,
		drawing->allocation.height);
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
