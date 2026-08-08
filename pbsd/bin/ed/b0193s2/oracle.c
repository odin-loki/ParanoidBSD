/*
 * oracle.c -- reference (specification) build for PBSD batch b0193s2.
 *
 * Contents: hbsd/src/bin/ed/io.c, with every ported function renamed with a
 * "ref_" prefix.  Function bodies are byte-for-byte the originals.  The only
 * additions are the declarations/macros that io.c picks up from "ed.h" (which
 * is not part of this batch), the globals that live in main.c, and two tiny
 * accessors so the differential harness can inspect io.c's file-static i/o
 * buffer.  Nothing inside an original function body has been altered.
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* ------------------------------------------------------------------ */
/* Definitions io.c inherits from ed.h                                 */
/* ------------------------------------------------------------------ */

#define ERR		(-2)
#define EMOD		(-3)
#define FATAL		(-4)

#define MINBUFSZ 512		/* minimum buffer size - must be > 0 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

/* gflags */
#define GLB 001		/* global command */
#define GPR 002		/* print after command */
#define GLS 004		/* list after command */
#define GNP 010		/* enumerate after command */
#define GSG 020		/* global substitute */

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

int mutex;			/* if set, signals set flags */

#define SPL1() mutex++
#define SPL0() (void)(--mutex)

/* REALLOC: assure at least a minimum size for buffer b */
#define REALLOC(b,n,i,err) \
if ((i) > (n)) { \
	int ti = (n); \
	char *ts; \
	SPL1(); \
	if ((b) != NULL) { \
		if ((ts = (char *) realloc((b), ti += max((i), MINBUFSZ))) == NULL) { \
			fprintf(stderr, "%s\n", strerror(errno)); \
			errmsg = "out of memory"; \
			SPL0(); \
			return err; \
		} \
	} else { \
		if ((ts = (char *) malloc(ti += max((i), MINBUFSZ))) == NULL) { \
			fprintf(stderr, "%s\n", strerror(errno)); \
			errmsg = "out of memory"; \
			SPL0(); \
			return err; \
		} \
	} \
	(n) = ti; \
	(b) = ts; \
	SPL0(); \
}

/* ------------------------------------------------------------------ */
/* Globals that ed defines in main.c                                   */
/* ------------------------------------------------------------------ */

const char *errmsg = NULL;	/* error message */
int scripted = 0;		/* if set, suppress diagnostics */
int isbinary;			/* if set, buffer contains ASCII NULs */
int isglobal;			/* if set, doing a global command */
int lineno;			/* script line number */
int cols = 72;			/* wrap column */
int rows = 22;			/* scroll length: ws_row - 2 */
char *ibuf;			/* ed command-line buffer */
int ibufsz;			/* ed command-line buffer size */
char *ibufp;			/* pointer to ed command-line buffer */

static int ref_has_trailing_escape(char *s, char *t);
int ref_get_tty_line(void);

/* ------------------------------------------------------------------ */
/* From main.c: get_extended_line()'s only external helper.            */
/* Body unmodified.                                                    */
/* ------------------------------------------------------------------ */

/* has_trailing_escape:  return the parity of escapes preceding a
   character in a string */
static int
ref_has_trailing_escape(char *s, char *t)
{
	return (s != t && *(t - 1) == '\\' && ref_has_trailing_escape(s, t - 1) == 0);
}

/* io.c: This file contains the i/o routines for the ed line editor */
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

static char *sbuf;		/* file i/o buffer */
static int sbufsz;		/* file i/o buffer size */
int newline_added;		/* if set, newline appended to input file */

/* get_stream_line: read a line of text from a stream; return line length */
int
ref_get_stream_line(FILE *fp)
{
	int c;
	int i = 0;

	while (((c = getc(fp)) != EOF || (!feof(fp) && !ferror(fp))) &&
	    c != '\n') {
		REALLOC(sbuf, sbufsz, i + 1, ERR);
		if (!(sbuf[i++] = c))
			isbinary = 1;
	}
	REALLOC(sbuf, sbufsz, i + 2, ERR);
	if (c == '\n')
		sbuf[i++] = c;
	else if (ferror(fp)) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "cannot read input file";
		return ERR;
	} else if (i) {
		sbuf[i++] = '\n';
		newline_added = 1;
	}
	sbuf[i] = '\0';
	return (isbinary && newline_added && i) ? --i : i;
}


/* put_stream_line: write a line of text to a stream; return status */
int
ref_put_stream_line(FILE *fp, const char *s, int len)
{
	while (len--)
		if (fputc(*s++, fp) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			errmsg = "cannot write file";
			return ERR;
		}
	return 0;
}

