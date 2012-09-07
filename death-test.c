#include <check.h>
#include <stdio.h>

#define _TESTING
#include "death.c"

#define DIM                 48
#define CELL_SIZE           20
#define PLAYER_LOOP         4
#define PLAYER_RATE         5
#define PLAYER_LEFT         100
#define PLAYER_TOP          248
#define FRAME_RATE          60
#define LIFE_RATE           2
#define CONTROL_SENSITIVITY 2
#define SPEED_START         1
#define SPEED_ZOOM          0.002
#define WINDOW_WIDTH        640
#define WINDOW_HEIGHT       480

void print_M(M *w, short width, short height) {
    int x, y;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            printf("%c", A(w, x, y) ? 'O' : '_');
        }
        printf("\n");
    }
}

M str_to_M(short width, char *in) {
    int x, y;
    M out;

    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            S(&out, x, y, 0);
        }
    }

    y = 0;
    while (in[width * y]) {
        for (x = 0; x < width; ++x) {
            S(&out, x, y, in[width * y + x] != '_');
        }
        ++y;
    }
    return out;
}

#define quit 0
#define playing_nil 1
#define splash 2
#define playing_up 3
#define dead 4
#define playing_down 5

#define state_playing(s) ((s) % 2)

START_TEST (test_event_handler)
{
    int type;
    int in;
    XEvent event;
    Display *display = XOpenDisplay(NULL);

    /* ignore all events other than KeyPress and KeyRelease */
    for (type = 0; type <= LASTEvent; ++type) {
        if ((type == KeyPress) || (type == KeyRelease)) {
            continue;
        }
        event.type = type;
        for (in = quit; in <= playing_down; ++in) {
            fail_unless(e(in, event) == in, "ignore event");
        }
    }


    /* pressing q from any state results in quit */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_q);
    for (in = quit; in <= playing_down; ++in) {
        fail_unless(e(in, event) == quit, "quit on q");
    }


    /* pressing up... */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Up);
    /* ... from any non-dead state results in playing_up */
    for (in = quit; in <= playing_down; ++in) {
        if (in != dead) {
            fail_unless(e(in, event) == playing_up, "*+up => up");
        }
    }
    /* ... but from dead, results in splash */
    fail_unless(e(dead, event) == dead, "dead+up => dead");


    /* pressing down... */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Down);
    /* ... from any non-dead state results in playing_down */
    for (in = quit; in <= playing_down; ++in) {
        if (in != dead) {
            fail_unless(e(in, event) == playing_down, "*+down => down");
        }
    }
    /* ... but from dead, results in splash */
    fail_unless(e(dead, event) == dead, "dead+down => dead");


    /* pressing any other key... */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_a);
    /* ... is a noop when playing */
    for (in = quit; in <= playing_down; ++in) {
        if (state_playing(in)) {
            fail_unless(e(in, event) == in, "%d+a => %d", in, e(in, event));
        }
    }
    /* ... splash -> playing_nil */
    fail_unless(e(splash, event) == playing_nil, "splash+a => playing_nil");
    /* ... dead -> splash */
    fail_unless(e(dead, event) == splash, "dead+a => splash");

    /* releasing up... */
    event.type         = KeyRelease;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Up);
    /* ... is ignored for everything but playing_up */
    for (in = quit; in <= playing_down; ++in) {
        if (in != playing_up) {
            fail_unless(e(in, event) == in, "*+!up => *");
        }
    }
    /* ... playing_up -> playing_nil */
    fail_unless(e(playing_up, event) == playing_nil, "up+!up => nil");

    /* releasing down... */
    event.type         = KeyRelease;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Down);
    /* ... is ignored for everything but playing_down */
    for (in = quit; in <= playing_down; ++in) {
        if (in != playing_down) {
            fail_unless(e(in, event) == in, "*+!down => *");
        }
    }
    /* ... playing_down -> playing_nil */
    fail_unless(e(playing_down, event) == playing_nil, "down+!down => nil");
}
END_TEST

