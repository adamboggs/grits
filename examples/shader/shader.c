#define GL_GLEXT_PROTOTYPES
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glext.h>

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

void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	g_message("key-press");
	if (event->keyval == GDK_q)
		gtk_main_quit();
}

gboolean on_expose(GtkWidget *drawing, GdkEventExpose *event, gpointer _)
{
	glClearColor(0.5, 0.5, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,1, -1,1, 10,-10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -5);

	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glVertex3f(-0.25, -0.75, 0.0);
	glVertex3f(-0.25,  0.75, 0.0);
	glVertex3f( 0.25,  0.75, 0.0);
	glVertex3f( 0.25, -0.75, 0.0);
	glEnd();

	/* Test projection */
	gdouble model[16], proj[16];
	gint view[4];
	glGetDoublev (GL_MODELVIEW_MATRIX,  model);
	glGetDoublev (GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT,          view);
	gdouble x=0.25, y=-0.75, z=0.0;
	gdouble px, py, pz;
	gluProject(x, y, z, model, proj, view, &px, &py, &pz);
	g_message("%f,%f,%f -> %f,%f,%f", x, y, z, px, py, pz);

	/* Textures */
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

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

	/* Setup shader */
	gdk_gl_drawable_gl_begin(gldrawable, glcontext);

	gchar *source;
	gboolean status = g_file_get_contents("shader.glsl", &source, NULL, NULL);
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

	/* Run */
	gtk_main();

	/* Clean up */
	glDetachShader(program, shader);
	glDeleteShader(shader);
	glDeleteProgram(program);

	gdk_gl_drawable_gl_end(gldrawable);
	return 0;
}
