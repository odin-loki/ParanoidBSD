/*
 * oracle.c -- PBSD batch b0335 reference (specification) implementation.
 *
 * Original C sources concatenated with every function renamed ref_*.
 * Bodies are otherwise unmodified.
 */

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#ifndef LONG_BIT
#define LONG_BIT (8 * (int)sizeof(long))
#endif

#include <sys/socket.h>
#if defined(__linux__)
#define _SYS_UN_H 1
struct sockaddr_un {
	unsigned char sun_len;
	sa_family_t sun_family;
	char sun_path[108];
};
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <fnmatch.h>
#include <limits.h>
#include <wchar.h>
#include <wctype.h>

#define __weak_reference(sym, alias) \
    __asm__(".weak " #alias); \
    __asm__(".equ " #alias ", " #sym)
#define __CONCAT(x,y) x ## y
#define __unused __attribute__((__unused__))
#define __used __attribute__((__used__))

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

/* collate.h minimal */
#define COLLATE_STR_LEN 24
#ifndef CHARCLASS_NAME_MAX
#define CHARCLASS_NAME_MAX 14
#endif

struct xlocale_refcounted { long retain_count; void (*destructor)(void *); };
struct xlocale_component {
    struct xlocale_refcounted base;
    char locale[32];
};
enum { XLC_COLLATE = 0, XLC_CTYPE, XLC_MONETARY, XLC_NUMERIC, XLC_TIME, XLC_MESSAGES, XLC_LAST };
struct _xlocale { struct xlocale_refcounted header; struct xlocale_component *components[XLC_LAST]; };
typedef struct _xlocale *locale_t;
struct xlocale_collate {
    struct xlocale_component header;
    int __collate_load_error;
};
locale_t __get_locale(void);
size_t __collate_collating_symbol(wchar_t *, size_t, const char *, size_t, mbstate_t *);
int __collate_equiv_class(const char *, size_t, mbstate_t *);
ssize_t __collate_equiv_match(int, const wchar_t *, size_t, wchar_t, const char *, size_t, mbstate_t *, size_t *);
int __wcollate_range_cmp(wchar_t, wchar_t);

/* libc_private.h minimal for pthread stubs */
typedef void *(*pthread_func_t)(void);
typedef pthread_func_t pthread_func_entry_t[2];

enum {
    PJT_ATFORK,
    PJT_ATTR_DESTROY,
    PJT_ATTR_GETDETACHSTATE,
    PJT_ATTR_GETGUARDSIZE,
    PJT_ATTR_GETINHERITSCHED,
    PJT_ATTR_GETSCHEDPARAM,
    PJT_ATTR_GETSCHEDPOLICY,
    PJT_ATTR_GETSCOPE,
    PJT_ATTR_GETSTACKADDR,
    PJT_ATTR_GETSTACKSIZE,
    PJT_ATTR_INIT,
    PJT_ATTR_SETDETACHSTATE,
    PJT_ATTR_SETGUARDSIZE,
    PJT_ATTR_SETINHERITSCHED,
    PJT_ATTR_SETSCHEDPARAM,
    PJT_ATTR_SETSCHEDPOLICY,
    PJT_ATTR_SETSCOPE,
    PJT_ATTR_SETSTACKADDR,
    PJT_ATTR_SETSTACKSIZE,
    PJT_CANCEL,
    PJT_CLEANUP_POP,
    PJT_CLEANUP_PUSH,
    PJT_COND_BROADCAST,
    PJT_COND_DESTROY,
    PJT_COND_INIT,
    PJT_COND_SIGNAL,
    PJT_COND_TIMEDWAIT,
    PJT_COND_WAIT,
    PJT_DETACH,
    PJT_EQUAL,
    PJT_EXIT,
    PJT_GETSPECIFIC,
    PJT_JOIN,
    PJT_KEY_CREATE,
    PJT_KEY_DELETE,
    PJT_KILL,
    PJT_MAIN_NP,
    PJT_MUTEXATTR_DESTROY,
    PJT_MUTEXATTR_INIT,
    PJT_MUTEXATTR_SETTYPE,
    PJT_MUTEX_DESTROY,
    PJT_MUTEX_INIT,
    PJT_MUTEX_LOCK,
    PJT_MUTEX_TRYLOCK,
    PJT_MUTEX_UNLOCK,
    PJT_ONCE,
    PJT_RWLOCK_DESTROY,
    PJT_RWLOCK_INIT,
    PJT_RWLOCK_RDLOCK,
    PJT_RWLOCK_TRYRDLOCK,
    PJT_RWLOCK_TRYWRLOCK,
    PJT_RWLOCK_UNLOCK,
    PJT_RWLOCK_WRLOCK,
    PJT_SELF,
    PJT_SETCANCELSTATE,
    PJT_SETCANCELTYPE,
    PJT_SETSPECIFIC,
    PJT_SIGMASK,
    PJT_TESTCANCEL,
    PJT_CLEANUP_POP_IMP,
    PJT_CLEANUP_PUSH_IMP,
    PJT_CANCEL_ENTER,
    PJT_CANCEL_LEAVE,
    PJT_MUTEX_CONSISTENT,
    PJT_MUTEXATTR_GETROBUST,
    PJT_MUTEXATTR_SETROBUST,
    PJT_GETTHREADID_NP,
    PJT_ATTR_GET_NP,
    PJT_GETNAME_NP,
    PJT_SUSPEND_ALL_NP,
    PJT_RESUME_ALL_NP,
    PJT_MAX
};

extern pthread_func_entry_t __thr_jtable[PJT_MAX];
extern int __isthreaded;
extern const char *_getprogname(void);
extern int _pthread_mutex_lock(pthread_mutex_t *);
extern int _pthread_mutex_unlock(pthread_mutex_t *);
extern int _writev(int, const struct iovec *, int);
extern int _open(const char *, int, ...);
extern int _close(int);
extern int _socket(int, int, int);
extern int _connect(int, const struct sockaddr *, socklen_t);
extern int _getsockopt(int, int, int, void *, socklen_t *);
extern int _setsockopt(int, int, int, const void *, socklen_t);
extern ssize_t send(int, const void *, size_t, int);
extern FILE *fwopen(void *, int (*)(void *, const char *, int));

#ifndef strlcpy
static size_t strlcpy(char *dst, const char *src, size_t siz) {
    size_t n = strlen(src);
    if (siz) { size_t c = n < siz - 1 ? n : siz - 1; memcpy(dst, src, c); dst[c] = 0; }
    return n;
}
#endif


/* ===== syslog.c ===== */
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1988, 1993
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





/* Maximum number of characters of ref_syslog message */
#define	MAXLINE		8192

static int	LogFile = -1;		/* fd for log */
static bool	connected;		/* have done connect */
static int	opened;			/* have done ref_openlog() */
static int	LogStat = 0;		/* status bits, set by ref_openlog() */
static pid_t	LogPid = -1;		/* process id to tag the entry with */
static const char *LogTag = NULL;	/* string to tag the entry with */
static int	LogTagLength = -1;	/* usable part of LogTag */
static int	LogFacility = LOG_USER;	/* default facility code */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
static pthread_mutex_t	syslog_mutex = PTHREAD_MUTEX_INITIALIZER;

#define	THREAD_LOCK()							\
	do { 								\
		if (__isthreaded) _pthread_mutex_lock(&syslog_mutex);	\
	} while(0)
#define	THREAD_UNLOCK()							\
	do {								\
		if (__isthreaded) _pthread_mutex_unlock(&syslog_mutex);	\
	} while(0)

/* RFC5424 defined value. */
#define NILVALUE "-"

static void	ref_disconnectlog(void); /* disconnect from syslogd */
static void	ref_connectlog(void);	/* (re)connect to syslogd */
static void	ref_openlog_unlocked(const char *, int, int);
static void	ref_parse_tag(void);	/* parse ident[NNN] if needed */

/*
 * Format of the magic cookie passed through the stdio hook
 */
struct bufcookie {
	char	*base;	/* start of buffer */
	int	left;
};

/*
 * stdio write hook for writing to a static string buffer
 * XXX: Maybe one day, dynamically allocate it so that the line length
 *      is `unlimited'.
 */
static int
ref_writehook(void *cookie, const char *buf, int len)
{
	struct bufcookie *h;	/* private `handle' */

	h = (struct bufcookie *)cookie;
	if (len > h->left) {
		/* clip in case of wraparound */
		len = h->left;
	}
	if (len > 0) {
		(void)memcpy(h->base, buf, len); /* `write' it. */
		h->base += len;
		h->left -= len;
	}
	return len;
}

/*
 * ref_syslog, ref_vsyslog --
 *	print message on log file; output is intended for syslogd(8).
 */
void
ref_syslog(int pri, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	ref_vsyslog(pri, fmt, ap);
	va_end(ap);
}

static void
ref_vsyslog1(int pri, const char *fmt, va_list ap)
{
	struct timeval now;
	struct tm tm;
	char ch, *p;
	long tz_offset;
	int cnt, fd, saved_errno;
	char hostname[MAXHOSTNAMELEN], *stdp, tbuf[MAXLINE], fmt_cpy[MAXLINE],
	    errstr[64], tz_sign;
	FILE *fp, *fmt_fp;
	struct bufcookie tbuf_cookie;
	struct bufcookie fmt_cookie;

#define	INTERNALLOG	LOG_ERR|LOG_CONS|LOG_PERROR|LOG_PID
	/* Check for invalid bits. */
	if (pri & ~(LOG_PRIMASK|LOG_FACMASK)) {
		ref_syslog(INTERNALLOG,
		    "ref_syslog: unknown facility/priority: %x", pri);
		pri &= LOG_PRIMASK|LOG_FACMASK;
	}

	saved_errno = errno;

	/* Check priority against ref_setlogmask values. */
	if (!(LOG_MASK(LOG_PRI(pri)) & LogMask))
		return;

	/* Set default facility if none specified. */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* Create the primary stdio hook */
	tbuf_cookie.base = tbuf;
	tbuf_cookie.left = sizeof(tbuf);
	fp = fwopen(&tbuf_cookie, ref_writehook);
	if (fp == NULL)
		return;

	/* Build the message according to RFC 5424. Tag and version. */
	(void)fprintf(fp, "<%d>1 ", pri);
	/* Timestamp similar to RFC 3339. */
	if (gettimeofday(&now, NULL) == 0 &&
	    localtime_r(&now.tv_sec, &tm) != NULL) {
		if (tm.tm_gmtoff < 0) {
			tz_sign = '-';
			tz_offset = -tm.tm_gmtoff;
		} else {
			tz_sign = '+';
			tz_offset = tm.tm_gmtoff;
		}

		(void)fprintf(fp,
		    "%04d-%02d-%02d"		/* Date. */
		    "T%02d:%02d:%02d.%06ld"	/* Time. */
		    "%c%02ld:%02ld ",		/* Time zone offset. */
		    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		    tm.tm_hour, tm.tm_min, tm.tm_sec, now.tv_usec,
		    tz_sign, tz_offset / 3600, (tz_offset % 3600) / 60);
	} else
		(void)fputs(NILVALUE " ", fp);
	/* Hostname. */
	(void)gethostname(hostname, sizeof(hostname));
	(void)fprintf(fp, "%s ",
	    hostname[0] == '\0' ? NILVALUE : hostname);
	if (LogStat & LOG_PERROR) {
		/* Transfer to string buffer */
		(void)fflush(fp);
		stdp = tbuf + (sizeof(tbuf) - tbuf_cookie.left);
	}
	/* Application name. */
	if (LogTag == NULL)
		LogTag = _getprogname();
	else if (LogTagLength == -1)
		ref_parse_tag();
	if (LogTagLength > 0)
		(void)fprintf(fp, "%.*s ", LogTagLength, LogTag);
	else
		(void)fprintf(fp, "%s ", LogTag == NULL ? NILVALUE : LogTag);
	/*
	 * Provide the process ID regardless of whether LOG_PID has been
	 * specified, as it provides valuable information. Many
	 * applications tend not to use this, even though they should.
	 */
	if (LogTagLength <= 0)
		LogPid = getpid();
	(void)fprintf(fp, "%d ", (int)LogPid);
	/* Message ID. */
	(void)fputs(NILVALUE " ", fp);
	/* Structured data. */
	(void)fputs(NILVALUE " ", fp);

	/* Check to see if we can skip expanding the %m */
	if (strstr(fmt, "%m")) {

		/* Create the second stdio hook */
		fmt_cookie.base = fmt_cpy;
		fmt_cookie.left = sizeof(fmt_cpy) - 1;
		fmt_fp = fwopen(&fmt_cookie, ref_writehook);
		if (fmt_fp == NULL) {
			fclose(fp);
			return;
		}

		/*
		 * Substitute error message for %m.  Be careful not to
		 * molest an escaped percent "%%m".  We want to pass it
		 * on untouched as the format is later parsed by vfprintf.
		 */
		for ( ; (ch = *fmt); ++fmt) {
			if (ch == '%' && fmt[1] == 'm') {
				++fmt;
				strerror_r(saved_errno, errstr, sizeof(errstr));
				fputs(errstr, fmt_fp);
			} else if (ch == '%' && fmt[1] == '%') {
				++fmt;
				fputc(ch, fmt_fp);
				fputc(ch, fmt_fp);
			} else {
				fputc(ch, fmt_fp);
			}
		}

		/* Null terminate if room */
		fputc(0, fmt_fp);
		fclose(fmt_fp);

		/* Guarantee null termination */
		fmt_cpy[sizeof(fmt_cpy) - 1] = '\0';

		fmt = fmt_cpy;
	}

	/* Message. */
	(void)vfprintf(fp, fmt, ap);
	(void)fclose(fp);

	cnt = sizeof(tbuf) - tbuf_cookie.left;

	/* Remove a trailing newline */
	if (tbuf[cnt - 1] == '\n')
		cnt--;

	/* Output to stderr if requested. */
	if (LogStat & LOG_PERROR) {
		struct iovec iov[2];
		struct iovec *v = iov;

		v->iov_base = stdp;
		v->iov_len = cnt - (stdp - tbuf);
		++v;
		v->iov_base = "\n";
		v->iov_len = 1;
		(void)_writev(STDERR_FILENO, iov, 2);
	}

	/* Get connected, output the message to the local logger. */
	if (!opened)
		ref_openlog_unlocked(LogTag, LogStat | LOG_NDELAY, 0);
	ref_connectlog();

	/*
	 * If the send() failed, there are two likely scenarios:
	 * 1) syslogd was restarted.  In this case make one (only) attempt
	 *    to reconnect.
	 * 2) We filled our buffer due to syslogd not being able to read
	 *    as fast as we write.  In this case prefer to lose the current
	 *    message rather than whole buffer of previously logged data.
	 */
	if (send(LogFile, tbuf, cnt, 0) < 0) {
		if (errno != ENOBUFS) {
			ref_disconnectlog();
			ref_connectlog();
			if (send(LogFile, tbuf, cnt, 0) >= 0)
				return;
		}
	} else
		return;

	/*
	 * Output the message to the console; try not to block
	 * as a blocking console should not stop other processes.
	 * Make sure the error reported is the one from the syslogd failure.
	 */
	if (LogStat & LOG_CONS &&
	    (fd = _open(_PATH_CONSOLE, O_WRONLY|O_NONBLOCK|O_CLOEXEC, 0)) >=
	    0) {
		struct iovec iov[2];
		struct iovec *v = iov;

		p = strchr(tbuf, '>') + 3;
		v->iov_base = p;
		v->iov_len = cnt - (p - tbuf);
		++v;
		v->iov_base = "\r\n";
		v->iov_len = 2;
		(void)_writev(fd, iov, 2);
		(void)_close(fd);
	}
}

static void
ref_syslog_cancel_cleanup(void *arg __unused)
{

	THREAD_UNLOCK();
}

void
ref_vsyslog(int pri, const char *fmt, va_list ap)
{

	THREAD_LOCK();
	ref_pthread_cleanup_push(ref_syslog_cancel_cleanup, NULL);
	ref_vsyslog1(pri, fmt, ap);
	ref_pthread_cleanup_pop(1);
}

/* Should be called with mutex acquired */
static void
ref_disconnectlog(void)
{
	/*
	 * If the user closed the FD and opened another in the same slot,
	 * that's their problem.  They should close it before calling on
	 * system services.
	 */
	if (LogFile != -1) {
		_close(LogFile);
		LogFile = -1;
	}
	connected = false;			/* retry connect */
}

/* Should be called with mutex acquired */
static void
ref_connectlog(void)
{
	struct sockaddr_un SyslogAddr;	/* AF_UNIX address of local logger */

	if (LogFile == -1) {
		socklen_t len;

		if ((LogFile = _socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC,
		    0)) == -1)
			return;
		if (_getsockopt(LogFile, SOL_SOCKET, SO_SNDBUF, &len,
		    &(socklen_t){sizeof(len)}) == 0) {
			if (len < MAXLINE) {
				len = MAXLINE;
				(void)_setsockopt(LogFile, SOL_SOCKET, SO_SNDBUF,
				    &len, sizeof(len));
			}
		}
	}
	if (!connected) {
		SyslogAddr.sun_len = sizeof(SyslogAddr);
		SyslogAddr.sun_family = AF_UNIX;

		(void)strncpy(SyslogAddr.sun_path, _PATH_LOG,
		    sizeof SyslogAddr.sun_path);
		if (_connect(LogFile, (struct sockaddr *)&SyslogAddr,
		    sizeof(SyslogAddr)) != -1)
			connected = true;
		else {
			(void)_close(LogFile);
			LogFile = -1;
		}
	}
}

static void
ref_openlog_unlocked(const char *ident, int logstat, int logfac)
{
	if (ident != NULL) {
		LogTag = ident;
		LogTagLength = -1;
	}
	LogStat = logstat;
	ref_parse_tag();
	if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
		LogFacility = logfac;

	if (LogStat & LOG_NDELAY)	/* open immediately */
		ref_connectlog();

	opened = 1;	/* ident and facility has been set */
}

void
ref_openlog(const char *ident, int logstat, int logfac)
{

	THREAD_LOCK();
	ref_pthread_cleanup_push(ref_syslog_cancel_cleanup, NULL);
	ref_openlog_unlocked(ident, logstat, logfac);
	ref_pthread_cleanup_pop(1);
}


void
ref_closelog(void)
{
	THREAD_LOCK();
	if (LogFile != -1) {
		(void)_close(LogFile);
		LogFile = -1;
	}
	LogTag = NULL;
	LogTagLength = -1;
	connected = false;
	THREAD_UNLOCK();
}

/* ref_setlogmask -- set the log mask level */
int
ref_setlogmask(int pmask)
{
	int omask;

	THREAD_LOCK();
	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	THREAD_UNLOCK();
	return (omask);
}

/*
 * Obtain LogPid from LogTag formatted as per RFC 3164,
 * Section 5.3 Originating Process Information:
 *
 * ident[NNN]
 */
static void
ref_parse_tag(void)
{
	char *begin, *end, *p;
	pid_t pid;

	if (LogTag == NULL || (LogStat & LOG_PID) != 0)
		return;
	/*
	 * LogTagLength is -1 if LogTag was not parsed yet.
	 * Avoid multiple passes over same LogTag.
	 */
	LogTagLength = 0;

	/* Check for presence of opening [ and non-empty ident. */
	if ((begin = strchr(LogTag, '[')) == NULL || begin == LogTag)
		return;
	/* Check for presence of closing ] at the very end and non-empty pid. */
	if ((end = strchr(begin + 1, ']')) == NULL || end[1] != 0 ||
	    (end - begin) < 2)
		return;

	/* Check for pid to contain digits only. */
	pid = (pid_t)strtol(begin + 1, &p, 10);
	if (p != end)
		return;

	LogPid = pid;
	LogTagLength = begin - LogTag;
}

/* ===== fnmatch.c ===== */
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
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
 * Function ref_fnmatch() as specified in POSIX 1003.2-1992, section B.6.
 * Compares a filename or pathname to a pattern.
 */

/*
 * Some notes on multibyte character support:
 * 1. Patterns with illegal byte sequences match nothing.
 * 2. Illegal byte sequences in the "string" argument are handled by treating
 *    them as single-byte characters with a value of the first byte of the
 *    sequence cast to wchar_t.
 * 3. Multibyte conversion state objects (mbstate_t) are passed around and
 *    used for most, but not all, conversions. Further work will be required
 *    to support state-dependent encodings.
 */



#define	EOS	'\0'

#define RANGE_MATCH     1
#define RANGE_NOMATCH   0
#define RANGE_ERROR     (-1)

static int ref_rangematch(const char *, wchar_t, const char *, int, char **,
    char **, mbstate_t *, mbstate_t *);
static int ref_fnmatch1(const char *, const char *, const char *, int, mbstate_t,
		mbstate_t);

int
ref_fnmatch(const char *pattern, const char *string, int flags)
{
	static const mbstate_t initial;

	return (ref_fnmatch1(pattern, string, string, flags, initial, initial));
}

static int
ref_fnmatch1(const char *pattern, const char *string, const char *stringstart,
    int flags, mbstate_t patmbs, mbstate_t strmbs)
{
	const char *bt_pattern, *bt_string;
	mbstate_t bt_patmbs, bt_strmbs;
	char *newp, *news;
	char c;
	wchar_t pc, sc;
	size_t pclen, sclen;

	bt_pattern = bt_string = NULL;
	for (;;) {
		pclen = mbrtowc(&pc, pattern, MB_LEN_MAX, &patmbs);
		if (pclen == (size_t)-1 || pclen == (size_t)-2)
			return (FNM_NOMATCH);
		pattern += pclen;
		sclen = mbrtowc(&sc, string, MB_LEN_MAX, &strmbs);
		if (sclen == (size_t)-1 || sclen == (size_t)-2) {
			sc = (unsigned char)*string;
			sclen = 1;
			memset(&strmbs, 0, sizeof(strmbs));
		}
		switch (pc) {
		case EOS:
			if ((flags & FNM_LEADING_DIR) && sc == '/')
				return (0);
			if (sc == EOS)
				return (0);
			goto backtrack;
		case '?':
			if (sc == EOS)
				return (FNM_NOMATCH);
			if (sc == '/' && (flags & FNM_PATHNAME))
				goto backtrack;
			if (sc == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				goto backtrack;
			string += sclen;
			break;
		case '*':
			c = *pattern;
			/* Collapse multiple stars. */
			while (c == '*')
				c = *++pattern;

			if (sc == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				goto backtrack;

			/* Optimize for pattern with * at end or before /. */
			if (c == EOS)
				if (flags & FNM_PATHNAME)
					return ((flags & FNM_LEADING_DIR) ||
					    strchr(string, '/') == NULL ?
					    0 : FNM_NOMATCH);
				else
					return (0);
			else if (c == '/' && flags & FNM_PATHNAME) {
				if ((string = strchr(string, '/')) == NULL)
					return (FNM_NOMATCH);
				break;
			}

			/*
			 * First try the shortest match for the '*' that
			 * could work. We can forget any earlier '*' since
			 * there is no way having it match more characters
			 * can help us, given that we are already here.
			 */
			bt_pattern = pattern;
			bt_patmbs = patmbs;
			bt_string = string;
			bt_strmbs = strmbs;
			break;
		case '[':
			if (sc == EOS)
				return (FNM_NOMATCH);
			if (sc == '/' && (flags & FNM_PATHNAME))
				goto backtrack;
			if (sc == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				goto backtrack;

			switch (ref_rangematch(pattern, sc, string + sclen, flags,
			    &newp, &news, &patmbs, &strmbs)) {
			case RANGE_ERROR:
				goto norm;
			case RANGE_MATCH:
				pattern = newp;
				string = news;
				break;
			case RANGE_NOMATCH:
				goto backtrack;
			}
			break;
		case '\\':
			if (!(flags & FNM_NOESCAPE)) {
				pclen = mbrtowc(&pc, pattern, MB_LEN_MAX,
				    &patmbs);
				if (pclen == 0 || pclen == (size_t)-1 ||
				    pclen == (size_t)-2)
					return (FNM_NOMATCH);
				pattern += pclen;
			}
			/* FALLTHROUGH */
		default:
		norm:
			string += sclen;
			if (pc == sc)
				;
			else if ((flags & FNM_CASEFOLD) &&
				 (towlower(pc) == towlower(sc)))
				;
			else {
		backtrack:
				/*
				 * If we have a mismatch (other than hitting
				 * the end of the string), go back to the last
				 * '*' seen and have it match one additional
				 * character.
				 */
				if (bt_pattern == NULL)
					return (FNM_NOMATCH);
				sclen = mbrtowc(&sc, bt_string, MB_LEN_MAX,
				    &bt_strmbs);
				if (sclen == (size_t)-1 ||
				    sclen == (size_t)-2) {
					sc = (unsigned char)*bt_string;
					sclen = 1;
					memset(&bt_strmbs, 0,
					    sizeof(bt_strmbs));
				}
				if (sc == EOS)
					return (FNM_NOMATCH);
				if (sc == '/' && flags & FNM_PATHNAME)
					return (FNM_NOMATCH);
				bt_string += sclen;
				pattern = bt_pattern;
				patmbs = bt_patmbs;
				string = bt_string;
				strmbs = bt_strmbs;
			}
			break;
		}
	}
	/* NOTREACHED */
}

static int
ref_rangematch(const char *pattern, wchar_t test, const char *string, int flags,
    char **newp, char **news, mbstate_t *patmbs, mbstate_t *strmbs)
{
	int negate, ok;
	wchar_t c, c2;
	size_t pclen;
	const char *origpat;
	struct xlocale_collate *table =
	    (struct xlocale_collate *)__get_locale()->components[XLC_COLLATE];
	wchar_t buf[COLLATE_STR_LEN];	/* STR_LEN defined in collate.h */
	const char *cp, *savestring;
	int special;
	mbstate_t save;
	size_t sclen, len;

	/*
	 * A bracket expression starting with an unquoted circumflex
	 * character produces unspecified results (IEEE 1003.2-1992,
	 * 3.13.2).  This implementation treats it like '!', for
	 * consistency with the regular expression syntax.
	 * J.T. Conklin (conklin@ngai.kaleida.com)
	 */
	if ((negate = (*pattern == '!' || *pattern == '^')))
		++pattern;

	if (flags & FNM_CASEFOLD)
		test = towlower(test);

	/*
	 * A right bracket shall lose its special meaning and represent
	 * itself in a bracket expression if it occurs first in the list.
	 * -- POSIX.2 2.8.3.2
	 */
	ok = 0;
	origpat = pattern;
	for (;;) {
		c = 0;
		if (*pattern == ']' && pattern > origpat) {
			break;
		} else if (*pattern == '\0') {
			return (RANGE_ERROR);
		} else if (*pattern == '/' && (flags & FNM_PATHNAME)) {
			return (RANGE_NOMATCH);
		} else if (*pattern == '\\' && !(flags & FNM_NOESCAPE)) {
			pattern++;
		} else if (*pattern == '[' &&
		    ((special = *(pattern + 1)) == '.' ||
		    special == '=' || special == ':')) {
			cp = (pattern += 2);
			while ((cp = strchr(cp, special))) {
				if (*(cp + 1) == ']')
					break;
				cp++;
			}
			if (!cp)
				return (RANGE_ERROR);
			if (special == '.') {
treat_like_collating_symbol:
				len = __collate_collating_symbol(buf,
				    COLLATE_STR_LEN, pattern,
				    cp - pattern, patmbs);
				if (len == (size_t)-1 || len == 0)
					return (RANGE_ERROR);
				pattern = cp + 2;
				if (len > 1) {
					wchar_t *wp, sc;

					/*
					 * No multi-character collation
					 * symbols as start of range.
					 */
					if (*(cp + 2) == '-' &&
					    *(cp + 3) != EOS &&
					    *(cp + 3) != ']')
						return (RANGE_ERROR);
					wp = buf;
					if (test != *wp++)
						continue;
					if (len == 1) {
						ok = 1;
						break;
					}
					memcpy(&save, strmbs, sizeof(save));
					savestring = string;
					while (--len > 0) {
						sclen = mbrtowc(&sc, string,
						    MB_LEN_MAX, strmbs);
						if (sclen == (size_t)-1 ||
						    sclen == (size_t)-2) {
							sc = (unsigned char)*string;
							sclen = 1;
							memset(&strmbs, 0,
							    sizeof(strmbs));
						}
						if (sc != *wp++) {
							memcpy(strmbs, &save,
							    sizeof(save));
							string = savestring;
							break;
						}
						string += sclen;
					}
					if (len == 0) {
						ok = 1;
						break;
					}
					continue; /* no match */
				}
				c = *buf;
			} else if (special == '=') {
				int ec;
				memcpy(&save, patmbs, sizeof(save));
				ec = __collate_equiv_class(pattern,
				    cp - pattern, patmbs);
				if (ec < 0)
					return (RANGE_ERROR);
				if (ec == 0) {
					memcpy(patmbs, &save, sizeof(save));
					goto treat_like_collating_symbol;
				}
				pattern = cp + 2;
				/* no equivalence classes as start of range */
				if (*(cp + 2) == '-' && *(cp + 3) != EOS &&
				    *(cp + 3) != ']')
					return (RANGE_ERROR);
				len = __collate_equiv_match(ec, NULL, 0, test,
				    string, strlen(string), strmbs, &sclen);
				if (len < 0)
					return (RANGE_ERROR);
				if (len > 0) {
					ok = 1;
					string += sclen;
					break;
				}
				continue;
			} else { /* special == ':' */
				wctype_t charclass;
				char name[CHARCLASS_NAME_MAX + 1];
				/* no character classes as start of range */
				if (*(cp + 2) == '-' && *(cp + 3) != EOS &&
				    *(cp + 3) != ']')
					return (RANGE_ERROR);
				/* assume character class names are ascii */
				if (cp - pattern > CHARCLASS_NAME_MAX)
					return (RANGE_ERROR);
				strlcpy(name, pattern, cp - pattern + 1);
				pattern = cp + 2;
				if ((charclass = wctype(name)) == 0)
					return (RANGE_ERROR);
				if (iswctype(test, charclass)) {
					ok = 1;
					break;
				}
				continue;
			}
		}
		if (!c) {
			pclen = mbrtowc(&c, pattern, MB_LEN_MAX, patmbs);
			if (pclen == (size_t)-1 || pclen == (size_t)-2)
				return (RANGE_NOMATCH);
			pattern += pclen;
		}
		if (flags & FNM_CASEFOLD)
			c = towlower(c);

		if (*pattern == '-' && *(pattern + 1) != EOS &&
		    *(pattern + 1) != ']') {
			if (*++pattern == '\\' && !(flags & FNM_NOESCAPE))
				if (*pattern != EOS)
					pattern++;
			pclen = mbrtowc(&c2, pattern, MB_LEN_MAX, patmbs);
			if (pclen == (size_t)-1 || pclen == (size_t)-2)
				return (RANGE_NOMATCH);
			pattern += pclen;
			if (c2 == EOS)
				return (RANGE_ERROR);

			if ((c2 == '[' && (special = *pattern) == '.') ||
			    special == '=' || special == ':') {

				/*
				 * No equivalence classes or character
				 * classes as end of range.
				 */
				if (special == '=' || special == ':')
					return (RANGE_ERROR);
				cp = ++pattern;
				while ((cp = strchr(cp, special))) {
					if (*(cp + 1) == ']')
						break;
					cp++;
				}
				if (!cp)
					return (RANGE_ERROR);
				len = __collate_collating_symbol(buf,
				    COLLATE_STR_LEN, pattern,
				    cp - pattern, patmbs);

				/*
				 * No multi-character collation symbols
				 *  as end of range.
				 */
				if (len != 1)
					return (RANGE_ERROR);
				pattern = cp + 2;
				c2 = *buf;
			}

			if (flags & FNM_CASEFOLD)
				c2 = towlower(c2);

			if (table->__collate_load_error ?
			    c <= test && test <= c2 :
			       __wcollate_range_cmp(c, test) <= 0
			    && __wcollate_range_cmp(test, c2) <= 0
			   ) {
				ok = 1;
				break;
			}
		} else if (c == test) {
			ok = 1;
			break;
		}
	}

	/* go to end of bracket expression */
	special = 0;
	while (*pattern != ']') {
		if (*pattern == 0)
			return (RANGE_ERROR);
		if (*pattern == special) {
			if (*++pattern == ']') {
				special = 0;
				pattern++;
			}
			continue;
		}
		if (!special && *pattern == '[') {
			special = *++pattern;
			if (special != '.' && special != '=' && special != ':')
				special = 0;
			else
				pattern++;
			continue;
		}
		pclen = mbrtowc(&c, pattern, MB_LEN_MAX, patmbs);
		if (pclen == (size_t)-1 || pclen == (size_t)-2)
			return (RANGE_NOMATCH);
		pattern += pclen;
	}

	*newp = (char *)++pattern;
	*news = (char *)string;

	return (ok == negate ? RANGE_NOMATCH : RANGE_MATCH);
}

/* ===== _pthread_stubs.c ===== */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001 Daniel Eischen <deischen@FreeBSD.org>.
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
 * THIS SOFTWARE IS PROVIDED BY DANIEL EISCHEN AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */



/*
 * Weak symbols: All libc internal usage of these functions should
 * use the weak symbol versions (_pthread_XXX).  If libpthread is
 * linked, it will override these functions with (non-weak) routines.
 * The _pthread_XXX functions are provided solely for internal libc
 * usage to avoid unwanted cancellation points and to differentiate
 * between application locks and libc locks (threads holding the
 * latter can't be allowed to exit/terminate).
 */

/* Define a null pthread structure just to satisfy _pthread_self. */
struct pthread {
};

static struct pthread	main_thread;

static int		ref_stub_main(void);
static void		ref_stub_void(void);
static void		*ref_stub_null(void);
static struct pthread	*ref_stub_self(void);
static int		ref_stub_zero(void);
static int		ref_stub_fail(void);
static int		ref_stub_true(void);
static void		ref_stub_exit(void);
static int		ref_stub_esrch(void);
static int		ref_stub_getname_np(pthread_t, char *, size_t);

#define	PJT_DUAL_ENTRY(entry)	\
	(pthread_func_t)entry, (pthread_func_t)entry

__attribute__((visibility("protected")))
pthread_func_entry_t __thr_jtable[PJT_MAX] = {
	[PJT_ATFORK] =			{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_DESTROY] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_GETDETACHSTATE] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_GETGUARDSIZE] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_GETINHERITSCHED] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_GETSCHEDPARAM] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_GETSCHEDPOLICY] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_GETSCOPE] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_GETSTACKADDR] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_GETSTACKSIZE] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_INIT] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_SETDETACHSTATE] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_SETGUARDSIZE] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_SETINHERITSCHED] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_SETSCHEDPARAM] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_SETSCHEDPOLICY] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_SETSCOPE] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_SETSTACKADDR] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_SETSTACKSIZE] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_CANCEL] =			{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_CLEANUP_POP] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_CLEANUP_PUSH] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_COND_BROADCAST] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_COND_DESTROY] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_COND_INIT] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_COND_SIGNAL] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_COND_TIMEDWAIT] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_COND_WAIT] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_DETACH] =			{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_EQUAL] =			{PJT_DUAL_ENTRY(ref_stub_true)},
	[PJT_EXIT] =			{PJT_DUAL_ENTRY(ref_stub_exit)},
	[PJT_GETSPECIFIC] =		{PJT_DUAL_ENTRY(ref_stub_null)},
	[PJT_JOIN] =			{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_KEY_CREATE] =		{PJT_DUAL_ENTRY(ref_stub_fail)},
	[PJT_KEY_DELETE] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_KILL] =			{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MAIN_NP] =			{PJT_DUAL_ENTRY(ref_stub_main)},
	[PJT_MUTEXATTR_DESTROY] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEXATTR_INIT] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEXATTR_SETTYPE] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEX_DESTROY] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEX_INIT] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEX_LOCK] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEX_TRYLOCK] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEX_UNLOCK] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ONCE] =			{PJT_DUAL_ENTRY(ref_stub_fail)},
	[PJT_RWLOCK_DESTROY] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_RWLOCK_INIT] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_RWLOCK_RDLOCK] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_RWLOCK_TRYRDLOCK] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_RWLOCK_TRYWRLOCK] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_RWLOCK_UNLOCK] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_RWLOCK_WRLOCK] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_SELF] =			{PJT_DUAL_ENTRY(ref_stub_self)},
	[PJT_SETCANCELSTATE] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_SETCANCELTYPE] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_SETSPECIFIC] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_SIGMASK] =			{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_TESTCANCEL] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_CLEANUP_POP_IMP] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_CLEANUP_PUSH_IMP] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_CANCEL_ENTER] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_CANCEL_LEAVE] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEX_CONSISTENT] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEXATTR_GETROBUST] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_MUTEXATTR_SETROBUST] =	{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_GETTHREADID_NP] =		{PJT_DUAL_ENTRY(ref_stub_zero)},
	[PJT_ATTR_GET_NP] =		{PJT_DUAL_ENTRY(ref_stub_esrch)},
	[PJT_GETNAME_NP] =		{PJT_DUAL_ENTRY(ref_stub_getname_np)},
	[PJT_SUSPEND_ALL_NP] =		{PJT_DUAL_ENTRY(ref_stub_void)},
	[PJT_RESUME_ALL_NP] =		{PJT_DUAL_ENTRY(ref_stub_void)},
};

