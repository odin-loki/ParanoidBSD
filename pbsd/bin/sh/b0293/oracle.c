/*
 * PBSD batch b0293 -- reference oracle.
 *
 * Partial ports from:
 *	hbsd/src/bin/sh/options.c
 *	hbsd/src/bin/sh/miscbltin.c
 *	hbsd/src/bin/sh/exec.c
 *
 * Every ported function is renamed with a "ref_" prefix via macros below so
 * that the function bodies remain byte-for-byte the originals.  Support for
 * headers outside this batch is supplied in the support section.
 */

#define _GNU_SOURCE 1

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/* ===================================================================== *
 * Support section
 * ===================================================================== */

#define CMDTABLESIZE 31

/* error.h / shell interrupt simulation */
int suppressint = 0;
#define INTOFF		(suppressint++)
#define INTON		(--suppressint)

#define EXERROR 1

static int oracle_exraised = 0;
static int oracle_error_thrown = 0;
static char oracle_error_msg[256];

static void
error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(oracle_error_msg, sizeof(oracle_error_msg), fmt, ap);
	va_end(ap);
	oracle_error_thrown = 1;
}

int
oracle_get_error_thrown(void)
{
	return oracle_error_thrown;
}

static void
exraise(int e)
{
	oracle_exraised = e;
}

static void
flushall(void)
{
}

/* mystring.h */
#define fmtstr oracle_fmtstr

static int
oracle_fmtstr(char *outbuf, int length, const char *fmt, ...)
{
	int n;
	va_list ap;

	va_start(ap, fmt);
	n = vsnprintf(outbuf, (size_t)length, fmt, ap);
	va_end(ap);
	return n;
}

/* var.h mocks for getopts */
#define VNOFUNC 1

static char oracle_optarg_buf[64];
static char oracle_optind_buf[16];
static char oracle_optvar_buf[16];
static int oracle_optarg_set;
static int oracle_unset_optarg;

static int
setvarsafe(const char *name, const char *value, int flags)
{
	(void)flags;
	if (strcmp(name, "OPTARG") == 0) {
		strncpy(oracle_optarg_buf, value, sizeof(oracle_optarg_buf) - 1);
		oracle_optarg_buf[sizeof(oracle_optarg_buf) - 1] = '\0';
		oracle_optarg_set = 1;
		return 0;
	}
	if (strcmp(name, "OPTIND") == 0) {
		strncpy(oracle_optind_buf, value, sizeof(oracle_optind_buf) - 1);
		oracle_optind_buf[sizeof(oracle_optind_buf) - 1] = '\0';
		return 0;
	}
	strncpy(oracle_optvar_buf, value, sizeof(oracle_optvar_buf) - 1);
	oracle_optvar_buf[sizeof(oracle_optvar_buf) - 1] = '\0';
	return 0;
}

static int
unsetvar(const char *name)
{
	(void)name;
	oracle_unset_optarg = 1;
	return 0;
}

static void
out2fmt_flush(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fflush(stderr);
}

/* options.h / shell globals for nextopt and getoptsreset */
struct shparam {
	char **p;
	int nparam;
	char **optp;
	char **optnext;
	char *optptr;
	int reset;
	int malloc;
};

struct shparam shellparam;

char **argptr;
char *shoptarg;
char *nextopt_optptr;

/* memalloc.h */
static void *
ckmalloc(size_t nbytes)
{
	void *p = malloc(nbytes);
	if (p == NULL)
		abort();
	return p;
}

static void
ckfree(void *p)
{
	free(p);
}

/* ===================================================================== *
 * Harness-visible oracle state accessors
 * ===================================================================== */

void
oracle_reset_all(void)
{
	suppressint = 0;
	oracle_exraised = 0;
	oracle_error_thrown = 0;
	memset(oracle_error_msg, 0, sizeof(oracle_error_msg));
	oracle_optarg_set = 0;
	oracle_unset_optarg = 0;
	memset(oracle_optarg_buf, 0, sizeof(oracle_optarg_buf));
	memset(oracle_optind_buf, 0, sizeof(oracle_optind_buf));
	memset(oracle_optvar_buf, 0, sizeof(oracle_optvar_buf));
	memset(&shellparam, 0, sizeof(shellparam));
	argptr = NULL;
	shoptarg = NULL;
	nextopt_optptr = NULL;
}

const char *
oracle_get_optarg(void)
{
	return oracle_optarg_buf;
}

int
oracle_optarg_was_set(void)
{
	return oracle_optarg_set;
}

int
oracle_optarg_was_unset(void)
{
	return oracle_unset_optarg;
}

const char *
oracle_get_optind(void)
{
	return oracle_optind_buf;
}

const char *
oracle_get_optvar(void)
{
	return oracle_optvar_buf;
}

int
oracle_get_exraised(void)
{
	return oracle_exraised;
}

int
oracle_get_shellparam_reset(void)
{
	return shellparam.reset;
}

/* globals for harness nextopt driving */
char **
oracle_argptr_ptr(void)
{
	return argptr;
}

void
oracle_set_argptr(char **ap)
{
	argptr = ap;
}

char *
oracle_get_shoptarg(void)
{
	return shoptarg;
}

void
oracle_set_nextopt_optptr(char *p)
{
	nextopt_optptr = p;
}

/* ===================================================================== *
 * ref_ renaming
 * ===================================================================== */

#define isbinary		ref_isbinary
#define hashname		ref_hashname
#define fdctx_init		ref_fdctx_init
#define fdgetc			ref_fdgetc
#define fdctx_destroy		ref_fdctx_destroy
#define nextopt			ref_nextopt
#define getoptsreset		ref_getoptsreset
#define freeparam		ref_freeparam
#define getopts			ref_getopts

/* miscbltin.c support */
#define READ_BUFLEN 1024

struct fdctx {
	int	fd;
	size_t	off;
	size_t	buflen;
	char	*ep;
	char	buf[READ_BUFLEN];
};

/* ===================================================================== *
 * exec.c -- isbinary, hashname
 * ===================================================================== */

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

bool
isbinary(const char *data, size_t len)
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

unsigned int
hashname(const char *p)
{
	unsigned int hashval;

	hashval = (unsigned char)*p << 4;
	while (*p)
		hashval += *p++;

	return (hashval % CMDTABLESIZE);
}

/* ===================================================================== *
 * miscbltin.c -- fdctx_init, fdgetc, fdctx_destroy
 * ===================================================================== */

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
fdctx_init(int fd, struct fdctx *fdc)
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
fdgetc(struct fdctx *fdc, char *c)
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
fdctx_destroy(struct fdctx *fdc)
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

/* ===================================================================== *
 * options.c -- nextopt, getoptsreset, freeparam, getopts
 * ===================================================================== */

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

int
nextopt(const char *optstring)
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

void
getoptsreset(const char *value)
{
	while (*value == '0')
		value++;
	if (strcmp(value, "1") == 0)
		shellparam.reset = 1;
}

void
freeparam(struct shparam *param)
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

int
getopts(char *optstr, char *optvar, char **optfirst, char ***optnext,
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
