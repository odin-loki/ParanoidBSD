/*
 * PBSD batch b0219 -- C++23 module port of
 *
 *	hbsd/src/bin/sh/memalloc.c
 *	hbsd/src/bin/sh/output.c
 *	hbsd/src/bin/sh/alias.c
 *
 * The port is deliberately literal: signedness, evaluation order, pointer
 * arithmetic and the original bugs are all preserved.  The only changes are
 * those the C++ language forces (explicit casts away from void *) plus the
 * scaffolding for the declarations that live in headers outside this batch.
 *
 * hbsd/src/bin/sh/show.c is not ported; see skipped.txt.
 */

module;

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

export module pbsd.bin.sh.b0219;

/* ------------------------------------------------------------------ */
/* shell.h                                                            */
/* ------------------------------------------------------------------ */

/*
 * ALIGN() comes from <sys/param.h> on FreeBSD; it is not available on this
 * host, so it is spelled out here with the amd64 alignment (sizeof(register_t)
 * == 8).  oracle.c uses the identical definition.
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

#define	INTOFF		suppressint++
#define	INTON		{ if (--suppressint == 0 && intpending) onint(); }
#define	is_int_on()	suppressint

/* ------------------------------------------------------------------ */
/* output.h                                                           */
/* ------------------------------------------------------------------ */

/*
 * emptyoutbuf() is not part of this batch (it calls flushout(), which calls
 * xwrite(); see skipped.txt), so the outc() macro of output.h is wired to a
 * scaffolding routine that simply refuses to continue.  The harness always
 * hands these routines a buffer that is large enough, so it is never reached.
 */
#define	outc(c, file)	((file)->nextc == (file)->bufend ? \
			    (outbuf_overflow(file), *(file)->nextc++ = (c)) : \
			    (*(file)->nextc++ = (c)))
#define	out1c(c)	outc(c, out1);

/* ------------------------------------------------------------------ */
/* memalloc.h                                                         */
/* ------------------------------------------------------------------ */

#define	stackblock()		stacknxt
#define	stackblocksize()	stacknleft
#define	CHECKSTRSPACE(n, p)	{ if (sstrend - (p) < (n)) \
				    (p) = makestrspace((n), (p)); }

/* ------------------------------------------------------------------ */
/* mystring.h                                                         */
/* ------------------------------------------------------------------ */

#define	equal(s1, s2)	(strcmp(s1, s2) == 0)

/* ------------------------------------------------------------------ */

