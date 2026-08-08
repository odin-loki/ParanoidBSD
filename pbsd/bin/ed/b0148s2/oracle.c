#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
/*
 * oracle.c -- reference specification for PBSD batch b0148s2.
 *
 * Original HardenedBSD undo.c concatenated; every batch function is
 * renamed with a ref_ prefix.  Function bodies are UNMODIFIED.  Supporting
 * types, macros, globals, and ed.h shims are added only where required so
 * the unmodified bodies compile and link.
 */

#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#define ERR		(-2)
#define EMOD		(-3)
#define FATAL		(-4)

#define MINBUFSZ 512
#ifdef INT_MAX
# define LINECHARS INT_MAX
#else
# define LINECHARS MAXINT
#endif

#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif

typedef struct line {
	struct line	*q_forw;
	struct line	*q_back;
	off_t		seek;
	int		len;
} line_t;

typedef struct undo {
#define UADD	0
#define UDEL 	1
#define UMOV	2
#define VMOV	3
	int type;
	line_t	*h;
	line_t  *t;
} undo_t;

#define REQUE(pred, succ) (pred)->q_forw = (succ), (succ)->q_back = (pred)

#define INSQUE(elem, pred) \
{ \
	REQUE((elem), (pred)->q_forw); \
	REQUE((pred), elem); \
}

#define SPL1() mutex++

