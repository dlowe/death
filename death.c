#include   <X11/Xlib.h>
#include <X11/keysym.h>
#include     <stdlib.h>
#include     <unistd.h>
#include       <time.h>
#include      <stdio.h>

int x, y;

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
    int n, a, b;

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

    L S(&o, x, y, ((x-a >= 0) && (x-a < 48) && (y-b >= 0) && (y-b < 48)) ? A(i, x-a, y-b) : ((rand() % 8) == 1));
    return o;
}

int e(int i, XEvent e) {
    long k;
    return e.type == KeyPress ? ((k=XLookupKeysym(&e.xkey, 0)) == XK_q ? 0 : (k == XK_Up ? (i == 4 ? i : 3) : (k == XK_Down ? (i == 4 ? i : 5) : (i == 2 ? 1 : (i == 4 ? 2 : i))))) : (e.type == KeyRelease ? ((k=XLookupKeysym(&e.xkey, 0)) == XK_Up ? (i == 3 ? 1 : i) : (i == 5 ? 1 : i)) : i);
}

struct {
    M w;
    int s, p, t, a, b;
} G;

#define R(x) freopen(x, "r", stdin); fread(&G.w, 288, 1, stdin);

void gt(int i, int s) {
    if (i == s || (i % 2 && s % 2)) {
        G.s = s;
    } else {
        if (s == 2) {
            R("2.d")
        } else if (s == 4) {
            R("1.d")
        } else {
            L S(&G.w, x, y, 0);
        }

        G.p = 1000;
        G.b = 240;
        G.t = 1;
        G.a = 0;
        G.s = s;
    }
}

#ifndef _TESTING
int main() {
    Display *d = XOpenDisplay(0);
    int s = DefaultScreen(d);
    Window w   = XCreateSimpleWindow(d, RootWindow(d, s), 40, 40, 640, 480, 3, 0, 0);
    Pixmap b   = XCreatePixmap(d, w, 640, 480, DefaultDepth(d, s)), p = XCreatePixmap(d, w, 20, 100, DefaultDepth(d, s));
    GC g       = DefaultGC(d, s);
    XGCValues W, B;

    XSelectInput(d, w, KeyPressMask | KeyReleaseMask);
    XMapWindow(d, w);
    W.foreground = WhitePixel(d, s);
    B.foreground = BlackPixel(d, s);

    R("0.d")

    L {
        XChangeGC(d, g, GCForeground, A(&G.w, x, y) ? &B : &W);
        if (y < 20) {
            if (x < 20) {
                XDrawPoint(d, p, g, x, y);
            } else if (x < 25) {
                XFillRectangle(d, p, g, 4 * (x - 20), 20 + 4 * y, 4, 4);
            }
        }
    }

    srand(time(0));
    gt(0, 2);

    while (G.s) {
        usleep(16666);

        while (XPending(d)) {
            XEvent v;
            XNextEvent(d, &v);
    
            gt(G.s, e(G.s, v));
        }

        if (! G.t) {
            G.w = wt(&G.w);
        }
        G.t = ++G.t % (G.s % 2 ? 30 : 120);
        G.p += 2;
        G.a += G.s % 2 ? G.p/1000 : 0;
        G.b += G.s == 3 ? -2 : (G.s == 5 ? 2 : 0);

        x = G.a >= 160 ? -8 : 0;
        y = G.b >= 400 ? -8 : (G.b <= 80 ? 8 : 0);
        G.a = x ? 0 : G.a;
        G.b = y ? 240 : G.b;
        G.w = ws(&G.w, x, y);

        XFillRectangle(d, b, g, 0, 0, 640, 480);
        L {
            if A(&G.w, x, y) {
                XCopyArea(d, p, b, g, 0, 0, 20, 20, x*20-G.a, y*20-G.b);
            }
        }
        if (G.s % 2) {
            for (x = G.a + 100; x < G.a + 120; ++x) {
                for (y = G.b + 248; y < G.b + 268; ++y) {
                    if A(&G.w, x / 20, y / 20) {
                        gt(G.s, 4);
                    }
                }
            }
            XCopyArea(d, p, b, g, 0, 20*(G.t / 8 + 1), 20, 20, 100, 248);
        }

        XCopyArea(d, b, w, g, 0, 0, 640, 480, 0, 0);
    }
}
#endif
