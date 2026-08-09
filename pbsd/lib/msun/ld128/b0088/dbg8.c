#include <stdio.h>
#include <string.h>
int main() {
  long double re=-0.0L, im=1.0L;
  unsigned char br[16], bi[16];
  memcpy(br,&re,16); memcpy(bi,&im,16);
  printf("re bytes: "); for(int i=0;i<16;i++) printf("%02x",br[i]);
  printf("\nim bytes: "); for(int i=0;i<16;i++) printf("%02x",bi[i]);
  printf("\nre=%La im=%La\n", re, im);
}
