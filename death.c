#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

typedef struct {
    unsigned char cells[288];
} world;

#define _o(x, y) ((x)*48+(y))
#define world_cell_alive(w, x, y)  (((w)->cells[_o(x,y) / 8] & (1 << (_o(x,y) % 8))) ? 1 : 0)
#define world_cell_set(w, x, y, b) (b) ? ((w)->cells[_o(x,y) / 8] |= (1 << (_o(x,y) % 8))) : ((w)->cells[_o(x,y) / 8] &= ~(1 << (_o(x,y) % 8)))

short world_cell_living_neighbors(world *in, short x, short y) {
    short n = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        if ((x+dx >= 0) && (x+dx < 48)) {
            for (int dy = -1; dy <= 1; ++dy) {
                if ((y+dy >= 0) && (y+dy < 48)) {
                    if (! ((dx == 0) && (dy == 0))) {
                        n += world_cell_alive(in, x+dx, y+dy);
                    }
                }
            }
        }
    }
    return n;
}

world world_step(world *in) {
    world out;

    for (int x = 0; x < 48; ++x) {
        for (int y = 0; y < 48; ++y) {
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

world world_slide(world *in, short dx, short dy) {
    world out;

    for (int x = 0; x < 48; ++x) {
        for (int y = 0; y < 48; ++y) {
            if ((x-dx >= 0) && (x-dx < 48) && (y-dy >= 0) && (y-dy < 48)) {
                world_cell_set(&out, x, y, world_cell_alive(in, x-dx, y-dy));
            } else {
                world_cell_set(&out, x, y, (rand() % 8) == 1);
            }
        }
    }
    return out;
}

int e(int i, XEvent e) {
    long k;
    return e.type == KeyPress ? ((k=XLookupKeysym(&e.xkey, 0)) == XK_q ? 0 : (k == XK_Up ? (i == 4 ? i : 3) : (k == XK_Down ? (i == 4 ? i : 5) : (i == 2 ? 1 : (i == 4 ? 2 : i))))) : (e.type == KeyRelease ? ((k=XLookupKeysym(&e.xkey, 0)) == XK_Up ? (i == 3 ? 1 : i) : (i == 5 ? 1 : i)) : i);
}

typedef struct {
    world w;
    int s;
    int tick;
    int start_dy;
    int dx, dy;
    float life_rate;
    float speed;
    float acceleration;
} game;

game game_transition(game *in, int s) {
    game out;
    if (in) {
        memcpy(&out, in, sizeof(game));
        if ((in->s == s) || ((in->s % 2) && (s % 2))) {
            out.s = s;
            return out;
        }
    }

    if (s == 2) {
        FILE *f = fopen("splash.dat", "r");
        fread(&out.w, sizeof(world), 1, f);
        fclose(f);
    } else if (s == 4) {
        FILE *f = fopen("dead.dat", "r");
        fread(&out.w, sizeof(world), 1, f);
        fclose(f);
    } else {
        for (int x = 0; x < 48; ++x) {
            for (int y = 0; y < 48; ++y) {
                world_cell_set(&out.w, x, y, 0);
            }
        }
    }

    if (s % 2) {
        out.life_rate    = 2;
        out.speed        = 1;
        out.acceleration = 0.002;
        out.start_dy     = ((48*20) / 2) - (480 / 2);
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
    out.tick = (in->tick + 1) % (int)(60 / out.life_rate);
    out.speed = in->speed + out.acceleration;
    out.dx = in->dx + out.speed;

    if (in->s == 3) {
        out.dy = in->dy - 2;
    }
    if (in->s == 5) {
        out.dy = in->dy + 2;
    }

    if (out.dx >= (8 * 20)) {
        out.dx = 0;
        out.w = world_slide(&in->w, -8, 0);
    }
    if (out.dy - out.start_dy >= (8 * 20)) {
        out.dy = out.start_dy;
        out.w  = world_slide(&in->w, 0, -8);
    }
    if (out.dy - out.start_dy <= -(8 * 20)) {
        out.dy = out.start_dy;
        out.w  = world_slide(&in->w, 0, 8);
    }

    return out;
}

short game_collision(game *in) {
    if (in->s % 2) {
        int ox, oy;
        for (ox = in->dx + 100; ox < in->dx + 100 + 20; ++ox) {
            for (oy = in->dy + 248; oy < in->dy + 248 + 20; ++oy) {
                int x, y;
                x = ox / 20;
                y = oy / 20;
                if (world_cell_alive(&in->w, x, y)) {
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
        40, 40, 640, 480, 3, BlackPixel(display, screen), WhitePixel(display, screen));
    Pixmap pixmap    = XCreatePixmap(display, window, 640, 480, DefaultDepth(display, screen));
    Pixmap sprites   = XCreatePixmap(display, window, 20, 20 * (4+1), DefaultDepth(display, screen));
    GC gc            = DefaultGC(display, screen);
    XGCValues gcv_white, gcv_black, gcv_green;
    int ptick = 0, pn = 0;

    game the_game;

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
    XMapWindow(display, window);
    gcv_white.foreground = WhitePixel(display, screen);
    gcv_black.foreground = BlackPixel(display, screen);
    gcv_green.foreground = 0x00ff00;

    FILE *f = fopen("sprites.dat", "r");
    fread(&the_game.w, sizeof(world), 1, f);
    fclose(f);

    for (int x = 0; x < 20; ++x) {
        for (int y = 0; y < 20; ++y) {
            if (world_cell_alive(&the_game.w, x, y)) {
                XChangeGC(display, gc, GCForeground, &gcv_black);
            } else {
                XChangeGC(display, gc, GCForeground, &gcv_white);
            }
            XDrawPoint(display, sprites, gc, x, y);
        }
    }

    for (int x = 20; x < 20 + 5; ++x) {
        for (int y = 0; y < 20; ++y) {
            if (world_cell_alive(&the_game.w, x, y)) {
                XChangeGC(display, gc, GCForeground, &gcv_green);
            } else {
                XChangeGC(display, gc, GCForeground, &gcv_white);
            }
            XFillRectangle(display, sprites, gc, 4 * (x - 20), 20 + 4 * y, 4, 4);
        }
    }

    srand(time(0));
    the_game = game_transition(NULL, 2);

    for (;;) {
        usleep(1000000 / 60);

        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);
    
            the_game = game_transition(&the_game, e(the_game.s, event));
        }

        if (! the_game.s) {
            break;
        }

        the_game = game_tick(&the_game);
        if (game_collision(&the_game)) {
            the_game = game_transition(&the_game, 4);
        }

        XChangeGC(display, gc, GCForeground, &gcv_white);
        XFillRectangle(display, pixmap, gc, 0, 0, 640, 480);
        for (int y = 0; y < 48; ++y) {
            int oy = y*20-the_game.dy;
            for (int x = 0; x < 48; ++x) {
                int ox = x*20-the_game.dx;
                if (world_cell_alive(&the_game.w, x, y)) {
                    XCopyArea(display, sprites, pixmap, gc, 0, 0, 20, 20, ox, oy);
                }
            }
        }
        if (the_game.s % 2) {
            XCopyArea(display, sprites, pixmap, gc, 0, 20*(pn + 1),
                20, 20, 100, 248);
            ptick = (ptick + 1) % (int)(60 / 5);
            if (ptick == 0) {
                pn = (pn + 1) % 4;
            }
        }

        XCopyArea(display, pixmap, window, gc, 0, 0, 640, 480, 0, 0);
        XFlush(display);
    }

    XCloseDisplay(display);
    return 0;
}
#endif
