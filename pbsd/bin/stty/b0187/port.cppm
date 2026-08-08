module;

#ifndef __dead2
#define __dead2	__attribute__((__noreturn__))
#endif

#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <err.h>
#include <limits.h>
#include <unistd.h>

#include <termios.h>
#include <sys/ttydefaults.h>

export module pbsd.bin.stty.b0187;

export namespace pbsd::bin_stty::b0187 {

struct info {
	int fd;
	int ldisc;
	int off;
	int set;
	int wset;
	const char *arg;
	struct termios t;
	struct winsize win;
};

struct cchar {
	const char *name;
	int sub;
	u_char def;
};

struct cchar cchars1[] = {
	{ "discard",	VDISCARD, 	CDISCARD },
	{ "dsusp", 	VDSUSP,		CDSUSP },
	{ "eof",	VEOF,		CEOF },
	{ "eol",	VEOL,		CEOL },
	{ "eol2",	VEOL2,		CEOL },
	{ "erase",	VERASE,		CERASE },
	{ "erase2",	VERASE2,	CERASE2 },
	{ "intr",	VINTR,		CINTR },
	{ "kill",	VKILL,		CKILL },
	{ "lnext",	VLNEXT,		CLNEXT },
	{ "min",	VMIN,		CMIN },
	{ "quit",	VQUIT,		CQUIT },
	{ "reprint",	VREPRINT, 	CREPRINT },
	{ "start",	VSTART,		CSTART },
	{ "status",	VSTATUS, 	CSTATUS },
	{ "stop",	VSTOP,		CSTOP },
	{ "susp",	VSUSP,		CSUSP },
	{ "time",	VTIME,		CTIME },
	{ "werase",	VWERASE,	CWERASE },
	{ NULL,		0,		0},
};

struct cchar cchars2[] = {
	{ "brk",	VEOL,		CEOL },
	{ "flush",	VDISCARD, 	CDISCARD },
	{ "rprnt",	VREPRINT, 	CREPRINT },
	{ NULL,		0,		0 },
};

namespace {

static int c_cchar(const void *a, const void *b)
{

        return (strcmp(((const struct cchar *)a)->name, ((const struct cchar *)b)->name));
}

} // namespace

/*-
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
checkredirect(void)
{
	struct stat sb1, sb2;

	if (isatty(STDOUT_FILENO) && isatty(STDERR_FILENO) &&
	    !fstat(STDOUT_FILENO, &sb1) && !fstat(STDERR_FILENO, &sb2) &&
	    (sb1.st_rdev != sb2.st_rdev))
warnx("stdout appears redirected, but stdin is the control descriptor");
}

/*-
 * Copyright (c) 1989, 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
usage(void)
{

	(void)fprintf(stderr,
	    "usage: stty [-a | -e | -g] [-f file] [arguments]\n");
	exit (1);
}

/*-
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
csearch(char ***argvp, struct info *ip)
{
	struct cchar *cp, tmp;
	long val;
	char *arg, *ep, *name;

	name = **argvp;

	tmp.name = name;
	if (!(cp = (struct cchar *)bsearch(&tmp, cchars1,
	    sizeof(cchars1)/sizeof(struct cchar) - 1, sizeof(struct cchar),
	    c_cchar)) && !(cp = (struct cchar *)bsearch(&tmp, cchars2,
	    sizeof(cchars2)/sizeof(struct cchar) - 1, sizeof(struct cchar),
	    c_cchar)))
		return (0);

	arg = *++*argvp;
	if (!arg) {
		warnx("option requires an argument -- %s", name);
		usage();
	}

#define CHK(s)  (*arg == s[0] && !strcmp(arg, s))
	if (CHK("undef") || CHK("<undef>"))
		ip->t.c_cc[cp->sub] = _POSIX_VDISABLE;
	else if (cp->sub == VMIN || cp->sub == VTIME) {
		val = strtol(arg, &ep, 10);
		if (val > UCHAR_MAX) {
			warnx("maximum option value is %d -- %s",
			    UCHAR_MAX, name);
			usage();
		}
		if (*ep != '\0') {
			warnx("option requires a numeric argument -- %s", name);
			usage();
		}
		ip->t.c_cc[cp->sub] = val;
	} else if (arg[0] == '^')
		ip->t.c_cc[cp->sub] = (arg[1] == '?') ? 0177 :
		    (arg[1] == '-') ? _POSIX_VDISABLE : arg[1] & 037;
	else
		ip->t.c_cc[cp->sub] = arg[0];
	ip->set = 1;
	return (1);
}

} // namespace pbsd::bin_stty::b0187
