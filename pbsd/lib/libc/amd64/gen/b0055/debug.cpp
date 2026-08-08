#include <cstdio>
extern "C" { typedef int fp_except_t; fp_except_t ref_fpgetsticky(void); }
import pbsd.lib.libc.amd64.gen.b0055;
struct FpEnv28 { unsigned fcw, fsw, ftw, fpu_op, fpu_sel, fpu_ip, fpu_dp; };
struct SavedFp { FpEnv28 x87; unsigned mxcsr; };
#define FP_STKY_FLD 0x3f
#define SSE_STKY_FLD 0x3f
static void save_fp(SavedFp *s) {
    __asm__ volatile("fnstenv %0" : "=m"(s->x87));
    __asm__ volatile("stmxcsr %0" : "=m"(s->mxcsr));
}
static void restore_fp(const SavedFp *s) {
    __asm__ volatile("fldenv %0" : : "m"(s->x87));
    __asm__ volatile("ldmxcsr %0" : : "m"(s->mxcsr));
}
static void apply(const SavedFp *base, unsigned short cw, unsigned short sw, unsigned mxcsr) {
    FpEnv28 env = base->x87;
    env.fcw = (env.fcw & ~0xffffu) | (cw & 0xffffu);
    env.fsw = (env.fsw & ~0xffffu) | ((env.fsw & ~FP_STKY_FLD) | (sw & FP_STKY_FLD)) & 0xffffu;
    __asm__ volatile("fldenv %0" : : "m"(env));
    __asm__ volatile("ldmxcsr %0" : : "m"(mxcsr));
}
int main() {
    SavedFp base; save_fp(&base);
    unsigned mxcsr_base = base.mxcsr & ~SSE_STKY_FLD;
    std::printf("trying 0x8080\n");
    apply(&base, (unsigned short)0x8080, (unsigned short)0x00c0, mxcsr_base | 0x01);
    std::printf("sticky=%d\n", ref_fpgetsticky());
    restore_fp(&base);
    return 0;
}
