/*
 * PBSD batch b0219 -- reference oracle.
 *
 * This file is the SPECIFICATION for pbsd/bin/sh/b0219/port.cppm.  It is the
 * original HardenedBSD C source of
 *
 *	hbsd/src/bin/sh/memalloc.c
 *	hbsd/src/bin/sh/output.c
 *	hbsd/src/bin/sh/alias.c
 *
 * concatenated, with every function renamed with a "ref_" prefix (call sites
 * inside the bodies are renamed to match, otherwise nothing would link) and
 * with the "static" storage class dropped so that the differential harness can
 * reach the file-local helpers.  No function body has been modified.
 *
 * Everything above the "===" banner is support scaffolding: the handful of
 * declarations that live in headers which are not part of this batch
 * (shell.h, error.h, output.h, memalloc.h, mystring.h, alias.h).  The exact
 * same scaffolding is reproduced in port.cppm so that the two sides are
 * compared on equal footing.
 *
 * hbsd/src/bin/sh/show.c is not present; see skipped.txt.
 */

#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

/* ------------------------------------------------------------------ */
/* shell.h                                                            */
/* ------------------------------------------------------------------ */

typedef void *pointer;

/*
 * ALIGN() comes from <sys/param.h> on FreeBSD; it is not available on this
 * host, so it is spelled out here with the amd64 alignment (sizeof(register_t)
 * == 8).  port.cppm uses the identical definition.
 */
#ifndef ALIGNBYTES
#define	ALIGNBYTES	(sizeof(long) - 1)
#endif
#ifndef ALIGN
#define	ALIGN(p)	(((uintptr_t)(p) + ALIGNBYTES) & ~ALIGNBYTES)
#endif

/* ------------------------------------------------------------------ */
/* error.h                                                            */
/* ------------------------------------------------------------------ */

static volatile int suppressint = 1;
static volatile int intpending = 0;

void ref_onint(void);

#define	INTOFF		suppressint++
#define	INTON		{ if (--suppressint == 0 && intpending) ref_onint(); }
#define	is_int_on()	suppressint

void
ref_onint(void)
{
	abort();
}

static jmp_buf err_jmp;
static int err_armed;

static void error(const char *, ...) __attribute__((__noreturn__));

static void
error(const char *msg, ...)
{
	(void)msg;
	if (err_armed)
		longjmp(err_jmp, 1);
	abort();
}

/* ------------------------------------------------------------------ */
/* output.h                                                           */
/* ------------------------------------------------------------------ */

struct output {
	char *nextc;
	char *bufend;
	char *buf;
	int bufsize;
	short fd;
	short flags;
};

/*
 * emptyoutbuf() is not part of this batch (it calls flushout(), which calls
 * xwrite(); see skipped.txt), so the outc() macro of output.h is wired to a
 * scaffolding routine that simply refuses to continue.  The harness always
 * hands these routines a buffer that is large enough, so it is never reached.
 */
void outbuf_overflow(struct output *) __attribute__((__noreturn__));

void
outbuf_overflow(struct output *dest)
{
	(void)dest;
	write(2, "b0219: output buffer overflow\n", 30);
	abort();
}

#define	outc(c, file)	((file)->nextc == (file)->bufend ? \
			    (outbuf_overflow(file), *(file)->nextc++ = (c)) : \
			    (*(file)->nextc++ = (c)))
#define	out1c(c)	outc(c, out1);

/* ------------------------------------------------------------------ */
/* memalloc.h                                                         */
/* ------------------------------------------------------------------ */

struct stackmark {
	struct stack_block *stackp;
	char *stacknxt;
	int stacknleft;
};

char *ref_makestrspace(int, char *);

#define	stackblock()		stacknxt
#define	stackblocksize()	stacknleft
#define	CHECKSTRSPACE(n, p)	{ if (sstrend - (p) < (n)) \
				    (p) = ref_makestrspace((n), (p)); }

/* ------------------------------------------------------------------ */
/* mystring.h                                                         */
/* ------------------------------------------------------------------ */

#define	equal(s1, s2)	(strcmp(s1, s2) == 0)

/* ------------------------------------------------------------------ */
/* alias.h                                                            */
/* ------------------------------------------------------------------ */

struct alias {
	struct alias *next;
	char *name;
	char *val;
	int flag;
};

#define	ALIASINUSE	1

