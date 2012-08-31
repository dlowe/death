#include <check.h>
#include <stdio.h>

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

START_TEST (test_state_playing)
{
    fail_unless(state_playing(splash)       == 0);
    fail_unless(state_playing(quit)         == 0);
    fail_unless(state_playing(dead)         == 0);
    fail_unless(state_playing(playing_nil)  == 1);
    fail_unless(state_playing(playing_up)   == 1);
    fail_unless(state_playing(playing_down) == 1);
}
END_TEST

short basic_world_assertions(world *in) {
    int x, y;
    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            short alive = world_cell_alive(in, x, y);
            short n = world_cell_living_neighbors(in, x, y);

            if (! ((alive == 0) || (alive == 1))) {
                return 0;
            }

            if ((n < 0) || (n > 8)) {
                return 0;
            }
        }
    }
    return 1;
}

short worlds_are_equal(world *w1, world *w2) {
    int x, y;
    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            if (world_cell_alive(w1, x, y) != world_cell_alive(w2, x, y)) {
                return 0;
            }
        }
    }
    return 1;
}

START_TEST (test_game_new)
{
    game g = game_transition(NULL, splash);
    fail_unless(basic_world_assertions(&g.w));

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
    world w = str_to_world(30, splash_s);

    fail_unless(g.dx == 0);
    fail_unless(g.dy == 0);
    fail_unless(worlds_are_equal(&g.w, &w));

    g = game_transition(NULL, dead);
    fail_unless(basic_world_assertions(&g.w));

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
    w = str_to_world(22, dead_s);
    fail_unless(worlds_are_equal(&g.w, &w));
}
END_TEST

void print_world(world *w, short width, short height) {
    int x, y;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            printf("%c", world_cell_alive(w, x, y) ? 'O' : '_');
        }
        printf("\n");
    }
}

START_TEST (test_world_step_block)
{
    char *block =
        "____"
        "_OO_"
        "_OO_"
        "____";

    world w0 = str_to_world(4, block);
    world w1 = world_step(&w0);
    fail_unless(worlds_are_equal(&w0, &w1));
}
END_TEST

START_TEST (test_world_step_beehive)
{
    char *beehive =
        "______"
        "__OO__"
        "_O__O_"
        "__OO__"
        "______";
    world w0 = str_to_world(6, beehive);
    world w1 = world_step(&w0);

    fail_unless(worlds_are_equal(&w0, &w1));
}
END_TEST

START_TEST (test_world_step_blinker)
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

    world w0 = str_to_world(5, blinker0);
    world w1_actual = world_step(&w0);
    world w2_actual = world_step(&w1_actual);
    world w1_expected = str_to_world(5, blinker1);

    fail_unless(worlds_are_equal(&w1_actual, &w1_expected));
    fail_unless(worlds_are_equal(&w2_actual, &w0));
}
END_TEST

START_TEST (test_world_step_glider)
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

    world w0 = str_to_world(8, glider0);
    world w1_actual = world_step(&w0);
    world w2_actual = world_step(&w1_actual);
    world w3_actual = world_step(&w2_actual);
    world w4_actual = world_step(&w3_actual);
    world w1_expected = str_to_world(8, glider1);
    world w2_expected = str_to_world(8, glider2);
    world w3_expected = str_to_world(8, glider3);
    world w4_expected = str_to_world(8, glider4);

    fail_unless(worlds_are_equal(&w1_actual, &w1_expected));
    fail_unless(worlds_are_equal(&w2_actual, &w2_expected));
    fail_unless(worlds_are_equal(&w3_actual, &w3_expected));
    fail_unless(worlds_are_equal(&w4_actual, &w4_expected));
}
END_TEST

START_TEST (test_game_tick)
{
    int i;
    game g = game_transition(NULL, playing_nil);
    for (i = 0; i < 1200; ++i) {
        game gnext = game_tick(&g);
        if (g.tick == 0) {
            /* step, so worlds should no longer be equal */
            fail_unless(! worlds_are_equal(&g.w, &gnext.w));
        } else {
            /* no step, so worlds should be equal */
            fail_unless(worlds_are_equal(&g.w, &gnext.w));
        }
        fail_unless((gnext.speed - (g.speed + SPEED_ZOOM)) < 0.00001);
        fail_unless(gnext.dy == g.dy);
        fail_unless(gnext.dx == (int)(g.dx + gnext.speed));
        fail_unless(gnext.s == g.s);
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
    tcase_add_test(tc, test_world_step_block);
    tcase_add_test(tc, test_world_step_beehive);
    tcase_add_test(tc, test_world_step_blinker);
    tcase_add_test(tc, test_world_step_glider);
    tcase_add_test(tc, test_game_new);
    tcase_add_test(tc, test_game_tick);

    suite = suite_create("death");
    suite_add_tcase(suite, tc);

    sr = srunner_create(suite);

    srunner_run_all(sr, CK_NORMAL);
    n_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return n_failed ? 1 : 0;
}
