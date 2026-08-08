#include <cstdio>
extern "C" long double ref_exp2l(long double);
int main() {
    std::printf("before\n");
    long double r = ref_exp2l(1.0L);
    std::printf("ref=%La\n", r);
    return 0;
}
