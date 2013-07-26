
/*
xbutton - add x button via ld_preload to kill application
Written in 2013 by <Ahmet Inan> <ainan@mathematik.uni-freiburg.de>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

static int bx;
static int by;
static int r2 = 1024;

static void button_press(Window window, int x, int y)
{
	fprintf(stderr, "button press %d %d %d\n", (unsigned)window, x, y);
	if ((x-bx)*(x-bx)+(y-by)*(y-by) < r2) {
		fprintf(stderr, "killing app\n");
		exit(0);
	}
}

static void resize_window(Window window, int width, int height)
{
	static Window first_window;
	if (first_window && first_window != window) {
		fprintf(stderr, "ignoring new window resize %d %d %d\n", (unsigned)window, width, height);
		return;
	}
	first_window = window;
	bx = width;
//	by = height;
	fprintf(stderr, "resize window %d %d %d\n", (unsigned)window, width, height);
}

int XNextEvent(Display *d, XEvent *e)
{
	static int (*real_XNextEvent)(Display *, XEvent *);
	if (!real_XNextEvent)
		real_XNextEvent = dlsym(RTLD_NEXT, "XNextEvent");
	int r = real_XNextEvent(d, e);
	if (!e)
		return r;
	if (ConfigureNotify == e->type)
		resize_window(e->xbutton.window, e->xconfigure.width, e->xconfigure.height);
	if (ButtonPress == e->type)
		button_press(e->xbutton.window, e->xbutton.x, e->xbutton.y);
	return r;
}

