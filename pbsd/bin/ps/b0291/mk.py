#!/usr/bin/env python3
import re, textwrap
from pathlib import Path
OUT = Path("/home/odin/pbsd/pbsd/bin/ps/b0291")
PS = Path("/home/odin/pbsd/hbsd/src/bin/ps")
SOURCES = [("fmt.c", PS/"fmt.c"), ("print.c", PS/"print.c"), ("keyword.c", PS/"keyword.c")]
FUNCS = sorted(set([
    "fmt_argv","aliased_keyword_index","check_keywords","resolve_aliases","showkey","parsefmt",
    "printheader","arguments","command","ucomm","tdnam","logname","state","pri","upr","username",
    "egroupname","rgroupname","runame","tdev","tname","longtname","started","lstarted","lockname",
    "wchan","nwchan","mwchan","vsize","cputime","cpunum","systime","usertime","elapsed","elapseds",
    "getpcpu","pcpu","pmem","pagein","maxrss","priorityr","kvar","rvar","emulname","label",
    "loginclass","jailname","shquote","cmdpart","vcmp","alias_errx","merge_alias","resolve_alias",
    "printtime","getpmem","printval"]), key=len, reverse=True)
PREAMBLE = r'''
#ifndef B0291_PS_BATCH
#define B0291_PS_BATCH
#define _DEFAULT_SOURCE 1
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
#include <signal.h>
#include <time.h>
#include <bsd/vis.h>
#ifndef __unused
#define __unused
#endif
#ifndef LONG_BIT
#define LONG_BIT (8*(int)sizeof(long))
#endif
#ifndef nitems
#define nitems(x) (sizeof(x)/sizeof((x)[0]))
#endif
typedef int fixpt_t; typedef unsigned long vm_size_t; typedef long segsz_t;
typedef unsigned int u_int; typedef unsigned long u_long;
typedef unsigned char u_char; typedef unsigned short u_short; typedef uint32_t lwpid_t;
#define STAILQ_HEAD(name,type) struct name { struct type *stqh_first; struct type **stqh_last; }
#define STAILQ_ENTRY(type) struct { struct type *stqe_next; }
#define STAILQ_FIRST(h) ((h)->stqh_first)
#define STAILQ_NEXT(e,f) ((e)->f.stqe_next)
#define STAILQ_EMPTY(h) (STAILQ_FIRST(h)==NULL)
#define STAILQ_INIT(h) do{STAILQ_FIRST(h)=NULL;(h)->stqh_last=&STAILQ_FIRST(h);}while(0)
#define STAILQ_INSERT_TAIL(h,e,f) do{STAILQ_NEXT(e,f)=NULL;*(h)->stqh_last=(e);(h)->stqh_last=&STAILQ_NEXT(e,f);}while(0)
#define STAILQ_FOREACH(v,h,f) for((v)=STAILQ_FIRST(h);(v);(v)=STAILQ_NEXT(v,f))
#define UNLIMITED 0
enum type { UNSPEC,CHAR,SCHAR,UCHAR,SHORT,USHORT,INT,UINT,LONG,ULONG,KPTR,PGTOK };
typedef struct kinfo_str { STAILQ_ENTRY(kinfo_str) ks_next; char *ks_str; } KINFO_STR;
typedef struct kinfo { struct kinfo_proc *ki_p; const char *ki_args; const char *ki_env; int ki_valid; double ki_pcpu; segsz_t ki_memsize; union { int level; char *prefix; } ki_d; STAILQ_HEAD(,kinfo_str) ki_ks; } KINFO;
typedef struct varent { STAILQ_ENTRY(varent) next_ve; const char *header; const struct var *var; u_int width; uint16_t flags; } VARENT;
struct var; typedef struct var VAR;
struct var { const char *name; union { const char *aliased; const VAR *final_kw; }; const char *header; const char *field; u_int flag; char *(*oproc)(struct kinfo *,struct varent *); size_t off; enum type type; const char *fmt; };
#define COMM 0x01
#define LJUST 0x02
#define USER 0x04
#define INF127 0x10
#define NOINHERIT 0x1000
#define RESOLVING_ALIAS 0x10000
#define RESOLVED_ALIAS 0x20000
#define VE_KEEP (1<<0)
STAILQ_HEAD(velisthead,varent);
#define SIDL 1
#define SRUN 2
#define SSLEEP 3
#define SSTOP 4
#define SZOMB 5
#define SWAIT 6
#define SLOCK 7
#define TDF_SINTR 0x8
#define MAXSLP 20
#define P_CONTROLT 0x2
#define P_PPWAIT 0x10
#define P_SYSTEM 0x200
#define P_TRACED 0x800
#define P_WEXIT 0x2000
#define P_JAILED 0x1000000
#define KI_CTTY 1
#define KI_SLEADER 2
#define KI_LOCKBLOCK 4
#define KI_CRF_CAPABILITY_MODE 1
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
#define KI_NSPARE_INT 2
#define KI_NSPARE_PTR 4
#define KI_NSPARE_LONG 12
struct priority { u_char pri_class,pri_level,pri_native,pri_user; };
struct kinfo_proc {
 int ki_structsize,ki_layout; void *ki_args,*ki_paddr,*ki_addr,*ki_tracep,*ki_textvp,*ki_fd,*ki_vmspace; const void *ki_wchan;
 pid_t ki_pid,ki_ppid,ki_pgid,ki_tpgid,ki_sid,ki_tsid; short ki_jobc,ki_spare_short1; uint32_t ki_tdev_freebsd11;
 sigset_t ki_siglist,ki_sigmask,ki_sigignore,ki_sigcatch; uid_t ki_uid,ki_ruid,ki_svuid; gid_t ki_rgid,ki_svgid;
 short ki_ngroups,ki_spare_short2; gid_t ki_groups[KI_NGROUPS]; vm_size_t ki_size; segsz_t ki_rssize,ki_swrss,ki_tsize,ki_dsize,ki_ssize;
 u_short ki_xstat,ki_acflag; fixpt_t ki_pctcpu; u_int ki_estcpu,ki_slptime,ki_swtime,ki_cow; uint64_t ki_runtime;
 struct timeval ki_start,ki_childtime; long ki_flag,ki_kiflag; int ki_traceflag; char ki_stat; signed char ki_nice; char ki_lock,ki_rqindex;
 u_char ki_oncpu_old,ki_lastcpu_old; char ki_tdname[TDNAMLEN+1],ki_wmesg[WMESGLEN+1],ki_login[LOGNAMELEN+1],ki_lockname[LOCKNAMELEN+1],ki_comm[COMMLEN+1],ki_emul[KI_EMULNAMELEN+1],ki_loginclass[LOGINCLASSLEN+1],ki_moretdname[MAXCOMLEN-TDNAMLEN+1],ki_sparestrings[38];
 int ki_spareints[KI_NSPARE_INT]; pid_t ki_reaper,ki_reapsubtree; uint64_t ki_tdev; int ki_oncpu,ki_lastcpu,ki_tracer,ki_flag2,ki_fibnum; u_int ki_cr_flags; int ki_jid,ki_numthreads; lwpid_t ki_tid;
 struct priority ki_pri; struct rusage ki_rusage,ki_rusage_ch; void *ki_pcb,*ki_kstack,*ki_udata,*ki_tdaddr,*ki_pd,*ki_uerrmsg,*ki_spareptrs[KI_NSPARE_PTR]; long ki_sparelongs[KI_NSPARE_LONG],ki_sflag,ki_tdflags;
};
#define ki_childstime ki_rusage_ch.ru_stime
#define ki_childutime ki_rusage_ch.ru_utime
#define KOFF(x) offsetof(struct kinfo_proc,x)
#define ROFF(x) offsetof(struct rusage,x)
#define LWPFMT "d"
#define NLWPFMT "d"
#define UIDFMT "u"
#define PIDFMT "d"
#define COMMAND_WIDTH 16
#define ARGUMENTS_WIDTH 16
#define ps_pgtok(a) (((a)*getpagesize())/1024)
extern fixpt_t ccpu; extern int cflag,eval,fscale,nlistread,rawcpu; extern unsigned long mempages; extern time_t now; extern int showthreads,sumrusage,termwidth; extern struct velisthead varlist; extern const size_t known_keywords_nb;
extern char *user_from_uid(uid_t,int); extern char *group_from_gid(gid_t,int); extern char *devname(dev_t,mode_t); extern char *jail_getname(int); extern int donlist(void);
typedef void *mac_t; extern int mac_prepare_process_label(mac_t*); extern int mac_get_pid(pid_t,mac_t); extern int mac_to_text(mac_t,char**); extern void mac_free(mac_t);
extern void xo_warnx(const char*,...); extern void xo_warn(const char*,...); extern void xo_err(int,const char*,...); extern void xo_errx(int,const char*,...); extern void xo_open_list(const char*); extern int xo_emit(const char*,...); extern void xo_close_list(const char*); extern int xo_finish(void);
extern jmp_buf b0291_err_jmp; extern int b0291_err_jmp_set,b0291_errx_code;
static inline void b0291_errx(int eval,const char *fmt,...){va_list ap;va_start(ap,fmt);if(b0291_err_jmp_set)longjmp(b0291_err_jmp,eval);vfprintf(stderr,fmt,ap);va_end(ap);exit(eval);} 
#undef errx
#define errx b0291_errx
#endif
'''

