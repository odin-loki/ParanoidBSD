#include <cstdio>
struct date { int y,m,d; };
extern "C" struct date *ref_gdate(int, struct date *);
import pbsd.lib.libcalendar.b0043;
namespace P = pbsd::lib_libcalendar::b0043;
int main() {
    int diffs=0;
    for (int nd = -2000000; nd <= 2147483647; nd += 131071) {
        P::date p{}; date r{};
        P::gdate(nd, &p);
        ref_gdate(nd, &r);
        if (p.y!=r.y||p.m!=r.m||p.d!=r.d) {
            printf("diff nd=%d port %d-%d-%d ref %d-%d-%d\n", nd,p.y,p.m,p.d,r.y,r.m,r.d);
            diffs++;
        }
    }
    printf("diffs=%d\n", diffs);
    return diffs;
}
