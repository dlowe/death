#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    Display *display = XOpenDisplay(NULL);
    int screen       = DefaultScreen(display);
    Window window    = XCreateSimpleWindow(display, RootWindow(display, screen),
        40, 40, 1024, 512, 3, BlackPixel(display, screen), WhitePixel(display, screen));

    enum { unmapped, splash, quit, playing, dead } state = unmapped;

    XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(display, window);

    while (state != quit) {
        struct timeval timeout;
        timeout.tv_sec  = 0;
        timeout.tv_usec = 1000;
        select(0, 0, 0, 0, &timeout);

        if (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            switch (state) {
                case unmapped:
                    if (event.type == MapNotify) {
                        state = splash;
                    }
                    break;
                case splash:
                    switch (event.type) {
                        case KeyPress:
                            XClearWindow(display, window);
                            XFlush(display);
                            switch ((long)XLookupKeysym(&event.xkey, 0)) {
                                case XK_q:
                                    state = quit;
                                    break;
                                default:
                                    state = playing;
                                    break;
                            };
                            break;
                    };
                    break;
                case playing:
                    switch (event.type) {
                        case KeyPress:
                            state = dead;
                            break;
                    };
                    break;
                case dead:
                    switch (event.type) {
                        case KeyPress:
                            state = splash;
                            break;
                    };
                    break;
                case quit:
                    break;
            };
        }

        /* repaint */
        if (state != unmapped) {
            XClearWindow(display, window);
            switch (state) {
                case splash: {
                    char *splash = "conway's game of DEATH (controls: q, up-arrow, down-arrow)";
                    XDrawString(display, window, DefaultGC(display, screen), 10, 50, splash, strlen(splash));
                    break;
                }
                case dead: {
                    char *death = "you died.";
                    XDrawString(display, window, DefaultGC(display, screen), 10, 50, death, strlen(death));
                    break;
                }
                default:
                    break;
            };
            XFlush(display);
        }
    }

    XCloseDisplay(display);
    return 0;
}
