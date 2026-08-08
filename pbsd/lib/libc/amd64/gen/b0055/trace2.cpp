#include <cstdio>
#define FP_MSKS_FLD 0x3f
#define FP_PRC_FLD 0x300
#define FP_RND_FLD 0xc00
#define FP_STKY_FLD 0x3f
#define FP_MSKS_OFF 0
#define FP_PRC_OFF 8
#define FP_RND_OFF 10
#define SSE_STKY_FLD 0x3f
struct FpEnv28 { unsigned fcw, fsw, ftw, fpu_op, fpu_sel, fpu_ip, fpu_dp; };
struct SavedFp { FpEnv28 x87; unsigned mxcsr; };
static unsigned short build_cw(unsigned rnd, unsigned prc, unsigned mask_en) {
    unsigned short cw = 0x037f;
    cw &= (unsigned short)~(FP_RND_FLD | FP_PRC_FLD | FP_MSKS_FLD);
    cw |= (unsigned short)((rnd << FP_RND_OFF) & FP_RND_FLD);
    cw |= (unsigned short)((prc << FP_PRC_OFF) & FP_PRC_FLD);
    cw |= (unsigned short)((~mask_en << FP_MSKS_OFF) & FP_MSKS_FLD);
    return cw;
}
static void apply_fp_state(const SavedFp *base, unsigned short cw, unsigned short sw, unsigned mxcsr) {
    FpEnv28 env = base->x87;
    env.fcw = (env.fcw & ~0xffffu) | (cw & 0xffffu);
    env.fsw = (env.fsw & ~0xffffu) | ((env.fsw & ~FP_STKY_FLD) | (sw & FP_STKY_FLD)) & 0xffffu;
    __asm__ volatile("fldenv %0"::"m"(env));
    __asm__ volatile("ldmxcsr %0"::"m"(mxcsr));
}
int main() {
    SavedFp base; __asm__ volatile("fnstenv %0":"=m"(base.x87)); __asm__ volatile("stmxcsr %0":"=m"(base.mxcsr));
    unsigned mxcsr_base = base.mxcsr & ~SSE_STKY_FLD;
    std::printf("test sticky 0xffff mxcsr\n");
    apply_fp_state(&base, build_cw(1,2,0x3f), (unsigned short)0xffff, mxcsr_base | 0xffffffc0u);
    std::printf("ok\n");
    return 0;
}
