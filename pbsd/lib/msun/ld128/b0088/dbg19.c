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
  long double pr,pi;
  ref_cexpl_parts(z,&pr,&pi);
  long double re=creall(r), im=cimagl(r);
  printf("match re=%d im=%d\n", memcmp(&re,&pr,16)==0, memcmp(&im,&pi,16)==0);
  return memcmp(&re,&pr,16)|memcmp(&im,&pi,16);
}
