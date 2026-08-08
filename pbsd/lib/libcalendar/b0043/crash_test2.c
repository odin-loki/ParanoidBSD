#include <stdio.h>
#include <limits.h>
struct date { int y, m, d; };
extern struct date *ref_gdate(int ndays, struct date *dt);
extern struct date *ref_jdate(int ndays, struct date *dt);
extern int ref_ndaysg(struct date *dt);
extern int ref_ndaysj(struct date *dt);
extern int ref_week(int nd, int *y);
extern int ref_weekday(int nd);
int main(void) {
    struct date dt;
    int y;
    int nds[] = {INT_MIN, -1000000, INT_MAX, INT_MAX/2, INT_MAX/4};
    for (int i = 0; i < 5; i++) {
        printf("gdate %d\n", nds[i]); fflush(stdout);
        ref_gdate(nds[i], &dt);
        printf("  ok\n");
    }
    for (int i = 0; i < 5; i++) {
        printf("jdate %d\n", nds[i]); fflush(stdout);
        ref_jdate(nds[i], &dt);
        printf("  ok\n");
    }
    for (int i = 0; i < 5; i++) {
        printf("week %d\n", nds[i]); fflush(stdout);
        ref_week(nds[i], &y);
        printf("  ok w=%d y=%d\n", y, y);
    }
    for (int i = 0; i < 5; i++) {
        printf("weekday %d\n", nds[i]); fflush(stdout);
        ref_weekday(nds[i]);
        printf("  ok\n");
    }
    return 0;
}
