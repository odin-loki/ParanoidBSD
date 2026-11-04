module;

#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#if defined(__has_include)
#if __has_include(<sysexits.h>)
#include <sysexits.h>
#else
#define EX_OK		0
#define EX_USAGE	64
#define EX_DATAERR	65
#define EX_OSERR	72
#endif
#else
#include <sysexits.h>
#endif

#if defined(__has_include)
#if __has_include(<sys/event.h>)
#include <sys/event.h>
#else
#define PWAIT_NEED_EVENT_STUB
#endif
#else
#define PWAIT_NEED_EVENT_STUB
#endif

#if defined(PWAIT_NEED_EVENT_STUB)
struct kevent {
	uintptr_t ident;
	short filter;
	unsigned short flags;
	unsigned int fflags;
	intptr_t data;
	void *udata;
};
#define EVFILT_PROC	(-5)
#define EVFILT_SIGNAL	(-6)
#define EV_ADD		0x0001
#define NOTE_EXIT	0x80000000U
#define EV_SET(kevp, a, b, c, d, e, f) do {				\
	struct kevent *_kev = (kevp);					\
	_kev->ident = (uintptr_t)(a);					\
	_kev->filter = (short)(b);					\
	_kev->flags = (unsigned short)(c);				\
	_kev->fflags = (unsigned int)(d);				\
	_kev->data = (intptr_t)(e);					\
	_kev->udata = (f);						\
} while (0)
int kqueue(void);
int kevent(int, const struct kevent *, int, struct kevent *, int,
    const struct timespec *);
#endif

#if defined(__has_include)
#if __has_include(<sys/sysctl.h>)
#include <sys/sysctl.h>
#else
int sysctlbyname(const char *, void *, size_t *, void *, size_t);
#endif
#else
int sysctlbyname(const char *, void *, size_t *, void *, size_t);
#endif

#include <sys/tree.h>

export module pbsd.bin.pwait.b0181;

