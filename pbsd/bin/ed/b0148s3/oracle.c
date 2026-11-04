/* oracle.c: reference build for batch b0148s3.
 *
 * Sources concatenated here (bodies UNMODIFIED, functions renamed ref_*):
 *   hbsd/src/bin/ed/glbl.c
 *
 * glbl.c includes "ed.h"; the declarations glbl.c actually needs from that
 * header are reproduced verbatim below so this file stands alone.
 */

/* glob.c: This file contains the global command routines for the ed line
   editor */
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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- from ed.h ---------------------------------------------------------- */

#ifndef LONG_BIT
#define LONG_BIT (CHAR_BIT * sizeof(long))
#endif

#define ERR		(-2)

#define MINBUFSZ 512		/* minimum buffer size - must be > 0 */

/* INC_MOD: increment a modulo l */
#define INC_MOD(l, k)	((l) + 1 > (k) ? 0 : (l) + 1)

/* Line node */
typedef struct	line {
	struct line	*q_forw;
	struct line	*q_back;
	off_t		seek;		/* address of line in scratch buffer */
	int		len;		/* length of line */
} line_t;

char *errmsg = NULL;		/* error message */

/*
 * mutex/SPL1/SPL0: ed's critical-section counter.  The sigflags dispatch that
 * ed.h's SPL0 performs lives in ed's signal machinery (main.c), which is not
 * part of this batch; sigflags is zero here, so the dispatch is unreachable
 * and only the counter is observable.
 */
static int mutex = 0;

/* SPL1: disable some interrupts (requires reliable signals) */
#define SPL1() mutex++

/* SPL0: enable all interrupts; check sigflags (requires reliable signals) */
#define SPL0() if (--mutex == 0) { }

/* ---- glbl.c ------------------------------------------------------------- */

static line_t **active_list;	/* list of lines active in a global command */
static long active_last;	/* index of last active line in active_list */
static long active_size;	/* size of active_list */
static long active_ptr;		/* active_list index (non-decreasing) */
static long active_ndx;		/* active_list index (modulo active_last) */

/* set_active_node: add a line node to the global-active list */
int
ref_set_active_node(line_t *lp)
{
	if (active_last + 1 > active_size) {
		size_t ti = active_size;
		line_t **ts;
		SPL1();
#if defined(sun) || defined(NO_REALLOC_NULL)
		if (active_list != NULL) {
#endif
			if ((ts = (line_t **) realloc(active_list,
			    (ti += MINBUFSZ) * sizeof(line_t *))) == NULL) {
				fprintf(stderr, "%s\n", strerror(errno));
				errmsg = "out of memory";
				SPL0();
				return ERR;
			}
#if defined(sun) || defined(NO_REALLOC_NULL)
		} else {
			if ((ts = (line_t **) malloc((ti += MINBUFSZ) *
			    sizeof(line_t **))) == NULL) {
				fprintf(stderr, "%s\n", strerror(errno));
				errmsg = "out of memory";
				SPL0();
				return ERR;
			}
		}
#endif
		active_size = ti;
		active_list = ts;
		SPL0();
	}
	active_list[active_last++] = lp;
	return 0;
}


/* unset_active_nodes: remove a range of lines from the global-active list */
void
ref_unset_active_nodes(line_t *np, line_t *mp)
{
	line_t *lp;
	long i;

	for (lp = np; lp != mp; lp = lp->q_forw)
		for (i = 0; i < active_last; i++)
			if (active_list[active_ndx] == lp) {
				active_list[active_ndx] = NULL;
				active_ndx = INC_MOD(active_ndx, active_last - 1);
				break;
			} else	active_ndx = INC_MOD(active_ndx, active_last - 1);
}


/* next_active_node: return the next global-active line node */
line_t *
ref_next_active_node(void)
{
	while (active_ptr < active_last && active_list[active_ptr] == NULL)
		active_ptr++;
	return (active_ptr < active_last) ? active_list[active_ptr++] : NULL;
}


/* clear_active_list: clear the global-active list */
void
ref_clear_active_list(void)
{
	SPL1();
	active_size = active_last = active_ptr = active_ndx = 0;
	free(active_list);
	active_list = NULL;
	SPL0();
}

/* ---- observability accessors (test scaffolding, not part of glbl.c) ----- */

long ref_get_active_last(void) { return active_last; }
long ref_get_active_size(void) { return active_size; }
long ref_get_active_ptr(void) { return active_ptr; }
long ref_get_active_ndx(void) { return active_ndx; }
line_t **ref_get_active_list(void) { return active_list; }
int ref_get_mutex(void) { return mutex; }
char *ref_get_errmsg(void) { return errmsg; }
void ref_reset_errmsg(void) { errmsg = NULL; }
