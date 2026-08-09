/*
 * oracle.c -- batch b0293 reference implementation (the specification).
 *
 * The original HardenedBSD C sources for the portable subset of
 *	bin/sh/options.c
 *	bin/sh/miscbltin.c
 *	bin/sh/histedit.c
 *	bin/sh/exec.c
 * concatenated below, with every ported function renamed with a "ref_"
 * prefix.  The function bodies are unmodified.  Only the surrounding
 * environment (headers, missing defines, and the small shell runtime the
 * functions link against) has been supplied.
 */

#define _GNU_SOURCE 1

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT		(sizeof(long) * 8)
#endif
#ifndef __unused
#define __unused		__attribute__((__unused__))
#endif

/* ------------------------------------------------------------------ */
/* Shell runtime the ported functions depend on.                       */
/* ------------------------------------------------------------------ */

#define EXERROR			1
#define VNOFUNC			0x40
#define BUILTIN_SPECIAL		0x80

#define INTOFF			ref_intlevel++
#define INTON			ref_intlevel--

#define is_digit(c)		((unsigned)((c) - '0') <= 9)
#define equal(s1, s2)		(strcmp(s1, s2) == 0)

union node;
struct funcdef;

int ref_intlevel = 0;

jmp_buf ref_errjmp;
int ref_error_raised = 0;
char ref_error_msg[512];

void
ref_error_reset(void)
{
	ref_error_raised = 0;
	ref_error_msg[0] = '\0';
}

_Noreturn void error(const char *, ...);
_Noreturn void exraise(int);

void
error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(ref_error_msg, sizeof(ref_error_msg), fmt, ap);
	va_end(ap);
	ref_error_raised = 1;
	longjmp(ref_errjmp, 1);
}

void
exraise(int e)
{
	snprintf(ref_error_msg, sizeof(ref_error_msg), "exraise %d", e);
	ref_error_raised = 2;
	longjmp(ref_errjmp, 1);
}

void
flushall(void)
{
}

/* --- output capture --- */

char ref_outlog[16384];
size_t ref_outlogn = 0;

void
ref_outlog_reset(void)
{
	ref_outlogn = 0;
	ref_outlog[0] = '\0';
}

static void
outlog_add(const char *s)
{
	size_t l = strlen(s);

	if (ref_outlogn + l + 1 < sizeof(ref_outlog)) {
		memcpy(ref_outlog + ref_outlogn, s, l);
		ref_outlogn += l;
		ref_outlog[ref_outlogn] = '\0';
	}
}

void
out1str(const char *s)
{
	outlog_add(s);
}

void
out2str(const char *s)
{
	outlog_add(s);
}

void
out1fmt(const char *fmt, ...)
{
	char b[2048];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(b, sizeof(b), fmt, ap);
	va_end(ap);
	outlog_add(b);
}

void
out2fmt_flush(const char *fmt, ...)
{
	char b[2048];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(b, sizeof(b), fmt, ap);
	va_end(ap);
	outlog_add(b);
}

int
fmtstr(char *outbuf, size_t length, const char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vsnprintf(outbuf, length, fmt, ap);
	va_end(ap);
	return (r);
}

/* --- variable capture --- */

char ref_varlog[16384];
size_t ref_varlogn = 0;
int ref_setvar_calls = 0;
int ref_setvar_failat = 0;
char ref_last_optarg[256];
char ref_last_optind[32];
char ref_last_optvar[32];
int ref_optarg_state = -1;	/* -1 never, 0 unset, 1 set */

void
ref_varlog_reset(void)
{
	ref_varlogn = 0;
	ref_varlog[0] = '\0';
	ref_setvar_calls = 0;
	ref_optarg_state = -1;
	ref_last_optarg[0] = '\0';
	ref_last_optind[0] = '\0';
	ref_last_optvar[0] = '\0';
}

static void
varlog_add(const char *s)
{
	size_t l = strlen(s);

	if (ref_varlogn + l + 1 < sizeof(ref_varlog)) {
		memcpy(ref_varlog + ref_varlogn, s, l);
		ref_varlogn += l;
		ref_varlog[ref_varlogn] = '\0';
	}
}

int
setvarsafe(const char *name, const char *val, int flags)
{
	char b[1024];

	ref_setvar_calls++;
	snprintf(b, sizeof(b), "set(%s=%s,%d);", name ? name : "(null)",
	    val ? val : "(null)", flags);
	varlog_add(b);
	if (name != NULL) {
		if (strcmp(name, "OPTARG") == 0) {
			ref_optarg_state = 1;
			snprintf(ref_last_optarg, sizeof(ref_last_optarg), "%s",
			    val ? val : "");
		} else if (strcmp(name, "OPTIND") == 0) {
			snprintf(ref_last_optind, sizeof(ref_last_optind), "%s",
			    val ? val : "");
		} else {
			snprintf(ref_last_optvar, sizeof(ref_last_optvar), "%s",
			    val ? val : "");
		}
	}
	return (ref_setvar_failat != 0 && ref_setvar_calls == ref_setvar_failat)
	    ? 1 : 0;
}

void
setvar(const char *name, const char *val, int flags)
{
	char b[1024];

	snprintf(b, sizeof(b), "setvar(%s=%s,%d);", name ? name : "(null)",
	    val ? val : "(null)", flags);
	varlog_add(b);
}