export namespace pbsd::bin_pwait::b0181 {

/*-
 * Copyright (c) 2004-2009, Jilles Tjoelker
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the
 * following conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 * 2. Redistributions in binary form must reproduce the
 *    above copyright notice, this list of conditions and
 *    the following disclaimer in the documentation and/or
 *    other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

struct pid {
	RB_ENTRY(pid) entry;
	pid_t pid;
};

int
pidcmp(const struct pid *a, const struct pid *b)
{
	return (a->pid > b->pid ? 1 : a->pid < b->pid ? -1 : 0);
}

RB_HEAD(pidtree, pid);
static struct pidtree pids = RB_INITIALIZER(&pids);
RB_GENERATE_STATIC(pidtree, pid, entry, pidcmp);

void
usage(void)
{
	fprintf(stderr, "usage: pwait [-t timeout] [-opv] pid ...\n");
	exit(EX_USAGE);
}

/*
 * pwait - wait for processes to terminate
 */
int
main(int argc, char *argv[])
{
	struct itimerval itv;
	struct kevent *e;
	struct pid k, *p;
	char *end, *s;
	double timeout;
	size_t sz;
	long pid;
	pid_t mypid;
	int i, kq, n, ndone, nleft, opt, pid_max, ret, status;
	bool oflag, pflag, tflag, verbose;

	oflag = false;
	pflag = false;
	tflag = false;
	verbose = false;
	memset(&itv, 0, sizeof(itv));

	while ((opt = getopt(argc, argv, "opt:v")) != -1) {
		switch (opt) {
		case 'o':
			oflag = true;
			break;
		case 'p':
			pflag = true;
			break;
		case 't':
			tflag = true;
			errno = 0;
			timeout = strtod(optarg, &end);
			if (end == optarg || errno == ERANGE || timeout < 0) {
				errx(EX_DATAERR, "timeout value");
			}
			switch (*end) {
			case '\0':
				break;
			case 's':
				end++;
				break;
			case 'h':
				timeout *= 60;
				/* FALLTHROUGH */
			case 'm':
				timeout *= 60;
				end++;
				break;
			default:
				errx(EX_DATAERR, "timeout unit");
			}
			if (*end != '\0') {
				errx(EX_DATAERR, "timeout unit");
			}
			if (timeout > 100000000L) {
				errx(EX_DATAERR, "timeout value");
			}
			itv.it_value.tv_sec = (time_t)timeout;
			timeout -= (time_t)timeout;
			itv.it_value.tv_usec =
			    (suseconds_t)(timeout * 1000000UL);
			break;
		case 'v':
			verbose = true;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		usage();
	}

	if ((kq = kqueue()) < 0)
		err(EX_OSERR, "kqueue");

	sz = sizeof(pid_max);
	if (sysctlbyname("kern.pid_max", &pid_max, &sz, NULL, 0) != 0) {
		pid_max = 99999;
	}
	if ((e = (struct kevent *)malloc((argc + tflag) * sizeof(*e))) == NULL) {
		err(EX_OSERR, "malloc");
	}
	ndone = nleft = 0;
	mypid = getpid();
	for (n = 0; n < argc; n++) {
		s = argv[n];
		/* Undocumented Solaris compat */
		if (strncmp(s, "/proc/", 6) == 0) {
			s += 6;
		}
		errno = 0;
		pid = strtol(s, &end, 10);
		if (pid < 0 || pid > pid_max || *end != '\0' || errno != 0) {
			warnx("%s: bad process id", s);
			continue;
		}
		if (pid == mypid) {
			warnx("%s: skipping my own pid", s);
			continue;
		}
		if ((p = (struct pid *)malloc(sizeof(*p))) == NULL) {
			err(EX_OSERR, NULL);
		}
		p->pid = pid;
		if (RB_INSERT(pidtree, &pids, p) != NULL) {
			/* Duplicate. */
			free(p);
			continue;
		}
		EV_SET(e + nleft, pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, NULL);
		if (kevent(kq, e + nleft, 1, NULL, 0, NULL) == -1) {
			if (errno != ESRCH)
				err(EX_OSERR, "kevent()");
			warn("%ld", pid);
			RB_REMOVE(pidtree, &pids, p);
			free(p);
			ndone++;
		} else {
			nleft++;
		}
	}

	if ((ndone == 0 || !oflag) && nleft > 0 && tflag) {
		/*
		 * Explicitly detect SIGALRM so that an exit status of 124
		 * can be returned rather than 142.
		 */
		EV_SET(e + nleft, SIGALRM, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
		if (kevent(kq, e + nleft, 1, NULL, 0, NULL) == -1) {
			err(EX_OSERR, "kevent");
		}
		/* Ignore SIGALRM to not interrupt kevent(2). */
		signal(SIGALRM, SIG_IGN);
		if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
			err(EX_OSERR, "setitimer");
		}
	}
	ret = EX_OK;
	while ((ndone == 0 || !oflag) && ret == EX_OK && nleft > 0) {
		n = kevent(kq, NULL, 0, e, nleft + tflag, NULL);
		if (n == -1) {
			err(EX_OSERR, "kevent");
		}
		for (i = 0; i < n; i++) {
			if (e[i].filter == EVFILT_SIGNAL) {
				if (verbose) {
					printf("timeout\n");
				}
				ret = 124;
			}
			pid = e[i].ident;
			if (verbose) {
				status = e[i].data;
				if (WIFEXITED(status)) {
					printf("%ld: exited with status %d.\n",
					    pid, WEXITSTATUS(status));
				} else if (WIFSIGNALED(status)) {
					printf("%ld: killed by signal %d.\n",
					    pid, WTERMSIG(status));
				} else {
					printf("%ld: terminated.\n", pid);
				}
			}
			k.pid = pid;
			if ((p = RB_FIND(pidtree, &pids, &k)) != NULL) {
				RB_REMOVE(pidtree, &pids, p);
				free(p);
				ndone++;
			}
			--nleft;
		}
	}
	if (pflag) {
		RB_FOREACH(p, pidtree, &pids) {
			printf("%d\n", p->pid);
		}
	}
	exit(ret);
}

} // namespace pbsd::bin_pwait::b0181
