/* oracle.c - reference implementation for batch b0334 */
#ifndef B0334_PS_BATCH
#define B0334_PS_BATCH

#define _DEFAULT_SOURCE 1
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <locale.h>
#include <pwd.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


#ifndef __unused
#define __unused
#endif
#ifndef __dead2
#define __dead2
#endif
#ifndef LONG_BIT
#define LONG_BIT (8*(int)sizeof(long))
#endif
#ifndef MAXLOGNAME
#define MAXLOGNAME 256
#endif
#ifndef _POSIX2_LINE_MAX
#define _POSIX2_LINE_MAX 2048
#endif
#ifndef _PATH_DEVNULL
#define _PATH_DEVNULL "/dev/null"
#endif
#ifndef _PATH_DEV
#define _PATH_DEV "/dev/"
#endif
#ifndef _PATH_TTY
#define _PATH_TTY "/dev/tty"
#endif
#ifndef _PATH_CONSOLE
#define _PATH_CONSOLE "/dev/console"
#endif
#ifndef NODEV
#define NODEV ((dev_t)-1)
#endif
#ifndef COMMLEN
#define COMMLEN 19
#endif
#ifndef GID_MAX
#define GID_MAX UINT_MAX
#endif
#ifndef UID_MAX
#define UID_MAX UINT_MAX
#endif
#ifndef KERN_PROC_ALL
#define KERN_PROC_ALL 1
#endif
#ifndef KERN_PROC_PROC
#define KERN_PROC_PROC 2
#endif
#ifndef KERN_PROC_PID
#define KERN_PROC_PID 3
#endif
#ifndef KERN_PROC_PGRP
#define KERN_PROC_PGRP 4
#endif
#ifndef KERN_PROC_SESSION
#define KERN_PROC_SESSION 5
#endif
#ifndef KERN_PROC_TTY
#define KERN_PROC_TTY 6
#endif
#ifndef KERN_PROC_UID
#define KERN_PROC_UID 7
#endif
#ifndef KERN_PROC_RGID
#define KERN_PROC_RGID 8
#endif
#ifndef KERN_PROC_RUID
#define KERN_PROC_RUID 9
#endif
#ifndef KERN_PROC_INC_THREAD
#define KERN_PROC_INC_THREAD 0x10
#endif
#ifndef P_CONTROLT
#define P_CONTROLT 0x2
#endif
#ifndef SZOMB
#define SZOMB 5
#endif
#ifndef XO_STYLE_TEXT
#define XO_STYLE_TEXT 0
#endif
#define UNLIMITED 0
#ifndef S_ISCHR
#define S_ISCHR(m) (((m) & 0170000) == 0020000)
#endif

#define STAILQ_HEAD(name, type) \
    struct name { struct type *stqh_first; struct type **stqh_last; }
#define STAILQ_ENTRY(type) struct { struct type *stqe_next; }
#define STAILQ_FIRST(head) ((head)->stqh_first)
#define STAILQ_NEXT(elm, field) ((elm)->field.stqe_next)
#define STAILQ_EMPTY(head) (STAILQ_FIRST(head) == NULL)
#define STAILQ_INIT(head) do { \
    STAILQ_FIRST(head) = NULL; (head)->stqh_last = &STAILQ_FIRST(head); \
} while (0)
#define STAILQ_INSERT_TAIL(head, elm, field) do { \
    STAILQ_NEXT(elm, field) = NULL; \
    *(head)->stqh_last = (elm); \
    (head)->stqh_last = &STAILQ_NEXT(elm, field); \
} while (0)
#define STAILQ_REMOVE_HEAD(head, field) do { \
    if ((STAILQ_FIRST(head) = STAILQ_NEXT(STAILQ_FIRST(head), field)) == NULL) \
        (head)->stqh_last = &STAILQ_FIRST(head); \
} while (0)
#define STAILQ_FOREACH(var, head, field) \
    for ((var) = STAILQ_FIRST(head); (var); (var) = STAILQ_NEXT(var, field))
#define STAILQ_FOREACH_SAFE(var, head, field, tvar) \
    for ((var) = STAILQ_FIRST(head); \
        (var) && ((tvar) = STAILQ_NEXT(var, field), 1); (var) = (tvar))
#define STAILQ_REMOVE_AFTER(head, elm, field) do { \
    if ((STAILQ_NEXT(elm, field) = \
        STAILQ_NEXT(STAILQ_NEXT(elm, field), field)) == NULL) \
        (head)->stqh_last = &STAILQ_NEXT(elm, field); \
} while (0)
#define STAILQ_CONCAT(head1, head2) do { \
    if (!STAILQ_EMPTY(head2)) { \
        *(head1)->stqh_last = (head2)->stqh_first; \
        (head1)->stqh_last = (head2)->stqh_last; \
        STAILQ_INIT(head2); \
    } \
} while (0)
#define STAILQ_SWAP(head1, head2, type) do { \
    struct type *stqh_first_tmp; \
    struct type **stqh_last_tmp; \
    stqh_first_tmp = (head1)->stqh_first; \
    stqh_last_tmp = (head1)->stqh_last; \
    (head1)->stqh_first = (head2)->stqh_first; \
    (head1)->stqh_last = (head2)->stqh_last; \
    (head2)->stqh_first = stqh_first_tmp; \
    (head2)->stqh_last = stqh_last_tmp; \
    if (STAILQ_EMPTY(head1)) (head1)->stqh_last = &STAILQ_FIRST(head1); \
    if (STAILQ_EMPTY(head2)) (head2)->stqh_last = &STAILQ_FIRST(head2); \
} while (0)
#define STAILQ_SPLIT_AFTER(head, elm, rest, field) do { \
    if (STAILQ_NEXT((elm), field) == NULL) \
        STAILQ_INIT(rest); \
    else { \
        STAILQ_FIRST(rest) = STAILQ_NEXT((elm), field); \
        (rest)->stqh_last = (head)->stqh_last; \
        STAILQ_NEXT((elm), field) = NULL; \
        (head)->stqh_last = &STAILQ_NEXT((elm), field); \
    } \
} while (0)
#define STAILQ_HEAD_INITIALIZER(head) { NULL, &(head).stqh_first }

typedef int fixpt_t;
typedef long segsz_t;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned char u_char;

enum type { UNSPEC, CHAR, SCHAR, UCHAR, SHORT, USHORT, INT, UINT, LONG, ULONG, KPTR, PGTOK };

typedef struct kinfo_str {
    STAILQ_ENTRY(kinfo_str) ks_next;
    char *ks_str;
} KINFO_STR;

typedef struct kinfo_proc {
    pid_t ki_pid, ki_ppid, ki_pgid, ki_sid;
    uid_t ki_uid, ki_ruid;
    gid_t ki_rgid;
    int ki_jid;
    dev_t ki_tdev;
    long ki_flag;
    char ki_stat;
    char ki_comm[COMMLEN + 1];
    char ki_tdname[17];
    char ki_moretdname[17];
    int ki_numthreads;
    segsz_t ki_tsize, ki_dsize, ki_ssize;
    pid_t ki_tid;
} kinfo_proc;

typedef struct kinfo {
    struct kinfo_proc *ki_p;
    const char *ki_args;
    const char *ki_env;
    int ki_valid;
    double ki_pcpu;
    segsz_t ki_memsize;
    union { int level; char *prefix; } ki_d;
    STAILQ_HEAD(, kinfo_str) ki_ks;
} KINFO;

typedef struct varent {
    STAILQ_ENTRY(varent) next_ve;
    const char *header;
    const struct var *var;
    u_int width;
    uint16_t flags;
} VARENT;

struct var;
typedef struct var VAR;

struct var {
    const char *name;
    union { const char *aliased; const VAR *final_kw; };
    const char *header;
    const char *field;
    u_int flag;
    char *(*oproc)(struct kinfo *, struct varent *);
    size_t off;
    enum type type;
    const char *fmt;
};

