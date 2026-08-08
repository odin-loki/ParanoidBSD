module;

#define _DEFAULT_SOURCE
#define _WANT_FREEBSD_BITSET

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef __dead2
#define __dead2	__attribute__((__noreturn__))
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

export module pbsd.bin.cpuset.b0183;

export namespace pbsd::bin_cpuset::b0183 {

#define _BITSET_BITS		(sizeof(unsigned long) * 8)
#define __howmany(x, y)		(((x) + ((y) - 1)) / (y))
#define __bitset_words(_s)	(__howmany(_s, _BITSET_BITS))
#define __BITSET_DEFINE(_t, _s)						\
struct _t {								\
	unsigned long __bits[__bitset_words((_s))];			\
}
#define __bitset_mask(_s, n)						\
	(1UL << (__builtin_constant_p(__bitset_words((_s)) == 1) ?	\
	    (size_t)(n) : ((n) % _BITSET_BITS)))
#define __bitset_word(_s, n)						\
	(__builtin_constant_p(__bitset_words((_s)) == 1) ?		\
	 0 : ((n) / _BITSET_BITS))
#define __BIT_CLR(_s, n, p)						\
	((p)->__bits[__bitset_word(_s, n)] &= ~__bitset_mask((_s), (n)))
#define __BIT_COPY(_s, f, t)	(void)(*(t) = *(f))
#define __BIT_ISSET(_s, n, p)						\
	((((p)->__bits[__bitset_word(_s, n)] & __bitset_mask((_s), (n))) != 0))
#define __BIT_SET(_s, n, p)						\
	((p)->__bits[__bitset_word(_s, n)] |= __bitset_mask((_s), (n)))
#define __BIT_ZERO(_s, p) do {						\
	size_t __i;							\
	for (__i = 0; __i < __bitset_words((_s)); __i++)		\
		(p)->__bits[__i] = 0L;					\
} while (0)

__BITSET_DEFINE(bitset, 1);
using bitset_view = struct bitset;

#define BIT_CLR(_s, n, p)			__BIT_CLR(_s, n, p)
#define BIT_COPY(_s, f, t)			__BIT_COPY(_s, f, t)
#define BIT_ISSET(_s, n, p)			__BIT_ISSET(_s, n, p)
#define BIT_SET(_s, n, p)			__BIT_SET(_s, n, p)
#define BIT_ZERO(_s, p)				__BIT_ZERO(_s, p)

#define CPU_MAXSIZE	1024
#define CPU_SETSIZE	CPU_MAXSIZE
__BITSET_DEFINE(_cpuset, CPU_SETSIZE);
typedef struct _cpuset cpuset_t;

#define DOMAINSET_MAXSIZE	256
#define DOMAINSET_SETSIZE	DOMAINSET_MAXSIZE
__BITSET_DEFINE(_domainset, DOMAINSET_SETSIZE);
typedef struct _domainset domainset_t;

#define CPU_CLR(n, p)			__BIT_CLR(CPU_SETSIZE, n, p)
#define CPU_ISSET(n, p)			__BIT_ISSET(CPU_SETSIZE, n, p)
#define CPU_SET(n, p)			__BIT_SET(CPU_SETSIZE, n, p)
#define CPU_ZERO(p) 			__BIT_ZERO(CPU_SETSIZE, p)

#define DOMAINSET_CLR(n, p)		__BIT_CLR(DOMAINSET_SETSIZE, n, p)
#define DOMAINSET_COPY(f, t)		__BIT_COPY(DOMAINSET_SETSIZE, f, t)
#define DOMAINSET_ISSET(n, p)		__BIT_ISSET(DOMAINSET_SETSIZE, n, p)
#define DOMAINSET_SET(n, p)		__BIT_SET(DOMAINSET_SETSIZE, n, p)
#define DOMAINSET_ZERO(p) 		__BIT_ZERO(DOMAINSET_SETSIZE, p)

#define CPU_LEVEL_ROOT		1
#define CPU_LEVEL_CPUSET	2
#define CPU_LEVEL_WHICH		3

#define CPU_WHICH_TID		1
#define CPU_WHICH_PID		2
#define CPU_WHICH_CPUSET	3
#define CPU_WHICH_IRQ		4
#define CPU_WHICH_JAIL		5
#define CPU_WHICH_DOMAIN	6

#define DOMAINSET_POLICY_INVALID	0
#define DOMAINSET_POLICY_ROUNDROBIN	1
#define DOMAINSET_POLICY_FIRSTTOUCH	2
#define DOMAINSET_POLICY_PREFER		3
#define DOMAINSET_POLICY_INTERLEAVE	4

