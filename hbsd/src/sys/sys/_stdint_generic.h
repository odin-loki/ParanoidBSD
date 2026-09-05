/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Odin Loch <odin.loch@outlook.com.au>
 */

/*
 * PBSD: <machine/_stdint.h>, once.
 *
 * Five copies of this file exist in the tree - arm, arm64, powerpc, riscv
 * and x86, 970 lines - and each is the same seventy-one macros with the
 * numbers changed. They describe the target's integer types, which is
 * something the compiler already knows exactly: __INT64_MAX__,
 * __UINT32_C_SUFFIX__, __INTPTR_WIDTH__ and the rest are how it says so.
 *
 * So none of the numbers are written here. Every macro below is defined in
 * terms of a compiler predefine, which makes this file correct for a target
 * before anyone has ported anything to it - the numbers come from the
 * backend that will compile the code, not from a header someone edited.
 *
 * tools/stdint_generic_check.py preprocesses this and each existing
 * <machine/_stdint.h> for all six targets and compares the resulting
 * values, macro by macro. That is the whole argument for replacing them:
 * not that this is tidier, but that it expands to the same thing.
 */

#ifndef _SYS__STDINT_GENERIC_H_
#define	_SYS__STDINT_GENERIC_H_

/*
 * ISO/IEC 9899:1999 7.18.4 Macros for integer constants.
 *
 * The suffix predefines expand to nothing for the narrow types, so the
 * two-step concatenation is needed to paste an empty token.
 */
#define	__STDINT_JOIN1(a, b)	a ## b
#define	__STDINT_JOIN(a, b)	__STDINT_JOIN1(a, b)

#define	INT8_C(c)	__STDINT_JOIN(c, __INT8_C_SUFFIX__)
#define	INT16_C(c)	__STDINT_JOIN(c, __INT16_C_SUFFIX__)
#define	INT32_C(c)	__STDINT_JOIN(c, __INT32_C_SUFFIX__)
#define	INT64_C(c)	__STDINT_JOIN(c, __INT64_C_SUFFIX__)

#define	UINT8_C(c)	__STDINT_JOIN(c, __UINT8_C_SUFFIX__)
#define	UINT16_C(c)	__STDINT_JOIN(c, __UINT16_C_SUFFIX__)
#define	UINT32_C(c)	__STDINT_JOIN(c, __UINT32_C_SUFFIX__)
#define	UINT64_C(c)	__STDINT_JOIN(c, __UINT64_C_SUFFIX__)

#define	INTMAX_C(c)	__STDINT_JOIN(c, __INTMAX_C_SUFFIX__)
#define	UINTMAX_C(c)	__STDINT_JOIN(c, __UINTMAX_C_SUFFIX__)

/*
 * 7.18.2.1 Limits of exact-width integer types.
 *
 * The minimum is written as (-MAX - 1) rather than as a literal, because a
 * literal one below the maximum does not have the type it looks like it has.
 */
#define	INT8_MAX	__INT8_MAX__
#define	INT16_MAX	__INT16_MAX__
#define	INT32_MAX	__INT32_MAX__
#define	INT64_MAX	__INT64_MAX__

#define	INT8_MIN	(-INT8_MAX - 1)
#define	INT16_MIN	(-INT16_MAX - 1)
#define	INT32_MIN	(-INT32_MAX - 1)
#define	INT64_MIN	(-INT64_MAX - 1)

#define	UINT8_MAX	__UINT8_MAX__
#define	UINT16_MAX	__UINT16_MAX__
#define	UINT32_MAX	__UINT32_MAX__
#define	UINT64_MAX	__UINT64_MAX__

/* 7.18.2.2 Limits of minimum-width integer types. */
#define	INT_LEAST8_MAX	 __INT_LEAST8_MAX__
#define	INT_LEAST16_MAX	 __INT_LEAST16_MAX__
#define	INT_LEAST32_MAX	 __INT_LEAST32_MAX__
#define	INT_LEAST64_MAX	 __INT_LEAST64_MAX__

#define	INT_LEAST8_MIN	 (-INT_LEAST8_MAX - 1)
#define	INT_LEAST16_MIN	 (-INT_LEAST16_MAX - 1)
#define	INT_LEAST32_MIN	 (-INT_LEAST32_MAX - 1)
#define	INT_LEAST64_MIN	 (-INT_LEAST64_MAX - 1)

#define	UINT_LEAST8_MAX	 __UINT_LEAST8_MAX__
#define	UINT_LEAST16_MAX __UINT_LEAST16_MAX__
#define	UINT_LEAST32_MAX __UINT_LEAST32_MAX__
#define	UINT_LEAST64_MAX __UINT_LEAST64_MAX__

