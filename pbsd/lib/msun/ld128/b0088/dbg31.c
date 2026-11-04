#include <stdio.h>
#include <string.h>
#include <math.h>
int main() {
  long double s1,c1,s2,c2;
  sincosl(1.0L,&s1,&c1);
  sincosl(1.0L,&s2,&c2);
  printf("s match %d c match %d\n", !memcmp(&s1,&s2,16), !memcmp(&c1,&c2,16));
}
