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

int main() {
    std::printf("ref prec=%d\n", ref_fpgetprec());
    std::printf("port prec=%d\n", port::fpgetprec());
    std::printf("ref round=%d\n", ref_fpgetround());
    std::printf("port round=%d\n", port::fpgetround());
    std::printf("ref mask=%d\n", ref_fpgetmask());
    std::printf("port mask=%d\n", port::fpgetmask());
    std::printf("ref sticky=%d\n", ref_fpgetsticky());
    std::printf("port sticky=%d\n", port::fpgetsticky());
    return 0;
}
