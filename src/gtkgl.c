#include <gtk/gtk.h>

/***************************
 * GtkGlExt implementation *
 ***************************/
#if defined(USE_GTKGLEXT)
#include <gtk/gtkgl.h>
void gtk_gl_enable(GtkWidget *widget)
{
	GdkGLConfig *glconfig = gdk_gl_config_new_by_mode(
			GDK_GL_MODE_RGBA   | GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_ALPHA  | GDK_GL_MODE_DOUBLE);
	gtk_widget_set_gl_capability(widget,
			glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
}

void gtk_gl_begin(GtkWidget *widget)
{
	GdkGLContext  *glcontext  = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	gdk_gl_drawable_gl_begin(gldrawable, glcontext);
}

void gtk_gl_end(GtkWidget *widget)
{
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	gdk_gl_drawable_swap_buffers(gldrawable);
	gdk_gl_drawable_gl_end(gldrawable);
}

void gtk_gl_disable(GtkWidget *widget)
{
}


/**********************
 * X11 implementation *
 **********************/
#elif defined(USE_GLX)
#include <GL/glx.h>
#include <gdk/gdkx.h>
void gtk_gl_enable(GtkWidget *widget)
{
	g_debug("GtkGl: enable");
	GdkScreen *screen   = gdk_screen_get_default();
	Display   *xdisplay = GDK_SCREEN_XDISPLAY(screen);
	gint       nscreen  = GDK_SCREEN_XNUMBER(screen);

	/* Create context */
	int attribs[] = {GLX_RGBA,
	                 GLX_RED_SIZE,    1,
	                 GLX_GREEN_SIZE,  1,
	                 GLX_BLUE_SIZE,   1,
	                 GLX_ALPHA_SIZE,  1,
	                 GLX_DOUBLEBUFFER,
	                 GLX_DEPTH_SIZE,  1,
	                 None};
	XVisualInfo *xvinfo  = glXChooseVisual(xdisplay, nscreen, attribs);
	if (!xvinfo)
		g_error("GtkGl: enable - unable to get valid OpenGL Visual");
	GLXContext   context = glXCreateContext(xdisplay, xvinfo, NULL, False);
	g_object_set_data(G_OBJECT(widget), "glcontext", context);

	/* Fix up colormap */
	GdkVisual   *visual = gdk_x11_screen_lookup_visual(screen, xvinfo->visualid);
	GdkColormap *cmap   = gdk_colormap_new(visual, FALSE);
	gtk_widget_set_colormap(widget, cmap);

	/* Disable GTK double buffering */
	gtk_widget_set_double_buffered(widget, FALSE);
}

void gtk_gl_begin(GtkWidget *widget)
{
	g_debug("GtkGl: begin");
	Display   *xdisplay = GDK_SCREEN_XDISPLAY(gtk_widget_get_screen(widget));
	Window     xwindow  = GDK_WINDOW_XID(gtk_widget_get_window(widget));
	GLXContext context  = g_object_get_data(G_OBJECT(widget), "glcontext");
	glXMakeCurrent(xdisplay, xwindow, context);
}

void gtk_gl_end(GtkWidget *widget)
{
	g_debug("GtkGl: end");
	Display   *xdisplay = GDK_SCREEN_XDISPLAY(gtk_widget_get_screen(widget));
	Window     xwindow  = GDK_WINDOW_XID(gtk_widget_get_window(widget));
	glXSwapBuffers(xdisplay, xwindow);
}

void gtk_gl_disable(GtkWidget *widget)
{
	g_debug("GtkGl: disable");
	Display   *xdisplay = GDK_SCREEN_XDISPLAY(gtk_widget_get_screen(widget));
	GLXContext context  = g_object_get_data(G_OBJECT(widget), "glcontext");
	glXDestroyContext(xdisplay, context);
}


/************************
 * Win32 implementation *
 ************************/
#elif defined(USE_WGL)
#include <windows.h>
#include <gdk/gdkwin32.h>
static void on_realize(GtkWidget *widget, gpointer _)
{
	g_debug("GtkGl: on_realize");
	gdk_window_ensure_native(gtk_widget_get_window(widget));
	gtk_widget_set_double_buffered(widget, FALSE);

	HWND  hwnd = GDK_WINDOW_HWND(gtk_widget_get_window(widget));
	HDC   hDC  = GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd = {
		.nSize       = sizeof(pfd),
		.nVersion    = 1,
		.dwFlags     = PFD_DRAW_TO_WINDOW
		             | PFD_SUPPORT_OPENGL
		             | PFD_DOUBLEBUFFER,
		//.dwFlags     = PFD_SUPPORT_OPENGL
		//             | PFD_DRAW_TO_WINDOW,
		.iPixelType  = PFD_TYPE_RGBA,
		.cColorBits  = 24,
		.cAlphaBits  = 8,
		.cDepthBits  = 32,
		.iLayerType  = PFD_MAIN_PLANE,
	};
	int pf = ChoosePixelFormat(hDC, &pfd);
	if (pf == 0)
		g_error("GtkGl: ChoosePixelFormat failed");
	if (!SetPixelFormat(hDC, pf, &pfd))
		g_error("GtkGl: SetPixelFormat failed");
	HGLRC hRC = wglCreateContext(hDC);
	if (hRC == NULL)
		g_error("GtkGl: wglCreateContext failed");
	g_object_set_data(G_OBJECT(widget), "glcontext", hRC);
}

void gtk_gl_enable(GtkWidget *widget)
{
	g_debug("GtkGl: enable");
	g_signal_connect(widget, "realize", G_CALLBACK(on_realize), NULL);
}

void gtk_gl_begin(GtkWidget *widget)
{
	g_debug("GtkGl: begin");
	HWND  hwnd = GDK_WINDOW_HWND(gtk_widget_get_window(widget));
	HDC   hDC  = GetDC(hwnd);
	HGLRC hRC  = g_object_get_data(G_OBJECT(widget), "glcontext");
	if (!wglMakeCurrent(hDC, hRC))
		g_error("GtkGl: wglMakeCurrent failed");
}

void gtk_gl_end(GtkWidget *widget)
{
	g_debug("GtkGl: end");
	HWND  hwnd = GDK_WINDOW_HWND(gtk_widget_get_window(widget));
	HDC   hDC  = GetDC(hwnd);
	if (!SwapBuffers(hDC))
		g_error("GtkGl: SwapBuffers failed");
}

void gtk_gl_disable(GtkWidget *widget)
{
	g_debug("GtkGl: disable");
	HGLRC hRC = g_object_get_data(G_OBJECT(widget), "glcontext");
	wglDeleteContext(hRC);
}



/**************************
 * Mac OSX implementation *
 **************************/
#elif defined(USE_CGL)
void gtk_gl_enable(GtkWidget *widget)
{
	CGDisplayCapture( kCGDirectMainDisplay );
	CGLPixelFormatAttribute attribs[] =
	{
		kCGLPFANoRecovery,
		kCGLPFADoubleBuffer,
		kCGLPFAFullScreen,
		kCGLPFAStencilSize, ( CGLPixelFormatAttribute ) 8,
		kCGLPFADisplayMask, ( CGLPixelFormatAttribute ) CGDisplayIDToOpenGLDisplayMask( kCGDirectMainDisplay ),
		( CGLPixelFormatAttribute ) NULL
	};

	CGLPixelFormatObj pixelFormatObj;
	GLint numPixelFormats;
	CGLChoosePixelFormat( attribs, &pixelFormatObj, &numPixelFormats );

	CGLCreateContext( pixelFormatObj, NULL, &contextObj );

	CGLDestroyPixelFormat( pixelFormatObj );

	CGLSetCurrentContext( contextObj );
	CGLSetFullScreen( contextObj );
}

void gtk_gl_begin(GtkWidget *widget)
{
}

void gtk_gl_end(GtkWidget *widget)
{
}

void gtk_gl_disable(GtkWidget *widget)
{
}


/****************************
 * Undefined implementation *
 ****************************/
#else
#warning "Unimplemented GtkGl"
void gtk_gl_enable(GtkWidget *widget) { }
void gtk_gl_begin(GtkWidget *widget) { }
void gtk_gl_end(GtkWidget *widget) { }
void gtk_gl_disable(GtkWidget *widget) { }
#endif