def strip_inc(t):
    return '\n'.join(l for l in t.splitlines() if not re.match(r'^\s*#\s*include\b',l))+'\n'

def ren(t,p):
    o,i,n=[],0,len(t)
    while i<n:
        c=t[i]
        if c=='"':
            o.append(c); i+=1
            while i<n:
                if t[i]=='\\' and i+1<n: o.append(t[i:i+2]); i+=2
                elif t[i]=='"': o.append('"'); i+=1; break
                else: o.append(t[i]); i+=1
            continue
        if c=="'":
            o.append(c); i+=1
            while i<n and t[i]!="'":
                if t[i]=='\\' and i+1<n: o.append(t[i:i+2]); i+=2
                else: o.append(t[i]); i+=1
            if i<n: o.append(t[i]); i+=1
            continue
        m=False
        for fn in FUNCS:
            if t.startswith(fn,i):
                b=i==0 or not (t[i-1].isalnum() or t[i-1]=='_')
                a=i+len(fn)==n or not (t[i+len(fn)].isalnum() or t[i+len(fn)]=='_')
                if b and a: o.append(p+fn); i+=len(fn); m=True; break
        if not m: o.append(c); i+=1
    return ''.join(o)

def body(pref=''):
    parts=[]
    for n,p in SOURCES:
        b=strip_inc(p.read_text()).replace('static VAR keywords[]','VAR keywords[]')
        if pref: b=ren(b,pref)
        parts.append(f'/* ---- {n} ---- */\n'+b)
    return '\n'.join(parts)

