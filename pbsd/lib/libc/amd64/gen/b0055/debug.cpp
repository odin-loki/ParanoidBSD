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
static void save_fp(SavedFp*s){__asm__ volatile("fnstenv %0":"=m"(s->x87));__asm__ volatile("stmxcsr %0":"=m"(s->mxcsr));}
static void restore_fp(const SavedFp*s){__asm__ volatile("fldenv %0"::"m"(s->x87));__asm__ volatile("ldmxcsr %0"::"m"(s->mxcsr));}
static unsigned short build_cw(unsigned rnd,unsigned prc,unsigned mask_en){unsigned short cw=0x037f;cw&=(unsigned short)~(FP_RND_FLD|FP_PRC_FLD|FP_MSKS_FLD);cw|=(unsigned short)((rnd<<FP_RND_OFF)&FP_RND_FLD);cw|=(unsigned short)((prc<<FP_PRC_OFF)&FP_PRC_FLD);cw|=(unsigned short)((~mask_en<<FP_MSKS_OFF)&FP_MSKS_FLD);return cw;}
static void apply_fp_state(const SavedFp*base,unsigned short cw,unsigned short sw,unsigned mxcsr){FpEnv28 env=base->x87;env.fcw=(env.fcw&~0xffffu)|(cw&0xffffu);env.fsw=(env.fsw&~0xffffu)|((env.fsw&~FP_STKY_FLD)|(sw&FP_STKY_FLD))&0xffffu;__asm__ volatile("fldenv %0"::"m"(env));__asm__ volatile("ldmxcsr %0"::"m"(mxcsr));}
int main() {
    SavedFp base; save_fp(&base);
    std::printf("1 prec high-bit\n");
    apply_fp_state(&base, build_cw(0, 2, 0x15), 0, base.mxcsr);
    std::printf("prec ref=%d port=%d\n", ref_fpgetprec(), port::fpgetprec());
    std::printf("2 round high-bit\n");
    apply_fp_state(&base, build_cw(2, 2, 0x2a), 0, base.mxcsr);
    std::printf("round ref=%d port=%d\n", ref_fpgetround(), port::fpgetround());
    std::printf("3 mask high-bit\n");
    apply_fp_state(&base, build_cw(1, 2, 0x15), 0, base.mxcsr);
    std::printf("mask ref=%d port=%d\n", ref_fpgetmask(), port::fpgetmask());
    unsigned mxcsr_base = base.mxcsr & ~SSE_STKY_FLD;
    std::printf("4 sticky high-bit cw\n");
    apply_fp_state(&base, (unsigned short)0x8080, (unsigned short)0x00c0, mxcsr_base | 0x01);
    std::printf("sticky ref=%d port=%d\n", ref_fpgetsticky(), port::fpgetsticky());
    restore_fp(&base);
    return 0;
}
