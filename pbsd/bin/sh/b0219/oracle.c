/*
 * oracle.c -- reference implementations for batch b0219.
 */

#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#include <sys/param.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif


#ifndef ALIGNBYTES
#define ALIGNBYTES (sizeof(long) - 1)
#endif
#ifndef ALIGN
#define ALIGN(p) (((unsigned long)(p) + ALIGNBYTES) & ~ALIGNBYTES)
#endif

#define out1c(c) outc((c), out1)
#define out2c(c) outcslow((c), out2)

struct oracle_fwopen_cookie {
	void *cookie;
	int (*writefn)(void *, const char *, int);
};

static ssize_t
oracle_fwcookie_write(void *c, const char *buf, size_t size)
{
	struct oracle_fwopen_cookie *fc = (struct oracle_fwopen_cookie *)c;
	int r = fc->writefn(fc->cookie, buf, (int)size);
	return (r < 0) ? -1 : (ssize_t)r;
}

static FILE *
oracle_fwopen(void *cookie, int (*writefn)(void *, const char *, int))
{
	static struct oracle_fwopen_cookie fc;
	static cookie_io_functions_t io = { NULL, oracle_fwcookie_write, NULL, NULL };

	fc.cookie = cookie;
	fc.writefn = writefn;
	return fopencookie(&fc, "w", io);
}

#define fwopen oracle_fwopen

#define __unused

typedef void *pointer;

#define equal(s1, s2) (strcmp((s1), (s2)) == 0)

#define ALIASINUSE 1

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

#define OUTBUFSIZ BUFSIZ
#define MEM_OUT -2
#define OUTPUT_ERR 01

struct stack_block {
	struct stack_block *prev;
};
#define SPACE(sp) ((char *)(sp) + ALIGN(sizeof(struct stack_block)))

struct stackmark {
	struct stack_block *stackp;
	char *stacknxt;
	int stacknleft;
};

#define stackblock() stacknxt
#define stackblocksize() stacknleft

#define CHECKSTRSPACE(n, p) \
	{ if ((size_t)(sstrend - p) < (size_t)(n)) p = makestrspace((n), (p)); }

#define INTOFF oracle_suppressint++
#define INTON do { if (--oracle_suppressint == 0 && oracle_intpending) oracle_onint(); } while (0)
#define is_int_on() oracle_suppressint
#define int_pending() oracle_intpending

static volatile sig_atomic_t oracle_suppressint = 1;
static volatile sig_atomic_t oracle_intpending = 0;

static void oracle_onint(void) {}

static int oracle_error_flag = 0;

void error(const char *fmt, ...)
{
	oracle_error_flag = 1;
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	abort();
}

