#include <climits>
#include <cstdio>
struct date { int y, m, d; };
extern "C" struct date *ref_gdate(int ndays, struct date *dt);
extern "C" struct date *cpp_gdate(int ndays, struct date *dt);
int main() {
    int diffs = 0, tests = 0;
    for (int nd = -100000; nd <= 100000; nd++) {
        date r={}, c={};
        ref_gdate(nd, &r);
        cpp_gdate(nd, &c);
        tests++;
        if (r.y!=c.y||r.m!=c.m||r.d!=c.d) {
            if (diffs < 10) printf("diff nd=%d ref %d-%d-%d cpp %d-%d-%d\n", nd,r.y,r.m,r.d,c.y,c.m,c.d);
            diffs++;
        }
    }
    printf("tests=%d diffs=%d\n", tests, diffs);
    return diffs ? 1 : 0;
}
