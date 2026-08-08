#include <stdio.h>
#include <limits.h>
struct date { int y, m, d; };
extern int ref_week(int nd, int *y);
int main(void) {
    int y, vals[] = {INT_MAX, INT_MAX/2, INT_MAX/4, -1000000, 0};
    for (int i = 0; i < 5; i++) {
        int nd = vals[i];
        int w = ref_week(nd, &y);
        printf("week(%d) = %d y=%d\n", nd, w, y);
    }
    return 0;
}