void warning(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

char **oracle_argptr;
char *oracle_nextopt_optptr;
char *oracle_shoptarg;

int oracle_nextopt(const char *optstring)
{
	char *p;
	const char *q;
	char c;

	if ((p = oracle_nextopt_optptr) == NULL || *p == '\0') {
		p = *oracle_argptr;
		if (p == NULL || *p != '-' || *++p == '\0')
			return '\0';
		oracle_argptr++;
		if (p[0] == '-' && p[1] == '\0')
			return '\0';
	}
	c = *p++;
	for (q = optstring; *q != c;) {
		if (*q == '\0')
			error("Illegal option -%c", c);
		if (*++q == ':')
			q++;
	}
	if (*++q == ':') {
		if (*p == '\0' && (p = *oracle_argptr++) == NULL)
			error("No arg for -%c option", c);
		oracle_shoptarg = p;
		p = NULL;
	}
	if (p != NULL && *p != '\0')
		oracle_nextopt_optptr = p;
	else
		oracle_nextopt_optptr = NULL;
	return c;
}

#define nextopt oracle_nextopt
#define argptr oracle_argptr
#define shoptarg oracle_shoptarg
#define nextopt_optptr oracle_nextopt_optptr

struct output output = {NULL, NULL, NULL, OUTBUFSIZ, 1, 0};
struct output errout = {NULL, NULL, NULL, 256, 2, 0};
struct output memout = {NULL, NULL, NULL, 64, MEM_OUT, 0};
struct output *out1 = &output;
struct output *out2 = &errout;

char nullstr[1] = {0};

char *stacknxt;
int stacknleft;
char *sstrend;
static struct stack_block *stackp;


void ref_emptyoutbuf(struct output *dest);
void ref_outstr(const char *p, struct output *file);
void ref_outqstr(const char *p, struct output *file);
void ref_outbin(const void *data, size_t len, struct output *file);
void ref_outcslow(int c, struct output *file);
void ref_flushout(struct output *dest);
int ref_xwrite(int fd, const char *buf, int nbytes);
void ref_doformat(struct output *dest, const char *f, va_list ap);

#define outc(c, file) \
	((file)->nextc == (file)->bufend ? (emptyoutbuf(file), *(file)->nextc++ = (c)) : (*(file)->nextc++ = (c)))

void oracle_reset_state(void)
{
	oracle_suppressint = 1;
	oracle_intpending = 0;
	oracle_error_flag = 0;
	oracle_argptr = NULL;
	oracle_nextopt_optptr = NULL;
	oracle_shoptarg = NULL;
	out1 = &output;
	out2 = &errout;
	if (output.buf) { free(output.buf); output.buf = NULL; }
	output.nextc = NULL;
	output.bufend = NULL;
	output.flags = 0;
	if (errout.buf) { free(errout.buf); errout.buf = NULL; }
	errout.nextc = NULL;
	errout.bufend = NULL;
	errout.flags = 0;
	if (memout.buf) { free(memout.buf); memout.buf = NULL; }
	memout.nextc = NULL;
	memout.bufend = NULL;
	memout.bufsize = 64;
	memout.flags = 0;
	while (stackp) {
		struct stack_block *sp = stackp;
		stackp = sp->prev;
		free(sp);
	}
	stacknxt = NULL;
	stacknleft = 0;
	sstrend = NULL;
}

struct output *oracle_get_memout(void)
{
	return &memout;
}

void oracle_set_out1_memout(void)
{
	out1 = &memout;
}

void oracle_restore_out1(void)
{
	out1 = &output;
}


/* --- memalloc.c --- */
#define badalloc ref_badalloc
#define ckmalloc ref_ckmalloc
#define ckrealloc ref_ckrealloc
#define ckfree ref_ckfree
#define savestr ref_savestr
#define stnewblock ref_stnewblock
#define stalloc ref_stalloc
#define stunalloc ref_stunalloc
#define stsavestr ref_stsavestr
#define setstackmark ref_setstackmark
#define popstackmark ref_popstackmark
#define growstackblock ref_growstackblock
#define growstrstackblock ref_growstrstackblock
#define growstackstr ref_growstackstr
#define makestrspace ref_makestrspace
#define stputbin ref_stputbin
#define stputs ref_stputs
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


static void
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
	p = ckmalloc(len + 1);
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





static void
stnewblock(int nbytes)
{
	struct stack_block *sp;
	int allocsize;

	if (nbytes < MINSIZE)
		nbytes = MINSIZE;

	allocsize = ALIGN(sizeof(struct stack_block)) + ALIGN(nbytes);

	INTOFF;
	sp = ckmalloc(allocsize);
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
	stacknxt = p;
}


char *
stsavestr(const char *s)
{
	char *p;
	size_t len;

	len = strlen(s);
	p = stalloc(len + 1);
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

static void
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
		sp = ckrealloc((pointer)oldstackp, newlen);
		sp->prev = stackp;
		stackp = sp;
		stacknxt = SPACE(sp);
		stacknleft = newlen - (stacknxt - (char*)sp);
		sstrend = stacknxt + stacknleft;
		INTON;
	} else {
		newlen -= ALIGN(sizeof(struct stack_block));
		p = stalloc(newlen);
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

static char *
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


/* --- output.c --- */
#define doformat_wr ref_doformat_wr
#define byteseq ref_byteseq
#define outdqstr ref_outdqstr
#define outcslow ref_outcslow
#define out1str ref_out1str
#define out1qstr ref_out1qstr
#define out2str ref_out2str
#define out2qstr ref_out2qstr
#define outstr ref_outstr
#define outqstr ref_outqstr
#define outbin ref_outbin
#define emptyoutbuf ref_emptyoutbuf
#define flushall ref_flushall
#define flushout ref_flushout
#define freestdout ref_freestdout
#define outiserror ref_outiserror
#define outclearerror ref_outclearerror
#define outfmt ref_outfmt
#define out1fmt ref_out1fmt
#define out2fmt_flush ref_out2fmt_flush
#define fmtstr ref_fmtstr
#define doformat ref_doformat
#define out1fp ref_out1fp
#define xwrite ref_xwrite
#define badalloc ref_badalloc
#define ckmalloc ref_ckmalloc
#define ckrealloc ref_ckrealloc
#define ckfree ref_ckfree
#define savestr ref_savestr
#define stnewblock ref_stnewblock
#define stalloc ref_stalloc
#define stunalloc ref_stunalloc
#define stsavestr ref_stsavestr
#define setstackmark ref_setstackmark
#define popstackmark ref_popstackmark
#define growstackblock ref_growstackblock
#define growstrstackblock ref_growstrstackblock
#define growstackstr ref_growstackstr
#define makestrspace ref_makestrspace
#define stputbin ref_stputbin
#define stputs ref_stputs
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

static int doformat_wr(void *, const char *, int);


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

static void
byteseq(int ch, struct output *file)
{
	char seq[4];

	seq[0] = '\\';
	seq[1] = (ch >> 6 & 0x3) + '0';
	seq[2] = (ch >> 3 & 0x7) + '0';
	seq[3] = (ch & 0x7) + '0';
	outbin(seq, 4, file);
}

static void
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

	p = data;
	while (len-- > 0)
		outc(*p++, file);
}

void
emptyoutbuf(struct output *dest)
{
	int offset, newsize;

	if (dest->buf == NULL) {
		INTOFF;
		dest->buf = ckmalloc(dest->bufsize);
		dest->nextc = dest->buf;
		dest->bufend = dest->buf + dest->bufsize;
		INTON;
	} else if (dest->fd == MEM_OUT) {
		offset = dest->nextc - dest->buf;
		newsize = dest->bufsize << 1;
		INTOFF;
		dest->buf = ckrealloc(dest->buf, newsize);
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

static int
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


/* --- alias.c --- */
#define setalias ref_setalias
#define freealias ref_freealias
#define unalias ref_unalias
#define rmaliases ref_rmaliases
#define lookupalias ref_lookupalias
#define comparealiases ref_comparealiases
#define printalias ref_printalias
#define printaliases ref_printaliases
#define aliascmd ref_aliascmd
#define unaliascmd ref_unaliascmd
#define hashalias ref_hashalias
#define iteralias ref_iteralias
#define badalloc ref_badalloc
#define ckmalloc ref_ckmalloc
#define ckrealloc ref_ckrealloc
#define ckfree ref_ckfree
#define savestr ref_savestr
#define stnewblock ref_stnewblock
#define stalloc ref_stalloc
#define stunalloc ref_stunalloc
#define stsavestr ref_stsavestr
#define setstackmark ref_setstackmark
#define popstackmark ref_popstackmark
#define growstackblock ref_growstackblock
#define growstrstackblock ref_growstrstackblock
#define growstackstr ref_growstackstr
#define makestrspace ref_makestrspace
#define stputbin ref_stputbin
#define stputs ref_stputs
#define doformat_wr ref_doformat_wr
#define byteseq ref_byteseq
#define outdqstr ref_outdqstr
#define outcslow ref_outcslow
#define out1str ref_out1str
#define out1qstr ref_out1qstr
#define out2str ref_out2str
#define out2qstr ref_out2qstr
#define outstr ref_outstr
#define outqstr ref_outqstr
#define outbin ref_outbin
#define emptyoutbuf ref_emptyoutbuf
#define flushall ref_flushall
#define flushout ref_flushout
#define freestdout ref_freestdout
#define outiserror ref_outiserror
#define outclearerror ref_outclearerror
#define outfmt ref_outfmt
#define out1fmt ref_out1fmt
#define out2fmt_flush ref_out2fmt_flush
#define fmtstr ref_fmtstr
#define doformat ref_doformat
#define out1fp ref_out1fp
#define xwrite ref_xwrite
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

static void setalias(const char *, const char *);
static int unalias(const char *);
static size_t hashalias(const char *);

static
void
setalias(const char *name, const char *val)
{
	struct alias *ap, **app;

	unalias(name);
	app = &atab[hashalias(name)];
	INTOFF;
	ap = ckmalloc(sizeof (struct alias));
	ap->name = savestr(name);
	ap->val = savestr(val);
	ap->flag = 0;
	ap->next = *app;
	*app = ap;
	aliases++;
	INTON;
}

static void
freealias(struct alias *ap)
{
	ckfree(ap->name);
	ckfree(ap->val);
	ckfree(ap);
}

static int
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

static void
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

static int
comparealiases(const void *p1, const void *p2)
{
	const struct alias *const *a1 = p1;
	const struct alias *const *a2 = p2;

	return strcmp((*a1)->name, (*a2)->name);
}

static void
printalias(const struct alias *a)
{
	out1fmt("%s=", a->name);
	out1qstr(a->val);
	out1c('\n');
}

static void
printaliases(void)
{
	int i, j;
	struct alias **sorted, *ap;

	INTOFF;
	sorted = ckmalloc(aliases * sizeof(*sorted));
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

static size_t
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

