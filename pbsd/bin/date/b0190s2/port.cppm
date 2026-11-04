/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1985, 1987, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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
 * PBSD port of bin/date/date.c -- batch b0190s2.
 */

module;

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

export module pbsd.bin.date.b0190s2;

export namespace pbsd::bin_date::b0190s2 {

/*
 * The strftime_ns function is a wrapper around strftime(3), which adds support
 * for features absent from strftime(3). Currently, the only extra feature is
 * support for %N, the nanosecond conversion specification.
 *
 * The functions scans the format string for the non-standard conversion
 * specifications and replaces them with the date and time values before
 * passing the format string to strftime(3). The handling of the non-standard
 * conversion specifications happens before the call to strftime(3) to handle
 * cases like "%%N" correctly ("%%N" should yield "%N" instead of nanoseconds).
 */
size_t
strftime_ns(char * __restrict s, size_t maxsize, const char * __restrict format,
    const struct tm * __restrict t, long nsec, long res)
{
	size_t ret;
	char *newformat;
	char *oldformat;
	const char *prefix;
	const char *suffix;
	const char *tok;
	long number;
	int i, len, prefixlen, width, zeroes;
	bool seen_percent, seen_dash, seen_width;

	seen_percent = false;
	if ((newformat = strdup(format)) == NULL)
		err(1, "strdup");
	tok = newformat;
	for (tok = newformat; *tok != '\0'; tok++) {
		switch (*tok) {
		case '%':
			/*
			 * If the previous token was a percent sign,
			 * then there are two percent tokens in a row.
			 */
			if (seen_percent) {
				seen_percent = false;
			} else {
				seen_percent = true;
				seen_dash = seen_width = false;
				prefixlen = tok - newformat;
				width = 0;
			}
			break;
		case 'N':
			if (!seen_percent)
				break;
			oldformat = newformat;
			prefix = oldformat;
			suffix = tok + 1;
			/*
			 * Prepare the number we are about to print.  If
			 * the requested width is less than 9, we need to
			 * cut off the least significant digits.  If it is
			 * more than 9, we will have to append zeroes.
			 */
			if (seen_dash) {
				/*
				 * Calculate number of singificant digits
				 * based on res which is the clock's
				 * resolution in nanoseconds.
				 */
				for (width = 9, number = res;
				     width > 0 && number > 0;
				     width--, number /= 10)
					/* nothing */;
			}
			number = nsec;
			zeroes = 0;
			if (width == 0) {
				width = 9;
			} else if (width > 9) {
				zeroes = width - 9;
				width = 9;
			} else {
				for (i = 0; i < 9 - width; i++)
					number /= 10;
			}
			/*
			 * Construct a new format string from the prefix
			 * (i.e., the part of the old format from its
			 * beginning to the currently handled "%N"
			 * conversion specification), the nanoseconds, and
			 * the suffix (i.e., the part of the old format
			 * from the next token to the end).
			 */
			asprintf(&newformat, "%.*s%.*ld%.*d%n%s", prefixlen,
			    prefix, width, number, zeroes, 0, &len, suffix);
			if (newformat == NULL)
				err(1, "asprintf");
			free(oldformat);
			tok = newformat + len - 1;
			seen_percent = false;
			break;
		case '-':
			if (seen_percent) {
				if (seen_dash || seen_width) {
					seen_percent = false;
					break;
				}
				seen_dash = true;
			}
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (seen_percent) {
				if (seen_dash) {
					seen_percent = false;
					break;
				}
				width = width * 10 + *tok - '0';
				seen_width = true;
			}
			break;
		default:
			seen_percent = false;
			break;
		}
	}

	ret = strftime(s, maxsize, newformat, t);
	free(newformat);
	return (ret);
}

} /* namespace pbsd::bin_date::b0190s2 */
