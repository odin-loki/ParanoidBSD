#define complex _Complex
#include <complex.h>
#include <cstdio>
int main() {
  long double _Complex z;
  __real__ z = 1.0L; __imag__ z = 2.0L;
  unsigned char *p = (unsigned char*)&z;
  std::printf("sizeof(ld)=%zu sizeof(cx)=%zu\n", sizeof(long double), sizeof(long double _Complex));
  for (size_t i=0;i<sizeof(z);i++) std::printf("%02x", p[i]);
  std::printf("\n");
}
