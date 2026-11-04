/*	$NetBSD: pkill.c,v 1.16 2005/10/10 22:13:20 kleink Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
 * Copyright (c) 2005 Pawel Jakub Dawidek <pjd@FreeBSD.org>
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andrew Doran.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * ORACLE for PBSD batch b0210 -- bin/pkill.
 *
 * The function bodies below are byte-for-byte the HardenedBSD originals.
 * Renaming to the ref_ namespace is done with object-like macros so that not
 * a single character inside any body had to be touched.  Everything above the
 * "ORIGINAL SOURCE BEGINS HERE" marker is added scaffolding: header shims for
 * facilities this host lacks (sys/queue.h, getprogname(), jail_getid()) and a
 * setjmp/longjmp trap for exit()/err()/errx() so the differential harness can
 * observe the exit status and the diagnostic text of the error paths without
 * tearing down the process.
 */

/*
 * -std=c11 hides strsep(3), strdup(3), getsid(2), fileno(3) and S_IFCHR
 * behind the feature-test macros; the BSD original compiles with them all
 * visible.  The C++ front end enables _GNU_SOURCE by default, so this keeps
 * the two translation units on identical declarations.
 */
#define	_GNU_SOURCE	1

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h>

#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <paths.h>
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/* <sys/queue.h> SLIST subset (this host has no <sys/queue.h>).		*/
/* ------------------------------------------------------------------ */
#define	SLIST_HEAD(name, type)						\
struct name {								\
	struct type *slh_first;						\
}

#define	SLIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define	SLIST_ENTRY(type)						\
struct {								\
	struct type *sle_next;						\
}

#define	SLIST_FIRST(head)	((head)->slh_first)
#define	SLIST_NEXT(elm, field)	((elm)->field.sle_next)

#define	SLIST_INSERT_HEAD(head, elm, field) do {			\
	SLIST_NEXT((elm), field) = SLIST_FIRST((head));			\
	SLIST_FIRST((head)) = (elm);					\
} while (0)

#define	SLIST_FOREACH(var, head, field)					\
	for ((var) = SLIST_FIRST((head));				\
	    (var);							\
	    (var) = SLIST_NEXT((var), field))

/* ------------------------------------------------------------------ */
/* BSD facilities missing on this host.				*/
/* ------------------------------------------------------------------ */
#ifndef	LONG_BIT
#define	LONG_BIT	(CHAR_BIT * (int)sizeof(long))
#endif

static const char *
ref_getprogname(void)
{

	return ("pkill");
}
#define	getprogname	ref_getprogname

/*
 * jail(3) is not available here.  A host with no jails makes jail_getid()
 * fail for every argument, which is exactly what this returns.  Both the
 * oracle and the port are linked against this same behaviour, so the
 * differential comparison stays meaningful.
 */
static int
ref_jail_getid(const char *name)
{

	(void)name;
	errno = EINVAL;
	return (-1);
}
#define	jail_getid	ref_jail_getid

/* ------------------------------------------------------------------ */
/* exit()/err()/errx()/fprintf() trap.					*/
/* ------------------------------------------------------------------ */
jmp_buf	ref_trap_env;
static int	ref_trap_code;
static char	ref_trap_msg[16384];
static size_t	ref_trap_len;

static void
ref_trap_reset(void)
{

	ref_trap_code = 0;
	ref_trap_len = 0;
	ref_trap_msg[0] = '\0';
}

static void
ref_trap_vappend(const char *fmt, va_list ap)
{
	int n;

	if (ref_trap_len >= sizeof(ref_trap_msg) - 1)
		return;
	n = vsnprintf(ref_trap_msg + ref_trap_len,
	    sizeof(ref_trap_msg) - ref_trap_len, fmt, ap);
	if (n < 0)
		return;
	ref_trap_len += (size_t)n;
	if (ref_trap_len >= sizeof(ref_trap_msg))
		ref_trap_len = sizeof(ref_trap_msg) - 1;
}

