#define _GNU_SOURCE
#define complex _Complex
#include <complex.h>
#include <stdio.h>
#include <string.h>
long double complex ref_cexpl(long double complex);
void ref_cexpl_parts(long double complex, long double*, long double*);
int main() {
  long double complex z = CMPLXL(1.13565234062941439494919310779707650e+04L, 1.0L);
  long double complex r = ref_cexpl(z);
  long double pr,pi, re, im;
  ref_cexpl_parts(z,&pr,&pi);
  re = creall(r); im = cimagl(r);
  unsigned char a[16],b[16],c[16],d[16];
  memcpy(a,&re,16); memcpy(b,&im,16);
  memcpy(c,&pr,16); memcpy(d,&pi,16);
  printf("ref re: "); for(int i=15;i>=0;i--) printf("%02x",a[i]);
  printf("\nprt re: "); for(int i=15;i>=0;i--) printf("%02x",c[i]);
  printf("\nref im: "); for(int i=15;i>=0;i--) printf("%02x",b[i]);
  printf("\nprt im: "); for(int i=15;i>=0;i--) printf("%02x",d[i]);
  printf("\n");
}
