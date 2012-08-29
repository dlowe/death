#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    Display *display = XOpenDisplay(NULL);
    int screen       = DefaultScreen(display);
    Window window    = XCreateSimpleWindow(display, RootWindow(display, screen),
        40, 40, 1024, 512, 3, BlackPixel(display, screen), WhitePixel(display, screen));

    enum { splash, quit, playing, dead } state = splash;
    short up, down;

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
    XMapWindow(display, window);

    while (state != quit) {
        usleep(16666);

        /* X11 events */
        if (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            switch (state) {
                case splash:
                    switch (event.type) {
                        case KeyPress:
                            switch ((long)XLookupKeysym(&event.xkey, 0)) {
                                case XK_q:
                                    state = quit;
                                    break;
                                default:
                                    up = down = 0;
                                    state = playing;
                                    break;
                            };
                            break;
                    };
                    break;
                case playing:
                    switch (event.type) {
                        case KeyPress:
                            switch ((long)XLookupKeysym(&event.xkey, 0)) {
                                case XK_Up:
                                    up = 1;
                                    break;
                                case XK_Down:
                                    down = 1;
                                    break;
                                case XK_q:
                                    state = dead;
                                    break;
                            };
                            break;
                        case KeyRelease:
                            switch ((long)XLookupKeysym(&event.xkey, 0)) {
                                case XK_Up:
                                    up = 0;
                                    break;
                                case XK_Down:
                                    down = 0;
                                    break;
                            };
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
        XClearWindow(display, window);
        switch (state) {
            case splash: {
                char *splash = "conway's game of DEATH (controls: q, up-arrow, down-arrow)";
                XDrawString(display, window, DefaultGC(display, screen), 10, 50, splash, strlen(splash));
                break;
            }
            case playing:
                if (up) {
                    char *upmsg = "up";
                    XDrawString(display, window, DefaultGC(display, screen), 10, 50, upmsg, strlen(upmsg));
                }
                if (down) {
                    char *downmsg = "down";
                    XDrawString(display, window, DefaultGC(display, screen), 10, 80, downmsg, strlen(downmsg));
                }
                break;
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

    XCloseDisplay(display);
    return 0;
}
