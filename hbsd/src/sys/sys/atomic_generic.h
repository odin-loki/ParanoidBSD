/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Odin Loch <odin.loch@outlook.com.au>
 */

/*
 * PBSD: atomic(9), once, for every architecture.
 *
 * The tree has six hand-written implementations of this interface -
 * sys/<arch>/include/atomic.h, 5,107 lines, 145 inline-asm sites, 247 of the
 * operation x width x ordering space written out literally and the rest
 * produced by six unrelated sets of macros. tools/atomic_survey.py has the
 * per-architecture figures. A seventh architecture means writing it a
 * seventh time, and lock-free atomics are the hardest part of a port to get
 * right and the least forgiving when it is wrong.
 *
 * Clang already knows the instruction for every one of these on every target
 * PBSD builds. That is what __atomic_* is: the compiler's own model of the
 * memory ordering, lowered per target by the backend that also compiles the
 * hand-written version. So this file is the whole interface expressed in
 * those builtins, and the per-architecture work becomes deciding which
 * widths the hardware can do lock-free rather than writing the operations.
 *
 * Mapping from FreeBSD's ordering suffixes to the C11 model:
 *
 *   (none)   __ATOMIC_RELAXED   atomic, no ordering - what atomic(9) means
 *                               by an unsuffixed operation
 *   _acq_    __ATOMIC_ACQUIRE
 *   _rel_    __ATOMIC_RELEASE
 *   _barr_   __ATOMIC_SEQ_CST   x86's full-barrier forms
 *
 * What this does not do, measured rather than assumed:
 *
 *   i386 64-bit is not lock-free here. The i386 ABI aligns uint64_t to 4,
 *   and __atomic will not assume the 8 that CMPXCHG8B wants, so it emits a
 *   call to libatomic - a symbol the kernel does not have. Telling the
 *   compiler the alignment is 8 does produce CMPXCHG8B (checked), but it is
 *   a promise about the caller's object that a header cannot make, and
 *   clang says so with -Walign-mismatch. sys/i386/include/atomic.h makes
 *   that promise for itself and keeps its own 64-bit block; everything
 *   below 64 bits, on every architecture including i386, comes from here.
 *
 *   Every other target does all four widths lock-free, armv7 included -
 *   AAPCS aligns 64-bit types to 8, so LDREXD/STREXD is available to the
 *   compiler without anyone asserting anything.
 *
 * tools/atomic_generic_check.py is where those statements come from. It
 * compiles this header for all six targets with a shimmed <sys/types.h> and
 * nothing else on the include path, one object per width, and reads the
 * undefined symbols back out.
 *
 * NOT YET INCLUDED BY ANY ARCHITECTURE. This is the implementation and the
 * measurement; the switch-over is a separate change and wants a green
 * buildworld behind it.
 */

#ifndef _SYS_ATOMIC_GENERIC_H_
#define	_SYS_ATOMIC_GENERIC_H_

#include <sys/cdefs.h>
#include <sys/types.h>

#ifndef __has_builtin
#define	__has_builtin(x)	0
#endif

#if !__has_builtin(__atomic_load_n)
#error "sys/atomic_generic.h needs a compiler with the __atomic builtins"
#endif

/*
 * One operation, one width, one ordering. Every macro below expands to
 * static inline functions with exactly the signatures atomic(9) documents,
 * so a translation unit cannot tell this from the hand-written header.
 */

#define	_ATOMIC_GEN_VOID(op, expr, T, W, sfx, mo)			\
static __inline void							\
atomic_##op##sfx##_##W(volatile T *p, T v)				\
{									\
	(void)expr(p, v, mo);						\
}

#define	_ATOMIC_GEN_FETCH(op, expr, T, W, sfx, mo)			\
static __inline T							\
atomic_##op##sfx##_##W(volatile T *p, T v)				\
{									\
	return (expr(p, v, mo));					\
}

/*
 * cmpset returns 1 on success. Strong, because atomic(9) callers use the
 * return value to decide and a spurious failure would be a behaviour change
 * rather than a slowdown.
 */
#define	_ATOMIC_GEN_CMPSET(T, W, sfx, mo)				\
static __inline int							\
atomic_cmpset##sfx##_##W(volatile T *p, T cmp, T newv)			\
{									\
	return (__atomic_compare_exchange_n(p, &cmp, newv, 0,		\
	    mo, __ATOMIC_RELAXED) ? 1 : 0);				\
}

/*
 * fcmpset writes the observed value back through cmp on failure, and is
 * allowed to fail spuriously - that is the whole difference, and it is why
 * callers wrap it in a loop.
 */
