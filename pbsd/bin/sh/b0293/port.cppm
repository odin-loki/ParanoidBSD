module;

#include <cstdbool>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cstdint>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

export module pbsd.bin.sh.b0293;

export namespace pbsd::bin_sh::b0293 {

/* ===================================================================== *
 * Support (mirrors oracle.c support section)
 * ===================================================================== */

#define CMDTABLESIZE 31
#define READ_BUFLEN 1024
#define EXERROR 1
#define VNOFUNC 1

struct shparam {
	char **p;
	int nparam;
	char **optp;
	char **optnext;
	char *optptr;
	int reset;
	int malloc;
};

struct fdctx {
	int	fd;
	size_t	off;
	size_t	buflen;
	char	*ep;
	char	buf[READ_BUFLEN];
};

int suppressint = 0;
#define INTOFF		(suppressint++)
#define INTON		(--suppressint)

static int port_exraised = 0;
static int port_error_flag = 0;
static char port_error_msg[256];

static char port_optarg_buf[64];
static char port_optind_buf[16];
static char port_optvar_buf[16];
static int port_optarg_set;
static int port_unset_optarg;

struct shparam shellparam;
char **argptr;
char *shoptarg;
char *nextopt_optptr;

static void
error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(port_error_msg, sizeof(port_error_msg), fmt, ap);
	va_end(ap);
	port_error_flag = 1;
	
}

static void
exraise(int e)
{
	port_exraised = e;
}

static void
flushall(void)
{
}

static int
fmtstr(char *outbuf, int length, const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(outbuf, (size_t)length, fmt, ap);
	va_end(ap);
	return n;
}

static int
setvarsafe(const char *name, const char *value, int flags)
{
	(void)flags;
	if (std::strcmp(name, "OPTARG") == 0) {
		std::strncpy(port_optarg_buf, value, sizeof(port_optarg_buf) - 1);
		port_optarg_buf[sizeof(port_optarg_buf) - 1] = '\0';
		port_optarg_set = 1;
		return 0;
	}
	if (std::strcmp(name, "OPTIND") == 0) {
		std::strncpy(port_optind_buf, value, sizeof(port_optind_buf) - 1);
		port_optind_buf[sizeof(port_optind_buf) - 1] = '\0';
		return 0;
	}
	std::strncpy(port_optvar_buf, value, sizeof(port_optvar_buf) - 1);
	port_optvar_buf[sizeof(port_optvar_buf) - 1] = '\0';
	return 0;
}

static int
unsetvar(const char *name)
{
	(void)name;
	port_unset_optarg = 1;
	return 0;
}

static void
out2fmt_flush(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fflush(stderr);
}

static void *
ckmalloc(size_t nbytes)
{
	void *p = std::malloc(nbytes);
	if (p == NULL)
		std::abort();
	return p;
}

static void
ckfree(void *p)
{
	std::free(p);
}

void
port_reset_all()
{
	suppressint = 0;
	port_exraised = 0;
	port_error_flag = 0;
	std::memset(port_error_msg, 0, sizeof(port_error_msg));
	port_optarg_set = 0;
	port_unset_optarg = 0;
	std::memset(port_optarg_buf, 0, sizeof(port_optarg_buf));
	std::memset(port_optind_buf, 0, sizeof(port_optind_buf));
	std::memset(port_optvar_buf, 0, sizeof(port_optvar_buf));
	std::memset(&shellparam, 0, sizeof(shellparam));
	argptr = NULL;
	shoptarg = NULL;
	nextopt_optptr = NULL;
}

const char *
port_get_optarg()
{
	return port_optarg_buf;
}

int
port_optarg_was_set()
{
	return port_optarg_set;
}

int
port_optarg_was_unset()
{
	return port_unset_optarg;
}

const char *
port_get_optind()
{
	return port_optind_buf;
}

const char *
port_get_optvar()
{
	return port_optvar_buf;
}

int
port_get_exraised()
{
	return port_exraised;
}

int
port_get_shellparam_reset()
{
	return shellparam.reset;
}

int
port_error_thrown()
{
	return port_error_flag;
}

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

	nul = (const char *)std::memchr(data, '\0', len);
	if (nul == NULL)
		return false;
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

	cur = lseek(fd, 0, SEEK_CUR);
	*fdc = (struct fdctx){
		.fd = fd,
		.buflen = (cur != -1) ? (size_t)READ_BUFLEN : (size_t)1,
		.ep = &fdc->buf[0],
	};
}

ssize_t
fdgetc(struct fdctx *fdc, char *c)
{
	ssize_t nread;

	if (&fdc->buf[fdc->off] == fdc->ep) {
		nread = read(fdc->fd, fdc->buf, fdc->buflen);
		if (nread <= 0) {
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
		if (p[0] == '-' && p[1] == '\0')
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
	if (std::strcmp(value, "1") == 0)
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
		if (p[0] == '-' && p[1] == '\0')
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

} /* namespace */
