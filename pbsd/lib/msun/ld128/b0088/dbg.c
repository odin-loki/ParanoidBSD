#define _GNU_SOURCE
#define complex _Complex
#include <complex.h>
#include <stdio.h>
#include <string.h>
long double complex ref_cexpl(long double complex);
static void dump(const char*t, long double x) {
  unsigned char b[16]; memcpy(b,&x,16);
  printf("%s=",t); for(int i=15;i>=0;i--) printf("%02x",b[i]); printf("\n");
}
int main() {
  long double complex z = CMPLXL(1.0L/0.0L, 0.0L);
  long double complex r1 = ref_cexpl(z);
  long double complex r2 = ref_cexpl(z);
  dump("r1_re", creall(r1)); dump("r1_im", cimagl(r1));
  dump("r2_re", creall(r2)); dump("r2_im", cimagl(r2));
  return memcmp(&r1,&r2,32);
}
