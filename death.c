#include   <X11/Xlib.h>
#include <X11/keysym.h>
#include     <stdlib.h>
#include     <unistd.h>
#include       <time.h>
#include      <stdio.h>

int x, y, a, b;

#define B(x, y) ((x)*48+(y))
#define C(x, y) (1 << B(x,y) % 8)
#define A(x, y) ((c.c.c[B(x,y) / 8] & C(x,y)) ? 1 : 0)
#define S(b) (b) ? (c.d.c[B(x,y) / 8] |= C(x,y)) : (c.d.c[B(x,y) / 8] &= ~(C(x,y)))
#define L for (x = 0; x < 48; ++x) for (y = 0; y < 48; ++y)

struct {
    struct { char c[288]; } c, d;
    int s, p, t, a, b;
} c;

#define R(x) freopen(x, "r", stdin); fread(&c.c, 288, 1, stdin);

void gt(int i, int s) {
    if (i == s || (i % 2 && s % 2)) {
        c.s = s;
    } else {
        if (s == 2) {
            R("2.d")
        } else if (s == 4) {
            R("1.d")
        } else {
            L S(0);
            c.c = c.d;
        }

        c.p = 1000;
        c.b = 240;
        c.t = 1;
        c.a = 0;
        c.s = s;
    }
}

int main() {
    Display *d = XOpenDisplay(0);
    int s = DefaultScreen(d);
    Window w   = XCreateSimpleWindow(d, RootWindow(d, s), 40, 40, 640, 480, 3, 0, 0);
    Pixmap u   = XCreatePixmap(d, w, 640, 480, DefaultDepth(d, s)), p = XCreatePixmap(d, w, 20, 100, DefaultDepth(d, s));
    GC g       = DefaultGC(d, s);
    XGCValues W, B;

    XSelectInput(d, w, KeyPressMask | KeyReleaseMask);
    XMapWindow(d, w);
    W.foreground = WhitePixel(d, s);
    B.foreground = BlackPixel(d, s);

    R("0.d")

    L {
        XChangeGC(d, g, GCForeground, A(x, y) ? &B : &W);
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

    while (c.s) {
        usleep(16666);

        while (XPending(d)) {
            long k;
            XEvent v;
            XNextEvent(d, &v);
    
            gt(c.s, v.type == KeyPress ? ((k=XLookupKeysym(&v.xkey, 0)) == XK_q ? 0 : (k == XK_Up ? (c.s == 4 ? c.s : 3) : (k == XK_Down ? (c.s == 4 ? c.s : 5) : (c.s == 2 ? 1 : (c.s == 4 ? 2 : c.s))))) : (v.type == KeyRelease ? ((k=XLookupKeysym(&v.xkey, 0)) == XK_Up ? (c.s == 3 ? 1 : c.s) : (c.s == 5 ? 1 : c.s)) : c.s));
        }

        if (! c.t) {
            int n;
            L {
                for (n = 0, a = -1; a <= 1; ++a) {
                    if (x+a >= 0 && x+a < 48) {
                        for (b = -1; b <= 1; ++b) {
                            if (y+b >= 0 && y+b < 48) {
                                n += (a || b) && A(x+a, y+b);
                            }
                        }
                    }
                }

                S(A(x, y) ? n == 2 || n == 3 : n == 3);
            }
            c.c = c.d;
        }
        c.t = ++c.t % (c.s % 2 ? 30 : 120);
        c.p += 2;
        c.a += c.s % 2 ? c.p/1000 : 0;
        c.b += c.s == 3 ? -2 : (c.s == 5 ? 2 : 0);

        a = c.a >= 160 ? -8 : 0;
        b = c.b >= 400 ? -8 : (c.b <= 80 ? 8 : 0);
        c.a = a ? 0 : c.a;
        c.b = b ? 240 : c.b;

        L S(((x-a >= 0) && (x-a < 48) && (y-b >= 0) && (y-b < 48)) ? A(x-a, y-b) : ((rand() % 8) == 1));
        c.c = c.d;

        XFillRectangle(d, u, g, 0, 0, 640, 480);
        L {
            if A(x, y) {
                XCopyArea(d, p, u, g, 0, 0, 20, 20, x*20-c.a, y*20-c.b);
            }
        }
        if (c.s % 2) {
            for (x = c.a + 100; x < c.a + 120; ++x) {
                for (y = c.b + 248; y < c.b + 268; ++y) {
                    if A(x / 20, y / 20) {
                        gt(c.s, 4);
                    }
                }
            }
            XCopyArea(d, p, u, g, 0, 20*(c.t / 8 + 1), 20, 20, 100, 248);
        }

        XCopyArea(d, u, w, g, 0, 0, 640, 480, 0, 0);
    }
}
