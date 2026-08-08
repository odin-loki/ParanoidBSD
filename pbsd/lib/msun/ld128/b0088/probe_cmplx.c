#define _GNU_SOURCE
#include <stdio.h>
#define complex _Complex
#include <complex.h>
int main(void) {
  long double complex z = CMPLXL(1.0L, 2.0L);
  unsigned char *p = (unsigned char*)&z;
  for (int i=0;i<32;i++) printf("%02x", p[i]);
  printf("\n");
}