#define	_ATOMIC_GEN_FCMPSET(T, W, sfx, mo)				\
static __inline int							\
atomic_fcmpset##sfx##_##W(volatile T *p, T *cmp, T newv)		\
{									\
	return (__atomic_compare_exchange_n(p, cmp, newv, 1,		\
	    mo, __ATOMIC_RELAXED) ? 1 : 0);				\
}

#define	_ATOMIC_GEN_LOAD(T, W, sfx, mo)					\
static __inline T							\
atomic_load##sfx##_##W(const volatile T *p)				\
{									\
	return (__atomic_load_n(p, mo));				\
}

#define	_ATOMIC_GEN_STORE(T, W, sfx, mo)				\
static __inline void							\
atomic_store##sfx##_##W(volatile T *p, T v)				\
{									\
	__atomic_store_n(p, v, mo);					\
}

#define	_ATOMIC_GEN_SWAP(T, W)						\
static __inline T							\
atomic_swap_##W(volatile T *p, T v)					\
{									\
	return (__atomic_exchange_n(p, v, __ATOMIC_RELAXED));		\
}									\
									\
static __inline T							\
atomic_readandclear_##W(volatile T *p)					\
{									\
	return (__atomic_exchange_n(p, (T)0, __ATOMIC_RELAXED));	\
}

/*
 * testandset/testandclear take a bit number, and atomic(9) says it is taken
 * modulo the width - the callers rely on that, and the x86 instruction does
 * it in hardware. The mask is built in the operand type so a shift by the
 * width is not undefined here either.
 */
#define	_ATOMIC_GEN_BIT(T, W, sfx, mo)					\
static __inline int							\
atomic_testandset##sfx##_##W(volatile T *p, u_int bit)			\
{									\
	T mask = (T)(((T)1) << (bit & (W - 1)));			\
	return ((__atomic_fetch_or(p, mask, mo) & mask) != 0);		\
}									\
									\
static __inline int							\
atomic_testandclear##sfx##_##W(volatile T *p, u_int bit)		\
{									\
	T mask = (T)(((T)1) << (bit & (W - 1)));			\
	return ((__atomic_fetch_and(p, (T)~mask, mo) & mask) != 0);	\
}

/* atomic_clear clears the bits given, so it is an and-not. */
#define	_atomic_gen_andnot(p, v, mo)	__atomic_fetch_and(p, (__typeof(v))~(v), mo)