/* forward declarations of the ported routines */
pointer ref_ckmalloc(size_t);
pointer ref_ckrealloc(pointer, int);
void ref_ckfree(pointer);
char *ref_savestr(const char *);
void ref_stnewblock(int);
pointer ref_stalloc(int);
void ref_stunalloc(pointer);
char *ref_stsavestr(const char *);
void ref_setstackmark(struct stackmark *);
void ref_popstackmark(struct stackmark *);
void ref_growstackblock(int);
char *ref_growstrstackblock(int, int);
char *ref_growstackstr(void);
char *ref_stputbin(const char *, size_t, char *);
char *ref_stputs(const char *, char *);

void ref_outcslow(int, struct output *);
void ref_out1str(const char *);
void ref_out1qstr(const char *);
void ref_out2str(const char *);
void ref_out2qstr(const char *);
void ref_outstr(const char *, struct output *);
void ref_byteseq(int, struct output *);
void ref_outdqstr(const char *, struct output *);
void ref_outqstr(const char *, struct output *);
void ref_outbin(const void *, size_t, struct output *);
void ref_freestdout(void);
int ref_outiserror(struct output *);
void ref_outclearerror(struct output *);
void ref_fmtstr(char *, int, const char *, ...);

void ref_setalias(const char *, const char *);
void ref_freealias(struct alias *);
int ref_unalias(const char *);
void ref_rmaliases(void);
struct alias *ref_lookupalias(const char *, int);
int ref_comparealiases(const void *, const void *);
size_t ref_hashalias(const char *);
const struct alias *ref_iteralias(const struct alias *);

/* ================================================================== */
/* hbsd/src/bin/sh/memalloc.c                                         */
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
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

void
ref_badalloc(const char *message)
{
	write(2, message, strlen(message));
	abort();
}

/*
 * Like malloc, but returns an error when out of space.
 */

pointer
ref_ckmalloc(size_t nbytes)
{
	pointer p;

	if (!is_int_on())
		ref_badalloc("Unsafe ckmalloc() call\n");
	p = malloc(nbytes);
	if (p == NULL)
		error("Out of space");
	return p;
}


/*
 * Same for realloc.
 */

pointer
ref_ckrealloc(pointer p, int nbytes)
{
	if (!is_int_on())
		ref_badalloc("Unsafe ckrealloc() call\n");
	p = realloc(p, nbytes);
	if (p == NULL)
		error("Out of space");
	return p;
}

void
ref_ckfree(pointer p)
{
	if (!is_int_on())
		ref_badalloc("Unsafe ckfree() call\n");
	free(p);
}


/*
 * Make a copy of a string in safe storage.
 */

char *
ref_savestr(const char *s)
{
	char *p;
	size_t len;

	len = strlen(s);
	p = ref_ckmalloc(len + 1);
	memcpy(p, s, len + 1);
	return p;
}


/*
 * Parse trees for commands are allocated in lifo order, so we use a stack
 * to make this more efficient, and also to avoid all sorts of exception
 * handling code to handle interrupts in the middle of a parse.
 *
 * The size 496 was chosen because with 16-byte alignment the total size
 * for the allocated block is 512.
 */

#define MINSIZE 496		/* minimum size of a block. */


struct stack_block {
	struct stack_block *prev;
	/* Data follows */
};
#define SPACE(sp)	((char*)(sp) + ALIGN(sizeof(struct stack_block)))

static struct stack_block *stackp;
static char *stacknxt;
static int stacknleft;
static char *sstrend;


void
ref_stnewblock(int nbytes)
{
	struct stack_block *sp;
	int allocsize;

	if (nbytes < MINSIZE)
		nbytes = MINSIZE;

	allocsize = ALIGN(sizeof(struct stack_block)) + ALIGN(nbytes);

	INTOFF;
	sp = ref_ckmalloc(allocsize);
	sp->prev = stackp;
	stacknxt = SPACE(sp);
	stacknleft = allocsize - (stacknxt - (char*)sp);
	sstrend = stacknxt + stacknleft;
	stackp = sp;
	INTON;
}


pointer
ref_stalloc(int nbytes)
{
	char *p;

	nbytes = ALIGN(nbytes);
	if (nbytes > stacknleft)
		ref_stnewblock(nbytes);
	p = stacknxt;
	stacknxt += nbytes;
	stacknleft -= nbytes;
	return p;
}


void
ref_stunalloc(pointer p)
{
	if (p == NULL) {		/*DEBUG */
		write(STDERR_FILENO, "stunalloc\n", 10);
		abort();
	}
	stacknleft += stacknxt - (char *)p;
	stacknxt = p;
}


char *
ref_stsavestr(const char *s)
{
	char *p;
	size_t len;

	len = strlen(s);
	p = ref_stalloc(len + 1);
	memcpy(p, s, len + 1);
	return p;
}