#define SPL0() \
if (--mutex == 0) { \
	if (sigflags & (1 << (SIGHUP - 1))) handle_hup(SIGHUP); \
	if (sigflags & (1 << (SIGINT - 1))) handle_int(SIGINT); \
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

int isbinary;
int isglobal;
int modified;
int mutex;
int sigflags;
int newline_added;
int seek_write;

long addr_last;
long current_addr;

const char *errmsg = "";

int oracle_malloc_fail_at;
int oracle_malloc_calls;
int oracle_realloc_fail_at;
int oracle_realloc_calls;
int oracle_hup_calls;
int oracle_int_calls;

void
oracle_reset_hooks(void)
{
	oracle_malloc_fail_at = 0;
	oracle_malloc_calls = 0;
	oracle_realloc_fail_at = 0;
	oracle_realloc_calls = 0;
	oracle_hup_calls = 0;
	oracle_int_calls = 0;
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

static void *
oracle_realloc(void *q, size_t n)
{
	void *p;

	oracle_realloc_calls++;
	if (oracle_realloc_fail_at != 0 &&
	    oracle_realloc_calls >= oracle_realloc_fail_at)
		return (NULL);
	p = realloc(q, n);
	return (p);
}

#define malloc oracle_malloc

void
oracle_reset_globals(void)
{
	isbinary = 0;
	isglobal = 0;
	modified = 0;
	mutex = 0;
	sigflags = 0;
	newline_added = 0;
	seek_write = 0;
	addr_last = 0;
	current_addr = 0;
	errmsg = "";
	oracle_reset_hooks();
}

void
unmark_line_node(line_t *lp)
{
	(void)lp;
}

void
handle_hup(int s)
{
	(void)s;
	oracle_hup_calls++;
}

void
handle_int(int s)
{
	(void)s;
	oracle_int_calls++;
}

undo_t *ref_push_undo_stack(int type, long from, long to);
int ref_pop_undo_stack(void);
void ref_clear_undo_stack(void);
line_t *ref_get_addressed_line_node(long n);
void ref_clear_active_list(void);
const char *ref_put_sbuf_line(const char *cs);
void ref_add_line_node(line_t *lp);
long ref_get_line_node_addr(line_t *lp);
char *ref_get_sbuf_line(line_t *lp);
int ref_open_sbuf(void);
int ref_close_sbuf(void);

#define push_undo_stack		ref_push_undo_stack
#define pop_undo_stack		ref_pop_undo_stack
#define clear_undo_stack	ref_clear_undo_stack
#define get_addressed_line_node	ref_get_addressed_line_node
#define clear_active_list	ref_clear_active_list
#define put_sbuf_line		ref_put_sbuf_line
#define add_line_node		ref_add_line_node
#define get_line_node_addr	ref_get_line_node_addr
#define get_sbuf_line		ref_get_sbuf_line
#define open_sbuf		ref_open_sbuf
#define close_sbuf		ref_close_sbuf

#define realloc oracle_realloc

/* ------------------------------------------------------------------ */
/* hbsd/src/bin/ed/undo.c                                                */
/* ------------------------------------------------------------------ */

/* undo.c: This file contains the undo routines for the ed line editor */
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


#define USIZE 100				/* undo stack size */
static undo_t *ustack = NULL;			/* undo stack */
static long usize = 0;				/* stack size variable */
static long u_p = 0;				/* undo stack pointer */

undo_t *
ref_push_undo_stack(int type, long from, long to)
{
	undo_t *t;

#if defined(sun) || defined(NO_REALLOC_NULL)
	if (ustack == NULL &&
	    (ustack = (undo_t *) malloc((usize = USIZE) * sizeof(undo_t))) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "out of memory";
		return NULL;
	}
#endif
	t = ustack;
	if (u_p < usize ||
	    (t = (undo_t *) realloc(ustack, (usize += USIZE) * sizeof(undo_t))) != NULL) {
		ustack = t;
		ustack[u_p].type = type;
		ustack[u_p].t = get_addressed_line_node(to);
		ustack[u_p].h = get_addressed_line_node(from);
		return ustack + u_p++;
	}
	/* out of memory - release undo stack */
	fprintf(stderr, "%s\n", strerror(errno));
	errmsg = "out of memory";
	clear_undo_stack();
	free(ustack);
	ustack = NULL;
	usize = 0;
	return NULL;
}


#define USWAP(x,y) { \
	undo_t utmp; \
	utmp = x, x = y, y = utmp; \
}


long u_current_addr = -1;	/* if >= 0, undo enabled */
long u_addr_last = -1;		/* if >= 0, undo enabled */

int
ref_pop_undo_stack(void)
{
	long n;
	long o_current_addr = current_addr;
	long o_addr_last = addr_last;

	if (u_current_addr == -1 || u_addr_last == -1) {
		errmsg = "nothing to undo";
		return ERR;
	} else if (u_p)
		modified = 1;
	get_addressed_line_node(0);	/* this get_addressed_line_node last! */
	SPL1();
	for (n = u_p; n-- > 0;) {
		switch(ustack[n].type) {
		case UADD:
			REQUE(ustack[n].h->q_back, ustack[n].t->q_forw);
			break;
		case UDEL:
			REQUE(ustack[n].h->q_back, ustack[n].h);
			REQUE(ustack[n].t, ustack[n].t->q_forw);
			break;
		case UMOV:
		case VMOV:
			REQUE(ustack[n - 1].h, ustack[n].h->q_forw);
			REQUE(ustack[n].t->q_back, ustack[n - 1].t);
			REQUE(ustack[n].h, ustack[n].t);
			n--;
			break;
		default:
			/*NOTREACHED*/
			;
		}
		ustack[n].type ^= 1;
	}
	/* reverse undo stack order */
	for (n = u_p; n-- > (u_p + 1)/ 2;)
		USWAP(ustack[n], ustack[u_p - 1 - n]);
	if (isglobal)
		clear_active_list();
	current_addr = u_current_addr, u_current_addr = o_current_addr;
	addr_last = u_addr_last, u_addr_last = o_addr_last;
	SPL0();
	return 0;
}


void
ref_clear_undo_stack(void)
{
	line_t *lp, *ep, *tl;

	while (u_p--)
		if (ustack[u_p].type == UDEL) {
			ep = ustack[u_p].t->q_forw;
			for (lp = ustack[u_p].h; lp != ep; lp = tl) {
				unmark_line_node(lp);
				tl = lp->q_forw;
				free(lp);
			}
		}
	u_p = 0;
	u_current_addr = current_addr;
	u_addr_last = addr_last;
}

#undef realloc

long
oracle_u_p(void)
{
	return u_p;
}

long
oracle_usize(void)
{
	return usize;
}

long
oracle_ustack_off(void *e)
{
	if (e == NULL || ustack == NULL)
		return (-1);
	return ((undo_t *)e - ustack);
}

int
oracle_stack_entry(long i, int *type, void **h, void **t)
{
	if (i < 0 || i >= u_p || ustack == NULL)
		return (0);
	*type = ustack[i].type;
	*h = ustack[i].h;
	*t = ustack[i].t;
	return (1);
}

void
oracle_discard_undo_stack(void)
{
	free(ustack);
	ustack = NULL;
	usize = 0;
	u_p = 0;
	u_current_addr = -1;
	u_addr_last = -1;
}

static undo_t *
oracle_push_undo_nodes(int type, line_t *h, line_t *t)
{
	undo_t *tp;

#if defined(sun) || defined(NO_REALLOC_NULL)
	if (ustack == NULL &&
	    (ustack = (undo_t *) malloc((usize = USIZE) * sizeof(undo_t))) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "out of memory";
		return NULL;
	}
#endif
	tp = ustack;
	if (u_p < usize ||
	    (tp = (undo_t *) realloc(ustack, (usize += USIZE) * sizeof(undo_t))) != NULL) {
		ustack = tp;
		ustack[u_p].type = type;
		ustack[u_p].h = h;
		ustack[u_p].t = t;
		return ustack + u_p++;
	}
	fprintf(stderr, "%s\n", strerror(errno));
	errmsg = "out of memory";
	clear_undo_stack();
	free(ustack);
	ustack = NULL;
	usize = 0;
	return NULL;
}

void
oracle_inject_orphan_udel(long n)
{
	line_t *lp;

	lp = get_addressed_line_node(n);
	REQUE(lp->q_back, lp->q_forw);
	addr_last--;
	get_addressed_line_node(0);
	oracle_push_undo_nodes(UDEL, lp, lp);
}

static line_t **active_list;
static long active_last;
static long active_size;
static long active_ptr;
static long active_ndx;

void
ref_clear_active_list(void)
{
	SPL1();
	active_size = active_last = active_ptr = active_ndx = 0;
	free(active_list);
	active_list = NULL;
	SPL0();
}

static FILE *sfp;
static off_t sfseek;
static line_t buffer_head;

void *
oracle_buffer_head(void)
{
	return (&buffer_head);
}

void *
oracle_node_forw(void *p)
{
	return (((line_t *)p)->q_forw);
}

void *
oracle_node_back(void *p)
{
	return (((line_t *)p)->q_back);
}

char *
ref_get_sbuf_line(line_t *lp)
{
	static char *sfbuf = NULL;
	static size_t sfbufsz;

	size_t len;

	if (lp == &buffer_head)
		return NULL;
	seek_write = 1;
	if (sfseek != lp->seek) {
		sfseek = lp->seek;
		if (fseeko(sfp, sfseek, SEEK_SET) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			errmsg = "cannot seek temp file";
			return NULL;
		}
	}
	len = lp->len;
	REALLOC(sfbuf, sfbufsz, len + 1, NULL);
	if (fread(sfbuf, sizeof(char), len, sfp) != len) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "cannot read temp file";
		return NULL;
	}
	sfseek += len;
	sfbuf[len] = '\0';
	return sfbuf;
}

