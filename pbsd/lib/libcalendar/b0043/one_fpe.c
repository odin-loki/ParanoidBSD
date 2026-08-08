#include <stdio.h>
struct date { int y, m, d; };
extern struct date *ref_easterg(int y, struct date *dt);
int main(void) {
    struct date dt;
    printf("calling\n"); fflush(stdout);
    ref_easterg(-99983, &dt);
    printf("ok\n");
    return 0;
}
