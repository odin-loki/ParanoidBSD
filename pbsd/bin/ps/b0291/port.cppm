module;

#ifndef B0291_PS_BATCH
#define B0291_PS_BATCH

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vis.h>

#ifndef LONG_BIT
#define LONG_BIT (8 * (int)sizeof(long))
#endif

#ifndef nitems
#define nitems(x) (sizeof(x) / sizeof((x)[0]))
#endif

typedef int fixpt_t;
typedef unsigned long vm_size_t;
typedef long segsz_t;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned char u_char;
typedef unsigned short u_short;

#define STAILQ_HEAD(name, type) \
struct name { struct type *stqh_first; struct type **stqh_last; }
#define STAILQ_ENTRY(type) \
struct { struct type *stqe_next; }
#define STAILQ_FIRST(head) ((head)->stqh_first)
#define STAILQ_NEXT(elm, field) ((elm)->field.stqe_next)
#define STAILQ_EMPTY(head) (STAILQ_FIRST(head) == NULL)
#define STAILQ_INIT(head) do { \
	STAILQ_FIRST((head)) = NULL; (head)->stqh_last = &STAILQ_FIRST((head)); \
} while (0)
#define STAILQ_INSERT_TAIL(head, elm, field) do { \
	STAILQ_NEXT((elm), field) = NULL; \
	*(head)->stqh_last = (elm); \
	(head)->stqh_last = &STAILQ_NEXT((elm), field); \
} while (0)
#define STAILQ_FOREACH(var, head, field) \
	for ((var) = STAILQ_FIRST((head)); (var); (var) = STAILQ_NEXT((var), field))

#define UNLIMITED 0
enum type { UNSPEC, CHAR, SCHAR, UCHAR, SHORT, USHORT, INT, UINT, LONG, ULONG, KPTR, PGTOK };

typedef struct kinfo_str {
	STAILQ_ENTRY(kinfo_str) ks_next;
	char *ks_str;
} KINFO_STR;

typedef struct kinfo {
	struct kinfo_proc *ki_p;
	const char *ki_args;
	const char *ki_env;
	int ki_valid;
	double ki_pcpu;
	segsz_t ki_memsize;
	union {
		int level;
		char *prefix;
	} ki_d;
	STAILQ_HEAD(, kinfo_str) ki_ks;
} KINFO;

typedef struct varent {
	STAILQ_ENTRY(varent) next_ve;
	const char *header;
	const struct var *var;
	u_int width;
#define VE_KEEP (1 << 0)
	uint16_t flags;
} VARENT;

struct var;
typedef struct var VAR;

struct var {
	const char *name;
	union {
		const char *aliased;
		const VAR *final_kw;
	};
	const char *header;
	const char *field;
#define COMM 0x01
#define LJUST 0x02
#define USER 0x04
#define INF127 0x10
#define NOINHERIT 0x1000
#define RESOLVING_ALIAS 0x10000
#define RESOLVED_ALIAS 0x20000
	u_int flag;
	char *(*oproc)(struct kinfo *, struct varent *);
	size_t off;
	enum type type;
	const char *fmt;
};

STAILQ_HEAD(velisthead, varent);

#define SIDL 1
#define SRUN 2
#define SSLEEP 3
#define SSTOP 4
#define SZOMB 5
#define SWAIT 6
#define SLOCK 7

#define TDF_SINTR 0x00000008
#define MAXSLP 20

#define P_CONTROLT 0x00000002
#define P_PPWAIT 0x00000010
#define P_SYSTEM 0x00000200
#define P_TRACED 0x00000800
#define P_WEXIT 0x00002000
#define P_JAILED 0x01000000

#define KI_CTTY 0x00000001
#define KI_SLEADER 0x00000002
#define KI_LOCKBLOCK 0x00000004
#define KI_CRF_CAPABILITY_MODE 0x00000001

#define PRI_ITHD 1
#define PRI_REALTIME 2
#define PRI_TIMESHARE 3
#define PRI_IDLE 4
#define PRI_MIN_REALTIME 8
#define PRI_MIN_KERN 40
#define PRI_MIN_TIMESHARE 56
#define PRI_MIN_IDLE 224
#define PRI_MIN_ITHD 0
#define PUSER PRI_MIN_TIMESHARE
#define NZERO 20

#define RTP_PRIO_REALTIME PRI_REALTIME
#define RTP_PRIO_NORMAL PRI_TIMESHARE
#define RTP_PRIO_IDLE PRI_IDLE
#define RTP_PRIO_ITHD PRI_ITHD

#define NOCPU (-1)
#define NODEV ((dev_t)-1)
#ifndef S_IFCHR
#define S_IFCHR 0020000
#endif

#define WMESGLEN 8
#define LOCKNAMELEN 8
#define TDNAMLEN 16
#define COMMLEN 19
#define KI_EMULNAMELEN 16
#define KI_NGROUPS 16
#define LOGNAMELEN 17
#define LOGINCLASSLEN 17
#define MAXCOMLEN 32

struct priority {
	u_char pri_class;
	u_char pri_level;
	u_char pri_native;
	u_char pri_user;
};

struct kinfo_proc {
	int ki_structsize;
	int ki_layout;
	void *ki_args;
	void *ki_paddr;
	void *ki_addr;
	void *ki_tracep;
	void *ki_textvp;
	void *ki_fd;
	void *ki_vmspace;
	const void *ki_wchan;
	pid_t ki_pid;
	pid_t ki_ppid;
	pid_t ki_pgid;
	pid_t ki_tpgid;
	pid_t ki_sid;
	pid_t ki_tsid;
	short ki_jobc;
	short ki_spare_short1;
	uint32_t ki_tdev_freebsd11;
	uint32_t ki_siglist[4];
	uint32_t ki_sigmask[4];
	uint32_t ki_sigignore[4];
	uint32_t ki_sigcatch[4];
	uid_t ki_uid;
	uid_t ki_ruid;
	uid_t ki_svuid;
	gid_t ki_rgid;
	gid_t ki_svgid;
	short ki_ngroups;
	short ki_spare_short2;
	gid_t ki_groups[KI_NGROUPS];
	vm_size_t ki_size;
	segsz_t ki_rssize;
	segsz_t ki_swrss;
	segsz_t ki_tsize;
	segsz_t ki_dsize;
	segsz_t ki_ssize;
	u_short ki_xstat;
	u_short ki_acflag;
	fixpt_t ki_pctcpu;
	u_int ki_estcpu;
	u_int ki_slptime;
	u_int ki_swtime;
	u_int ki_cow;
	uint64_t ki_runtime;
	struct timeval ki_start;
	struct timeval ki_childtime;
	long ki_flag;
	long ki_kiflag;
	int ki_traceflag;
	char ki_stat;
	signed char ki_nice;
	char ki_lock;
	char ki_rqindex;
	u_char ki_oncpu_old;
	u_char ki_lastcpu_old;
	char ki_tdname[TDNAMLEN + 1];
	char ki_wmesg[WMESGLEN + 1];
	char ki_login[LOGNAMELEN + 1];
	char ki_lockname[LOCKNAMELEN + 1];
	char ki_comm[COMMLEN + 1];
	char ki_emul[KI_EMULNAMELEN + 1];
	char ki_loginclass[LOGINCLASSLEN + 1];
	char ki_moretdname[MAXCOMLEN - TDNAMLEN + 1];
	char ki_sparestrings[38];
	int ki_spareints[2];
	pid_t ki_reaper;
	pid_t ki_reapsubtree;
	uint64_t ki_tdev;
	int ki_oncpu;
	int ki_lastcpu;
	int ki_tracer;
	int ki_flag2;
	int ki_fibnum;
	u_int ki_cr_flags;
	int ki_jid;
	int ki_numthreads;
	long ki_tdflags;
	struct priority ki_pri;
	struct rusage ki_rusage;
	struct timeval ki_childutime;
	struct timeval ki_childstime;
};

