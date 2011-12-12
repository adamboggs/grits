#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

//#define SYS_CAIRO
//#define SYS_GTKGLEXT
//#define SYS_X11
//#define SYS_WIN
//#define SYS_MAC

/************************
 * Cairo implementation *
 ************************/
#if defined(SYS_CAIRO)
gpointer setup(GtkWidget *widget) { return NULL; }
gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);
	cairo_t *cairo = gdk_cairo_create(gtk_widget_get_window(widget));
	cairo_set_source_rgb(cairo, 1, 1, 1);
	cairo_arc(cairo,
		alloc.width/2, alloc.height/2,
		MIN(alloc.width/2,alloc.height/2),
		0, 2*G_PI);
	cairo_fill(cairo);
	return FALSE;
}


/***************************
 * GtkGlExt implementation *
 ***************************/
#elif defined(SYS_GTKGLEXT)
#include <gtk/gtkgl.h>
#include <GL/gl.h>
void realize(GtkWidget *widget, gpointer user_data)
{
	gdk_window_ensure_native(gtk_widget_get_window(widget));
}
gpointer setup(GtkWidget *widget)
{
	//GdkGLConfig *glconfig = gdk_gl_config_new_by_mode(
	//		GDK_GL_MODE_RGBA   | GDK_GL_MODE_DEPTH |
	//		GDK_GL_MODE_ALPHA);
	GdkGLConfig *glconfig = gdk_gl_config_new_by_mode(
			GDK_GL_MODE_RGBA   | GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_ALPHA  | GDK_GL_MODE_DOUBLE);
	gtk_widget_set_gl_capability(widget,
			glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
	return NULL;
}
gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);
	GdkGLContext  *glcontext  = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	gdk_gl_drawable_gl_begin(gldrawable, glcontext);
	glViewport(0, 0, alloc.width, alloc.height);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.0, 0.0);
	for (double i = 0; i < 2*G_PI; i+=(2*G_PI)/100)
		glVertex2d(sin(i), cos(i));
	glEnd();
	gdk_gl_drawable_swap_buffers(gldrawable);
	gdk_gl_drawable_gl_end(gldrawable);
	return FALSE;
}


/**********************
 * X11 implementation *
 **********************/
#elif defined(SYS_X11)
#include <GL/gl.h>
#include <GL/glx.h>
#include <gdk/gdkx.h>
void realize(GtkWidget *widget, gpointer user_data)
{
      gdk_window_ensure_native(gtk_widget_get_window(widget));
}
gpointer setup(GtkWidget *widget)
{
	GdkScreen *screen    = gdk_screen_get_default();
	Display   *xdisplay  = GDK_SCREEN_XDISPLAY(screen);
	gint       nscreen   = GDK_SCREEN_XNUMBER(screen);

	/* Create context */
	int attribs[]        = {GLX_RGBA,
	                        GLX_RED_SIZE,    1,
	                        GLX_GREEN_SIZE,  1,
	                        GLX_BLUE_SIZE,   1,
	                        GLX_ALPHA_SIZE,  1,
	                        GLX_DOUBLEBUFFER,
	                        GLX_DEPTH_SIZE,  1,
	                        None};
	XVisualInfo *xvinfo  = glXChooseVisual(xdisplay, nscreen, attribs);
	GLXContext   context = glXCreateContext(xdisplay, xvinfo, 0, False);

	/* Fix up colormap */
	GdkVisual   *visual  = gdk_x11_screen_lookup_visual(screen, xvinfo->visualid);
	GdkColormap *cmap    = gdk_colormap_new(visual, FALSE);
	gtk_widget_set_colormap(widget, cmap);

	/* Disable GTK double buffering */
	gtk_widget_set_double_buffered(widget, FALSE);

	return context;
}
gboolean expose(GtkWidget *widget, GdkEventExpose *event, GLXContext context)
{
	/* Make current */
	Display     *xdisplay = GDK_SCREEN_XDISPLAY(gdk_screen_get_default());
	Window       xwindow  = GDK_WINDOW_XID(gtk_widget_get_window(widget));
	glXMakeCurrent(xdisplay, xwindow, context);

	GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
	g_message("window: w=%x tl=%x",
		(guint)GDK_WINDOW_XID(gtk_widget_get_window(widget)),
		(guint)GDK_WINDOW_XID(gtk_widget_get_window(toplevel)));

	/* Drawing */
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);
	glViewport(0, 0, alloc.width, alloc.height);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.0, 0.0);
	for (double i = 0; i < 2*G_PI; i+=(2*G_PI)/100)
		glVertex2d(sin(i), cos(i));
	glEnd();

	/* Swap buffers */
	glXSwapBuffers(xdisplay, xwindow);
	return FALSE;
}


/************************
 * Win32 implementation *
 ************************/
