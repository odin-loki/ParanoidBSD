/* port.cppm: C++23 module port of hbsd/src/bin/ed/sub.c (batch b0193s1).
 *
 * The ported bodies are a faithful transliteration of the C originals:
 * behaviour, integer signedness, evaluation order and pointer arithmetic are
 * preserved exactly, bugs included.  ed.h's macros are reproduced verbatim
 * so the generated code matches the original expansion.
 *
 * search_and_replace is not part of this module -- see skipped.txt.
 */

module;

#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

export module pbsd.bin.ed.b0193s1;

/* ------------------------------------------------------------------ */
/* Definitions taken from bin/ed/ed.h                                  */
/* ------------------------------------------------------------------ */

#define ERR		(-2)
#define EMOD		(-3)
#define FATAL		(-4)

#define MINBUFSZ 512		/* minimum buffer size - must be > 0 */
#define SE_MAX 30		/* max subexpressions in a regular expression */
#ifdef INT_MAX
# define LINECHARS INT_MAX	/* max chars per line */
#else
# define LINECHARS MAXINT	/* max chars per line */
#endif

#ifndef LONG_BIT
# define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/* gflags */
#define GLB 001		/* global command */
#define GPR 002		/* print after command */
#define GLS 004		/* list after command */
#define GNP 010		/* enumerate after command */
#define GSG 020		/* global substitute */

#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
# define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/* SPL1: disable some interrupts (requires reliable signals) */
#define SPL1() mutex++

/* SPL0: enable all interrupts; check sigflags (requires reliable signals) */
#define SPL0() \
if (--mutex == 0) { \
	if (sigflags & (1 << (SIGHUP - 1))) handle_hup(SIGHUP); \
	if (sigflags & (1 << (SIGINT - 1))) handle_int(SIGINT); \
}

/* STRTOL: convert a string to long */
#define STRTOL(i, p) { \
	if (((i = strtol(p, &p, 10)) == LONG_MIN || i == LONG_MAX) && \
	    errno == ERANGE) { \
		errmsg = "number out of range"; \
	    	i = 0; \
		return ERR; \
	} \
}

/* REALLOC: assure at least a minimum size for buffer b */
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

/* NUL_TO_NEWLINE: overwrite ASCII NULs with newlines */
#define NUL_TO_NEWLINE(s, l) translit_text(s, l, '\0', '\n')

/* NEWLINE_TO_NUL: overwrite newlines with ASCII NULs */
#define NEWLINE_TO_NUL(s, l) translit_text(s, l, '\n', '\0')

#define IBUF_GUARD	8	/* readable slack in front of ibuf */
#define IBUF_TOTAL	264	/* whole ibuf allocation */
#define IBUF_USABLE	(IBUF_TOTAL - IBUF_GUARD)
#define SCRIPT_MAX	8
#define SCRIPT_LEN	128
#define LINE_STORE	512
#define SFBUF_SZ	640
#define GUARD_BYTE	0x7f

