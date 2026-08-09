#!/usr/bin/env python3
"""One-shot generator for b0291 oracle.c, port.cppm, build.sh."""
import re
import textwrap
from pathlib import Path

OUT = Path("/home/odin/pbsd/pbsd/bin/ps/b0291")
HBSD_PS = Path("/home/odin/pbsd/hbsd/src/bin/ps")
SOURCES = [
    ("fmt.c", HBSD_PS / "fmt.c"),
    ("keyword.c", HBSD_PS / "keyword.c"),
    ("print.c", HBSD_PS / "print.c"),
]

PUBLIC_FUNCS = [
    "fmt_argv",
    "aliased_keyword_index", "check_keywords", "resolve_aliases", "showkey", "parsefmt",
    "printheader", "arguments", "command", "ucomm", "tdnam", "logname", "state",
    "pri", "upr", "username", "egroupname", "rgroupname", "runame", "tdev", "tname",
    "longtname", "started", "lstarted", "lockname", "wchan", "nwchan", "mwchan",
    "vsize", "cputime", "cpunum", "systime", "usertime", "elapsed", "elapseds",
    "getpcpu", "pcpu", "pmem", "pagein", "maxrss", "priorityr", "kvar", "rvar",
    "emulname", "label", "loginclass", "jailname",
]
STATIC_FUNCS = [
    "shquote", "cmdpart", "vcmp", "alias_errx", "merge_alias", "resolve_alias",
    "printtime", "getpmem", "printval",
]
ALL_RENAME = PUBLIC_FUNCS + STATIC_FUNCS
FPTR_NAMES = [
    "arguments", "pcpu", "pmem", "kvar", "loginclass", "ucomm", "command",
    "emulname", "elapsed", "elapseds", "egroupname", "cpunum", "maxrss",
    "label", "lockname", "logname", "lstarted", "mwchan", "nwchan", "pagein",
    "pri", "priorityr", "rgroupname", "runame", "started", "state", "systime",
    "tdev", "tdnam", "tname", "longtname", "upr", "usertime", "username",
    "vsize", "wchan", "cputime", "rvar",
]

PREAMBLE = r'''
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
'''


def strip_includes(text: str) -> str:
    out = []
    for line in text.splitlines():
        if re.match(r'^\s*#\s*include\b', line):
            continue
        out.append(line)
    return '\n'.join(out) + '\n'


def rename_funcs(text: str) -> str:
    for fn in sorted(ALL_RENAME, key=len, reverse=True):
        text = re.sub(rf'\b{fn}\b', f'ref_{fn}', text)
    return text


def sources_only(prefix: str = "") -> str:
    parts = []
    for name, path in SOURCES:
        body = strip_includes(path.read_text())
        # Expose keyword table for harness differential tests (not a function body).
        body = body.replace('static VAR keywords[]', 'VAR keywords[]')
        if prefix:
            for fn in sorted(ALL_RENAME, key=len, reverse=True):
                body = re.sub(rf'\b{fn}\b', f'{prefix}{fn}', body)
        parts.append(f'/* ---- {name} ---- */\n')
        parts.append(body)
    return '\n'.join(parts)


def gen_oracle() -> str:
    fptr = '\n'.join(f'#define {fn} ref_{fn}' for fn in FPTR_NAMES)
    return (
        '/* oracle.c - reference for PBSD batch b0291 */\n'
        + PREAMBLE + '\n/* keyword table function pointer aliases */\n' + fptr + '\n'
        + sources_only('ref_')
    )


def gen_port() -> str:
    return (
        'module;\n'
        + PREAMBLE
        + '\nexport module pbsd.bin.ps.b0291;\n\n'
        + 'export namespace pbsd::bin_ps::b0291 {\n\n'
        + sources_only('')
        + '\n} // namespace pbsd::bin_ps::b0291\n'
    )


def gen_build_sh() -> str:
    return textwrap.dedent('''\
        #!/bin/sh
        set -e
        cd "$(dirname "$0")"
        CC=${CC:-cc}
        CXX=${CXX:-c++}
        MODNAME=pbsd.bin.ps.b0291
        rm -rf gcm.cache
        rm -f oracle.o port.o harness.o port.pcm harness
        $CC -std=c11 -O2 -c oracle.c -o oracle.o
        if $CXX --version 2>&1 | grep -qi clang; then
            $CXX -std=c++23 -O2 -x c++-module port.cppm --precompile -o port.pcm
            $CXX -std=c++23 -O2 -c port.pcm -o port.o
            $CXX -std=c++23 -O2 -fmodule-file=$MODNAME=port.pcm -c harness.cpp -o harness.o
        else
            $CXX -std=c++23 -fmodules-ts -O2 -c -x c++ port.cppm -o port.o
            $CXX -std=c++23 -fmodules-ts -O2 -c harness.cpp -o harness.o
        fi
        $CXX -std=c++23 -O2 -o harness harness.o port.o oracle.o -lm \\
            -Wl,--wrap=malloc -Wl,--wrap=exit
        exec ./harness
    ''')


if __name__ == '__main__':
    OUT.joinpath('oracle.c').write_text(gen_oracle())
    OUT.joinpath('port.cppm').write_text(gen_port())
    OUT.joinpath('build.sh').write_text(gen_build_sh())
    print('wrote oracle.c port.cppm build.sh')
