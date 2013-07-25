
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
		fprintf(stderr, "Xlib: button press %d %d\n", ev->event_x, ev->event_y);
		xbutton(ev->event_x, ev->event_y);
	}
	return e;
}

xcb_void_cookie_t xcb_create_window(xcb_connection_t *c, uint8_t depth, xcb_window_t wid, xcb_window_t parent, int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t border_width, uint16_t _class, xcb_visualid_t visual, uint32_t value_mask, const uint32_t *value_list)
{
	static xcb_void_cookie_t (*real_xcb_create_window)(xcb_connection_t *, uint8_t, xcb_window_t, xcb_window_t, int16_t, int16_t, uint16_t, uint16_t, uint16_t, uint16_t, xcb_visualid_t, uint32_t, const uint32_t *);
	if (!real_xcb_create_window)
		real_xcb_create_window = dlsym(RTLD_NEXT, "xcb_create_window");
	fprintf(stderr, "XCB: create window %d %d\n", width, height);
	return real_xcb_create_window(c, depth, wid, parent, x, y, width, height, border_width, _class, visual, value_mask, value_list);
}

Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int class, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes)
{
	static Window (*real_XCreateWindow)(Display *, Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual *, unsigned long, XSetWindowAttributes *);
	if (!real_XCreateWindow)
		real_XCreateWindow = dlsym(RTLD_NEXT, "XCreateWindow");
	fprintf(stderr, "Xlib: create window %d %d\n", width, height);
	return real_XCreateWindow(display, parent, x, y, width, height, border_width, depth, class, visual, valuemask, attributes);
}

