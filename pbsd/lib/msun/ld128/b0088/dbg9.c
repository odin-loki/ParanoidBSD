#include <stdio.h>
#include <math.h>
int main() {
  long double im = 1.0L;
  printf("im==1: %d\n", im == 1.0L);
  printf("sinl=%La cosl=%La\n", sinl(im), cosl(im));
  long double re = -0.0L;
  printf("re==0: %d re==-0: %d\n", re==0.0L, re==-0.0L);
}
