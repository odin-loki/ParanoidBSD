#include "prelude.h"
#undef __AVX512BWINTRIN_H
#define __AVX512BWINTRIN_H
#undef __DEFAULT_FN_ATTRS512
#define __DEFAULT_FN_ATTRS512                                                     __attribute__((__always_inline__, __nodebug__,                                                 __target__("avx512bw,evex512"), __min_vector_width__(512)))
#undef __DEFAULT_FN_ATTRS
#define __DEFAULT_FN_ATTRS                                                        __attribute__((__always_inline__, __nodebug__,                                                 __target__("avx512bw,no-evex512")))
#undef _kshiftli_mask32
#define _kshiftli_mask32(A, I)    ((__mmask32)__builtin_ia32_kshiftlisi((__mmask32)(A), (unsigned int)(I)))
#undef _kshiftri_mask32
#define _kshiftri_mask32(A, I)    ((__mmask32)__builtin_ia32_kshiftrisi((__mmask32)(A), (unsigned int)(I)))
#undef _kshiftli_mask64
#define _kshiftli_mask64(A, I)    ((__mmask64)__builtin_ia32_kshiftlidi((__mmask64)(A), (unsigned int)(I)))
#undef _kshiftri_mask64
#define _kshiftri_mask64(A, I)    ((__mmask64)__builtin_ia32_kshiftridi((__mmask64)(A), (unsigned int)(I)))
#undef _mm512_cmp_epi8_mask
#define _mm512_cmp_epi8_mask(a, b, p)    ((__mmask64)__builtin_ia32_cmpb512_mask((__v64qi)(__m512i)(a),                                            (__v64qi)(__m512i)(b), (int)(p),                                            (__mmask64)-1))
#undef _mm512_mask_cmp_epi8_mask
#define _mm512_mask_cmp_epi8_mask(m, a, b, p)    ((__mmask64)__builtin_ia32_cmpb512_mask((__v64qi)(__m512i)(a),                                            (__v64qi)(__m512i)(b), (int)(p),                                            (__mmask64)(m)))
#undef _mm512_cmp_epu8_mask
#define _mm512_cmp_epu8_mask(a, b, p)    ((__mmask64)__builtin_ia32_ucmpb512_mask((__v64qi)(__m512i)(a),                                             (__v64qi)(__m512i)(b), (int)(p),                                             (__mmask64)-1))
#undef _mm512_mask_cmp_epu8_mask
#define _mm512_mask_cmp_epu8_mask(m, a, b, p)    ((__mmask64)__builtin_ia32_ucmpb512_mask((__v64qi)(__m512i)(a),                                             (__v64qi)(__m512i)(b), (int)(p),                                             (__mmask64)(m)))
#undef _mm512_cmp_epi16_mask
#define _mm512_cmp_epi16_mask(a, b, p)    ((__mmask32)__builtin_ia32_cmpw512_mask((__v32hi)(__m512i)(a),                                            (__v32hi)(__m512i)(b), (int)(p),                                            (__mmask32)-1))
#undef _mm512_mask_cmp_epi16_mask
#define _mm512_mask_cmp_epi16_mask(m, a, b, p)    ((__mmask32)__builtin_ia32_cmpw512_mask((__v32hi)(__m512i)(a),                                            (__v32hi)(__m512i)(b), (int)(p),                                            (__mmask32)(m)))
#undef _mm512_cmp_epu16_mask
#define _mm512_cmp_epu16_mask(a, b, p)    ((__mmask32)__builtin_ia32_ucmpw512_mask((__v32hi)(__m512i)(a),                                             (__v32hi)(__m512i)(b), (int)(p),                                             (__mmask32)-1))
#undef _mm512_mask_cmp_epu16_mask
#define _mm512_mask_cmp_epu16_mask(m, a, b, p)    ((__mmask32)__builtin_ia32_ucmpw512_mask((__v32hi)(__m512i)(a),                                             (__v32hi)(__m512i)(b), (int)(p),                                             (__mmask32)(m)))
#undef _mm512_cmpeq_epi8_mask
#define _mm512_cmpeq_epi8_mask(A, B)      _mm512_cmp_epi8_mask((A), (B), _MM_CMPINT_EQ)
#undef _mm512_mask_cmpeq_epi8_mask
#define _mm512_mask_cmpeq_epi8_mask(k, A, B)      _mm512_mask_cmp_epi8_mask((k), (A), (B), _MM_CMPINT_EQ)
#undef _mm512_cmpge_epi8_mask
#define _mm512_cmpge_epi8_mask(A, B)      _mm512_cmp_epi8_mask((A), (B), _MM_CMPINT_GE)
#undef _mm512_mask_cmpge_epi8_mask
#define _mm512_mask_cmpge_epi8_mask(k, A, B)      _mm512_mask_cmp_epi8_mask((k), (A), (B), _MM_CMPINT_GE)
#undef _mm512_cmpgt_epi8_mask
#define _mm512_cmpgt_epi8_mask(A, B)      _mm512_cmp_epi8_mask((A), (B), _MM_CMPINT_GT)
#undef _mm512_mask_cmpgt_epi8_mask
#define _mm512_mask_cmpgt_epi8_mask(k, A, B)      _mm512_mask_cmp_epi8_mask((k), (A), (B), _MM_CMPINT_GT)
#undef _mm512_cmple_epi8_mask
#define _mm512_cmple_epi8_mask(A, B)      _mm512_cmp_epi8_mask((A), (B), _MM_CMPINT_LE)
#undef _mm512_mask_cmple_epi8_mask
#define _mm512_mask_cmple_epi8_mask(k, A, B)      _mm512_mask_cmp_epi8_mask((k), (A), (B), _MM_CMPINT_LE)
#undef _mm512_cmplt_epi8_mask
#define _mm512_cmplt_epi8_mask(A, B)      _mm512_cmp_epi8_mask((A), (B), _MM_CMPINT_LT)
#undef _mm512_mask_cmplt_epi8_mask
#define _mm512_mask_cmplt_epi8_mask(k, A, B)      _mm512_mask_cmp_epi8_mask((k), (A), (B), _MM_CMPINT_LT)
#undef _mm512_cmpneq_epi8_mask
#define _mm512_cmpneq_epi8_mask(A, B)      _mm512_cmp_epi8_mask((A), (B), _MM_CMPINT_NE)
#undef _mm512_mask_cmpneq_epi8_mask
#define _mm512_mask_cmpneq_epi8_mask(k, A, B)      _mm512_mask_cmp_epi8_mask((k), (A), (B), _MM_CMPINT_NE)
#undef _mm512_cmpeq_epu8_mask
#define _mm512_cmpeq_epu8_mask(A, B)      _mm512_cmp_epu8_mask((A), (B), _MM_CMPINT_EQ)
#undef _mm512_mask_cmpeq_epu8_mask
#define _mm512_mask_cmpeq_epu8_mask(k, A, B)      _mm512_mask_cmp_epu8_mask((k), (A), (B), _MM_CMPINT_EQ)
#undef _mm512_cmpge_epu8_mask
#define _mm512_cmpge_epu8_mask(A, B)      _mm512_cmp_epu8_mask((A), (B), _MM_CMPINT_GE)
#undef _mm512_mask_cmpge_epu8_mask
#define _mm512_mask_cmpge_epu8_mask(k, A, B)      _mm512_mask_cmp_epu8_mask((k), (A), (B), _MM_CMPINT_GE)
#undef _mm512_cmpgt_epu8_mask
#define _mm512_cmpgt_epu8_mask(A, B)      _mm512_cmp_epu8_mask((A), (B), _MM_CMPINT_GT)
#undef _mm512_mask_cmpgt_epu8_mask
#define _mm512_mask_cmpgt_epu8_mask(k, A, B)      _mm512_mask_cmp_epu8_mask((k), (A), (B), _MM_CMPINT_GT)
#undef _mm512_cmple_epu8_mask
#define _mm512_cmple_epu8_mask(A, B)      _mm512_cmp_epu8_mask((A), (B), _MM_CMPINT_LE)
#undef _mm512_mask_cmple_epu8_mask
#define _mm512_mask_cmple_epu8_mask(k, A, B)      _mm512_mask_cmp_epu8_mask((k), (A), (B), _MM_CMPINT_LE)
#undef _mm512_cmplt_epu8_mask
#define _mm512_cmplt_epu8_mask(A, B)      _mm512_cmp_epu8_mask((A), (B), _MM_CMPINT_LT)
#undef _mm512_mask_cmplt_epu8_mask
#define _mm512_mask_cmplt_epu8_mask(k, A, B)      _mm512_mask_cmp_epu8_mask((k), (A), (B), _MM_CMPINT_LT)
#undef _mm512_cmpneq_epu8_mask
#define _mm512_cmpneq_epu8_mask(A, B)      _mm512_cmp_epu8_mask((A), (B), _MM_CMPINT_NE)
#undef _mm512_mask_cmpneq_epu8_mask
#define _mm512_mask_cmpneq_epu8_mask(k, A, B)      _mm512_mask_cmp_epu8_mask((k), (A), (B), _MM_CMPINT_NE)
#undef _mm512_cmpeq_epi16_mask
#define _mm512_cmpeq_epi16_mask(A, B)      _mm512_cmp_epi16_mask((A), (B), _MM_CMPINT_EQ)
#undef _mm512_mask_cmpeq_epi16_mask
#define _mm512_mask_cmpeq_epi16_mask(k, A, B)      _mm512_mask_cmp_epi16_mask((k), (A), (B), _MM_CMPINT_EQ)
#undef _mm512_cmpge_epi16_mask
#define _mm512_cmpge_epi16_mask(A, B)      _mm512_cmp_epi16_mask((A), (B), _MM_CMPINT_GE)
#undef _mm512_mask_cmpge_epi16_mask
#define _mm512_mask_cmpge_epi16_mask(k, A, B)      _mm512_mask_cmp_epi16_mask((k), (A), (B), _MM_CMPINT_GE)
#undef _mm512_cmpgt_epi16_mask
#define _mm512_cmpgt_epi16_mask(A, B)      _mm512_cmp_epi16_mask((A), (B), _MM_CMPINT_GT)
#undef _mm512_mask_cmpgt_epi16_mask
#define _mm512_mask_cmpgt_epi16_mask(k, A, B)      _mm512_mask_cmp_epi16_mask((k), (A), (B), _MM_CMPINT_GT)
#undef _mm512_cmple_epi16_mask
#define _mm512_cmple_epi16_mask(A, B)      _mm512_cmp_epi16_mask((A), (B), _MM_CMPINT_LE)
#undef _mm512_mask_cmple_epi16_mask
#define _mm512_mask_cmple_epi16_mask(k, A, B)      _mm512_mask_cmp_epi16_mask((k), (A), (B), _MM_CMPINT_LE)
#undef _mm512_cmplt_epi16_mask
#define _mm512_cmplt_epi16_mask(A, B)      _mm512_cmp_epi16_mask((A), (B), _MM_CMPINT_LT)
#undef _mm512_mask_cmplt_epi16_mask
#define _mm512_mask_cmplt_epi16_mask(k, A, B)      _mm512_mask_cmp_epi16_mask((k), (A), (B), _MM_CMPINT_LT)
#undef _mm512_cmpneq_epi16_mask
#define _mm512_cmpneq_epi16_mask(A, B)      _mm512_cmp_epi16_mask((A), (B), _MM_CMPINT_NE)
#undef _mm512_mask_cmpneq_epi16_mask
#define _mm512_mask_cmpneq_epi16_mask(k, A, B)      _mm512_mask_cmp_epi16_mask((k), (A), (B), _MM_CMPINT_NE)
#undef _mm512_cmpeq_epu16_mask
#define _mm512_cmpeq_epu16_mask(A, B)      _mm512_cmp_epu16_mask((A), (B), _MM_CMPINT_EQ)
#undef _mm512_mask_cmpeq_epu16_mask
#define _mm512_mask_cmpeq_epu16_mask(k, A, B)      _mm512_mask_cmp_epu16_mask((k), (A), (B), _MM_CMPINT_EQ)
#undef _mm512_cmpge_epu16_mask
#define _mm512_cmpge_epu16_mask(A, B)      _mm512_cmp_epu16_mask((A), (B), _MM_CMPINT_GE)
#undef _mm512_mask_cmpge_epu16_mask
#define _mm512_mask_cmpge_epu16_mask(k, A, B)      _mm512_mask_cmp_epu16_mask((k), (A), (B), _MM_CMPINT_GE)
#undef _mm512_cmpgt_epu16_mask
#define _mm512_cmpgt_epu16_mask(A, B)      _mm512_cmp_epu16_mask((A), (B), _MM_CMPINT_GT)
#undef _mm512_mask_cmpgt_epu16_mask
#define _mm512_mask_cmpgt_epu16_mask(k, A, B)      _mm512_mask_cmp_epu16_mask((k), (A), (B), _MM_CMPINT_GT)
#undef _mm512_cmple_epu16_mask
#define _mm512_cmple_epu16_mask(A, B)      _mm512_cmp_epu16_mask((A), (B), _MM_CMPINT_LE)
#undef _mm512_mask_cmple_epu16_mask
#define _mm512_mask_cmple_epu16_mask(k, A, B)      _mm512_mask_cmp_epu16_mask((k), (A), (B), _MM_CMPINT_LE)
#undef _mm512_cmplt_epu16_mask
#define _mm512_cmplt_epu16_mask(A, B)      _mm512_cmp_epu16_mask((A), (B), _MM_CMPINT_LT)
#undef _mm512_mask_cmplt_epu16_mask
#define _mm512_mask_cmplt_epu16_mask(k, A, B)      _mm512_mask_cmp_epu16_mask((k), (A), (B), _MM_CMPINT_LT)
#undef _mm512_cmpneq_epu16_mask
#define _mm512_cmpneq_epu16_mask(A, B)      _mm512_cmp_epu16_mask((A), (B), _MM_CMPINT_NE)
#undef _mm512_mask_cmpneq_epu16_mask
#define _mm512_mask_cmpneq_epu16_mask(k, A, B)      _mm512_mask_cmp_epu16_mask((k), (A), (B), _MM_CMPINT_NE)
#undef _mm512_shufflehi_epi16
#define _mm512_shufflehi_epi16(A, imm)    ((__m512i)__builtin_ia32_pshufhw512((__v32hi)(__m512i)(A), (int)(imm)))
#undef _mm512_mask_shufflehi_epi16
#define _mm512_mask_shufflehi_epi16(W, U, A, imm)    ((__m512i)__builtin_ia32_selectw_512((__mmask32)(U),                                         (__v32hi)_mm512_shufflehi_epi16((A),                                                                         (imm)),                                         (__v32hi)(__m512i)(W)))
#undef _mm512_maskz_shufflehi_epi16
#define _mm512_maskz_shufflehi_epi16(U, A, imm)    ((__m512i)__builtin_ia32_selectw_512((__mmask32)(U),                                         (__v32hi)_mm512_shufflehi_epi16((A),                                                                         (imm)),                                         (__v32hi)_mm512_setzero_si512()))
#undef _mm512_shufflelo_epi16
#define _mm512_shufflelo_epi16(A, imm)    ((__m512i)__builtin_ia32_pshuflw512((__v32hi)(__m512i)(A), (int)(imm)))
#undef _mm512_mask_shufflelo_epi16
#define _mm512_mask_shufflelo_epi16(W, U, A, imm)    ((__m512i)__builtin_ia32_selectw_512((__mmask32)(U),                                         (__v32hi)_mm512_shufflelo_epi16((A),                                                                         (imm)),                                         (__v32hi)(__m512i)(W)))
#undef _mm512_maskz_shufflelo_epi16
#define _mm512_maskz_shufflelo_epi16(U, A, imm)    ((__m512i)__builtin_ia32_selectw_512((__mmask32)(U),                                         (__v32hi)_mm512_shufflelo_epi16((A),                                                                         (imm)),                                         (__v32hi)_mm512_setzero_si512()))
#undef _mm512_bslli_epi128
#define _mm512_bslli_epi128(a, imm)    ((__m512i)__builtin_ia32_pslldqi512_byteshift((__v8di)(__m512i)(a), (int)(imm)))
#undef _mm512_bsrli_epi128
#define _mm512_bsrli_epi128(a, imm)    ((__m512i)__builtin_ia32_psrldqi512_byteshift((__v8di)(__m512i)(a), (int)(imm)))
#undef _mm512_alignr_epi8
#define _mm512_alignr_epi8(A, B, N)    ((__m512i)__builtin_ia32_palignr512((__v64qi)(__m512i)(A),                                        (__v64qi)(__m512i)(B), (int)(N)))
#undef _mm512_mask_alignr_epi8
#define _mm512_mask_alignr_epi8(W, U, A, B, N)    ((__m512i)__builtin_ia32_selectb_512((__mmask64)(U),                                (__v64qi)_mm512_alignr_epi8((A), (B), (int)(N)),                                (__v64qi)(__m512i)(W)))
#undef _mm512_maskz_alignr_epi8
#define _mm512_maskz_alignr_epi8(U, A, B, N)    ((__m512i)__builtin_ia32_selectb_512((__mmask64)(U),                                (__v64qi)_mm512_alignr_epi8((A), (B), (int)(N)),                                (__v64qi)(__m512i)_mm512_setzero_si512()))
#undef _mm512_dbsad_epu8
#define _mm512_dbsad_epu8(A, B, imm)    ((__m512i)__builtin_ia32_dbpsadbw512((__v64qi)(__m512i)(A),                                         (__v64qi)(__m512i)(B), (int)(imm)))
#undef _mm512_mask_dbsad_epu8
#define _mm512_mask_dbsad_epu8(W, U, A, B, imm)    ((__m512i)__builtin_ia32_selectw_512((__mmask32)(U),                                    (__v32hi)_mm512_dbsad_epu8((A), (B), (imm)),                                    (__v32hi)(__m512i)(W)))
#undef _mm512_maskz_dbsad_epu8
#define _mm512_maskz_dbsad_epu8(U, A, B, imm)    ((__m512i)__builtin_ia32_selectw_512((__mmask32)(U),                                    (__v32hi)_mm512_dbsad_epu8((A), (B), (imm)),                                    (__v32hi)_mm512_setzero_si512()))
typedef unsigned int __mmask32;
typedef unsigned long long __mmask64;
/*@ requires true; ensures \result == __A; */
static __inline__ unsigned int __DEFAULT_FN_ATTRS
_cvtmask32_u32(__mmask32 __A) {
  return (unsigned int)__builtin_ia32_kmovd((__mmask32)__A);
}
