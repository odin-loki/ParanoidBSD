#define complex _Complex
#include <complex.h>
#include <cstdio>
#include <cstring>
#ifndef CMPLXL
#define CMPLXL(x,y) ((long double _Complex){(long double)(x),(long double)(y)})
#endif
#define creall __real__
#define cimagl __imag__
extern "C" long double _Complex ref_cexpl(long double _Complex);
int main() {
  long double _Complex z1=0; __real__ z1=-0.0L; __imag__ z1=1.0L;
  long double _Complex z2 = CMPLXL(-0.0L, 1.0L);
  for (int k=0;k<2;k++) {
    long double _Complex z = k?z2:z1;
    printf("k=%d bytes=",k);
    unsigned char b[32]; memcpy(b,&z,32);
    for(int i=0;i<32;i++) printf("%02x",b[i]);
    printf(" creall=%La cimagl=%La\n", creall(z), cimagl(z));
    long double _Complex r = ref_cexpl(z);
    printf(" out creall=%La cimagl=%La\n", creall(r), cimagl(r));
  }
}
