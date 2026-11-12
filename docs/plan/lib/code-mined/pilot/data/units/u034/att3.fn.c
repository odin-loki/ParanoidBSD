uint32_t ossl_ml_dsa_key_compress_high_bits(uint32_t r, uint32_t gamma2)
{
    //@ assert r + 127 <= 12415;
    int32_t r1 = (r + 127) >> 7;
    //@ assert 0 <= r1 <= 96;

    if (gamma2 == ML_DSA_GAMMA2_Q_MINUS1_DIV32) {
        //@ assert r1 * 1025 + 2097152 < 4194304;
        r1 = (r1 * 1025 + (1 << 21)) >> 22;
        //@ assert r1 == 0;
        r1 &= 15; /* mod 16 */
        //@ assert r1 == 0;
        return r1;
    } else {
        //@ assert r1 * 11275 + 8388608 < 16777216;
        r1 = (r1 * 11275 + (1 << 23)) >> 24;
        //@ assert r1 == 0;
        r1 ^= ((43 - r1) >> 31) & r1;
        //@ assert r1 == 0;
        return r1;
    }
}
