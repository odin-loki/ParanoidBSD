/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * PBSD batch b0129: C++23 port of lib/libc/net/gai_strerror.c.
 *
 * NLS is not defined in the stock libc build, so the message-catalogue path is
 * not part of the translation unit and is not part of this port either.
 *
 * lib/libc/net/getproto.c is not in this module; see skipped.txt.
 */

export module pbsd.lib.libc.net.b0129;

namespace pbsd::lib_libc_net::b0129 {

/* EAI_* values as numbered by FreeBSD's <netdb.h>. */
inline constexpr int EAI_ADDRFAMILY = 1;
inline constexpr int EAI_AGAIN = 2;
inline constexpr int EAI_BADFLAGS = 3;
inline constexpr int EAI_FAIL = 4;
inline constexpr int EAI_FAMILY = 5;
inline constexpr int EAI_MEMORY = 6;
inline constexpr int EAI_NODATA = 7;
inline constexpr int EAI_NONAME = 8;
inline constexpr int EAI_SERVICE = 9;
inline constexpr int EAI_SOCKTYPE = 10;
inline constexpr int EAI_SYSTEM = 11;
inline constexpr int EAI_BADHINTS = 12;
inline constexpr int EAI_PROTOCOL = 13;
inline constexpr int EAI_OVERFLOW = 14;
inline constexpr int EAI_MAX = 15;

/*
 * The C original is a designated-array initialiser; the wrapper keeps the
 * EAI_* codes as the subscripts and keeps the array length at highest
 * designator + 1, as C computes it.
 */
struct ai_errlist_storage {
	const char *e[EAI_OVERFLOW + 1];
};

/*
 * Entries EAI_ADDRFAMILY (1) and EAI_NODATA (7) were omitted from RFC 3493,
 * but are or may be used as extensions or in old code.
 */
consteval ai_errlist_storage
init_ai_errlist()
{
	ai_errlist_storage t{};

	t.e[0] =		"Success";
	t.e[EAI_ADDRFAMILY] =	"Address family for hostname not supported";
	t.e[EAI_AGAIN] =	"Name could not be resolved at this time";
	t.e[EAI_BADFLAGS] =	"Flags parameter had an invalid value";
	t.e[EAI_FAIL] =		"Non-recoverable failure in name resolution";
	t.e[EAI_FAMILY] =	"Address family not recognized";
	t.e[EAI_MEMORY] =	"Memory allocation failure";
	t.e[EAI_NODATA] =	"No address associated with hostname";
	t.e[EAI_NONAME] =	"Name does not resolve";
	t.e[EAI_SERVICE] =	"Service was not recognized for socket type";
	t.e[EAI_SOCKTYPE] =	"Intended socket type was not recognized";
	t.e[EAI_SYSTEM] =	"System error returned in errno";
	t.e[EAI_BADHINTS] =	"Invalid value for hints";
	t.e[EAI_PROTOCOL] =	"Resolved protocol is unknown";
	t.e[EAI_OVERFLOW] =	"Argument buffer overflow";
	return t;
}

constexpr ai_errlist_storage ai_errlist_table = init_ai_errlist();
constexpr const char *const *ai_errlist = ai_errlist_table.e;

export const char *
gai_strerror(int ecode)
{
	if (ecode >= 0 && ecode < EAI_MAX)
		return (ai_errlist[ecode]);
	return ("Unknown error");
}

} /* namespace pbsd::lib_libc_net::b0129 */
