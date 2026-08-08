
/*
 * oracle.c -- reference implementations for batch b0230.
 */

#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
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
#include <sys/stat.h>
#include <sys/types.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef ALIGNBYTES
#define ALIGNBYTES (sizeof(long) - 1)
#endif
#ifndef ALIGN
#define ALIGN(p) (((unsigned long)(p) + ALIGNBYTES) & ~ALIGNBYTES)
#endif

#define __unused
#define NO_HISTORY 1
#define JOBS 1
#define DEBUG 1

typedef void *pointer;
typedef void (*sig_t)(int);

#define equal(s1, s2) (strcmp((s1), (s2)) == 0)

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

#define ALIASINUSE 1
#define VEXPORT 0x01
#define PEOF 0

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

#define STARTSTACKSTR(p) { p = stackblock(); STACKSTRNUL(p); }
#define STPUTC(c, p) do { CHECKSTRSPACE(1, p); *(p)++ = (c); } while (0)
#define STPUTS(s, p) (p = stputs((s), (p)))
#define STACKSTRNUL(p) do { CHECKSTRSPACE(1, p); *(p) = '\0'; } while (0)
#define STTOPC(p) ((p) > stackblock() ? (p)[-1] : '\0')
#define STUNPUTC(p) (--(p))

#define INTOFF oracle_suppressint++
#define INTON do { if (--oracle_suppressint == 0 && oracle_intpending) oracle_onint(); } while (0)
#define is_int_on() oracle_suppressint
#define int_pending() oracle_intpending
#define FORCEINTON do { oracle_suppressint = 0; } while (0)

#define TRACE(x)

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

