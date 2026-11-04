#define _GNU_SOURCE
#define complex _Complex
#include <complex.h>
#include <stdio.h>
#include <string.h>
long double complex ref_cexpl(long double complex);
int main() {
  long double complex z = CMPLXL(-0.0L, 1.0L);
  long double complex r = ref_cexpl(z);
  unsigned char bs[16], bc[16];
  memcpy(bs,&r,16); // wrong need split
  long double re = creall(r), im = cimagl(r);
  unsigned char br[16], bi[16];
  memcpy(br,&re,16); memcpy(bi,&im,16);
  printf("in creall=%d cimagl==1 %d\n", creall(z)==0.0L, cimagl(z)==1.0L);
  printf("out re: "); for(int i=15;i>=0;i--) printf("%02x",br[i]); printf("\n");
  printf("out im: "); for(int i=15;i>=0;i--) printf("%02x",bi[i]); printf("\n");
}
