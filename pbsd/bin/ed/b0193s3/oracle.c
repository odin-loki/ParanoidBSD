/* oracle.c: reference C implementation for PBSD batch b0193s3.
 *
 * Source: hbsd/src/bin/ed/main.c
 *
 * Every ported function is renamed with a `ref_' prefix by way of an
 * object-like macro placed immediately before its (verbatim) definition.
 * The function bodies below are byte-for-byte identical to the originals;
 * only the surrounding declarations/defines (which normally live in ed.h)
 * have been supplied here so that the subset compiles standalone.
 */

/* main.c: This file contains the main control and user-interface routines
   for the ed line editor. */
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

/*
 * CREDITS
 *
 *	This program is based on the editor algorithm described in
 *	Brian W. Kernighan and P. J. Plauger's book "Software Tools
 *	in Pascal," Addison-Wesley, 1981.
 *
 *	The buffering algorithm is attributed to Rodney Ruddock of
 *	the University of Guelph, Guelph, Ontario.
 *
 */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Definitions normally supplied by ed.h / <sys/param.h>.              */
/* ------------------------------------------------------------------ */

/* HardenedBSD's <sys/syslimits.h> value; pinned so that the C and C++ sides
   agree regardless of what the host <limits.h> chooses to expose. */
#undef PATH_MAX
#define PATH_MAX 1024

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#define MINBUFSZ 512		/* minimum buffer size - must be > 0 */

#define ERR (-2)
#define EMOD (-3)
#define FATAL (-4)

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/* SPL1/SPL0: raise/lower the interrupt mutex.  The signal-dispatch tail of
   the real SPL0() is elided because handle_hup()/handle_int() are not part
   of this batch; sigflags is never set here, so the behaviour is the same. */
#define SPL1() mutex++
#define SPL0() mutex--

/* REALLOC: assure at least a minimum size for buffer b.  The freshly
   obtained bytes are stamped with 0x7f so that a differential harness can
   compare the whole buffer, including bytes the callee never writes. */
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
	memset(ts + (n), 0x7f, (size_t)(ti - (n))); \
	(n) = ti; \
	(b) = ts; \
}

/* line node structure (ed.h) */
typedef struct line {
	struct line *q_forw;
	struct line *q_back;
	off_t seek;			/* address of line in scratch buffer */
	int len;			/* length of line */
} line_t;

/* ------------------------------------------------------------------ */
/* Globals.  `static' has been dropped from the file-scope objects that  */
/* the differential harness must observe; the storage class is not part  */
/* of any function body.                                                 */
/* ------------------------------------------------------------------ */

const char *errmsg = "";	/* error message buffer (ed.h) */

int mutex = 0;			/* if set, signals set "sigflags" */
int red = 0;			/* if set, restrict shell/directory access */

long current_addr;		/* current address in editor buffer */
long addr_last;			/* last address in editor buffer */

long first_addr, second_addr;
long addr_cnt;

/* ------------------------------------------------------------------ */
/* ref_ renaming                                                        */
/* ------------------------------------------------------------------ */

#define check_addr_range	ref_check_addr_range
#define mark_line_node		ref_mark_line_node
#define unmark_line_node	ref_unmark_line_node
#define dup_line_node		ref_dup_line_node
#define has_trailing_escape	ref_has_trailing_escape
#define strip_escapes		ref_strip_escapes
#define is_legal_filename	ref_is_legal_filename

int check_addr_range(long, long);
int mark_line_node(line_t *, int);
void unmark_line_node(line_t *);
line_t *dup_line_node(line_t *);
int has_trailing_escape(char *, char *);
char *strip_escapes(char *);
int is_legal_filename(char *);

/* ------------------------------------------------------------------ */
/* Verbatim bodies from hbsd/src/bin/ed/main.c                          */
/* ------------------------------------------------------------------ */

/* check_addr_range: return status of address range check */
int
check_addr_range(long n, long m)
{
	if (addr_cnt == 0) {
		first_addr = n;
		second_addr = m;
	}
	if (first_addr > second_addr || 1 > first_addr ||
	    second_addr > addr_last) {
		errmsg = "invalid address";
		return ERR;
	}
	return 0;
}


#define MAXMARK 26			/* max number of marks */

line_t *mark[MAXMARK];			/* line markers */
int markno;				/* line marker count */

/* mark_line_node: set a line node mark */
int
mark_line_node(line_t *lp, int n)
{
	if (!islower((unsigned char)n)) {
		errmsg = "invalid mark character";
		return ERR;
	} else if (mark[n - 'a'] == NULL)
		markno++;
	mark[n - 'a'] = lp;
	return 0;
}


/* unmark_line_node: clear line node mark */
void
unmark_line_node(line_t *lp)
{
	int i;

	for (i = 0; markno && i < MAXMARK; i++)
		if (mark[i] == lp) {
			mark[i] = NULL;
			markno--;
		}
}


/* dup_line_node: return a pointer to a copy of a line node */
line_t *
dup_line_node(line_t *lp)
{
	line_t *np;

	if ((np = (line_t *) malloc(sizeof(line_t))) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "out of memory";
		return NULL;
	}
	np->seek = lp->seek;
	np->len = lp->len;
	return np;
}


/* has_trailing_escape:  return the parity of escapes preceding a character
   in a string */
int
has_trailing_escape(char *s, char *t)
{
    return (s == t || *(t - 1) != '\\') ? 0 : !has_trailing_escape(s, t - 1);
}


/* strip_escapes: return a copy of escaped string of at most length PATH_MAX */
char *
strip_escapes(char *s)
{
	static char *file = NULL;
	static int filesz = 0;

	int i = 0;

	REALLOC(file, filesz, PATH_MAX, NULL);
	while (i < filesz - 1	/* Worry about a possible trailing escape */
	       && (file[i++] = (*s == '\\') ? *++s : *s))
		s++;
	return file;
}


/* is_legal_filename: return a legal filename */
int
is_legal_filename(char *s)
{
	if (red && (*s == '!' || !strcmp(s, "..") || strchr(s, '/'))) {
		errmsg = "shell access restricted";
		return 0;
	}
	return 1;
}