#define COMM 0x01
#define LJUST 0x02
#define USER 0x04
#define VE_KEEP (1 << 0)

STAILQ_HEAD(velisthead, varent);

#ifndef TIOCGWINSZ
#define TIOCGWINSZ 0x5413
#endif

static inline size_t strlcpy(char *dst, const char *src, size_t siz)
{
    size_t n = strlen(src);
    if (siz != 0) {
        size_t copy = n < siz - 1 ? n : siz - 1;
        memcpy(dst, src, copy);
        dst[copy] = '\0';
    }
    return n;
}
static inline size_t strlcat(char *dst, const char *src, size_t siz)
{
    size_t dlen = strnlen(dst, siz);
    return dlen + strlcpy(dst + dlen, src, siz > dlen ? siz - dlen : 0);
}

typedef void *kvm_t;

extern jmp_buf b0334_err_jmp;
extern int b0334_err_jmp_set, b0334_errx_code, b0334_exit_jmp_set;

extern void xo_warnx(const char *, ...);
extern void xo_warn(const char *, ...);
extern void xo_err(int, const char *, ...);
extern void xo_errx(int, const char *, ...);
extern void xo_error(const char *, ...);
extern int xo_parse_args(int, char *[]);
extern void xo_open_container(const char *);
extern void xo_close_container(const char *);
extern void xo_open_list(const char *);
extern void xo_close_list(const char *);
extern void xo_open_instance(const char *);
extern void xo_close_instance(const char *);
extern int xo_emit(const char *, ...);
extern int xo_finish(void);
extern int xo_get_style(void *);

extern kvm_t *kvm_openfiles(const char *, const char *, const char *, int, char *);
extern struct kinfo_proc *kvm_getprocs(kvm_t *, int, int, int *);
extern char *kvm_geterr(kvm_t *);
extern char **kvm_getargv(kvm_t *, const struct kinfo_proc *, int);
extern char **kvm_getenvv(kvm_t *, const struct kinfo_proc *, int);

extern void parsefmt(const char *, struct velisthead *, int);
extern void printheader(void);
extern double getpcpu(const KINFO *);
extern void showkey(void);
extern void check_keywords(void);
extern void resolve_aliases(void);
extern const char *fmt_argv(char **, char *, char *, size_t);
extern size_t aliased_keyword_index(const VAR *);

extern int b0334_getopt(int, char *const[], const char *);
extern int b0334_jail_getid(const char *);
extern int b0334_getgrgid(gid_t, struct group *);
extern int b0334_getgrnam(const char *, struct group *);
extern int b0334_getpwnam(const char *, struct passwd *);
extern int b0334_getpwuid(uid_t, struct passwd *);
extern int b0334_stat(const char *, struct stat *);
extern char *b0334_ttyname(int);
extern int b0334_sysctl_pid_max(int *);
extern int b0334_ioctl_winsz(int, struct winsize *);
extern char *b0334_getenv(const char *);
extern uid_t b0334_geteuid(void);
extern const size_t known_keywords_nb;

static inline void b0334_trap_errx(int code, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    if (b0334_err_jmp_set) { va_end(ap); longjmp(b0334_err_jmp, code); }
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(code);
}
#undef errx
#define errx b0334_trap_errx
#define getopt b0334_getopt
#define jail_getid b0334_jail_getid

struct group *getgrgid(gid_t gid) {
    static struct group g;
    if (b0334_getgrgid(gid, &g)) return &g;
    return NULL;
}
struct group *getgrnam(const char *name) {
    static struct group g;
    if (b0334_getgrnam(name, &g)) return &g;
    return NULL;
}
struct passwd *getpwnam(const char *name) {
    static struct passwd p;
    if (b0334_getpwnam(name, &p)) return &p;
    return NULL;
}
struct passwd *getpwuid(uid_t uid) {
    static struct passwd p;
    if (b0334_getpwuid(uid, &p)) return &p;
    return NULL;
}
int stat(const char *path, struct stat *sb) { return b0334_stat(path, sb); }
char *ttyname(int fd) { return b0334_ttyname(fd); }
char *getenv(const char *name) { return b0334_getenv(name); }
uid_t geteuid(void) { return b0334_geteuid(); }
static int sysctlbyname(const char *name, void *oldp, size_t *oldlenp,
    void *newp, size_t newlen)
{
    (void)newp; (void)newlen;
    if (strcmp(name, "kern.pid_max") == 0 && oldp != NULL && oldlenp != NULL) {
        if (b0334_sysctl_pid_max((int *)oldp) == 0) {
            *oldlenp = sizeof(int);
            return 0;
        }
    }
    return -1;
}
int ioctl(int fd, unsigned long req, ...) {
    va_list ap;
    va_start(ap, req);
    void *arg = va_arg(ap, void *);
    va_end(ap);
    if (req == TIOCGWINSZ) return b0334_ioctl_winsz(fd, (struct winsize *)arg);
    return -1;
}

int b0334_optind = 0;
char *b0334_optarg = NULL;
VARENT *ref_find_varentry(const char *);
#endif

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 2025 The FreeBSD Foundation
 *
 * Portions of this software were developed by Olivier Certner
 * <olce@FreeBSD.org> at Kumacom SARL under sponsorship from the FreeBSD
 * Foundation.
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
 * ------+---------+---------+-------- + --------+---------+---------+---------*
 * Copyright (c) 2004  - Garance Alistair Drosehn <gad@FreeBSD.org>.
 * All rights reserved.
 *
 * Significant modifications made to bring `ps' options somewhat closer
 * to the standard for `ps' as described in SingleUnixSpec-v3.
 * ------+---------+---------+-------- + --------+---------+---------+---------*
 */




#define	_PATH_PTS	"/dev/pts/"

#define	W_SEP	" \t"		/* "Whitespace" list separators */
#define	T_SEP	","		/* "Terminate-element" list separators */

/*
 * isdigit takes an `int', but expects values in the range of unsigned char.
 * This wrapper ensures that values from a 'char' end up in the correct range.
 */
#define	isdigitch(Anychar) isdigit((u_char)(Anychar))

int	 cflag;			/* -c */
int	 eval;			/* Exit value */
time_t	 now;			/* Current time(3) value */
int	 rawcpu;		/* -C */
int	 sumrusage;		/* -S */
int	 termwidth;		/* Width of the screen (0 == infinity). */
int	 showthreads;		/* will threads be shown? */

struct keyword_info {
	/*
	 * Whether there is (at least) one column referencing this keyword that
	 * must be kept.
	 */
#define	KWI_HAS_MUST_KEEP_COLUMN	(1 << 0)
	/*
	 * Whether a column with such a keyword has been seen.
	 */
#define	KWI_SEEN			(1 << 1)
	u_int flags;
};

struct velisthead varlist = STAILQ_HEAD_INITIALIZER(varlist);
static struct velisthead Ovarlist = STAILQ_HEAD_INITIALIZER(Ovarlist);

static kvm_t	*kd;
static int	 needcomm;	/* -o "command" */
static int	 needenv;	/* -e */
static int	 needuser;	/* -o "user" */
static int	 optfatal;	/* Fatal error parsing some list-option. */
static int	 pid_max;	/* kern.pid_max */

static enum sort { DEFAULT, SORTMEM, SORTCPU } sortby = DEFAULT;

struct listinfo;
typedef	int	addelem_rtn(struct listinfo *_inf, const char *_elem);

struct listinfo {
	int		 count;
	int		 maxcount;
	int		 elemsize;
	addelem_rtn	*addelem;
	const char	*lname;
	union {
		gid_t	*gids;
		int	*jids;
		pid_t	*pids;
		dev_t	*ttys;
		uid_t	*uids;
		void	*ptr;
	} l;
};

