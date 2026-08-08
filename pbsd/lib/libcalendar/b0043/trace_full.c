#include <stdio.h>
#include <limits.h>
struct date { int y, m, d; };
extern struct date *ref_gdate(int ndays, struct date *dt);
extern int ref_ndaysg(struct date *dt);
extern int ref_weekday(int nd);
int main(void) {
    struct date dt;
    /* simulate harness warmup */
    for (int i = 0; i < 1000; i++) ref_weekday(i);
    ref_gdate(INT_MAX, &dt);
    printf("after warmup: %d-%d-%d\n", dt.y, dt.m, dt.d);
    ref_gdate(INT_MAX, &dt);
    printf("second: %d-%d-%d\n", dt.y, dt.m, dt.d);
    return 0;
}
