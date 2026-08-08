#include <climits>
#include <cstdio>
struct c_date { int y, m, d; };
extern "C" {
struct date { int y, m, d; };
struct date *ref_gdate(int ndays, struct date *dt);
}
import pbsd.lib.libcalendar.b0043;
namespace P = pbsd::lib_libcalendar::b0043;

static void run_once(const char *label) {
    P::date pd = {0x44444444, 0x55555555, 0x66666666};
    c_date rd = {0x44444444, 0x55555555, 0x66666666};
    P::gdate(INT_MAX, &pd);
    ref_gdate(INT_MAX, reinterpret_cast<struct date*>(&rd));
    printf("%s port %d-%d-%d ref %d-%d-%d %s\n", label,
        pd.y, pd.m, pd.d, rd.y, rd.m, rd.d,
        (pd.y==rd.y && pd.m==rd.m && pd.d==rd.d) ? "MATCH" : "DIFF");
}

int main() {
    run_once("first");
    run_once("second");
    return 0;
}