int
unsetvar(const char *s)
{
	char b[1024];

	snprintf(b, sizeof(b), "unset(%s);", s ? s : "(null)");
	varlog_add(b);
	if (s != NULL && strcmp(s, "OPTARG") == 0)
		ref_optarg_state = 0;
	return (0);
}

/* --- allocation --- */

int ref_free_count = 0;
int ref_freelog_mode = 0;
char ref_freelog[8192];
size_t ref_freelogn = 0;

void
ref_free_reset(int mode)
{
	ref_free_count = 0;
	ref_freelog_mode = mode;
	ref_freelogn = 0;
	ref_freelog[0] = '\0';
}

static void
freelog_add(const char *s)
{
	size_t l = strlen(s);

	if (ref_freelogn + l + 2 < sizeof(ref_freelog)) {
		memcpy(ref_freelog + ref_freelogn, s, l);
		ref_freelogn += l;
		ref_freelog[ref_freelogn++] = '|';
		ref_freelog[ref_freelogn] = '\0';
	}
}

void *
ckmalloc(size_t nbytes)
{
	void *p;

	/*
	 * calloc rather than malloc so that fields the shell leaves
	 * uninitialised are reproducible on both sides of the comparison.
	 */
	p = calloc(1, nbytes ? nbytes : 1);
	if (p == NULL)
		error("Out of space");
	return (p);
}

void
ckfree(void *p)
{
	ref_free_count++;
	if (ref_freelog_mode == 1 && p != NULL)
		freelog_add((const char *)p);
	free(p);
}

char *
savestr(const char *s)
{
	char *p;

	p = ckmalloc(strlen(s) + 1);
	strcpy(p, s);
	return (p);
}

/* --- the shell "stack" string allocator --- */

#define REF_STACKSIZE		65536
#define SHELL_ALIGN(n)		(((n) + sizeof(void *) - 1) & ~(sizeof(void *) - 1))

char ref_stackarena[REF_STACKSIZE];
size_t ref_stacknxt = 0;

char *
ref_stack_base(void)
{
	return (ref_stackarena);
}

size_t
ref_stack_used(void)
{
	return (ref_stacknxt);
}

void
ref_stack_fill(unsigned char b, size_t n)
{
	if (n > (size_t)REF_STACKSIZE)
		n = (size_t)REF_STACKSIZE;
	memset(ref_stackarena, b, n);
	ref_stacknxt = 0;
}

static char *
stackblock(void)
{
	return (ref_stackarena + ref_stacknxt);
}

static char *
stalloc(size_t n)
{
	char *p = ref_stackarena + ref_stacknxt;

	ref_stacknxt += SHELL_ALIGN(n);
	if (ref_stacknxt > (size_t)REF_STACKSIZE)
		abort();
	return (p);
}

static void stunalloc(void *) __unused;

static void
stunalloc(void *p)
{
	ref_stacknxt = (size_t)((char *)p - ref_stackarena);
}

static char *
grabstackstr(char *p)
{
	char *result = stackblock();

	stalloc((size_t)(p - result));
	return (result);
}

#define STARTSTACKSTR(p)	((p) = stackblock())
#define CHECKSTRSPACE(n, p)	do {					\
	if ((size_t)((p) - ref_stackarena) + (size_t)(n) >		\
	    (size_t)REF_STACKSIZE)					\
		abort();						\
} while (0)
#define USTPUTC(c, p)		(*p++ = (c))
#define STPUTC(c, p)		do { CHECKSTRSPACE(1, p); USTPUTC(c, p); } while (0)
#define STACKSTRNUL(p)		(*(p) = '\0')
#define STPUTS(s, p)		do {					\
	const char *_sp = (s);						\
	while (*_sp)							\
		STPUTC(*_sp++, p);					\
} while (0)

/* --- misc string helpers (mystring.c) --- */

int
is_number(const char *p)
{
	do {
		if (!is_digit(*p))
			return (0);
	} while (*++p != '\0');
	return (1);
}

int
number(const char *s)
{
	if (!is_number(s))
		error("Illegal number: %s", s);
	return (atoi(s));
}

/* --- function definition helpers --- */

char ref_funclog[4096];
size_t ref_funclogn = 0;

void
ref_funclog_reset(void)
{
	ref_funclogn = 0;
	ref_funclog[0] = '\0';
}

void
unreffunc(struct funcdef *f)
{
	char b[64];
	size_t l;

	snprintf(b, sizeof(b), "unref(%ld);", (long)(intptr_t)f);
	l = strlen(b);
	if (ref_funclogn + l + 1 < sizeof(ref_funclog)) {
		memcpy(ref_funclog + ref_funclogn, b, l);
		ref_funclogn += l;
		ref_funclog[ref_funclogn] = '\0';
	}
}

struct funcdef *
copyfunc(union node *n)
{
	return ((struct funcdef *)(void *)n);
}

/* --- the builtin command table (builtins.c is generated; this is an
       equivalent hand-written table used by ref_find_builtin) --- */

const unsigned char builtincmd[] = {
	1, 1,			'a',
	2, 5,			'c', 'd',
	2, 6 | BUILTIN_SPECIAL,	'\200', '\377',
	4, 7,			'e', 'c', 'h', 'o',
	4, 9 | BUILTIN_SPECIAL,	'e', 'x', 'i', 't',
	4, 11,			'r', 'e', 'a', 'd',
	8, 13 | BUILTIN_SPECIAL, 'r', 'e', 'a', 'd', 'o', 'n', 'l', 'y',
	4, 15,			't', 'y', 'p', 'e',
	19, 17,			'v', 'e', 'r', 'y', 'l', 'o', 'n', 'g',
				'b', 'u', 'i', 'l', 't', 'i', 'n', 'n',
				'a', 'm', 'e',
	0
};

