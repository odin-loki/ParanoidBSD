#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
/*
 * oracle.c -- reference specification for PBSD batch b0148s1.
 *
 * Original HardenedBSD ed re.c concatenated; every batch function is
 * renamed with a ref_ prefix.  Function bodies are UNMODIFIED.  Supporting
 * types, macros, globals, and shims are added only where required so the
 * unmodified bodies compile and link.
 */

#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#define MINBUFSZ 512

typedef regex_t pattern_t;

#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#define SPL1() mutex++

#define SPL0() \
if (--mutex == 0) { \
}

#define REALLOC(b,n,i,err) \
if ((i) > (n)) { \
	size_t ti = (n); \
	char *ts; \
	SPL1(); \
	if ((ts = (char *) realloc((b), ti += max((i), MINBUFSZ))) == NULL) { \
		fprintf(stderr, "%s\n", strerror(errno)); \
		errmsg = "out of memory"; \
		SPL0(); \
		return err; \
	} \
	(n) = ti; \
	(b) = ts; \
	SPL0(); \
}

#define NUL_TO_NEWLINE(s, l) translit_text(s, l, '\0', '\n')

/* ------------------------------------------------------------------ */
/* globals and harness-visible state                                   */
/* ------------------------------------------------------------------ */

char oracle_ibuf_storage[65536];
char *ibuf = oracle_ibuf_storage;
char *ibufp;
int isbinary;
int mutex;
int patlock;

const char *errmsg = "";

unsigned char ctab[256];

int oracle_malloc_fail_at;
int oracle_malloc_calls;

void
oracle_reset_hooks(void)
{
	oracle_malloc_fail_at = 0;
	oracle_malloc_calls = 0;
}

static void *
oracle_malloc(size_t n)
{
	void *p;

	oracle_malloc_calls++;
	if (oracle_malloc_fail_at != 0 &&
	    oracle_malloc_calls >= oracle_malloc_fail_at)
		return (NULL);
	p = malloc(n);
	return (p);
}

#define malloc oracle_malloc

void
oracle_reset_globals(void)
{
	ibufp = ibuf;
	isbinary = 0;
	mutex = 0;
	patlock = 0;
	errmsg = "";
	oracle_reset_hooks();
}

/* forward declarations for batch functions */
pattern_t *ref_get_compiled_pattern(void);
char *ref_extract_pattern(int delimiter);
char *ref_parse_char_class(char *s);
char *ref_translit_text(char *s, int len, int from, int to);

#define get_compiled_pattern	ref_get_compiled_pattern
#define extract_pattern		ref_extract_pattern
#define parse_char_class	ref_parse_char_class
#define translit_text		ref_translit_text

/* ------------------------------------------------------------------ */
/* hbsd/src/bin/ed/re.c                                                  */
/* ------------------------------------------------------------------ */

/* re.c: This file contains the regular expression interface routines for
   the ed line editor. */
/*-
 * Copyright (c) 1993 Andrew Moore, Talke Studio.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

pattern_t *
ref_get_compiled_pattern(void)
{
	static pattern_t *expr = NULL;
	static char error[1024];

	char *exprs;
	char delimiter;
	int n;

	if ((delimiter = *ibufp) == ' ') {
		errmsg = "invalid pattern delimiter";
		return NULL;
	} else if (delimiter == '\n' || *++ibufp == '\n' || *ibufp == delimiter) {
		if (!expr)
			errmsg = "no previous pattern";
		return expr;
	} else if ((exprs = extract_pattern(delimiter)) == NULL)
		return NULL;
	/* buffer alloc'd && not reserved */
	if (expr && !patlock)
		regfree(expr);
	else if ((expr = (pattern_t *) malloc(sizeof(pattern_t))) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "out of memory";
		return NULL;
	}
	patlock = 0;
	if ((n = regcomp(expr, exprs, 0))) {
		regerror(n, expr, error, sizeof error);
		errmsg = error;
		free(expr);
		return expr = NULL;
	}
	return expr;
}


char *
ref_extract_pattern(int delimiter)
{
	static char *lhbuf = NULL;	/* buffer */
	static int lhbufsz = 0;		/* buffer size */

	char *nd;
	int len;

	for (nd = ibufp; *nd != delimiter && *nd != '\n'; nd++)
		switch (*nd) {
		default:
			break;
		case '[':
			if ((nd = parse_char_class(nd + 1)) == NULL) {
				errmsg = "unbalanced brackets ([])";
				return NULL;
			}
			break;
		case '\\':
			if (*++nd == '\n') {
				errmsg = "trailing backslash (\\)";
				return NULL;
			}
			break;
		}
	len = nd - ibufp;
	REALLOC(lhbuf, lhbufsz, len + 1, NULL);
	memcpy(lhbuf, ibufp, len);
	lhbuf[len] = '\0';
	ibufp = nd;
	return (isbinary) ? NUL_TO_NEWLINE(lhbuf, len) : lhbuf;
}


char *
ref_parse_char_class(char *s)
{
	int c, d;

	if (*s == '^')
		s++;
	if (*s == ']')
		s++;
	for (; *s != ']' && *s != '\n'; s++)
		if (*s == '[' && ((d = *(s+1)) == '.' || d == ':' || d == '='))
			for (s++, c = *++s; *s != ']' || c != d; s++)
				if ((c = *s) == '\n')
					return NULL;
	return  (*s == ']') ? s : NULL;
}

/* ------------------------------------------------------------------ */
/* hbsd/src/bin/ed/buf.c (translit_text shim for NUL_TO_NEWLINE)         */
/* ------------------------------------------------------------------ */

char *
ref_translit_text(char *s, int len, int from, int to)
{
	static int i = 0;

	unsigned char *us;

	ctab[i] = i;			/* restore table to initial state */
	ctab[i = from] = to;
	for (us = (unsigned char *) s; len-- > 0; us++)
		*us = ctab[*us];
	return s;
}

void
oracle_reset_batch(void)
{
	int i;

	oracle_reset_globals();
	for (i = 0; i < 256; i++)
		ctab[i] = i;
}
