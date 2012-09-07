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
#define L for (x = 0; x < 48; ++x) for (y = 0; y < 48; ++y)

M wt(M *i) {
    M o;
    int x, y, n, a, b;

    L {
        for (n = 0, a = -1; a <= 1; ++a) {
            if (x+a >= 0 && x+a < 48) {
                for (b = -1; b <= 1; ++b) {
                    if (y+b >= 0 && y+b < 48) {
                        n += (a || b) && A(i, x+a, y+b);
                    }
                }
            }
        }

        S(&o, x, y, A(i, x, y) ? n == 2 || n == 3 : n == 3);
    }

    return o;
}

M ws(M *i, int a, int b) {
    M o;
    int x, y;

    L S(&o, x, y, ((x-a >= 0) && (x-a < 48) && (y-b >= 0) && (y-b < 48)) ? A(i, x-a, y-b) : ((rand() % 8) == 1));
    return o;
}

int e(int i, XEvent e) {
    long k;
    return e.type == KeyPress ? ((k=XLookupKeysym(&e.xkey, 0)) == XK_q ? 0 : (k == XK_Up ? (i == 4 ? i : 3) : (k == XK_Down ? (i == 4 ? i : 5) : (i == 2 ? 1 : (i == 4 ? 2 : i))))) : (e.type == KeyRelease ? ((k=XLookupKeysym(&e.xkey, 0)) == XK_Up ? (i == 3 ? 1 : i) : (i == 5 ? 1 : i)) : i);
}

typedef struct {
    M w;
    int s, p, t, a, b;
} G;

G gt(G *i, int s) {
    G o;
    int x, y;

    if (i) {
        o = *i;
        if (i->s == s || (i->s % 2 && s % 2)) {
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
        L S(&o.w, x, y, 0);
    }

    o.p = 1000;
    o.b = 240;
    o.t = 1;
    o.a = 0;
    o.s = s;

    return o;
}

G gi(G *i) {
    G o = *i;
    int x, y;

    if (! o.t) {
        o.w = wt(&o.w);
    }
    o.t = ++o.t % (o.s % 2 ? 30 : 120);
    o.p += 2;
    o.a += o.s % 2 ? o.p/1000 : 0;
    o.b += o.s == 3 ? -2 : (o.s == 5 ? 2 : 0);

    x = o.a >= 160 ? -8 : 0;
    y = o.b >= 400 ? -8 : (o.b <= 80 ? 8 : 0);
    o.a = x ? 0 : o.a;
    o.b = y ? 240 : o.b;
    o.w = ws(&o.w, x, y);

    return o;
}

int gc(G *i) {
    int x, y;

    for (x = i->a + 100; x < i->a + 120; ++x) {
        for (y = i->b + 248; y < i->b + 268; ++y) {
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

    L {
        XChangeGC(d, g, GCForeground, A(&t.w, x, y) ? &B : &W);
        if (y < 20) {
            if (x < 20) {
                XDrawPoint(d, p, g, x, y);
            } else if (x < 25) {
                XFillRectangle(d, p, g, 4 * (x - 20), 20 + 4 * y, 4, 4);
            }
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
        L {
            if A(&t.w, x, y) {
                XCopyArea(d, p, b, g, 0, 0, 20, 20, x*20-t.a, y*20-t.b);
            }
        }
        if (t.s % 2) {
            if (gc(&t)) {
                t = gt(&t, 4);
            }
            XCopyArea(d, p, b, g, 0, 20*(t.t / 8 + 1), 20, 20, 100, 248);
        }

        XCopyArea(d, b, w, g, 0, 0, 640, 480, 0, 0);
    }
}
#endif
