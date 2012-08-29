#include <check.h>

#define _TESTING
#include "death.c"

START_TEST (test_event_handler)
{
    int type;
    state in;
    XEvent event;
    Display *display = XOpenDisplay(NULL);

    /* ignore all events other than KeyPress and KeyRelease */
    for (type = 0; type <= LASTEvent; ++type) {
        if ((type == KeyPress) || (type == KeyRelease)) {
            continue;
        }
        event.type = type;
        for (in = splash; in <= dead; ++in) {
            fail_unless(event_handler(in, event) == in);
        }
    }


    /* pressing q from any state results in quit */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_q);
    for (in = splash; in <= dead; ++in) {
        fail_unless(event_handler(in, event) == quit);
    }


    /* pressing up... */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Up);
    /* ... from any non-dead state results in playing_up */
    for (in = splash; in < dead; ++in) {
        fail_unless(event_handler(in, event) == playing_up);
    }
    /* ... but from dead, results in splash */
    fail_unless(event_handler(dead, event) == splash);


    /* pressing down... */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Down);
    /* ... from any non-dead state results in playing_down */
    for (in = splash; in < dead; ++in) {
        fail_unless(event_handler(in, event) == playing_down);
    }
    /* ... but from dead, results in splash */
    fail_unless(event_handler(dead, event) == splash);


    /* pressing any other key... */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_a);
    /* ... is a noop when playing */
    for (in = playing_nil; in <= playing_down; ++in) {
        fail_unless(event_handler(in, event) == in);
    }
    /* ... splash -> playing_nil */
    fail_unless(event_handler(splash, event) == playing_nil);
    /* ... dead -> splash */
    fail_unless(event_handler(dead, event) == splash);

    /* releasing up... */
    event.type         = KeyRelease;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Up);
    /* ... is ignored for everything but playing_up */
    for (in = splash; in <= dead; ++in) {
        if (in != playing_up) {
            fail_unless(event_handler(in, event) == in);
        }
    }
    /* ... playing_up -> playing_nil */
    fail_unless(event_handler(playing_up, event) == playing_nil);

    /* releasing down... */
    event.type         = KeyRelease;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Down);
    /* ... is ignored for everything but playing_down */
    for (in = splash; in <= dead; ++in) {
        if (in != playing_down) {
            fail_unless(event_handler(in, event) == in);
        }
    }
    /* ... playing_down -> playing_nil */
    fail_unless(event_handler(playing_down, event) == playing_nil);
}
END_TEST

int main(void) {
    TCase *tc;
    Suite *suite;
    SRunner *sr;
    int n_failed;
    
    tc = tcase_create("death");
    tcase_add_test(tc, test_event_handler);

    suite = suite_create("death");
    suite_add_tcase(suite, tc);

    sr = srunner_create(suite);

    srunner_run_all(sr, CK_NORMAL);
    n_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return n_failed ? 1 : 0;
}
