/*
 * PBSD batch b0207 - C++23 module port of the portable part of
 *   hbsd/src/bin/pax/options.c
 *   hbsd/src/bin/pax/tables.c
 *
 * The port is deliberately literal: integer types, signedness, evaluation
 * order, pointer arithmetic and the original (occasionally surprising)
 * behaviour are preserved exactly. See skipped.txt for the functions and
 * files of the batch which are not here.
 */

module;

#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Defines normally supplied by <sys/limits.h> and tables.h, which are not
 * part of this batch.
 */
#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif
#ifndef QUAD_MAX
#define QUAD_MAX	0x7fffffffffffffffLL
#endif
#define MAXKEYLEN	128		/* max number of chars for hash */

export module pbsd.bin.pax.b0207;

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992 Keith Muller.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego.
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

export namespace pbsd::bin_pax::b0207 {

/*
 * OPLIST is declared in pax.h, which is not part of this batch; it is
 * reproduced here exactly as the ported code uses it.
 */
typedef struct oplist {
	char		*name;		/* option variable name e.g. name= */
	char		*value;		/* value for option variable */
	struct oplist	*fow;		/* next option */
} OPLIST;

} /* namespace */

namespace pbsd::bin_pax::b0207 {

static OPLIST *ophead = NULL;	/* head for format specific options -x */
static OPLIST *optail = NULL;	/* option tail */

/*
 * paxwarn() lives in pax.c which is not part of this batch. It is replaced
 * here by a recording double, identical to the one the oracle carries, so
 * that the diagnostics produced by the ported code are comparable.
 */
static char	warnbuf[4096];
static int	warnlen;
static int	warncnt;

static void
warn_append(int set, const char *fmt, va_list ap)
{
	char tmp[1024];
	int n;

	(void)vsnprintf(tmp, sizeof(tmp), fmt, ap);
	++warncnt;
	n = snprintf(warnbuf + warnlen, sizeof(warnbuf) - (size_t)warnlen,
	    "[%d]%s\n", set, tmp);
	if (n < 0)
		return;
	warnlen += n;
	if (warnlen >= (int)sizeof(warnbuf))
		warnlen = (int)sizeof(warnbuf) - 1;
}

static void
paxwarn(int set, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	warn_append(set, fmt, ap);
	va_end(ap);
}

} /* namespace */

