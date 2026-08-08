#include <stdio.h>
#include <limits.h>
struct date { int y, m, d; };
extern struct date *ref_easterg(int y, struct date *dt);
int main(void) {
    struct date dt;
    int years[] = {INT_MIN, -1, 0, 1, INT_MAX};
    for (int i = 0; i < 5; i++) {
        printf("testing %d\n", years[i]);
        fflush(stdout);
        ref_easterg(years[i], &dt);
        printf("  ok: %d-%d-%d\n", dt.y, dt.m, dt.d);
    }
    return 0;
}
