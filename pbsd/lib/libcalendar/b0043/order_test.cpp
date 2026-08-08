#include <climits>
#include <cstdio>
struct c_date { int y, m, d; };
extern "C" {
struct date { int y, m, d; };
struct date *ref_gdate(int ndays, struct date *dt);
}
import pbsd.lib.libcalendar.b0043;
namespace P = pbsd::lib_libcalendar::b0043;
int main() {
    c_date rd = {0,0,0};
    P::date pd = {0,0,0};
    ref_gdate(INT_MAX, reinterpret_cast<date*>(&rd));
    printf("ref before port: %d-%d-%d\n", rd.y, rd.m, rd.d);
    P::gdate(INT_MAX, &pd);
    printf("port: %d-%d-%d\n", pd.y, pd.m, pd.d);
    ref_gdate(INT_MAX, reinterpret_cast<date*>(&rd));
    printf("ref after port: %d-%d-%d\n", rd.y, rd.m, rd.d);
    return 0;
}