const char *
ref_put_sbuf_line(const char *cs)
{
	line_t *lp;
	size_t len;
	const char *s;

	if ((lp = (line_t *) malloc(sizeof(line_t))) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "out of memory";
		return NULL;
	}
	for (s = cs; *s != '\n'; s++)
		;
	if (s - cs >= LINECHARS) {
		errmsg = "line too long";
		free(lp);
		return NULL;
	}
	len = s - cs;
	if (seek_write) {
		if (fseeko(sfp, (off_t)0, SEEK_END) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			errmsg = "cannot seek temp file";
			free(lp);
			return NULL;
		}
		sfseek = ftello(sfp);
		seek_write = 0;
	}
	if (fwrite(cs, sizeof(char), len, sfp) != len) {
		sfseek = -1;
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "cannot write temp file";
		free(lp);
		return NULL;
	}
	lp->len = len;
	lp->seek  = sfseek;
	add_line_node(lp);
	sfseek += len;
	return ++s;
}

void
ref_add_line_node(line_t *lp)
{
	line_t *cp;

	cp = get_addressed_line_node(current_addr);
	INSQUE(lp, cp);
	addr_last++;
	current_addr++;
}

long
ref_get_line_node_addr(line_t *lp)
{
	line_t *cp = &buffer_head;
	long n = 0;

	while (cp != lp && (cp = cp->q_forw) != &buffer_head)
		n++;
	if (n && cp == &buffer_head) {
		errmsg = "invalid address";
		return ERR;
	}
	 return n;
}

line_t *
ref_get_addressed_line_node(long n)
{
	static line_t *lp = &buffer_head;
	static long on = 0;

	SPL1();
	if (n > on)
		if (n <= (on + addr_last) >> 1)
			for (; on < n; on++)
				lp = lp->q_forw;
		else {
			lp = buffer_head.q_back;
			for (on = addr_last; on > n; on--)
				lp = lp->q_back;
		}
	else
		if (n >= on >> 1)
			for (; on > n; on--)
				lp = lp->q_back;
		else {
			lp = &buffer_head;
			for (on = 0; on < n; on++)
				lp = lp->q_forw;
		}
	SPL0();
	return lp;
}

static char sfn[15] = "";

int
ref_open_sbuf(void)
{
	int fd;
	int u;

	isbinary = newline_added = 0;
	u = umask(077);
	strcpy(sfn, "/tmp/ed.XXXXXX");
	if ((fd = mkstemp(sfn)) == -1 ||
	    (sfp = fdopen(fd, "w+")) == NULL) {
		if (fd != -1)
			close(fd);
		perror(sfn);
		errmsg = "cannot open temp file";
		umask(u);
		return ERR;
	}
	umask(u);
	return 0;
}

int
ref_close_sbuf(void)
{
	if (sfp) {
		if (fclose(sfp) < 0) {
			fprintf(stderr, "%s: %s\n", sfn, strerror(errno));
			errmsg = "cannot close temp file";
			return ERR;
		}
		sfp = NULL;
		unlink(sfn);
	}
	sfseek = seek_write = 0;
	return 0;
}

void
oracle_reset_batch(void)
{
	oracle_discard_undo_stack();
	clear_active_list();
	close_sbuf();
	oracle_reset_globals();
	REQUE(&buffer_head, &buffer_head);
}