#define KOFF(x) offsetof(struct kinfo_proc, x)
#define ROFF(x) offsetof(struct rusage, x)

#define LWPFMT "d"
#define NLWPFMT "d"
#define UIDFMT "u"
#define PIDFMT "d"

#define COMMAND_WIDTH 16
#define ARGUMENTS_WIDTH 16
#define ps_pgtok(a) (((a) * getpagesize()) / 1024)

extern fixpt_t ccpu;
extern int cflag, eval, fscale, nlistread, rawcpu;
extern unsigned long mempages;
extern time_t now;
extern int showthreads, sumrusage, termwidth;
extern struct velisthead varlist;
extern const size_t known_keywords_nb;

extern char *user_from_uid(uid_t uid, int nogroup);
extern char *group_from_gid(gid_t gid, int nogroup);
extern char *devname(dev_t dev, mode_t type);
extern char *jail_getname(int jid);
extern int donlist(void);

typedef void *mac_t;
extern int mac_prepare_process_label(mac_t *);
extern int mac_get_pid(pid_t, mac_t);
extern int mac_to_text(mac_t, char **);
extern void mac_free(mac_t);

extern void xo_warnx(const char *fmt, ...);
extern void xo_warn(const char *fmt, ...);
extern void xo_err(int eval, const char *fmt, ...);
extern void xo_errx(int eval, const char *fmt, ...);
extern void xo_open_list(const char *name);
extern int xo_emit(const char *fmt, ...);
extern void xo_close_list(const char *name);
extern int xo_finish(void);

extern jmp_buf b0291_err_jmp;
extern int b0291_err_jmp_set;
extern int b0291_errx_code;

static inline void b0291_errx(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (b0291_err_jmp_set)
		longjmp(b0291_err_jmp, eval);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(eval);
}
#undef errx
#define errx b0291_errx

#endif /* B0291_PS_BATCH */

export module pbsd.bin.ps.b0291;