/*
 * 7.18.2.3 Limits of fastest minimum-width integer types.
 *
 * These do not come from the compiler. FreeBSD's int_fast8_t is int32_t -
 * <machine/_types.h> says so on all six architectures, identically - and
 * clang's is a signed char, so __INT_FAST8_MAX__ is 127 where FreeBSD's
 * limit is 0x7fffffff. Using the compiler's answer here would quietly
 * change the ABI of a standard type. The tree's own choice, which happens
 * to be machine-independent, is used instead.
 */
#define	INT_FAST8_MAX	 INT32_MAX
#define	INT_FAST16_MAX	 INT32_MAX
#define	INT_FAST32_MAX	 INT32_MAX
#define	INT_FAST64_MAX	 INT64_MAX

#define	INT_FAST8_MIN	 INT32_MIN
#define	INT_FAST16_MIN	 INT32_MIN
#define	INT_FAST32_MIN	 INT32_MIN
#define	INT_FAST64_MIN	 INT64_MIN

#define	UINT_FAST8_MAX	 UINT32_MAX
#define	UINT_FAST16_MAX	 UINT32_MAX
#define	UINT_FAST32_MAX	 UINT32_MAX
#define	UINT_FAST64_MAX	 UINT64_MAX

/* 7.18.2.4 Limits of integer types capable of holding object pointers. */
#define	INTPTR_MAX	__INTPTR_MAX__
#define	INTPTR_MIN	(-INTPTR_MAX - 1)
#define	UINTPTR_MAX	__UINTPTR_MAX__

/* 7.18.2.5 Limits of greatest-width integer types. */
#define	INTMAX_MAX	__INTMAX_MAX__
#define	INTMAX_MIN	(-INTMAX_MAX - 1)
#define	UINTMAX_MAX	__UINTMAX_MAX__

/* 7.18.3 Limits of other integer types. */
#define	PTRDIFF_MAX	__PTRDIFF_MAX__
#define	PTRDIFF_MIN	(-PTRDIFF_MAX - 1)

/*
 * SIG_ATOMIC_MIN, SIG_ATOMIC_MAX and SIG_ATOMIC_WIDTH are NOT here.
 *
 * sig_atomic_t is the one type in this header whose width is a per-
 * architecture decision that nothing else predicts: 64-bit on amd64, arm64
 * and riscv, 32-bit on i386, arm and powerpc - powerpc64 included, so it
 * does not follow __LP64__ either. clang says 32-bit for amd64, which is
 * not what FreeBSD's ABI says.
 *
 * So <machine/_stdint.h> keeps those three and nothing else. Three lines
 * per architecture instead of about a hundred and eighty is where this
 * lands, and it lands there because it was checked rather than assumed.
 */

#define	SIZE_MAX	__SIZE_MAX__

/*
 * WINT_MIN and WINT_MAX.
 *
 * Not from the compiler, and this one is worth naming. FreeBSD's wint_t is
 * __ct_rune_t, which is int - sys/sys/_types.h, machine-independently, on
 * every architecture. clang agrees on five targets and not on riscv, where
 * it defines __WINT_UNSIGNED__ and __WINT_MAX__ as 4294967295U, following
 * the RISC-V psABI rather than FreeBSD.
 *
 * The first version of this file asked the compiler and came out with
 * WINT_MIN as 0 on riscv and INT32_MIN everywhere else - a signedness
 * change in a standard type, on one architecture, from a header that was
 * meant to make them all the same.
 */
#define	WINT_MIN	INT32_MIN
#define	WINT_MAX	INT32_MAX

/*
 * C23 7.22.2 and 7.22.3, widths.
 *
 * Guarded the way every <machine/_stdint.h> guards them: these names are
 * C23's, and defining them in a C99 translation unit is namespace
 * pollution rather than a feature.
 */
#if __ISO_C_VISIBLE >= 2023
#define	INT_FAST8_WIDTH		32
#define	INT_FAST16_WIDTH	32
#define	INT_FAST32_WIDTH	32
#define	INT_FAST64_WIDTH	64
#define	INTPTR_WIDTH		__INTPTR_WIDTH__
#define	INTMAX_WIDTH		__INTMAX_WIDTH__
#define	PTRDIFF_WIDTH		__PTRDIFF_WIDTH__
#define	SIZE_WIDTH		__SIZE_WIDTH__
#define	WCHAR_WIDTH		__WCHAR_WIDTH__
#define	WINT_WIDTH		32
#endif /* __ISO_C_VISIBLE >= 2023 */

#endif /* _SYS__STDINT_GENERIC_H_ */
