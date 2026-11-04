/*
 * Reference oracle: HardenedBSD thr_machdep.c with every function renamed
 * ref_*.  Function bodies are unmodified.  libc_private.h and thr_private.h
 * are unavailable outside the libthr build; minimal stand-ins are supplied.
 */

#define _WANT_P_OSREL

#ifndef P_OSREL_TLSBASE
#define	P_OSREL_TLSBASE			1500044
#endif

#ifndef THR_C_RUNTIME
#define	THR_C_RUNTIME			0x0004
#endif

#ifndef AMD64_GET_TLSBASE
#define	AMD64_GET_TLSBASE		135
#endif

#include <errno.h>

struct tcb;
struct pthread {
	struct tcb	*tcb;
};

extern int __getosreldate(void);
extern int sysarch(int, void *);
extern void amd64_set_tlsbase(void *);
extern void amd64_set_fsbase(void *);
extern int __thr_new_flags;

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 The FreeBSD Foundation
 * All rights reserved.
 *
 * This software was developed by Konstantin Belousov
 * under sponsorship from the FreeBSD Foundation.
 *
 */

void
ref___thr_setup_tsd(struct pthread *thread)
{
	void *base;
	int error;

	if (__getosreldate() < P_OSREL_TLSBASE) {
		amd64_set_tlsbase(thread->tcb);
		return;
	}

	/*
	 * Make tlsbase handling more compatible with code, like Go
	 * runtime, which wants to manage fsbase itself, and which do
	 * not need assistance in setting fsbase for signal handlers.
	 *
	 * If the main thread did not used amd64_set_tlsbase(), which
	 * means that rtld/libc was not utilized, do not use
	 * amd64_set_tlsbase() either.  Also do not mark new threads
	 * as using C runtime with the THR_C_RUNTIME flag.
	 */
	error = sysarch(AMD64_GET_TLSBASE, &base);
	if (error != 0 && errno == ESRCH) {
		__thr_new_flags &= ~THR_C_RUNTIME;
		amd64_set_fsbase(thread->tcb);
	} else {
		amd64_set_tlsbase(thread->tcb);
	}
}
