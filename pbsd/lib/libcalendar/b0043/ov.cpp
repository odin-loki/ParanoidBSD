#include <cstdio>
int f(int y) { return y * 365 + y / 4; }
int main() { printf("%d\n", f(5883516)); return 0; }