#define CPUSET_PARSE_OK			0
#define CPUSET_PARSE_GETAFFINITY	-1
#define CPUSET_PARSE_ERROR		-2
#define CPUSET_PARSE_OUT_OF_RANGE	-3
#define CPUSET_PARSE_GETDOMAIN		-4
#define CPUSET_PARSE_INVALID_CPU	CPUSET_PARSE_OUT_OF_RANGE

typedef int cpulevel_t;
typedef int cpuwhich_t;
typedef int cpusetid_t;
#ifndef _LWPID_T_DECLARED
typedef int lwpid_t;
#define _LWPID_T_DECLARED
#endif

extern "C" {
extern char jail_errmsg[1024];
int cpuset(cpusetid_t *);
int cpuset_setid(cpuwhich_t, id_t, cpusetid_t);
int cpuset_getid(cpulevel_t, cpuwhich_t, id_t, cpusetid_t *);
int cpuset_getaffinity(cpulevel_t, cpuwhich_t, id_t, size_t, cpuset_t *);
int cpuset_setaffinity(cpulevel_t, cpuwhich_t, id_t, size_t, const cpuset_t *);
int cpuset_getdomain(cpulevel_t, cpuwhich_t, id_t, size_t, domainset_t *, int *);
int cpuset_setdomain(cpulevel_t, cpuwhich_t, id_t, size_t, const domainset_t *, int);
int jail_getid(const char *name);
int execvp(const char *file, char *const argv[]);
}

struct numa_policy {
	const char 	*name;
	int		policy;
};

static const struct numa_policy policies[] = {
	{ "round-robin", DOMAINSET_POLICY_ROUNDROBIN },
	{ "rr", DOMAINSET_POLICY_ROUNDROBIN },
	{ "first-touch", DOMAINSET_POLICY_FIRSTTOUCH },
	{ "ft", DOMAINSET_POLICY_FIRSTTOUCH },
	{ "prefer", DOMAINSET_POLICY_PREFER },
	{ "interleave", DOMAINSET_POLICY_INTERLEAVE},
	{ "il", DOMAINSET_POLICY_INTERLEAVE},
	{ NULL, DOMAINSET_POLICY_INVALID }
};

static int
parselist(const char *list, struct bitset *mask, int size)
{
	enum { NONE, NUM, DASH } state;
	int lastnum;
	int curnum;
	const char *l;

	state = NONE;
	curnum = lastnum = 0;
	for (l = list; *l != '\0';) {
		if (isdigit((unsigned char)*l)) {
			curnum = atoi(l);
			if (curnum >= size)
				return (CPUSET_PARSE_INVALID_CPU);
			while (isdigit((unsigned char)*l))
				l++;
			switch (state) {
			case NONE:
				lastnum = curnum;
				state = NUM;
				break;
			case DASH:
				for (; lastnum <= curnum; lastnum++)
					BIT_SET(size, lastnum, mask);
				state = NONE;
				break;
			case NUM:
			default:
				goto parserr;
			}
			continue;
		}
		switch (*l) {
		case ',':
			switch (state) {
			case NONE:
				break;
			case NUM:
				BIT_SET(size, curnum, mask);
				state = NONE;
				break;
			case DASH:
				goto parserr;
				break;
			}
			break;
		case '-':
			if (state != NUM)
				goto parserr;
			state = DASH;
			break;
		default:
			goto parserr;
		}
		l++;
	}
	switch (state) {
		case NONE:
			break;
		case NUM:
			BIT_SET(size, curnum, mask);
			break;
		case DASH:
			goto parserr;
	}
	return (CPUSET_PARSE_OK);
parserr:
	return (CPUSET_PARSE_ERROR);
}

int
domainset_parselist(const char *list, domainset_t *mask, int *policyp)
{
	domainset_t rootmask;
	const struct numa_policy *policy;
	const char *l;
	int p;

	if (cpuset_getdomain(CPU_LEVEL_ROOT, CPU_WHICH_PID, -1,
	    sizeof(rootmask), &rootmask, &p) != 0)
		return (CPUSET_PARSE_GETDOMAIN);

	if (list == NULL || strcasecmp(list, "all") == 0 || *list == '\0') {
		*policyp = p;
		DOMAINSET_COPY(&rootmask, mask);
		return (CPUSET_PARSE_OK);
	}

	l = list;
	for (policy = &policies[0]; policy->name != NULL; policy++) {
		if (strncasecmp(l, policy->name, strlen(policy->name)) == 0) {
			p = policy->policy;
			l += strlen(policy->name);
			if (*l != ':' && *l != '\0')
				return (CPUSET_PARSE_ERROR);
			if (*l == ':')
				l++;
			break;
		}
	}
	*policyp = p;

	return (parselist(l, (struct bitset *)mask, DOMAINSET_SETSIZE));
}