/* --- shell option / parameter globals (options.h) --- */

struct shparam {
	int nparam;
	unsigned char malloc;
	unsigned char reset;
	char **p;
	char **optp;
	char **optnext;
	char *optptr;
};

/* --- command entry types (exec.h) --- */

union param {
	int index;
	int (*bltin)(int, char **);
	struct funcdef *func;
};

struct cmdentry {
	int cmdtype;
	union param u;
	int special;
	const char *cmdname;
};

#define CMDUNKNOWN	-1
#define CMDNORMAL	0
#define CMDFUNCTION	1
#define CMDBUILTIN	2

/* forward declarations for the ported functions */
struct fdctx;
struct limits;
struct rlimit;
struct tblentry;

void ref_setparam(int, char **);
void ref_freeparam(struct shparam *);
int ref_shiftcmd(int, char **);
void ref_getoptsreset(const char *);
int ref_getoptscmd(int, char **);
int ref_getopts(char *, char *, char **, char ***, char **);
int ref_nextopt(const char *);
void ref_fdctx_init(int, struct fdctx *);
ssize_t ref_fdgetc(struct fdctx *, char *);
void ref_fdctx_destroy(struct fdctx *);
char *ref_fc_replace(const char *, char *, char *);
int ref_not_fcnumber(const char *);
int ref_comparator(const void *, const void *, void *);
char **ref_add_match(char **, size_t, size_t *, char *);
bool ref_isbinary(const char *, size_t);
char *ref_padvance(const char **, const char **, const char *);
int ref_find_builtin(const char *, int *);
void ref_hashcd(void);
void ref_changepath(const char *);
void ref_clearcmdentry(void);
unsigned int ref_hashname(const char *);
struct tblentry *ref_cmdlookup(const char *, int);
const void *ref_itercmd(const void *, struct cmdentry *);
void ref_delete_cmd_entry(void);
void ref_addcmdentry(const char *, struct cmdentry *);
void ref_defun(const char *, union node *);
int ref_unsetfunc(const char *);
int ref_isfunc(const char *);

/*======================================================================
 * bin/sh/options.c
 *====================================================================*/

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

char *arg0;			/* value of $0 */
struct shparam shellparam;	/* current positional parameters */
char **argptr;			/* argument list for builtin commands */
char *shoptarg;			/* set by nextopt (like getopt) */
char *nextopt_optptr;		/* used by nextopt */

char *minusc;			/* argument to -c option */

/*
 * Set the shell parameters.
 */

void
ref_setparam(int argc, char **argv)
{
	char **newparam;
	char **ap;

	ap = newparam = ckmalloc((argc + 1) * sizeof *ap);
	while (*argv) {
		*ap++ = savestr(*argv++);
	}
	*ap = NULL;
	ref_freeparam(&shellparam);
	shellparam.malloc = 1;
	shellparam.nparam = argc;
	shellparam.p = newparam;
	shellparam.optp = NULL;
	shellparam.reset = 1;
	shellparam.optnext = NULL;
}


/*
 * Free the list of positional parameters.
 */

void
ref_freeparam(struct shparam *param)
{
	char **ap;

	if (param->malloc) {
		for (ap = param->p ; *ap ; ap++)
			ckfree(*ap);
		ckfree(param->p);
	}
	if (param->optp) {
		for (ap = param->optp ; *ap ; ap++)
			ckfree(*ap);
		ckfree(param->optp);
	}
}



/*
 * The shift builtin command.
 */

int
ref_shiftcmd(int argc, char **argv)
{
	int i, n;

	n = 1;
	if (argc > 1)
		n = number(argv[1]);
	if (n > shellparam.nparam)
		return 1;
	INTOFF;
	shellparam.nparam -= n;
	if (shellparam.malloc)
		for (i = 0; i < n; i++)
			ckfree(shellparam.p[i]);
	memmove(shellparam.p, shellparam.p + n,
	    (shellparam.nparam + 1) * sizeof(shellparam.p[0]));
	shellparam.reset = 1;
	INTON;
	return 0;
}


void
ref_getoptsreset(const char *value)
{
	while (*value == '0')
		value++;
	if (strcmp(value, "1") == 0)
		shellparam.reset = 1;
}

/*
 * The getopts builtin.  Shellparam.optnext points to the next argument
 * to be processed.  Shellparam.optptr points to the next character to
 * be processed in the current argument.  If shellparam.optnext is NULL,
 * then it's the first time getopts has been called.
 */

int
ref_getoptscmd(int argc, char **argv)
{
	char **optbase = NULL, **ap;
	int i;

	if (argc < 3)
		error("usage: getopts optstring var [arg]");

	if (shellparam.reset == 1) {
		INTOFF;
		if (shellparam.optp) {
			for (ap = shellparam.optp ; *ap ; ap++)
				ckfree(*ap);
			ckfree(shellparam.optp);
			shellparam.optp = NULL;
		}
		if (argc > 3) {
			shellparam.optp = ckmalloc((argc - 2) * sizeof *ap);
			memset(shellparam.optp, '\0', (argc - 2) * sizeof *ap);
			for (i = 0; i < argc - 3; i++)
				shellparam.optp[i] = savestr(argv[i + 3]);
		}
		INTON;
		optbase = argc == 3 ? shellparam.p : shellparam.optp;
		shellparam.optnext = optbase;
		shellparam.optptr = NULL;
		shellparam.reset = 0;
	} else
		optbase = shellparam.optp ? shellparam.optp : shellparam.p;

	return ref_getopts(argv[1], argv[2], optbase, &shellparam.optnext,
		       &shellparam.optptr);
}

