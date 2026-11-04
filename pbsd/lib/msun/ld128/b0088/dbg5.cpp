#define complex _Complex
#include <complex.h>
#include <cstdio>
#include <cstring>
#define creall __real__
#define cimagl __imag__
extern "C" {
void ref_cexpl_parts(long double _Complex, long double *, long double *);
long double _Complex ref_cexpl(long double _Complex);
}
static long double _Complex mkcx(long double re, long double im) {
  long double _Complex z=0; __real__ z=re; __imag__ z=im; return z;
}
int main() {
  long double _Complex z = mkcx(-0.0L, 1.0L);
  unsigned char b[32]; memcpy(b,&z,32);
  printf("z bytes: "); for(int i=0;i<32;i++) printf("%02x",b[i]); printf("\n");
  printf("creall=%La cimagl=%La\n", creall(z), cimagl(z));
  printf("y==0 %d x==0 %d\n", cimagl(z)==0, creall(z)==0);
  long double _Complex r = ref_cexpl(z);
  printf("out creall=%La cimagl=%La\n", creall(r), cimagl(r));
  long double pr,pi; ref_cexpl_parts(z,&pr,&pi);
  printf("parts pr=%La pi=%La\n", pr, pi);
}
