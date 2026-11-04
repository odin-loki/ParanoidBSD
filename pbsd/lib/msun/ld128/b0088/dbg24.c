#include <stdio.h>
#include <string.h>
extern void ref_cexpl_parts(long double, long double, long double*, long double*);
extern void pbsd_b0088_cexpl_parts(long double, long double, long double*, long double*);
static void dump(const char*t, long double x) {
  unsigned char b[16]; memcpy(b,&x,16);
  printf("%s=",t); for(int i=15;i>=0;i--) printf("%02x",b[i]);
}
int main() {
  long double x=-0.0L, y=1.0L;
  long double pr,pi,rr,ri;
  pbsd_b0088_cexpl_parts(x,y,&pr,&pi);
  ref_cexpl_parts(x,y,&rr,&ri);
  dump("port_re",pr); printf(" "); dump("port_im",pi); printf("\n");
  dump("ref_re",rr); printf(" "); dump("ref_im",ri); printf("\n");
}
