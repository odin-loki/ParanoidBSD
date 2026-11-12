__attribute__((unused))
static inline
crypto_int64 crypto_int64_equal_01(crypto_int64 crypto_int64_x,crypto_int64 crypto_int64_y) {
#if defined(__GNUC__) && defined(__x86_64__)
  crypto_int64 crypto_int64_q,crypto_int64_z;
  __asm__ ("xorq %0,%0\n movq $1,%1\n cmpq %3,%2\n cmoveq %1,%0" : "=&r"(crypto_int64_z), "=&r"(crypto_int64_q) : "r"(crypto_int64_x), "r"(crypto_int64_y) : "cc");
  return crypto_int64_z;
#elif defined(__GNUC__) && defined(__aarch64__)
  crypto_int64 crypto_int64_z;
  __asm__ ("cmp %1,%2\n cset %0,eq" : "=r"(crypto_int64_z) : "r"(crypto_int64_x), "r"(crypto_int64_y) : "cc");
  return crypto_int64_z;
#else
  /*@ assert crypto_int64_unequal_01(crypto_int64_x,crypto_int64_y) == (crypto_int64_x == crypto_int64_y ? 0 : 1); */
  /*@ assert 1 - crypto_int64_unequal_01(crypto_int64_x,crypto_int64_y) == (crypto_int64_x == crypto_int64_y ? 1 : 0); */
  return 1-crypto_int64_unequal_01(crypto_int64_x,crypto_int64_y);
#endif
}