int
ref_getopts(char *optstr, char *optvar, char **optfirst, char ***optnext,
    char **optptr)
{
	char *p, *q;
	char c = '?';
	int done = 0;
	int ind = 0;
	int err = 0;
	char s[10];
	const char *newoptarg = NULL;

	if ((p = *optptr) == NULL || *p == '\0') {
		/* Current word is done, advance */
		if (*optnext == NULL)
			return 1;
		p = **optnext;
		if (p == NULL || *p != '-' || *++p == '\0') {
atend:
			ind = *optnext - optfirst + 1;
			*optnext = NULL;
			p = NULL;
			done = 1;
			goto out;
		}
		(*optnext)++;
		if (p[0] == '-' && p[1] == '\0')	/* check for "--" */
			goto atend;
	}

	c = *p++;
	for (q = optstr; *q != c; ) {
		if (*q == '\0') {
			if (optstr[0] == ':') {
				s[0] = c;
				s[1] = '\0';
				newoptarg = s;
			}
			else
				out2fmt_flush("Illegal option -%c\n", c);
			c = '?';
			goto out;
		}
		if (*++q == ':')
			q++;
	}

	if (*++q == ':') {
		if (*p == '\0' && (p = **optnext) == NULL) {
			if (optstr[0] == ':') {
				s[0] = c;
				s[1] = '\0';
				newoptarg = s;
				c = ':';
			}
			else {
				out2fmt_flush("No arg for -%c option\n", c);
				c = '?';
			}
			goto out;
		}

		if (p == **optnext)
			(*optnext)++;
		newoptarg = p;
		p = NULL;
	}

out:
	if (*optnext != NULL)
		ind = *optnext - optfirst + 1;
	*optptr = p;
	if (newoptarg != NULL)
		err |= setvarsafe("OPTARG", newoptarg, 0);
	else {
		INTOFF;
		err |= unsetvar("OPTARG");
		INTON;
	}
	fmtstr(s, sizeof(s), "%d", ind);
	err |= setvarsafe("OPTIND", s, VNOFUNC);
	s[0] = c;
	s[1] = '\0';
	err |= setvarsafe(optvar, s, 0);
	if (err) {
		*optnext = NULL;
		*optptr = NULL;
		flushall();
		exraise(EXERROR);
	}
	return done;
}

/*
 * Standard option processing (a la getopt) for builtin routines.  The
 * only argument that is passed to nextopt is the option string; the
 * other arguments are unnecessary.  It returns the option, or '\0' on
 * end of input.
 */

int
ref_nextopt(const char *optstring)
{
	char *p;
	const char *q;
	char c;

	if ((p = nextopt_optptr) == NULL || *p == '\0') {
		p = *argptr;
		if (p == NULL || *p != '-' || *++p == '\0')
			return '\0';
		argptr++;
		if (p[0] == '-' && p[1] == '\0')	/* check for "--" */
			return '\0';
	}
	c = *p++;
	for (q = optstring ; *q != c ; ) {
		if (*q == '\0')
			error("Illegal option -%c", c);
		if (*++q == ':')
			q++;
	}
	if (*++q == ':') {
		if (*p == '\0' && (p = *argptr++) == NULL)
			error("No arg for -%c option", c);
		shoptarg = p;
		p = NULL;
	}
	if (p != NULL && *p != '\0')
		nextopt_optptr = p;
	else
		nextopt_optptr = NULL;
	return c;
}

/*======================================================================
 * bin/sh/miscbltin.c
 *====================================================================*/

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
 * Miscellaneous builtins.
 */

#define	READ_BUFLEN	1024
struct fdctx {
	int	fd;
	size_t	off;	/* offset in buf */
	size_t	buflen;
	char	*ep;	/* tail pointer */
	char	buf[READ_BUFLEN];
};

void
ref_fdctx_init(int fd, struct fdctx *fdc)
{
	off_t cur;

	/* Check if fd is seekable. */
	cur = lseek(fd, 0, SEEK_CUR);
	*fdc = (struct fdctx){
		.fd = fd,
		.buflen = (cur != -1) ? READ_BUFLEN : 1,
		.ep = &fdc->buf[0],	/* No data */
	};
}

ssize_t
ref_fdgetc(struct fdctx *fdc, char *c)
{
	ssize_t nread;

	if (&fdc->buf[fdc->off] == fdc->ep) {
		nread = read(fdc->fd, fdc->buf, fdc->buflen);
		if (nread > 0) {
			fdc->off = 0;
			fdc->ep = fdc->buf + nread;
		} else
			return (nread);
	}
	*c = fdc->buf[fdc->off++];

	return (1);
}