int
cpuset_parselist(const char *list, cpuset_t *mask)
{
	if (strcasecmp(list, "all") == 0) {
		if (cpuset_getaffinity(CPU_LEVEL_ROOT, CPU_WHICH_PID, -1,
		    sizeof(*mask), mask) != 0)
			return (CPUSET_PARSE_GETAFFINITY);
		return (CPUSET_PARSE_OK);
	}

	return (parselist(list, (struct bitset *)mask, CPU_SETSIZE));
}

inline int Cflag;
inline int cflag;
inline int dflag;
inline int gflag;
inline int iflag;
inline int jflag;
inline int lflag;
inline int nflag;
inline int pflag;
inline int rflag;
inline int sflag;
inline int tflag;
inline int xflag;
inline id_t id;
inline cpulevel_t level;
inline cpuwhich_t which;

void usage(void) __dead2;

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2007, 2008 	Jeffrey Roberson <jeff@freebsd.org>
 * All rights reserved.
 *
 * Copyright (c) 2008 Nokia Corporation
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
printset(struct bitset *mask, int size)
{
	int once;
	int bit;

	for (once = 0, bit = 0; bit < size; bit++) {
		if (BIT_ISSET(size, bit, mask)) {
			if (once == 0) {
				printf("%d", bit);
				once = 1;
			} else
				printf(", %d", bit);
		}
	}
	printf("\n");
}

static const char *whichnames[] = { NULL, "tid", "pid", "cpuset", "irq", "jail",
				    "domain" };
static const char *levelnames[] = { NULL, " root", " cpuset", "" };
static const char *policynames[] = { "invalid", "round-robin", "first-touch",
				    "prefer", "interleave" };

void
printaffinity(void)
{
	domainset_t domain;
	cpuset_t mask;
	int policy;

	if (cpuset_getaffinity(level, which, id, sizeof(mask), &mask) != 0)
		err(EXIT_FAILURE, "getaffinity");
	printf("%s %jd%s mask: ", whichnames[which], (intmax_t)id,
	    levelnames[level]);
	printset((struct bitset *)&mask, CPU_SETSIZE);
	if (dflag || xflag)
		goto out;
	if (cpuset_getdomain(level, which, id, sizeof(domain), &domain,
	    &policy) != 0)
		err(EXIT_FAILURE, "getdomain");
	printf("%s %jd%s domain policy: %s mask: ", whichnames[which],
	    (intmax_t)id, levelnames[level], policynames[policy]);
	printset((struct bitset *)&domain, DOMAINSET_SETSIZE);
out:
	exit(EXIT_SUCCESS);
}

void
printsetid(void)
{
	cpusetid_t setid;

	/*
	 * Only LEVEL_WHICH && WHICH_CPUSET has a numbered id.
	 */
	if (level == CPU_LEVEL_WHICH && !sflag)
		level = CPU_LEVEL_CPUSET;
	if (cpuset_getid(level, which, id, &setid))
		err(errno, "getid");
	printf("%s %jd%s id: %d\n", whichnames[which], (intmax_t)id,
	    levelnames[level], setid);
}