export namespace pbsd::bin_pax::b0207 {

void
warn_reset(void)
{
	warnbuf[0] = '\0';
	warnlen = 0;
	warncnt = 0;
}

int
warn_count(void)
{
	return (warncnt);
}

const char *
warn_text(void)
{
	return (warnbuf);
}

/*
 * ===========================================================================
 * hbsd/src/bin/pax/options.c
 * ===========================================================================
 */

/*
 * opt_next()
 *	called by format specific options routines to get each format specific
 *	flag and value specified with -o
 * Return:
 *	pointer to next OPLIST entry or NULL (end of list).
 */

OPLIST *
opt_next(void)
{
	OPLIST *opt;

	if ((opt = ophead) != NULL)
		ophead = ophead->fow;
	return(opt);
}

/*
 * opt_add()
 *	breaks the value supplied to -o into an option name and value. Options
 *	are given to -o in the form -o name-value,name=value
 *	multiple -o may be specified.
 * Return:
 *	0 if format in name=value format, -1 if -o is passed junk.
 */

int
opt_add(const char *str)
{
	OPLIST *opt;
	char *frpt;
	char *pt;
	char *endpt;
	char *lstr;

	if ((str == NULL) || (*str == '\0')) {
		paxwarn(0, "Invalid option name");
		return(-1);
	}
	if ((lstr = strdup(str)) == NULL) {
		paxwarn(0, "Unable to allocate space for option list");
		return(-1);
	}
	frpt = endpt = lstr;

	/*
	 * break into name and values pieces and stuff each one into a
	 * OPLIST structure. When we know the format, the format specific
	 * option function will go through this list
	 */
	while ((frpt != NULL) && (*frpt != '\0')) {
		if ((endpt = strchr(frpt, ',')) != NULL)
			*endpt = '\0';
		if ((pt = strchr(frpt, '=')) == NULL) {
			paxwarn(0, "Invalid options format");
			free(lstr);
			return(-1);
		}
		if ((opt = (OPLIST *)malloc(sizeof(OPLIST))) == NULL) {
			paxwarn(0, "Unable to allocate space for option list");
			free(lstr);
			return(-1);
		}
		lstr = NULL;	/* parts of string going onto the OPLIST */
		*pt++ = '\0';
		opt->name = frpt;
		opt->value = pt;
		opt->fow = NULL;
		if (endpt != NULL)
			frpt = endpt + 1;
		else
			frpt = NULL;
		if (ophead == NULL) {
			optail = ophead = opt;
			continue;
		}
		optail->fow = opt;
		optail = opt;
	}
	free(lstr);
	return(0);
}

/*
 * str_offt()
 *	Convert an expression of the following forms to an off_t > 0.
 * 	1) A positive decimal number.
 *	2) A positive decimal number followed by a b (mult by 512).
 *	3) A positive decimal number followed by a k (mult by 1024).
 *	4) A positive decimal number followed by a m (mult by 512).
 *	5) A positive decimal number followed by a w (mult by sizeof int)
 *	6) Two or more positive decimal numbers (with/without k,b or w).
 *	   separated by x (also * for backwards compatibility), specifying
 *	   the product of the indicated values.
 * Return:
 *	0 for an error, a positive value o.w.
 */

off_t
str_offt(char *val)
{
	char *expr;
	off_t num, t;

	num = strtoq(val, &expr, 0);
	if ((num == QUAD_MAX) || (num <= 0) || (expr == val))
		return(0);

	switch(*expr) {
	case 'b':
		t = num;
		num *= 512;
		if (t > num)
			return(0);
		++expr;
		break;
	case 'k':
		t = num;
		num *= 1024;
		if (t > num)
			return(0);
		++expr;
		break;
	case 'm':
		t = num;
		num *= 1048576;
		if (t > num)
			return(0);
		++expr;
		break;
	case 'w':
		t = num;
		num *= sizeof(int);
		if (t > num)
			return(0);
		++expr;
		break;
	}

	switch(*expr) {
		case '\0':
			break;
		case '*':
		case 'x':
			t = num;
			num *= str_offt(expr + 1);
			if (t > num)
				return(0);
			break;
		default:
			return(0);
	}
	return(num);
}

/*
 * no_op()
 *	for those option functions where the archive format has nothing to do.
 * Return:
 *	0
 */

int
no_op(void)
{
	return(0);
}

/*
 * ===========================================================================
 * hbsd/src/bin/pax/tables.c
 * ===========================================================================
 */

/*
 * st_hash()
 *	hashes filenames to a u_int for hashing into a table. Looks at the tail
 *	end of file, as this provides far better distribution than any other
 *	part of the name. For performance reasons we only care about the last
 *	MAXKEYLEN chars (should be at LEAST large enough to pick off the file
 *	name). Was tested on 500,000 name file tree traversal from the root
 *	and gave almost a perfectly uniform distribution of keys when used with
 *	prime sized tables (MAXKEYLEN was 128 in test). Hashes (sizeof int)
 *	chars at a time and pads with 0 for last addition.
 * Return:
 *	the hash value of the string MOD (%) the table size.
 */

u_int
st_hash(char *name, int len, int tabsz)
{
	char *pt;
	char *dest;
	char *end;
	int i;
	u_int key = 0;
	int steps;
	int res;
	u_int val;

	/*
	 * only look at the tail up to MAXKEYLEN, we do not need to waste
	 * time here (remember these are pathnames, the tail is what will
	 * spread out the keys)
	 */
	if (len > MAXKEYLEN) {
		pt = &(name[len - MAXKEYLEN]);
		len = MAXKEYLEN;
	} else
		pt = name;

	/*
	 * calculate the number of u_int size steps in the string and if
	 * there is a runt to deal with
	 */
	steps = len/sizeof(u_int);
	res = len % sizeof(u_int);

	/*
	 * add up the value of the string in unsigned integer sized pieces
	 * too bad we cannot have unsigned int aligned strings, then we
	 * could avoid the expensive copy.
	 */
	for (i = 0; i < steps; ++i) {
		end = pt + sizeof(u_int);
		dest = (char *)&val;
		while (pt < end)
			*dest++ = *pt++;
		key += val;
	}

	/*
	 * add in the runt padded with zero to the right
	 */
	if (res) {
		val = 0;
		end = pt + res;
		dest = (char *)&val;
		while (pt < end)
			*dest++ = *pt++;
		key += val;
	}

	/*
	 * return the result mod the table size
	 */
	return(key % tabsz);
}

} /* namespace pbsd::bin_pax::b0207 */
