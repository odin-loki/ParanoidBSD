#include <climits>
#include <cstdio>
struct date { int y, m, d; };
namespace {
// paste only gdate and its static deps - too heavy
}
extern "C" struct date *ref_gdate(int ndays, struct date *dt);
// compile port_plain with renamed gdate -> cpp_gdate