void
ref_fdctx_destroy(struct fdctx *fdc)
{
	off_t residue;

	if (fdc->buflen > 1) {
	/*
	 * Reposition the file offset.  Here is the layout of buf:
	 *
	 *     | off
	 *     v
	 * |*****************|-------|
	 * buf               ep   buf+buflen
	 *     |<- residue ->|
	 *
	 * off: current character
	 * ep:  offset just after read(2)
	 * residue: length for reposition
	 */
		residue = (fdc->ep - fdc->buf) - fdc->off;
		if (residue > 0)
			(void) lseek(fdc->fd, -residue, SEEK_CUR);
	}
}

/*
 * ulimit builtin
 *
 * This code, originally by Doug Gwyn, Doug Kingston, Eric Gisin, and
 * Michael Rendell was ripped from pdksh 5.0.8 and hacked for use with
 * ash by J.T. Conklin.
 *
 * Public domain.
 */

struct limits {
	const char *name;
	const char *units;
	int	cmd;
	short	factor;	/* multiply by to get rlim_{cur,max} values */
	char	option;
};

static const struct limits limits[] = {
#ifdef RLIMIT_CPU
	{ "cpu time",		"seconds",	RLIMIT_CPU,	   1, 't' },
#endif
#ifdef RLIMIT_FSIZE
	{ "file size",		"512-blocks",	RLIMIT_FSIZE,	 512, 'f' },
#endif
#ifdef RLIMIT_DATA
	{ "data seg size",	"kbytes",	RLIMIT_DATA,	1024, 'd' },
#endif
#ifdef RLIMIT_STACK
	{ "stack size",		"kbytes",	RLIMIT_STACK,	1024, 's' },
#endif
#ifdef  RLIMIT_CORE
	{ "core file size",	"512-blocks",	RLIMIT_CORE,	 512, 'c' },
#endif
#ifdef RLIMIT_RSS
	{ "max memory size",	"kbytes",	RLIMIT_RSS,	1024, 'm' },
#endif
#ifdef RLIMIT_MEMLOCK
	{ "locked memory",	"kbytes",	RLIMIT_MEMLOCK, 1024, 'l' },
#endif
#ifdef RLIMIT_NPROC
	{ "max user processes",	(char *)0,	RLIMIT_NPROC,      1, 'u' },
#endif
#ifdef RLIMIT_NOFILE
	{ "open files",		(char *)0,	RLIMIT_NOFILE,     1, 'n' },
#endif
#ifdef RLIMIT_VMEM
	{ "virtual mem size",	"kbytes",	RLIMIT_VMEM,	1024, 'v' },
#endif
#ifdef RLIMIT_SWAP
	{ "swap limit",		"kbytes",	RLIMIT_SWAP,	1024, 'w' },
#endif
#ifdef RLIMIT_SBSIZE
	{ "socket buffer size",	"bytes",	RLIMIT_SBSIZE,	   1, 'b' },
#endif
#ifdef RLIMIT_NPTS
	{ "pseudo-terminals",	(char *)0,	RLIMIT_NPTS,	   1, 'p' },
#endif
#ifdef RLIMIT_KQUEUES
	{ "kqueues",		(char *)0,	RLIMIT_KQUEUES,	   1, 'k' },
#endif
#ifdef RLIMIT_UMTXP
	{ "umtx shared locks",	(char *)0,	RLIMIT_UMTXP,	   1, 'o' },
#endif
#ifdef RLIMIT_PIPEBUF
	{ "pipebuf",		"kbytes",	RLIMIT_PIPEBUF, 1024, 'y' },
#endif
	{ (char *) 0,		(char *)0,	0,		   0, '\0' }
};

enum limithow { SOFT = 0x1, HARD = 0x2 };

void
ref_printlimit(enum limithow how, const struct rlimit *limit,
    const struct limits *l)
{
	rlim_t val = 0;

	if (how & SOFT)
		val = limit->rlim_cur;
	else if (how & HARD)
		val = limit->rlim_max;
	if (val == RLIM_INFINITY)
		out1str("unlimited\n");
	else
	{
		val /= l->factor;
		out1fmt("%jd\n", (intmax_t)val);
	}
}

/*======================================================================
 * bin/sh/histedit.c
 *====================================================================*/

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

char *
ref_fc_replace(const char *s, char *p, char *r)
{
	char *dest;
	int plen = strlen(p);

	STARTSTACKSTR(dest);
	while (*s) {
		if (*s == *p && strncmp(s, p, plen) == 0) {
			STPUTS(r, dest);
			s += plen;
			*p = '\0';	/* so no more matches */
		} else
			STPUTC(*s++, dest);
	}
	STPUTC('\0', dest);
	dest = grabstackstr(dest);

	return (dest);
}

int
ref_not_fcnumber(const char *s)
{
	if (s == NULL)
		return (0);
	if (*s == '-')
		s++;
	return (!is_number(s));
}

/*
 * Comparator function for qsort(). The use of curpos here is to skip
 * characters that we already know to compare equal (common prefix).
 */
int
ref_comparator(const void *a, const void *b, void *thunk)
{
	size_t curpos = (intptr_t)thunk;

	return (strcmp(*(char *const *)a + curpos,
		*(char *const *)b + curpos));
}

char
**ref_add_match(char **matches, size_t i, size_t *size, char *match_copy)
{
	if (match_copy == NULL)
		return (NULL);
	matches[i] = match_copy;
	if (i >= *size - 1) {
		*size *= 2;
		matches = reallocarray(matches, *size, sizeof(matches[0]));
	}

	return (matches);
}

