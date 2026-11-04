#define complex _Complex
#include <complex.h>
#include <cstdio>
#include <cstring>
extern "C" {
void ref_cexpl_parts(long double _Complex, long double *, long double *);
void pbsd_b0088_cexpl_parts(long double _Complex, long double *, long double *);
}
static long double _Complex mkcx(long double re, long double im) {
  long double _Complex z=0; __real__ z=re; __imag__ z=im; return z;
}
static void dump(const char*t, long double x) {
  unsigned char b[16]; memcpy(b,&x,16);
  printf("%s=",t); for(int i=15;i>=0;i--) printf("%02x",b[i]);
}
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
  for (size_t i=0;i<sizeof(cexp_xs)/sizeof(cexp_xs[0]);i++)
    for (size_t j=0;j<sizeof(cexp_ys)/sizeof(cexp_ys[0]);j++) {
      long double _Complex z = mkcx(cexp_xs[i],cexp_ys[j]);
      long double pr,pi,rr,ri;
      pbsd_b0088_cexpl_parts(z,&pr,&pi);
      ref_cexpl_parts(z,&rr,&ri);
      if (memcmp(&pr,&rr,16)||memcmp(&pi,&ri,16)) {
        printf("i=%zu j=%zu\n",i,j);
        dump("port_re",pr); printf(" "); dump("port_im",pi); printf("\n");
        dump("ref_re",rr); printf(" "); dump("ref_im",ri); printf("\n");
      }
    }
}
