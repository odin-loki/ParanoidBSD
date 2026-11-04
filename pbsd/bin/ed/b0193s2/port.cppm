/*
 * PBSD batch b0193s2 -- C++23 module port of hbsd/src/bin/ed/io.c
 *
 * Faithful port: behaviour, integer signedness, evaluation order, pointer
 * arithmetic and bugs are preserved exactly as written in the C original.
 *
 * The four buffer-subsystem entry points of io.c (read_file, read_stream,
 * write_file, write_stream) are not present here; see skipped.txt.
 */

module;

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

export module pbsd.bin.ed.b0193s2;

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

#define ESCAPES "\a\b\f\n\r\t\v\\"
#define ESCCHARS "abfnrtv\\"

export namespace pbsd::bin_ed::b0193s2 {

/* ------------------------------------------------------------------ */
/* Editor globals (ed defines these in main.c / io.c)                  */
/* ------------------------------------------------------------------ */

const char *errmsg = nullptr;	/* error message */
int scripted = 0;		/* if set, suppress diagnostics */
int isbinary;			/* if set, buffer contains ASCII NULs */
int isglobal;			/* if set, doing a global command */
int lineno;			/* script line number */
int cols = 72;			/* wrap column */
int rows = 22;			/* scroll length: ws_row - 2 */
char *ibuf;			/* ed command-line buffer */
int ibufsz;			/* ed command-line buffer size */
char *ibufp;			/* pointer to ed command-line buffer */
int mutex;			/* if set, signals set flags */

int get_tty_line(void);

/* ------------------------------------------------------------------ */
/* From main.c: get_extended_line()'s only external helper.            */
/* ------------------------------------------------------------------ */

/* has_trailing_escape:  return the parity of escapes preceding a
   character in a string */
int
has_trailing_escape(char *s, char *t)
{
	return (s != t && *(t - 1) == '\\' && has_trailing_escape(s, t - 1) == 0);
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
get_stream_line(FILE *fp)
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
put_stream_line(FILE *fp, const char *s, int len)
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
get_extended_line(int *sizep, int nonl)
{
	static char *cvbuf = NULL;		/* buffer */
	static int cvbufsz = 0;			/* buffer size */

	int l, n;
	char *t = ibufp;

	while (*t++ != '\n')
		;
	if ((l = t - ibufp) < 2 || !has_trailing_escape(ibufp, ibufp + l - 1)) {
		*sizep = l;
		return ibufp;
	}
	*sizep = -1;
	REALLOC(cvbuf, cvbufsz, l, NULL);
	memcpy(cvbuf, ibufp, l);
	*(cvbuf + --l - 1) = '\n'; 	/* strip trailing esc */
	if (nonl) l--; 			/* strip newline */
	for (;;) {
		if ((n = get_tty_line()) < 0)
			return NULL;
		else if (n == 0 || ibuf[n - 1] != '\n') {
			errmsg = "unexpected end-of-file";
			return NULL;
		}
		REALLOC(cvbuf, cvbufsz, l + n, NULL);
		memcpy(cvbuf + l, ibuf, n);
		l += n;
		if (n < 2 || !has_trailing_escape(cvbuf, cvbuf + l - 1))
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
get_tty_line(void)
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



/* put_tty_line: print text to stdout */
int
put_tty_line(const char *s, int l, long n, int gflag)
{
	int col = 0;
	int lc = 0;
	const char *cp;

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
				if (get_tty_line() < 0)
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
/* Test-support accessors for the file-static i/o buffer.              */
/* ------------------------------------------------------------------ */

char **sbuf_addr(void) { return &sbuf; }
int *sbufsz_addr(void) { return &sbufsz; }

} /* namespace pbsd::bin_ed::b0193s2 */
