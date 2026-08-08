#include <stdio.h>
#include <limits.h>
struct date { int y, m, d; };
extern struct date *ref_gdate(int ndays, struct date *dt);
int main(void) {
    struct date dt = {0x44444444, 0x55555555, 0x66666666};
    ref_gdate(INT_MAX, &dt);
    printf("ref %d-%d-%d\n", dt.y, dt.m, dt.d);
    return 0;
}