OUT.joinpath('oracle.c').write_text('/* oracle.c */\n'+PREAMBLE+body('ref_'))
OUT.joinpath('port.cppm').write_text('module;\n'+PREAMBLE+'\nexport module pbsd.bin.ps.b0291;\nexport namespace pbsd::bin_ps::b0291 {\n'+body('')+'\n}\n')
OUT.joinpath('build.sh').write_text(textwrap.dedent('''\
#!/bin/sh
set -e
cd "$(dirname "$0")"
CC=${CC:-cc}; CXX=${CXX:-c++}; MOD=pbsd.bin.ps.b0291
rm -rf gcm.cache oracle.o port.o harness.o port.pcm harness
$CC -std=c11 -O2 -c oracle.c -o oracle.o
if $CXX --version 2>&1|grep -qi clang; then
 $CXX -std=c++23 -O2 -x c++-module port.cppm --precompile -o port.pcm
 $CXX -std=c++23 -O2 -c port.pcm -o port.o
 $CXX -std=c++23 -O2 -fmodule-file=$MOD=port.pcm -c harness.cpp -o harness.o
else
 $CXX -std=c++23 -fmodules-ts -O2 -c -x c++ port.cppm -o port.o
 $CXX -std=c++23 -fmodules-ts -O2 -c harness.cpp -o harness.o
fi
$CXX -std=c++23 -O2 -o harness harness.o port.o oracle.o -lm -lbsd -Wl,--wrap=malloc -Wl,--wrap=exit
exec ./harness
'''))
print('ok')
