#include <cstdio>
struct date { int y, m, d; };
int main() { printf("size=%zu align=%zu\n", sizeof(date), alignof(date)); }
