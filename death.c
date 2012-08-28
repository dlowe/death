#include <X11/Xlib.h>

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
                    case Expose:
                        /* initial draw */
                        break;
                    case KeyPress:
                        state = quit;
                        break;
                };
            case playing:
                break;
            case dead:
                break;
            case quit:
                break;
        };
    }

    XCloseDisplay(display);
    return 0;
}