/*======================================================================
 * bin/sh/exec.c
 *====================================================================*/

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
 * When commands are first encountered, they are entered in a hash table.
 * This ensures that a full path search will not have to be done for them
 * on each invocation.
 *
 * We should investigate converting to a linear search, even though that
 * would make the command name "hash" a misnomer.
 */

#define CMDTABLESIZE 31		/* should be prime */

struct tblentry {
	struct tblentry *next;	/* next entry in hash chain */
	union param param;	/* definition of builtin function */
	int special;		/* flag for special builtin commands */
	signed char cmdtype;	/* index identifying command */
	char cmdname[];		/* name of command */
};


static struct tblentry *cmdtable[CMDTABLESIZE];
static int cmdtable_cd = 0;	/* cmdtable contains cd-dependent entries */


bool
ref_isbinary(const char *data, size_t len)
{
	const char *nul, *p;
	bool hasletter;

	nul = memchr(data, '\0', len);
	if (nul == NULL)
		return false;
	/*
	 * POSIX says we shall allow execution if the initial part intended
	 * to be parsed by the shell consists of characters and does not
	 * contain the NUL character. This allows concatenating a shell
	 * script (ending with exec or exit) and a binary payload.
	 *
	 * In order to reject common binary files such as PNG images, check
	 * that there is a lowercase letter or expansion before the last
	 * newline before the NUL character, in addition to the check for
	 * the newline character suggested by POSIX.
	 */
	hasletter = false;
	for (p = data; *p != '\0'; p++) {
		if ((*p >= 'a' && *p <= 'z') || *p == '$' || *p == '`')
			hasletter = true;
		if (hasletter && *p == '\n')
			return false;
	}
	return true;
}

/*
 * Do a path search.  The variable path (passed by reference) should be
 * set to the start of the path before the first call; padvance will update
 * this value as it proceeds.  Successive calls to padvance will return
 * the possible path expansions in sequence.  If popt is not NULL, options
 * are processed: if an option (indicated by a percent sign) appears in
 * the path entry then *popt will be set to point to it; else *popt will be
 * set to NULL.  If popt is NULL, percent signs are not special.
 */

char *
ref_padvance(const char **path, const char **popt, const char *name)
{
	const char *p, *start;
	char *q;
	size_t len, namelen;

	if (*path == NULL)
		return NULL;
	start = *path;
	if (popt != NULL)
		for (p = start; *p && *p != ':' && *p != '%'; p++)
			; /* nothing */
	else
		for (p = start; *p && *p != ':'; p++)
			; /* nothing */
	namelen = strlen(name);
	len = p - start + namelen + 2;	/* "2" is for '/' and '\0' */
	STARTSTACKSTR(q);
	CHECKSTRSPACE(len, q);
	if (p != start) {
		memcpy(q, start, p - start);
		q += p - start;
		*q++ = '/';
	}
	memcpy(q, name, namelen + 1);
	if (popt != NULL) {
		if (*p == '%') {
			*popt = ++p;
			while (*p && *p != ':')  p++;
		} else
			*popt = NULL;
	}
	if (*p == ':')
		*path = p + 1;
	else
		*path = NULL;
	return stalloc(len);
}


/*
 * Search the table of builtin commands.
 */

int
ref_find_builtin(const char *name, int *special)
{
	const unsigned char *bp;
	size_t len;

	len = strlen(name);
	for (bp = builtincmd ; *bp ; bp += 2 + bp[0]) {
		if (bp[0] == len && memcmp(bp + 2, name, len) == 0) {
			*special = (bp[1] & BUILTIN_SPECIAL) != 0;
			return bp[1] & ~BUILTIN_SPECIAL;
		}
	}
	return -1;
}



/*
 * Called when a cd is done.  If any entry in cmdtable depends on the current
 * directory, simply clear cmdtable completely.
 */

void
ref_hashcd(void)
{
	if (cmdtable_cd)
		ref_clearcmdentry();
}



/*
 * Called before PATH is changed.  The argument is the new value of PATH;
 * pathval() still returns the old value at this point.  Called with
 * interrupts off.
 */

void
ref_changepath(const char *newval __unused)
{
	ref_clearcmdentry();
}


/*
 * Clear out cached utility locations.
 */

void
ref_clearcmdentry(void)
{
	struct tblentry **tblp;
	struct tblentry **pp;
	struct tblentry *cmdp;

	INTOFF;
	for (tblp = cmdtable ; tblp < &cmdtable[CMDTABLESIZE] ; tblp++) {
		pp = tblp;
		while ((cmdp = *pp) != NULL) {
			if (cmdp->cmdtype == CMDNORMAL) {
				*pp = cmdp->next;
				ckfree(cmdp);
			} else {
				pp = &cmdp->next;
			}
		}
	}
	cmdtable_cd = 0;
	INTON;
}


unsigned int
ref_hashname(const char *p)
{
	unsigned int hashval;

	hashval = (unsigned char)*p << 4;
	while (*p)
		hashval += *p++;

	return (hashval % CMDTABLESIZE);
}


/*
 * Locate a command in the command hash table.  If "add" is nonzero,
 * add the command to the table if it is not already present.  The
 * variable "lastcmdentry" is set to point to the address of the link
 * pointing to the entry, so that delete_cmd_entry can delete the
 * entry.
 */

static struct tblentry **lastcmdentry;


