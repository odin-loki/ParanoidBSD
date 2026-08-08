#include <stdio.h>
#include <limits.h>
struct date { int y, m, d; };
extern struct date *ref_gdate(int ndays, struct date *dt);
extern int ref_week(int nd, int *y);
int main(void) {
    struct date dt;
    ref_gdate(INT_MIN, &dt);
    printf("gdate(INT_MIN) = %d-%d-%d\n", dt.y, dt.m, dt.d);
    int y = dt.y + 1;
    printf("starting y=%d\n", y);
    for (int i = 0; i < 5; i++) {
        printf("y=%d\n", y);
        y--;
    }
    return 0;
}
