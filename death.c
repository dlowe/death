#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

#define DIM 48

typedef struct {
    unsigned char cells[(DIM * DIM) / 8];
} world;

#define _o(d, x, y) ((x)*d+(y))
#define world_cell_alive(w, d, x, y)  (((w)->cells[_o(d,x,y) / 8] & (1 << (_o(d,x,y) % 8))) ? 1 : 0)
#define world_cell_set(w, d, x, y, b) (b) ? ((w)->cells[_o(d,x,y) / 8] |= (1 << (_o(d,x,y) % 8))) : ((w)->cells[_o(d,x,y) / 8] &= ~(1 << (_o(d,x,y) % 8)))

short world_cell_living_neighbors(world *in, short x, short y) {
    short n = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        if ((x+dx >= 0) && (x+dx < DIM)) {
            for (int dy = -1; dy <= 1; ++dy) {
                if ((y+dy >= 0) && (y+dy < DIM)) {
                    if (! ((dx == 0) && (dy == 0))) {
                        n += world_cell_alive(in, DIM, x+dx, y+dy);
                    }
                }
            }
        }
    }
    return n;
}

world world_step(world *in) {
    world out;

    for (int x = 0; x < DIM; ++x) {
        for (int y = 0; y < DIM; ++y) {
            short n = world_cell_living_neighbors(in, x, y);
            if (world_cell_alive(in, DIM, x, y)) {
                world_cell_set(&out, DIM, x, y, (n == 2) || (n == 3));
            } else {
                world_cell_set(&out, DIM, x, y, n == 3);
            }
        }
    }

    return out;
}