#define	_ATOMIC_GEN_WIDTH(T, W)						\
	_ATOMIC_GEN_VOID(add,	   __atomic_fetch_add, T, W,	 , __ATOMIC_RELAXED) \
	_ATOMIC_GEN_VOID(add,	   __atomic_fetch_add, T, W, _acq, __ATOMIC_ACQUIRE) \
	_ATOMIC_GEN_VOID(add,	   __atomic_fetch_add, T, W, _rel, __ATOMIC_RELEASE) \
	_ATOMIC_GEN_VOID(add,	   __atomic_fetch_add, T, W, _barr, __ATOMIC_SEQ_CST) \
	_ATOMIC_GEN_VOID(subtract, __atomic_fetch_sub, T, W,	 , __ATOMIC_RELAXED) \
	_ATOMIC_GEN_VOID(subtract, __atomic_fetch_sub, T, W, _acq, __ATOMIC_ACQUIRE) \
	_ATOMIC_GEN_VOID(subtract, __atomic_fetch_sub, T, W, _rel, __ATOMIC_RELEASE) \
	_ATOMIC_GEN_VOID(subtract, __atomic_fetch_sub, T, W, _barr, __ATOMIC_SEQ_CST) \
	_ATOMIC_GEN_VOID(set,	   __atomic_fetch_or,  T, W,	 , __ATOMIC_RELAXED) \
	_ATOMIC_GEN_VOID(set,	   __atomic_fetch_or,  T, W, _acq, __ATOMIC_ACQUIRE) \
	_ATOMIC_GEN_VOID(set,	   __atomic_fetch_or,  T, W, _rel, __ATOMIC_RELEASE) \
	_ATOMIC_GEN_VOID(set,	   __atomic_fetch_or,  T, W, _barr, __ATOMIC_SEQ_CST) \
	_ATOMIC_GEN_VOID(clear,	   _atomic_gen_andnot, T, W,	 , __ATOMIC_RELAXED) \
	_ATOMIC_GEN_VOID(clear,	   _atomic_gen_andnot, T, W, _acq, __ATOMIC_ACQUIRE) \
	_ATOMIC_GEN_VOID(clear,	   _atomic_gen_andnot, T, W, _rel, __ATOMIC_RELEASE) \
	_ATOMIC_GEN_VOID(clear,	   _atomic_gen_andnot, T, W, _barr, __ATOMIC_SEQ_CST) \
	_ATOMIC_GEN_FETCH(fetchadd, __atomic_fetch_add, T, W,	  , __ATOMIC_RELAXED) \
	_ATOMIC_GEN_FETCH(fetchadd, __atomic_fetch_add, T, W, _acq, __ATOMIC_ACQUIRE) \
	_ATOMIC_GEN_FETCH(fetchadd, __atomic_fetch_add, T, W, _rel, __ATOMIC_RELEASE) \
	_ATOMIC_GEN_CMPSET(T, W,     , __ATOMIC_RELAXED)		\
	_ATOMIC_GEN_CMPSET(T, W, _acq, __ATOMIC_ACQUIRE)		\
	_ATOMIC_GEN_CMPSET(T, W, _rel, __ATOMIC_RELEASE)		\
	_ATOMIC_GEN_FCMPSET(T, W,     , __ATOMIC_RELAXED)		\
	_ATOMIC_GEN_FCMPSET(T, W, _acq, __ATOMIC_ACQUIRE)		\
	_ATOMIC_GEN_FCMPSET(T, W, _rel, __ATOMIC_RELEASE)		\
	_ATOMIC_GEN_LOAD(T, W,	   , __ATOMIC_RELAXED)			\
	_ATOMIC_GEN_LOAD(T, W, _acq, __ATOMIC_ACQUIRE)			\
	_ATOMIC_GEN_STORE(T, W,	    , __ATOMIC_RELAXED)			\
	_ATOMIC_GEN_STORE(T, W, _rel, __ATOMIC_RELEASE)			\
	_ATOMIC_GEN_SWAP(T, W)						\
	_ATOMIC_GEN_BIT(T, W,	  , __ATOMIC_RELAXED)			\
	_ATOMIC_GEN_BIT(T, W, _acq, __ATOMIC_ACQUIRE)

_ATOMIC_GEN_WIDTH(uint8_t,  8)
_ATOMIC_GEN_WIDTH(uint16_t, 16)
_ATOMIC_GEN_WIDTH(uint32_t, 32)
_ATOMIC_GEN_WIDTH(uint64_t, 64)

static __inline void
atomic_thread_fence_acq(void)
{
	__atomic_thread_fence(__ATOMIC_ACQUIRE);
}

static __inline void
atomic_thread_fence_rel(void)
{
	__atomic_thread_fence(__ATOMIC_RELEASE);
}

static __inline void
atomic_thread_fence_acq_rel(void)
{
	__atomic_thread_fence(__ATOMIC_ACQ_REL);
}

