#include <climits>
#include <cstdio>
struct date { int y,m,d; };
extern "C" struct date *ref_gdate(int, struct date *);
import pbsd.lib.libcalendar.b0043;
namespace P = pbsd::lib_libcalendar::b0043;
int main() {
    int vals[] = {INT_MIN,-1000000,-10000,-1000,-100,-10,-1,0,1,2,3,4,5,6,7,10,28,29,30,31,59,60,90,100,365,366,1000,10000,577736,577737,577738,577739,730000,1000000,INT_MAX/4,INT_MAX/2,INT_MAX};
    for (int nd : vals) {
        P::date p{}; date r{};
        P::gdate(nd, &p);
        ref_gdate(nd, &r);
        if (p.y!=r.y||p.m!=r.m||p.d!=r.d)
            printf("FAIL nd=%d port %d-%d-%d ref %d-%d-%d\n", nd,p.y,p.m,p.d,r.y,r.m,r.d);
    }
    printf("done\n");
}
