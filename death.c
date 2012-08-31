#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define DIM 64

typedef struct {
    unsigned char cells[(DIM * DIM) / 8];
} world;

#define _o(x, y) ((x)*DIM+(y))
#define world_cell_alive(w, x, y)  (((w)->cells[_o(x,y) / 8] & (1 << (_o(x,y) % 8))) ? 1 : 0)
#define world_cell_set(w, x, y, b) (b) ? ((w)->cells[_o(x,y) / 8] |= (1 << (_o(x,y) % 8))) : ((w)->cells[_o(x,y) / 8] &= ~(1 << (_o(x,y) % 8)))

short world_cell_living_neighbors(world *in, short x, short y) {
    short n = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        if ((x+dx >= 0) && (x+dx < DIM)) {
            for (int dy = -1; dy <= 1; ++dy) {
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

typedef enum { splash, quit, playing_nil, playing_up, playing_down, dead } state;

#define state_playing(s) (((s) == playing_nil) || ((s) == playing_up) || ((s) == playing_down))

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

world str_to_world(short width, char *in) {
    int x, y;
    world out;

    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            world_cell_set(&out, x, y, 0);
        }
    }

    y = 0;
    while (in[width * y]) {
        for (x = 0; x < width; ++x) {
            world_cell_set(&out, x, y, in[width * y + x] != '_');
        }
        ++y;
    }
    return out;
}

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

typedef struct {
    world w;
    state s;
    int tick;
    int dx, dy;
    float life_rate;
    float speed;
    float acceleration;
} game;

game game_transition(game *in, state s) {
    game out;
    if (in) {
        memcpy(&out, in, sizeof(game));
        if ((in->s == s) || (state_playing(in->s) && state_playing(s))) {
            out.s = s;
            return out;
        }
    }

    if (s == splash) {
        char *splash_s =
            "______________________________"
            "__OO_____________________O____"
            "_O____O__OO__O_O_OO__O_O_O__OO"
            "_O___O_O_O_O_OOO__OO_O_O___OO_"
            "_O___O_O_O_O_OOO_O_O__OO____OO"
            "__OO__O__O_O_OOO_OOO___O___OO_"
            "______________________O_______"
            "__OO_____________________O____"
            "_O___OO__OOO__OO____O___O_____"
            "_OOO__OO_OOO_O_O___O_O_OOO____"
            "_O_O_O_O_OOO_OO____O_O__O_____"
            "__OO_OOO_O_O__OO____O___O_____"
            "______________________________"
            "_OO__OOO__O__OOO_O_O__________"
            "_O_O_O___O_O__O__O_O__________"
            "_O_O_OOO_OOO__O__OOO__________"
            "_O_O_O___O_O__O__O_O__________"
            "_OO__OOO_O_O__O__O_O__________";
        out.w = str_to_world(30, splash_s);
    } else if (s == dead) {
        char *dead_s =
            "______________________"
            "______________________"
            "______________________"
            "______________________"
            "________OO__O__O_O_OOO"
            "_______O___O_O_OOO_O__"
            "_______OOO_OOO_OOO_OOO"
            "_______O_O_O_O_O_O_O__"
            "________OO_O_O_O_O_OOO"
            "______________________"
            "________O__O_O_OOO_OO_"
            "_______O_O_O_O_O___O_O"
            "_______O_O_O_O_OOO_OOO"
            "_______O_O_O_O_O___OO_"
            "________O___O__OOO_O_O";
        out.w = str_to_world(22, dead_s);
    } else {
        for (int x = 0; x < DIM; ++x) {
            for (int y = 0; y < DIM; ++y) {
                world_cell_set(&out.w, x, y, (rand() % 8) == 1);
            }
        }
    }

    out.tick  = 1;
    out.dx    = 0;
    out.dy    = 0;
    out.s     = s;
    if (state_playing(s)) {
        out.life_rate    = LIFE_RATE;
        out.speed        = SPEED_START;
        out.acceleration = SPEED_ZOOM;
    } else {
        out.life_rate    = 0.5;
        out.speed        = 0;
        out.acceleration = 0;
    }

    return out;
}

game game_tick(game *in) {
    game out;

    memcpy(&out, in, sizeof(game));
    if (in->tick == 0) {
        out.w = world_step(&in->w);
    }
    out.tick = (in->tick + 1) % (int)(FRAME_RATE / out.life_rate);
    out.speed = in->speed + out.acceleration;
    out.dx = in->dx + out.speed;

    if (in->s == playing_up) {
        out.dy = in->dy - CONTROL_SENSITIVITY;
    }
    if (in->s == playing_down) {
        out.dy = in->dy + CONTROL_SENSITIVITY;
    }

    return out;
}

short game_collision(game *in) {
    if (state_playing(in->s)) {
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
    }
    return 0;
}

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

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
    the_game = game_transition(NULL, splash);

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

    for (;;) {
        usleep(1000000 / FRAME_RATE);

        /* X11 events */
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);
    
            the_game = game_transition(&the_game, event_handler(the_game.s, event));
        }

        if (the_game.s == quit) {
            break;
        }

        /* tick the game */
        the_game = game_tick(&the_game);
        if (game_collision(&the_game)) {
            the_game = game_transition(&the_game, dead);
        }

        /* repaint into buffer */
        XChangeGC(display, gc, GCForeground, &gcv_white);
        XFillRectangle(display, pixmap, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        for (int y = 0; y < DIM; ++y) {
            int oy = y*CELL_SIZE-the_game.dy;
            for (int x = 0; x < DIM; ++x) {
                int ox = x*CELL_SIZE-the_game.dx;
                if (world_cell_alive(&the_game.w, x, y)) {
                    XCopyArea(display, cell, pixmap, gc, 0, 0, CELL_SIZE, CELL_SIZE, ox, oy);
                }
            }
        }
        if (state_playing(the_game.s)) {
            XCopyArea(display, player, pixmap, gc, 0, 0, PLAYER_WIDTH, PLAYER_HEIGHT,
                PLAYER_LEFT, PLAYER_TOP);
        }

        /* copy buffer & flush */
        XCopyArea(display, pixmap, window, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
        XFlush(display);
    }

    XCloseDisplay(display);
    return 0;
}
#endif