export namespace pbsd::bin_ps::b0291 {

/* ---- fmt.c ---- */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993, 1994
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




static char *cmdpart(char *);
static char *shquote(char **);

static char *
shquote(char **argv)
{
	long arg_max;
	static size_t buf_size;
	size_t len;
	char **p, *dst, *src;
	static char *buf = NULL;

	if (buf == NULL) {
		if ((arg_max = sysconf(_SC_ARG_MAX)) == -1)
			errx(1, "sysconf _SC_ARG_MAX failed");
		if (arg_max >= LONG_MAX / 4 || arg_max >= (long)(SIZE_MAX / 4))
			errx(1, "sysconf _SC_ARG_MAX preposterously large");
		buf_size = 4 * arg_max + 1;
		if ((buf = malloc(buf_size)) == NULL)
			errx(1, "malloc failed");
	}

	if (*argv == NULL) {
		buf[0] = '\0';
		return (buf);
	}
	dst = buf;
	for (p = argv; (src = *p++) != NULL; ) {
		if (*src == '\0')
			continue;
		len = (buf_size - 1 - (dst - buf)) / 4;
		strvisx(dst, src, strlen(src) < len ? strlen(src) : len,
		    VIS_NL | VIS_CSTYLE);
		while (*dst != '\0')
			dst++;
		if ((buf_size - 1 - (dst - buf)) / 4 > 0)
			*dst++ = ' ';
	}
	/* Chop off trailing space */
	if (dst != buf && dst[-1] == ' ')
		dst--;
	*dst = '\0';
	return (buf);
}

static char *
cmdpart(char *arg0)
{
	char *cp;

	return ((cp = strrchr(arg0, '/')) != NULL ? cp + 1 : arg0);
}

const char *
fmt_argv(char **argv, char *cmd, char *thread, size_t maxlen)
{
	size_t len;
	char *ap, *cp;

	if (argv == NULL || argv[0] == NULL) {
		if (cmd == NULL)
			return ("");
		ap = NULL;
		len = maxlen + 3;
	} else {
		ap = shquote(argv);
		len = strlen(ap) + maxlen + 4;
	}
	cp = malloc(len);
	if (cp == NULL)
		errx(1, "malloc failed");
	if (ap == NULL) {
		if (thread != NULL) {
			asprintf(&ap, "%s/%s", cmd, thread);
			sprintf(cp, "[%.*s]", (int)maxlen, ap);
			free(ap);
		} else
			sprintf(cp, "[%.*s]", (int)maxlen, cmd);
	} else if (strncmp(cmdpart(argv[0]), cmd, maxlen) != 0)
		sprintf(cp, "%s (%.*s)", ap, (int)maxlen, cmd);
	else
		strcpy(cp, ap);
	return (cp);
}

/* ---- keyword.c ---- */

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
 */





static int  vcmp(const void *, const void *);

/* Compute offset in common structures. */
#define	KOFF(x)	offsetof(struct kinfo_proc, x)
#define	ROFF(x)	offsetof(struct rusage, x)

#define	LWPFMT	"d"
#define	NLWPFMT	"d"
#define	UIDFMT	"u"
#define	PIDFMT	"d"

/* PLEASE KEEP THE TABLE BELOW SORTED ALPHABETICALLY!!! */
VAR keywords[] = {
	{"%cpu", {NULL}, "%CPU", "percent-cpu", 0, pcpu, 0, UNSPEC, NULL},
	{"%mem", {NULL}, "%MEM", "percent-memory", 0, pmem, 0, UNSPEC, NULL},
	{"acflag", {NULL}, "ACFLG", "accounting-flag", 0, kvar, KOFF(ki_acflag),
	 USHORT, "x"},
	{"acflg", {"acflag"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"args", {NULL}, "COMMAND", "arguments", COMM|LJUST|USER, arguments, 0,
	 UNSPEC, NULL},
	{"blocked", {"sigmask"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"caught", {"sigcatch"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"class", {NULL}, "CLASS", "login-class", LJUST, loginclass, 0,
	 UNSPEC, NULL},
	{"comm", {NULL}, "COMMAND", "command", LJUST, ucomm, 0, UNSPEC, NULL},
	{"command", {NULL}, "COMMAND", "command", COMM|LJUST|USER, command, 0,
	 UNSPEC, NULL},
	{"cow", {NULL}, "COW", "copy-on-write-faults", 0, kvar, KOFF(ki_cow),
	 UINT, "u"},
	{"cpu", {NULL}, "C", "on-cpu", 0, cpunum, 0, UNSPEC, NULL},
	{"cputime", {"time"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"dsiz", {NULL}, "DSIZ", "data-size", 0, kvar, KOFF(ki_dsize),
	 PGTOK, "ld"},
	{"egid", {"gid"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"egroup", {"group"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"emul", {NULL}, "EMUL", "emulation-envirnment", LJUST, emulname, 0,
	 UNSPEC, NULL},
	{"etime", {NULL}, "ELAPSED", "elapsed-time", USER, elapsed, 0,
	 UNSPEC, NULL},
	{"etimes", {NULL}, "ELAPSED", "elapsed-times", USER, elapseds, 0,
	 UNSPEC, NULL},
	{"euid", {"uid"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"f", {NULL}, "F", "flags", 0, kvar, KOFF(ki_flag), LONG, "lx"},
	{"f2", {NULL}, "F2", "flags2", 0, kvar, KOFF(ki_flag2), INT, "08x"},
	{"fib", {NULL}, "FIB", "fib", 0, kvar, KOFF(ki_fibnum), INT, "d"},
	{"flags", {"f"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"flags2", {"f2"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"gid", {NULL}, "GID", "gid", 0, kvar, KOFF(ki_groups), UINT, UIDFMT},
	{"group", {NULL}, "GROUP", "group", LJUST, egroupname, 0, UNSPEC, NULL},
	{"ignored", {"sigignore"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"inblk", {NULL}, "INBLK", "read-blocks", USER, rvar, ROFF(ru_inblock),
	 LONG, "ld"},
	{"inblock", {"inblk"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"jail", {NULL}, "JAIL", "jail-name", LJUST, jailname, 0, UNSPEC, NULL},
	{"jid", {NULL}, "JID", "jail-id", 0, kvar, KOFF(ki_jid), INT, "d"},
	{"jobc", {NULL}, "JOBC", "job-control-count", 0, kvar, KOFF(ki_jobc),
	 SHORT, "d"},
	{"ktrace", {NULL}, "KTRACE", "ktrace", 0, kvar, KOFF(ki_traceflag),
	 INT, "x"},
	{"label", {NULL}, "LABEL", "label", LJUST, label, 0, UNSPEC, NULL},
	{"lim", {NULL}, "LIM", "memory-limit", 0, maxrss, 0, UNSPEC, NULL},
	{"lockname", {NULL}, "LOCK", "lock-name", LJUST, lockname, 0,
	 UNSPEC, NULL},
	{"login", {NULL}, "LOGIN", "login-name", LJUST, logname, 0,
	 UNSPEC, NULL},
	{"logname", {"login"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"lstart", {NULL}, "STARTED", "start-time", LJUST|USER, lstarted, 0,
	 UNSPEC, NULL},
	{"lwp", {NULL}, "LWP", "thread-id", 0, kvar, KOFF(ki_tid),
	 UINT, LWPFMT},
	{"majflt", {NULL}, "MAJFLT", "major-faults", USER, rvar, ROFF(ru_majflt),
	 LONG, "ld"},
	{"minflt", {NULL}, "MINFLT", "minor-faults", USER, rvar, ROFF(ru_minflt),
	 LONG, "ld"},
	{"msgrcv", {NULL}, "MSGRCV", "received-messages", USER, rvar,
	 ROFF(ru_msgrcv), LONG, "ld"},
	{"msgsnd", {NULL}, "MSGSND", "sent-messages", USER, rvar,
	 ROFF(ru_msgsnd), LONG, "ld"},
	{"mwchan", {NULL}, "MWCHAN", "wait-channel", LJUST, mwchan, 0,
	 UNSPEC, NULL},
	{"ni", {"nice"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"nice", {NULL}, "NI", "nice", 0, kvar, KOFF(ki_nice), SCHAR, "d"},
	{"nivcsw", {NULL}, "NIVCSW", "involuntary-context-switches", USER, rvar,
	 ROFF(ru_nivcsw), LONG, "ld"},
	{"nlwp", {NULL}, "NLWP", "threads", 0, kvar, KOFF(ki_numthreads),
	 UINT, NLWPFMT},
	{"nsignals", {"nsigs"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"nsigs", {NULL}, "NSIGS", "signals-taken", USER, rvar,
	 ROFF(ru_nsignals), LONG, "ld"},
	{"nswap", {NULL}, "NSWAP", "swaps", USER, rvar, ROFF(ru_nswap),
	 LONG, "ld"},
	{"nvcsw", {NULL}, "NVCSW", "voluntary-context-switches", USER, rvar,
	 ROFF(ru_nvcsw), LONG, "ld"},
	{"nwchan", {NULL}, "NWCHAN", "wait-channel-address", LJUST, nwchan, 0,
	 UNSPEC, NULL},
	{"oublk", {NULL}, "OUBLK", "written-blocks", USER, rvar,
	 ROFF(ru_oublock), LONG, "ld"},
	{"oublock", {"oublk"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"paddr", {NULL}, "PADDR", "process-address", 0, kvar, KOFF(ki_paddr),
	 KPTR, "lx"},
	{"pagein", {NULL}, "PAGEIN", "pageins", USER, pagein, 0, UNSPEC, NULL},
	{"pcpu", {"%cpu"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"pending", {"sig"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"pgid", {NULL}, "PGID", "process-group", 0, kvar, KOFF(ki_pgid),
	 UINT, PIDFMT},
	{"pid", {NULL}, "PID", "pid", 0, kvar, KOFF(ki_pid), UINT, PIDFMT},
	{"pmem", {"%mem"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"ppid", {NULL}, "PPID", "ppid", 0, kvar, KOFF(ki_ppid), UINT, PIDFMT},
	{"pri", {NULL}, "PRI", "priority", 0, pri, 0, UNSPEC, NULL},
	{"re", {NULL}, "RE", "residency-time", INF127, kvar, KOFF(ki_swtime),
	 UINT, "d"},
	{"rgid", {NULL}, "RGID", "real-gid", 0, kvar, KOFF(ki_rgid),
	 UINT, UIDFMT},
	{"rgroup", {NULL}, "RGROUP", "real-group", LJUST, rgroupname, 0,
	 UNSPEC, NULL},
	{"rss", {NULL}, "RSS", "rss", 0, kvar, KOFF(ki_rssize), PGTOK, "ld"},
	{"rtprio", {NULL}, "RTPRIO", "realtime-priority", 0, priorityr,
	 KOFF(ki_pri), UNSPEC, NULL},
	{"ruid", {NULL}, "RUID", "real-uid", 0, kvar, KOFF(ki_ruid),
	 UINT, UIDFMT},
	{"ruser", {NULL}, "RUSER", "real-user", LJUST, runame, 0, UNSPEC, NULL},
	{"sid", {NULL}, "SID", "sid", 0, kvar, KOFF(ki_sid), UINT, PIDFMT},
	{"sig", {NULL}, "PENDING", "signals-pending", 0, kvar, KOFF(ki_siglist),
	 INT, "x"},
	{"sigcatch", {NULL}, "CAUGHT", "signals-caught", 0, kvar,
	 KOFF(ki_sigcatch), UINT, "x"},
	{"sigignore", {NULL}, "IGNORED", "signals-ignored", 0, kvar,
	 KOFF(ki_sigignore), UINT, "x"},
	{"sigmask", {NULL}, "BLOCKED", "signal-mask", 0, kvar, KOFF(ki_sigmask),
	 UINT, "x"},
	{"sl", {NULL}, "SL", "sleep-time", INF127, kvar, KOFF(ki_slptime),
	 UINT, "d"},
	{"ssiz", {NULL}, "SSIZ", "stack-size", 0, kvar, KOFF(ki_ssize),
	 PGTOK, "ld"},
	{"start", {NULL}, "STARTED", "start-time", LJUST|USER, started, 0,
	 UNSPEC, NULL},
	{"stat", {"state"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"state", {NULL}, "STAT", "state", LJUST, state, 0, UNSPEC, NULL},
	{"svgid", {NULL}, "SVGID", "saved-gid", 0, kvar, KOFF(ki_svgid),
	 UINT, UIDFMT},
	{"svuid", {NULL}, "SVUID", "saved-uid", 0, kvar, KOFF(ki_svuid),
	 UINT, UIDFMT},
	{"systime", {NULL}, "SYSTIME", "system-time", USER, systime, 0,
	 UNSPEC, NULL},
	{"tdaddr", {NULL}, "TDADDR", "thread-address", 0, kvar, KOFF(ki_tdaddr),
	 KPTR, "lx"},
	{"tdev", {NULL}, "TDEV", "terminal-device", 0, tdev, 0, UNSPEC, NULL},
	{"tdnam", {"tdname"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"tdname", {NULL}, "TDNAME", "thread-name", LJUST, tdnam, 0,
	 UNSPEC, NULL},
	{"tid", {"lwp"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"time", {NULL}, "TIME", "cpu-time", USER, cputime, 0, UNSPEC, NULL},
	{"tpgid", {NULL}, "TPGID", "terminal-process-gid", 0, kvar,
	 KOFF(ki_tpgid), UINT, PIDFMT},
	{"tracer", {NULL}, "TRACER", "tracer", 0, kvar, KOFF(ki_tracer),
	 UINT, PIDFMT},
	{"tsid", {NULL}, "TSID", "terminal-sid", 0, kvar, KOFF(ki_tsid),
	 UINT, PIDFMT},
	{"tsiz", {NULL}, "TSIZ", "text-size", 0, kvar, KOFF(ki_tsize),
	 PGTOK, "ld"},
	{"tt", {NULL}, "TT ", "terminal-name", 0, tname, 0, UNSPEC, NULL},
	{"tty", {NULL}, "TTY", "tty", LJUST, longtname, 0, UNSPEC, NULL},
	{"ucomm", {NULL}, "UCOMM", "accounting-name", LJUST, ucomm, 0,
	 UNSPEC, NULL},
	{"uid", {NULL}, "UID", "uid", 0, kvar, KOFF(ki_uid), UINT, UIDFMT},
	{"upr", {NULL}, "UPR", "user-priority", 0, upr, 0, UNSPEC, NULL},
	{"uprocp", {NULL}, "UPROCP", "process-address", 0, kvar, KOFF(ki_paddr),
	 KPTR, "lx"},
	{"user", {NULL}, "USER", "user", LJUST, username, 0, UNSPEC, NULL},
	{"usertime", {NULL}, "USERTIME", "user-time", USER, usertime, 0,
	 UNSPEC, NULL},
	{"usrpri", {"upr"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"vmaddr", {NULL}, "VMADDR", "vmspace-address", 0, kvar,
	 KOFF(ki_vmspace), KPTR, "lx"},
	{"vsize", {"vsz"}, NULL, NULL, 0, NULL, 0, UNSPEC, NULL},
	{"vsz", {NULL}, "VSZ", "virtual-size", 0, vsize, 0, UNSPEC, NULL},
	{"wchan", {NULL}, "WCHAN", "wait-channel", LJUST, wchan, 0,
	 UNSPEC, NULL},
	{"xstat", {NULL}, "XSTAT", "exit-status", 0, kvar, KOFF(ki_xstat),
	 USHORT, "x"},
};

const size_t known_keywords_nb = nitems(keywords);

size_t
aliased_keyword_index(const VAR *const v)
{
	const VAR *const fv = (v->flag & RESOLVED_ALIAS) == 0 ?
	    v : v->final_kw;
	const size_t idx = fv - keywords;

	assert(idx < known_keywords_nb);
	return (idx);
}

/*
 * Sanity checks on declared keywords.
 *
 * Checks specific to aliases are done in resolve_alias() instead.
 *
 * Currently, only checks that keywords are alphabetically ordered by their
 * names.  More checks could be added, such as the absence of type (UNSPEC),
 * 'fmt' (NULL) when the output routine is not kval()/rval().
 *
 * Called from main() on PS_CHECK_KEYWORDS, else available when debugging.
 */
void
check_keywords(void)
{
	const VAR *k, *next_k;
	bool order_violated = false;

	k = &keywords[0];
	for (size_t i = 1; i < known_keywords_nb; ++i) {
		next_k = &keywords[i];
		if (vcmp(k, next_k) >= 0) {
			xo_warnx("keywords bad order: '%s' followed by '%s'",
			    k->name, next_k->name);
			order_violated = true;
		}
		k = next_k;
	}
	if (order_violated)
		/* Must be the case as we rely on bsearch() + vcmp(). */
		xo_errx(2, "keywords are not in ascending order "
		    "(internal error)");
}

static void
alias_errx(const char *const name, const char *const what)
{
	xo_errx(2, "alias keyword '%s' specifies %s (internal error)",
	    name, what);
}

static void
merge_alias(VAR *const k, VAR *const tgt)
{
	if ((tgt->flag & RESOLVED_ALIAS) != 0)
		k->final_kw = tgt->final_kw;
	else {
		k->final_kw = tgt;
		assert(tgt->aliased == NULL);
	}

#define MERGE_IF_SENTINEL(field, sentinel) do {				\
	if (k->field == sentinel)					\
		k->field = tgt->field;					\
} while (0)

	MERGE_IF_SENTINEL(header, NULL);
	MERGE_IF_SENTINEL(field, NULL);
	/* If NOINHERIT is present, no merge occurs. */
	MERGE_IF_SENTINEL(flag, 0);

#undef MERGE_IF_SENTINEL

	/* We also check that aliases don't specify things they should not. */
#define MERGE_CHECK_SENTINEL(field, sentinel, field_descr) do {		\
	if (k->field != sentinel)					\
		alias_errx(k->name, field_descr);			\
	k->field = tgt->field;						\
} while (0);

	MERGE_CHECK_SENTINEL(oproc, NULL, "an output routine");
	MERGE_CHECK_SENTINEL(off, 0, "a structure offset");
	MERGE_CHECK_SENTINEL(type, UNSPEC, "a different type than UNSPEC");
	MERGE_CHECK_SENTINEL(fmt, NULL, "a printf format");

#undef MERGE_CHECK_SENTINEL
}

static void
resolve_alias(VAR *const k)
{
	VAR *t, key;

	if ((k->flag & RESOLVED_ALIAS) != 0 || k->aliased == NULL)
		return;

	if ((k->flag & RESOLVING_ALIAS) != 0)
		xo_errx(2, "cycle when resolving alias keyword '%s'", k->name);
	k->flag |= RESOLVING_ALIAS;

	key.name = k->aliased;
	t = bsearch(&key, keywords, known_keywords_nb, sizeof(VAR), vcmp);
	if (t == NULL)
		xo_errx(2, "unknown target '%s' for keyword alias '%s'",
		    k->aliased, k->name);

	resolve_alias(t);
	merge_alias(k, t);

	k->flag &= ~RESOLVING_ALIAS;
	k->flag |= RESOLVED_ALIAS;
}

/*
 * Resolve all aliases immediately.
 *
 * Called from main() on PS_CHECK_KEYWORDS, else available when debugging.
 */
void
resolve_aliases(void)
{
	for (size_t i = 0; i < known_keywords_nb; ++i)
		resolve_alias(&keywords[i]);
}

void
showkey(void)
{
	const VAR *v;
	const VAR *const end = keywords + known_keywords_nb;
	const char *sep;
	int i;

	i = 0;
	sep = "";
	xo_open_list("key");
	for (v = keywords; v < end; ++v) {
		const char *const p = v->name;
		const int len = strlen(p);

		if (termwidth && (i += len + 1) > termwidth) {
			i = len;
			sep = "\n";
		}
		xo_emit("{P:/%hs}{l:key/%hs}", sep, p);
		sep = " ";
	}
	xo_emit("\n");
	xo_close_list("key");
	if (xo_finish() < 0)
		xo_err(1, "stdout");
}

void
parsefmt(const char *p, struct velisthead *const var_list,
    const int user)
{
	char *copy, *cp;
	char *hdr_p, sep;
	size_t sep_idx;
	VAR *v, key;
	struct varent *vent;

	cp = copy = strdup(p);
	if (copy == NULL)
		xo_err(1, "strdup");

	sep = cp[0]; /* We only care if it's 0 or not here. */
	sep_idx = -1;
	while (sep != '\0') {
		cp += sep_idx + 1;

		/*
		 * If an item contains an equals sign, it specifies a column
		 * header, may contain embedded separator characters and
		 * is always the last item.
		 */
		sep_idx = strcspn(cp, "= \t,\n");
		sep = cp[sep_idx];
		cp[sep_idx] = 0;
		if (sep == '=') {
			hdr_p = cp + sep_idx + 1;
			sep = '\0'; /* No more keywords. */
		} else
			hdr_p = NULL;

		/* At this point, '*cp' is '\0' iff 'sep_idx' is 0. */
		if (*cp == '\0') {
			/*
			 * Empty keyword.  Skip it, and silently unless some
			 * header has been specified.
			 */
			if (hdr_p != NULL)
				xo_warnx("empty keyword with header '%s'",
				    hdr_p);
			continue;
		}

		/* Find the keyword. */
		key.name = cp;
		v = bsearch(&key, keywords,
		    known_keywords_nb, sizeof(VAR), vcmp);
		if (v == NULL) {
			xo_warnx("%s: keyword not found", cp);
			eval = 1;
			continue;
		}

#ifndef PS_CHECK_KEYWORDS
		/*
		 * On PS_CHECK_KEYWORDS, this is not necessary as all aliases
		 * are resolved at startup in main() by calling
		 * resolve_aliases().
		 */
		resolve_alias(v);
#endif

		if ((vent = malloc(sizeof(struct varent))) == NULL)
			xo_errx(1, "malloc failed");
		vent->header = v->header;
		if (hdr_p) {
			hdr_p = strdup(hdr_p);
			if (hdr_p)
				vent->header = hdr_p;
		}
		vent->width = strlen(vent->header);
		vent->var = v;
		vent->flags = user ? VE_KEEP : 0;
		STAILQ_INSERT_TAIL(var_list, vent, next_ve);
	}

	free(copy);

	if (STAILQ_EMPTY(var_list)) {
		xo_warnx("no valid keywords; valid keywords:");
		showkey();
		exit(1);
	}
}

static int
vcmp(const void *a, const void *b)
{
        return (strcmp(((const VAR *)a)->name, ((const VAR *)b)->name));
}

/* ---- print.c ---- */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
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





#define	COMMAND_WIDTH	16
#define	ARGUMENTS_WIDTH	16

#define	ps_pgtok(a)	(((a) * getpagesize()) / 1024)

void
printheader(void)
{
	const VAR *v;
	struct varent *vent;

	STAILQ_FOREACH(vent, &varlist, next_ve)
		if (*vent->header != '\0')
			break;
	if (!vent)
		return;

	STAILQ_FOREACH(vent, &varlist, next_ve) {
		v = vent->var;
		if (v->flag & LJUST) {
			if (STAILQ_NEXT(vent, next_ve) == NULL)	/* last one */
				xo_emit("{T:/%hs}", vent->header);
			else
				xo_emit("{T:/%-*hs}", vent->width, vent->header);
		} else
			xo_emit("{T:/%*hs}", vent->width, vent->header);
		if (STAILQ_NEXT(vent, next_ve) != NULL)
			xo_emit("{P: }");
	}
	xo_emit("\n");
}

char *
arguments(KINFO *k, VARENT *ve)
{
	char *vis_args;

	if ((vis_args = malloc(strlen(k->ki_args) * 4 + 1)) == NULL)
		xo_errx(1, "malloc failed");
	strvis(vis_args, k->ki_args, VIS_TAB | VIS_NL | VIS_NOSLASH);

	if (STAILQ_NEXT(ve, next_ve) != NULL && strlen(vis_args) > ARGUMENTS_WIDTH)
		vis_args[ARGUMENTS_WIDTH] = '\0';

	return (vis_args);
}

char *
command(KINFO *k, VARENT *ve)
{
	char *vis_args, *vis_env, *str;

	if (cflag) {
		/* If it is the last field, then don't pad */
		if (STAILQ_NEXT(ve, next_ve) == NULL) {
			asprintf(&str, "%s%s%s%s%s",
			    k->ki_d.prefix ? k->ki_d.prefix : "",
			    k->ki_p->ki_comm,
			    (showthreads && k->ki_p->ki_numthreads > 1) ? "/" : "",
			    (showthreads && k->ki_p->ki_numthreads > 1) ? k->ki_p->ki_tdname : "",
			    (showthreads && k->ki_p->ki_numthreads > 1) ? k->ki_p->ki_moretdname : "");
		} else
			str = strdup(k->ki_p->ki_comm);

		return (str);
	}
	if ((vis_args = malloc(strlen(k->ki_args) * 4 + 1)) == NULL)
		xo_errx(1, "malloc failed");
	strvis(vis_args, k->ki_args, VIS_TAB | VIS_NL | VIS_NOSLASH);

	if (STAILQ_NEXT(ve, next_ve) == NULL) {
		/* last field */

		if (k->ki_env) {
			if ((vis_env = malloc(strlen(k->ki_env) * 4 + 1))
			    == NULL)
				xo_errx(1, "malloc failed");
			strvis(vis_env, k->ki_env,
			    VIS_TAB | VIS_NL | VIS_NOSLASH);
		} else
			vis_env = NULL;

		asprintf(&str, "%s%s%s%s",
		    k->ki_d.prefix ? k->ki_d.prefix : "",
		    vis_env ? vis_env : "",
		    vis_env ? " " : "",
		    vis_args);

		if (vis_env != NULL)
			free(vis_env);
		free(vis_args);
	} else {
		/* ki_d.prefix & ki_env aren't shown for interim fields */
		str = vis_args;

		if (strlen(str) > COMMAND_WIDTH)
			str[COMMAND_WIDTH] = '\0';
	}

	return (str);
}

char *
ucomm(KINFO *k, VARENT *ve)
{
	char *str;

	if (STAILQ_NEXT(ve, next_ve) == NULL) {	/* last field, don't pad */
		asprintf(&str, "%s%s%s%s%s",
		    k->ki_d.prefix ? k->ki_d.prefix : "",
		    k->ki_p->ki_comm,
		    (showthreads && k->ki_p->ki_numthreads > 1) ? "/" : "",
		    (showthreads && k->ki_p->ki_numthreads > 1) ? k->ki_p->ki_tdname : "",
		    (showthreads && k->ki_p->ki_numthreads > 1) ? k->ki_p->ki_moretdname : "");
	} else {
		if (showthreads && k->ki_p->ki_numthreads > 1)
			asprintf(&str, "%s/%s%s", k->ki_p->ki_comm,
			    k->ki_p->ki_tdname, k->ki_p->ki_moretdname);
		else
			str = strdup(k->ki_p->ki_comm);
	}
	return (str);
}

char *
tdnam(KINFO *k, VARENT *ve __unused)
{
	char *str;

	if (showthreads && k->ki_p->ki_numthreads > 1)
		asprintf(&str, "%s%s", k->ki_p->ki_tdname,
		    k->ki_p->ki_moretdname);
	else
		str = strdup("      ");

	return (str);
}

char *
logname(KINFO *k, VARENT *ve __unused)
{

	if (*k->ki_p->ki_login == '\0')
		return (NULL);
	return (strdup(k->ki_p->ki_login));
}

char *
state(KINFO *k, VARENT *ve __unused)
{
	long flag, tdflags;
	char *cp, *buf;

	buf = malloc(16);
	if (buf == NULL)
		xo_errx(1, "malloc failed");

	flag = k->ki_p->ki_flag;
	tdflags = k->ki_p->ki_tdflags;	/* XXXKSE */
	cp = buf;

	switch (k->ki_p->ki_stat) {

	case SSTOP:
		*cp = 'T';
		break;

	case SSLEEP:
		if (tdflags & TDF_SINTR)	/* interruptible (long) */
			*cp = k->ki_p->ki_slptime >= MAXSLP ? 'I' : 'S';
		else
			*cp = 'D';
		break;

	case SRUN:
	case SIDL:
		*cp = 'R';
		break;

	case SWAIT:
		*cp = 'W';
		break;

	case SLOCK:
		*cp = 'L';
		break;

	case SZOMB:
		*cp = 'Z';
		break;

	default:
		*cp = '?';
	}
	cp++;
	if (k->ki_p->ki_nice < NZERO || k->ki_p->ki_pri.pri_class == PRI_REALTIME)
		*cp++ = '<';
	else if (k->ki_p->ki_nice > NZERO || k->ki_p->ki_pri.pri_class == PRI_IDLE)
		*cp++ = 'N';
	if (flag & P_TRACED)
		*cp++ = 'X';
	if (flag & P_WEXIT && k->ki_p->ki_stat != SZOMB)
		*cp++ = 'E';
	if (flag & P_PPWAIT)
		*cp++ = 'V';
	if ((flag & P_SYSTEM) || k->ki_p->ki_lock > 0)
		*cp++ = 'L';
	if ((k->ki_p->ki_cr_flags & KI_CRF_CAPABILITY_MODE) != 0)
		*cp++ = 'C';
	if (k->ki_p->ki_kiflag & KI_SLEADER)
		*cp++ = 's';
	if ((flag & P_CONTROLT) && k->ki_p->ki_pgid == k->ki_p->ki_tpgid)
		*cp++ = '+';
	if (flag & P_JAILED)
		*cp++ = 'J';
	*cp = '\0';
	return (buf);
}

#define	scalepri(x)	((x) - PUSER)

char *
pri(KINFO *k, VARENT *ve __unused)
{
	char *str;

	asprintf(&str, "%d", scalepri(k->ki_p->ki_pri.pri_level));
	return (str);
}

char *
upr(KINFO *k, VARENT *ve __unused)
{
	char *str;

	asprintf(&str, "%d", scalepri(k->ki_p->ki_pri.pri_user));
	return (str);
}
#undef scalepri

char *
username(KINFO *k, VARENT *ve __unused)
{

	return (strdup(user_from_uid(k->ki_p->ki_uid, 0)));
}

char *
egroupname(KINFO *k, VARENT *ve __unused)
{

	return (strdup(group_from_gid(k->ki_p->ki_groups[0], 0)));
}

char *
rgroupname(KINFO *k, VARENT *ve __unused)
{

	return (strdup(group_from_gid(k->ki_p->ki_rgid, 0)));
}

char *
runame(KINFO *k, VARENT *ve __unused)
{

	return (strdup(user_from_uid(k->ki_p->ki_ruid, 0)));
}

char *
tdev(KINFO *k, VARENT *ve __unused)
{
	dev_t dev;
	char *str;

	dev = k->ki_p->ki_tdev;
	if (dev == NODEV)
		str = strdup("-");
	else
		asprintf(&str, "%#jx", (uintmax_t)dev);

	return (str);
}

char *
tname(KINFO *k, VARENT *ve __unused)
{
	dev_t dev;
	char *ttname, *str;

	dev = k->ki_p->ki_tdev;
	if (dev == NODEV || (ttname = devname(dev, S_IFCHR)) == NULL)
		str = strdup("- ");
	else {
		if (strncmp(ttname, "tty", 3) == 0 ||
		    strncmp(ttname, "cua", 3) == 0)
			ttname += 3;
		if (strncmp(ttname, "pts/", 4) == 0)
			ttname += 4;
		asprintf(&str, "%s%c", ttname,
		    k->ki_p->ki_kiflag & KI_CTTY ? ' ' : '-');
	}

	return (str);
}

char *
longtname(KINFO *k, VARENT *ve __unused)
{
	dev_t dev;
	const char *ttname;

	dev = k->ki_p->ki_tdev;
	if (dev == NODEV || (ttname = devname(dev, S_IFCHR)) == NULL)
		ttname = "-";

	return (strdup(ttname));
}

char *
started(KINFO *k, VARENT *ve __unused)
{
	time_t then;
	struct tm *tp;
	size_t buflen = 100;
	char *buf;

	if (!k->ki_valid)
		return (NULL);

	buf = malloc(buflen);
	if (buf == NULL)
		xo_errx(1, "malloc failed");

	then = k->ki_p->ki_start.tv_sec;
	tp = localtime(&then);
	if (now - k->ki_p->ki_start.tv_sec < 24 * 3600) {
		(void)strftime(buf, buflen, "%H:%M  ", tp);
	} else if (now - k->ki_p->ki_start.tv_sec < 7 * 86400) {
		(void)strftime(buf, buflen, "%a%H  ", tp);
	} else
		(void)strftime(buf, buflen, "%e%b%y", tp);
	return (buf);
}

char *
lstarted(KINFO *k, VARENT *ve __unused)
{
	time_t then;
	char *buf;
	size_t buflen = 100;

	if (!k->ki_valid)
		return (NULL);

	buf = malloc(buflen);
	if (buf == NULL)
		xo_errx(1, "malloc failed");

	then = k->ki_p->ki_start.tv_sec;
	(void)strftime(buf, buflen, "%c", localtime(&then));
	return (buf);
}

char *
lockname(KINFO *k, VARENT *ve __unused)
{
	char *str;

	if (k->ki_p->ki_kiflag & KI_LOCKBLOCK) {
		if (k->ki_p->ki_lockname[0] != 0)
			str = strdup(k->ki_p->ki_lockname);
		else
			str = strdup("???");
	} else
		str = NULL;

	return (str);
}

char *
wchan(KINFO *k, VARENT *ve __unused)
{
	char *str;

	if (k->ki_p->ki_wchan) {
		if (k->ki_p->ki_wmesg[0] != 0)
			str = strdup(k->ki_p->ki_wmesg);
		else
			asprintf(&str, "%lx", (long)k->ki_p->ki_wchan);
	} else
		str = NULL;

	return (str);
}

char *
nwchan(KINFO *k, VARENT *ve __unused)
{
	char *str;

	if (k->ki_p->ki_wchan)
		asprintf(&str, "%0lx", (long)k->ki_p->ki_wchan);
	else
		str = NULL;

	return (str);
}

char *
mwchan(KINFO *k, VARENT *ve __unused)
{
	char *str;

	if (k->ki_p->ki_wchan) {
		if (k->ki_p->ki_wmesg[0] != 0)
			str = strdup(k->ki_p->ki_wmesg);
		else
                        asprintf(&str, "%lx", (long)k->ki_p->ki_wchan);
	} else if (k->ki_p->ki_kiflag & KI_LOCKBLOCK) {
		if (k->ki_p->ki_lockname[0]) {
			str = strdup(k->ki_p->ki_lockname);
		} else
			str = strdup("???");
	} else
		str = NULL;

	return (str);
}

char *
vsize(KINFO *k, VARENT *ve __unused)
{
	char *str;

	asprintf(&str, "%lu", (u_long)(k->ki_p->ki_size / 1024));
	return (str);
}

static char *
printtime(KINFO *k, VARENT *ve __unused, long secs, long psecs)
/* psecs is "parts" of a second. first micro, then centi */
{
	static char decimal_point;
	char *str;

	if (decimal_point == '\0')
		decimal_point = localeconv()->decimal_point[0];
	if (!k->ki_valid) {
		secs = 0;
		psecs = 0;
	} else {
		/* round and scale to 100's */
		psecs = (psecs + 5000) / 10000;
		secs += psecs / 100;
		psecs = psecs % 100;
	}
	asprintf(&str, "%ld:%02ld%c%02ld",
	    secs / 60, secs % 60, decimal_point, psecs);
	return (str);
}

char *
cputime(KINFO *k, VARENT *ve)
{
	long secs, psecs;

	/*
	 * This counts time spent handling interrupts.  We could
	 * fix this, but it is not 100% trivial (and interrupt
	 * time fractions only work on the sparc anyway).	XXX
	 */
	secs = k->ki_p->ki_runtime / 1000000;
	psecs = k->ki_p->ki_runtime % 1000000;
	if (sumrusage) {
		secs += k->ki_p->ki_childtime.tv_sec;
		psecs += k->ki_p->ki_childtime.tv_usec;
	}
	return (printtime(k, ve, secs, psecs));
}

char *
cpunum(KINFO *k, VARENT *ve __unused)
{
	char *cpu;

	if (k->ki_p->ki_stat == SRUN && k->ki_p->ki_oncpu != NOCPU) {
		asprintf(&cpu, "%d", k->ki_p->ki_oncpu);
	} else {
		asprintf(&cpu, "%d", k->ki_p->ki_lastcpu);
	}
	return (cpu);
}

char *
systime(KINFO *k, VARENT *ve)
{
	long secs, psecs;

	secs = k->ki_p->ki_rusage.ru_stime.tv_sec;
	psecs = k->ki_p->ki_rusage.ru_stime.tv_usec;
	if (sumrusage) {
		secs += k->ki_p->ki_childstime.tv_sec;
		psecs += k->ki_p->ki_childstime.tv_usec;
	}
	return (printtime(k, ve, secs, psecs));
}

char *
usertime(KINFO *k, VARENT *ve)
{
	long secs, psecs;

	secs = k->ki_p->ki_rusage.ru_utime.tv_sec;
	psecs = k->ki_p->ki_rusage.ru_utime.tv_usec;
	if (sumrusage) {
		secs += k->ki_p->ki_childutime.tv_sec;
		psecs += k->ki_p->ki_childutime.tv_usec;
	}
	return (printtime(k, ve, secs, psecs));
}

char *
elapsed(KINFO *k, VARENT *ve __unused)
{
	time_t val;
	int days, hours, mins, secs;
	char *str;

	if (!k->ki_valid)
		return (NULL);
	val = now - k->ki_p->ki_start.tv_sec;
	days = val / (24 * 60 * 60);
	val %= 24 * 60 * 60;
	hours = val / (60 * 60);
	val %= 60 * 60;
	mins = val / 60;
	secs = val % 60;
	if (days != 0)
		asprintf(&str, "%3d-%02d:%02d:%02d", days, hours, mins, secs);
	else if (hours != 0)
		asprintf(&str, "%02d:%02d:%02d", hours, mins, secs);
	else
		asprintf(&str, "%02d:%02d", mins, secs);

	return (str);
}

char *
elapseds(KINFO *k, VARENT *ve __unused)
{
	time_t val;
	char *str;

	if (!k->ki_valid)
		return (NULL);
	val = now - k->ki_p->ki_start.tv_sec;
	asprintf(&str, "%jd", (intmax_t)val);
	return (str);
}

double
getpcpu(const KINFO *k)
{
	static int failure;

	if (!nlistread)
		failure = donlist();
	if (failure)
		return (0.0);

#define	fxtofl(fixpt)	((double)(fixpt) / fscale)

	/* XXX - I don't like this */
	if (k->ki_p->ki_swtime == 0)
		return (0.0);
	if (rawcpu)
		return (100.0 * fxtofl(k->ki_p->ki_pctcpu));
	return (100.0 * fxtofl(k->ki_p->ki_pctcpu) /
		(1.0 - exp(k->ki_p->ki_swtime * log(fxtofl(ccpu)))));
}

char *
pcpu(KINFO *k, VARENT *ve __unused)
{
	char *str;

	asprintf(&str, "%.1f", getpcpu(k));
	return (str);
}

static double
getpmem(KINFO *k)
{
	static int failure;
	double fracmem;

	if (!nlistread)
		failure = donlist();
	if (failure)
		return (0.0);

	/* XXX want pmap ptpages, segtab, etc. (per architecture) */
	/* XXX don't have info about shared */
	fracmem = ((double)k->ki_p->ki_rssize) / mempages;
	return (100.0 * fracmem);
}

char *
pmem(KINFO *k, VARENT *ve __unused)
{
	char *str;

	asprintf(&str, "%.1f", getpmem(k));
	return (str);
}

char *
pagein(KINFO *k, VARENT *ve __unused)
{
	char *str;

	asprintf(&str, "%ld", k->ki_valid ? k->ki_p->ki_rusage.ru_majflt : 0);
	return (str);
}

/* ARGSUSED */
char *
maxrss(KINFO *k __unused, VARENT *ve __unused)
{

	/* XXX not yet */
	return (NULL);
}

char *
priorityr(KINFO *k, VARENT *ve __unused)
{
	struct priority *lpri;
	char *str;
	unsigned class, level;

	lpri = &k->ki_p->ki_pri;
	class = lpri->pri_class;
	level = lpri->pri_level;
	switch (class) {
	case RTP_PRIO_REALTIME:
	/* alias for PRI_REALTIME */
		asprintf(&str, "real:%u", level - PRI_MIN_REALTIME);
		break;
	case RTP_PRIO_NORMAL:
	/* alias for PRI_TIMESHARE */
		if (level >= PRI_MIN_TIMESHARE)
			asprintf(&str, "normal:%u", level - PRI_MIN_TIMESHARE);
		else
			asprintf(&str, "kernel:%u", level - PRI_MIN_KERN);
		break;
	case RTP_PRIO_IDLE:
	/* alias for PRI_IDLE */
		asprintf(&str, "idle:%u", level - PRI_MIN_IDLE);
		break;
	case RTP_PRIO_ITHD:
	/* alias for PRI_ITHD */
		asprintf(&str, "intr:%u", level - PRI_MIN_ITHD);
		break;
	default:
		asprintf(&str, "%u:%u", class, level);
		break;
	}
	return (str);
}

/*
 * Generic output routines.  Print fields from various prototype
 * structures.
 */
static char *
printval(void *bp, const VAR *v)
{
	static char ofmt[32] = "%";
	const char *fcp;
	char *cp, *str;

	cp = ofmt + 1;
	fcp = v->fmt;
	while ((*cp++ = *fcp++));

#define	CHKINF127(n)	(((n) > 127) && (v->flag & INF127) ? 127 : (n))

	switch (v->type) {
	case UNSPEC:
		xo_errx(1, "cannot print value of unspecified type "
		    "(internal error)");
		break;
	case CHAR:
		(void)asprintf(&str, ofmt, *(char *)bp);
		break;
	case SCHAR:
		(void)asprintf(&str, ofmt, *(signed char *)bp);
		break;
	case UCHAR:
		(void)asprintf(&str, ofmt, *(u_char *)bp);
		break;
	case SHORT:
		(void)asprintf(&str, ofmt, *(short *)bp);
		break;
	case USHORT:
		(void)asprintf(&str, ofmt, *(u_short *)bp);
		break;
	case INT:
		(void)asprintf(&str, ofmt, *(int *)bp);
		break;
	case UINT:
		(void)asprintf(&str, ofmt, CHKINF127(*(u_int *)bp));
		break;
	case LONG:
		(void)asprintf(&str, ofmt, *(long *)bp);
		break;
	case ULONG:
		(void)asprintf(&str, ofmt, *(u_long *)bp);
		break;
	case KPTR:
		(void)asprintf(&str, ofmt, *(u_long *)bp);
		break;
	case PGTOK:
		(void)asprintf(&str, ofmt, ps_pgtok(*(u_long *)bp));
		break;
	default:
		xo_errx(1, "unknown type (internal error)");
		break;
	}

	return (str);
}

char *
kvar(KINFO *k, VARENT *ve)
{
	const VAR *v;

	v = ve->var;
	return (printval((char *)((char *)k->ki_p + v->off), v));
}

char *
rvar(KINFO *k, VARENT *ve)
{
	const VAR *v;

	v = ve->var;
	if (!k->ki_valid)
		return (NULL);
	return (printval((char *)((char *)(&k->ki_p->ki_rusage) + v->off), v));
}

char *
emulname(KINFO *k, VARENT *ve __unused)
{

	return (strdup(k->ki_p->ki_emul));
}

char *
label(KINFO *k, VARENT *ve __unused)
{
	char *string;
	mac_t proclabel;
	int error;

	string = NULL;
	if (mac_prepare_process_label(&proclabel) == -1) {
		xo_warn("mac_prepare_process_label");
		goto out;
	}
	error = mac_get_pid(k->ki_p->ki_pid, proclabel);
	if (error == 0) {
		if (mac_to_text(proclabel, &string) == -1)
			string = NULL;
	}
	mac_free(proclabel);
out:
	return (string);
}

char *
loginclass(KINFO *k, VARENT *ve __unused)
{

	/*
	 * Don't display login class for system processes;
	 * login classes are used for resource limits,
	 * and limits don't apply to system processes.
	 */
	if (k->ki_p->ki_flag & P_SYSTEM) {
		return (strdup("-"));
	}
	return (strdup(k->ki_p->ki_loginclass));
}

char *
jailname(KINFO *k, VARENT *ve __unused)
{
	char *name;

	if (k->ki_p->ki_jid == 0)
		return (strdup("-"));
	name = jail_getname(k->ki_p->ki_jid);
	if (name == NULL)
		return (strdup("-"));
	return (name);
}

} // namespace pbsd::bin_ps::b0291