void
ref_setstackmark(struct stackmark *mark)
{
	mark->stackp = stackp;
	mark->stacknxt = stacknxt;
	mark->stacknleft = stacknleft;
	/* Ensure this block stays in place. */
	if (stackp != NULL && stacknxt == SPACE(stackp))
		ref_stalloc(1);
}


void
ref_popstackmark(struct stackmark *mark)
{
	struct stack_block *sp;

	INTOFF;
	while (stackp != mark->stackp) {
		sp = stackp;
		stackp = sp->prev;
		ref_ckfree(sp);
	}
	stacknxt = mark->stacknxt;
	stacknleft = mark->stacknleft;
	if (stacknleft != 0)
		sstrend = stacknxt + stacknleft;
	else
		sstrend = stacknxt;
	INTON;
}


/*
 * When the parser reads in a string, it wants to stick the string on the
 * stack and only adjust the stack pointer when it knows how big the
 * string is.  Stackblock (defined in stack.h) returns a pointer to a block
 * of space on top of the stack and stackblocklen returns the length of
 * this block.  Growstackblock will grow this space by at least one byte,
 * possibly moving it (like realloc).  Grabstackblock actually allocates the
 * part of the block that has been used.
 */

void
ref_growstackblock(int min)
{
	char *p;
	int newlen;
	char *oldspace;
	int oldlen;
	struct stack_block *sp;
	struct stack_block *oldstackp;

	if (min < stacknleft)
		min = stacknleft;
	if ((unsigned int)min >=
	    INT_MAX / 2 - ALIGN(sizeof(struct stack_block)))
		error("Out of space");
	min += stacknleft;
	min += ALIGN(sizeof(struct stack_block));
	newlen = 512;
	while (newlen < min)
		newlen <<= 1;
	oldspace = stacknxt;
	oldlen = stacknleft;

	if (stackp != NULL && stacknxt == SPACE(stackp)) {
		INTOFF;
		oldstackp = stackp;
		stackp = oldstackp->prev;
		sp = ref_ckrealloc((pointer)oldstackp, newlen);
		sp->prev = stackp;
		stackp = sp;
		stacknxt = SPACE(sp);
		stacknleft = newlen - (stacknxt - (char*)sp);
		sstrend = stacknxt + stacknleft;
		INTON;
	} else {
		newlen -= ALIGN(sizeof(struct stack_block));
		p = ref_stalloc(newlen);
		if (oldlen != 0)
			memcpy(p, oldspace, oldlen);
		ref_stunalloc(p);
	}
}



/*
 * The following routines are somewhat easier to use than the above.
 * The user declares a variable of type STACKSTR, which may be declared
 * to be a register.  The macro STARTSTACKSTR initializes things.  Then
 * the user uses the macro STPUTC to add characters to the string.  In
 * effect, STPUTC(c, p) is the same as *p++ = c except that the stack is
 * grown as necessary.  When the user is done, she can just leave the
 * string there and refer to it using stackblock().  Or she can allocate
 * the space for it using grabstackstr().  If it is necessary to allow
 * someone else to use the stack temporarily and then continue to grow
 * the string, the user should use grabstack to allocate the space, and
 * then call ungrabstr(p) to return to the previous mode of operation.
 *
 * USTPUTC is like STPUTC except that it doesn't check for overflow.
 * CHECKSTACKSPACE can be called before USTPUTC to ensure that there
 * is space for at least one character.
 */

char *
ref_growstrstackblock(int n, int min)
{
	ref_growstackblock(min);
	return stackblock() + n;
}

char *
ref_growstackstr(void)
{
	int len;

	len = stackblocksize();
	return (ref_growstrstackblock(len, 0));
}


/*
 * Called from CHECKSTRSPACE.
 */

char *
ref_makestrspace(int min, char *p)
{
	int len;

	len = p - stackblock();
	return (ref_growstrstackblock(len, min));
}


char *
ref_stputbin(const char *data, size_t len, char *p)
{
	CHECKSTRSPACE(len, p);
	memcpy(p, data, len);
	return (p + len);
}

char *
ref_stputs(const char *data, char *p)
{
	return (ref_stputbin(data, strlen(data), p));
}

/* ================================================================== */
/* hbsd/src/bin/sh/output.c                                           */
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
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
 * Shell output routines.  We use our own output routines because:
 *	When a builtin command is interrupted we have to discard
 *		any pending output.
 *	When a builtin command appears in back quotes, we want to
 *		save the output of the command in a region obtained
 *		via malloc, rather than doing a fork and reading the
 *		output of the command via a pipe.
 */