static int	 ref_addelem_gid(struct listinfo *, const char *);
static int	 ref_addelem_jid(struct listinfo *, const char *);
static int	 ref_addelem_pid(struct listinfo *, const char *);
static int	 ref_addelem_tty(struct listinfo *, const char *);
static int	 ref_addelem_uid(struct listinfo *, const char *);
static void	 ref_add_list(struct listinfo *, const char *);
static void	 ref_descendant_sort(KINFO *, int);
static void	 ref_format_output(KINFO *);
static void	*ref_expand_list(struct listinfo *);
static const char *
		 ref_fmt(char **(*)(kvm_t *, const struct kinfo_proc *, int),
		    KINFO *, char *, char *, int);
static void	 ref_free_list(struct listinfo *);
static void	 ref_init_list(struct listinfo *, addelem_rtn, int, const char *);
static char	*ref_kludge_oldps_options(const char *, char *, const char *);
static int	 ref_pscomp(const void *, const void *);
static void	 ref_saveuser(KINFO *);
static void	 ref_scan_vars(struct keyword_info *);
static void	 ref_remove_redundant_columns(struct keyword_info *);
static void	 ref_pidmax_init(void);
static void	 ref_usage(void);

static const char dfmt[] = "pid,tt,state,time,command";
static const char jfmt[] = "user,pid,ppid,pgid,sid,jobc,state,tt,time,command";
static const char lfmt[] = "uid,pid,ppid,cpu,pri,nice,vsz,rss,mwchan,state,"
			   "tt,time,command";
static const char ufmt[] = "user,pid,%cpu,%mem,vsz,rss,tt,state,start,time,command";
static const char vfmt[] = "pid,state,time,sl,re,pagein,vsz,rss,lim,tsiz,"
			   "%cpu,%mem,command";
static const char Zfmt[] = "label";

#define	PS_ARGS	"AaCcD:defG:gHhjJ:LlM:mN:O:o:p:rSTt:U:uvwXxZ"

