#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef enum { splash, quit, playing_nil, playing_up, playing_down, dead } state;

#define DIM 1000
#define world_cell_alive(w, x, y)  (w)->cells[(x)][(y)]
#define world_cell_set(w, x, y, b) (w)->cells[(x)][(y)] = (b)

typedef struct {
    short cells[DIM][DIM];
} world;

world world_new(void) {
    int x, y;
    world out;

    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            world_cell_set(&out, x, y, rand() % 2);
        }
    }

    return out;
}

short world_cell_living_neighbors(world *in, short x, short y) {
    short n = 0;
    short dx, dy;
    for (dx = -1; dx <= 1; ++dx) {
        if ((x+dx >= 0) && (x+dx < DIM)) {
            for (dy = -1; dy <= 1; ++dy) {
                if ((y+dy >= 0) && (y+dy < DIM)) {
                    if (! ((dx == 0) && (dy == 0))) {
                        n += world_cell_alive(in, x+dx, y+dy);
                        if (n >= 4) {
                            return 4;
                        }
                    }
                }
            }
        }
    }
    return n;
}

world world_step(world *in) {
    world out;
    int x, y;

    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            short n = world_cell_living_neighbors(in, x, y);
            if (world_cell_alive(in, x, y)) {
                world_cell_set(&out, x, y, (n == 2) || (n == 3));
            } else {
                world_cell_set(&out, x, y, n == 3);
            }
        }
    }

    return out;
}

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
    short tick = 0;
    int dx = 0, dy = 0;
    Display *display = XOpenDisplay(NULL);
    int screen       = DefaultScreen(display);
    Window window    = XCreateSimpleWindow(display, RootWindow(display, screen),
        40, 40, 1024, 512, 3, BlackPixel(display, screen), WhitePixel(display, screen));

    state game_state = splash;
    world the_world;

    srand(time(0));
    the_world = world_new();

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
    XMapWindow(display, window);

    while (game_state != quit) {
        usleep(50000);

        /* X11 events */
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            game_state = event_handler(game_state, event);
        }

        /* game */
        if (game_state == playing_up) {
            dy = dy - 4;
        }
        if (game_state == playing_down) {
            dy = dy + 4;
        }
        if ((game_state == playing_nil) || (game_state == playing_up) || (game_state == playing_down)) {
            if (tick == 0) {
                the_world = world_step(&the_world);
            }
            tick = (tick + 1) % 3;
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
            case playing_up:
            case playing_down: {
                int x, y;
                for (y = 0; y < DIM; ++y) {
                    for (x = 0; x < DIM; ++x) {
                        if (world_cell_alive(&the_world, x, y)) {
                            XFillRectangle(display, window, DefaultGC(display, screen), x*20 - dx, y*20 - dy, 20, 20);
                        }
                    }
                }
                dx = dx + 8;
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
