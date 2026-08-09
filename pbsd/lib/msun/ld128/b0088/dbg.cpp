#define complex _Complex
#include <complex.h>
#include <cstdio>
#include <cstring>
import pbsd.lib.msun.ld128.b0088;
extern "C" long double _Complex ref_cexpl(long double _Complex);
static void dump(const char*t, long double x) {
  unsigned char b[16]; memcpy(b,&x,16);
  printf("%s=",t); for(int i=15;i>=0;i--) printf("%02x",b[i]); printf("\n");
}
int main() {
  long double _Complex z=0; __real__ z=1.0L/0.0L; __imag__ z=0.0L;
  long double _Complex p = pbsd::lib_msun_ld128::b0088::cexpl(z);
  long double _Complex r = ref_cexpl(z);
  dump("port_re", __real__ p); dump("port_im", __imag__ p);
  dump("ref_re", __real__ r); dump("ref_im", __imag__ r);
  return 0;
}
