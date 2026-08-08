#define _GNU_SOURCE
#define complex _Complex
#include <complex.h>
#include <stdio.h>
#include <string.h>
void ref_cexpl_parts(long double complex, long double *, long double *);
void pbsd_b0088_cexpl_parts(long double complex, long double *, long double *);
static void dump(const char*t, long double x) {
  unsigned char b[16]; memcpy(b,&x,16);
  printf("%s=",t); for(int i=15;i>=0;i--) printf("%02x",b[i]); printf("\n");
}
int main() {
  long double complex z = CMPLXL(1.0L/0.0L, 0.0L);
  long double pr,pi,rr,ri;
  pbsd_b0088_cexpl_parts(z,&pr,&pi);
  ref_cexpl_parts(z,&rr,&ri);
  dump("port_re",pr); dump("port_im",pi);
  dump("ref_re",rr); dump("ref_im",ri);
  return (memcmp(&pr,&rr,16)|memcmp(&pi,&ri,16));
}