int
ref_main(int argc, char *argv[])
{
	struct listinfo gidlist, jidlist, pgrplist, pidlist;
	struct listinfo ruidlist, sesslist, ttylist, uidlist;
	struct kinfo_proc *kp;
	KINFO *kinfo = NULL, *next_KINFO;
	KINFO_STR *ks;
	struct varent *vent;
	struct winsize ws = { .ws_row = 0 };
	const char *nlistf, *memf, *str;
	char *cols;
	int all, ch, elem, flag, _fmt, i, lineno, linelen, left;
	int descendancy, nentries, nkept, nselectors;
	int prtheader, wflag, what, xkeep, xkeep_implied;
	int fwidthmin, fwidthmax;
	char errbuf[_POSIX2_LINE_MAX];
	char fmtbuf[_POSIX2_LINE_MAX];
	enum { NONE = 0, UP = 1, DOWN = 2, BOTH = 1 | 2 } directions = NONE;
	struct { int traversed; int initial; } pid_count;
	struct keyword_info *keywords_info;

	(void) setlocale(LC_ALL, "");
	time(&now);			/* Used by routines in print.c. */

	/*
	 * Compute default output line length before processing options.
	 * If COLUMNS is set, use it.  Otherwise, if this is part of an
	 * interactive job (i.e. one associated with a terminal), use
	 * the terminal width.  "Interactive" is determined by whether
	 * any of stdout, stderr, or stdin is a terminal.  The intent
	 * is that "ps", "ps | more", and "ps | grep" all use the same
	 * default line length unless -w is specified.
	 *
	 * If not interactive, the default length was traditionally 79.
	 * It has been changed to unlimited.  This is mostly for the
	 * benefit of non-interactive scripts, which arguably should
	 * use -ww, but is compatible with Linux.
	 */
	if ((cols = getenv("COLUMNS")) != NULL && *cols != '\0')
		termwidth = atoi(cols);
	else if ((ioctl(STDOUT_FILENO, TIOCGWINSZ, (char *)&ws) == -1 &&
	     ioctl(STDERR_FILENO, TIOCGWINSZ, (char *)&ws) == -1 &&
	     ioctl(STDIN_FILENO,  TIOCGWINSZ, (char *)&ws) == -1) ||
	     ws.ws_col == 0)
		termwidth = UNLIMITED;
	else
		termwidth = ws.ws_col - 1;

	/*
	 * Hide a number of option-processing kludges in a separate routine,
	 * to support some historical BSD behaviors, such as `ps axu'.
	 */
	if (argc > 1)
		argv[1] = ref_kludge_oldps_options(PS_ARGS, argv[1], argv[2]);

	ref_pidmax_init();

#ifdef PS_CHECK_KEYWORDS
	/* Check for obvious problems in the keywords array. */
	check_keywords();
	/* Resolve all aliases at start to spot errors. */
	resolve_aliases();
#endif

	all = descendancy = _fmt = nselectors = optfatal = 0;
	prtheader = showthreads = wflag = xkeep_implied = 0;
	xkeep = -1;			/* Neither -x nor -X. */
	ref_init_list(&gidlist, ref_addelem_gid, sizeof(gid_t), "group");
	ref_init_list(&jidlist, ref_addelem_jid, sizeof(int), "jail id");
	ref_init_list(&pgrplist, ref_addelem_pid, sizeof(pid_t), "process group");
	ref_init_list(&pidlist, ref_addelem_pid, sizeof(pid_t), "process id");
	ref_init_list(&ruidlist, ref_addelem_uid, sizeof(uid_t), "ruser");
	ref_init_list(&sesslist, ref_addelem_pid, sizeof(pid_t), "session id");
	ref_init_list(&ttylist, ref_addelem_tty, sizeof(dev_t), "tty");
	ref_init_list(&uidlist, ref_addelem_uid, sizeof(uid_t), "user");
	memf = _PATH_DEVNULL;
	nlistf = NULL;

	argc = xo_parse_args(argc, argv);
	if (argc < 0)
		exit(1);

	while ((ch = getopt(argc, argv, PS_ARGS)) != -1)
		switch (ch) {
		case 'A':
			all = xkeep = 1;
			break;
		case 'a':
			all = 1;
			break;
		case 'C':
			rawcpu = 1;
			break;
		case 'c':
			cflag = 1;
			break;
		case 'D': {
				size_t len = strlen(optarg);

				if (len <= 2 &&
					strncasecmp(optarg, "up", len) == 0)
					directions |= UP;
				else if (len <= 4 &&
					strncasecmp(optarg, "down", len) == 0)
					directions |= DOWN;
				else if (len <= 4 &&
					strncasecmp(optarg, "both", len) == 0)
					directions |= BOTH;
				else
					ref_usage();
				break;
			}
		case 'd':
			descendancy = 1;
			break;
		case 'e':			/* XXX set ufmt */
			needenv = 1;
			break;
		case 'f':
			/* compat */
			break;
		case 'G':
			ref_add_list(&gidlist, optarg);
			xkeep_implied = 1;
			nselectors++;
			break;
		case 'g':
#if 0
			/*
			 * XXX - This behavior is still under debate since it
			 *	conflicts with the (undocumented) `-g' option
			 *	and is non-standard.  However, it is the
			 *	behavior of most UNIX systems except
			 *	SunOS/Solaris/illumos (see next comment; see
			 *	also comment for '-s' below).
			 */
			ref_add_list(&pgrplist, optarg);
			xkeep_implied = 1;
			nselectors++;
			break;
#else
			/*
			 * The historical BSD-ish (from SunOS) behavior: Also
			 * display process group leaders (but we do not filter
			 * them out).
			 */
			break;			/* no-op */
#endif
		case 'H':
			showthreads = KERN_PROC_INC_THREAD;
			break;
		case 'h':
			prtheader = ws.ws_row > 5 ? ws.ws_row : 22;
			break;
		case 'J':
			ref_add_list(&jidlist, optarg);
			xkeep_implied = 1;
			nselectors++;
			break;
		case 'j':
			parsefmt(jfmt, &varlist, 0);
			_fmt = 1;
			break;
		case 'L':
			showkey();
			exit(0);
		case 'l':
			parsefmt(lfmt, &varlist, 0);
			_fmt = 1;
			break;
		case 'M':
			memf = optarg;
			break;
		case 'm':
			sortby = SORTMEM;
			break;
		case 'N':
			nlistf = optarg;
			break;
		case 'O':
			parsefmt(optarg, &Ovarlist, 1);
			break;
		case 'o':
			parsefmt(optarg, &varlist, 1);
			_fmt = 1;
			break;
		case 'p':
			ref_add_list(&pidlist, optarg);
			/*
			 * Note: `-p' does not *set* xkeep, but any values
			 * from pidlist are checked before xkeep is.  That
			 * way they are always matched, even if the user
			 * specifies `-X'.
			 */
			nselectors++;
			break;
		case 'r':
			sortby = SORTCPU;
			break;
		case 'S':
			sumrusage = 1;
			break;
#if 0
		case 's':
			/*
			 * XXX - This non-standard option is still under debate.
			 *	It is supported on Solaris, Linux, IRIX, and
			 *	OpenBSD but conflicts with '-s' on NetBSD.  This
			 *	is the same functionality as POSIX option '-g',
			 *	but the cited systems do not provide it under
			 *	'-g', only under '-s'.
			 */
			ref_add_list(&sesslist, optarg);
			xkeep_implied = 1;
			nselectors++;
			break;
#endif
		case 'T':
			if ((optarg = ttyname(STDIN_FILENO)) == NULL)
				xo_errx(1, "stdin: not a terminal");
			/* FALLTHROUGH */
		case 't':
			ref_add_list(&ttylist, optarg);
			xkeep_implied = 1;
			nselectors++;
			break;
		case 'U':
			ref_add_list(&ruidlist, optarg);
			xkeep_implied = 1;
			nselectors++;
			break;
		case 'u':
#if 0
			/*
			 * POSIX's '-u' behavior.
			 *
			 * This has not been activated because:
			 * 1. Option '-U' is a substitute for most users, and
			 *    those that care seem more likely to want to match
			 *    on the real user ID to display all processes
			 *    launched by some users.
			 * 2. '-u' has been a canned display on the BSDs for
			 *    a very long time (POLA).
			 */
			ref_add_list(&uidlist, optarg);
			xkeep_implied = 1;
			nselectors++;
			break;
#else
			/* Historical BSD's '-u'. */
			parsefmt(ufmt, &varlist, 0);
			sortby = SORTCPU;
			_fmt = 1;
			break;
#endif
		case 'v':
			parsefmt(vfmt, &varlist, 0);
			sortby = SORTMEM;
			_fmt = 1;
			break;
		case 'w':
			if (wflag)
				termwidth = UNLIMITED;
			else if (termwidth < 131 && termwidth != UNLIMITED)
				termwidth = 131;
			wflag++;
			break;
		case 'X':
			/*
			 * Note that `-X' and `-x' are not standard "selector"
			 * options. For most selector-options, we check *all*
			 * processes to see if any are matched by the given
			 * value(s).  After we have a set of all the matched
			 * processes, then `-X' and `-x' govern whether we
			 * modify that *matched* set for processes which do
			 * not have a controlling terminal.  `-X' causes
			 * those processes to be deleted from the matched
			 * set, while `-x' causes them to be kept.
			 */
			xkeep = 0;
			break;
		case 'x':
			xkeep = 1;
			break;
		case 'Z':
			parsefmt(Zfmt, &varlist, 0);
			break;
		case '?':
		default:
			ref_usage();
		}
	argc -= optind;
	argv += optind;

	/*
	 * If there arguments after processing all the options, attempt
	 * to treat them as a list of process ids.
	 */
	while (*argv) {
		if (!isdigitch(**argv))
			break;
		ref_add_list(&pidlist, *argv);
		argv++;
	}
	if (*argv) {
		xo_warnx("illegal argument: %s\n", *argv);
		ref_usage();
	}
	if (optfatal)
		exit(1);		/* Error messages already printed. */
	if (xkeep < 0)			/* Neither -X nor -x was specified. */
		xkeep = xkeep_implied;

	kd = kvm_openfiles(nlistf, memf, NULL, O_RDONLY, errbuf);
	if (kd == NULL)
		xo_errx(1, "%s", errbuf);

	if (!_fmt)
		parsefmt(dfmt, &varlist, 0);

	if (!STAILQ_EMPTY(&Ovarlist)) {
		VARENT *const pid_entry = ref_find_varentry("pid");

		/*
		 * We insert the keywords passed by '-O' after the process ID if
		 * specified, else at start.
		 */
		if (pid_entry != NULL) {
			struct velisthead rest;

			STAILQ_SPLIT_AFTER(&varlist, pid_entry, &rest, next_ve);
			STAILQ_CONCAT(&varlist, &Ovarlist);
			STAILQ_CONCAT(&varlist, &rest);
		}
		else {
			STAILQ_SWAP(&varlist, &Ovarlist, varent);
			STAILQ_CONCAT(&varlist, &Ovarlist);
		}
	}

	keywords_info = calloc(known_keywords_nb, sizeof(struct keyword_info));
	if (keywords_info == NULL)
		xo_errx(1, "malloc failed");
	/*
	 * Scan requested variables, noting which structures are needed and
	 * which keywords are specified.
	 */
	ref_scan_vars(keywords_info);
	/*
	 * Remove redundant columns from "canned" displays (see the callee's
	 * herald comment for more details).
	 */
	ref_remove_redundant_columns(keywords_info);
	free(keywords_info);
	keywords_info = NULL;

	if (all)
		/*
		 * We have to display all processes, regardless of other
		 * options.
		 */
		nselectors = 0;
	else if (nselectors == 0) {
		/*
		 * Default is to request our processes only.  As per POSIX, we
		 * match processes by their effective user IDs and we use our
		 * effective user ID as our own identity.
		 */
		ref_expand_list(&uidlist);
		uidlist.l.uids[uidlist.count++] = geteuid();
		nselectors = 1;
	}

	/*
	 * Get process list.  If the user requested just one selector-
	 * option, then kvm_getprocs can be asked to return just those
	 * processes.  Otherwise, have it return all processes, and
	 * then this routine will search that full list and select the
	 * processes which match any of the user's selector-options.
	 */
	what = showthreads != 0 ? KERN_PROC_ALL : KERN_PROC_PROC;
	flag = 0;
	if (nselectors == 1) {
		if (gidlist.count == 1) {
			what = KERN_PROC_RGID | showthreads;
			flag = *gidlist.l.gids;
			nselectors = 0;
		} else if (pgrplist.count == 1) {
			what = KERN_PROC_PGRP | showthreads;
			flag = *pgrplist.l.pids;
			nselectors = 0;
		} else if (pidlist.count == 1 && directions == NONE) {
			what = KERN_PROC_PID | showthreads;
			flag = *pidlist.l.pids;
			nselectors = 0;
		} else if (ruidlist.count == 1) {
			what = KERN_PROC_RUID | showthreads;
			flag = *ruidlist.l.uids;
			nselectors = 0;
		} else if (sesslist.count == 1) {
			what = KERN_PROC_SESSION | showthreads;
			flag = *sesslist.l.pids;
			nselectors = 0;
		} else if (ttylist.count == 1) {
			what = KERN_PROC_TTY | showthreads;
			flag = *ttylist.l.ttys;
			nselectors = 0;
		} else if (uidlist.count == 1) {
			what = KERN_PROC_UID | showthreads;
			flag = *uidlist.l.uids;
			nselectors = 0;
		}
	}

	/*
	 * select procs
	 */
	nentries = -1;
	kp = kvm_getprocs(kd, what, flag, &nentries);
	/*
	 * Ignore ESRCH to preserve behaviour of "ps -p nonexistent-pid"
	 * not reporting an error.
	 */
	if ((kp == NULL && errno != ESRCH) || (kp != NULL && nentries < 0))
		xo_errx(1, "%s", kvm_geterr(kd));
	nkept = 0;
	pid_count.initial = pidlist.count;
	if (directions & DOWN)
		for (elem = 0; elem < pidlist.count; elem++)
			for (i = 0; i < nentries; i++) {
				if (kp[i].ki_ppid == kp[i].ki_pid)
					continue;
				if (kp[i].ki_ppid == pidlist.l.pids[elem]) {
					if (pidlist.count >= pidlist.maxcount)
						ref_expand_list(&pidlist);
					pidlist.l.pids[pidlist.count++] = kp[i].ki_pid;
				}
			}
	pid_count.traversed = pidlist.count;
	if (directions & UP)
		for (elem = 0; elem < pidlist.count; elem++) {
			if (elem >= pid_count.initial && elem < pid_count.traversed)
				continue;
			for (i = 0; i < nentries; i++) {
				if (kp[i].ki_ppid == kp[i].ki_pid)
					continue;
				if (kp[i].ki_pid == pidlist.l.pids[elem]) {
					if (pidlist.count >= pidlist.maxcount)
						ref_expand_list(&pidlist);
					pidlist.l.pids[pidlist.count++] = kp[i].ki_ppid;
				}
			}
		}
	if (nentries > 0) {
		if ((kinfo = malloc(nentries * sizeof(*kinfo))) == NULL)
			xo_errx(1, "malloc failed");
		for (i = nentries; --i >= 0; ++kp) {
			/*
			 * If the user specified multiple selection-criteria,
			 * then keep any process matched by the inclusive OR
			 * of all the selection-criteria given.
			 */
			if (pidlist.count > 0) {
				for (elem = 0; elem < pidlist.count; elem++)
					if (kp->ki_pid == pidlist.l.pids[elem])
						goto keepit;
			}
			/*
			 * Note that we had to process pidlist before
			 * filtering out processes which do not have
			 * a controlling terminal.
			 */
			if (xkeep == 0) {
				if ((kp->ki_tdev == NODEV ||
				    (kp->ki_flag & P_CONTROLT) == 0))
					continue;
			}
			if (nselectors == 0)
				goto keepit;
			if (gidlist.count > 0) {
				for (elem = 0; elem < gidlist.count; elem++)
					if (kp->ki_rgid == gidlist.l.gids[elem])
						goto keepit;
			}
			if (jidlist.count > 0) {
				for (elem = 0; elem < jidlist.count; elem++)
					if (kp->ki_jid == jidlist.l.jids[elem])
						goto keepit;
			}
			if (pgrplist.count > 0) {
				for (elem = 0; elem < pgrplist.count; elem++)
					if (kp->ki_pgid ==
					    pgrplist.l.pids[elem])
						goto keepit;
			}
			if (ruidlist.count > 0) {
				for (elem = 0; elem < ruidlist.count; elem++)
					if (kp->ki_ruid ==
					    ruidlist.l.uids[elem])
						goto keepit;
			}
			if (sesslist.count > 0) {
				for (elem = 0; elem < sesslist.count; elem++)
					if (kp->ki_sid == sesslist.l.pids[elem])
						goto keepit;
			}
			if (ttylist.count > 0) {
				for (elem = 0; elem < ttylist.count; elem++)
					if (kp->ki_tdev == ttylist.l.ttys[elem])
						goto keepit;
			}
			if (uidlist.count > 0) {
				for (elem = 0; elem < uidlist.count; elem++)
					if (kp->ki_uid == uidlist.l.uids[elem])
						goto keepit;
			}
			/*
			 * This process did not match any of the user's
			 * selector-options, so skip the process.
			 */
			continue;

		keepit:
			next_KINFO = &kinfo[nkept];
			next_KINFO->ki_p = kp;
			next_KINFO->ki_d.level = 0;
			next_KINFO->ki_d.prefix = NULL;
			next_KINFO->ki_pcpu = getpcpu(next_KINFO);
			if (sortby == SORTMEM)
				next_KINFO->ki_memsize = kp->ki_tsize +
				    kp->ki_dsize + kp->ki_ssize;
			if (needuser)
				ref_saveuser(next_KINFO);
			nkept++;
		}
	}

	if (nkept == 0) {
		printheader();
		if (xo_finish() < 0)
			xo_err(1, "stdout");
		exit(1);
	}

	/*
	 * sort proc list
	 */
	qsort(kinfo, nkept, sizeof(KINFO), ref_pscomp);

	/*
	 * We want things in descendant order
	 */
	if (descendancy)
		ref_descendant_sort(kinfo, nkept);


	/*
	 * Prepare formatted output.
	 */
	for (i = 0; i < nkept; i++)
		ref_format_output(&kinfo[i]);

	/*
	 * Print header.
	 */
	xo_open_container("process-information");
	printheader();
	if (xo_get_style(NULL) != XO_STYLE_TEXT)
		termwidth = UNLIMITED;

	/*
	 * Output formatted lines.
	 */
	xo_open_list("process");
	for (i = lineno = 0; i < nkept; i++) {
		linelen = 0;
		xo_open_instance("process");
		STAILQ_FOREACH(vent, &varlist, next_ve) {
			ks = STAILQ_FIRST(&kinfo[i].ki_ks);
			STAILQ_REMOVE_HEAD(&kinfo[i].ki_ks, ks_next);
			/* Truncate rightmost column if necessary.  */
			fwidthmax = _POSIX2_LINE_MAX;
			if (STAILQ_NEXT(vent, next_ve) == NULL &&
			   termwidth != UNLIMITED && ks->ks_str != NULL) {
				left = termwidth - linelen;
				if (left > 0 && left < (int)strlen(ks->ks_str))
					fwidthmax = left;
			}

			str = ks->ks_str;
			if (str == NULL)
				str = "-";
			/* No padding for the last column, if it's LJUST. */
			fwidthmin = (xo_get_style(NULL) != XO_STYLE_TEXT ||
			    (STAILQ_NEXT(vent, next_ve) == NULL &&
			    (vent->var->flag & LJUST))) ? 0 : vent->width;
			snprintf(fmtbuf, sizeof(fmtbuf), "{:%s/%%%s%d..%dhs}",
			    vent->var->field ? vent->var->field : vent->var->name,
			    (vent->var->flag & LJUST) ? "-" : "",
			    fwidthmin, fwidthmax);
			xo_emit(fmtbuf, str);
			linelen += fwidthmin;

			if (ks->ks_str != NULL) {
				free(ks->ks_str);
				ks->ks_str = NULL;
			}
			free(ks);
			ks = NULL;

			if (STAILQ_NEXT(vent, next_ve) != NULL) {
				xo_emit("{P: }");
				linelen++;
			}
		}
	        xo_emit("\n");
		xo_close_instance("process");
		if (prtheader && lineno++ == prtheader - 4) {
			xo_emit("\n");
			printheader();
			lineno = 0;
		}
	}
	xo_close_list("process");
	xo_close_container("process-information");
	if (xo_finish() < 0)
		xo_err(1, "stdout");

	ref_free_list(&gidlist);
	ref_free_list(&jidlist);
	ref_free_list(&pidlist);
	ref_free_list(&pgrplist);
	ref_free_list(&ruidlist);
	ref_free_list(&sesslist);
	ref_free_list(&ttylist);
	ref_free_list(&uidlist);
	for (i = 0; i < nkept; i++)
		free(kinfo[i].ki_d.prefix);
	free(kinfo);

	exit(eval);
}

