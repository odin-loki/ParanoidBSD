[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline __floating_decimal_32 __f2d(const uint32_t __ieeeMantissa, const uint32_t __ieeeExponent) {
  int32_t __e2;
  uint32_t __m2;
  if (__ieeeExponent == 0) {
    // We subtract 2 so that the bounds computation has 2 additional bits.
    __e2 = 1 - __FLOAT_BIAS - __FLOAT_MANTISSA_BITS - 2;
    __m2 = __ieeeMantissa;
  } else {
    __e2 = static_cast<int32_t>(__ieeeExponent) - __FLOAT_BIAS - __FLOAT_MANTISSA_BITS - 2;
    __m2 = (1u << __FLOAT_MANTISSA_BITS) | __ieeeMantissa;
  }
  const bool __even = (__m2 & 1) == 0;
  const bool __acceptBounds = __even;

  // Step 2: Determine the interval of valid decimal representations.
  const uint32_t __mv = 4 * __m2;
  const uint32_t __mp = 4 * __m2 + 2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  const uint32_t __mmShift = __ieeeMantissa != 0 || __ieeeExponent <= 1;
  const uint32_t __mm = 4 * __m2 - 1 - __mmShift;

  // Step 3: Convert to a decimal power base using 64-bit arithmetic.
  uint32_t __vr, __vp, __vm;
  int32_t __e10;
  bool __vmIsTrailingZeros = false;
  bool __vrIsTrailingZeros = false;
  uint8_t __lastRemovedDigit = 0;
  if (__e2 >= 0) {
    const uint32_t __q = __log10Pow2(__e2);
    __e10 = static_cast<int32_t>(__q);
    const int32_t __k = __FLOAT_POW5_INV_BITCOUNT + __pow5bits(static_cast<int32_t>(__q)) - 1;
    const int32_t __i = -__e2 + static_cast<int32_t>(__q) + __k;
    __vr = __mulPow5InvDivPow2(__mv, __q, __i);
    __vp = __mulPow5InvDivPow2(__mp, __q, __i);
    __vm = __mulPow5InvDivPow2(__mm, __q, __i);
    if (__q != 0 && (__vp - 1) / 10 <= __vm / 10) {
      // We need to know one removed digit even if we are not going to loop below. We could use
      // __q = X - 1 above, except that would require 33 bits for the result, and we've found that
      // 32-bit arithmetic is faster even on 64-bit machines.
      const int32_t __l = __FLOAT_POW5_INV_BITCOUNT + __pow5bits(static_cast<int32_t>(__q - 1)) - 1;
      __lastRemovedDigit = static_cast<uint8_t>(__mulPow5InvDivPow2(__mv, __q - 1,
        -__e2 + static_cast<int32_t>(__q) - 1 + __l) % 10);
    }
    if (__q <= 9) {
      // The largest power of 5 that fits in 24 bits is 5^10, but __q <= 9 seems to be safe as well.
      // Only one of __mp, __mv, and __mm can be a multiple of 5, if any.
      if (__mv % 5 == 0) {
        __vrIsTrailingZeros = __multipleOfPowerOf5(__mv, __q);
      } else if (__acceptBounds) {
        __vmIsTrailingZeros = __multipleOfPowerOf5(__mm, __q);
      } else {
        __vp -= __multipleOfPowerOf5(__mp, __q);
      }
    }
  } else {
    const uint32_t __q = __log10Pow5(-__e2);
    __e10 = static_cast<int32_t>(__q) + __e2;
    const int32_t __i = -__e2 - static_cast<int32_t>(__q);
    const int32_t __k = __pow5bits(__i) - __FLOAT_POW5_BITCOUNT;
    int32_t __j = static_cast<int32_t>(__q) - __k;
    __vr = __mulPow5divPow2(__mv, static_cast<uint32_t>(__i), __j);
    __vp = __mulPow5divPow2(__mp, static_cast<uint32_t>(__i), __j);
    __vm = __mulPow5divPow2(__mm, static_cast<uint32_t>(__i), __j);
    if (__q != 0 && (__vp - 1) / 10 <= __vm / 10) {
      __j = static_cast<int32_t>(__q) - 1 - (__pow5bits(__i + 1) - __FLOAT_POW5_BITCOUNT);
      __lastRemovedDigit = static_cast<uint8_t>(__mulPow5divPow2(__mv, static_cast<uint32_t>(__i + 1), __j) % 10);
    }
    if (__q <= 1) {
      // {__vr,__vp,__vm} is trailing zeros if {__mv,__mp,__mm} has at least __q trailing 0 bits.
      // __mv = 4 * __m2, so it always has at least two trailing 0 bits.
      __vrIsTrailingZeros = true;
      if (__acceptBounds) {
        // __mm = __mv - 1 - __mmShift, so it has 1 trailing 0 bit iff __mmShift == 1.
        __vmIsTrailingZeros = __mmShift == 1;
      } else {
        // __mp = __mv + 2, so it always has at least one trailing 0 bit.
        --__vp;
      }
    } else if (__q < 31) { // TRANSITION(ulfjack): Use a tighter bound here.
      __vrIsTrailingZeros = __multipleOfPowerOf2(__mv, __q - 1);
    }
  }

  // Step 4: Find the shortest decimal representation in the interval of valid representations.
  int32_t __removed = 0;
  uint32_t _Output;
  if (__vmIsTrailingZeros || __vrIsTrailingZeros) {
    // General case, which happens rarely (~4.0%).
    while (__vp / 10 > __vm / 10) {
#ifdef __clang__ // TRANSITION, LLVM-23106
      __vmIsTrailingZeros &= __vm - (__vm / 10) * 10 == 0;
#else
      __vmIsTrailingZeros &= __vm % 10 == 0;
#endif
      __vrIsTrailingZeros &= __lastRemovedDigit == 0;
      __lastRemovedDigit = static_cast<uint8_t>(__vr % 10);
      __vr /= 10;
      __vp /= 10;
      __vm /= 10;
      ++__removed;
    }
    if (__vmIsTrailingZeros) {
      while (__vm % 10 == 0) {
        __vrIsTrailingZeros &= __lastRemovedDigit == 0;
        __lastRemovedDigit = static_cast<uint8_t>(__vr % 10);
        __vr /= 10;
        __vp /= 10;
        __vm /= 10;
        ++__removed;
      }
    }
    if (__vrIsTrailingZeros && __lastRemovedDigit == 5 && __vr % 2 == 0) {
      // Round even if the exact number is .....50..0.
      __lastRemovedDigit = 4;
    }
    // We need to take __vr + 1 if __vr is outside bounds or we need to round up.
    _Output = __vr + ((__vr == __vm && (!__acceptBounds || !__vmIsTrailingZeros)) || __lastRemovedDigit >= 5);
  } else {
    // Specialized for the common case (~96.0%). Percentages below are relative to this.
    // Loop iterations below (approximately):
    // 0: 13.6%, 1: 70.7%, 2: 14.1%, 3: 1.39%, 4: 0.14%, 5+: 0.01%
    while (__vp / 10 > __vm / 10) {
      __lastRemovedDigit = static_cast<uint8_t>(__vr % 10);
      __vr /= 10;
      __vp /= 10;
      __vm /= 10;
      ++__removed;
    }
    // We need to take __vr + 1 if __vr is outside bounds or we need to round up.
    _Output = __vr + (__vr == __vm || __lastRemovedDigit >= 5);
  }
  const int32_t __exp = __e10 + __removed;

  __floating_decimal_32 __fd;
  __fd.__exponent = __exp;
  __fd.__mantissa = _Output;
  return __fd;
}
