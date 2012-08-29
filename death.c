#include <X11/Xlib.h>
#include <string.h>

int main(void) {
    Display *display = XOpenDisplay(NULL);
    int screen       = DefaultScreen(display);
    Window window    = XCreateSimpleWindow(display, RootWindow(display, screen),
        40, 40, 1024, 512, 3, BlackPixel(display, screen), WhitePixel(display, screen));

    enum { unmapped, splash, quit, playing, dead } state = unmapped;

    XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(display, window);

    while (state != quit) {
        XEvent event;
        XNextEvent(display, &event);

        switch (state) {
            case unmapped:
                if (event.type == MapNotify) {
                    state = splash;
                }
                break;
            case splash:
                switch (event.type) {
                    case Expose: {
                        char *splash = "conway's game of DEATH (controls: q, up-arrow, down-arrow)";
                        XDrawString(display, window, DefaultGC(display, screen), 10, 50, splash,
                            strlen(splash));
                        XFlush(display);
                        break;
                    }
                    case KeyPress:
                        XClearWindow(display, window);
                        XFlush(display);
                        state = playing;
                        break;
                };
                break;
            case playing:
                switch (event.type) {
                    case KeyPress: {
                        char *death = "you died.";
                        XClearWindow(display, window);
                        XDrawString(display, window, DefaultGC(display, screen), 10, 50, death,
                            strlen(death));
                        XFlush(display);
                        state = dead;
                        break;
                    }
                };
                break;
            case dead:
                switch (event.type) {
                    case KeyPress:
                        XClearWindow(display, window);
                        XFlush(display);
                        state = quit;
                        break;
                };
                break;
            case quit:
                break;
        };
    }

    XCloseDisplay(display);
    return 0;
}
