#include <stdio.h>
#include <string.h>
#include <math.h>
int main() {
  long double s = sinl(1.0L), c = cosl(1.0L);
  unsigned char bs[16], bc[16];
  memcpy(bs,&s,16); memcpy(bc,&c,16);
  printf("sin bytes: "); for(int i=15;i>=0;i--) printf("%02x",bs[i]); printf("\n");
  printf("cos bytes: "); for(int i=15;i>=0;i--) printf("%02x",bc[i]); printf("\n");
}