static int
ref_addelem_gid(struct listinfo *inf, const char *elem)
{
	struct group *grp;
	const char *nameorID;
	char *endp;
	u_long bigtemp;

	if (*elem == '\0' || strlen(elem) >= MAXLOGNAME) {
		if (*elem == '\0')
			xo_warnx("Invalid (zero-length) %s name", inf->lname);
		else
			xo_warnx("%s name too long: %s", inf->lname, elem);
		optfatal = 1;
		return (0);		/* Do not add this value. */
	}

	/*
	 * SUSv3 states that `ps -G grouplist' should match "real-group
	 * ID numbers", and does not mention group-names.  I do want to
	 * also support group-names, so this tries for a group-id first,
	 * and only tries for a name if that doesn't work.  This is the
	 * opposite order of what is done in addelem_uid(), but in
	 * practice the order would only matter for group-names which
	 * are all-numeric.
	 */
	grp = NULL;
	nameorID = "named";
	errno = 0;
	bigtemp = strtoul(elem, &endp, 10);
	if (errno == 0 && *endp == '\0' && bigtemp <= GID_MAX) {
		nameorID = "name or ID matches";
		grp = getgrgid((gid_t)bigtemp);
	}
	if (grp == NULL)
		grp = getgrnam(elem);
	if (grp == NULL) {
		xo_warnx("No %s %s '%s'", inf->lname, nameorID, elem);
		optfatal = 1;
		return (0);
	}
	if (inf->count >= inf->maxcount)
		ref_expand_list(inf);
	inf->l.gids[(inf->count)++] = grp->gr_gid;
	return (1);
}

