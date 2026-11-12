#include "prelude.h"
#undef crypto_declassify
#define crypto_declassify(x, y) do {} while (0)
#undef int8
#define int8 crypto_int8
#undef uint8
#define uint8 crypto_uint8
#undef int16
#define int16 crypto_int16
#undef uint16
#define uint16 crypto_uint16
#undef int32
#define int32 crypto_int32
#undef uint32
#define uint32 crypto_uint32
#undef int64
#define int64 crypto_int64
#undef uint64
#define uint64 crypto_uint64
#undef crypto_int16_h
#define crypto_int16_h
#undef crypto_int16
#define crypto_int16 int16_t
#undef crypto_int16_unsigned
#define crypto_int16_unsigned uint16_t
#undef crypto_int32_h
#define crypto_int32_h
#undef crypto_int32
#define crypto_int32 int32_t
#undef crypto_int32_unsigned
#define crypto_int32_unsigned uint32_t
#undef crypto_int64_h
#define crypto_int64_h
#undef crypto_int64
#define crypto_int64 int64_t
#undef crypto_int64_unsigned
#define crypto_int64_unsigned uint64_t
#undef int32_MINMAX
#define int32_MINMAX(a,b) crypto_int32_minmax(&a,&b)
#undef p
#define p 761
#undef q
#define q 4591
#undef w
#define w 286
#undef q12
#define q12 ((q - 1) / 2)
#undef Hash_bytes
#define Hash_bytes 32
#undef Small_bytes
#define Small_bytes ((p + 3) / 4)
#undef SecretKeys_bytes
#define SecretKeys_bytes (2 * Small_bytes)
#undef Confirm_bytes
#define Confirm_bytes 32
typedef int8_t small;
typedef int16_t Fq;
typedef small Inputs[p];
/*@ requires true; ensures \result == (crypto_int64_x == crypto_int64_y ? 1 : 0); */
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
  return 1-crypto_int64_unequal_01(crypto_int64_x,crypto_int64_y);
#endif
}