START_TEST (test_state_playing)
{
    fail_unless(state_playing(splash)       == 0, "!splash");
    fail_unless(state_playing(quit)         == 0, "!quit");
    fail_unless(state_playing(dead)         == 0, "!dead");
    fail_unless(state_playing(playing_nil)  == 1, "nil");
    fail_unless(state_playing(playing_up)   == 1, "up");
    fail_unless(state_playing(playing_down) == 1, "down");
}
END_TEST

short basic_M_assertions(M *in) {
    int x, y;
    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            short alive = A(in, x, y);

            if (! ((alive == 0) || (alive == 1))) {
                return 0;
            }

        }
    }
    return 1;
}

short Ms_are_equal(M *w1, M *w2) {
    int x, y;
    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            if (A(w1, x, y) != A(w2, x, y)) {
                return 0;
            }
        }
    }
    return 1;
}

START_TEST (test_game_new)
{
    G g = gt(NULL, splash);
    fail_unless(basic_M_assertions(&g.w), "splash basics");

    char *splash_s =
            "______________________________"
            "__OO_____________________O____"
            "_O____O__OO__O_O_OO__O_O_O__OO"
            "_O___O_O_O_O_OOO__OO_O_O___OO_"
            "_O___O_O_O_O_OOO_O_O__OO____OO"
            "__OO__O__O_O_OOO_OOO___O___OO_"
            "______________________O_______"
            "__OO_____________________O____"
            "_O___OO__OOO__OO____O___O_____"
            "_OOO__OO_OOO_O_O___O_O_OOO____"
            "_O_O_O_O_OOO_OO____O_O__O_____"
            "__OO_OOO_O_O__OO____O___O_____"
            "______________________________"
            "_OO__OOO__O__OOO_O_O__________"
            "_O_O_O___O_O__O__O_O__________"
            "_O_O_OOO_OOO__O__OOO__________"
            "_O_O_O___O_O__O__O_O__________"
            "_OO__OOO_O_O__O__O_O__________";
    M w = str_to_M(30, splash_s);

    fail_unless(g.a == 0, "dx is 0");
    fail_unless(g.b == 0, "dy is 0");
    fail_unless(Ms_are_equal(&g.w, &w), "splash rendered");

    g = gt(NULL, dead);
    fail_unless(basic_M_assertions(&g.w), "dead basics");

    char *dead_s =
            "______________________"
            "______________________"
            "______________________"
            "______________________"
            "________OO__O__O_O_OOO"
            "_______O___O_O_OOO_O__"
            "_______OOO_OOO_OOO_OOO"
            "_______O_O_O_O_O_O_O__"
            "________OO_O_O_O_O_OOO"
            "______________________"
            "________O__O_O_OOO_OO_"
            "_______O_O_O_O_O___O_O"
            "_______O_O_O_O_OOO_OOO"
            "_______O_O_O_O_O___OO_"
            "________O___O__OOO_O_O";
    w = str_to_M(22, dead_s);
    fail_unless(Ms_are_equal(&g.w, &w), "dead rendered");
}
END_TEST

START_TEST (test_wt_block)
{
    char *block =
        "____"
        "_OO_"
        "_OO_"
        "____";

    M w0 = str_to_M(4, block);
    M w1 = wt(&w0);
    fail_unless(Ms_are_equal(&w0, &w1), "block step 1");
}
END_TEST

START_TEST (test_wt_beehive)
{
    char *beehive =
        "______"
        "__OO__"
        "_O__O_"
        "__OO__"
        "______";
    M w0 = str_to_M(6, beehive);
    M w1 = wt(&w0);

    fail_unless(Ms_are_equal(&w0, &w1), "beehive step 1");
}
END_TEST

START_TEST (test_wt_blinker)
{
    char *blinker0 =
        "_____"
        "__O__"
        "__O__"
        "__O__"
        "_____";

    char *blinker1 =
        "_____"
        "_____"
        "_OOO_"
        "_____"
        "_____";

    M w0 = str_to_M(5, blinker0);
    M w1_actual = wt(&w0);
    M w2_actual = wt(&w1_actual);
    M w1_expected = str_to_M(5, blinker1);

    fail_unless(Ms_are_equal(&w1_actual, &w1_expected), "blinker step 1");
    fail_unless(Ms_are_equal(&w2_actual, &w0), "blinker step 2");
}
END_TEST