static int
ref_addelem_jid(struct listinfo *inf, const char *elem)
{
	int tempid;

	if (*elem == '\0') {
		xo_warnx("Invalid (zero-length) jail id");
		optfatal = 1;
		return (0);		/* Do not add this value. */
	}

	tempid = jail_getid(elem);
	if (tempid < 0) {
		xo_warnx("Invalid %s: %s", inf->lname, elem);
		optfatal = 1;
		return (0);
	}

	if (inf->count >= inf->maxcount)
		ref_expand_list(inf);
	inf->l.jids[(inf->count)++] = tempid;
	return (1);
}

static int
ref_addelem_pid(struct listinfo *inf, const char *elem)
{
	char *endp;
	long tempid;

	if (*elem == '\0') {
		xo_warnx("Invalid (zero-length) process id");
		optfatal = 1;
		return (0);		/* Do not add this value. */
	}

	errno = 0;
	tempid = strtol(elem, &endp, 10);
	if (*endp != '\0' || tempid < 0 || elem == endp) {
		xo_warnx("Invalid %s: %s", inf->lname, elem);
		errno = ERANGE;
	} else if (errno != 0 || tempid > pid_max) {
		xo_warnx("%s too large: %s", inf->lname, elem);
		errno = ERANGE;
	}
	if (errno == ERANGE) {
		optfatal = 1;
		return (0);
	}
	if (inf->count >= inf->maxcount)
		ref_expand_list(inf);
	inf->l.pids[(inf->count)++] = tempid;
	return (1);
}

/*
 * The user can specify a device via one of three formats:
 *     1) fully qualified, e.g.:     /dev/ttyp0 /dev/console	/dev/pts/0
 *     2) missing "/dev", e.g.:      ttyp0      console		pts/0
 *     3) two-letters, e.g.:         p0         co		0
 *        (matching letters that would be seen in the "TT" column)
 */
static int
ref_addelem_tty(struct listinfo *inf, const char *elem)
{
	const char *ttypath;
	struct stat sb;
	char pathbuf[PATH_MAX], pathbuf2[PATH_MAX], pathbuf3[PATH_MAX];

	ttypath = NULL;
	pathbuf2[0] = '\0';
	pathbuf3[0] = '\0';
	switch (*elem) {
	case '/':
		ttypath = elem;
		break;
	case 'c':
		if (strcmp(elem, "co") == 0) {
			ttypath = _PATH_CONSOLE;
			break;
		}
		/* FALLTHROUGH */
	default:
		strlcpy(pathbuf, _PATH_DEV, sizeof(pathbuf));
		strlcat(pathbuf, elem, sizeof(pathbuf));
		ttypath = pathbuf;
		if (strncmp(pathbuf, _PATH_TTY, strlen(_PATH_TTY)) == 0)
			break;
		if (strncmp(pathbuf, _PATH_PTS, strlen(_PATH_PTS)) == 0)
			break;
		if (strcmp(pathbuf, _PATH_CONSOLE) == 0)
			break;
		/* Check to see if /dev/tty${elem} exists */
		strlcpy(pathbuf2, _PATH_TTY, sizeof(pathbuf2));
		strlcat(pathbuf2, elem, sizeof(pathbuf2));
		if (stat(pathbuf2, &sb) == 0 && S_ISCHR(sb.st_mode)) {
			/* No need to repeat stat() && S_ISCHR() checks */
			ttypath = NULL;
			break;
		}
		/* Check to see if /dev/pts/${elem} exists */
		strlcpy(pathbuf3, _PATH_PTS, sizeof(pathbuf3));
		strlcat(pathbuf3, elem, sizeof(pathbuf3));
		if (stat(pathbuf3, &sb) == 0 && S_ISCHR(sb.st_mode)) {
			/* No need to repeat stat() && S_ISCHR() checks */
			ttypath = NULL;
			break;
		}
		break;
	}
	if (ttypath) {
		if (stat(ttypath, &sb) == -1) {
			if (pathbuf3[0] != '\0')
				xo_warn("%s, %s, and %s", pathbuf3, pathbuf2,
				    ttypath);
			else
				xo_warn("%s", ttypath);
			optfatal = 1;
			return (0);
		}
		if (!S_ISCHR(sb.st_mode)) {
			if (pathbuf3[0] != '\0')
				xo_warnx("%s, %s, and %s: Not a terminal",
				    pathbuf3, pathbuf2, ttypath);
			else
				xo_warnx("%s: Not a terminal", ttypath);
			optfatal = 1;
			return (0);
		}
	}
	if (inf->count >= inf->maxcount)
		ref_expand_list(inf);
	inf->l.ttys[(inf->count)++] = sb.st_rdev;
	return (1);
}

static int
ref_addelem_uid(struct listinfo *inf, const char *elem)
{
	struct passwd *pwd;
	char *endp;
	u_long bigtemp;

	if (*elem == '\0' || strlen(elem) >= MAXLOGNAME) {
		if (*elem == '\0')
			xo_warnx("Invalid (zero-length) %s name", inf->lname);
		else
			xo_warnx("%s name too long: %s", inf->lname, elem);
		optfatal = 1;
		return (0);		/* Do not add this value. */
	}

	pwd = getpwnam(elem);
	if (pwd == NULL) {
		errno = 0;
		bigtemp = strtoul(elem, &endp, 10);
		if (errno != 0 || *endp != '\0' || bigtemp > UID_MAX)
			xo_warnx("No %s named '%s'", inf->lname, elem);
		else {
			/* The string is all digits, so it might be a userID. */
			pwd = getpwuid((uid_t)bigtemp);
			if (pwd == NULL)
				xo_warnx("No %s name or ID matches '%s'",
				    inf->lname, elem);
		}
	}
	if (pwd == NULL) {
		/*
		 * These used to be treated as minor warnings (and the
		 * option was simply ignored), but now they are fatal
		 * errors (and the command will be aborted).
		 */
		optfatal = 1;
		return (0);
	}
	if (inf->count >= inf->maxcount)
		ref_expand_list(inf);
	inf->l.uids[(inf->count)++] = pwd->pw_uid;
	return (1);
}

