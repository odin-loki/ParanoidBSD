#include <stdio.h>
#include <string.h>
extern void ref_cexpl_parts(long double, long double, long double*, long double*);
extern void pbsd_b0088_cexpl_parts(long double, long double, long double*, long double*);
int main() {
  static const long double cexp_xs[] = {
    0.0L, -0.0L, 1.0L, -1.0L, 0.5L,
    1.13565234062941439494919310779707649e+04L,
    1.13565234062941439494919310779707650e+04L,
    2.27892930024498818830197576893019292e+04L,
    2.27892930024498818830197576893019293e+04L,
    1.0L / 0.0L, -1.0L / 0.0L,
    0.0L / 0.0L,
  };
  static const long double cexp_ys[] = {
    0.0L, -0.0L, 1.0L, -1.0L,
    0x1.921fb54442d1846p+1L, -0x1.921fb54442d1846p+1L,
    1.0L / 0.0L, -1.0L / 0.0L,
    0.0L / 0.0L,
  };
  int fails=0;
  for (size_t i=0;i<sizeof(cexp_xs)/sizeof(cexp_xs[0]);i++)
    for (size_t j=0;j<sizeof(cexp_ys)/sizeof(cexp_ys[0]);j++) {
      long double pr,pi,rr,ri;
      pbsd_b0088_cexpl_parts(cexp_xs[i],cexp_ys[j],&pr,&pi);
      ref_cexpl_parts(cexp_xs[i],cexp_ys[j],&rr,&ri);
      fails += (memcmp(&pr,&rr,16)|memcmp(&pi,&ri,16))!=0;
    }
  printf("fails=%d\n",fails);
}
