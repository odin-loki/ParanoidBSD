#include <cstdio>
struct FpEnv28 { unsigned fcw, fsw, ftw, fpu_op, fpu_sel, fpu_ip, fpu_dp; };
struct SavedFp { FpEnv28 x87; unsigned mxcsr; };
extern "C" { typedef int fp_except_t; typedef enum { FP_PS=0, FP_PRS, FP_PD, FP_PE } fp_prec_t; fp_prec_t ref_fpgetprec(void); }
import pbsd.lib.libc.amd64.gen.b0055;
static void save_fp(SavedFp *s) { __asm__ volatile("fnstenv %0":"=m"(s->x87)); __asm__ volatile("stmxcsr %0":"=m"(s->mxcsr)); }
static void restore_fp(const SavedFp *s) { __asm__ volatile("fldenv %0"::"m"(s->x87)); __asm__ volatile("ldmxcsr %0"::"m"(s->mxcsr)); }
int main() {
    SavedFp s; save_fp(&s);
    std::printf("1 ref=%d\n", ref_fpgetprec());
    std::printf("2 port=%d\n", (int)pbsd::lib_libc_amd64_gen::b0055::fpgetprec());
    restore_fp(&s);
    std::printf("done\n");
    return 0;
}
