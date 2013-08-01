
/*
xbutton - add x button via ld_preload to kill application
Written in 2013 by <Ahmet Inan> <ainan@mathematik.uni-freiburg.de>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>

static int bx;
static int by;
static int radius = 32;
static int debug;

static void msg(char *fmt, ...)
{
	if (!debug)
		return;
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

static void draw_xbutton(Display *display, Drawable d, GC gc)
{
	int x = bx < radius*2 ? radius : bx - radius;
	int y = by < radius*2 ? radius : by - radius;
	XDrawString(display, d, gc, x, y, "[x]", 3);
}

static void button_press(Window window, int x, int y)
{
	msg("button press %d %d %d\n", (unsigned)window, x, y);
	if ((x-bx)*(x-bx)+(y-by)*(y-by) < radius*radius) {
		msg("killing app\n");
		exit(0);
	}
}

static void resize_window(Window window, int width, int height)
{
	msg("resize window %d %d %d\n", (unsigned)window, width, height);
	static int corner = 1, init, resize_first, resize_max;
	if (!init) {
		init = 1;
		char *str;
		if ((str = getenv("XBUTTON_CORNER"))) {
			if (!strcmp(str, "NW"))
				corner = 0;
			else if (!strcmp(str, "NE"))
				corner = 1;
			else if (!strcmp(str, "SW"))
				corner = 2;
			else if (!strcmp(str, "SE"))
				corner = 3;
			else
				msg("ignoring ");
			msg("XBUTTON_CORNER=%s\n", str);
		}
		if ((str = getenv("XBUTTON_RESIZE"))) {
			if (!strcmp(str, "first"))
				resize_first = 1;
			else if (!strcmp(str, "max"))
				resize_max = 1;
			else
				msg("ignoring ");
			msg("XBUTTON_RESIZE=%s\n", str);
		}
		if ((str = getenv("XBUTTON_R"))) {
			int tmp = atoi(str);
			if (tmp > 0) {
				radius = tmp;
				msg("XBUTTON_R=%d\n", radius);
			} else {
				msg("ignoring XBUTTON_R=%s\n", str);
			}
		}
		if ((str = getenv("XBUTTON_DEBUG"))) {
			int tmp = atoi(str);
			if (1 == tmp) {
				debug = 1;
				msg("XBUTTON_DEBUG=1\n");
			} else {
				msg("ignoring XBUTTON_R=%s\n", str);
			}
		}
	}
	if (resize_first) {
		static Window first_window;
		if (first_window && first_window != window) {
			msg("ignoring new window resize %d %d %d\n", (unsigned)window, width, height);
			return;
		}
		first_window = window;
	} else if (resize_max) {
		static int last_width, last_height;
		last_width = width = last_width > width ? last_width : width;
		last_height = height = last_height > height ? last_height : height;
	}
	bx = corner & 1 ? width - 1 : 0;
	by = corner & 2 ? height - 1 : 0;
	msg("new xbutton position %d %d\n", bx, by);
}

int XNextEvent(Display *d, XEvent *e)
{
	static int (*real_XNextEvent)(Display *, XEvent *);
	if (!real_XNextEvent)
		real_XNextEvent = dlsym(RTLD_NEXT, "XNextEvent");
	int r = real_XNextEvent(d, e);
	if (!e)
		return r;
	switch (e->type) {
		case ButtonPress:
			button_press(e->xbutton.window, e->xbutton.x, e->xbutton.y);
			break;
		case ConfigureNotify:
			resize_window(e->xconfigure.window, e->xconfigure.width, e->xconfigure.height);
			break;
		case CreateNotify:
			resize_window(e->xcreatewindow.window, e->xcreatewindow.width, e->xcreatewindow.height);
			break;
		case ConfigureRequest:
			resize_window(e->xconfigure.window, e->xconfigure.width, e->xconfigure.height);
			break;
		case ResizeRequest:
			resize_window(e->xresizerequest.window, e->xresizerequest.width, e->xresizerequest.height);
			break;
	}
	return r;
}

int XResizeWindow(Display *display, Window w, unsigned int width, unsigned int height)
{
	static int (*real_XResizeWindow)(Display *, Window, unsigned int, unsigned int);
	if (!real_XResizeWindow)
		real_XResizeWindow = dlsym(RTLD_NEXT, "XResizeWindow");
	resize_window(w, width, height);
	return real_XResizeWindow(display, w, width, height);
}

int XDrawLines(Display *display, Drawable d, GC gc, XPoint *points, int npoints, int mode)
{
	static int (*real_XDrawLines)(Display *, Drawable, GC, XPoint *, int, int);
	if (!real_XDrawLines)
		real_XDrawLines = dlsym(RTLD_NEXT, "XDrawLines");
	int r = real_XDrawLines(display, d, gc, points, npoints, mode);
	msg("XDrawLines\n");
	draw_xbutton(display, d, gc);
	return r;
}

int XPutImage(Display *display, Drawable d, GC gc, XImage *image, int src_x, int src_y, int dest_x, int dest_y, unsigned int width, unsigned int height)
{
	static int (*real_XPutImage)(Display *, Drawable, GC, XImage *, int, int, int, int, unsigned int, unsigned int);
	if (!real_XPutImage)
		real_XPutImage = dlsym(RTLD_NEXT, "XPutImage");
	int r = real_XPutImage(display, d, gc, image, src_x, src_y, dest_x, dest_y, width, height);
	msg("XPutImage\n");
	draw_xbutton(display, d, gc);
	return r;
}

int XCopyArea(Display *display, Drawable src, Drawable dest, GC gc, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y)
{
	static int (*real_XCopyArea)(Display *, Drawable, Drawable, GC, int, int, unsigned int, unsigned int, int, int);
	if (!real_XCopyArea)
		real_XCopyArea = dlsym(RTLD_NEXT, "XCopyArea");
	int r = real_XCopyArea(display, src, dest, gc, src_x, src_y, width, height, dest_x, dest_y);
	msg("XCopyArea\n");
	draw_xbutton(display, dest, gc);
	return r;
}

int XCopyPlane(Display *display, Drawable src, Drawable dest, GC gc, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y, unsigned long plane)
{
	static int (*real_XCopyPlane)(Display *, Drawable, Drawable, GC, int, int, unsigned int, unsigned int, int, int, unsigned long);
	if (!real_XCopyPlane)
		real_XCopyPlane = dlsym(RTLD_NEXT, "XCopyPlane");
	int r = real_XCopyPlane(display, src, dest, gc, src_x, src_y, width, height, dest_x, dest_y, plane);
	msg("XCopyPlane\n");
	draw_xbutton(display, dest, gc);
	return r;
}