struct tblentry *
ref_cmdlookup(const char *name, int add)
{
	struct tblentry *cmdp;
	struct tblentry **pp;
	size_t len;

	pp = &cmdtable[ref_hashname(name)];
	for (cmdp = *pp ; cmdp ; cmdp = cmdp->next) {
		if (equal(cmdp->cmdname, name))
			break;
		pp = &cmdp->next;
	}
	if (add && cmdp == NULL) {
		INTOFF;
		len = strlen(name);
		cmdp = *pp = ckmalloc(sizeof (struct tblentry) + len + 1);
		cmdp->next = NULL;
		cmdp->cmdtype = CMDUNKNOWN;
		memcpy(cmdp->cmdname, name, len + 1);
		INTON;
	}
	lastcmdentry = pp;
	return cmdp;
}

const void *
ref_itercmd(const void *entry, struct cmdentry *result)
{
	const struct tblentry *e = entry;
	size_t i = 0;

	if (e != NULL) {
		if (e->next != NULL) {
			e = e->next;
			goto success;
		}
		i = ref_hashname(e->cmdname) + 1;
	}
	for (; i < CMDTABLESIZE; i++)
		if ((e = cmdtable[i]) != NULL)
			goto success;

	return (NULL);
success:
	result->cmdtype = e->cmdtype;
	result->cmdname = e->cmdname;

	return (e);
}

/*
 * Delete the command entry returned on the last lookup.
 */

void
ref_delete_cmd_entry(void)
{
	struct tblentry *cmdp;

	INTOFF;
	cmdp = *lastcmdentry;
	*lastcmdentry = cmdp->next;
	ckfree(cmdp);
	INTON;
}



/*
 * Add a new command entry, replacing any existing command entry for
 * the same name.
 */

void
ref_addcmdentry(const char *name, struct cmdentry *entry)
{
	struct tblentry *cmdp;

	INTOFF;
	cmdp = ref_cmdlookup(name, 1);
	if (cmdp->cmdtype == CMDFUNCTION) {
		unreffunc(cmdp->param.func);
	}
	cmdp->cmdtype = entry->cmdtype;
	cmdp->param = entry->u;
	cmdp->special = entry->special;
	INTON;
}


/*
 * Define a shell function.
 */

void
ref_defun(const char *name, union node *func)
{
	struct cmdentry entry;

	INTOFF;
	entry.cmdtype = CMDFUNCTION;
	entry.u.func = copyfunc(func);
	entry.special = 0;
	ref_addcmdentry(name, &entry);
	INTON;
}


/*
 * Delete a function if it exists.
 * Called with interrupts off.
 */

int
ref_unsetfunc(const char *name)
{
	struct tblentry *cmdp;

	if ((cmdp = ref_cmdlookup(name, 0)) != NULL && cmdp->cmdtype == CMDFUNCTION) {
		unreffunc(cmdp->param.func);
		ref_delete_cmd_entry();
		return (0);
	}
	return (0);
}


/*
 * Check if a function by a certain name exists.
 */
int
ref_isfunc(const char *name)
{
	struct tblentry *cmdp;
	cmdp = ref_cmdlookup(name, 0);
	return (cmdp != NULL && cmdp->cmdtype == CMDFUNCTION);
}

/*======================================================================
 * Test hooks.  Not part of the port; they only expose file-scope state
 * and provide setjmp landing pads for error()/exraise().
 *====================================================================*/

int
ref_try_nextopt(const char *optstring, int *res)
{
	if (setjmp(ref_errjmp))
		return (1);
	*res = ref_nextopt(optstring);
	return (0);
}

int
ref_try_getopts(char *optstr, char *optvar, char **optfirst, char ***optnext,
    char **optptr, int *res)
{
	if (setjmp(ref_errjmp))
		return (1);
	*res = ref_getopts(optstr, optvar, optfirst, optnext, optptr);
	return (0);
}

int
ref_try_getoptscmd(int argc, char **argv, int *res)
{
	if (setjmp(ref_errjmp))
		return (1);
	*res = ref_getoptscmd(argc, argv);
	return (0);
}

int
ref_try_shiftcmd(int argc, char **argv, int *res)
{
	if (setjmp(ref_errjmp))
		return (1);
	*res = ref_shiftcmd(argc, argv);
	return (0);
}

int
ref_try_setparam(int argc, char **argv)
{
	if (setjmp(ref_errjmp))
		return (1);
	ref_setparam(argc, argv);
	return (0);
}

int
ref_try_freeparam(void)
{
	if (setjmp(ref_errjmp))
		return (1);
	ref_freeparam(&shellparam);
	return (0);
}

/* --- fdctx --- */

static struct fdctx ref_fdc;
static char ref_fdc_c;

void
ref_fdc_guard(unsigned char b)
{
	memset(&ref_fdc, b, sizeof(ref_fdc));
	ref_fdc_c = (char)b;
}

void
ref_fdc_init(int fd)
{
	ref_fdctx_init(fd, &ref_fdc);
}

long
ref_fdc_getc(void)
{
	return ((long)ref_fdgetc(&ref_fdc, &ref_fdc_c));
}

void
ref_fdc_destroy(void)
{
	ref_fdctx_destroy(&ref_fdc);
}

int ref_fdc_fd(void) { return (ref_fdc.fd); }
unsigned long ref_fdc_off(void) { return ((unsigned long)ref_fdc.off); }
unsigned long ref_fdc_buflen(void) { return ((unsigned long)ref_fdc.buflen); }
long ref_fdc_epoff(void) { return ((long)(ref_fdc.ep - ref_fdc.buf)); }
const void *ref_fdc_bufptr(void) { return (ref_fdc.buf); }
unsigned long ref_fdc_bufsize(void) { return ((unsigned long)READ_BUFLEN); }
int ref_fdc_lastc(void) { return ((int)ref_fdc_c); }

