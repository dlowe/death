#include <stdio.h>

#define _TESTING
#include "death.c"

#define DIM 48

world str_to_world(short width, char *in) {
    int x, y;
    world out;

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

int main(void) {
    {
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

        world splash_world = str_to_world(30, splash_s);
        FILE *f = fopen("2.d", "w");
        fwrite(&splash_world, sizeof(world), 1, f);
        fclose(f);
    }

    {
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
        world dead_world = str_to_world(22, dead_s);
        FILE *f = fopen("1.d", "w");
        fwrite(&dead_world, sizeof(world), 1, f);
        fclose(f);
    }

    {
        char *cell_s =
            "_OOOOOOOOOOOOOOOOOO_" "O__O_"
            "OOOOOOOOOOOOOOOOOOOO" "____O"
            "OOOOOOOOOOOOOOOOOOOO" "O___O"
            "OOOOOOOOOOOOOOOOOOOO" "_OOOO"
            "OOOOOOOOOOOOOOOOOOOO" "_____"
            "OOOOOOOOOOOOOOOOOOOO" "_____"
            "OOOOOOOOOOOOOOOOOOOO" "__OO_"
            "OOOOOOOOOOOOOOOOOOOO" "OO_OO"
            "OOOOOOOOOOOOOOOOOOOO" "OOOO_"
            "OOOOOOOOOOOOOOOOOOOO" "_OO__"
            "OOOOOOOOOOOOOOOOOOOO" "_____"
            "OOOOOOOOOOOOOOOOOOOO" "_OOOO"
            "OOOOOOOOOOOOOOOOOOOO" "O___O"
            "OOOOOOOOOOOOOOOOOOOO" "____O"
            "OOOOOOOOOOOOOOOOOOOO" "O__O_"
            "OOOOOOOOOOOOOOOOOOOO" "_OO__"
            "OOOOOOOOOOOOOOOOOOOO" "OOOO_"
            "OOOOOOOOOOOOOOOOOOOO" "OO_OO"
            "OOOOOOOOOOOOOOOOOOOO" "__OO_"
            "_OOOOOOOOOOOOOOOOOO_" "_____";

        world cell_world = str_to_world(25, cell_s);
        FILE *f = fopen("0.d", "w");
        fwrite(&cell_world, sizeof(world), 1, f);
        fclose(f);
    }
}