#define OUTBUFSIZ BUFSIZ
#define MEM_OUT -2		/* output to dynamically allocated memory */
#define OUTPUT_ERR 01		/* error occurred on output */

static struct output output = {NULL, NULL, NULL, OUTBUFSIZ, 1, 0};
static struct output errout = {NULL, NULL, NULL, 256, 2, 0};
static struct output memout = {NULL, NULL, NULL, 64, MEM_OUT, 0};
static struct output *out1 = &output;
static struct output *out2 = &errout;

void
ref_outcslow(int c, struct output *file)
{
	outc(c, file);
}

void
ref_out1str(const char *p)
{
	ref_outstr(p, out1);
}

void
ref_out1qstr(const char *p)
{
	ref_outqstr(p, out1);
}

void
ref_out2str(const char *p)
{
	ref_outstr(p, out2);
}

void
ref_out2qstr(const char *p)
{
	ref_outqstr(p, out2);
}

void
ref_outstr(const char *p, struct output *file)
{
	ref_outbin(p, strlen(p), file);
}

void
ref_byteseq(int ch, struct output *file)
{
	char seq[4];

	seq[0] = '\\';
	seq[1] = (ch >> 6 & 0x3) + '0';
	seq[2] = (ch >> 3 & 0x7) + '0';
	seq[3] = (ch & 0x7) + '0';
	ref_outbin(seq, 4, file);
}

void
ref_outdqstr(const char *p, struct output *file)
{
	const char *end;
	mbstate_t mbs;
	size_t clen;
	wchar_t wc;

	memset(&mbs, '\0', sizeof(mbs));
	end = p + strlen(p);
	ref_outstr("$'", file);
	while ((clen = mbrtowc(&wc, p, end - p + 1, &mbs)) != 0) {
		if (clen == (size_t)-2) {
			while (p < end)
				ref_byteseq(*p++, file);
			break;
		}
		if (clen == (size_t)-1) {
			memset(&mbs, '\0', sizeof(mbs));
			ref_byteseq(*p++, file);
			continue;
		}
		if (wc == L'\n')
			ref_outcslow('\n', file), p++;
		else if (wc == L'\r')
			ref_outstr("\\r", file), p++;
		else if (wc == L'\t')
			ref_outstr("\\t", file), p++;
		else if (!iswprint(wc)) {
			for (; clen > 0; clen--)
				ref_byteseq(*p++, file);
		} else {
			if (wc == L'\'' || wc == L'\\')
				ref_outcslow('\\', file);
			ref_outbin(p, clen, file);
			p += clen;
		}
	}
	ref_outcslow('\'', file);
}

/* Like outstr(), but quote for re-input into the shell. */
void
ref_outqstr(const char *p, struct output *file)
{
	int i;

	if (p[0] == '\0') {
		ref_outstr("''", file);
		return;
	}
	for (i = 0; p[i] != '\0'; i++) {
		if ((p[i] > '\0' && p[i] < ' ' && p[i] != '\n') ||
		    (p[i] & 0x80) != 0 || p[i] == '\'') {
			ref_outdqstr(p, file);
			return;
		}
	}

	if (p[strcspn(p, "|&;<>()$`\\\" \n*?[~#=")] == '\0' ||
			strcmp(p, "[") == 0) {
		ref_outstr(p, file);
		return;
	}

	ref_outcslow('\'', file);
	ref_outstr(p, file);
	ref_outcslow('\'', file);
}

void
ref_outbin(const void *data, size_t len, struct output *file)
{
	const char *p;

	p = data;
	while (len-- > 0)
		outc(*p++, file);
}


void
ref_freestdout(void)
{
	output.nextc = output.buf;
}


int
ref_outiserror(struct output *file)
{
	return (file->flags & OUTPUT_ERR);
}


void
ref_outclearerror(struct output *file)
{
	file->flags &= ~OUTPUT_ERR;
}


void
ref_fmtstr(char *outbuf, int length, const char *fmt, ...)
{
	va_list ap;

	INTOFF;
	va_start(ap, fmt);
	vsnprintf(outbuf, length, fmt, ap);
	va_end(ap);
	INTON;
}

/* ================================================================== */
/* hbsd/src/bin/sh/alias.c                                            */
/* ================================================================== */

/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
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

#define ATABSIZE 39

static struct alias *atab[ATABSIZE];
static int aliases;

void
ref_setalias(const char *name, const char *val)
{
	struct alias *ap, **app;

	ref_unalias(name);
	app = &atab[ref_hashalias(name)];
	INTOFF;
	ap = ref_ckmalloc(sizeof (struct alias));
	ap->name = ref_savestr(name);
	ap->val = ref_savestr(val);
	ap->flag = 0;
	ap->next = *app;
	*app = ap;
	aliases++;
	INTON;
}

