/*
 * Batch b0129 oracle: the original HardenedBSD C sources, concatenated.
 *
 * The only mechanical change is that each function gained a "ref_" prefix.
 * No function body is altered.
 *
 * lib/libc/net/getproto.c is absent on purpose; see skipped.txt.
 *
 * Added defines: the EAI_* values, normally supplied by FreeBSD's <netdb.h>.
 * <netdb.h> is not included here because the host libc uses different EAI_*
 * numbering, which would silently change the table indices.
 */

/* ===== lib/libc/net/gai_strerror.c ===== */

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

#define	EAI_ADDRFAMILY	 1	/* address family for hostname not supported */
#define	EAI_AGAIN	 2	/* temporary failure in name resolution */
#define	EAI_BADFLAGS	 3	/* invalid value for ai_flags */
#define	EAI_FAIL	 4	/* non-recoverable failure in name resolution */
#define	EAI_FAMILY	 5	/* ai_family not supported */
#define	EAI_MEMORY	 6	/* memory allocation failure */
#define	EAI_NODATA	 7	/* no address associated with hostname */
#define	EAI_NONAME	 8	/* hostname nor servname provided, or not known */
#define	EAI_SERVICE	 9	/* servname not supported for ai_socktype */
#define	EAI_SOCKTYPE	10	/* ai_socktype not supported */
#define	EAI_SYSTEM	11	/* system error returned in errno */
#define	EAI_BADHINTS	12	/* invalid value for hints */
#define	EAI_PROTOCOL	13	/* resolved protocol is unknown */
#define	EAI_OVERFLOW	14	/* argument buffer overflow */
#define	EAI_MAX		15

/*
 * Entries EAI_ADDRFAMILY (1) and EAI_NODATA (7) were omitted from RFC 3493,
 * but are or may be used as extensions or in old code.
 */
static const char *const ai_errlist[] = {
	[0] =			"Success",
	[EAI_ADDRFAMILY] =	"Address family for hostname not supported",
	[EAI_AGAIN] =		"Name could not be resolved at this time",
	[EAI_BADFLAGS] =	"Flags parameter had an invalid value",
	[EAI_FAIL] =		"Non-recoverable failure in name resolution",
	[EAI_FAMILY] =		"Address family not recognized",
	[EAI_MEMORY] =		"Memory allocation failure",
	[EAI_NODATA] =		"No address associated with hostname",
	[EAI_NONAME] =		"Name does not resolve",
	[EAI_SERVICE] =		"Service was not recognized for socket type",
	[EAI_SOCKTYPE] =	"Intended socket type was not recognized",
	[EAI_SYSTEM] =		"System error returned in errno",
	[EAI_BADHINTS] =	"Invalid value for hints",
	[EAI_PROTOCOL] =	"Resolved protocol is unknown",
	[EAI_OVERFLOW] =	"Argument buffer overflow",
};

const char *
ref_gai_strerror(int ecode)
{
#if defined(NLS)
	nl_catd catd;
	char *buf;
	int saved_errno;

	saved_errno = errno;
	if (thr_main() != 0)
		buf = gai_buf;
	else {
		if (thr_once(&gai_init_once, gai_keycreate) != 0 ||
		    !gai_keycreated)
			goto thr_err;
		if ((buf = thr_getspecific(gai_key)) == NULL) {
			if ((buf = malloc(sizeof(gai_buf))) == NULL)
				goto thr_err;
			if (thr_setspecific(gai_key, buf) != 0) {
				free(buf);
				goto thr_err;
			}
		}
	}

	catd = catopen("libc", NL_CAT_LOCALE);
	if (ecode > 0 && ecode < EAI_MAX)
		strlcpy(buf, catgets(catd, 3, ecode, ai_errlist[ecode]),
		    sizeof(gai_buf));
	else if (ecode == 0)
		strlcpy(buf, catgets(catd, 3, NL_MSGMAX - 1, "Success"),
		    sizeof(gai_buf));
	else
		strlcpy(buf, catgets(catd, 3, NL_MSGMAX, "Unknown error"),
		    sizeof(gai_buf));
	catclose(catd);
	errno = saved_errno;
	return (buf);

thr_err:
	errno = saved_errno;
#endif
	if (ecode >= 0 && ecode < EAI_MAX)
		return (ai_errlist[ecode]);
	return ("Unknown error");
}
