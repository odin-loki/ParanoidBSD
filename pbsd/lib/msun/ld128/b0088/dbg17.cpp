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
  static const long double cexp_xs[] = {0.0L, -0.0L};
  static const long double cexp_ys[] = {0.0L, -0.0L, 1.0L};
  long double _Complex z1=0; __real__ z1=-0.0L; __imag__ z1=1.0L;
  long double _Complex z2 = mkcx(-0.0L, 1.0L);
  long double _Complex z3 = mkcx(cexp_xs[1], cexp_ys[2]);
  for (int k=1;k<=3;k++) {
    long double _Complex z = k==1?z1:k==2?z2:z3;
    unsigned char bz[32]; memcpy(bz,&z,32);
    printf("k=%d z=",k); for(int i=0;i<32;i++) printf("%02x",bz[i]); printf("\n");
    long double pr,pi,rr,ri;
    pbsd_b0088_cexpl_parts(z,&pr,&pi);
    ref_cexpl_parts(z,&rr,&ri);
    dump("port_im",pi); printf(" ref_im="); dump("",ri); printf(" match=%d\n",!memcmp(&pi,&ri,16));
  }
}
