#define complex _Complex
#include <complex.h>
#include <cstdio>
#include <cstring>
extern "C" long double _Complex ref_cexpl(long double _Complex);
int main() {
  long double _Complex z=0; __real__ z=-0.0L; __imag__ z=1.0L;
  long double _Complex r = ref_cexpl(z);
  long double re, im;
  memcpy(&re,&r,16);
  memcpy(&im,(char*)&r+16,16);
  unsigned char br[16],bi[16]; memcpy(br,&re,16); memcpy(bi,&im,16);
  printf("out re: "); for(int i=15;i>=0;i--) printf("%02x",br[i]); printf("\n");
  printf("out im: "); for(int i=15;i>=0;i--) printf("%02x",bi[i]); printf("\n");
}