/*
 * Weak aliases for exported (pthread_*) and internal (_pthread_*) routines.
 */
#define	WEAK_REF(sym, alias)	__weak_reference(sym, alias)

#define	FUNC_TYPE(name)		__CONCAT(name, _func_t)
#define	FUNC_INT(name)		__CONCAT(name, _int)
#define	FUNC_EXP(name)		__CONCAT(name, _exp)

#define	STUB_FUNC(name, idx, ret)				\
	static ret FUNC_EXP(name)(void) __used;			\
	static ret FUNC_INT(name)(void) __used;			\
	WEAK_REF(FUNC_EXP(name), name);				\
	WEAK_REF(FUNC_INT(name), __CONCAT(_, name));		\
	typedef ret (*FUNC_TYPE(name))(void);			\
	static ret FUNC_EXP(name)(void)				\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][0];	\
		return (func());				\
	}							\
	static ret FUNC_INT(name)(void)				\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][1];	\
		return (func());				\
	}

#define	STUB_FUNC1(name, idx, ret, p0_type)			\
	static ret FUNC_EXP(name)(p0_type) __used;		\
	static ret FUNC_INT(name)(p0_type) __used;		\
	WEAK_REF(FUNC_EXP(name), name);				\
	WEAK_REF(FUNC_INT(name), __CONCAT(_, name));		\
	typedef ret (*FUNC_TYPE(name))(p0_type);		\
	static ret FUNC_EXP(name)(p0_type p0)			\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][0];	\
		return (func(p0));				\
	}							\
	static ret FUNC_INT(name)(p0_type p0)			\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][1];	\
		return (func(p0));				\
	}