int
main(int argc, char *argv[])
{
	domainset_t domains;
	cpusetid_t setid;
	cpuset_t mask;
	int policy;
	lwpid_t tid;
	pid_t pid;
	int ch;

	CPU_ZERO(&mask);
	DOMAINSET_ZERO(&domains);
	policy = DOMAINSET_POLICY_INVALID;
	level = CPU_LEVEL_WHICH;
	which = CPU_WHICH_PID;
	id = pid = tid = setid = -1;
	while ((ch = getopt(argc, argv, "Ccd:gij:l:n:p:rs:t:x:")) != -1) {
		switch (ch) {
		case 'C':
			Cflag = 1;
			break;
		case 'c':
			cflag = 1;
			level = CPU_LEVEL_CPUSET;
			break;
		case 'd':
			dflag = 1;
			which = CPU_WHICH_DOMAIN;
			id = atoi(optarg);
			break;
		case 'g':
			gflag = 1;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'j':
			jflag = 1;
			which = CPU_WHICH_JAIL;
			id = jail_getid(optarg);
			if (id < 0)
				errx(EXIT_FAILURE, "%s", jail_errmsg);
			break;
		case 'l':
			lflag = 1;
			cpuset_parselist(optarg, &mask);
			break;
		case 'n':
			nflag = 1;
			domainset_parselist(optarg, &domains, &policy);
			break;
		case 'p':
			pflag = 1;
			which = CPU_WHICH_PID;
			id = pid = atoi(optarg);
			break;
		case 'r':
			level = CPU_LEVEL_ROOT;
			rflag = 1;
			break;
		case 's':
			sflag = 1;
			which = CPU_WHICH_CPUSET;
			id = setid = atoi(optarg);
			break;
		case 't':
			tflag = 1;
			which = CPU_WHICH_TID;
			id = tid = atoi(optarg);
			break;
		case 'x':
			xflag = 1;
			which = CPU_WHICH_IRQ;
			id = atoi(optarg);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (gflag) {
		if (argc || Cflag || lflag || nflag)
			usage();
		/* Only one identity specifier. */
		if (dflag + jflag + xflag + sflag + pflag + tflag > 1)
			usage();
		if (iflag)
			printsetid();
		else
			printaffinity();
		exit(EXIT_SUCCESS);
	}

	if (dflag || iflag || rflag)
		usage();
	/*
	 * The user wants to run a command with a set and possibly cpumask.
	 */
	if (argc) {
		if (Cflag || pflag || tflag || xflag || jflag)
			usage();
		if (sflag) {
			if (cpuset_setid(CPU_WHICH_PID, -1, setid))
				err(argc, "setid");
		} else {
			if (cpuset(&setid))
				err(argc, "newid");
		}
		if (lflag) {
			if (cpuset_setaffinity(level, CPU_WHICH_PID,
			    -1, sizeof(mask), &mask) != 0)
				err(EXIT_FAILURE, "setaffinity");
		}
		if (nflag) {
			if (cpuset_setdomain(level, CPU_WHICH_PID,
			    -1, sizeof(domains), &domains, policy) != 0)
				err(EXIT_FAILURE, "setdomain");
		}
		errno = 0;
		execvp(*argv, argv);
		err(errno == ENOENT ? 127 : 126, "%s", *argv);
	}
	/*
	 * We're modifying something that presently exists.
	 */
	if (Cflag && (jflag || !pflag || sflag || tflag || xflag))
		usage();
	if ((!lflag && !nflag) && cflag)
		usage();
	if ((!lflag && !nflag) && !(Cflag || sflag))
		usage();
	/* You can only set a mask on a thread. */
	if (tflag && (sflag | pflag | xflag | jflag))
		usage();
	/* You can only set a mask on an irq. */
	if (xflag && (jflag | pflag | sflag | tflag))
		usage();
	if (Cflag) {
		/*
		 * Create a new cpuset and move the specified process
		 * into the set.
		 */
		if (cpuset(&setid) < 0)
			err(EXIT_FAILURE, "newid");
		sflag = 1;
	}
	if (pflag && sflag) {
		if (cpuset_setid(CPU_WHICH_PID, pid, setid))
			err(EXIT_FAILURE, "setid");
		/*
		 * If the user specifies a set and a list we want the mask
		 * to effect the pid and not the set.
		 */
		which = CPU_WHICH_PID;
		id = pid;
	}
	if (lflag) {
		if (cpuset_setaffinity(level, which, id, sizeof(mask),
		    &mask) != 0)
			err(EXIT_FAILURE, "setaffinity");
	}
	if (nflag) {
		if (cpuset_setdomain(level, which, id, sizeof(domains),
		    &domains, policy) != 0)
			err(EXIT_FAILURE, "setdomain");
	}

	exit(EXIT_SUCCESS);
}

void
usage(void)
{

	fprintf(stderr,
    "usage: cpuset [-l cpu-list] [-n policy:domain-list] [-s setid] cmd ...\n");
	fprintf(stderr,
    "       cpuset [-l cpu-list] [-n policy:domain-list] [-s setid] -p pid\n");
	fprintf(stderr,
    "       cpuset [-c] [-l cpu-list] [-n policy:domain-list] -C -p pid\n");
	fprintf(stderr,
    "       cpuset [-c] [-l cpu-list] [-n policy:domain-list]\n"
    "              [-j jailid | -p pid | -t tid | -s setid | -x irq]\n");
	fprintf(stderr,
    "       cpuset -g [-cir]\n"
    "              [-d domain | -j jailid | -p pid | -t tid | -s setid | -x irq]\n");
	exit(1);
}

void
reset_globals(void)
{
	Cflag = 0;
	cflag = 0;
	dflag = 0;
	gflag = 0;
	iflag = 0;
	jflag = 0;
	lflag = 0;
	nflag = 0;
	pflag = 0;
	rflag = 0;
	sflag = 0;
	tflag = 0;
	xflag = 0;
	id = 0;
	level = 0;
	which = 0;
}

} // namespace pbsd::bin_cpuset::b0183