/* --- printlimit --- */

void
ref_printlimit_w(int how, rlim_t cur, rlim_t max, short factor)
{
	struct rlimit rl;
	struct limits l;

	memset(&rl, 0, sizeof(rl));
	memset(&l, 0, sizeof(l));
	rl.rlim_cur = cur;
	rl.rlim_max = max;
	l.name = "x";
	l.units = "y";
	l.cmd = 0;
	l.factor = factor;
	l.option = 'z';
	ref_printlimit((enum limithow)how, &rl, &l);
}

int
ref_limits_count(void)
{
	int i = 0;

	while (limits[i].name != NULL)
		i++;
	return (i);
}

short
ref_limits_factor(int i)
{
	return (limits[i].factor);
}

/* --- cmdtable --- */

void
ref_set_cmdtable_cd(int v)
{
	cmdtable_cd = v;
}

int
ref_get_cmdtable_cd(void)
{
	return (cmdtable_cd);
}

void
ref_reset_cmdtable(void)
{
	int i;

	for (i = 0; i < CMDTABLESIZE; i++) {
		struct tblentry *e = cmdtable[i], *n;

		while (e != NULL) {
			n = e->next;
			free(e);
			e = n;
		}
		cmdtable[i] = NULL;
	}
	cmdtable_cd = 0;
	lastcmdentry = NULL;
}

void
ref_dump_cmdtable(char *out, size_t n)
{
	size_t o = 0;
	int i;

	if (n == 0)
		return;
	out[0] = '\0';
	for (i = 0; i < CMDTABLESIZE; i++) {
		const struct tblentry *e;

		for (e = cmdtable[i]; e != NULL; e = e->next) {
			unsigned long long v = 0;
			int k;

			memcpy(&v, &e->param, sizeof(e->param));
			k = snprintf(out + o, n - o, "[%d:%s:%d:%d:%llu]",
			    i, e->cmdname, (int)e->cmdtype, e->special, v);
			if (k < 0 || (size_t)k >= n - o) {
				out[n - 1] = '\0';
				return;
			}
			o += (size_t)k;
		}
	}
	out[o] = '\0';
}

const void *
ref_cmdlookup_w(const char *name, int add)
{
	return (ref_cmdlookup(name, add));
}

void
ref_entry_info(const void *ep, char *out, size_t n)
{
	const struct tblentry *e = ep;
	unsigned long long v = 0;

	if (e == NULL) {
		snprintf(out, n, "NULL");
		return;
	}
	memcpy(&v, &e->param, sizeof(e->param));
	snprintf(out, n, "%s:%d:%d:%llu", e->cmdname, (int)e->cmdtype,
	    e->special, v);
}

void
ref_addcmdentry_w(const char *name, int cmdtype, long value, int special)
{
	struct cmdentry entry;

	memset(&entry, 0, sizeof(entry));
	entry.cmdtype = cmdtype;
	if (cmdtype == CMDFUNCTION)
		entry.u.func = (struct funcdef *)(intptr_t)value;
	else
		entry.u.index = (int)value;
	entry.special = special;
	entry.cmdname = NULL;
	ref_addcmdentry(name, &entry);
}

void
ref_defun_w(const char *name, long value)
{
	ref_defun(name, (union node *)(intptr_t)value);
}

int
	ref_iter_step(const void **state, char *nameout, size_t n, int *typeout)
{
	struct cmdentry result;
	const void *r;

	memset(&result, 0, sizeof(result));
	r = ref_itercmd(*state, &result);
	if (r == NULL) {
		*state = NULL;
		return (0);
	}
	*state = r;
	snprintf(nameout, n, "%s", result.cmdname);
	*typeout = result.cmdtype;
	return (1);
}

/* --- harness hooks (not part of the specification) --- */

void
oracle_reset_all(void)
{
	ref_error_reset();
	ref_outlog_reset();
	ref_varlog_reset();
	ref_free_reset(0);
	ref_stack_fill(0x7f, REF_STACKSIZE);
	ref_funclog_reset();
	memset(&shellparam, 0, sizeof(shellparam));
	argptr = NULL;
	shoptarg = NULL;
	nextopt_optptr = NULL;
	arg0 = NULL;
	minusc = NULL;
	ref_intlevel = 0;
}

const char *
oracle_get_optarg(void)
{
	return (ref_last_optarg);
}

int
oracle_optarg_was_set(void)
{
	return (ref_optarg_state == 1);
}

int
oracle_optarg_was_unset(void)
{
	return (ref_optarg_state == 0);
}

const char *
oracle_get_optind(void)
{
	return (ref_last_optind);
}

const char *
oracle_get_optvar(void)
{
	return (ref_last_optvar);
}

int
oracle_get_shellparam_reset(void)
{
	return ((int)shellparam.reset);
}

int
oracle_get_error_thrown(void)
{
	return (ref_error_raised != 0);
}

void
oracle_set_argptr(char **ap)
{
	argptr = ap;
}

char *
oracle_get_shoptarg(void)
{
	return (shoptarg);
}

void
oracle_set_nextopt_optptr(char *p)
{
	nextopt_optptr = p;
}