#define	STUB_FUNC2(name, idx, ret, p0_type, p1_type)		\
	static ret FUNC_EXP(name)(p0_type, p1_type) __used;	\
	static ret FUNC_INT(name)(p0_type, p1_type) __used;	\
	WEAK_REF(FUNC_EXP(name), name);				\
	WEAK_REF(FUNC_INT(name), __CONCAT(_, name));		\
	typedef ret (*FUNC_TYPE(name))(p0_type, p1_type);	\
	static ret FUNC_EXP(name)(p0_type p0, p1_type p1)	\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][0];	\
		return (func(p0, p1));				\
	}							\
	static ret FUNC_INT(name)(p0_type p0, p1_type p1)	\
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][1];	\
		return (func(p0, p1));				\
	}

#define	STUB_FUNC3(name, idx, ret, p0_type, p1_type, p2_type)	\
	static ret FUNC_EXP(name)(p0_type, p1_type, p2_type) __used; \
	static ret FUNC_INT(name)(p0_type, p1_type, p2_type) __used; \
	WEAK_REF(FUNC_EXP(name), name);				\
	WEAK_REF(FUNC_INT(name), __CONCAT(_, name));		\
	typedef ret (*FUNC_TYPE(name))(p0_type, p1_type, p2_type); \
	static ret FUNC_EXP(name)(p0_type p0, p1_type p1, p2_type p2) \
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][0];	\
		return (func(p0, p1, p2));			\
	}							\
	static ret FUNC_INT(name)(p0_type p0, p1_type p1, p2_type p2) \
	{							\
		FUNC_TYPE(name) func;				\
		func = (FUNC_TYPE(name))__thr_jtable[idx][1];	\
		return (func(p0, p1, p2));			\
	}