void errorwithstatus(int status, const char *fmt, ...)
{
	(void)status;
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	abort();
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

#define outc(c, file) \
	((file)->nextc == (file)->bufend ? (emptyoutbuf(file), *(file)->nextc++ = (c)) : (*(file)->nextc++ = (c)))

void oracle_reset_state(void);
struct output *oracle_get_memout(void);
void oracle_set_out1_memout(void);
void oracle_restore_out1(void);

/* shell globals */
int Pflag = 0;
int iflag = 0;
int mflag = 0;
int debug = 0;
int rootshell = 1;
int verifyflag = 0;
int vflag = 0;
int whichprompt = 1;
volatile sig_atomic_t suppressint = 0;
int evalskip = 0;
int skipcount = 0;
int exitstatus = 0;
int oexitstatus = 0;

#define SET_PENDING_INT oracle_intpending = 1

struct jmploc {
	void *loc;
};
struct jmploc *handler = NULL;

int setjmp(struct jmploc *loc) { (void)loc; return 0; }

void onint(void) {}

void evalstring(char *s, int flags) { (void)s; (void)flags; }
void setjobctl(int on) { (void)on; }
void histsave(void) {}
void forcealias(void) {}

void hashcd(void) {}

struct varpair { char *name; char *val; int flags; };
static struct varpair oracle_vars[128];
static int oracle_var_n = 0;

char *lookupvar(const char *name)
{
	int i;
	for (i = 0; i < oracle_var_n; i++)
		if (strcmp(oracle_vars[i].name, name) == 0)
			return oracle_vars[i].val;
	return NULL;
}

void setvar(const char *name, const char *val, int flags)
{
	int i;
	for (i = 0; i < oracle_var_n; i++) {
		if (strcmp(oracle_vars[i].name, name) == 0) {
			if (oracle_vars[i].val)
				free(oracle_vars[i].val);
			oracle_vars[i].val = val ? strdup(val) : NULL;
			oracle_vars[i].flags = flags;
			return;
		}
	}
	if (oracle_var_n < 128) {
		oracle_vars[oracle_var_n].name = strdup(name);
		oracle_vars[oracle_var_n].val = val ? strdup(val) : NULL;
		oracle_vars[oracle_var_n].flags = flags;
		oracle_var_n++;
	}
}

char *bltinlookup(const char *name, int remove)
{
	char *v = lookupvar(name);
	(void)remove;
	return v;
}

char *padvance(char **path, const char *dot, const char *dest)
{
	char *p, *q, *r;
	static char padbuf[PATH_MAX];

	(void)dot;
	p = *path;
	if (p == NULL)
		return NULL;
	if (*p == '\0') {
		*path = NULL;
		return stsavestr(dest);
	}
	q = padbuf;
	while (*p != '\0' && *p != ':') {
		if (q < padbuf + PATH_MAX - 1)
			*q++ = *p;
		p++;
	}
	if (*p == ':')
		p++;
	*path = p;
	if (q == padbuf) {
		r = stsavestr(dest);
		return r;
	}
	if (q[-1] != '/')
		*q++ = '/';
	r = dest;
	while (*r != '\0' && q < padbuf + PATH_MAX - 1)
		*q++ = *r++;
	*q = '\0';
	return stsavestr(padbuf);
}

#ifndef NSIG
#define NSIG 64
#endif

static const char *oracle_sys_signame[NSIG];
#define sys_signame oracle_sys_signame
int sys_nsig = NSIG;

static int is_number(const char *p)
{
	if (*p == '\0')
		return 0;
	while (*p) {
		if (!isdigit((unsigned char)*p))
			return 0;
		p++;
	}
	return 1;
}

void oracle_reset_state(void)
{
	int i;
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
	for (i = 0; i < oracle_var_n; i++) {
		free(oracle_vars[i].name);
		free(oracle_vars[i].val);
	}
	oracle_var_n = 0;
	Pflag = 0;
	iflag = 0;
	mflag = 0;
	debug = 0;
	rootshell = 1;
	verifyflag = 0;
	vflag = 0;
	whichprompt = 1;
	suppressint = 0;
	evalskip = 0;
	skipcount = 0;
	exitstatus = 0;
	oexitstatus = 0;
}

struct output *oracle_get_memout(void) { return &memout; }
void oracle_set_out1_memout(void) { out1 = &memout; }
void oracle_restore_out1(void) { out1 = &output; }

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


/* --- mknodes.c --- */
#define savestr ref_mknodes_savestr
#define error ref_mknodes_error
#define main ref_mknodes_main
#define parsenode ref_parsenode
#define parsefield ref_parsefield
#define output ref_mknodes_output
#define outsizes ref_outsizes
#define outfunc ref_outfunc
#define indent ref_indent
#define nextfield ref_nextfield
#define skipbl ref_skipbl
#define readline ref_readline

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
 * This program reads the nodetypes file and nodes.c.pat file.  It generates
 * the files nodes.h and nodes.c.
 */


#define MAXTYPES 50		/* max number of node types */
#define MAXFIELDS 20		/* max fields in a structure */
#define BUFLEN 100		/* size of character buffers */

/* field types */
#define T_NODE 1		/* union node *field */
#define T_NODELIST 2		/* struct nodelist *field */
#define T_STRING 3
#define T_INT 4			/* int field */
#define T_OTHER 5		/* other */
#define T_TEMP 6		/* don't copy this field */


struct field {			/* a structure field */
	char *name;		/* name of field */
	int type;			/* type of field */
	char *decl;		/* declaration of field */
};


struct str {			/* struct representing a node structure */
	char *tag;		/* structure tag */
	int nfields;		/* number of fields in the structure */
	struct field field[MAXFIELDS];	/* the fields of the structure */
	int done;			/* set if fully parsed */
};


static int ntypes;			/* number of node types */
static char *nodename[MAXTYPES];	/* names of the nodes */
static struct str *nodestr[MAXTYPES];	/* type of structure used by the node */
static int nstr;			/* number of structures */
static struct str str[MAXTYPES];	/* the structures */
static struct str *curstr;		/* current structure */
static char line[1024];
static int linno;
static char *linep;

void ref_parsenode(void);
void ref_parsefield(void);
void ref_mknodes_output(char *);
void ref_outsizes(FILE *);
void ref_outfunc(FILE *, int);
void ref_indent(int, FILE *);
int ref_nextfield(char *);
void ref_skipbl(void);
int ref_readline(FILE *);
void ref_mknodes_error(const char *, ...) __printf0like(1, 2) __dead2;
char *ref_mknodes_savestr(const char *);


int
ref_mknodes_main(int argc, char *argv[])
{
	FILE *infp;

	if (argc != 3)
		ref_mknodes_error("usage: mknodes file");
	if ((infp = fopen(argv[1], "r")) == NULL)
		ref_mknodes_error("Can't open %s: %s", argv[1], strerror(errno));
	while (ref_readline(infp)) {
		if (line[0] == ' ' || line[0] == '\t')
			ref_parsefield();
		else if (line[0] != '\0')
			ref_parsenode();
	}
	fclose(infp);
	ref_mknodes_output(argv[2]);
	exit(0);
}



static void
ref_parsenode(void)
{
	char name[BUFLEN];
	char tag[BUFLEN];
	struct str *sp;

	if (curstr && curstr->nfields > 0)
		curstr->done = 1;
	ref_nextfield(name);
	if (! ref_nextfield(tag))
		ref_mknodes_error("Tag expected");
	if (*linep != '\0')
		ref_mknodes_error("Garbage at end of line");
	nodename[ntypes] = ref_mknodes_savestr(name);
	for (sp = str ; sp < str + nstr ; sp++) {
		if (strcmp(sp->tag, tag) == 0)
			break;
	}
	if (sp >= str + nstr) {
		sp->tag = ref_mknodes_savestr(tag);
		sp->nfields = 0;
		curstr = sp;
		nstr++;
	}
	nodestr[ntypes] = sp;
	ntypes++;
}


static void
ref_parsefield(void)
{
	char name[BUFLEN];
	char type[BUFLEN];
	char decl[2 * BUFLEN];
	struct field *fp;

	if (curstr == NULL || curstr->done)
		ref_mknodes_error("No current structure to add field to");
	if (! ref_nextfield(name))
		ref_mknodes_error("No field name");
	if (! ref_nextfield(type))
		ref_mknodes_error("No field type");
	fp = &curstr->field[curstr->nfields];
	fp->name = ref_mknodes_savestr(name);
	if (strcmp(type, "nodeptr") == 0) {
		fp->type = T_NODE;
		sprintf(decl, "union node *%s", name);
	} else if (strcmp(type, "nodelist") == 0) {
		fp->type = T_NODELIST;
		sprintf(decl, "struct nodelist *%s", name);
	} else if (strcmp(type, "string") == 0) {
		fp->type = T_STRING;
		sprintf(decl, "char *%s", name);
	} else if (strcmp(type, "int") == 0) {
		fp->type = T_INT;
		sprintf(decl, "int %s", name);
	} else if (strcmp(type, "other") == 0) {
		fp->type = T_OTHER;
	} else if (strcmp(type, "temp") == 0) {
		fp->type = T_TEMP;
	} else {
		ref_mknodes_error("Unknown type %s", type);
	}
	if (fp->type == T_OTHER || fp->type == T_TEMP) {
		ref_skipbl();
		fp->decl = ref_mknodes_savestr(linep);
	} else {
		if (*linep)
			ref_mknodes_error("Garbage at end of line");
		fp->decl = ref_mknodes_savestr(decl);
	}
	curstr->nfields++;
}


static const char writer[] = "\
/*\n\
 * This file was generated by the mknodes program.\n\
 */\n\
\n";

static void
ref_mknodes_output(char *file)
{
	FILE *hfile;
	FILE *cfile;
	FILE *patfile;
	int i;
	struct str *sp;
	struct field *fp;
	char *p;

	if ((patfile = fopen(file, "r")) == NULL)
		ref_mknodes_error("Can't open %s: %s", file, strerror(errno));
	if ((hfile = fopen("nodes.h", "w")) == NULL)
		ref_mknodes_error("Can't create nodes.h: %s", strerror(errno));
	if ((cfile = fopen("nodes.c", "w")) == NULL)
		ref_mknodes_error("Can't create nodes.c");
	fputs(writer, hfile);
	for (i = 0 ; i < ntypes ; i++)
		fprintf(hfile, "#define %s %d\n", nodename[i], i);
	fputs("\n\n\n", hfile);
	for (sp = str ; sp < &str[nstr] ; sp++) {
		fprintf(hfile, "struct %s {\n", sp->tag);
		for (i = sp->nfields, fp = sp->field ; --i >= 0 ; fp++) {
			fprintf(hfile, "      %s;\n", fp->decl);
		}
		fputs("};\n\n\n", hfile);
	}
	fputs("union node {\n", hfile);
	fprintf(hfile, "      int type;\n");
	for (sp = str ; sp < &str[nstr] ; sp++) {
		fprintf(hfile, "      struct %s %s;\n", sp->tag, sp->tag);
	}
	fputs("};\n\n\n", hfile);
	fputs("struct nodelist {\n", hfile);
	fputs("\tstruct nodelist *next;\n", hfile);
	fputs("\tunion node *n;\n", hfile);
	fputs("};\n\n\n", hfile);
	fputs("struct funcdef;\n", hfile);
	fputs("struct funcdef *copyfunc(union node *);\n", hfile);
	fputs("union node *getfuncnode(struct funcdef *);\n", hfile);
	fputs("void reffunc(struct funcdef *);\n", hfile);
	fputs("void unreffunc(struct funcdef *);\n", hfile);
	if (ferror(hfile))
		ref_mknodes_error("Can't write to nodes.h");
	if (fclose(hfile))
		ref_mknodes_error("Can't close nodes.h");

	fputs(writer, cfile);
	while (fgets(line, sizeof line, patfile) != NULL) {
		for (p = line ; *p == ' ' || *p == '\t' ; p++);
		if (strcmp(p, "%SIZES\n") == 0)
			ref_outsizes(cfile);
		else if (strcmp(p, "%CALCSIZE\n") == 0)
			ref_outfunc(cfile, 1);
		else if (strcmp(p, "%COPY\n") == 0)
			ref_outfunc(cfile, 0);
		else
			fputs(line, cfile);
	}
	fclose(patfile);
	if (ferror(cfile))
		ref_mknodes_error("Can't write to nodes.c");
	if (fclose(cfile))
		ref_mknodes_error("Can't close nodes.c");
}



static void
ref_outsizes(FILE *cfile)
{
	int i;

	fprintf(cfile, "static const short nodesize[%d] = {\n", ntypes);
	for (i = 0 ; i < ntypes ; i++) {
		fprintf(cfile, "      ALIGN(sizeof (struct %s)),\n", nodestr[i]->tag);
	}
	fprintf(cfile, "};\n");
}


static void
ref_outfunc(FILE *cfile, int calcsize)
{
	struct str *sp;
	struct field *fp;
	int i;

	fputs("      if (n == NULL)\n", cfile);
	if (calcsize)
		fputs("	    return;\n", cfile);
	else
		fputs("	    return NULL;\n", cfile);
	if (calcsize)
		fputs("      result->blocksize += nodesize[n->type];\n", cfile);
	else {
		fputs("      new = state->block;\n", cfile);
		fputs("      state->block = (char *)state->block + nodesize[n->type];\n", cfile);
	}
	fputs("      switch (n->type) {\n", cfile);
	for (sp = str ; sp < &str[nstr] ; sp++) {
		for (i = 0 ; i < ntypes ; i++) {
			if (nodestr[i] == sp)
				fprintf(cfile, "      case %s:\n", nodename[i]);
		}
		for (i = sp->nfields ; --i >= 1 ; ) {
			fp = &sp->field[i];
			switch (fp->type) {
			case T_NODE:
				if (calcsize) {
					ref_indent(12, cfile);
					fprintf(cfile, "calcsize(n->%s.%s, result);\n",
						sp->tag, fp->name);
				} else {
					ref_indent(12, cfile);
					fprintf(cfile, "new->%s.%s = copynode(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_NODELIST:
				if (calcsize) {
					ref_indent(12, cfile);
					fprintf(cfile, "sizenodelist(n->%s.%s, result);\n",
						sp->tag, fp->name);
				} else {
					ref_indent(12, cfile);
					fprintf(cfile, "new->%s.%s = copynodelist(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_STRING:
				if (calcsize) {
					ref_indent(12, cfile);
					fprintf(cfile, "result->stringsize += strlen(n->%s.%s) + 1;\n",
						sp->tag, fp->name);
				} else {
					ref_indent(12, cfile);
					fprintf(cfile, "new->%s.%s = nodesavestr(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_INT:
			case T_OTHER:
				if (! calcsize) {
					ref_indent(12, cfile);
					fprintf(cfile, "new->%s.%s = n->%s.%s;\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			}
		}
		ref_indent(12, cfile);
		fputs("break;\n", cfile);
	}
	fputs("      };\n", cfile);
	if (! calcsize)
		fputs("      new->type = n->type;\n", cfile);
}


static void
ref_indent(int amount, FILE *fp)
{
	while (amount >= 8) {
		putc('\t', fp);
		amount -= 8;
	}
	while (--amount >= 0) {
		putc(' ', fp);
	}
}


static int
ref_nextfield(char *buf)
{
	char *p, *q;

	p = linep;
	while (*p == ' ' || *p == '\t')
		p++;
	q = buf;
	while (*p != ' ' && *p != '\t' && *p != '\0')
		*q++ = *p++;
	*q = '\0';
	linep = p;
	return (q > buf);
}


static void
ref_skipbl(void)
{
	while (*linep == ' ' || *linep == '\t')
		linep++;
}


static int
ref_readline(FILE *infp)
{
	char *p;

	if (fgets(line, 1024, infp) == NULL)
		return 0;
	for (p = line ; *p != '#' && *p != '\n' && *p != '\0' ; p++);
	while (p > line && (p[-1] == ' ' || p[-1] == '\t'))
		p--;
	*p = '\0';
	linep = line;
	linno++;
	if (p - line > BUFLEN)
		ref_mknodes_error("Line too long");
	return 1;
}



static void
ref_mknodes_error(const char *msg, ...)
{
	va_list va;
	va_start(va, msg);

	(void) fprintf(stderr, "line %d: ", linno);
	(void) vfprintf(stderr, msg, va);
	(void) fputc('\n', stderr);

	va_end(va);

	exit(2);
}



static char *
ref_mknodes_savestr(const char *s)
{
	char *p;

	if ((p = malloc(strlen(s) + 1)) == NULL)
		ref_mknodes_error("Out of space");
	(void) strcpy(p, s);
	return p;
}
/* --- cd.c --- */
#define cdcmd ref_cdcmd
#define docd ref_docd
#define cdlogical ref_cdlogical
#define cdphysical ref_cdphysical
#define getcomponent ref_getcomponent
#define findcwd ref_findcwd
#define updatepwd ref_updatepwd
#define pwdcmd ref_pwdcmd
#define getpwd ref_getpwd
#define getpwd2 ref_getpwd2
#define pwd_init ref_pwd_init

/*-
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
 * The cd and pwd commands.
 */


int ref_cdlogical(char *);
int ref_cdphysical(char *);
int ref_docd(char *, int, int);
char *ref_getcomponent(char **);
char *ref_findcwd(char *);
void ref_updatepwd(char *);
char *ref_getpwd(void);
char *ref_getpwd2(void);

static char *curdir = NULL;	/* current working directory */

int
ref_cdcmd(int argc __unused, char **argv __unused)
{
	const char *dest;
	const char *path;
	char *p;
	struct stat statb;
	int ch, phys, print = 0, getcwderr = 0;
	int rc;
	int errno1 = ENOENT;

	phys = Pflag;
	while ((ch = nextopt("eLP")) != '\0') {
		switch (ch) {
		case 'e':
			getcwderr = 1;
			break;
		case 'L':
			phys = 0;
			break;
		case 'P':
			phys = 1;
			break;
		}
	}

	if (*argptr != NULL && argptr[1] != NULL)
		error("too many arguments");

	if ((dest = *argptr) == NULL && (dest = bltinlookup("HOME", 1)) == NULL)
		error("HOME not set");
	if (dest[0] == '-' && dest[1] == '\0') {
		dest = bltinlookup("OLDPWD", 1);
		if (dest == NULL)
			error("OLDPWD not set");
		print = 1;
	}
	if (dest[0] == '/' ||
	    (dest[0] == '.' && (dest[1] == '/' || dest[1] == '\0')) ||
	    (dest[0] == '.' && dest[1] == '.' && (dest[2] == '/' || dest[2] == '\0')) ||
	    (path = bltinlookup("CDPATH", 1)) == NULL)
		path = "";
	while ((p = padvance(&path, NULL, dest)) != NULL) {
		if (stat(p, &statb) < 0) {
			if (errno != ENOENT)
				errno1 = errno;
		} else if (!S_ISDIR(statb.st_mode))
			errno1 = ENOTDIR;
		else {
			if (!print) {
				/*
				 * XXX - rethink
				 */
				if (p[0] == '.' && p[1] == '/' && p[2] != '\0')
					print = strcmp(p + 2, dest);
				else
					print = strcmp(p, dest);
			}
			rc = ref_docd(p, print, phys);
			if (rc >= 0)
				return getcwderr ? rc : 0;
			if (errno != ENOENT)
				errno1 = errno;
		}
	}
	error("%s: %s", dest, strerror(errno1));
	/*NOTREACHED*/
	return 0;
}


/*
 * Actually change the directory.  In an interactive shell, print the
 * directory name if "print" is nonzero.
 */
static int
ref_docd(char *dest, int print, int phys)
{
	int rc;

	TRACE(("ref_docd(\"%s\", %d, %d) called\n", dest, print, phys));

	/* If logical cd fails, fall back to physical. */
	if ((phys || (rc = ref_cdlogical(dest)) < 0) && (rc = ref_cdphysical(dest)) < 0)
		return (-1);

	if (print && iflag && curdir) {
		out1fmt("%s\n", curdir);
		/*
		 * Ignore write errors to preserve the invariant that the
		 * current directory is changed iff the exit status is 0
		 * (or 1 if -e was given and the full pathname could not be
		 * determined).
		 */
		flushout(out1);
		outclearerror(out1);
	}

	return (rc);
}

static int
ref_cdlogical(char *dest)
{
	char *p;
	char *q;
	char *component;
	char *path;
	struct stat statb;
	int first;
	int badstat;

	/*
	 *  Check each component of the path. If we find a symlink or
	 *  something we can't stat, clear curdir to force a getcwd()
	 *  next time we get the value of the current directory.
	 */
	badstat = 0;
	path = stsavestr(dest);
	STARTSTACKSTR(p);
	if (*dest == '/') {
		STPUTC('/', p);
		path++;
	}
	first = 1;
	while ((q = ref_getcomponent(&path)) != NULL) {
		if (q[0] == '\0' || (q[0] == '.' && q[1] == '\0'))
			continue;
		if (! first)
			STPUTC('/', p);
		first = 0;
		component = q;
		STPUTS(q, p);
		if (equal(component, ".."))
			continue;
		STACKSTRNUL(p);
		if (lstat(stackblock(), &statb) < 0) {
			badstat = 1;
			break;
		}
	}

	INTOFF;
	if ((p = ref_findcwd(badstat ? NULL : dest)) == NULL || chdir(p) < 0) {
		INTON;
		return (-1);
	}
	ref_updatepwd(p);
	INTON;
	return (0);
}

static int
ref_cdphysical(char *dest)
{
	char *p;
	int rc = 0;

	INTOFF;
	if (chdir(dest) < 0) {
		INTON;
		return (-1);
	}
	p = ref_findcwd(NULL);
	if (p == NULL) {
		warning("warning: failed to get name of current directory");
		rc = 1;
	}
	ref_updatepwd(p);
	INTON;
	return (rc);
}

/*
 * Get the next component of the path name pointed to by *path.
 * This routine overwrites *path and the string pointed to by it.
 */
static char *
ref_getcomponent(char **path)
{
	char *p;
	char *start;

	if ((p = *path) == NULL)
		return NULL;
	start = *path;
	while (*p != '/' && *p != '\0')
		p++;
	if (*p == '\0') {
		*path = NULL;
	} else {
		*p++ = '\0';
		*path = p;
	}
	return start;
}


static char *
ref_findcwd(char *dir)
{
	char *new;
	char *p;
	char *path;

	/*
	 * If our argument is NULL, we don't know the current directory
	 * any more because we traversed a symbolic link or something
	 * we couldn't stat().
	 */
	if (dir == NULL || curdir == NULL)
		return ref_getpwd2();
	path = stsavestr(dir);
	STARTSTACKSTR(new);
	if (*dir != '/') {
		STPUTS(curdir, new);
		if (STTOPC(new) == '/')
			STUNPUTC(new);
	}
	while ((p = ref_getcomponent(&path)) != NULL) {
		if (equal(p, "..")) {
			while (new > stackblock() && (STUNPUTC(new), *new) != '/');
		} else if (*p != '\0' && ! equal(p, ".")) {
			STPUTC('/', new);
			STPUTS(p, new);
		}
	}
	if (new == stackblock())
		STPUTC('/', new);
	STACKSTRNUL(new);
	return stackblock();
}

/*
 * Update curdir (the name of the current directory) in response to a
 * cd command.  We also call hashcd to let the routines in exec.c know
 * that the current directory has changed.
 */
static void
ref_updatepwd(char *dir)
{
	char *prevdir;

	hashcd();				/* update command hash table */

	setvar("PWD", dir, VEXPORT);
	setvar("OLDPWD", curdir, VEXPORT);
	prevdir = curdir;
	curdir = dir ? savestr(dir) : NULL;
	ckfree(prevdir);
}

int
ref_pwdcmd(int argc __unused, char **argv __unused)
{
	char *p;
	int ch, phys;

	phys = Pflag;
	while ((ch = nextopt("LP")) != '\0') {
		switch (ch) {
		case 'L':
			phys = 0;
			break;
		case 'P':
			phys = 1;
			break;
		}
	}

	if (*argptr != NULL)
		error("too many arguments");

	if (!phys && ref_getpwd()) {
		out1str(curdir);
		out1c('\n');
	} else {
		if ((p = ref_getpwd2()) == NULL)
			error(".: %s", strerror(errno));
		out1str(p);
		out1c('\n');
	}

	return 0;
}

/*
 * Get the current directory and cache the result in curdir.
 */
static char *
ref_getpwd(void)
{
	char *p;

	if (curdir)
		return curdir;

	p = ref_getpwd2();
	if (p != NULL) {
		INTOFF;
		curdir = savestr(p);
		INTON;
	}

	return curdir;
}

#define MAXPWD 256

/*
 * Return the current directory.
 */
static char *
ref_getpwd2(void)
{
	char *pwd;
	int i;

	for (i = MAXPWD;; i *= 2) {
		pwd = stalloc(i);
		if (getcwd(pwd, i) != NULL)
			return pwd;
		stunalloc(pwd);
		if (errno != ERANGE)
			break;
	}

	return NULL;
}

/*
 * Initialize PWD in a new shell.
 * If the shell is interactive, we need to warn if this fails.
 */
void
ref_pwd_init(int warn)
{
	char *pwd;
	struct stat stdot, stpwd;

	pwd = lookupvar("PWD");
	if (pwd && *pwd == '/' && stat(".", &stdot) != -1 &&
	    stat(pwd, &stpwd) != -1 &&
	    stdot.st_dev == stpwd.st_dev &&
	    stdot.st_ino == stpwd.st_ino) {
		if (curdir)
			ckfree(curdir);
		curdir = savestr(pwd);
	}
	if (ref_getpwd() == NULL && warn)
		out2fmt_flush("sh: cannot determine working directory\n");
	setvar("PWD", curdir, VEXPORT);
}
/* --- trap.c --- */
#define sigstring_to_signum ref_sigstring_to_signum
#define printsignals ref_printsignals
#define trapcmd ref_trapcmd
#define clear_traps ref_clear_traps
#define have_traps ref_have_traps
#define setsignal ref_setsignal
#define getsigaction ref_getsigaction
#define ignoresig ref_ignoresig
#define issigchldtrapped ref_issigchldtrapped
#define onsig ref_onsig
#define dotrap ref_dotrap
#define trap_init ref_trap_init
#define setinteractive ref_setinteractive
#define exitshell ref_exitshell
#define exitshell_savedstatus ref_exitshell_savedstatus

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


#ifndef NO_HISTORY
#endif


/*
 * Sigmode records the current value of the signal handlers for the various
 * modes.  A value of zero means that the current handler is not known.
 * S_HARD_IGN indicates that the signal was ignored on entry to the shell,
 */

#define S_DFL 1			/* default signal handling (SIG_DFL) */
#define S_CATCH 2		/* signal is caught */
#define S_IGN 3			/* signal is ignored (SIG_IGN) */
#define S_HARD_IGN 4		/* signal is ignored permanently */
#define S_RESET 5		/* temporary - to reset a hard ignored sig */


static char sigmode[NSIG];	/* current value of signal */
volatile sig_atomic_t pendingsig;	/* indicates some signal received */
volatile sig_atomic_t pendingsig_waitcmd;	/* indicates wait builtin should be interrupted */
static int in_dotrap;			/* do we execute in a trap handler? */
static char *volatile trap[NSIG];	/* trap handler commands */
static volatile sig_atomic_t gotsig[NSIG];
				/* indicates specified signal received */
static int ignore_sigchld;	/* Used while handling SIGCHLD traps. */
static int last_trapsig;

int exiting;		/* ref_exitshell() has been called */
int exiting_exitstatus;	/* value passed to ref_exitshell() */

int ref_getsigaction(int, sig_t *);


/*
 * Map a string to a signal number.
 *
 * Note: the signal number may exceed NSIG.
 */
static int
ref_sigstring_to_signum(char *sig)
{

	if (is_number(sig)) {
		int signo;

		signo = atoi(sig);
		return ((signo >= 0 && signo < NSIG) ? signo : (-1));
	} else if (strcasecmp(sig, "EXIT") == 0) {
		return (0);
	} else {
		int n;

		if (strncasecmp(sig, "SIG", 3) == 0)
			sig += 3;
		for (n = 1; n < sys_nsig; n++)
			if (sys_signame[n] &&
			    strcasecmp(sys_signame[n], sig) == 0)
				return (n);
	}
	return (-1);
}


/*
 * Print a list of valid signal names.
 */
static void
ref_printsignals(void)
{
	int n, outlen;

	outlen = 0;
	for (n = 1; n < sys_nsig; n++) {
		if (sys_signame[n]) {
			out1fmt("%s", sys_signame[n]);
			outlen += strlen(sys_signame[n]);
		} else {
			out1fmt("%d", n);
			outlen += 3;	/* good enough */
		}
		++outlen;
		if (outlen > 71 || n == sys_nsig - 1) {
			out1str("\n");
			outlen = 0;
		} else {
			out1c(' ');
		}
	}
}


/*
 * The trap builtin.
 */
int
ref_trapcmd(int argc __unused, char **argv)
{
	char *action;
	int signo;
	int errors = 0;
	int i;

	while ((i = nextopt("l")) != '\0') {
		switch (i) {
		case 'l':
			ref_printsignals();
			return (0);
		}
	}
	argv = argptr;

	if (*argv == NULL) {
		for (signo = 0 ; signo < sys_nsig ; signo++) {
			if (signo < NSIG && trap[signo] != NULL) {
				out1str("trap -- ");
				out1qstr(trap[signo]);
				if (signo == 0) {
					out1str(" EXIT\n");
				} else if (sys_signame[signo]) {
					out1fmt(" %s\n", sys_signame[signo]);
				} else {
					out1fmt(" %d\n", signo);
				}
			}
		}
		return 0;
	}
	action = NULL;
	if (*argv && !is_number(*argv)) {
		if (strcmp(*argv, "-") == 0)
			argv++;
		else {
			action = *argv;
			argv++;
		}
	}
	for (; *argv; argv++) {
		if ((signo = ref_sigstring_to_signum(*argv)) == -1) {
			warning("bad signal %s", *argv);
			errors = 1;
			continue;
		}
		INTOFF;
		if (action)
			action = savestr(action);
		if (trap[signo])
			ckfree(trap[signo]);
		trap[signo] = action;
		if (signo != 0)
			ref_setsignal(signo);
		INTON;
	}
	return errors;
}


/*
 * Clear traps on a fork.
 */
void
ref_clear_traps(void)
{
	char *volatile *tp;

	for (tp = trap ; tp <= &trap[NSIG - 1] ; tp++) {
		if (*tp && **tp) {	/* trap not NULL or SIG_IGN */
			INTOFF;
			ckfree(*tp);
			*tp = NULL;
			if (tp != &trap[0])
				ref_setsignal(tp - trap);
			INTON;
		}
	}
}


/*
 * Check if we have any traps enabled.
 */
int
ref_have_traps(void)
{
	char *volatile *tp;

	for (tp = trap ; tp <= &trap[NSIG - 1] ; tp++) {
		if (*tp && **tp)	/* trap not NULL or SIG_IGN */
			return 1;
	}
	return 0;
}

/*
 * Set the signal handler for the specified signal.  The routine figures
 * out what it should be set to.
 */
void
ref_setsignal(int signo)
{
	int action;
	sig_t sigact = SIG_DFL;
	struct sigaction sa;
	char *t;

	if ((t = trap[signo]) == NULL)
		action = S_DFL;
	else if (*t != '\0')
		action = S_CATCH;
	else
		action = S_IGN;
	if (action == S_DFL) {
		switch (signo) {
		case SIGINT:
			action = S_CATCH;
			break;
		case SIGQUIT:
#ifdef DEBUG
			if (debug)
				break;
#endif
			action = S_CATCH;
			break;
		case SIGTERM:
			if (rootshell && iflag)
				action = S_IGN;
			break;
#if JOBS
		case SIGTSTP:
		case SIGTTOU:
			if (rootshell && mflag)
				action = S_IGN;
			break;
#endif
		}
	}

	t = &sigmode[signo];
	if (*t == 0) {
		/*
		 * current setting unknown
		 */
		if (!ref_getsigaction(signo, &sigact)) {
			/*
			 * Pretend it worked; maybe we should give a warning
			 * here, but other shells don't. We don't alter
			 * sigmode, so that we retry every time.
			 */
			return;
		}
		if (sigact == SIG_IGN) {
			if (mflag && (signo == SIGTSTP ||
			     signo == SIGTTIN || signo == SIGTTOU)) {
				*t = S_IGN;	/* don't hard ignore these */
			} else
				*t = S_HARD_IGN;
		} else {
			*t = S_RESET;	/* force to be set */
		}
	}
	if (*t == S_HARD_IGN || *t == action)
		return;
	switch (action) {
		case S_DFL:	sigact = SIG_DFL;	break;
		case S_CATCH:  	sigact = ref_onsig;		break;
		case S_IGN:	sigact = SIG_IGN;	break;
	}
	*t = action;
	sa.sa_handler = sigact;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(signo, &sa, NULL);
}


/*
 * Return the current setting for sig w/o changing it.
 */
static int
ref_getsigaction(int signo, sig_t *sigact)
{
	struct sigaction sa;

	if (sigaction(signo, (struct sigaction *)0, &sa) == -1)
		return 0;
	*sigact = (sig_t) sa.sa_handler;
	return 1;
}


/*
 * Ignore a signal.
 */
void
ref_ignoresig(int signo)
{

	if (sigmode[signo] == 0)
		ref_setsignal(signo);
	if (sigmode[signo] != S_IGN && sigmode[signo] != S_HARD_IGN) {
		signal(signo, SIG_IGN);
		sigmode[signo] = S_IGN;
	}
}


int
ref_issigchldtrapped(void)
{

	return (trap[SIGCHLD] != NULL && *trap[SIGCHLD] != '\0');
}


/*
 * Signal handler.
 */
void
ref_onsig(int signo)
{

	if (signo == SIGINT && trap[SIGINT] == NULL) {
		if (suppressint)
			SET_PENDING_INT;
		else
			onint();
		return;
	}

	/* If we are currently in a wait builtin, prepare to break it */
	if (signo == SIGINT || signo == SIGQUIT)
		pendingsig_waitcmd = signo;

	if (trap[signo] != NULL && trap[signo][0] != '\0' &&
	    (signo != SIGCHLD || !ignore_sigchld)) {
		gotsig[signo] = 1;
		pendingsig = signo;
		pendingsig_waitcmd = signo;
	}
}


/*
 * Called to execute a trap.  Perhaps we should avoid entering new trap
 * handlers while we are executing a trap handler.
 */
void
ref_dotrap(void)
{
	struct stackmark smark;
	int i;
	int savestatus, prev_evalskip, prev_skipcount;

	in_dotrap++;
	for (;;) {
		pendingsig = 0;
		pendingsig_waitcmd = 0;
		for (i = 1; i < NSIG; i++) {
			if (gotsig[i]) {
				gotsig[i] = 0;
				if (trap[i]) {
					/*
					 * Ignore SIGCHLD to avoid infinite
					 * recursion if the trap action does
					 * a fork.
					 */
					if (i == SIGCHLD)
						ignore_sigchld++;

					/*
					 * Backup current evalskip
					 * state and reset it before
					 * executing a trap, so that the
					 * trap is not disturbed by an
					 * ongoing break/continue/return
					 * statement.
					 */
					prev_evalskip  = evalskip;
					prev_skipcount = skipcount;
					evalskip = 0;

					last_trapsig = i;
					savestatus = exitstatus;
					setstackmark(&smark);
					evalstring(stsavestr(trap[i]), 0);
					popstackmark(&smark);

					/*
					 * If such a command was not
					 * already in progress, allow a
					 * break/continue/return in the
					 * trap action to have an effect
					 * outside of it.
					 */
					if (evalskip == 0 ||
					    prev_evalskip != 0) {
						evalskip  = prev_evalskip;
						skipcount = prev_skipcount;
						exitstatus = savestatus;
					}

					if (i == SIGCHLD)
						ignore_sigchld--;
				}
				break;
			}
		}
		if (i >= NSIG)
			break;
	}
	in_dotrap--;
}


void
ref_trap_init(void)
{
	ref_setsignal(SIGINT);
	ref_setsignal(SIGQUIT);
}


/*
 * Controls whether the shell is interactive or not based on iflag.
 */
void
ref_setinteractive(void)
{
	ref_setsignal(SIGTERM);
}


/*
 * Called to exit the shell.
 */
void
ref_exitshell(int status)
{
	TRACE(("ref_exitshell(%d) pid=%d\n", status, getpid()));
	exiting = 1;
	exiting_exitstatus = status;
	ref_exitshell_savedstatus();
}

void
ref_exitshell_savedstatus(void)
{
	struct jmploc loc1, loc2;
	char *p;
	int sig = 0;
	sigset_t sigs;

	if (!exiting) {
		if (in_dotrap && last_trapsig) {
			sig = last_trapsig;
			exiting_exitstatus = sig + 128;
		} else
			exiting_exitstatus = oexitstatus;
	}
	exitstatus = oexitstatus = exiting_exitstatus;
	if (!setjmp(loc1.loc)) {
		handler = &loc1;
		if ((p = trap[0]) != NULL && *p != '\0') {
			/*
			 * Reset evalskip, or the trap on EXIT could be
			 * interrupted if the last command was a "return".
			 */
			evalskip = 0;
			trap[0] = NULL;
			FORCEINTON;
			evalstring(p, 0);
		}
	}
	if (!setjmp(loc2.loc)) {
		handler = &loc2;		/* probably unnecessary */
		FORCEINTON;
		flushall();
#if JOBS
		setjobctl(0);
#endif
#ifndef NO_HISTORY
		histsave();
#endif
	}
	if (sig != 0 && sig != SIGSTOP && sig != SIGTSTP && sig != SIGTTIN &&
	    sig != SIGTTOU) {
		signal(sig, SIG_DFL);
		sigemptyset(&sigs);
		sigaddset(&sigs, sig);
		sigprocmask(SIG_UNBLOCK, &sigs, NULL);
		kill(getpid(), sig);
		/* If the default action is to ignore, fall back to _exit(). */
	}
	_exit(exiting_exitstatus);
}
/* --- input.c --- */
#define pgetc_macro ref_preadbuffer
#define resetinput ref_resetinput
#define pgetc ref_pgetc
#define preadfd ref_preadfd
#define preadbuffer ref_preadbuffer
#define preadateof ref_preadateof
#define pungetc ref_pungetc
#define pushstring ref_pushstring
#define popstring ref_popstring
#define setinputfile ref_setinputfile
#define setinputfd ref_setinputfd
#define setinputstring ref_setinputstring
#define pushfile ref_pushfile
#define popfile ref_popfile
#define getcurrentfile ref_getcurrentfile
#define popfilesupto ref_popfilesupto
#define popallfiles ref_popallfiles
#define closescript ref_closescript

/*-
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
 * This file implements the input routines used by the parser.
 */

#ifndef NO_HISTORY
#endif

#define EOF_NLEFT -99		/* value of parsenleft when EOF pushed back */

struct strpush {
	struct strpush *prev;	/* preceding string on stack */
	const char *prevstring;
	int prevnleft;
	int prevlleft;
	struct alias *ap;	/* if push was associated with an alias */
};

/*
 * The parsefile structure pointed to by the global variable parsefile
 * contains information about the current file being read.
 */

struct parsefile {
	struct parsefile *prev;	/* preceding file on stack */
	int linno;		/* current line */
	int fd;			/* file descriptor (or -1 if string) */
	int nleft;		/* number of chars left in this line */
	int lleft;		/* number of lines left in this buffer */
	const char *nextc;	/* next char in buffer */
	char *buf;		/* input buffer */
	struct strpush *strpush; /* for pushing strings at this level */
	struct strpush basestrpush; /* so pushing one is fast */
};


int plinno = 1;			/* input line number */
int parsenleft;			/* copy of parsefile->nleft */
static int parselleft;		/* copy of parsefile->lleft */
const char *parsenextc;		/* copy of parsefile->nextc */
static char basebuf[BUFSIZ + 1];/* buffer for top level input file */
static struct parsefile basepf = {	/* top level input file */
	.nextc = basebuf,
	.buf = basebuf
};
static struct parsefile *parsefile = &basepf;	/* current input file */
int whichprompt;		/* 1 == PS1, 2 == PS2 */

void ref_pushfile(void);
int ref_preadfd(void);
void ref_popstring(void);

void
ref_resetinput(void)
{
	ref_popallfiles();
	parselleft = parsenleft = 0;	/* clear input buffer */
}



/*
 * Read a character from the script, returning PEOF on end of file.
 * Nul characters in the input are silently discarded.
 */

int
ref_pgetc(void)
{
	return ref_preadbuffer();
}


static int
ref_preadfd(void)
{
	int nr;
	parsenextc = parsefile->buf;

retry:
#ifndef NO_HISTORY
	if (parsefile->fd == 0 && el) {
		static const char *rl_cp;
		static int el_len;

		if (rl_cp == NULL) {
			el_resize(el);
			rl_cp = el_gets(el, &el_len);
		}
		if (rl_cp == NULL)
			nr = el_len == 0 ? 0 : -1;
		else {
			nr = el_len;
			if (nr > BUFSIZ)
				nr = BUFSIZ;
			memcpy(parsefile->buf, rl_cp, nr);
			if (nr != el_len) {
				el_len -= nr;
				rl_cp += nr;
			} else
				rl_cp = NULL;
		}
	} else
#endif
		nr = read(parsefile->fd, parsefile->buf, BUFSIZ);

	if (nr <= 0) {
                if (nr < 0) {
                        if (errno == EINTR)
                                goto retry;
                        if (parsefile->fd == 0 && errno == EWOULDBLOCK) {
                                int flags = fcntl(0, F_GETFL, 0);
                                if (flags >= 0 && flags & O_NONBLOCK) {
                                        flags &=~ O_NONBLOCK;
                                        if (fcntl(0, F_SETFL, flags) >= 0) {
						out2fmt_flush("sh: turning off NDELAY mode\n");
                                                goto retry;
                                        }
                                }
                        }
                }
                nr = -1;
	}
	return nr;
}

/*
 * Refill the input buffer and return the next input character:
 *
 * 1) If a string was pushed back on the input, pop it;
 * 2) If an EOF was pushed back (parsenleft == EOF_NLEFT) or we are reading
 *    from a string so we can't refill the buffer, return EOF.
 * 3) If there is more in this buffer, use it else call read to fill it.
 * 4) Process input up to the next newline, deleting nul characters.
 */

int
ref_preadbuffer(void)
{
	char *p, *q, *r, *end;
	char savec;

	while (parsefile->strpush) {
		/*
		 * Add a space to the end of an alias to ensure that the
		 * alias remains in use while parsing its last word.
		 * This avoids alias recursions.
		 */
		if (parsenleft == -1 && parsefile->strpush->ap != NULL)
			return ' ';
		ref_popstring();
		if (--parsenleft >= 0)
			return (*parsenextc++);
	}
	if (parsenleft == EOF_NLEFT || parsefile->buf == NULL)
		return PEOF;

again:
	if (parselleft <= 0) {
		if ((parselleft = ref_preadfd()) == -1) {
			parselleft = parsenleft = EOF_NLEFT;
			return PEOF;
		}
	}

	p = parsefile->buf + (parsenextc - parsefile->buf);
	end = p + parselleft;
	*end = '\0';
	q = strchrnul(p, '\n');
	if (q != end && *q == '\0') {
		/* delete nul characters */
		for (r = q; q != end; q++) {
			if (*q != '\0')
				*r++ = *q;
		}
		parselleft -= end - r;
		if (parselleft == 0)
			goto again;
		end = p + parselleft;
		*end = '\0';
		q = strchrnul(p, '\n');
	}
	if (q == end) {
		parsenleft = parselleft;
		parselleft = 0;
	} else /* *q == '\n' */ {
		q++;
		parsenleft = q - parsenextc;
		parselleft -= parsenleft;
	}
	parsenleft--;

	savec = *q;
	*q = '\0';

#ifndef NO_HISTORY
	if (parsefile->fd == 0 && hist &&
	    parsenextc[strspn(parsenextc, " \t\n")] != '\0') {
		HistEvent he;
		INTOFF;
		history(hist, &he, whichprompt == 1 ? H_ENTER : H_ADD,
		    parsenextc);
		INTON;
	}
#endif

	if (vflag) {
		out2str(parsenextc);
		flushout(out2);
	}

	*q = savec;

	return *parsenextc++;
}

/*
 * Returns if we are certain we are at EOF. Does not cause any more input
 * to be read from the outside world.
 */

int
ref_preadateof(void)
{
	if (parsenleft > 0)
		return 0;
	if (parsefile->strpush)
		return 0;
	if (parsenleft == EOF_NLEFT || parsefile->buf == NULL)
		return 1;
	return 0;
}

/*
 * Undo the last call to ref_pgetc.  Only one character may be pushed back.
 * PEOF may be pushed back.
 */

void
ref_pungetc(void)
{
	parsenleft++;
	parsenextc--;
}

/*
 * Push a string back onto the input at this current parsefile level.
 * We handle aliases this way.
 */
void
ref_pushstring(const char *s, int len, struct alias *ap)
{
	struct strpush *sp;

	INTOFF;
/*out2fmt_flush("*** calling ref_pushstring: %s, %d\n", s, len);*/
	if (parsefile->strpush) {
		sp = ckmalloc(sizeof (struct strpush));
		sp->prev = parsefile->strpush;
		parsefile->strpush = sp;
	} else
		sp = parsefile->strpush = &(parsefile->basestrpush);
	sp->prevstring = parsenextc;
	sp->prevnleft = parsenleft;
	sp->prevlleft = parselleft;
	sp->ap = ap;
	if (ap)
		ap->flag |= ALIASINUSE;
	parsenextc = s;
	parsenleft = len;
	INTON;
}

static void
ref_popstring(void)
{
	struct strpush *sp = parsefile->strpush;

	INTOFF;
	if (sp->ap) {
		if (parsenextc != sp->ap->val &&
		    (parsenextc[-1] == ' ' || parsenextc[-1] == '\t'))
			forcealias();
		sp->ap->flag &= ~ALIASINUSE;
	}
	parsenextc = sp->prevstring;
	parsenleft = sp->prevnleft;
	parselleft = sp->prevlleft;
/*out2fmt_flush("*** calling ref_popstring: restoring to '%s'\n", parsenextc);*/
	parsefile->strpush = sp->prev;
	if (sp != &(parsefile->basestrpush))
		ckfree(sp);
	INTON;
}

/*
 * Set the input to take input from a file.  If push is set, push the
 * old input onto the stack first.
 * About verify:
 *   -1: Obey verifyflag
 *    0: Do not verify
 *    1: Do verify
 */

void
ref_setinputfile(const char *fname, int push, int verify)
{
	int e;
	int fd;
	int fd2;
	int oflags = O_RDONLY | O_CLOEXEC;

	if (verify == 1 || (verify == -1 && verifyflag))
		oflags |= O_VERIFY;

	INTOFF;
	if ((fd = open(fname, oflags)) < 0) {
		e = errno;
		errorwithstatus(e == ENOENT || e == ENOTDIR ? 127 : 126,
		    "cannot open %s: %s", fname, strerror(e));
	}
	if (fd < 10) {
		fd2 = fcntl(fd, F_DUPFD_CLOEXEC, 10);
		close(fd);
		if (fd2 < 0)
			error("Out of file descriptors");
		fd = fd2;
	}
	ref_setinputfd(fd, push);
	INTON;
}


/*
 * Like ref_setinputfile, but takes an open file descriptor (which should have
 * its FD_CLOEXEC flag already set).  Call this with interrupts off.
 */

void
ref_setinputfd(int fd, int push)
{
	if (push) {
		ref_pushfile();
		parsefile->buf = ckmalloc(BUFSIZ + 1);
	}
	if (parsefile->fd > 0)
		close(parsefile->fd);
	parsefile->fd = fd;
	if (parsefile->buf == NULL)
		parsefile->buf = ckmalloc(BUFSIZ + 1);
	parselleft = parsenleft = 0;
	plinno = 1;
}


/*
 * Like ref_setinputfile, but takes input from a string.
 */

void
ref_setinputstring(const char *string, int push)
{
	INTOFF;
	if (push)
		ref_pushfile();
	parsenextc = string;
	parselleft = parsenleft = strlen(string);
	parsefile->buf = NULL;
	plinno = 1;
	INTON;
}



/*
 * To handle the "." command, a stack of input files is used.  Pushfile
 * adds a new entry to the stack and ref_popfile restores the previous level.
 */

static void
ref_pushfile(void)
{
	struct parsefile *pf;

	parsefile->nleft = parsenleft;
	parsefile->lleft = parselleft;
	parsefile->nextc = parsenextc;
	parsefile->linno = plinno;
	pf = (struct parsefile *)ckmalloc(sizeof (struct parsefile));
	pf->prev = parsefile;
	pf->fd = -1;
	pf->strpush = NULL;
	pf->basestrpush.prev = NULL;
	parsefile = pf;
}


void
ref_popfile(void)
{
	struct parsefile *pf = parsefile;

	INTOFF;
	if (pf->fd >= 0)
		close(pf->fd);
	if (pf->buf)
		ckfree(pf->buf);
	while (pf->strpush)
		ref_popstring();
	parsefile = pf->prev;
	ckfree(pf);
	parsenleft = parsefile->nleft;
	parselleft = parsefile->lleft;
	parsenextc = parsefile->nextc;
	plinno = parsefile->linno;
	INTON;
}


/*
 * Return current file (to go back to it later using ref_popfilesupto()).
 */

struct parsefile *
ref_getcurrentfile(void)
{
	return parsefile;
}


/*
 * Pop files until the given file is on top again. Useful for regular
 * builtins that read shell commands from files or strings.
 * If the given file is not an active file, an error is raised.
 */

void
ref_popfilesupto(struct parsefile *file)
{
	while (parsefile != file && parsefile != &basepf)
		ref_popfile();
	if (parsefile != file)
		error("ref_popfilesupto() misused");
}

/*
 * Return to top level.
 */

void
ref_popallfiles(void)
{
	while (parsefile != &basepf)
		ref_popfile();
}



/*
 * Close the file(s) that the shell is reading commands from.  Called
 * after a fork is done.
 */

void
ref_closescript(void)
{
	ref_popallfiles();
	if (parsefile->fd > 0) {
		close(parsefile->fd);
		parsefile->fd = 0;
	}
}
