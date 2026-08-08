#include <climits>
#include <cstdio>
struct date { int y, m, d; };
extern "C" struct date *ref_gdate(int ndays, struct date *dt);
extern "C" struct date *cpp_gdate(int ndays, struct date *dt);
int main() {
    date r = {}, c = {};
    ref_gdate(INT_MAX, &r);
    cpp_gdate(INT_MAX, &c);
    printf("ref %d-%d-%d\n", r.y, r.m, r.d);
    printf("cpp %d-%d-%d\n", c.y, c.m, c.d);
    return (r.y!=c.y||r.m!=c.m||r.d!=c.d);
}
