#include <stdio.h>
#include <string.h>
extern void ref_cexpl_parts(long double, long double, long double*, long double*);
extern void pbsd_b0088_cexpl_parts(long double, long double, long double*, long double*);
int main() {
  static const long double cexp_xs[] = {0.0L, -0.0L, 1.0L};
  static const long double cexp_ys[] = {0.0L, -0.0L, 1.0L};
  for (size_t i=0;i<=1;i++)
    for (size_t j=0;j<(i==1?3:3);j++) {
      if (i==1 && j==2) {
        long double pr,pi,rr,ri,a,b,c,d;
        ref_cexpl_parts(cexp_xs[1],cexp_ys[2],&a,&b);
        ref_cexpl_parts(cexp_xs[1],cexp_ys[2],&c,&d);
        pbsd_b0088_cexpl_parts(cexp_xs[1],cexp_ys[2],&pr,&pi);
        ref_cexpl_parts(cexp_xs[1],cexp_ys[2],&rr,&ri);
        printf("ref self %d port vs ref %d\n",
          !memcmp(&a,&c,16)&&!memcmp(&b,&d,16),
          !memcmp(&pr,&rr,16)&&!memcmp(&pi,&ri,16));
        return 0;
      }
      long double a,b;
      ref_cexpl_parts(cexp_xs[i],cexp_ys[j],&a,&b);
      pbsd_b0088_cexpl_parts(cexp_xs[i],cexp_ys[j],&a,&b);
    }
}