/* get_extended_line: get an extended line from stdin */
char *
ref_get_extended_line(int *sizep, int nonl)
{
	static char *cvbuf = NULL;		/* buffer */
	static int cvbufsz = 0;			/* buffer size */

	int l, n;
	char *t = ibufp;

	while (*t++ != '\n')
		;
	if ((l = t - ibufp) < 2 || !ref_has_trailing_escape(ibufp, ibufp + l - 1)) {
		*sizep = l;
		return ibufp;
	}
	*sizep = -1;
	REALLOC(cvbuf, cvbufsz, l, NULL);
	memcpy(cvbuf, ibufp, l);
	*(cvbuf + --l - 1) = '\n'; 	/* strip trailing esc */
	if (nonl) l--; 			/* strip newline */
	for (;;) {
		if ((n = ref_get_tty_line()) < 0)
			return NULL;
		else if (n == 0 || ibuf[n - 1] != '\n') {
			errmsg = "unexpected end-of-file";
			return NULL;
		}
		REALLOC(cvbuf, cvbufsz, l + n, NULL);
		memcpy(cvbuf + l, ibuf, n);
		l += n;
		if (n < 2 || !ref_has_trailing_escape(cvbuf, cvbuf + l - 1))
			break;
		*(cvbuf + --l - 1) = '\n'; 	/* strip trailing esc */
		if (nonl) l--; 			/* strip newline */
	}
	REALLOC(cvbuf, cvbufsz, l + 1, NULL);
	cvbuf[l] = '\0';
	*sizep = l;
	return cvbuf;
}


/* get_tty_line: read a line of text from stdin; return line length */
int
ref_get_tty_line(void)
{
	int oi = 0;
	int i = 0;
	int c;

	for (;;)
		switch (c = getchar()) {
		default:
			oi = 0;
			REALLOC(ibuf, ibufsz, i + 2, ERR);
			if (!(ibuf[i++] = c)) isbinary = 1;
			if (c != '\n')
				continue;
			lineno++;
			ibuf[i] = '\0';
			ibufp = ibuf;
			return i;
		case EOF:
			if (ferror(stdin)) {
				fprintf(stderr, "stdin: %s\n", strerror(errno));
				errmsg = "cannot read stdin";
				clearerr(stdin);
				ibufp = NULL;
				return ERR;
			} else {
				clearerr(stdin);
				if (i != oi) {
					oi = i;
					continue;
				} else if (i)
					ibuf[i] = '\0';
				ibufp = ibuf;
				return i;
			}
		}
}



#define ESCAPES "\a\b\f\n\r\t\v\\"
#define ESCCHARS "abfnrtv\\"

/* put_tty_line: print text to stdout */
int
ref_put_tty_line(const char *s, int l, long n, int gflag)
{
	int col = 0;
	int lc = 0;
	char *cp;

	if (gflag & GNP) {
		printf("%ld\t", n);
		col = 8;
	}
	for (; l--; s++) {
		if ((gflag & GLS) && ++col > cols) {
			fputs("\\\n", stdout);
			col = 1;
#ifndef BACKWARDS
			if (!scripted && !isglobal && ++lc > rows) {
				lc = 0;
				fputs("Press <RETURN> to continue... ", stdout);
				fflush(stdout);
				if (ref_get_tty_line() < 0)
					return ERR;
			}
#endif
		}
		if (gflag & GLS) {
			if (31 < *s && *s < 127 && *s != '\\')
				putchar(*s);
			else {
				putchar('\\');
				col++;
				if (*s && (cp = strchr(ESCAPES, *s)) != NULL)
					putchar(ESCCHARS[cp - ESCAPES]);
				else {
					putchar((((unsigned char) *s & 0300) >> 6) + '0');
					putchar((((unsigned char) *s & 070) >> 3) + '0');
					putchar(((unsigned char) *s & 07) + '0');
					col += 2;
				}
			}

		} else
			putchar(*s);
	}
#ifndef BACKWARDS
	if (gflag & GLS)
		putchar('$');
#endif
	putchar('\n');
	return 0;
}

/* ------------------------------------------------------------------ */
/* Test-support accessors: expose io.c's file-static i/o buffer so the */
/* harness can guard-fill and compare it.  Not part of the original.   */
/* ------------------------------------------------------------------ */

char **ref_sbuf_addr(void) { return &sbuf; }
int *ref_sbufsz_addr(void) { return &sbufsz; }