START_TEST (test_wt_glider)
{
    char *glider0 =
        "________"
        "__O_____"
        "___O____"
        "_OOO____"
        "________";

    char *glider1 =
        "________"
        "________"
        "_O_O____"
        "__OO____"
        "__O_____";

    char *glider2 =
        "________"
        "________"
        "___O____"
        "_O_O____"
        "__OO____";

    char *glider3 =
        "________"
        "________"
        "__O_____"
        "___OO___"
        "__OO____";

    char *glider4 =
        "________"
        "________"
        "___O____"
        "____O___"
        "__OOO___";

    M w0 = str_to_M(8, glider0);
    M w1_actual = wt(&w0);
    M w2_actual = wt(&w1_actual);
    M w3_actual = wt(&w2_actual);
    M w4_actual = wt(&w3_actual);
    M w1_expected = str_to_M(8, glider1);
    M w2_expected = str_to_M(8, glider2);
    M w3_expected = str_to_M(8, glider3);
    M w4_expected = str_to_M(8, glider4);

    fail_unless(Ms_are_equal(&w1_actual, &w1_expected), "glider step 1");
    fail_unless(Ms_are_equal(&w2_actual, &w2_expected), "glider step 2");
    fail_unless(Ms_are_equal(&w3_actual, &w3_expected), "glider step 3");
    fail_unless(Ms_are_equal(&w4_actual, &w4_expected), "glider step 4");
}
END_TEST

START_TEST (test_ws)
{
    short ox = 3, oy = 3;
    char *t =
        "_______"
        "_______"
        "_______"
        "___O___"
        "_______"
        "_______"
        "_______";

    M w0 = str_to_M(7, t);
    for (short dx = -2; dx <= 2; ++dx) {
        for (short dy = -2; dy <= 2; ++dy) {
            M w1 = ws(&w0, dx, dy);
            fail_unless(basic_M_assertions(&w1), "slide basics");
            fail_unless(A(&w1, ox+dx, oy+dy), "slide correct cell alive");
            for (short tx = -1; tx <= 1; ++tx) {
                for (short ty = -1; ty <= 1; ++ty) {
                    if (tx || ty) {
                        fail_unless(! A(&w1, ox+dx+tx, oy+dy+ty), "slide correct cell dead");
                    }
                }
            }
        }
    }
}
END_TEST

START_TEST (test_gi)
{
    int i;
    G g = gt(NULL, playing_nil);
    for (int x = 0; x < DIM; ++x) {
        for (int y = 0; y < DIM; ++y) {
            S(&g.w, x, y, (rand() % 8) == 1);
        }
    }
    for (i = 0; i < 30; ++i) {
        G gnext = gi(&g);
        if (g.t == 0) {
            /* step, so Ms should no longer be equal */
            fail_unless(! Ms_are_equal(&g.w, &gnext.w), "unequal after step");
        } else {
            /* no step, so Ms should be equal */
            fail_unless(Ms_are_equal(&g.w, &gnext.w), "equal after !step");
        }
        fail_unless(gnext.p == g.p + 2, "speed increased");
        fail_unless(gnext.b == g.b, "dy constant");
        fail_unless(gnext.a == (int)(g.a + gnext.p/1000), "dx increased");
        fail_unless(gnext.s == g.s, "state constant");
        g = gnext;
    }
}
END_TEST

int main(void) {
    TCase *tc;
    Suite *suite;
    SRunner *sr;
    int n_failed;
    
    tc = tcase_create("death");
    tcase_add_test(tc, test_event_handler);
    tcase_add_test(tc, test_state_playing);
    tcase_add_test(tc, test_wt_block);
    tcase_add_test(tc, test_wt_beehive);
    tcase_add_test(tc, test_wt_blinker);
    tcase_add_test(tc, test_wt_glider);
    tcase_add_test(tc, test_ws);
    tcase_add_test(tc, test_game_new);
    tcase_add_test(tc, test_gi);

    suite = suite_create("death");
    suite_add_tcase(suite, tc);

    sr = srunner_create(suite);

    srunner_run_all(sr, CK_NORMAL);
    n_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return n_failed ? 1 : 0;
}