STUB_FUNC1(pthread_cond_broadcast, PJT_COND_BROADCAST, int, void *)
STUB_FUNC1(pthread_cond_destroy, PJT_COND_DESTROY, int, void *)
STUB_FUNC2(pthread_cond_init,	PJT_COND_INIT, int, void *, void *)
STUB_FUNC1(pthread_cond_signal,	PJT_COND_SIGNAL, int, void *)
STUB_FUNC2(pthread_cond_wait,	PJT_COND_WAIT, int, void *, void *)
STUB_FUNC1(pthread_getspecific,	PJT_GETSPECIFIC, void *, pthread_key_t)
STUB_FUNC2(pthread_key_create,	PJT_KEY_CREATE, int, void *, void *)
STUB_FUNC1(pthread_key_delete,	PJT_KEY_DELETE, int, pthread_key_t)
STUB_FUNC(pthread_main_np,	PJT_MAIN_NP, int)
STUB_FUNC1(pthread_mutex_destroy, PJT_MUTEX_DESTROY, int, void *)
STUB_FUNC2(pthread_mutex_init,	PJT_MUTEX_INIT, int, void *, void *)
STUB_FUNC1(pthread_mutex_lock,	PJT_MUTEX_LOCK, int, void *)
STUB_FUNC1(pthread_mutex_trylock, PJT_MUTEX_TRYLOCK, int, void *)
STUB_FUNC1(pthread_mutex_unlock, PJT_MUTEX_UNLOCK, int, void *)
STUB_FUNC1(pthread_mutex_consistent, PJT_MUTEX_CONSISTENT, int, void *)
STUB_FUNC1(pthread_mutexattr_destroy, PJT_MUTEXATTR_DESTROY, int, void *)
STUB_FUNC1(pthread_mutexattr_init, PJT_MUTEXATTR_INIT, int, void *)
STUB_FUNC2(pthread_mutexattr_settype, PJT_MUTEXATTR_SETTYPE, int, void *, int)
STUB_FUNC2(pthread_mutexattr_getrobust, PJT_MUTEXATTR_GETROBUST, int, void *,
    int *)
