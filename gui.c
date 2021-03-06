/*
 * pngview - a standalone PNG viewer
 * Copyright 2013 Vegard Edvardsen
 */

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "png.h"

void gui_display_image(png_t *png)
{
    Display *display = XOpenDisplay(NULL);
    int screen = DefaultScreen(display);
    int black = BlackPixel(display, screen);
    Window window = XCreateSimpleWindow(
        display,
        DefaultRootWindow(display),
        0, 0,
        png->width, png->height,
        0,
        black, black);

    char title[500];
    snprintf(title, 500, "%s - %dx%d - pngview",
        png->name, png->width, png->height);
    XStoreName(display, window, title);

    XSizeHints *hints = XAllocSizeHints();
    hints->flags = PMinSize | PMaxSize;
    hints->min_width = hints->max_width = png->width;
    hints->min_height = hints->max_height = png->height;
    XSetWMNormalHints(display, window, hints);
    XFree(hints);

    XMapWindow(display, window);

    GC gc = DefaultGC(display, screen);
    Visual *visual = DefaultVisual(display, screen);

    char *image_data = png_get_data(png);
    XImage *image = XCreateImage(display, visual, 24, ZPixmap, 0,
        image_data, png->width, png->height, 32, 0);

    XSelectInput(display, window, ExposureMask);

    while (1) {
        XEvent event;
        XNextEvent(display, &event);
        
        switch(event.type) {
            case Expose:
                if (event.xexpose.count > 0) {
                    break;
                }

                XPutImage(display, window, gc, image, 0, 0, 0, 0,
                    png->width, png->height);
                break;
        }
    }

    XCloseDisplay(display);
}
