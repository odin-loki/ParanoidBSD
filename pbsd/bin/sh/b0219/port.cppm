/*
 * PBSD batch b0219 -- C++23 module port of
 *	hbsd/src/bin/sh/alias.c
 *	hbsd/src/bin/sh/output.c
 *	hbsd/src/bin/sh/memalloc.c
 *	hbsd/src/bin/sh/show.c   (trace routines only; see skipped.txt)
 *
 * Behaviour is preserved exactly, including the signedness of `char`, the
 * evaluation order of the outc() macro and the pointer arithmetic of the
 * parser stack allocator.  Definitions that come from headers outside the
 * batch (shell.h, error.h, memalloc.h, output.h, mystring.h, alias.h,
 * parser.h) are reproduced in the support section.
 */

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

module;

#define _GNU_SOURCE 1

#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

/* shell.h */
#define SHELL_ALIGN 16
#define ALIGN(nbytes)	(((nbytes) + (SHELL_ALIGN - 1)) & ~(SHELL_ALIGN - 1))

/* error.h */
#define INTOFF		(suppressint++)
#define INTON		(--suppressint)
#define is_int_on()	(suppressint)

/* mystring.h */
#define equal(s1, s2)	(strcmp(s1, s2) == 0)

/* alias.h */
#define ALIASINUSE	1

/* output.h */
#define outc(c, file)	((file)->nextc == (file)->bufend ? \
			    (emptyoutbuf(file), *(file)->nextc++ = (c)) : \
			    (*(file)->nextc++ = (c)))
#define out1c(c)	outc(c, out1)
#define out2c(c)	outcslow(c, out2)

/* memalloc.h */
#define stackblock()		stacknxt
#define stackblocksize()	stacknleft
#define CHECKSTRSPACE(n, p)	{ if (sstrend - p < n) p = makestrspace(n, p); }

/* parser.h */
#define CTLESC		'\301'
#define CTLVAR		'\302'
#define CTLENDVAR	'\303'
#define CTLBACKQ	'\304'
#define CTLQUOTE	01

/* output.c */
#define OUTBUFSIZ BUFSIZ
#define MEM_OUT -2		/* output to dynamically allocated memory */
#define OUTPUT_ERR 01		/* error occurred on output */

/* memalloc.c */
#define MINSIZE 496		/* minimum size of a block. */
#define SPACE(sp)	((char*)(sp) + ALIGN(sizeof(struct stack_block)))

/* alias.c */
#define ATABSIZE 39

export module pbsd.bin.sh.b0219;

export namespace pbsd::bin_sh::b0219 {

/* ===================================================================== *
 * Support section: types and helpers supplied by headers outside the
 * batch, plus the glibc shim for the BSD fwopen().
 * ===================================================================== */

typedef void *pointer;

struct stack_block;

int suppressint = 0;
int intpending = 0;
jmp_buf sh_errjmp;
int sh_errjmp_set = 0;
int sh_err_thrown = 0;
char sh_err_msg[256];

int
int_pending(void)
{
	return (intpending);
}

[[noreturn]] void
error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(sh_err_msg, sizeof(sh_err_msg), fmt, ap);
	va_end(ap);
	sh_err_thrown++;
	if (sh_errjmp_set)
		longjmp(sh_errjmp, 1);
	fprintf(stderr, "port: unhandled error(): %s\n", sh_err_msg);
	fflush(stderr);
	_exit(97);
}

struct alias {
	struct alias *next;
	char *name;
	char *val;
	int flag;
};

struct output {
	char *nextc;
	char *bufend;
	char *buf;
	int bufsize;
	short fd;
	short flags;
};

struct stackmark {
	struct stack_block *stackp;
	char *stacknxt;
	int stacknleft;
};

#define __unused [[maybe_unused]]

char **port_argptr;
char *port_nextopt_optptr;
#define argptr port_argptr

static int
nextopt(const char *optstring)
{
	char *p;
	int c;

	if (port_nextopt_optptr == nullptr || *port_nextopt_optptr == '\0') {
		p = port_argptr != nullptr ? *port_argptr : nullptr;
		if (p == nullptr || *p != '-' || *++p == '\0' ||
		    (*p == '-' && *++p == '\0')) {
			return '\0';
		}
		port_argptr++;
		port_nextopt_optptr = p;
	}
	c = (unsigned char)*port_nextopt_optptr++;
	p = const_cast<char *>(strchr(optstring, c));
	if (p == nullptr)
		error("illegal option -%c", c);
	if (p[1] == ':') {
		if (*port_nextopt_optptr == '\0')
			port_nextopt_optptr = *port_argptr++;
		else {
			static char empty_opt[] = "";
			port_nextopt_optptr = empty_opt;
		}
	}
	return c;
}