namespace pbsd::bin_ed::b0193s1 {

export typedef regex_t pattern_t;

/* Line node */
export typedef struct	line {
	struct line	*q_forw;
	struct line	*q_back;
	off_t		seek;		/* address of line in scratch buffer */
	int		len;		/* length of line */
} line_t;

/* ------------------------------------------------------------------ */
/* Test environment: globals of ed(1) plus models of the out-of-batch  */
/* helpers.  Mirrored byte-for-byte in oracle.c.                       */
/* ------------------------------------------------------------------ */

static char *ibuf_base;
static char *ibuf;
static char *ibufp;
static int ibufsz;

static int isbinary;
static int isglobal;
static int mutex;
static int sigflags;

static const char *errmsg;

static void
handle_hup(int signo)
{
	(void)signo;
}

static void
handle_int(int signo)
{
	(void)signo;
}

/* translit_text: model of the ed(1) helper of the same name (main.c) */
static char *
translit_text(char *s, int len, int from, int to)
{
	char *sp;

	for (sp = s; len-- > 0; sp++)
		if (*sp == from)
			*sp = to;
	return s;
}

/* scripted terminal input for get_tty_line */
static char script_text[SCRIPT_MAX][SCRIPT_LEN];
static int script_ret[SCRIPT_MAX];
static int script_n;
static int script_pos;

/* get_tty_line: model of the ed(1) helper (io.c): install the next scripted
   line in ibuf, rewind ibufp and return its length; a negative script entry
   or an exhausted script reports failure. */
static int
get_tty_line(void)
{
	int n;

	if (script_pos >= script_n)
		return -1;
	if (script_ret[script_pos] >= 0) {
		script_pos++;
		return -1;
	}
	n = (int)strlen(script_text[script_pos]);
	memcpy(ibuf, script_text[script_pos], (size_t)n + 1);
	ibufp = ibuf;
	script_pos++;
	return n;
}

static char line_store[LINE_STORE];
static int line_fail;
static char sfbuf[SFBUF_SZ];

/* get_sbuf_line: model of the ed(1) helper (buf.c): copy lp->len bytes of
   the line out of the scratch store into a static buffer and NUL terminate */
static char *
get_sbuf_line(line_t *lp)
{
	int len;

	memset(sfbuf, GUARD_BYTE, sizeof(sfbuf));
	if (line_fail)
		return NULL;
	len = lp->len;
	memcpy(sfbuf, line_store, (size_t)len);
	sfbuf[len] = '\0';
	return sfbuf;
}

export int apply_subst_template(const char *, regmatch_t *, int, int);
export int extract_subst_tail(int *, long *);
export char *extract_subst_template(void);
export int substitute_matching_text(pattern_t *, line_t *, int, int);

/* ================================================================== */
/* PORT OF: hbsd/src/bin/ed/sub.c                                      */
/* ================================================================== */

/* sub.c: This file contains the substitution routines for the ed
   line editor */
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

static char *rhbuf;		/* rhs substitution buffer */
static int rhbufsz;		/* rhs substitution buffer size */
static int rhbufi;		/* rhs substitution buffer index */

/* extract_subst_tail: extract substitution tail from the command buffer */
int
extract_subst_tail(int *flagp, long *np)
{
	char delimiter;

	*flagp = *np = 0;
	if ((delimiter = *ibufp) == '\n') {
		rhbufi = 0;
		*flagp = GPR;
		return 0;
	} else if (extract_subst_template() == NULL)
		return  ERR;
	else if (*ibufp == '\n') {
		*flagp = GPR;
		return 0;
	} else if (*ibufp == delimiter)
		ibufp++;
	if ('1' <= *ibufp && *ibufp <= '9') {
		STRTOL(*np, ibufp);
		return 0;
	} else if (*ibufp == 'g') {
		ibufp++;
		*flagp = GSG;
		return 0;
	}
	return 0;
}


/* extract_subst_template: return pointer to copy of substitution template
   in the command buffer */
char *
extract_subst_template(void)
{
	int n = 0;
	int i = 0;
	char c;
	char delimiter = *ibufp++;

	if (*ibufp == '%' && *(ibufp + 1) == delimiter) {
		ibufp++;
		if (!rhbuf)
			errmsg = "no previous substitution";
		return rhbuf;
	}
	while (*ibufp != delimiter) {
		REALLOC(rhbuf, rhbufsz, i + 2, NULL);
		if ((c = rhbuf[i++] = *ibufp++) == '\n' && *ibufp == '\0') {
			i--, ibufp--;
			break;
		} else if (c != '\\')
			;
		else if ((rhbuf[i++] = *ibufp++) != '\n')
			;
		else if (!isglobal) {
			while ((n = get_tty_line()) == 0 ||
			    (n > 0 && ibuf[n - 1] != '\n'))
				clearerr(stdin);
			if (n < 0)
				return NULL;
		}
	}
	REALLOC(rhbuf, rhbufsz, i + 1, NULL);
	rhbuf[rhbufi = i] = '\0';
	return  rhbuf;
}


static char *rbuf;		/* substitute_matching_text buffer */
static int rbufsz;		/* substitute_matching_text buffer size */

/* search_and_replace: OMITTED -- see skipped.txt */

/* substitute_matching_text: replace text matched by a pattern according to
   a substitution template; return pointer to the modified text */
int
substitute_matching_text(pattern_t *pat, line_t *lp, int gflag, int kth)
{
	int off = 0;
	int changed = 0;
	int matchno = 0;
	int i = 0;
	regmatch_t rm[SE_MAX];
	char *txt;
	char *eot;

	if ((txt = get_sbuf_line(lp)) == NULL)
		return ERR;
	if (isbinary)
		NUL_TO_NEWLINE(txt, lp->len);
	eot = txt + lp->len;
	if (!regexec(pat, txt, SE_MAX, rm, 0)) {
		do {
			if (!kth || kth == ++matchno) {
				changed++;
				i = rm[0].rm_so;
				REALLOC(rbuf, rbufsz, off + i, ERR);
				if (isbinary)
					NEWLINE_TO_NUL(txt, rm[0].rm_eo);
				memcpy(rbuf + off, txt, i);
				off += i;
				if ((off = apply_subst_template(txt, rm, off,
				    pat->re_nsub)) < 0)
					return ERR;
			} else {
				i = rm[0].rm_eo;
				REALLOC(rbuf, rbufsz, off + i, ERR);
				if (isbinary)
					NEWLINE_TO_NUL(txt, i);
				memcpy(rbuf + off, txt, i);
				off += i;
			}
			txt += rm[0].rm_eo;
		} while (*txt &&
                        (!changed || ((gflag & GSG) && rm[0].rm_eo)) &&
		        !regexec(pat, txt, SE_MAX, rm, REG_NOTBOL));
		i = eot - txt;
		REALLOC(rbuf, rbufsz, off + i + 2, ERR);
		if (i > 0 && !rm[0].rm_eo && (gflag & GSG)) {
			errmsg = "infinite substitution loop";
			return  ERR;
		}
		if (isbinary)
			NEWLINE_TO_NUL(txt, i);
		memcpy(rbuf + off, txt, i);
		memcpy(rbuf + off + i, "\n", 2);
	}
	return changed ? off + i + 1 : 0;
}


/* apply_subst_template: modify text according to a substitution template;
   return offset to end of modified text */
int
apply_subst_template(const char *boln, regmatch_t *rm, int off, int re_nsub)
{
	int j = 0;
	int k = 0;
	int n;
	char *sub = rhbuf;

	for (; sub - rhbuf < rhbufi; sub++)
		if (*sub == '&') {
			j = rm[0].rm_so;
			k = rm[0].rm_eo;
			REALLOC(rbuf, rbufsz, off + k - j, ERR);
			while (j < k)
				rbuf[off++] = boln[j++];
		} else if (*sub == '\\' && '1' <= *++sub && *sub <= '9' &&
		    (n = *sub - '0') <= re_nsub) {
			j = rm[n].rm_so;
			k = rm[n].rm_eo;
			REALLOC(rbuf, rbufsz, off + k - j, ERR);
			while (j < k)
				rbuf[off++] = boln[j++];
		} else {
			REALLOC(rbuf, rbufsz, off + 1, ERR);
			rbuf[off++] = *sub;
		}
	REALLOC(rbuf, rbufsz, off + 1, ERR);
	rbuf[off] = '\0';
	return off;
}

/* ================================================================== */
/* END PORT -- test environment accessors follow                       */
/* ================================================================== */

export void
env_reset(void)
{
	free(rhbuf);
	rhbuf = NULL;
	rhbufsz = 0;
	rhbufi = 0;

	free(rbuf);
	rbuf = NULL;
	rbufsz = 0;

	if (ibuf_base == NULL)
		ibuf_base = (char *)malloc(IBUF_TOTAL);
	memset(ibuf_base, GUARD_BYTE, IBUF_TOTAL);
	ibuf_base[IBUF_TOTAL - 2] = '\n';
	ibuf_base[IBUF_TOTAL - 1] = '\0';
	ibuf = ibuf_base + IBUF_GUARD;
	ibufsz = IBUF_USABLE;
	ibuf[0] = '\0';
	ibufp = ibuf;

	isbinary = 0;
	isglobal = 0;
	mutex = 0;
	sigflags = 0;
	errmsg = NULL;

	memset(script_text, 0, sizeof(script_text));
	memset(script_ret, 0, sizeof(script_ret));
	script_n = 0;
	script_pos = 0;

	memset(line_store, 0, sizeof(line_store));
	line_fail = 0;
	memset(sfbuf, GUARD_BYTE, sizeof(sfbuf));
}

export void
env_set_ibuf(const char *s, int n)
{
	memset(ibuf_base, GUARD_BYTE, IBUF_TOTAL);
	ibuf_base[IBUF_TOTAL - 2] = '\n';
	ibuf_base[IBUF_TOTAL - 1] = '\0';
	ibuf = ibuf_base + IBUF_GUARD;
	ibufsz = IBUF_USABLE;
	if (n < 0)
		n = 0;
	if (n > IBUF_USABLE - 16)
		n = IBUF_USABLE - 16;
	memcpy(ibuf, s, (size_t)n);
	ibuf[n] = '\0';
	ibufp = ibuf;
}

export void
env_set_isglobal(int v)
{
	isglobal = v;
}

export void
env_set_isbinary(int v)
{
	isbinary = v;
}

export void
env_set_rhbuf(const char *s, int n, int cap)
{
	free(rhbuf);
	if (cap <= 0) {
		rhbuf = NULL;
		rhbufsz = 0;
		rhbufi = n;
		return;
	}
	rhbuf = (char *)malloc((size_t)cap);
	memset(rhbuf, GUARD_BYTE, (size_t)cap);
	if (n > 0)
		memcpy(rhbuf, s, (size_t)n);
	if (n < cap)
		rhbuf[n] = '\0';
	rhbufsz = cap;
	rhbufi = n;
}

export void
env_set_rbuf(int cap)
{
	free(rbuf);
	if (cap <= 0) {
		rbuf = NULL;
		rbufsz = 0;
		return;
	}
	rbuf = (char *)malloc((size_t)cap);
	memset(rbuf, GUARD_BYTE, (size_t)cap);
	rbufsz = cap;
}

export void
env_set_line(const char *t, int n, int fail)
{
	memset(line_store, 0, sizeof(line_store));
	if (n > 0)
		memcpy(line_store, t, (size_t)n);
	line_fail = fail;
}

export void
env_script_add(const char *t, int neg)
{
	if (script_n >= SCRIPT_MAX)
		return;
	memset(script_text[script_n], 0, SCRIPT_LEN);
	if (!neg)
		memcpy(script_text[script_n], t, strlen(t));
	script_ret[script_n] = neg ? -1 : 0;
	script_n++;
}

export int env_get_rhbufi(void) { return rhbufi; }
export int env_get_rhbufsz(void) { return rhbufsz; }
export const char *env_get_rhbuf(void) { return rhbuf; }
export int env_get_rbufsz(void) { return rbufsz; }
export const char *env_get_rbuf(void) { return rbuf; }
export const char *env_get_ibuf_base(void) { return ibuf_base; }
export int env_get_ibufsz(void) { return ibufsz; }
export long env_get_ibufp_off(void) { return (long)(ibufp - ibuf); }
export const char *env_get_errmsg(void) { return errmsg; }
export int env_get_mutex(void) { return mutex; }
export int env_get_isbinary(void) { return isbinary; }
export int env_get_script_pos(void) { return script_pos; }
export const char *env_get_sfbuf(void) { return sfbuf; }

} /* namespace pbsd::bin_ed::b0193s1 */
