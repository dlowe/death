#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

typedef struct {
    char c[288];
} M;

#define B(x, y) ((x)*48+(y))
#define C(x, y) (1 << B(x,y) % 8)
#define A(w, x, y) (((w)->c[B(x,y) / 8] & C(x,y)) ? 1 : 0)
#define S(w, x, y, b) (b) ? ((w)->c[B(x,y) / 8] |= C(x,y)) : ((w)->c[B(x,y) / 8] &= ~(C(x,y)))
#define L(b) for (x = 0; x < 48; ++x) for (y = 0; y < 48; ++y) b

M wt(M *in) {
    M o;
    int x, y, n, a, b;

    L({
        for (n = 0, a = -1; a <= 1; ++a) {
            if (x+a >= 0 && x+a < 48) {
                for (b = -1; b <= 1; ++b) {
                    if (y+b >= 0 && y+b < 48) {
                        n += (a || b) && A(in, x+a, y+b);
                    }
                }
            }
        }

        S(&o, x, y, A(in, x, y) ? n == 2 || n == 3 : n == 3);
    })

    return o;
}

M ws(M *in, int a, int b) {
    M o;
    int x, y;

    L(S(&o, x, y, ((x-a >= 0) && (x-a < 48) && (y-b >= 0) && (y-b < 48)) ? A(in, x-a, y-b) : ((rand() % 8) == 1)));
    return o;
}

int e(int i, XEvent e) {
    long k;
    return e.type == KeyPress ? ((k=XLookupKeysym(&e.xkey, 0)) == XK_q ? 0 : (k == XK_Up ? (i == 4 ? i : 3) : (k == XK_Down ? (i == 4 ? i : 5) : (i == 2 ? 1 : (i == 4 ? 2 : i))))) : (e.type == KeyRelease ? ((k=XLookupKeysym(&e.xkey, 0)) == XK_Up ? (i == 3 ? 1 : i) : (i == 5 ? 1 : i)) : i);
}

typedef struct {
    M w;
    int s, t, y, dx, dy;
    float p;
} G;

G gt(G *i, int s) {
    G o;
    int x, y;

    if (i) {
        o = *i;
        if ((i->s == s) || ((i->s % 2) && (s % 2))) {
            o.s = s;
            return o;
        }
    }

    if (s == 2) {
        freopen("2.d", "r", stdin);
        fread(&o.w, 288, 1, stdin);
    } else if (s == 4) {
        freopen("1.d", "r", stdin);
        fread(&o.w, 288, 1, stdin);
    } else {
        L(S(&o.w, x, y, 0));
    }

    o.p  = 1;
    o.dy = o.y = s % 2 ? 240 : 0;
    o.t  = 1;
    o.dx = 0;
    o.s  = s;

    return o;
}

G gi(G *i) {
    G o = *i;

    if (! o.t) {
        o.w = wt(&o.w);
    }
    o.t = ++o.t % (o.s % 2 ? 30 : 120);
    o.p += 0.002;
    o.dx += o.s % 2 ? o.p : 0;
    o.dy += o.s == 3 ? -2 : (o.s == 5 ? 2 : 0);

    if (o.dx >= 160) {
        o.dx = 0;
        o.w = ws(&o.w, -8, 0);
    }
    if (o.dy - o.y >= 160) {
        o.dy = o.y;
        o.w  = ws(&o.w, 0, -8);
    }
    if (o.dy - o.y <= -160) {
        o.dy = o.y;
        o.w  = ws(&o.w, 0, 8);
    }

    return o;
}

int gc(G *i) {
    int x, y;

    for (x = i->dx + 100; x < i->dx + 120; ++x) {
        for (y = i->dy + 248; y < i->dy + 268; ++y) {
            if A(&i->w, x / 20, y / 20) {
                return 1;
            }
        }
    }
    return 0;
}

#ifndef _TESTING
int main() {
    Display *d = XOpenDisplay(0);
    int x, y, s = DefaultScreen(d);
    Window w   = XCreateSimpleWindow(d, RootWindow(d, s), 40, 40, 640, 480, 3, 0, 0);
    Pixmap b   = XCreatePixmap(d, w, 640, 480, DefaultDepth(d, s)), p = XCreatePixmap(d, w, 20, 20 * (4+1), DefaultDepth(d, s));
    GC g       = DefaultGC(d, s);
    XGCValues W, B;
    G t;

    XSelectInput(d, w, KeyPressMask | KeyReleaseMask);
    XMapWindow(d, w);
    W.foreground = WhitePixel(d, s);
    B.foreground = BlackPixel(d, s);

    freopen("0.d", "r", stdin);
    fread(&t.w, 288, 1, stdin);

    for (x = 0; x < 20; ++x) {
        for (y = 0; y < 20; ++y) {
            XChangeGC(d, g, GCForeground, A(&t.w, x, y) ? &B : &W);
            XDrawPoint(d, p, g, x, y);
        }
    }

    for (x = 20; x < 25; ++x) {
        for (y = 0; y < 20; ++y) {
            XChangeGC(d, g, GCForeground, A(&t.w, x, y) ? &B : &W);
            XFillRectangle(d, p, g, 4 * (x - 20), 20 + 4 * y, 4, 4);
        }
    }

    srand(time(0));
    t = gt(0, 2);

    while (t.s) {
        usleep(16666);

        while (XPending(d)) {
            XEvent v;
            XNextEvent(d, &v);
    
            t = gt(&t, e(t.s, v));
        }

        t = gi(&t);

        XFillRectangle(d, b, g, 0, 0, 640, 480);
        L({
            if A(&t.w, x, y) {
                XCopyArea(d, p, b, g, 0, 0, 20, 20, x*20-t.dx, y*20-t.dy);
            }
        })
        if (t.s % 2) {
            if (gc(&t)) {
                t = gt(&t, 4);
            }
            XCopyArea(d, p, b, g, 0, 20*(t.t / 8 + 1), 20, 20, 100, 248);
        }

        XCopyArea(d, b, w, g, 0, 0, 640, 480, 0, 0);
    }
    return 0;
}
#endif