#elif defined(SYS_WIN)
#include <GL/gl.h>
#include <windows.h>
#include <gdk/gdkwin32.h>
void realize(GtkWidget *widget, gpointer user_data)
{
	gdk_window_ensure_native(gtk_widget_get_window(widget));
}
gpointer setup(GtkWidget *widget)
{
	/* Create context */
	//HWND  hwnd = GDK_WINDOW_HWND(gtk_widget_get_window(widget));
	//HGLRC hDC  = GetDC(hwnd);           // get the device context for window
	//HDC   hRC  = wglCreateContext(hDC); // create rendering context
	//wglMakeCurrent(hDC,hRC);            // make rendering context current

	/* Delete context */
	//wglMakeCurrent(hDC,NULL);    // deselect rendering context
	//wglDeleteContext(hRC);       // delete rendering context
	//PostQuitMessage(0);          // send wm_quit

	/* Disable GTK double buffering */
	gtk_widget_set_double_buffered(widget, FALSE);
	return FALSE;
}
gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
	GdkWindow *window   = gtk_widget_get_window(widget);
	GdkWindow *topwin   = gtk_widget_get_window(toplevel);
	gdk_window_ensure_native(window);

	PIXELFORMATDESCRIPTOR pfo = {}, pfd = {
		.nSize       = sizeof(pfd),
		.nVersion    = 1,
		.dwFlags     = PFD_DRAW_TO_WINDOW // "Correct" way
		             | PFD_SUPPORT_OPENGL
		             | PFD_DOUBLEBUFFER,
		//.dwFlags     = PFD_SUPPORT_OPENGL  // Works in wine
		//             | PFD_DRAW_TO_WINDOW,
		.iPixelType  = PFD_TYPE_RGBA,
		.cColorBits  = 24,
		.cAlphaBits  = 8,
		.cDepthBits  = 32,
		.iLayerType  = PFD_MAIN_PLANE,
	};
	HWND  hwnd = GDK_WINDOW_HWND(window);
	HDC   hDC  = GetDC(hwnd);           // get the device context for window
	int   pf   = ChoosePixelFormat(hDC, &pfd);
	int   st0  = DescribePixelFormat(hDC, pf, sizeof(pfd), &pfo);
	int   st1  = SetPixelFormat(hDC, pf, &pfd);
	HGLRC hRC  = wglCreateContext(hDC);
	int   st2  = wglMakeCurrent(hDC, hRC);

	g_message("dc: %p, %p, %p", hDC, GetDC(hwnd), wglGetCurrentDC());

	g_message("window: pf=%d st=%d,%d,%d dc=%p rc=%p wins=%x=%x!=%x",
		pf, st0,st1,st2, hDC, hRC, (guint)hwnd,
		(guint)GDK_WINDOW_HWND(window),
		(guint)GDK_WINDOW_HWND(topwin));

	g_message("pdfOut: dwFlags=%lx=%lx, ipt=%x=%x, layer=%x=%x, {c,a,d}bits=%d,%d,%d",
		pfo.dwFlags,    pfd.dwFlags,
		pfo.iPixelType, pfd.iPixelType,
		pfo.iLayerType, pfd.iLayerType,
		pfo.cColorBits, pfo.cAlphaBits, pfo.cDepthBits);

	/* Drawing */
	GtkAllocation alloc = widget->allocation;
	glViewport(0, 0, alloc.width, alloc.height);
	g_message("alloc: %dx%d", alloc.width, alloc.height);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.0, 0.0);
	for (double i = 0; i < 2*G_PI; i+=(2*G_PI)/100)
		glVertex2d(sin(i), cos(i));
	glEnd();

	/* Swap buffers */
	SwapBuffers(hDC);

	/* Cleanup */
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	return TRUE;
}


/**************************
 * Mac OSX implementation *
 **************************/
#elif defined(SYS_MAC)
#include <gdk/gdkquartz.h>
gpointer setup(GtkWidget *widget)
{
	/* Create context */
	NSOpenGLPixelFormatAttribute attribs[] = {
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADepthSize, 1,
		NSOpenGLPFADoubleBuffer,
		0
	};
	NSOpenGLPixelFormat *pix  = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	NSOpenGLContext     *ctx  = [[NSOpenGLContext     alloc] initWithFormat:pix shareContext:nil];

	/* Disable GTK double buffering */
	gtk_widget_set_double_buffered(widget, FALSE);

	return ctx;
}
gboolean configure(GtkWidget *widget, GdkEventConfigure *event, NSOpenGLContext *ctx)
{
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	GdkWindow *win  = gtk_widget_get_window(widget);
	NSView    *view = gdk_quartz_window_get_nsview(win);
	NSRect     rect = NSMakeRect(alloc.x, alloc.y, alloc.width, alloc.height);

	[view setFrame:rect];
	[ctx  update];
	return FALSE;
}
gboolean expose(GtkWidget *widget, GdkEventExpose *event, NSOpenGLContext *ctx)
{
	gdk_window_ensure_native(gtk_widget_get_window(widget));

	GdkWindow *win  = gtk_widget_get_window(widget);
	NSView    *view = gdk_quartz_window_get_nsview(win);

	configure(widget, NULL, ctx);

	[ctx setView:view];
	[ctx makeCurrentContext];

	/* Drawing */
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);
	glViewport(0, 0, alloc.width, alloc.height);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.0, 0.0);
	for (double i = 0; i < 2*G_PI; i+=(2*G_PI)/100)
		glVertex2d(sin(i), cos(i));
	glEnd();

	[ctx flushBuffer];
	return FALSE;
}


/****************************
 * Undefined implementation *
 ****************************/
#else
gpointer setup(GtkWidget *widget) { return NULL; }
gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	g_message("unimplemented");
	return FALSE;
}
#endif



/***************
 * Common code *
 ***************/
gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if (event->keyval == GDK_q)
		gtk_main_quit();
	return FALSE;
}
int main(int argc, char **argv)
{
	gtk_init_check(&argc, &argv);
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *box    = gtk_vbox_new(FALSE, 5);
	GtkWidget *draw   = gtk_drawing_area_new();
	GtkWidget *label  = gtk_label_new("Hello, World");
	GtkWidget *button = gtk_button_new_with_label("Hello, World");
	gpointer   data   = setup(draw);
	g_signal_connect(window, "destroy",         G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press),     NULL);
	//g_signal_connect(draw,   "configure-event", G_CALLBACK(configure),     data);
	g_signal_connect(draw,   "expose-event",    G_CALLBACK(expose),        data);
	gtk_widget_set_size_request(draw,   300, 300);
	gtk_widget_set_size_request(button, -1,  50);
	gtk_box_pack_start(GTK_BOX(box), label,  FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), draw,   TRUE,  TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), box);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}
