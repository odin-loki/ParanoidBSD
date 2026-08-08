#define _DEFAULT_SOURCE
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
import pbsd.bin.pax.b0207;
namespace P = pbsd::bin_pax::b0207;
extern "C" { u_int ref_st_hash(char*,int,int); }
#define HBUFSZ 1024
#define DATA_OFF 192
#define MAXDLEN 400
static void hash_case(const unsigned char *data,int dlen,int off,int len,int tabsz){
  unsigned char a[HBUFSZ],b[HBUFSZ];
  memset(a,0x7f,sizeof(a)); memset(b,0x7f,sizeof(b));
  if(dlen>0){memcpy(a+off,data,(size_t)dlen); memcpy(b+off,data,(size_t)dlen);}
  u_int ra=P::st_hash((char*)a+off,len,tabsz);
  u_int rb=ref_st_hash((char*)b+off,len,tabsz);
  if(ra!=rb){fprintf(stderr,"mismatch len=%d tabsz=%d ra=%u rb=%u\n",len,tabsz,ra,rb); abort();}
  if(memcmp(a,b,sizeof(a))!=0){fprintf(stderr,"buf mismatch\n"); abort();}
}
int main(){
  static const int lens[]={-8,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,15,16,17,31,32,33,63,64,65,124,125,126,127,128,129,130,131,132,133,191,192,255,256,257,383,384,399,400};
  static const int tabs[]={1,2,3,4,5,7,8,13,16,17,127,251,317,1021,4093,65521,1000003,2147483647,-1};
  unsigned char data[MAXDLEN];
  for(int p=0;p<8;p++){
    for(int i=0;i<MAXDLEN;i++){
      switch(p){
        case 0:data[i]=0;break; case 1:data[i]='a';break;
        case 2:data[i]=(unsigned char)(i&0xff);break; case 3:data[i]=0xff;break;
        case 4:data[i]=(unsigned char)(0x80+(i&0x7f));break;
        case 5:data[i]=(unsigned char)((i%3)?0:0x80);break;
        case 6:data[i]=(unsigned char)((i&1)?0xff:0);break;
        default:data[i]=0x7f;break;
      }
    }
    for(int l=0;l<(int)(sizeof(lens)/sizeof(lens[0]));l++){
      int len=lens[l], dlen=len>0?len:0; if(dlen>MAXDLEN)dlen=MAXDLEN;
      for(int t=0;t<(int)(sizeof(tabs)/sizeof(tabs[0]));t++)
        hash_case(data,dlen,DATA_OFF,len,tabs[t]);
    }
  }
  for(int i=0;i<256;i++){data[0]=(unsigned char)i; hash_case(data,1,DATA_OFF,1,317); hash_case(data,1,DATA_OFF,1,1); hash_case(data,1,DATA_OFF+1,1,65521);}
  for(int off=DATA_OFF;off<DATA_OFF+4;off++){
    for(int i=0;i<32;i++) data[i]=(unsigned char)(0x81+i);
    for(int l=0;l<(int)(sizeof(lens)/sizeof(lens[0]));l++) hash_case(data,32,off,lens[l],317);
  }
  puts("hand ok");
  return 0;
}