static void
ref_trap_appendf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	ref_trap_vappend(fmt, ap);
	va_end(ap);
}

static void ref_trap_exit(int) __attribute__((__noreturn__));

static void
ref_trap_exit(int code)
{

	ref_trap_code = code;
	longjmp(ref_trap_env, 1);
}

static void ref_trap_errx(int, const char *, ...) __attribute__((__noreturn__));

static void
ref_trap_errx(int code, const char *fmt, ...)
{
	va_list ap;

	ref_trap_appendf("%s: ", ref_getprogname());
	va_start(ap, fmt);
	ref_trap_vappend(fmt, ap);
	va_end(ap);
	ref_trap_appendf("\n");
	ref_trap_exit(code);
}

static void ref_trap_err(int, const char *, ...) __attribute__((__noreturn__));

static void
ref_trap_err(int code, const char *fmt, ...)
{
	va_list ap;
	int sverrno;

	sverrno = errno;
	ref_trap_appendf("%s: ", ref_getprogname());
	va_start(ap, fmt);
	ref_trap_vappend(fmt, ap);
	va_end(ap);
	ref_trap_appendf(": %s\n", strerror(sverrno));
	ref_trap_exit(code);
}

static int
ref_trap_fprintf(FILE *stream, const char *fmt, ...)
{
	va_list ap;
	size_t before;

	if (stream != stderr && stream != stdout) {
		int n;

		va_start(ap, fmt);
		n = vfprintf(stream, fmt, ap);
		va_end(ap);
		return (n);
	}
	before = ref_trap_len;
	va_start(ap, fmt);
	ref_trap_vappend(fmt, ap);
	va_end(ap);
	return ((int)(ref_trap_len - before));
}

#define	exit(code)	ref_trap_exit(code)
#define	err		ref_trap_err
#define	errx		ref_trap_errx
#define	fprintf		ref_trap_fprintf

/* ------------------------------------------------------------------ */
/* ref_ renaming.  Object-like macros, so the bodies stay untouched.	*/
/* ------------------------------------------------------------------ */
#define	usage		ref_usage
#define	makelist	ref_makelist
#define	takepid		ref_takepid

/* ============ ORIGINAL SOURCE BEGINS HERE ============ */

#define	STATUS_MATCH	0
#define	STATUS_NOMATCH	1
#define	STATUS_BADUSAGE	2
#define	STATUS_ERROR	3

#define	MIN_PID	5
#define	MAX_PID	99999

/* Ignore system-processes (if '-S' flag is not specified) and myself. */
#define	PSKIP(kp)	((kp)->ki_pid == mypid ||			\
			 (!kthreads && ((kp)->ki_flag & P_KPROC) != 0))

enum listtype {
	LT_GENERIC,
	LT_USER,
	LT_GROUP,
	LT_TTY,
	LT_PGRP,
	LT_JAIL,
	LT_SID,
	LT_CLASS
};

struct list {
	SLIST_ENTRY(list) li_chain;
	long	li_number;
	char	*li_name;
};

SLIST_HEAD(listhead, list);

static char	*selected;
static const char *delim = "\n";
static int	nproc;
static int	pgrep;
static int	signum = SIGTERM;
static int	newest;
static int	oldest;
static int	interactive;
static int	inverse;
static int	longfmt;
static int	matchargs;
static int	fullmatch;
static int	kthreads;
static int	quiet;
static pid_t	mypid;

static struct listhead euidlist = SLIST_HEAD_INITIALIZER(euidlist);
static struct listhead ruidlist = SLIST_HEAD_INITIALIZER(ruidlist);
static struct listhead rgidlist = SLIST_HEAD_INITIALIZER(rgidlist);
static struct listhead pgrplist = SLIST_HEAD_INITIALIZER(pgrplist);
static struct listhead ppidlist = SLIST_HEAD_INITIALIZER(ppidlist);
static struct listhead tdevlist = SLIST_HEAD_INITIALIZER(tdevlist);
static struct listhead sidlist = SLIST_HEAD_INITIALIZER(sidlist);
static struct listhead jidlist = SLIST_HEAD_INITIALIZER(jidlist);
static struct listhead classlist = SLIST_HEAD_INITIALIZER(classlist);

