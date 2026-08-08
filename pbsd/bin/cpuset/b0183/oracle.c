/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2007, 2008 	Jeffrey Roberson <jeff@freebsd.org>
 * All rights reserved.
 *
 * Copyright (c) 2008 Nokia Corporation
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * PBSD batch b0183 oracle.
 *
 * Source: hbsd/src/bin/cpuset/cpuset.c
 *
 * The original C source, with every ported function renamed with a "ref_"
 * prefix (and the "static" storage class dropped so the harness can link
 * against it).  Function bodies are UNMODIFIED.  The definitions below the
 * include block are the ones <sys/param.h>, <sys/_bitset.h> and
 * <sys/bitset.h> supply on FreeBSD when cpuset.c is compiled with
 * _WANT_FREEBSD_BITSET; they are reproduced here so the file builds on a
 * non-FreeBSD host.
 */

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __dead2
#define	__dead2		__attribute__((__noreturn__))
#endif

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#ifndef howmany
#define	howmany(x, y)	(((x) + ((y) - 1)) / (y))
#endif

#define	_BITSET_BITS		(sizeof(long) * 8)

#define	__bitset_words(_s)	(howmany(_s, _BITSET_BITS))

#define	__bitset_mask(_s, n)						\
	(1L << ((__bitset_words((_s)) == 1) ?				\
	    (size_t)(n) : ((n) % _BITSET_BITS)))

#define	__bitset_word(_s, n)						\
	((__bitset_words((_s)) == 1) ? 0 : ((n) / _BITSET_BITS))

#define	BIT_ISSET(_s, n, p)						\
	((((p)->__bits[__bitset_word(_s, n)] & __bitset_mask(_s, n)) != 0))

struct bitset {
	long	__bits[__bitset_words(1)];
};

void ref_usage(void) __dead2;

void
ref_printset(struct bitset *mask, int size)
{
	int once;
	int bit;

	for (once = 0, bit = 0; bit < size; bit++) {
		if (BIT_ISSET(size, bit, mask)) {
			if (once == 0) {
				printf("%d", bit);
				once = 1;
			} else
				printf(", %d", bit);
		}
	}
	printf("\n");
}

void
ref_usage(void)
{

	fprintf(stderr,
    "usage: cpuset [-l cpu-list] [-n policy:domain-list] [-s setid] cmd ...\n");
	fprintf(stderr,
    "       cpuset [-l cpu-list] [-n policy:domain-list] [-s setid] -p pid\n");
	fprintf(stderr,
    "       cpuset [-c] [-l cpu-list] [-n policy:domain-list] -C -p pid\n");
	fprintf(stderr,
    "       cpuset [-c] [-l cpu-list] [-n policy:domain-list]\n"
    "              [-j jailid | -p pid | -t tid | -s setid | -x irq]\n");
	fprintf(stderr,
    "       cpuset -g [-cir]\n"
    "              [-d domain | -j jailid | -p pid | -t tid | -s setid | -x irq]\n");
	exit(1);
}