static void
ref_add_list(struct listinfo *inf, const char *argp)
{
	const char *savep;
	char *cp, *endp;
	int toolong;
	char elemcopy[PATH_MAX];

	if (*argp == '\0')
		inf->addelem(inf, argp);
	while (*argp != '\0') {
		while (*argp != '\0' && strchr(W_SEP, *argp) != NULL)
			argp++;
		savep = argp;
		toolong = 0;
		cp = elemcopy;
		if (strchr(T_SEP, *argp) == NULL) {
			endp = elemcopy + sizeof(elemcopy) - 1;
			while (*argp != '\0' && cp <= endp &&
			    strchr(W_SEP T_SEP, *argp) == NULL)
				*cp++ = *argp++;
			if (cp > endp)
				toolong = 1;
		}
		if (!toolong) {
			*cp = '\0';
			/*
			 * Add this single element to the given list.
			 */
			inf->addelem(inf, elemcopy);
		} else {
			/*
			 * The string is too long to copy.  Find the end
			 * of the string to print out the warning message.
			 */
			while (*argp != '\0' && strchr(W_SEP T_SEP,
			    *argp) == NULL)
				argp++;
			xo_warnx("Value too long: %.*s", (int)(argp - savep),
			    savep);
			optfatal = 1;
		}
		/*
		 * Skip over any number of trailing whitespace characters,
		 * but only one (at most) trailing element-terminating
		 * character.
		 */
		while (*argp != '\0' && strchr(W_SEP, *argp) != NULL)
			argp++;
		if (*argp != '\0' && strchr(T_SEP, *argp) != NULL) {
			argp++;
			/* Catch case where string ended with a comma. */
			if (*argp == '\0')
				inf->addelem(inf, argp);
		}
	}
}

static void
ref_descendant_sort(KINFO *ki, int items)
{
	int dst, lvl, maxlvl, n, ndst, nsrc, siblings, src;
	unsigned char *path;
	KINFO kn;

	/*
	 * First, sort the entries by descendancy, tracking the descendancy
	 * depth in the ki_d.level field.
	 */
	src = 0;
	maxlvl = 0;
	while (src < items) {
		if (ki[src].ki_d.level) {
			src++;
			continue;
		}
		for (nsrc = 1; src + nsrc < items; nsrc++)
			if (!ki[src + nsrc].ki_d.level)
				break;

		for (dst = 0; dst < items; dst++) {
			if (ki[dst].ki_p->ki_pid == ki[src].ki_p->ki_pid)
				continue;
			if (ki[dst].ki_p->ki_pid == ki[src].ki_p->ki_ppid)
				break;
		}

		if (dst == items) {
			src += nsrc;
			continue;
		}

		for (ndst = 1; dst + ndst < items; ndst++)
			if (ki[dst + ndst].ki_d.level <= ki[dst].ki_d.level)
				break;

		for (n = src; n < src + nsrc; n++) {
			ki[n].ki_d.level += ki[dst].ki_d.level + 1;
			if (maxlvl < ki[n].ki_d.level)
				maxlvl = ki[n].ki_d.level;
		}

		while (nsrc) {
			if (src < dst) {
				kn = ki[src];
				memmove(ki + src, ki + src + 1,
				    (dst - src + ndst - 1) * sizeof *ki);
				ki[dst + ndst - 1] = kn;
				nsrc--;
				dst--;
				ndst++;
			} else if (src != dst + ndst) {
				kn = ki[src];
				memmove(ki + dst + ndst + 1, ki + dst + ndst,
				    (src - dst - ndst) * sizeof *ki);
				ki[dst + ndst] = kn;
				ndst++;
				nsrc--;
				src++;
			} else {
				ndst += nsrc;
				src += nsrc;
				nsrc = 0;
			}
		}
	}

	/*
	 * Now populate ki_d.prefix (instead of ki_d.level) with the command
	 * prefix used to show descendancies.
	 */
	path = calloc((maxlvl + 7) / 8, sizeof(unsigned char));
	for (src = 0; src < items; src++) {
		if ((lvl = ki[src].ki_d.level) == 0) {
			ki[src].ki_d.prefix = NULL;
			continue;
		}
		if ((ki[src].ki_d.prefix = malloc(lvl * 2 + 1)) == NULL)
			xo_errx(1, "malloc failed");
		for (n = 0; n < lvl - 2; n++) {
			ki[src].ki_d.prefix[n * 2] =
			    path[n / 8] & 1 << (n % 8) ? '|' : ' ';
			ki[src].ki_d.prefix[n * 2 + 1] = ' ';
		}
		if (n == lvl - 2) {
			/* Have I any more siblings? */
			for (siblings = 0, dst = src + 1; dst < items; dst++) {
				if (ki[dst].ki_d.level > lvl)
					continue;
				if (ki[dst].ki_d.level == lvl)
					siblings = 1;
				break;
			}
			if (siblings)
				path[n / 8] |= 1 << (n % 8);
			else
				path[n / 8] &= ~(1 << (n % 8));
			ki[src].ki_d.prefix[n * 2] = siblings ? '|' : '`';
			ki[src].ki_d.prefix[n * 2 + 1] = '-';
			n++;
		}
		strcpy(ki[src].ki_d.prefix + n * 2, "- ");
	}
	free(path);
}

static void *
ref_expand_list(struct listinfo *inf)
{
	void *newlist;
	int newmax;

	newmax = (inf->maxcount + 1) << 1;
	newlist = realloc(inf->l.ptr, newmax * inf->elemsize);
	if (newlist == NULL) {
		free(inf->l.ptr);
		xo_errx(1, "realloc to %d %ss failed", newmax, inf->lname);
	}
	inf->maxcount = newmax;
	inf->l.ptr = newlist;

	return (newlist);
}

static void
ref_free_list(struct listinfo *inf)
{

	inf->count = inf->elemsize = inf->maxcount = 0;
	if (inf->l.ptr != NULL)
		free(inf->l.ptr);
	inf->addelem = NULL;
	inf->lname = NULL;
	inf->l.ptr = NULL;
}

static void
ref_init_list(struct listinfo *inf, addelem_rtn artn, int elemsize,
    const char *lname)
{

	inf->count = inf->maxcount = 0;
	inf->elemsize = elemsize;
	inf->addelem = artn;
	inf->lname = lname;
	inf->l.ptr = NULL;
}

VARENT *
ref_find_varentry(const char *name)
{
	struct varent *vent;

	STAILQ_FOREACH(vent, &varlist, next_ve) {
		if (strcmp(vent->var->name, name) == 0)
			return vent;
	}
	return NULL;
}

static void
ref_scan_vars(struct keyword_info *const keywords_info)
{
	struct varent *vent;
	const VAR *v;

	STAILQ_FOREACH(vent, &varlist, next_ve) {
		v = vent->var;
		if (v->flag & USER)
			needuser = 1;
		if (v->flag & COMM)
			needcomm = 1;
		if ((vent->flags & VE_KEEP) != 0)
			keywords_info[aliased_keyword_index(v)].flags |=
			    KWI_HAS_MUST_KEEP_COLUMN;
	}
}

/*
 * For each explicitly requested keyword, remove all the same keywords
 * from "canned" displays.  If the same keyword appears multiple times
 * only in "canned displays", then keep the first (leftmost) occurence
 * only (with the reasoning that columns requested first are the most
 * important as their positions catch the eye more).
 */
static void
ref_remove_redundant_columns(struct keyword_info *const keywords_info)
{
	struct varent *prev_vent, *vent, *next_vent;

	prev_vent = NULL;
	STAILQ_FOREACH_SAFE(vent, &varlist, next_ve, next_vent) {
		const VAR *const v = vent->var;
		struct keyword_info *const kwi =
		    &keywords_info[aliased_keyword_index(v)];

		/*
		 * If the current column is not marked as to absolutely keep,
		 * and we have either already output one with the same keyword
		 * or know we will output one later, remove it.
		 */
		if ((vent->flags & VE_KEEP) == 0 &&
		    (kwi->flags & (KWI_HAS_MUST_KEEP_COLUMN | KWI_SEEN)) != 0) {
			if (prev_vent == NULL)
				STAILQ_REMOVE_HEAD(&varlist, next_ve);
			else
				STAILQ_REMOVE_AFTER(&varlist, prev_vent,
				    next_ve);
		} else
			prev_vent = vent;


		kwi->flags |= KWI_SEEN;
	}
}

