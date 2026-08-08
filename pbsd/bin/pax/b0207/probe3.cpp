#define _DEFAULT_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
import pbsd.bin.pax.b0207;
namespace P = pbsd::bin_pax::b0207;
#define HBUFSZ 1024
#define DATA_OFF 192
#define MAXDLEN 400
static uint64_t rng_state;
static uint64_t rng_next(void){uint64_t x=rng_state;x^=x<<13;x^=x>>7;x^=x<<17;rng_state=x;return x;}
static uint32_t rnd(uint32_t n){return (uint32_t)(rng_next()%(uint64_t)n);}
int main(){
  static const int tabs[]={1,2,3,4,5,7,8,13,16,17,127,251,317,1021,4093,65521,1000003,2147483647,-1};
  unsigned char data[MAXDLEN];
  rng_state=0xc0ffee1234567890ULL;
  for(long n=0;n<200000;n++){
    int dlen=(int)rnd(MAXDLEN+1);
    for(int i=0;i<dlen;i++) data[i]=(unsigned char)rnd(256);
    int len;
    switch(rnd(8)){
      case 0: len=dlen?(int)rnd((uint32_t)dlen):0; break;
      case 1: len=dlen+(int)rnd(9)-4; if(len>MAXDLEN)len=MAXDLEN; break;
      case 2: len=-(int)rnd(9); break;
      case 3: len=124+(int)rnd(11); break;
      default: len=dlen; break;
    }
    int off=DATA_OFF+(int)rnd(4);
    int t=rnd(2)?tabs[rnd(sizeof(tabs)/sizeof(tabs[0]))]:1+(int)rnd(100000);
    unsigned char a[HBUFSZ];
    memset(a,0x7f,sizeof(a));
    if(dlen>0) memcpy(a+off,data,(size_t)dlen);
    (void)P::st_hash((char*)a+off,len,t);
  }
  puts("ok");
  return 0;
}
