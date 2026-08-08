#include <stdio.h>
typedef int fp_except_t;
typedef enum { FP_PS=0, FP_PRS, FP_PD, FP_PE } fp_prec_t;
fp_prec_t ref_fpgetprec(void);
int main(void) {
    printf("prec=%d\n", ref_fpgetprec());
    return 0;
}
