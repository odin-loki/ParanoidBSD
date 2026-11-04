/*-
 * Copyright (c) 2001, 2002 Mike Barcroft <mike@FreeBSD.org>
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Klaus Klein.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MACHINE__STDINT_H_
#define	_MACHINE__STDINT_H_

/*
 * PBSD: the seventy limit and constant macros are written once, in terms
 * of the compiler's own predefines, in <sys/_stdint_generic.h>. This file
 * keeps only what is genuinely per-architecture.
 *
 * tools/stdint_generic_check.py proves the replacement rather than
 * asserting it: docs/migration/stdint_expansions.json records what this
 * header expanded to before the change, by value, for all six targets,
 * and the gate compares every macro against it.
 */
#include <sys/_stdint_generic.h>

/*
 * sig_atomic_t is the one type here that is not a function of the
 * compiler's model: it is 64-bit on amd64, arm64 and riscv and 32-bit on
 * i386, arm and powerpc - powerpc64 included, so it does not follow
 * __LP64__ - and clang and FreeBSD disagree about amd64. So these three
 * stay where the tree's answer is.
 */
#define	SIG_ATOMIC_MIN	INT64_MIN
#define	SIG_ATOMIC_MAX	INT64_MAX

#if __ISO_C_VISIBLE >= 2023
#define	SIG_ATOMIC_WIDTH	INT64_WIDTH
#endif /* __ISO_C_VISIBLE >= 2023 */

#endif /* !_MACHINE__STDINT_H_ */
