#define complex _Complex
#include <complex.h>
#include <cstdio>
#include <cstring>
import pbsd.lib.msun.ld128.b0088;
extern "C" {
void ref_cexpl_parts(long double _Complex, long double *, long double *);
void pbsd_b0088_cexpl_parts(long double _Complex, long double *, long double *);
}
static void dump(const char*t, long double x) {
  unsigned char b[16]; memcpy(b,&x,16);
  printf("%s=",t); for(int i=15;i>=0;i--) printf("%02x",b[i]);
}
int main() {
  long double _Complex z=0; __real__ z=-0.0L; __imag__ z=1.0L;
  long double pr,pi,rr,ri;
  pbsd_b0088_cexpl_parts(z,&pr,&pi);
  ref_cexpl_parts(z,&rr,&ri);
  dump("port_re",pr); printf(" "); dump("port_im",pi); printf("\n");
  dump("ref_re",rr); printf(" "); dump("ref_im",ri); printf("\n");
  return memcmp(&pr,&rr,16)|memcmp(&pi,&ri,16);
}