static __inline void
atomic_thread_fence_seq_cst(void)
{
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

/*
 * The C-type spellings.
 *
 * atomic(9) has char/short/int/long/ptr alongside the fixed widths, and they
 * are the same operations under another name - which is why every
 * architecture's header ends in a block like this one, and why only one of
 * them needs to exist.
 *
 * They are forwarding functions rather than #defines because the C type and
 * the fixed-width type are not always the same type even when they are the
 * same width: on ILP32, u_long is unsigned long and uint32_t is unsigned
 * int. A macro would pass the wrong pointer type and the diagnostic would
 * point at the caller.
 *
 * long and ptr are the only widths that move, and they move with the data
 * model rather than with the architecture, so __LP64__ decides both.
 */
#define	_ATOMIC_CTYPE_VOID(op, sfx, CT, cname, W)			\
static __inline void							\
atomic_##op##sfx##_##cname(volatile CT *p, CT v)			\
{									\
	atomic_##op##sfx##_##W((volatile uint##W##_t *)p,		\
	    (uint##W##_t)v);						\
}

#define	_ATOMIC_CTYPE_FETCH(op, sfx, CT, cname, W)			\
static __inline CT							\
atomic_##op##sfx##_##cname(volatile CT *p, CT v)			\
{									\
	return ((CT)atomic_##op##sfx##_##W((volatile uint##W##_t *)p,	\
	    (uint##W##_t)v));						\
}

#define	_ATOMIC_CTYPE_CMPSET(sfx, CT, cname, W)				\
static __inline int							\
atomic_cmpset##sfx##_##cname(volatile CT *p, CT cmp, CT newv)		\
{									\
	return (atomic_cmpset##sfx##_##W((volatile uint##W##_t *)p,	\
	    (uint##W##_t)cmp, (uint##W##_t)newv));			\
}									\
									\
static __inline int							\
atomic_fcmpset##sfx##_##cname(volatile CT *p, CT *cmp, CT newv)		\
{									\
	return (atomic_fcmpset##sfx##_##W((volatile uint##W##_t *)p,	\
	    (uint##W##_t *)cmp, (uint##W##_t)newv));			\
}

#define	_ATOMIC_CTYPE_BIT(sfx, CT, cname, W)				\
static __inline int							\
atomic_testandset##sfx##_##cname(volatile CT *p, u_int bit)		\
{									\
	return (atomic_testandset##sfx##_##W(				\
	    (volatile uint##W##_t *)p, bit));				\
}

#define	_ATOMIC_GEN_CTYPE(CT, cname, W)					\
	_ATOMIC_CTYPE_VOID(add,	     , CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(add,	 _acq, CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(add,	 _rel, CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(add,	_barr, CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(subtract,	  , CT, cname, W)		\
	_ATOMIC_CTYPE_VOID(subtract,  _acq, CT, cname, W)		\
	_ATOMIC_CTYPE_VOID(subtract,  _rel, CT, cname, W)		\
	_ATOMIC_CTYPE_VOID(subtract, _barr, CT, cname, W)		\
	_ATOMIC_CTYPE_VOID(set,	     , CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(set,	 _acq, CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(set,	 _rel, CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(set,	_barr, CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(clear,      , CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(clear,  _acq, CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(clear,  _rel, CT, cname, W)			\
	_ATOMIC_CTYPE_VOID(clear, _barr, CT, cname, W)			\
	_ATOMIC_CTYPE_FETCH(fetchadd,	  , CT, cname, W)		\
	_ATOMIC_CTYPE_FETCH(fetchadd, _acq, CT, cname, W)		\
	_ATOMIC_CTYPE_FETCH(fetchadd, _rel, CT, cname, W)		\
	_ATOMIC_CTYPE_FETCH(swap,	  , CT, cname, W)		\
	_ATOMIC_CTYPE_CMPSET(	 , CT, cname, W)			\
	_ATOMIC_CTYPE_CMPSET(_acq, CT, cname, W)			\
	_ATOMIC_CTYPE_CMPSET(_rel, CT, cname, W)			\
	_ATOMIC_CTYPE_BIT(    , CT, cname, W)				\
	_ATOMIC_CTYPE_BIT(_acq, CT, cname, W)				\
									\
static __inline CT							\
atomic_load_##cname(const volatile CT *p)				\
{									\
	return ((CT)atomic_load_##W((const volatile uint##W##_t *)p));	\
}									\
									\
static __inline CT							\
atomic_load_acq_##cname(const volatile CT *p)				\
{									\
	return ((CT)atomic_load_acq_##W(				\
	    (const volatile uint##W##_t *)p));				\
}									\
									\
static __inline void							\
atomic_store_##cname(volatile CT *p, CT v)				\
{									\
	atomic_store_##W((volatile uint##W##_t *)p, (uint##W##_t)v);	\
}									\
									\
static __inline void							\
atomic_store_rel_##cname(volatile CT *p, CT v)				\
{									\
	atomic_store_rel_##W((volatile uint##W##_t *)p,			\
	    (uint##W##_t)v);						\
}									\
									\
static __inline CT							\
atomic_readandclear_##cname(volatile CT *p)				\
{									\
	return ((CT)atomic_readandclear_##W(				\
	    (volatile uint##W##_t *)p));				\
}									\
									\
static __inline int							\
atomic_testandclear_##cname(volatile CT *p, u_int bit)			\
{									\
	return (atomic_testandclear_##W((volatile uint##W##_t *)p,	\
	    bit));							\
}

_ATOMIC_GEN_CTYPE(u_char,  char,  8)
_ATOMIC_GEN_CTYPE(u_short, short, 16)
_ATOMIC_GEN_CTYPE(u_int,   int,   32)
#ifdef __LP64__
_ATOMIC_GEN_CTYPE(u_long,   long, 64)
_ATOMIC_GEN_CTYPE(uintptr_t, ptr, 64)
#else
_ATOMIC_GEN_CTYPE(u_long,   long, 32)
_ATOMIC_GEN_CTYPE(uintptr_t, ptr, 32)
#endif

#endif /* _SYS_ATOMIC_GENERIC_H_ */