void	usage(void) __attribute__((__noreturn__));
void	makelist(struct listhead *, enum listtype, char *);
int	takepid(const char *, int);

void
usage(void)
{
	const char *ustr;

	if (pgrep)
		ustr = "[-LSfilnoqvx] [-d delim]";
	else
		ustr = "[-signal] [-ILfilnovx]";

	fprintf(stderr,
		"usage: %s %s [-F pidfile] [-G gid] [-M core] [-N system]\n"
		"             [-P ppid] [-U uid] [-c class] [-g pgrp] [-j jail]\n"
		"             [-s sid] [-t tty] [-u euid] pattern ...\n",
		getprogname(), ustr);

	exit(STATUS_BADUSAGE);
}

void
makelist(struct listhead *head, enum listtype type, char *src)
{
	struct list *li;
	struct passwd *pw;
	struct group *gr;
	struct stat st;
	const char *cp;
	char *sp, *ep, buf[MAXPATHLEN];
	int empty;

	empty = 1;

	while ((sp = strsep(&src, ",")) != NULL) {
		if (*sp == '\0')
			usage();

		if ((li = malloc(sizeof(*li))) == NULL) {
			err(STATUS_ERROR, "Cannot allocate %zu bytes",
			    sizeof(*li));
		}

		SLIST_INSERT_HEAD(head, li, li_chain);
		empty = 0;

		if (type != LT_CLASS)
			li->li_number = (uid_t)strtol(sp, &ep, 0);

		if (type != LT_CLASS && *ep == '\0') {
			switch (type) {
			case LT_PGRP:
				if (li->li_number == 0)
					li->li_number = getpgrp();
				break;
			case LT_SID:
				if (li->li_number == 0)
					li->li_number = getsid(mypid);
				break;
			case LT_JAIL:
				if (li->li_number < 0)
					errx(STATUS_BADUSAGE,
					     "Negative jail ID `%s'", sp);
				/* For compatibility with old -j */
				if (li->li_number == 0)
					li->li_number = -1;	/* any jail */
				break;
			case LT_TTY:
				if (li->li_number < 0)
					errx(STATUS_BADUSAGE,
					     "Negative /dev/pts tty `%s'", sp);
				snprintf(buf, sizeof(buf), _PATH_DEV "pts/%s",
				    sp);
				if (stat(buf, &st) != -1)
					goto foundtty;
				if (errno == ENOENT)
					errx(STATUS_BADUSAGE, "No such tty: `"
					    _PATH_DEV "pts/%s'", sp);
				err(STATUS_ERROR, "Cannot access `"
				    _PATH_DEV "pts/%s'", sp);
				break;
			default:
				break;
			}
			continue;
		}

		switch (type) {
		case LT_USER:
			if ((pw = getpwnam(sp)) == NULL)
				errx(STATUS_BADUSAGE, "Unknown user `%s'", sp);
			li->li_number = pw->pw_uid;
			break;
		case LT_GROUP:
			if ((gr = getgrnam(sp)) == NULL)
				errx(STATUS_BADUSAGE, "Unknown group `%s'", sp);
			li->li_number = gr->gr_gid;
			break;
		case LT_TTY:
			if (strcmp(sp, "-") == 0) {
				li->li_number = -1;
				break;
			} else if (strcmp(sp, "co") == 0) {
				cp = "console";
			} else {
				cp = sp;
			}

			snprintf(buf, sizeof(buf), _PATH_DEV "%s", cp);
			if (stat(buf, &st) != -1)
				goto foundtty;

			snprintf(buf, sizeof(buf), _PATH_DEV "tty%s", cp);
			if (stat(buf, &st) != -1)
				goto foundtty;

			if (errno == ENOENT)
				errx(STATUS_BADUSAGE, "No such tty: `%s'", sp);
			err(STATUS_ERROR, "Cannot access `%s'", sp);

foundtty:		if ((st.st_mode & S_IFCHR) == 0)
				errx(STATUS_BADUSAGE, "Not a tty: `%s'", sp);

			li->li_number = st.st_rdev;
			break;
		case LT_JAIL: {
			int jid;

			if (strcmp(sp, "none") == 0)
				li->li_number = 0;
			else if (strcmp(sp, "any") == 0)
				li->li_number = -1;
			else if ((jid = jail_getid(sp)) != -1)
				li->li_number = jid;
			else if (*ep != '\0')
				errx(STATUS_BADUSAGE,
				     "Invalid jail ID or name `%s'", sp);
			break;
		}
		case LT_CLASS:
			li->li_number = -1;
			li->li_name = strdup(sp);
			if (li->li_name == NULL)
				err(STATUS_ERROR, "Cannot allocate memory");
			break;
		default:
			usage();
		}
	}

	if (empty)
		usage();
}