export namespace pbsd::bin_sh::b0219 {

/* --- shell.h ------------------------------------------------------ */

typedef void *pointer;

/* --- error.h ------------------------------------------------------ */

volatile int suppressint = 1;
volatile int intpending = 0;

void
onint(void)
{
	abort();
}

jmp_buf err_jmp;
int err_armed;

[[noreturn]] void
error(const char *msg, ...)
{
	(void)msg;
	if (err_armed)
		longjmp(err_jmp, 1);
	abort();
}

/* --- output.h ----------------------------------------------------- */

struct output {
	char *nextc;
	char *bufend;
	char *buf;
	int bufsize;
	short fd;
	short flags;
};

[[noreturn]] void
outbuf_overflow(struct output *dest)
{
	(void)dest;
	write(2, "b0219: output buffer overflow\n", 30);
	abort();
}

/* --- memalloc.h --------------------------------------------------- */

struct stackmark {
	struct stack_block *stackp;
	char *stacknxt;
	int stacknleft;
};

/* --- alias.h ------------------------------------------------------ */

struct alias {
	struct alias *next;
	char *name;
	char *val;
	int flag;
};

const int ALIASINUSE = 1;

/* forward declarations of the ported routines */
pointer ckmalloc(size_t);
pointer ckrealloc(pointer, int);
void ckfree(pointer);
char *savestr(const char *);
void stnewblock(int);
pointer stalloc(int);
void stunalloc(pointer);
char *stsavestr(const char *);
void setstackmark(struct stackmark *);
void popstackmark(struct stackmark *);
void growstackblock(int);
char *growstrstackblock(int, int);
char *growstackstr(void);
char *makestrspace(int, char *);
char *stputbin(const char *, size_t, char *);
char *stputs(const char *, char *);

void outcslow(int, struct output *);
void out1str(const char *);
void out1qstr(const char *);
void out2str(const char *);
void out2qstr(const char *);
void outstr(const char *, struct output *);
void byteseq(int, struct output *);
void outdqstr(const char *, struct output *);
void outqstr(const char *, struct output *);
void outbin(const void *, size_t, struct output *);
void freestdout(void);
int outiserror(struct output *);
void outclearerror(struct output *);
void fmtstr(char *, int, const char *, ...);

void setalias(const char *, const char *);
void freealias(struct alias *);
int unalias(const char *);
void rmaliases(void);
struct alias *lookupalias(const char *, int);
int comparealiases(const void *, const void *);
size_t hashalias(const char *);
const struct alias *iteralias(const struct alias *);

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
badalloc(const char *message)
{
	write(2, message, strlen(message));
	abort();
}

/*
 * Like malloc, but returns an error when out of space.
 */

pointer
ckmalloc(size_t nbytes)
{
	pointer p;

	if (!is_int_on())
		badalloc("Unsafe ckmalloc() call\n");
	p = malloc(nbytes);
	if (p == NULL)
		error("Out of space");
	return p;
}


/*
 * Same for realloc.
 */

pointer
ckrealloc(pointer p, int nbytes)
{
	if (!is_int_on())
		badalloc("Unsafe ckrealloc() call\n");
	p = realloc(p, nbytes);
	if (p == NULL)
		error("Out of space");
	return p;
}

void
ckfree(pointer p)
{
	if (!is_int_on())
		badalloc("Unsafe ckfree() call\n");
	free(p);
}


/*
 * Make a copy of a string in safe storage.
 */

char *
savestr(const char *s)
{
	char *p;
	size_t len;

	len = strlen(s);
	p = (char *)ckmalloc(len + 1);
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

struct stack_block *stackp;
char *stacknxt;
int stacknleft;
char *sstrend;


void
stnewblock(int nbytes)
{
	struct stack_block *sp;
	int allocsize;

	if (nbytes < MINSIZE)
		nbytes = MINSIZE;

	allocsize = ALIGN(sizeof(struct stack_block)) + ALIGN(nbytes);

	INTOFF;
	sp = (struct stack_block *)ckmalloc(allocsize);
	sp->prev = stackp;
	stacknxt = SPACE(sp);
	stacknleft = allocsize - (stacknxt - (char*)sp);
	sstrend = stacknxt + stacknleft;
	stackp = sp;
	INTON;
}


pointer
stalloc(int nbytes)
{
	char *p;

	nbytes = ALIGN(nbytes);
	if (nbytes > stacknleft)
		stnewblock(nbytes);
	p = stacknxt;
	stacknxt += nbytes;
	stacknleft -= nbytes;
	return p;
}


void
stunalloc(pointer p)
{
	if (p == NULL) {		/*DEBUG */
		write(STDERR_FILENO, "stunalloc\n", 10);
		abort();
	}
	stacknleft += stacknxt - (char *)p;
	stacknxt = (char *)p;
}


char *
stsavestr(const char *s)
{
	char *p;
	size_t len;

	len = strlen(s);
	p = (char *)stalloc(len + 1);
	memcpy(p, s, len + 1);
	return p;
}


void
setstackmark(struct stackmark *mark)
{
	mark->stackp = stackp;
	mark->stacknxt = stacknxt;
	mark->stacknleft = stacknleft;
	/* Ensure this block stays in place. */
	if (stackp != NULL && stacknxt == SPACE(stackp))
		stalloc(1);
}


void
popstackmark(struct stackmark *mark)
{
	struct stack_block *sp;

	INTOFF;
	while (stackp != mark->stackp) {
		sp = stackp;
		stackp = sp->prev;
		ckfree(sp);
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
growstackblock(int min)
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
		sp = (struct stack_block *)ckrealloc((pointer)oldstackp, newlen);
		sp->prev = stackp;
		stackp = sp;
		stacknxt = SPACE(sp);
		stacknleft = newlen - (stacknxt - (char*)sp);
		sstrend = stacknxt + stacknleft;
		INTON;
	} else {
		newlen -= ALIGN(sizeof(struct stack_block));
		p = (char *)stalloc(newlen);
		if (oldlen != 0)
			memcpy(p, oldspace, oldlen);
		stunalloc(p);
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
growstrstackblock(int n, int min)
{
	growstackblock(min);
	return stackblock() + n;
}

char *
growstackstr(void)
{
	int len;

	len = stackblocksize();
	return (growstrstackblock(len, 0));
}


/*
 * Called from CHECKSTRSPACE.
 */

char *
makestrspace(int min, char *p)
{
	int len;

	len = p - stackblock();
	return (growstrstackblock(len, min));
}


char *
stputbin(const char *data, size_t len, char *p)
{
	CHECKSTRSPACE(len, p);
	memcpy(p, data, len);
	return (p + len);
}

char *
stputs(const char *data, char *p)
{
	return (stputbin(data, strlen(data), p));
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

struct output output = {NULL, NULL, NULL, OUTBUFSIZ, 1, 0};
struct output errout = {NULL, NULL, NULL, 256, 2, 0};
struct output memout = {NULL, NULL, NULL, 64, MEM_OUT, 0};
struct output *out1 = &output;
struct output *out2 = &errout;

void
outcslow(int c, struct output *file)
{
	outc(c, file);
}

void
out1str(const char *p)
{
	outstr(p, out1);
}

void
out1qstr(const char *p)
{
	outqstr(p, out1);
}

void
out2str(const char *p)
{
	outstr(p, out2);
}

void
out2qstr(const char *p)
{
	outqstr(p, out2);
}

void
outstr(const char *p, struct output *file)
{
	outbin(p, strlen(p), file);
}

void
byteseq(int ch, struct output *file)
{
	char seq[4];

	seq[0] = '\\';
	seq[1] = (ch >> 6 & 0x3) + '0';
	seq[2] = (ch >> 3 & 0x7) + '0';
	seq[3] = (ch & 0x7) + '0';
	outbin(seq, 4, file);
}

void
outdqstr(const char *p, struct output *file)
{
	const char *end;
	mbstate_t mbs;
	size_t clen;
	wchar_t wc;

	memset(&mbs, '\0', sizeof(mbs));
	end = p + strlen(p);
	outstr("$'", file);
	while ((clen = mbrtowc(&wc, p, end - p + 1, &mbs)) != 0) {
		if (clen == (size_t)-2) {
			while (p < end)
				byteseq(*p++, file);
			break;
		}
		if (clen == (size_t)-1) {
			memset(&mbs, '\0', sizeof(mbs));
			byteseq(*p++, file);
			continue;
		}
		if (wc == L'\n')
			outcslow('\n', file), p++;
		else if (wc == L'\r')
			outstr("\\r", file), p++;
		else if (wc == L'\t')
			outstr("\\t", file), p++;
		else if (!iswprint(wc)) {
			for (; clen > 0; clen--)
				byteseq(*p++, file);
		} else {
			if (wc == L'\'' || wc == L'\\')
				outcslow('\\', file);
			outbin(p, clen, file);
			p += clen;
		}
	}
	outcslow('\'', file);
}

/* Like outstr(), but quote for re-input into the shell. */
void
outqstr(const char *p, struct output *file)
{
	int i;

	if (p[0] == '\0') {
		outstr("''", file);
		return;
	}
	for (i = 0; p[i] != '\0'; i++) {
		if ((p[i] > '\0' && p[i] < ' ' && p[i] != '\n') ||
		    (p[i] & 0x80) != 0 || p[i] == '\'') {
			outdqstr(p, file);
			return;
		}
	}

	if (p[strcspn(p, "|&;<>()$`\\\" \n*?[~#=")] == '\0' ||
			strcmp(p, "[") == 0) {
		outstr(p, file);
		return;
	}

	outcslow('\'', file);
	outstr(p, file);
	outcslow('\'', file);
}

void
outbin(const void *data, size_t len, struct output *file)
{
	const char *p;

	p = (const char *)data;
	while (len-- > 0)
		outc(*p++, file);
}


void
freestdout(void)
{
	output.nextc = output.buf;
}


int
outiserror(struct output *file)
{
	return (file->flags & OUTPUT_ERR);
}


void
outclearerror(struct output *file)
{
	file->flags &= ~OUTPUT_ERR;
}


void
fmtstr(char *outbuf, int length, const char *fmt, ...)
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

struct alias *atab[ATABSIZE];
int aliases;

void
setalias(const char *name, const char *val)
{
	struct alias *ap, **app;

	unalias(name);
	app = &atab[hashalias(name)];
	INTOFF;
	ap = (struct alias *)ckmalloc(sizeof (struct alias));
	ap->name = savestr(name);
	ap->val = savestr(val);
	ap->flag = 0;
	ap->next = *app;
	*app = ap;
	aliases++;
	INTON;
}

void
freealias(struct alias *ap)
{
	ckfree(ap->name);
	ckfree(ap->val);
	ckfree(ap);
}

int
unalias(const char *name)
{
	struct alias *ap, **app;

	app = &atab[hashalias(name)];

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
				freealias(ap);
				INTON;
			}
			aliases--;
			return (0);
		}
	}

	return (1);
}

void
rmaliases(void)
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
				freealias(ap);
			}
		}
	}
	aliases = 0;
	INTON;
}

