#define complex _Complex
#include <complex.h>
#include <cstdio>
#include <cstring>
extern "C" {
void ref_cexpl_parts(long double _Complex, long double*, long double*);
void pbsd_b0088_cexpl_parts(long double _Complex, long double*, long double*);
}
int main() {
  long double _Complex z=0;
  __real__ z = 1.13565234062941439494919310779707650e+04L;
  __imag__ z = 1.0L;
  long double pr,pi,rr,ri;
  pbsd_b0088_cexpl_parts(z,&pr,&pi);
  ref_cexpl_parts(z,&rr,&ri);
  printf("re match %d im match %d\n", !memcmp(&pr,&rr,16), !memcmp(&pi,&ri,16));
  return memcmp(&pr,&rr,16)|memcmp(&pi,&ri,16);
}
