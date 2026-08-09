#define _GNU_SOURCE
#include <cstdio>
import pbsd.lib.libc.stdio.b0104;
int main() {
  FILE *fp = tmpfile();
  int r = pbsd::lib_libc_stdio::b0104::fprintf(fp, "%d", 42);
  std::printf("r=%d\n", r);
  return 0;
}
