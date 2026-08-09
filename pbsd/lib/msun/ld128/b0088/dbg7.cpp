#define complex _Complex
#include <complex.h>
#include <cstdio>
#include <cstring>
int main() {
  long double re=-0.0L, im=1.0L;
  long double _Complex z=0;
  std::memcpy(&z, &re, sizeof(re));
  std::memcpy((char*)&z + sizeof(re), &im, sizeof(im));
  unsigned char b[32]; memcpy(b,&z,32);
  printf("bytes: "); for(int i=0;i<32;i++) printf("%02x",b[i]); printf("\n");
  long double r2, i2;
  memcpy(&r2, &z, 16);
  memcpy(&i2, (char*)&z+16, 16);
  printf("memcpy re=%La im=%La\n", r2, i2);
  printf("creall=%La cimagl=%La\n", __real__ z, __imag__ z);
}