STUB_FUNC2(pthread_mutexattr_setrobust, PJT_MUTEXATTR_SETROBUST, int, void *,
    int)
STUB_FUNC2(pthread_once,	PJT_ONCE, int, void *, void *)
STUB_FUNC1(pthread_rwlock_destroy, PJT_RWLOCK_DESTROY, int, void *)
STUB_FUNC2(pthread_rwlock_init,	PJT_RWLOCK_INIT, int, void *, void *)
STUB_FUNC1(pthread_rwlock_rdlock, PJT_RWLOCK_RDLOCK, int, void *)
STUB_FUNC1(pthread_rwlock_tryrdlock, PJT_RWLOCK_TRYRDLOCK, int, void *)
STUB_FUNC1(pthread_rwlock_trywrlock, PJT_RWLOCK_TRYWRLOCK, int, void *)
STUB_FUNC1(pthread_rwlock_unlock, PJT_RWLOCK_UNLOCK, int, void *)
STUB_FUNC1(pthread_rwlock_wrlock, PJT_RWLOCK_WRLOCK, int, void *)
STUB_FUNC(pthread_self,		PJT_SELF, pthread_t)
STUB_FUNC(pthread_getthreadid_np, PJT_GETTHREADID_NP, int)
STUB_FUNC2(pthread_setspecific, PJT_SETSPECIFIC, int, pthread_key_t, void *)
STUB_FUNC3(pthread_sigmask, PJT_SIGMASK, int, int, void *, void *)
STUB_FUNC3(pthread_atfork, PJT_ATFORK, int, void *, void *, void*)
STUB_FUNC1(pthread_attr_destroy, PJT_ATTR_DESTROY, int, void *);
STUB_FUNC2(pthread_attr_getdetachstate, PJT_ATTR_GETDETACHSTATE, int, void *, void *)
STUB_FUNC2(pthread_attr_getguardsize, PJT_ATTR_GETGUARDSIZE, int, void *, void *)
STUB_FUNC2(pthread_attr_getstackaddr, PJT_ATTR_GETSTACKADDR, int, void *, void *)
STUB_FUNC2(pthread_attr_getstacksize, PJT_ATTR_GETSTACKSIZE, int, void *, void *)
STUB_FUNC2(pthread_attr_getinheritsched, PJT_ATTR_GETINHERITSCHED, int, void *, void *)
STUB_FUNC2(pthread_attr_getschedparam, PJT_ATTR_GETSCHEDPARAM, int, void *, void *)
STUB_FUNC2(pthread_attr_getschedpolicy, PJT_ATTR_GETSCHEDPOLICY, int, void *, void *)
STUB_FUNC2(pthread_attr_getscope, PJT_ATTR_GETSCOPE, int, void *, void *)
STUB_FUNC1(pthread_attr_init, PJT_ATTR_INIT, int, void *)
STUB_FUNC2(pthread_attr_setdetachstate, PJT_ATTR_SETDETACHSTATE, int, void *, int)
STUB_FUNC2(pthread_attr_setguardsize, PJT_ATTR_SETGUARDSIZE, int, void *, size_t)
STUB_FUNC2(pthread_attr_setstackaddr, PJT_ATTR_SETSTACKADDR, int, void *, void *)
STUB_FUNC2(pthread_attr_setstacksize, PJT_ATTR_SETSTACKSIZE, int, void *, size_t)
STUB_FUNC2(pthread_attr_setinheritsched, PJT_ATTR_SETINHERITSCHED, int, void *, int)
STUB_FUNC2(pthread_attr_setschedparam, PJT_ATTR_SETSCHEDPARAM, int, void *, void *)
STUB_FUNC2(pthread_attr_setschedpolicy, PJT_ATTR_SETSCHEDPOLICY, int, void *, int)
STUB_FUNC2(pthread_attr_setscope, PJT_ATTR_SETSCOPE, int, void *, int)
STUB_FUNC1(pthread_cancel, PJT_CANCEL, int, void *)
STUB_FUNC1(pthread_cleanup_pop, PJT_CLEANUP_POP, int, int)
STUB_FUNC2(pthread_cleanup_push, PJT_CLEANUP_PUSH, void, void *, void *)
STUB_FUNC3(pthread_cond_timedwait, PJT_COND_TIMEDWAIT, int, void *, void *, void *)
STUB_FUNC1(pthread_detach, PJT_DETACH, int, void *)
STUB_FUNC2(pthread_equal, PJT_EQUAL, int, void *, void *)
STUB_FUNC1(pthread_exit, PJT_EXIT, void, void *)
STUB_FUNC2(pthread_join, PJT_JOIN, int, void *, void *)
STUB_FUNC2(pthread_kill, PJT_KILL, int, void *, int)
STUB_FUNC2(pthread_setcancelstate, PJT_SETCANCELSTATE, int, int, void *)
STUB_FUNC2(pthread_setcanceltype, PJT_SETCANCELTYPE, int, int, void *)
STUB_FUNC(pthread_testcancel, PJT_TESTCANCEL, void)
STUB_FUNC1(__pthread_cleanup_pop_imp, PJT_CLEANUP_POP_IMP, void, int)
STUB_FUNC3(__pthread_cleanup_push_imp, PJT_CLEANUP_PUSH_IMP, void, void *,
    void *, void *)
