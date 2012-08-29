#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef enum { splash, quit, playing_nil, playing_up, playing_down, dead } state;

state event_handler(state in, XEvent event) {
    switch (event.type) {
        case KeyPress:
            switch ((long)XLookupKeysym(&event.xkey, 0)) {
                case XK_q:
                    return quit;
                case XK_Up:
                    if (in != dead) {
                        return playing_up;
                    }
                case XK_Down:
                    if (in != dead) {
                        return playing_down;
                    }
                default:
                    if (in == splash) {
                        return playing_nil;
                    } else if (in == dead) {
                        return splash;
                    }
            };
        case KeyRelease:
            switch ((long)XLookupKeysym(&event.xkey, 0)) {
                case XK_Up:
                    if (in == playing_up) {
                        return playing_nil;
                    }
                    break;
                case XK_Down:
                    if (in == playing_down) {
                        return playing_nil;
                    }
            };
    };
    return in;
}

#ifndef _TESTING
int main(void) {
    Display *display = XOpenDisplay(NULL);
    int screen       = DefaultScreen(display);
    Window window    = XCreateSimpleWindow(display, RootWindow(display, screen),
        40, 40, 1024, 512, 3, BlackPixel(display, screen), WhitePixel(display, screen));

    state game_state = splash;

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
    XMapWindow(display, window);

    while (game_state != quit) {
        usleep(16666);

        /* X11 events */
        if (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            game_state = event_handler(game_state, event);
        }

        /* repaint */
        XClearWindow(display, window);
        switch (game_state) {
            case splash: {
                char *splash = "conway's game of DEATH (controls: q, up-arrow, down-arrow)";
                XDrawString(display, window, DefaultGC(display, screen), 10, 50, splash, strlen(splash));
                break;
            }
            case playing_nil:
                break;
            case playing_up: {
                char *upmsg = "up";
                XDrawString(display, window, DefaultGC(display, screen), 10, 50, upmsg, strlen(upmsg));
                break;
            }
            case playing_down: {
                char *downmsg = "down";
                XDrawString(display, window, DefaultGC(display, screen), 10, 80, downmsg, strlen(downmsg));
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

    XCloseDisplay(display);
    return 0;
}
#endif
