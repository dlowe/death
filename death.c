#include   <X11/Xlib.h>
#include <X11/keysym.h>
#include     <stdlib.h>
#include     <unistd.h>
#include       <time.h>
#include      <stdio.h>

#define B(x, y) ((x)*48+(y))
#define C(x, y) (1 << B(x,y) % 8)
#define A(x, y) ((c.c.c[B(x,y) / 8] & C(x,y)) ? 1 : 0)
#define S(b) (b) ? (c.d.c[B(d,e) / 8] |= C(d,e)) : (c.d.c[B(d,e) / 8] &= ~(C(d,e)))
#define L for (d = 0; d < 48; ++d) for (e = 0; e < 48; ++e)
#define R(d) freopen(d, "r", stdin); fread(&c.c, 288, 1, stdin);

int a, b, d, e, f;

struct {
    struct { char c[288]; } c, d;
    int f, g, a, b;
} c;

void h(int i, int s) {
    if (i == s || (i % 2 && s % 2)) {
        f = s;
    } else {
        if (s == 2) {
            R("splash.d")
        } else if (s == 4) {
            R("dead.d")
        } else {
            L S(0);
            c.c = c.d;
        }

        c.f = 1000;
        c.b = 240;
        c.g = 1;
        c.a = 0;
        f = s;
    }
}

int main() {
    Display *D = XOpenDisplay(0);
    int s = DefaultScreen(D);
    Window w   = XCreateSimpleWindow(D, RootWindow(D, s), 40, 40, 640, 480, 3, 0, 0);
    Pixmap u   = XCreatePixmap(D, w, 640, 480, DefaultDepth(D, s)), p = XCreatePixmap(D, w, 20, 100, DefaultDepth(D, s));
    GC g       = DefaultGC(D, s);
    XGCValues W, B;
    long k;

    XSelectInput(D, w, KeyPressMask | KeyReleaseMask);
    XMapWindow(D, w);
    W.foreground = WhitePixel(D, s);
    B.foreground = BlackPixel(D, s);

    R("sprites.d")

    L {
        XChangeGC(D, g, GCForeground, A(d, e) ? &B : &W);
        if (e < 20) {
            if (d < 20) {
                XDrawPoint(D, p, g, d, e);
            } else if (d < 25) {
                XFillRectangle(D, p, g, 4 * (d - 20), 20 + 4 * e, 4, 4);
            }
        }
    }

    srand(time(0));
    h(0, 2);

    while (f) {
        usleep(16666);

        while (XPending(D)) {
            XEvent v;
            XNextEvent(D, &v);
    
            h(f, v.type == KeyPress ? ((k=XLookupKeysym(&v.xkey, 0)) == XK_q ? 0 : (k == XK_Up ? (f == 4 ? f : 3) : (k == XK_Down ? (f == 4 ? f : 5) : (f == 2 ? 1 : (f == 4 ? 2 : f))))) : (v.type == KeyRelease ? ((k=XLookupKeysym(&v.xkey, 0)) == XK_Up ? (f == 3 ? 1 : f) : (f == 5 ? 1 : f)) : f));
        }

        if (! c.g) {
            L {
                for (k = 0, a = -1; a <= 1; ++a) {
                    if (d+a >= 0 && d+a < 48) {
                        for (b = -1; b <= 1; ++b) {
                            if (e+b >= 0 && e+b < 48) {
                                k += (a || b) && A(d+a, e+b);
                            }
                        }
                    }
                }

                S(A(d, e) ? k == 2 || k == 3 : k == 3);
            }
            c.c = c.d;
        }
        c.g = ++c.g % (f % 2 ? 30 : 120);
        c.f += 2;
        c.a += f % 2 ? c.f/1000 : 0;
        c.b += f == 3 ? -2 : (f == 5 ? 2 : 0);

        a = c.a >= 160 ? -8 : 0;
        b = c.b >= 400 ? -8 : (c.b <= 80 ? 8 : 0);
        c.a = a ? 0 : c.a;
        c.b = b ? 240 : c.b;

        L S(((d-a >= 0) && (d-a < 48) && (e-b >= 0) && (e-b < 48)) ? A(d-a, e-b) : ((rand() % 8) == 1));
        c.c = c.d;

        XFillRectangle(D, u, g, 0, 0, 640, 480);
        L {
            if A(d, e) {
                XCopyArea(D, p, u, g, 0, 0, 20, 20, d*20-c.a, e*20-c.b);
            }
        }
        if (f % 2) {
            for (d = c.a + 100; d < c.a + 120; ++d) {
                for (e = c.b + 248; e < c.b + 268; ++e) {
                    if A(d / 20, e / 20) {
                        h(f, 4);
                    }
                }
            }
            XCopyArea(D, p, u, g, 0, 20*(c.g / 8 + 1), 20, 20, 100, 248);
        }

        XCopyArea(D, u, w, g, 0, 0, 640, 480, 0, 0);
    }
    return 0;
}
