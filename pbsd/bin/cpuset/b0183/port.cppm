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
 * PBSD batch b0183: C++23 port of hbsd/src/bin/cpuset/cpuset.c.
 *
 * printset() and usage() are ported here.  printaffinity(), printsetid() and
 * main() are listed in skipped.txt: they are wrappers around the FreeBSD
 * cpuset(2)/domainset(2)/jail(3) interfaces, which do not exist on the port
 * host, so no oracle can be built for them.
 */

module;

#include <cstddef>
#include <cstdio>
#include <cstdlib>

export module pbsd.bin.cpuset.b0183;

export namespace pbsd::bin_cpuset::b0183 {

/*
 * Supplied on FreeBSD by <sys/param.h>, <sys/_bitset.h> and <sys/bitset.h>
 * once _WANT_FREEBSD_BITSET is defined.  The arithmetic, the types it is
 * performed in and the evaluation order match the macros exactly.
 */
inline constexpr std::size_t _BITSET_BITS = sizeof(long) * 8;

constexpr std::size_t
howmany_(std::size_t x, std::size_t y) noexcept
{
	return ((x + (y - 1)) / y);
}

constexpr std::size_t
__bitset_words(int _s) noexcept
{
	return (howmany_(static_cast<std::size_t>(_s), _BITSET_BITS));
}

constexpr std::size_t
__bitset_word(int _s, int n) noexcept
{
	return (__bitset_words(_s) == 1 ? 0 :
	    static_cast<std::size_t>(n) / _BITSET_BITS);
}

constexpr long
__bitset_mask(int _s, int n) noexcept
{
	return (1L << (__bitset_words(_s) == 1 ?
	    static_cast<std::size_t>(n) :
	    static_cast<std::size_t>(n) % _BITSET_BITS));
}

struct bitset {
	long	__bits[1];
};

constexpr bool
BIT_ISSET(int _s, int n, const bitset *p) noexcept
{
	return ((p->__bits[__bitset_word(_s, n)] & __bitset_mask(_s, n)) != 0);
}

[[noreturn]] void usage(void);

void
printset(bitset *mask, int size)
{
	int once;
	int bit;

	for (once = 0, bit = 0; bit < size; bit++) {
		if (BIT_ISSET(size, bit, mask)) {
			if (once == 0) {
				std::printf("%d", bit);
				once = 1;
			} else
				std::printf(", %d", bit);
		}
	}
	std::printf("\n");
}

[[noreturn]] void
usage(void)
{

	std::fprintf(stderr,
    "usage: cpuset [-l cpu-list] [-n policy:domain-list] [-s setid] cmd ...\n");
	std::fprintf(stderr,
    "       cpuset [-l cpu-list] [-n policy:domain-list] [-s setid] -p pid\n");
	std::fprintf(stderr,
    "       cpuset [-c] [-l cpu-list] [-n policy:domain-list] -C -p pid\n");
	std::fprintf(stderr,
    "       cpuset [-c] [-l cpu-list] [-n policy:domain-list]\n"
    "              [-j jailid | -p pid | -t tid | -s setid | -x irq]\n");
	std::fprintf(stderr,
    "       cpuset -g [-cir]\n"
    "              [-d domain | -j jailid | -p pid | -t tid | -s setid | -x irq]\n");
	std::exit(1);
}

} // namespace pbsd::bin_cpuset::b0183
