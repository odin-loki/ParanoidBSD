/*
 * Reference oracle for batch b0075.
 *
 * Original HardenedBSD sources, concatenated, with every function renamed with
 * a "ref_" prefix.  Function bodies are unmodified.  The only additions are the
 * declarations/defines that the private libc headers would otherwise supply.
 *
 * Included source files:
 *   hbsd/src/lib/libc/locale/collcmp.c
 *
 * See skipped.txt for the files that are not represented here.
 */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 1996 by Andrey A. Chernov, Moscow, Russia.
 * All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
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

#include <string.h>
#include <wchar.h>
/* "collate.h" only declares these two entry points; supplied here instead. */
int ref___collate_range_cmp(char c1, char c2);
int ref___wcollate_range_cmp(wchar_t c1, wchar_t c2);

/*
 * Compare two characters using collate
 */

int ref___collate_range_cmp(char c1, char c2)
{
	char s1[2], s2[2];

	s1[0] = c1;
	s1[1] = '\0';
	s2[0] = c2;
	s2[1] = '\0';
	return (strcoll(s1, s2));
}

int ref___wcollate_range_cmp(wchar_t c1, wchar_t c2)
{
	wchar_t s1[2], s2[2];

	s1[0] = c1;
	s1[1] = L'\0';
	s2[0] = c2;
	s2[1] = L'\0';
	return (wcscoll(s1, s2));
}