world world_slide(world *in, short dx, short dy) {
    world out;

    for (int x = 0; x < DIM; ++x) {
        for (int y = 0; y < DIM; ++y) {
            if ((x-dx >= 0) && (x-dx < DIM) && (y-dy >= 0) && (y-dy < DIM)) {
                world_cell_set(&out, DIM, x, y, world_cell_alive(in, DIM, x-dx, y-dy));
            } else {
                world_cell_set(&out, DIM, x, y, (rand() % 8) == 1);
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
                    } else {
                        return dead;
                    }
                case XK_Down:
                    if (in != dead) {
                        return playing_down;
                    } else {
                        return dead;
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

#define CELL_SIZE           20
#define PLAYER_LOOP         4
#define PLAYER_RATE         5
#define PLAYER_LEFT         100
#define PLAYER_TOP          248
#define FRAME_RATE          60
#define LIFE_RATE           2
#define CONTROL_SENSITIVITY 2
#define SPEED_START         1
#define SPEED_ZOOM          0.002
#define WINDOW_WIDTH        640
#define WINDOW_HEIGHT       480

typedef struct {
    world w;
    state s;
    int tick;
    int start_dy;
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
        FILE *f = fopen("splash.dat", "r");
        fread(&out.w, sizeof(world), 1, f);
        fclose(f);
    } else if (s == dead) {
        FILE *f = fopen("dead.dat", "r");
        fread(&out.w, sizeof(world), 1, f);
        fclose(f);
    } else {
        for (int x = 0; x < DIM; ++x) {
            for (int y = 0; y < DIM; ++y) {
                world_cell_set(&out.w, x, DIM, y, 0);
            }
        }
    }

    if (state_playing(s)) {
        out.life_rate    = LIFE_RATE;
        out.speed        = SPEED_START;
        out.acceleration = SPEED_ZOOM;
        out.start_dy     = ((DIM*CELL_SIZE) / 2) - (WINDOW_HEIGHT / 2);
    } else {
        out.life_rate    = 0.5;
        out.speed        = 0;
        out.acceleration = 0;
        out.start_dy     = 0;
    }
    out.tick  = 1;
    out.dy    = out.start_dy;
    out.dx    = 0;
    out.s     = s;

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

    if (out.dx >= (8 * CELL_SIZE)) {
        out.dx = 0;
        out.w = world_slide(&in->w, -8, 0);
    }
    if (out.dy - out.start_dy >= (8 * CELL_SIZE)) {
        out.dy = out.start_dy;
        out.w  = world_slide(&in->w, 0, -8);
    }
    if (out.dy - out.start_dy <= -(8 * CELL_SIZE)) {
        out.dy = out.start_dy;
        out.w  = world_slide(&in->w, 0, 8);
    }

    return out;
}

short game_collision(game *in) {
    if (state_playing(in->s)) {
        int ox, oy;
        for (ox = in->dx + PLAYER_LEFT; ox < in->dx + PLAYER_LEFT + CELL_SIZE; ++ox) {
            for (oy = in->dy + PLAYER_TOP; oy < in->dy + PLAYER_TOP + CELL_SIZE; ++oy) {
                int x, y;
                x = ox / CELL_SIZE;
                y = oy / CELL_SIZE;
                if (world_cell_alive(&in->w, DIM, x, y)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

#ifndef _TESTING
int main(void) {
    Display *display = XOpenDisplay(NULL);
    int screen       = DefaultScreen(display);
    Window window    = XCreateSimpleWindow(display, RootWindow(display, screen),
        40, 40, WINDOW_WIDTH, WINDOW_HEIGHT, 3, BlackPixel(display, screen), WhitePixel(display, screen));
    Pixmap pixmap    = XCreatePixmap(display, window, WINDOW_WIDTH, WINDOW_HEIGHT, DefaultDepth(display, screen));
    Pixmap sprites   = XCreatePixmap(display, window, CELL_SIZE, CELL_SIZE * (PLAYER_LOOP+1), DefaultDepth(display, screen));
    GC gc            = DefaultGC(display, screen);
    XGCValues gcv_white, gcv_black, gcv_green;
    int ptick = 0, pn = 0;

    game the_game;

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
    XMapWindow(display, window);
    gcv_white.foreground = WhitePixel(display, screen);
    gcv_black.foreground = BlackPixel(display, screen);
    gcv_green.foreground = 0x00ff00;

    /* build a sprite for cells */
    FILE *f = fopen("sprites.dat", "r");
    fread(&the_game.w, sizeof(world), 1, f);
    fclose(f);

    for (int x = 0; x < CELL_SIZE; ++x) {
        for (int y = 0; y < CELL_SIZE; ++y) {
            if (world_cell_alive(&the_game.w, DIM, x, y)) {
                XChangeGC(display, gc, GCForeground, &gcv_black);
            } else {
                XChangeGC(display, gc, GCForeground, &gcv_white);
            }
            XDrawPoint(display, sprites, gc, x, y);
        }
    }

    for (int x = CELL_SIZE; x < CELL_SIZE + 5; ++x) {
        for (int y = 0; y < CELL_SIZE; ++y) {
            if (world_cell_alive(&the_game.w, DIM, x, y)) {
                XChangeGC(display, gc, GCForeground, &gcv_green);
            } else {
                XChangeGC(display, gc, GCForeground, &gcv_white);
            }
            XFillRectangle(display, sprites, gc, 4 * (x - CELL_SIZE), CELL_SIZE + 4 * y, 4, 4);
        }
    }

    srand(time(0));
    the_game = game_transition(NULL, splash);

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
                if (world_cell_alive(&the_game.w, DIM, x, y)) {
                    XCopyArea(display, sprites, pixmap, gc, 0, 0, CELL_SIZE, CELL_SIZE, ox, oy);
                }
            }
        }
        if (state_playing(the_game.s)) {
            XCopyArea(display, sprites, pixmap, gc, 0, CELL_SIZE*(pn + 1),
                CELL_SIZE, CELL_SIZE, PLAYER_LEFT, PLAYER_TOP);
            ptick = (ptick + 1) % (int)(FRAME_RATE / PLAYER_RATE);
            if (ptick == 0) {
                pn = (pn + 1) % PLAYER_LOOP;
            }
        }

        /* copy buffer & flush */
        XCopyArea(display, pixmap, window, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
        XFlush(display);
    }

    XCloseDisplay(display);
    return 0;
}
#endif
