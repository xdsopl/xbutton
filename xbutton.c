
/*
xbutton - add x button via ld_preload to kill application
Written in 2013 by <Ahmet Inan> <ainan@mathematik.uni-freiburg.de>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <X11/Xlib.h>

static void xbutton(int x, int y)
{
	if (x*x+y*y < 4096)
		exit(0);
}

xcb_generic_event_t *xcb_poll_for_event(xcb_connection_t *c)
{
	static xcb_generic_event_t *(*real_xcb_poll_for_event)(xcb_connection_t *);
	if (!real_xcb_poll_for_event)
		real_xcb_poll_for_event = dlsym(RTLD_NEXT, "xcb_poll_for_event");
	xcb_generic_event_t *e = real_xcb_poll_for_event(c);
	if (e && XCB_BUTTON_PRESS == e->response_type) {
		xcb_button_press_event_t *ev = (xcb_button_press_event_t *)e;
		xbutton(ev->event_x, ev->event_y);
	}
	return e;
}

int XNextEvent(Display *d, XEvent *e)
{
	static int (*real_XNextEvent)(Display *, XEvent *);
	if (!real_XNextEvent)
		real_XNextEvent = dlsym(RTLD_NEXT, "XNextEvent");
	int r = real_XNextEvent(d, e);
	if (e && ButtonPress == e->type)
		xbutton(e->xbutton.x, e->xbutton.y);
	return r;
}

