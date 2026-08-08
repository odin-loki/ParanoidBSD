#include <stdio.h>
#include <stdint.h>

struct FpEnv28 {
    unsigned fcw;
    unsigned fsw;
    unsigned ftw;
    unsigned fpu_op;
    unsigned fpu_sel;
    unsigned fpu_ip;
    unsigned fpu_dp;
};

int main(void) {
    struct FpEnv28 env;
    unsigned mxcsr;
    __asm__ volatile("fnstenv %0" : "=m"(env));
    __asm__ volatile("stmxcsr %0" : "=m"(mxcsr));
    printf("saved fcw=%#x fsw=%#x mxcsr=%#x\n", env.fcw, env.fsw, mxcsr);
    env.fcw = (env.fcw & ~0xffffu) | 0x037f;
    __asm__ volatile("fldenv %0" : : "m"(env));
    printf("after fldenv ok\n");
    return 0;
}