struct alias *
lookupalias(const char *name, int check)
{
	struct alias *ap;

	if (aliases == 0)
		return (NULL);
	for (ap = atab[hashalias(name)]; ap; ap = ap->next) {
		if (equal(name, ap->name)) {
			if (check && (ap->flag & ALIASINUSE))
				return (NULL);
			return (ap);
		}
	}

	return (NULL);
}

int
comparealiases(const void *p1, const void *p2)
{
	const struct alias *const *a1 = (const struct alias *const *)p1;
	const struct alias *const *a2 = (const struct alias *const *)p2;

	return strcmp((*a1)->name, (*a2)->name);
}

size_t
hashalias(const char *p)
{
	unsigned int hashval;

	hashval = (unsigned char)*p << 4;
	while (*p)
		hashval+= *p++;
	return (hashval % ATABSIZE);
}

const struct alias *
iteralias(const struct alias *index)
{
	size_t i = 0;

	if (index != NULL) {
		if (index->next != NULL)
			return (index->next);
		i = hashalias(index->name) + 1;
	}
	for (; i < ATABSIZE; i++)
		if (atab[i] != NULL)
			return (atab[i]);

	return (NULL);
}

/* ================================================================== */
/* Harness support probes.  These are not ports of anything; they only  */
/* expose module-scope state so that the differential harness can       */
/* compare it.  No behaviour of the routines above depends on them.     */
/* ================================================================== */

void
set_suppressint(int v)
{
	suppressint = v;
}

int
get_suppressint(void)
{
	return suppressint;
}

void
err_arm(int v)
{
	err_armed = v;
}

jmp_buf *
err_jmp_ptr(void)
{
	return &err_jmp;
}

void
stack_probe(int *nleft, long *nxtoff, long *ssoff, int *depth)
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
get_stacknxt(void)
{
	return stacknxt;
}

char *
get_stackp(void)
{
	return (char *)stackp;
}

char *
get_sstrend(void)
{
	return sstrend;
}

int
get_stacknleft(void)
{
	return stacknleft;
}

size_t
sizeof_stackmark(void)
{
	return sizeof(struct stackmark);
}

size_t
sizeof_alias(void)
{
	return sizeof(struct alias);
}

int
alias_count(void)
{
	return aliases;
}

struct alias *
atab_get(int i)
{
	return atab[i];
}

struct output *
var_output(void)
{
	return &output;
}

struct output *
var_errout(void)
{
	return &errout;
}

struct output *
var_memout(void)
{
	return &memout;
}

struct output **
var_out1(void)
{
	return &out1;
}

struct output **
var_out2(void)
{
	return &out2;
}

} /* namespace pbsd::bin_sh::b0219 */
