static __inline__ unsigned int __DEFAULT_FN_ATTRS
_cvtmask32_u32(__mmask32 __A) {
  return (unsigned int)__builtin_ia32_kmovd((__mmask32)__A);
}
