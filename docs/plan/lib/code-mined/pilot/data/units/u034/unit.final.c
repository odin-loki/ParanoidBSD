#include "prelude.h"
#undef ML_DSA_GAMMA2_Q_MINUS1_DIV32
#define ML_DSA_GAMMA2_Q_MINUS1_DIV32 ((ML_DSA_Q - 1) / 32)
#undef ML_DSA_Q
#define ML_DSA_Q 8380417
/*@ requires r < 12289 && (gamma2 == 384 || gamma2 == 140); ensures gamma2 == 384 ? \result < 16 : \result < 44; */
uint32_t ossl_ml_dsa_key_compress_high_bits(uint32_t r, uint32_t gamma2)
{
    int32_t r1 = (r + 127) >> 7;

    if (gamma2 == ML_DSA_GAMMA2_Q_MINUS1_DIV32) {
        r1 = (r1 * 1025 + (1 << 21)) >> 22;
        r1 &= 15; /* mod 16 */
        return r1;
    } else {
        r1 = (r1 * 11275 + (1 << 23)) >> 24;
        r1 ^= ((43 - r1) >> 31) & r1;
        return r1;
    }
}
