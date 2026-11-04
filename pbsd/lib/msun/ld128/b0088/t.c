#define _GNU_SOURCE
#define complex _Complex
#include <complex.h>
#include <stdio.h>
#include <string.h>

extern long double _Complex ref_cexpl(long double _Complex);

static void dump(const char *t, long double _Complex z) {
  unsigned char b[32]; memset(b,0,32); memcpy(b,&z,32);
  printf("%s: ", t);
  for (int i=31;i>=0;i--) printf("%02x", b[i]);
  printf(" (%La,%La)\n", creall(z), cimagl(z));
}

int main(void) {
  long double _Complex z, r;
  __real__ z = 1.0L/0.0L;
  __imag__ z = 0.0L;
  dump("in", z);
  r = ref_cexpl(z);
  dump("ref", r);
  return 0;
}
