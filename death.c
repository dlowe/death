#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef enum { splash, quit, playing_nil, playing_up, playing_down, dead } state;

#define DIM                 80
#define WINDOW_WIDTH        1024
#define WINDOW_HEIGHT       512
#define CELL_SIZE           20
#define PLAYER_WIDTH        20
#define PLAYER_HEIGHT       16
#define PLAYER_LEFT         100
#define PLAYER_TOP          248
#define FRAME_RATE          60
#define LIFE_RATE           2
#define CONTROL_SENSITIVITY 2
#define SPEED_START         1
#define SPEED_ZOOM          0.005

#define world_cell_alive(w, x, y)  (/*printf("%d,%d\n", x, y),*/(w)->cells[(x)*(DIM)+(y)])
#define world_cell_set(w, x, y, b) (/*printf("%d,%d,%d\n", x, y, b),*/(w)->cells[(x)*(DIM)+(y)] = (b))

typedef struct {
    unsigned char cells[DIM * DIM];
} world;

typedef struct {
    world w;
    state s;
    int tick;
    int dx, dy;
    float speed;
} game;

game game_new(void) {
    int x, y;
    game out;

    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            world_cell_set(&out.w, x, y, (rand() % 8) == 1);
        }
    }

    out.tick  = 0;
    out.dx    = 0;
    out.dy    = 0;
    out.speed = SPEED_START;
    out.s     = splash;

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

game game_tick(game *in) {
    game out;

    memcpy(&out.w, &in->w, sizeof(world));
    if (in->tick == 0) {
        out.w = world_step(&in->w);
    }
    out.tick = (in->tick + 1) % (FRAME_RATE / LIFE_RATE);
    out.speed = in->speed + SPEED_ZOOM;
    out.dx = in->dx + out.speed;
    out.dy = in->dy;
    out.s = in->s;

    if (in->s == playing_up) {
        out.dy = in->dy - CONTROL_SENSITIVITY;
    }
    if (in->s == playing_down) {
        out.dy = in->dy + CONTROL_SENSITIVITY;
    }

    return out;
}

short game_collision(game *in) {
    int ox, oy;
    for (ox = in->dx + PLAYER_LEFT; ox < in->dx + PLAYER_LEFT + PLAYER_WIDTH; ++ox) {
        for (oy = in-> dy + PLAYER_TOP; oy < in->dy + PLAYER_TOP + PLAYER_HEIGHT; ++oy) {
            int x, y;
            x = ox / CELL_SIZE;
            y = oy / CELL_SIZE;
            if (world_cell_alive(&in->w, x, y)) {
                return 1;
            }
        }
    }
    return 0;
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
    Display *display = XOpenDisplay(NULL);
    int screen       = DefaultScreen(display);
    Window window    = XCreateSimpleWindow(display, RootWindow(display, screen),
        40, 40, WINDOW_WIDTH, WINDOW_HEIGHT, 3, BlackPixel(display, screen), WhitePixel(display, screen));
    Pixmap pixmap    = XCreatePixmap(display, window, WINDOW_WIDTH, WINDOW_HEIGHT, DefaultDepth(display, screen));
    Pixmap player    = XCreatePixmap(display, window, PLAYER_WIDTH, PLAYER_HEIGHT, DefaultDepth(display, screen));
    Pixmap cell      = XCreatePixmap(display, window, CELL_SIZE, CELL_SIZE, DefaultDepth(display, screen));
    GC gc            = DefaultGC(display, screen);
    XGCValues gcv_white, gcv_black, gcv_green;

    game the_game;

    srand(time(0));
    the_game = game_new();

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

    while (the_game.s != quit) {
        usleep(1000000 / FRAME_RATE);

        /* X11 events */
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            the_game.s = event_handler(the_game.s, event);
        }

        /* tick the game */
        the_game = game_tick(&the_game);
        if (game_collision(&the_game)) {
            the_game.s = dead;
        }

        /* repaint into buffer */
        XChangeGC(display, gc, GCForeground, &gcv_white);
        XFillRectangle(display, pixmap, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        XChangeGC(display, gc, GCForeground, &gcv_black);
        switch (the_game.s) {
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
                    int oy = y*CELL_SIZE-the_game.dy;
                    for (x = 0; x < DIM; ++x) {
                        int ox = x*CELL_SIZE-the_game.dx;
                        if (world_cell_alive(&the_game.w, x, y)) {
                            XCopyArea(display, cell, pixmap, gc, 0, 0, CELL_SIZE, CELL_SIZE, ox, oy);
                        }
                    }
                }
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