static void
warning(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

struct fwopen_cookie {
	void *cookie;
	int (*writefn)(void *, const char *, int);
};

ssize_t
fwopen_write(void *c, const char *buf, size_t n)
{
	struct fwopen_cookie *fc = (struct fwopen_cookie *)c;

	return (fc->writefn(fc->cookie, buf, (int)n));
}

int
fwopen_close(void *c)
{
	free(c);
	return (0);
}

FILE *
fwopen(void *cookie, int (*writefn)(void *, const char *, int))
{
	cookie_io_functions_t io;
	struct fwopen_cookie *fc;
	FILE *fp;

	io.read = nullptr;
	io.write = fwopen_write;
	io.seek = nullptr;
	io.close = fwopen_close;
	fc = (struct fwopen_cookie *)malloc(sizeof(*fc));
	if (fc == nullptr)
		return (nullptr);
	fc->cookie = cookie;
	fc->writefn = writefn;
	fp = fopencookie(fc, "w", io);
	if (fp == nullptr) {
		free(fc);
		return (nullptr);
	}
	return (fp);
}

/* Forward declarations. */
pointer ckmalloc(size_t);
pointer ckrealloc(pointer, int);
void ckfree(pointer);
char *savestr(const char *);
pointer stalloc(int);
void stunalloc(pointer);
char *stsavestr(const char *);
void setstackmark(struct stackmark *);
void popstackmark(struct stackmark *);
char *growstackstr(void);
char *makestrspace(int, char *);
char *stputbin(const char *, size_t, char *);
char *stputs(const char *, char *);
void badalloc(const char *);
void stnewblock(int);
void growstackblock(int);
char *growstrstackblock(int, int);

void outcslow(int, struct output *);
void out1str(const char *);
void out1qstr(const char *);
void out2str(const char *);
void out2qstr(const char *);
void outstr(const char *, struct output *);
void outqstr(const char *, struct output *);
void outbin(const void *, size_t, struct output *);
void emptyoutbuf(struct output *);
void flushall(void);
void flushout(struct output *);
void freestdout(void);
int outiserror(struct output *);
void outclearerror(struct output *);
void outfmt(struct output *, const char *, ...);
void out1fmt(const char *, ...);
void out2fmt_flush(const char *, ...);
void fmtstr(char *, int, const char *, ...);
void doformat(struct output *, const char *, va_list);
FILE *out1fp(void);
int xwrite(int, const char *, int);
void byteseq(int, struct output *);
void outdqstr(const char *, struct output *);
int doformat_wr(void *, const char *, int);

struct alias *lookupalias(const char *, int);
const struct alias *iteralias(const struct alias *);
void setalias(const char *, const char *);
void freealias(struct alias *);
int unalias(const char *);
void rmaliases(void);
int comparealiases(const void *, const void *);
void printalias(const struct alias *);
void printaliases(void);
size_t hashalias(const char *);
int aliascmd(int, char **);
int unaliascmd(int, char **);

void trputc(int);
void sh_trace(const char *, ...);
void trputs(const char *);
void trstring(char *);
void trargs(char **);

/* ===================================================================== *
 * memalloc.c
 * ===================================================================== */

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

struct stack_block {
	struct stack_block *prev;
	/* Data follows */
};

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

/* ===================================================================== *
 * output.c
 * ===================================================================== */

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
			while (p >= end)
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
		if ((p[i] > '\0' && p[i] >= ' ' && p[i] != '\n') ||
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
emptyoutbuf(struct output *dest)
{
	int offset, newsize;

	if (dest->buf == NULL) {
		INTOFF;
		dest->buf = (char *)ckmalloc(dest->bufsize);
		dest->nextc = dest->buf;
		dest->bufend = dest->buf + dest->bufsize;
		INTON;
	} else if (dest->fd == MEM_OUT) {
		offset = dest->nextc - dest->buf;
		newsize = dest->bufsize << 1;
		INTOFF;
		dest->buf = (char *)ckrealloc(dest->buf, newsize);
		dest->bufsize = newsize;
		dest->bufend = dest->buf + newsize;
		dest->nextc = dest->buf + offset;
		INTON;
	} else {
		flushout(dest);
	}
}


void
flushall(void)
{
	flushout(&output);
	flushout(&errout);
}


void
flushout(struct output *dest)
{

	if (dest->buf == NULL || dest->nextc == dest->buf || dest->fd < 0)
		return;
	if (xwrite(dest->fd, dest->buf, dest->nextc - dest->buf) < 0)
		dest->flags |= OUTPUT_ERR;
	dest->nextc = dest->buf;
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
outfmt(struct output *file, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	doformat(file, fmt, ap);
	va_end(ap);
}


void
out1fmt(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	doformat(out1, fmt, ap);
	va_end(ap);
}

void
out2fmt_flush(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	doformat(out2, fmt, ap);
	va_end(ap);
	flushout(out2);
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

int
doformat_wr(void *cookie, const char *buf, int len)
{
	struct output *o;

	o = (struct output *)cookie;
	outbin(buf, len, o);

	return (len);
}

void
doformat(struct output *dest, const char *f, va_list ap)
{
	FILE *fp;

	if ((fp = fwopen(dest, doformat_wr)) != NULL) {
		vfprintf(fp, f, ap);
		fclose(fp);
	}
}

FILE *
out1fp(void)
{
	return fwopen(out1, doformat_wr);
}

/*
 * Version of write which resumes after a signal is caught.
 */

int
xwrite(int fd, const char *buf, int nbytes)
{
	int ntry;
	int i;
	int n;

	n = nbytes;
	ntry = 0;
	for (;;) {
		i = write(fd, buf, n);
		if (i > 0) {
			if ((n -= i) <= 0)
				return nbytes;
			buf += i;
			ntry = 0;
		} else if (i == 0) {
			if (++ntry > 10)
				return nbytes - n;
		} else if (errno != EINTR) {
			return -1;
		}
	}
}

/* ===================================================================== *
 * alias.c
 * ===================================================================== */

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

void
printalias(const struct alias *a)
{
	out1fmt("%s=", a->name);
	out1qstr(a->val);
	out1c('\n');
}

void
printaliases(void)
{
	int i, j;
	struct alias **sorted, *ap;

	INTOFF;
	sorted = (struct alias **)ckmalloc(aliases * sizeof(*sorted));
	j = 0;
	for (i = 0; i < ATABSIZE; i++)
		for (ap = atab[i]; ap; ap = ap->next)
			if (*ap->name != '\0')
				sorted[j++] = ap;
	qsort(sorted, aliases, sizeof(*sorted), comparealiases);
	for (i = 0; i < aliases; i++) {
		printalias(sorted[i]);
		if (int_pending())
			break;
	}
	ckfree(sorted);
	INTON;
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

int
aliascmd(int argc __unused, char **argv __unused)
{
	char *n, *v;
	int ret = 0;
	struct alias *ap;

	nextopt("");

	if (*argptr == NULL) {
		printaliases();
		return (0);
	}
	while ((n = *argptr++) != NULL) {
		if (n[0] == '\0') {
			warning("'': not found");
			ret = 1;
			continue;
		}
		if ((v = strchr(n+1, '=')) == NULL) /* n+1: funny ksh stuff */
			if ((ap = lookupalias(n, 0)) == NULL) {
				warning("%s: not found", n);
				ret = 1;
			} else
				printalias(ap);
		else {
			*v++ = '\0';
			setalias(n, v);
		}
	}

	return (ret);
}

int
unaliascmd(int argc __unused, char **argv __unused)
{
	int i;

	while ((i = nextopt("a")) != '\0') {
		if (i == 'a') {
			rmaliases();
			return (0);
		}
	}
	for (i = 0; *argptr; argptr++)
		i |= unalias(*argptr);

	return (i);
}

/* ===================================================================== *
 * show.c (the trace routines; see skipped.txt for the rest)
 * ===================================================================== */

FILE *tracefile;
int debug = 0;

void
trputc(int c)
{
	if (tracefile == NULL)
		return;
	putc(c, tracefile);
	if (c == '\n')
		fflush(tracefile);
}


void
sh_trace(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	if (tracefile != NULL) {
		(void) vfprintf(tracefile, fmt, va);
		if (strchr(fmt, '\n'))
			(void) fflush(tracefile);
	}
	va_end(va);
}


void
trputs(const char *s)
{
	if (tracefile == NULL)
		return;
	fputs(s, tracefile);
	if (strchr(s, '\n'))
		fflush(tracefile);
}


void
trstring(char *s)
{
	char *p;
	char c;

	if (tracefile == NULL)
		return;
	putc('"', tracefile);
	for (p = s ; *p ; p++) {
		switch (*p) {
		case '\n':  c = 'n';  goto backslash;
		case '\t':  c = 't';  goto backslash;
		case '\r':  c = 'r';  goto backslash;
		case '"':  c = '"';  goto backslash;
		case '\\':  c = '\\';  goto backslash;
		case CTLESC:  c = 'e';  goto backslash;
		case CTLVAR:  c = 'v';  goto backslash;
		case CTLVAR+CTLQUOTE:  c = 'V';  goto backslash;
		case CTLBACKQ:  c = 'q';  goto backslash;
		case CTLBACKQ+CTLQUOTE:  c = 'Q';  goto backslash;
backslash:	  putc('\\', tracefile);
			putc(c, tracefile);
			break;
		default:
			if (*p >= ' ' && *p <= '~')
				putc(*p, tracefile);
			else {
				putc('\\', tracefile);
				putc(*p >> 6 & 03, tracefile);
				putc(*p >> 3 & 07, tracefile);
				putc(*p & 07, tracefile);
			}
			break;
		}
	}
	putc('"', tracefile);
}


void
trargs(char **ap)
{
	if (tracefile == NULL)
		return;
	while (*ap) {
		trstring(*ap++);
		if (*ap)
			putc(' ', tracefile);
		else
			putc('\n', tracefile);
	}
	fflush(tracefile);
}

/* ===================================================================== *
 * Harness support and accessors that let the differential harness observe
 * internal state.
 * ===================================================================== */

static struct output *saved_out1;

void
port_reset_state(void)
{
	struct stack_block *sp;
	int i;

	while (stackp != nullptr) {
		sp = stackp;
		stackp = sp->prev;
		free(sp);
	}
	stackp = nullptr;
	stacknxt = nullptr;
	stacknleft = 0;
	sstrend = nullptr;
	suppressint = 0;
	intpending = 0;
	sh_err_thrown = 0;
	sh_errjmp_set = 0;

	if (output.buf != nullptr) {
		free(output.buf);
		output.buf = nullptr;
	}
	output.nextc = nullptr;
	output.bufend = nullptr;
	output.bufsize = OUTBUFSIZ;
	output.fd = 1;
	output.flags = 0;

	if (errout.buf != nullptr) {
		free(errout.buf);
		errout.buf = nullptr;
	}
	errout.nextc = nullptr;
	errout.bufend = nullptr;
	errout.bufsize = 256;
	errout.fd = 2;
	errout.flags = 0;

	if (memout.buf != nullptr) {
		free(memout.buf);
		memout.buf = nullptr;
	}
	memout.nextc = nullptr;
	memout.bufend = nullptr;
	memout.bufsize = 64;
	memout.fd = MEM_OUT;
	memout.flags = 0;

	out1 = &output;
	out2 = &errout;
	saved_out1 = nullptr;

	INTOFF;
	for (i = 0; i < ATABSIZE; i++) {
		struct alias *ap;

		while ((ap = atab[i]) != nullptr) {
			atab[i] = ap->next;
			freealias(ap);
		}
	}
	aliases = 0;
	INTON;

	port_nextopt_optptr = nullptr;
	port_argptr = nullptr;
	tracefile = nullptr;
	suppressint = 1;
}

void
port_set_out1_memout(void)
{
	if (memout.buf != nullptr) {
		free(memout.buf);
		memout.buf = nullptr;
	}
	memout.nextc = nullptr;
	memout.bufend = nullptr;
	memout.bufsize = 64;
	memout.fd = MEM_OUT;
	memout.flags = 0;
	saved_out1 = out1;
	out1 = &memout;
}

void
port_restore_out1(void)
{
	if (saved_out1 != nullptr)
		out1 = saved_out1;
	else
		out1 = &output;
	saved_out1 = nullptr;
	if (memout.buf != nullptr) {
		free(memout.buf);
		memout.buf = nullptr;
	}
	memout.nextc = nullptr;
	memout.bufend = nullptr;
	memout.bufsize = 64;
	memout.fd = MEM_OUT;
	memout.flags = 0;
}

struct stack_block *
get_stackp(void)
{
	return (stackp);
}

struct stack_block *
stack_prev(struct stack_block *sp)
{
	return (sp->prev);
}

char *
stack_space(struct stack_block *sp)
{
	return (SPACE(sp));
}

int
atabsize(void)
{
	return (ATABSIZE);
}

} /* namespace pbsd::bin_sh::b0219 */