void
ref_freealias(struct alias *ap)
{
	ref_ckfree(ap->name);
	ref_ckfree(ap->val);
	ref_ckfree(ap);
}

int
ref_unalias(const char *name)
{
	struct alias *ap, **app;

	app = &atab[ref_hashalias(name)];

	for (ap = *app; ap; app = &(ap->next), ap = ap->next) {
		if (equal(name, ap->name)) {
			/*
			 * if the alias is currently in use (i.e. its
			 * buffer is being used by the input routine) we
			 * just null out the name instead of freeing it.
			 * We could clear it out later, but this situation
			 * is so rare that it hardly seems worth it.
			 */
			if (ap->flag & ALIASINUSE)
				*ap->name = '\0';
			else {
				INTOFF;
				*app = ap->next;
				ref_freealias(ap);
				INTON;
			}
			aliases--;
			return (0);
		}
	}

	return (1);
}

void
ref_rmaliases(void)
{
	struct alias *ap, **app;
	int i;

	INTOFF;
	for (i = 0; i < ATABSIZE; i++) {
		app = &atab[i];
		while (*app) {
			ap = *app;
			if (ap->flag & ALIASINUSE) {
				*ap->name = '\0';
				app = &(*app)->next;
			} else {
				*app = ap->next;
				ref_freealias(ap);
			}
		}
	}
	aliases = 0;
	INTON;
}

struct alias *
ref_lookupalias(const char *name, int check)
{
	struct alias *ap;

	if (aliases == 0)
		return (NULL);
	for (ap = atab[ref_hashalias(name)]; ap; ap = ap->next) {
		if (equal(name, ap->name)) {
			if (check && (ap->flag & ALIASINUSE))
				return (NULL);
			return (ap);
		}
	}

	return (NULL);
}

int
ref_comparealiases(const void *p1, const void *p2)
{
	const struct alias *const *a1 = p1;
	const struct alias *const *a2 = p2;

	return strcmp((*a1)->name, (*a2)->name);
}

size_t
ref_hashalias(const char *p)
{
	unsigned int hashval;

	hashval = (unsigned char)*p << 4;
	while (*p)
		hashval+= *p++;
	return (hashval % ATABSIZE);
}

const struct alias *
ref_iteralias(const struct alias *index)
{
	size_t i = 0;

	if (index != NULL) {
		if (index->next != NULL)
			return (index->next);
		i = ref_hashalias(index->name) + 1;
	}
	for (; i < ATABSIZE; i++)
		if (atab[i] != NULL)
			return (atab[i]);

	return (NULL);
}

/* ================================================================== */
/* Harness support probes.  These are not ports of anything; they only  */
/* expose file-scope state so that the differential harness can compare */
/* it.  No behaviour of the routines above depends on them.             */
/* ================================================================== */

void
ref_set_suppressint(int v)
{
	suppressint = v;
}

int
ref_get_suppressint(void)
{
	return suppressint;
}

void
ref_err_arm(int v)
{
	err_armed = v;
}

jmp_buf *
ref_err_jmp(void)
{
	return &err_jmp;
}

void
ref_stack_probe(int *nleft, long *nxtoff, long *ssoff, int *depth)
{
	struct stack_block *sp;
	int d = 0;

	*nleft = stacknleft;
	*nxtoff = stackp != NULL ? (long)(stacknxt - (char *)stackp) : -1L;
	*ssoff = (long)(sstrend - stacknxt);
	for (sp = stackp; sp != NULL; sp = sp->prev)
		d++;
	*depth = d;
}

char *
ref_get_stacknxt(void)
{
	return stacknxt;
}

char *
ref_get_stackp(void)
{
	return (char *)stackp;
}

char *
ref_get_sstrend(void)
{
	return sstrend;
}

int
ref_get_stacknleft(void)
{
	return stacknleft;
}

size_t
ref_sizeof_stackmark(void)
{
	return sizeof(struct stackmark);
}

size_t
ref_sizeof_alias(void)
{
	return sizeof(struct alias);
}

int
ref_alias_count(void)
{
	return aliases;
}

struct alias *
ref_atab_get(int i)
{
	return atab[i];
}

struct output *
ref_var_output(void)
{
	return &output;
}

struct output *
ref_var_errout(void)
{
	return &errout;
}

struct output *
ref_var_memout(void)
{
	return &memout;
}

struct output **
ref_var_out1(void)
{
	return &out1;
}

struct output **
ref_var_out2(void)
{
	return &out2;
}