int
takepid(const char *pidfile, int pidfilelock)
{
	char *endp, line[BUFSIZ];
	FILE *fh;
	long rval;

	fh = fopen(pidfile, "r");
	if (fh == NULL)
		err(STATUS_ERROR, "Cannot open pidfile `%s'", pidfile);

	if (pidfilelock) {
		/*
		 * If we can lock pidfile, this means that daemon is not
		 * running, so would be better not to kill some random process.
		 */
		if (flock(fileno(fh), LOCK_EX | LOCK_NB) == 0) {
			(void)fclose(fh);
			errx(STATUS_ERROR, "File '%s' can be locked", pidfile);
		} else {
			if (errno != EWOULDBLOCK) {
				errx(STATUS_ERROR,
				    "Error while locking file '%s'", pidfile);
			}
		}
	}

	if (fgets(line, sizeof(line), fh) == NULL) {
		if (feof(fh)) {
			(void)fclose(fh);
			errx(STATUS_ERROR, "Pidfile `%s' is empty", pidfile);
		}
		(void)fclose(fh);
		err(STATUS_ERROR, "Cannot read from pid file `%s'", pidfile);
	}
	(void)fclose(fh);

	rval = strtol(line, &endp, 10);
	if (*endp != '\0' && !isspace((unsigned char)*endp))
		errx(STATUS_ERROR, "Invalid pid in file `%s'", pidfile);
	else if (rval < MIN_PID || rval > MAX_PID)
		errx(STATUS_ERROR, "Invalid pid in file `%s'", pidfile);
	return (rval);
}

/* ============ ORIGINAL SOURCE ENDS HERE ============ */

/*
 * Harness entry points.  Each installs the longjmp target, runs the original
 * function and reports -1 when the function returned normally or the exit
 * status the original would have handed to exit(3) otherwise.
 */
int
ref_call_usage(void)
{

	ref_trap_reset();
	if (setjmp(ref_trap_env) == 0) {
		ref_usage();
		return (-1);
	}
	return (ref_trap_code);
}

int
ref_call_makelist(struct listhead *head, int type, char *src)
{

	ref_trap_reset();
	if (setjmp(ref_trap_env) == 0) {
		ref_makelist(head, (enum listtype)type, src);
		return (-1);
	}
	return (ref_trap_code);
}

int
ref_call_takepid(const char *pidfile, int pidfilelock, int *out)
{

	ref_trap_reset();
	if (setjmp(ref_trap_env) == 0) {
		*out = ref_takepid(pidfile, pidfilelock);
		return (-1);
	}
	return (ref_trap_code);
}

const char *
ref_trap_message(void)
{

	return (ref_trap_msg);
}

void
ref_set_pgrep(int v)
{

	pgrep = v;
}

void
ref_set_mypid(pid_t v)
{

	mypid = v;
}
