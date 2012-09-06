#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

typedef struct {
    unsigned char c[288];
} world;

#define B(x, y) ((x)*48+(y))
#define C(x, y) (1 << B(x,y) % 8)
#define A(w, x, y) (((w)->c[B(x,y) / 8] & C(x,y)) ? 1 : 0)
#define S(w, x, y, b) (b) ? ((w)->c[B(x,y) / 8] |= C(x,y)) : ((w)->c[B(x,y) / 8] &= ~(C(x,y)))

short world_cell_living_neighbors(world *in, short x, short y) {
    short n = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        if ((x+dx >= 0) && (x+dx < 48)) {
            for (int dy = -1; dy <= 1; ++dy) {
                if ((y+dy >= 0) && (y+dy < 48)) {
                    if (! ((dx == 0) && (dy == 0))) {
                        n += A(in, x+dx, y+dy);
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
            if (A(in, x, y)) {
                S(&out, x, y, (n == 2) || (n == 3));
            } else {
                S(&out, x, y, n == 3);
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
                S(&out, x, y, A(in, x-dx, y-dy));
            } else {
                S(&out, x, y, (rand() % 8) == 1);
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
    int s, tick, start_dy, dx, dy;
    float life_rate, speed, acceleration;
} G;

G game_transition(G *in, int s) {
    G o;
    if (in) {
        o = *in;
        if ((in->s == s) || ((in->s % 2) && (s % 2))) {
            o.s = s;
            return o;
        }
    }

    if (s == 2) {
        freopen("2.d", "r", stdin);
        fread(&o.w, sizeof(world), 1, stdin);
    } else if (s == 4) {
        freopen("1.d", "r", stdin);
        fread(&o.w, sizeof(world), 1, stdin);
    } else {
        for (int x = 0; x < 48; ++x) {
            for (int y = 0; y < 48; ++y) {
                S(&o.w, x, y, 0);
            }
        }
    }

    o.life_rate = s % 2 ? 2 : 0.5;
    o.speed     = s % 2;
    o.acceleration = s % 2 ? 0.002 : 0;
    o.dy = o.start_dy     = s % 2 ? 240 : 0;
    o.tick  = 1;
    o.dx    = 0;
    o.s     = s;

    return o;
}

G game_tick(G *in) {
    G o = *in;

    if (o.tick == 0) {
        o.w = world_step(&o.w);
    }
    o.tick = (o.tick + 1) % (int)(60 / o.life_rate);
    o.speed += o.acceleration;
    o.dx += o.speed;
    o.dy += o.s == 3 ? -2 : (o.s == 5 ? 2 : 0);

    if (o.dx >= 160) {
        o.dx = 0;
        o.w = world_slide(&o.w, -8, 0);
    }
    if (o.dy - o.start_dy >= 160) {
        o.dy = o.start_dy;
        o.w  = world_slide(&o.w, 0, -8);
    }
    if (o.dy - o.start_dy <= -160) {
        o.dy = o.start_dy;
        o.w  = world_slide(&o.w, 0, 8);
    }

    return o;
}

int game_collision(G *in) {
    if (in->s % 2) {
        int ox, oy;
        for (ox = in->dx + 100; ox < in->dx + 100 + 20; ++ox) {
            for (oy = in->dy + 248; oy < in->dy + 248 + 20; ++oy) {
                int x, y;
                x = ox / 20;
                y = oy / 20;
                if (A(&in->w, x, y)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

#ifndef _TESTING
int main(void) {
    Display *d = XOpenDisplay(NULL);
    int s      = DefaultScreen(d);
    Window w   = XCreateSimpleWindow(d, RootWindow(d, s), 40, 40, 640, 480, 3, 0, 0);
    Pixmap b   = XCreatePixmap(d, w, 640, 480, DefaultDepth(d, s));
    Pixmap p   = XCreatePixmap(d, w, 20, 20 * (4+1), DefaultDepth(d, s));
    GC g       = DefaultGC(d, s);
    XGCValues W, B;
    int ptick = 0, pn = 0;

    G t;

    XSelectInput(d, w, KeyPressMask | KeyReleaseMask);
    XMapWindow(d, w);
    W.foreground = WhitePixel(d, s);
    B.foreground = BlackPixel(d, s);

    freopen("0.d", "r", stdin);
    fread(&t.w, sizeof(world), 1, stdin);

    for (int x = 0; x < 20; ++x) {
        for (int y = 0; y < 20; ++y) {
            XChangeGC(d, g, GCForeground, A(&t.w, x, y) ? &B : &W);
            XDrawPoint(d, p, g, x, y);
        }
    }

    for (int x = 20; x < 20 + 5; ++x) {
        for (int y = 0; y < 20; ++y) {
            XChangeGC(d, g, GCForeground, A(&t.w, x, y) ? &B : &W);
            XFillRectangle(d, p, g, 4 * (x - 20), 20 + 4 * y, 4, 4);
        }
    }

    srand(time(0));
    t = game_transition(NULL, 2);

    for (; ; ) {
        usleep(16666);

        while (XPending(d)) {
            XEvent v;
            XNextEvent(d, &v);
    
            t = game_transition(&t, e(t.s, v));
        }

        if (! t.s) {
            break;
        }

        t = game_tick(&t);
        if (game_collision(&t)) {
            t = game_transition(&t, 4);
        }

        XChangeGC(d, g, GCForeground, &W);
        XFillRectangle(d, b, g, 0, 0, 640, 480);
        for (int y = 0; y < 48; ++y) {
            int oy = y*20-t.dy;
            for (int x = 0; x < 48; ++x) {
                int ox = x*20-t.dx;
                if (A(&t.w, x, y)) {
                    XCopyArea(d, p, b, g, 0, 0, 20, 20, ox, oy);
                }
            }
        }
        if (t.s % 2) {
            XCopyArea(d, p, b, g, 0, 20*(pn + 1), 20, 20, 100, 248);
            ptick = (ptick + 1) % 12;
            if (ptick == 0) {
                pn = (pn + 1) % 4;
            }
        }

        XCopyArea(d, b, w, g, 0, 0, 640, 480, 0, 0);
        XFlush(d);
    }

    XCloseDisplay(d);
    return 0;
}
#endif
