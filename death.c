#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef enum { splash, quit, playing_nil, playing_up, playing_down, dead } state;

#define DIM 80
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
            world_cell_set(&out, x, y, (rand() % 8) == 1);
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

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 512
#define CELL_SIZE     20
#define PLAYER_WIDTH  20
#define PLAYER_HEIGHT 16
#define PLAYER_LEFT   100
#define PLAYER_TOP    248

#ifndef _TESTING
int main(void) {
    short tick = 0;
    int dx = 0, dy = 0;
    Display *display = XOpenDisplay(NULL);
    int screen       = DefaultScreen(display);
    Window window    = XCreateSimpleWindow(display, RootWindow(display, screen),
        40, 40, WINDOW_WIDTH, WINDOW_HEIGHT, 3, BlackPixel(display, screen), WhitePixel(display, screen));
    Pixmap pixmap    = XCreatePixmap(display, window, WINDOW_WIDTH, WINDOW_HEIGHT, DefaultDepth(display, screen));
    Pixmap player    = XCreatePixmap(display, window, PLAYER_WIDTH, PLAYER_HEIGHT, DefaultDepth(display, screen));
    Pixmap cell      = XCreatePixmap(display, window, CELL_SIZE, CELL_SIZE, DefaultDepth(display, screen));
    GC gc            = DefaultGC(display, screen);
    XGCValues gcv_white, gcv_black, gcv_green;

    state game_state = splash;
    world the_world;

    srand(time(0));
    the_world = world_new();

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
    XMapWindow(display, window);
    gcv_white.foreground = WhitePixel(display, screen);
    gcv_black.foreground = BlackPixel(display, screen);
    gcv_green.foreground = 0x00ff00;

    /* build a sprite for cells */
    XChangeGC(display, gc, GCForeground, &gcv_black);
    XFillRectangle(display, cell, gc, 0, 0, CELL_SIZE, CELL_SIZE);
    XChangeGC(display, gc, GCForeground, &gcv_white);
    XDrawPoint(display, cell, gc, 0, 0);
    XDrawPoint(display, cell, gc, 0, CELL_SIZE-1);
    XDrawPoint(display, cell, gc, CELL_SIZE-1, 0);
    XDrawPoint(display, cell, gc, CELL_SIZE-1, CELL_SIZE-1);
    XChangeGC(display, gc, GCForeground, &gcv_black);

    /* build a player sprite */
    XChangeGC(display, gc, GCForeground, &gcv_white);
    XFillRectangle(display, player, gc, 0, 0, PLAYER_WIDTH, PLAYER_HEIGHT);
    XChangeGC(display, gc, GCForeground, &gcv_green);
    XFillRectangle(display, player, gc,  0,  0, 4, 4);
    XFillRectangle(display, player, gc,  0,  8, 4, 4);
    XFillRectangle(display, player, gc,  4, 12, 4, 4);
    XFillRectangle(display, player, gc,  8, 12, 4, 4);
    XFillRectangle(display, player, gc, 12,  0, 4, 4);
    XFillRectangle(display, player, gc, 12, 12, 4, 4);
    XFillRectangle(display, player, gc, 16,  4, 4, 4);
    XFillRectangle(display, player, gc, 16,  8, 4, 4);
    XFillRectangle(display, player, gc, 16, 12, 4, 4);
    XChangeGC(display, gc, GCForeground, &gcv_black);

    while (game_state != quit) {
        usleep(16666);

        /* X11 events */
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            game_state = event_handler(game_state, event);
        }

        /* game */
        if (game_state == playing_up) {
            dy = dy - 2;
        }
        if (game_state == playing_down) {
            dy = dy + 2;
        }
        if ((game_state == playing_nil) || (game_state == playing_up) || (game_state == playing_down)) {
            if (tick == 0) {
                the_world = world_step(&the_world);
            }
            tick = (tick + 1) % 30;
        }

        /* repaint into buffer */
        XChangeGC(display, gc, GCForeground, &gcv_white);
        XFillRectangle(display, pixmap, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        XChangeGC(display, gc, GCForeground, &gcv_black);
        switch (game_state) {
            case splash: {
                char *splash = "conway's game of DEATH (controls: q, up-arrow, down-arrow)";
                XDrawString(display, pixmap, gc, 10, 50, splash, strlen(splash));
                break;
            }
            case playing_nil:
            case playing_up:
            case playing_down: {
                int x, y;

                XCopyArea(display, player, pixmap, gc, 0, 0, PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_LEFT, PLAYER_TOP);

                for (y = 0; y < DIM; ++y) {
                    for (x = 0; x < DIM; ++x) {
                        if (world_cell_alive(&the_world, x, y)) {
                            int ox = x*CELL_SIZE-dx;
                            int oy = y*CELL_SIZE-dy;
                            XCopyArea(display, cell, pixmap, gc, 0, 0, CELL_SIZE, CELL_SIZE, ox, oy);
                        }
                    }
                }
                dx = dx + 1;
                break;
            }
            case dead: {
                char *death = "you died.";
                XDrawString(display, pixmap, gc, 10, 50, death, strlen(death));
                break;
            }
            default:
                break;
        };

        /* copy buffer & flush */
        XCopyArea(display, pixmap, window, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
        XFlush(display);
    }

    XCloseDisplay(display);
    return 0;
}
#endif
