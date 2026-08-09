#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <math.h>
int main() {
  long double s1,c1,s2,c2;
  sincosl(1.0L,&s1,&c1);
  sincosl(1.0L,&s2,&c2);
  unsigned char a[16],b[16],c[16],d[16];
  memcpy(a,&s1,16); memcpy(b,&c1,16); memcpy(c,&s2,16); memcpy(d,&c2,16);
  printf("s1:"); for(int i=15;i>=0;i--) printf("%02x",a[i]);
  printf("\ns2:"); for(int i=15;i>=0;i--) printf("%02x",c[i]);
  printf("\nc1:"); for(int i=15;i>=0;i--) printf("%02x",b[i]);
  printf("\nc2:"); for(int i=15;i>=0;i--) printf("%02x",d[i]);
  printf("\n");
}
