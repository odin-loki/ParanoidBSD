#include <climits>
#include <cstdio>
struct date { int y,m,d; };
extern "C" struct date *ref_jdate(int, struct date *);
import pbsd.lib.libcalendar.b0043;
namespace P = pbsd::lib_libcalendar::b0043;
int main() {
    P::date p{}; date r{};
    P::jdate(INT_MAX, &p);
    ref_jdate(INT_MAX, &r);
    printf("port %d-%d-%d ref %d-%d-%d\n", p.y,p.m,p.d,r.y,r.m,r.d);
}