STUB_FUNC1(_pthread_cancel_enter, PJT_CANCEL_ENTER, void, int)
STUB_FUNC1(_pthread_cancel_leave, PJT_CANCEL_LEAVE, void, int)
STUB_FUNC2(pthread_attr_get_np, PJT_ATTR_GET_NP, int, pthread_t, pthread_attr_t *)
STUB_FUNC3(pthread_getname_np, PJT_GETNAME_NP, int, pthread_t, char *, size_t)
STUB_FUNC(pthread_suspend_all_np, PJT_SUSPEND_ALL_NP, void);
STUB_FUNC(pthread_resume_all_np, PJT_RESUME_ALL_NP, void);

static int
ref_stub_zero(void)
{
	return (0);
}

static void
ref_stub_void(void)
{

}

static void *
ref_stub_null(void)
{
	return (NULL);
}

static struct pthread *
ref_stub_self(void)
{
	return (&main_thread);
}

static int
ref_stub_fail(void)
{
	return (ENOSYS);
}

static int
ref_stub_main(void)
{
	return (-1);
}

static int
ref_stub_true(void)
{
	return (1);
}

static void
ref_stub_exit(void)
{
	exit(0);
}

static int
ref_stub_esrch(void)
{
	return (ESRCH);
}

static int
ref_stub_getname_np(pthread_t thread, char *buf, size_t len)
{
	if (thread != &main_thread)
		return (ESRCH);
	if (len >= 1)
		buf[0] = '\0';
	return (0);
}
