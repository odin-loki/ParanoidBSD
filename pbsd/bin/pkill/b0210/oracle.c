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
 * Reference oracle for PBSD batch b0210: bin/pkill/pkill.c.
 *
 * Function bodies are reproduced verbatim; only names carry the ref_ prefix.
 * main() is absent for the reason recorded in skipped.txt.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <err.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

/* FreeBSD types and constants reproduced for the ported functions. */
#ifndef MAXCOMLEN
#define	MAXCOMLEN	19
#endif
#ifndef LOGINCLASSLEN
#define	LOGINCLASSLEN	17
#endif

#define	P_KPROC		0x00000001
#define	P_CONTROLT	0x00000002
#define	P_SYSTEM	0x00000004

struct kinfo_proc {
	pid_t		ki_pid;
	uid_t		ki_uid;
	uid_t		ki_ruid;
	gid_t		ki_rgid;
	pid_t		ki_ppid;
	pid_t		ki_pgid;
	dev_t		ki_tdev;
	pid_t		ki_sid;
	int		ki_jid;
	int		ki_flag;
	char		ki_comm[MAXCOMLEN + 1];
	char		ki_loginclass[LOGINCLASSLEN];
	struct timeval	ki_start;
};

typedef struct _kvm	kvm_t;

#define	STATUS_MATCH	0
#define	STATUS_NOMATCH	1
#define	STATUS_BADUSAGE	2
#define	STATUS_ERROR	3

#define	MIN_PID	5
#define	MAX_PID	99999

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

static struct kinfo_proc *plist;
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
static int	cflags = 0;	/* REG_EXTENDED not required for these fns */
static int	quiet;
static kvm_t	*kd;
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

static const char *ref_progname = "pkill";

const char *
getprogname(void)
{

	return (ref_progname);
}

static void	ref_usage(void) __attribute__((__noreturn__));
static int	ref_killact(const struct kinfo_proc *);
static int	ref_grepact(const struct kinfo_proc *);
static void	ref_makelist(struct listhead *, enum listtype, char *);
static int	ref_takepid(const char *, int);

static void
ref_usage(void)
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

static void
ref_show_process(const struct kinfo_proc *kp)
{
	char **argv;

	if (quiet) {
		assert(pgrep);
		return;
	}
	if ((longfmt || !pgrep) && matchargs &&
	    (argv = kvm_getargv(kd, kp, 0)) != NULL) {
		printf("%d ", (int)kp->ki_pid);
		for (; *argv != NULL; argv++) {
			printf("%s", *argv);
			if (argv[1] != NULL)
				putchar(' ');
		}
	} else if (longfmt || !pgrep)
		printf("%d %s", (int)kp->ki_pid, kp->ki_comm);
	else
		printf("%d", (int)kp->ki_pid);
}

static int
ref_killact(const struct kinfo_proc *kp)
{
	int ch, first;

	if (interactive) {
		/*
		 * Be careful, ask before killing.
		 */
		printf("kill ");
		ref_show_process(kp);
		printf("? ");
		fflush(stdout);
		first = ch = getchar();
		while (ch != '\n' && ch != EOF)
			ch = getchar();
		if (first != 'y' && first != 'Y')
			return (1);
	}
	if (kill(kp->ki_pid, signum) == -1) {
		/* 
		 * Check for ESRCH, which indicates that the process
		 * disappeared between us matching it and us
		 * signalling it; don't issue a warning about it.
		 */
		if (errno != ESRCH)
			warn("signalling pid %d", (int)kp->ki_pid);
		/*
		 * Return 0 to indicate that the process should not be
		 * considered a match, since we didn't actually get to
		 * signal it.
		 */
		return (0);
	}
	return (1);
}

static int
ref_grepact(const struct kinfo_proc *kp)
{
	static bool first = true;

	if (!quiet && !first)
		printf("%s", delim);
	ref_show_process(kp);
	first = false;
	return (1);
}

static void
ref_makelist(struct listhead *head, enum listtype type, char *src)
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
			ref_usage();

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
			ref_usage();
		}
	}

	if (empty)
		ref_usage();
}

static int
ref_takepid(const char *pidfile, int pidfilelock)
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

/* ------------------------------------------------------------------ */
/* Harness entry points (do not alter the bodies above).               */
/* ------------------------------------------------------------------ */

void
ref_usage_call(void)
{

	ref_usage();
}

void
ref_show_process_call(const struct kinfo_proc *kp)
{

	ref_show_process(kp);
}

int
ref_killact_call(const struct kinfo_proc *kp)
{

	return (ref_killact(kp));
}

int
ref_grepact_call(const struct kinfo_proc *kp)
{

	return (ref_grepact(kp));
}

void
ref_makelist_call(struct listhead *head, enum listtype type, char *src)
{

	ref_makelist(head, type, src);
}

int
ref_takepid_call(const char *pidfile, int pidfilelock)
{

	return (ref_takepid(pidfile, pidfilelock));
}

void
ref_set_pgrep(int v)
{

	pgrep = v;
}

void
ref_set_quiet(int v)
{

	quiet = v;
}

void
ref_set_longfmt(int v)
{

	longfmt = v;
}

void
ref_set_matchargs(int v)
{

	matchargs = v;
}

void
ref_set_interactive(int v)
{

	interactive = v;
}

void
ref_set_signum(int v)
{

	signum = v;
}

void
ref_set_kd(kvm_t *k)
{

	kd = k;
}

void
ref_set_mypid(pid_t p)
{

	mypid = p;
}

void
ref_set_delim(const char *d)
{

	delim = d;
}

void
ref_set_progname(const char *p)
{

	ref_progname = p;
}

void
ref_list_clear(struct listhead *head)
{
	struct list *li, *nli;

	SLIST_FOREACH_SAFE(li, head, li_chain, nli) {
		if (li->li_name != NULL)
			free(li->li_name);
		free(li);
	}
	SLIST_INIT(head);
}

int
ref_list_count(struct listhead *head)
{
	struct list *li;
	int n;

	n = 0;
	SLIST_FOREACH(li, head, li_chain)
		n++;
	return (n);
}

long
ref_list_number(struct listhead *head, int idx)
{
	struct list *li;
	int n;

	n = 0;
	SLIST_FOREACH(li, head, li_chain) {
		if (n == idx)
			return (li->li_number);
		n++;
	}
	return (0);
}

const char *
ref_list_name(struct listhead *head, int idx)
{
	struct list *li;
	int n;

	n = 0;
	SLIST_FOREACH(li, head, li_chain) {
		if (n == idx)
			return (li->li_name);
		n++;
	}
	return (NULL);
}

struct listhead *
ref_ppidlist(void)
{

	return (&ppidlist);
}

struct listhead *
ref_ruidlist(void)
{

	return (&ruidlist);
}

struct listhead *
ref_rgidlist(void)
{

	return (&rgidlist);
}

struct listhead *
ref_pgrplist(void)
{

	return (&pgrplist);
}

struct listhead *
ref_tdevlist(void)
{

	return (&tdevlist);
}

struct listhead *
ref_sidlist(void)
{

	return (&sidlist);
}

struct listhead *
ref_jidlist(void)
{

	return (&jidlist);
}

struct listhead *
ref_classlist(void)
{

	return (&classlist);
}

struct listhead *
ref_euidlist(void)
{

	return (&euidlist);
}
