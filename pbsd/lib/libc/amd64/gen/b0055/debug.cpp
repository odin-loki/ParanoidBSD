#include <cstdio>
extern "C" {
typedef enum { FP_RN=0, FP_RM, FP_RP, FP_RZ } fp_rnd_t;
typedef enum { FP_PS=0, FP_PRS, FP_PD, FP_PE } fp_prec_t;
typedef int fp_except_t;
fp_prec_t ref_fpgetprec(void);
fp_rnd_t ref_fpgetround(void);
fp_except_t ref_fpgetmask(void);
fp_except_t ref_fpgetsticky(void);
}
import pbsd.lib.libc.amd64.gen.b0055;
namespace port = pbsd::lib_libc_amd64_gen::b0055;
#define FP_MSKS_FLD 0x3f
#define FP_PRC_FLD 0x300
#define FP_RND_FLD 0xc00
#define FP_STKY_FLD 0x3f
#define FP_MSKS_OFF 0
#define FP_PRC_OFF 8
#define FP_RND_OFF 10
#define SSE_STKY_FLD 0x3f
struct FpEnv28 { unsigned fcw,fsw,ftw,fpu_op,fpu_sel,fpu_ip,fpu_dp; };
struct SavedFp { FpEnv28 x87; unsigned mxcsr; };
static unsigned short build_cw(unsigned rnd,unsigned prc,unsigned mask_en){unsigned short cw=0x037f;cw&=(unsigned short)~(FP_RND_FLD|FP_PRC_FLD|FP_MSKS_FLD);cw|=(unsigned short)((rnd<<FP_RND_OFF)&FP_RND_FLD);cw|=(unsigned short)((prc<<FP_PRC_OFF)&FP_PRC_FLD);cw|=(unsigned short)((~mask_en<<FP_MSKS_OFF)&FP_MSKS_FLD);return cw;}
static unsigned stick_mxcsr(unsigned base,unsigned sticky){return (base&~SSE_STKY_FLD)|(sticky&SSE_STKY_FLD);}
static void apply_fp_state(const SavedFp*base,unsigned short cw,unsigned short sw,unsigned mxcsr){FpEnv28 env=base->x87;env.fcw=(env.fcw&~0xffffu)|(cw&0xffffu);env.fsw=(env.fsw&~0xffffu)|((env.fsw&~FP_STKY_FLD)|(sw&FP_STKY_FLD))&0xffffu;__asm__ volatile("fldenv %0"::"m"(env));__asm__ volatile("ldmxcsr %0"::"m"(mxcsr));}
static void T(const SavedFp*b,const char*tag,unsigned short cw,unsigned short sw,unsigned mxcsr){std::fprintf(stderr,"%s\n",tag);apply_fp_state(b,cw,sw,mxcsr);std::fprintf(stderr,"  prec=%d/%d round=%d/%d mask=%d/%d sticky=%d/%d\n",ref_fpgetprec(),(int)port::fpgetprec(),ref_fpgetround(),(int)port::fpgetround(),ref_fpgetmask(),port::fpgetmask(),ref_fpgetsticky(),port::fpgetsticky());}
int main(){SavedFp base;__asm__ volatile("fnstenv %0":"=m"(base.x87));__asm__ volatile("stmxcsr %0":"=m"(base.mxcsr));unsigned mxcsr_base=base.mxcsr&~SSE_STKY_FLD;unsigned rnd,prc,bit;
T(&base,"default",build_cw(ref_fpgetround(),ref_fpgetprec(),ref_fpgetmask()),0,base.mxcsr);
for(rnd=0;rnd<=3;rnd++)T(&base,"rnd",build_cw(rnd,FP_PD,0),0,base.mxcsr);
T(&base,"rnd3",build_cw(3,FP_PD,0),0,base.mxcsr);
T(&base,"rnd4",build_cw(4,FP_PD,0),0,base.mxcsr);
for(prc=0;prc<=3;prc++)T(&base,"prc",build_cw(FP_RN,prc,0),0,base.mxcsr);
T(&base,"prc3",build_cw(FP_RN,3,0),0,base.mxcsr);
T(&base,"prc4",build_cw(FP_RN,4,0),0,base.mxcsr);
T(&base,"mask0",build_cw(FP_RN,FP_PD,0),0,base.mxcsr);
T(&base,"maskall",build_cw(FP_RN,FP_PD,FP_MSKS_FLD),0,base.mxcsr);
for(bit=0;bit<6;bit++)T(&base,"maskbit",build_cw(FP_RN,FP_PD,1u<<bit),0,base.mxcsr);
T(&base,"maskbf1",build_cw(FP_RM,FP_PS,0x3f),0,base.mxcsr);
T(&base,"maskbf2",build_cw(FP_RP,FP_PE,0x00),0,base.mxcsr);
for(bit=0;bit<6;bit++){T(&base,"stkx87",build_cw(FP_RN,FP_PD,0),(unsigned short)(1u<<bit),mxcsr_base);T(&base,"stksse",build_cw(FP_RN,FP_PD,0),0,mxcsr_base|(1u<<bit));}
T(&base,"stk0",build_cw(FP_RN,FP_PD,0),0,mxcsr_base);
T(&base,"stkall",build_cw(FP_RN,FP_PD,0),FP_STKY_FLD,mxcsr_base|SSE_STKY_FLD);
T(&base,"stk15",build_cw(FP_RN,FP_PD,0),0x15,mxcsr_base|0x2a);
std::fprintf(stderr,"before high-bit prec\n");
T(&base,"hiprec",build_cw(FP_RN,FP_PD,0x15),0,base.mxcsr);
std::fprintf(stderr,"before high-bit round\n");
T(&base,"hiround",build_cw(2,FP_PD,0x2a),0,base.mxcsr);
std::fprintf(stderr,"done\n");return 0;}
