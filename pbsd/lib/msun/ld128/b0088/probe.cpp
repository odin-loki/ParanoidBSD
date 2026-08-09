#define _GNU_SOURCE
#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef _Complex long double ldouble_complex;

extern ldouble_complex ref_cexpl(ldouble_complex);

static void dump(const char *tag, ldouble_complex z) {
    unsigned char b[32];
  memset(b,0,sizeof(b));
  memcpy(b,&z,sizeof(z));
  printf("%s: ", tag);
  for (int i=31;i>=0;i--) printf("%02x", b[i]);
  printf(" re=%La im=%La\n", creall(z), cimagl(z));
}

int main() {
  ldouble_complex z;
  __real__(z) = 1.0L/0.0L;
  __imag__(z) = 0.0L;
  dump("input", z);
  dump("ref", ref_cexpl(z));
  return 0;
}
