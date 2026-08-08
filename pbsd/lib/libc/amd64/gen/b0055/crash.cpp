#include <cstdio>
extern "C" { typedef int fp_except_t; fp_except_t ref_fpgetsticky(void); }
import pbsd.lib.libc.amd64.gen.b0055;
#define FP_STKY_FLD 0x3f
#define SSE_STKY_FLD 0x3f
struct FpEnv28 { unsigned fcw,fsw,ftw,fpu_op,fpu_sel,fpu_ip,fpu_dp; };
struct SavedFp { FpEnv28 x87; unsigned mxcsr; };
static unsigned short build_cw(unsigned rnd,unsigned prc,unsigned m){unsigned short cw=0x037f;cw&=~0xf3f;cw|=(rnd<<10)&0xc00;cw|=(prc<<8)&0x300;cw|=((~m)&0x3f);return cw;}
static void apply(const SavedFp*b,unsigned short cw,unsigned short sw,unsigned mx){FpEnv28 e=b->x87;e.fcw=(e.fcw&~0xffffu)|(cw&0xffffu);e.fsw=(e.fsw&~0xffffu)|((e.fsw&~FP_STKY_FLD)|(sw&FP_STKY_FLD))&0xffffu;__asm__ volatile("fldenv %0"::"m"(e));__asm__ volatile("ldmxcsr %0"::"m"(mx));}
int main(){SavedFp b;__asm__ volatile("fnstenv %0":"=m"(b.x87));__asm__ volatile("stmxcsr %0":"=m"(b.mxcsr));unsigned mb=b.mxcsr&~0x3fu;
for(int i=0;i<1000;i++){
 unsigned short cw=build_cw(3,1,0);
 unsigned mx=(mb|0x15);
 apply(&b,cw,0xff,mx);
 int r=ref_fpgetsticky();
 if(i==0) std::printf("cw=%#x mx=%#x sticky=%d\n",cw,mx,r);
}
std::printf("ok\n");}