static void
ref_format_output(KINFO *ki)
{
	struct varent *vent;
	const VAR *v;
	KINFO_STR *ks;
	char *str;
	u_int len;

	STAILQ_INIT(&ki->ki_ks);
	STAILQ_FOREACH(vent, &varlist, next_ve) {
		v = vent->var;
		str = (v->oproc)(ki, vent);
		ks = malloc(sizeof(*ks));
		if (ks == NULL)
			xo_errx(1, "malloc failed");
		ks->ks_str = str;
		STAILQ_INSERT_TAIL(&ki->ki_ks, ks, ks_next);
		if (str != NULL) {
			len = strlen(str);
		} else
			len = 1; /* "-" */
		if (vent->width < len)
			vent->width = len;
	}
}

static const char *
ref_fmt(char **(*fn)(kvm_t *, const struct kinfo_proc *, int), KINFO *ki,
    char *comm, char *thread, int maxlen)
{
	const char *s;

	s = fmt_argv((*fn)(kd, ki->ki_p, termwidth), comm,
	    showthreads && ki->ki_p->ki_numthreads > 1 ? thread : NULL, maxlen);
	return (s);
}

static void
ref_saveuser(KINFO *ki)
{
	char tdname[COMMLEN + 1];

	ki->ki_valid = 1;

	/*
	 * save arguments if needed
	 */
	if (needcomm) {
		if (ki->ki_p->ki_stat == SZOMB) {
			ki->ki_args = strdup("<defunct>");
		} else {
			(void)snprintf(tdname, sizeof(tdname), "%s%s",
			    ki->ki_p->ki_tdname, ki->ki_p->ki_moretdname);
			ki->ki_args = ref_fmt(kvm_getargv, ki,
			    ki->ki_p->ki_comm, tdname, COMMLEN * 2 + 1);
		}
		if (ki->ki_args == NULL)
			xo_errx(1, "malloc failed");
	} else {
		ki->ki_args = NULL;
	}
	if (needenv) {
		ki->ki_env = ref_fmt(kvm_getenvv, ki, (char *)NULL,
		    (char *)NULL, 0);
		if (ki->ki_env == NULL)
			xo_errx(1, "malloc failed");
	} else {
		ki->ki_env = NULL;
	}
}

/* A macro used to improve the readability of pscomp(). */
#define	DIFF_RETURN(a, b, field) do {	\
	if ((a)->field != (b)->field)	\
		return (((a)->field < (b)->field) ? -1 : 1);	\
} while (0)

static int
ref_pscomp(const void *a, const void *b)
{
	const KINFO *ka, *kb;

	ka = a;
	kb = b;
	/* SORTCPU and SORTMEM are sorted in descending order. */
	if (sortby == SORTCPU)
		DIFF_RETURN(kb, ka, ki_pcpu);
	if (sortby == SORTMEM)
		DIFF_RETURN(kb, ka, ki_memsize);
	/*
	 * TTY's are sorted in ascending order, except that all NODEV
	 * processes come before all processes with a device.
	 */
	if (ka->ki_p->ki_tdev != kb->ki_p->ki_tdev) {
		if (ka->ki_p->ki_tdev == NODEV)
			return (-1);
		if (kb->ki_p->ki_tdev == NODEV)
			return (1);
		DIFF_RETURN(ka, kb, ki_p->ki_tdev);
	}

	/* PID's and TID's (threads) are sorted in ascending order. */
	DIFF_RETURN(ka, kb, ki_p->ki_pid);
	DIFF_RETURN(ka, kb, ki_p->ki_tid);
	return (0);
}
#undef DIFF_RETURN

/*
 * ICK (all for getopt), would rather hide the ugliness
 * here than taint the main code.
 *
 *  ps foo -> ps -foo
 *  ps 34 -> ps -p34
 *
 * The old convention that 't' with no trailing tty arg means the users
 * tty, is only supported if argv[1] doesn't begin with a '-'.  This same
 * feature is available with the option 'T', which takes no argument.
 */
static char *
ref_kludge_oldps_options(const char *optlist, char *origval, const char *nextarg)
{
	size_t len;
	char *argp, *cp, *newopts, *ns, *optp, *pidp;

	/*
	 * See if the original value includes any option which takes an
	 * argument (and will thus use up the remainder of the string).
	 */
	argp = NULL;
	if (optlist != NULL) {
		for (cp = origval; *cp != '\0'; cp++) {
			optp = strchr(optlist, *cp);
			if ((optp != NULL) && *(optp + 1) == ':') {
				argp = cp;
				break;
			}
		}
	}
	if (argp != NULL && *origval == '-')
		return (origval);

	/*
	 * if last letter is a 't' flag with no argument (in the context
	 * of the oldps options -- option string NOT starting with a '-' --
	 * then convert to 'T' (meaning *this* terminal, i.e. ttyname(0)).
	 *
	 * However, if a flag accepting a string argument is found earlier
	 * in the option string (including a possible `t' flag), then the
	 * remainder of the string must be the argument to that flag; so
	 * do not modify that argument.  Note that a trailing `t' would
	 * cause argp to be set, if argp was not already set by some
	 * earlier option.
	 */
	len = strlen(origval);
	cp = origval + len - 1;
	pidp = NULL;
	if (*cp == 't' && *origval != '-' && cp == argp) {
		if (nextarg == NULL || *nextarg == '-' || isdigitch(*nextarg))
			*cp = 'T';
	} else if (argp == NULL) {
		/*
		 * The original value did not include any option which takes
		 * an argument (and that would include `p' and `t'), so check
		 * the value for trailing number, or comma-separated list of
		 * numbers, which we will treat as a pid request.
		 */
		if (isdigitch(*cp)) {
			while (cp >= origval && (*cp == ',' || isdigitch(*cp)))
				--cp;
			pidp = cp + 1;
		}
	}

	/*
	 * If nothing needs to be added to the string, then return
	 * the "original" (although possibly modified) value.
	 */
	if (*origval == '-' && pidp == NULL)
		return (origval);

	/*
	 * Create a copy of the string to add '-' and/or 'p' to the
	 * original value.
	 */
	if ((newopts = ns = malloc(len + 3)) == NULL)
		xo_errx(1, "malloc failed");

	if (*origval != '-')
		*ns++ = '-';	/* add option flag */

	if (pidp == NULL)
		strcpy(ns, origval);
	else {
		/*
		 * Copy everything before the pid string, add the `p',
		 * and then copy the pid string.
		 */
		len = pidp - origval;
		memcpy(ns, origval, len);
		ns += len;
		*ns++ = 'p';
		strcpy(ns, pidp);
	}

	return (newopts);
}

static void
ref_pidmax_init(void)
{
	size_t intsize;

	intsize = sizeof(pid_max);
	if (sysctlbyname("kern.pid_max", &pid_max, &intsize, NULL, 0) < 0) {
		xo_warn("unable to read kern.pid_max");
		pid_max = 99999;
	}
}

static void __dead2
ref_usage(void)
{
#define	SINGLE_OPTS	"[-aCcdeHhjlmrSTuvwXxZ]"

	xo_error("%s\n%s\n%s\n%s\n%s\n",
	    "usage: ps [--libxo] " SINGLE_OPTS " [-O fmt | -o fmt]",
	    "          [-G gid[,gid...]] [-J jid[,jid...]] [-M core] [-N system]",
	    "          [-p pid[,pid...]] [-t tty[,tty...]] [-U user[,user...]]",
	    "          [-D up | down | both]",
	    "       ps [--libxo] -L");
	exit(1);
}

