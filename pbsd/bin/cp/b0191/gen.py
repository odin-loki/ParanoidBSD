#!/usr/bin/env python3
import pathlib, re

ROOT = pathlib.Path(__file__).resolve().parents[4]
utils_src = (ROOT / 'hbsd/src/bin/cp/utils.c').read_text()
cp_src = (ROOT / 'hbsd/src/bin/cp/cp.c').read_text()
OUT = pathlib.Path(__file__).resolve().parent

def strip_includes(text):
    return ''.join(l for l in text.splitlines(True) if not l.startswith('#include'))

def transform_cp(cp_body, ref=False):
    cp = cp_body
    cp = cp.replace('static char dot[] = ".";',
                    'static char ref_dot[] = ".";\n#define dot ref_dot' if ref else 'char dot[] = ".";')
    cp = cp.replace('static int\nftscmp', 'int\nref_ftscmp' if ref else 'int\nftscmp')
    cp = cp.replace('static int\ncopy(', 'int\nref_copy(' if ref else 'int\ncopy(')
    cp = cp.replace('static void\nsiginfo', 'void\nref_siginfo' if ref else 'void\nsiginfo')
    if ref:
        cp = cp.replace('int\nmain(', 'int\nref_main(')
        cp = cp.replace('copy(char *argv[], enum op type,', 'ref_copy(char *argv[], enum ref_op type,')
    cp = re.sub(r'^PATH_T to = \{ \.dir = -1, \.end = to\.path \};\n', '', cp, flags=re.M)
    cp = re.sub(r'^bool Nflag, fflag, iflag, lflag, nflag, pflag, sflag, vflag;\n', '', cp, flags=re.M)
    cp = re.sub(r'^static bool Hflag, Lflag, Pflag, Rflag, rflag, Sflag;\n', '', cp, flags=re.M)
    cp = re.sub(r'^bool Hflag, Lflag, Pflag, Rflag, rflag, Sflag;\n', '', cp, flags=re.M)
    cp = re.sub(r'^volatile sig_atomic_t info;\n', '', cp, flags=re.M)
    cp = re.sub(r'^enum op \{ FILE_TO_FILE, FILE_TO_DIR, DIR_TO_DNE \};\n', '', cp, flags=re.M)
    cp = re.sub(r'^static int copy\(char \*\[\], enum op, int, struct stat \*\);\n', '', cp, flags=re.M)
    cp = re.sub(r'^static void siginfo\(int __unused\);\n', '', cp, flags=re.M)
    if ref:
        cp = cp.replace('enum op type;', 'enum ref_op type;')
    return cp

def transform_utils(utils_body, ref=False):
    u = utils_body
    if ref:
        u = u.replace('static ssize_t\ncopy_fallback', 'ssize_t\nref_copy_fallback')
        for fn, ret in [
            ('copy_file', 'int'), ('copy_link', 'int'), ('copy_fifo', 'int'),
            ('copy_special', 'int'), ('setfile', 'int'), ('preserve_fd_acls', 'int'),
            ('preserve_dir_acls', 'int'), ('usage', 'void')
        ]:
            u = u.replace(f'{ret}\n{fn}(', f'{ret}\nref_{fn}(')
    else:
        u = u.replace('static ssize_t\ncopy_fallback', 'ssize_t\ncopy_fallback')
        u = u.replace('buf = malloc(bufsize);', 'buf = (char *)malloc(bufsize);')
    return u

def transform_port_cp(cp_body):
    cp = transform_cp(cp_body, ref=False)
    cp = cp.replace('(void)signal(SIGINFO, siginfo);',
                    '(void)signal(SIGINFO, +siginfo);')
    cp = cp.replace('Sflag ? ftscmp : NULL',
                    'Sflag ? (int (*)(const FTSENT **, const FTSENT **))ftscmp : NULL')
    return cp

def transform_port_utils(utils_body):
    return transform_utils(utils_body, ref=False)

LINUX_COMPAT = '''
#if defined(__linux__)
#define st_flags st_blksize
#include <bsd/string.h>
#include <bsd/err.h>
#else
#include <string.h>
#endif

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif
#ifndef SIGINFO
#define SIGINFO SIGUSR1
#endif
#ifndef O_SEARCH
#define O_SEARCH 0
#endif
'''

COMMON_INCLUDES = r'''
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/param.h>
#include <sys/stat.h>
#if defined(__linux__)
#define st_flags st_blksize
#endif
#include <sys/types.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#ifdef siginfo
#undef siginfo
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
''' + LINUX_COMPAT

utils_o = transform_utils(strip_includes(utils_src), ref=True)
cp_o = transform_cp(strip_includes(cp_src), ref=True)

oracle_preamble = r'''/*-
 * Oracle for PBSD batch b0191 -- concatenated cp/utils.c and cp/cp.c
 */
''' + COMMON_INCLUDES + r'''

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef MAXPHYS
#define MAXPHYS 65536
#endif

#ifndef AT_RESOLVE_BENEATH
#define AT_RESOLVE_BENEATH 0
#endif
#ifndef O_RESOLVE_BENEATH
#define O_RESOLVE_BENEATH 0
#endif
#ifndef ENOTCAPABLE
#define ENOTCAPABLE EACCES
#endif

typedef void *acl_t;
typedef unsigned int acl_type_t;
#ifndef ACL_TYPE_NFS4
#define ACL_TYPE_NFS4 0x00000004
#endif
#ifndef ACL_TYPE_ACCESS
#define ACL_TYPE_ACCESS 0x00000002
#endif
#ifndef _PC_ACL_NFS4
#define _PC_ACL_NFS4 64
#endif
#ifndef _PC_ACL_EXTENDED
#define _PC_ACL_EXTENDED 65
#endif

acl_t acl_get_fd_np(int, acl_type_t);
int acl_is_trivial_np(acl_t, int *);
int acl_set_fd_np(int, acl_t, acl_type_t);
int acl_free(acl_t);
int fchflags(int, unsigned long);
int chflagsat(int, const char *, unsigned long, int);

typedef struct {
	int		 dir;
	char		 base[PATH_MAX + 1];
	char		*end;
	char		 path[PATH_MAX];
} PATH_T;

enum ref_op { FILE_TO_FILE, FILE_TO_DIR, DIR_TO_DNE };

PATH_T ref_to = { .dir = -1, .end = ref_to.path };
bool ref_Nflag, ref_fflag, ref_iflag, ref_lflag, ref_nflag, ref_pflag, ref_sflag, ref_vflag;
static bool ref_Hflag, ref_Lflag, ref_Pflag, ref_Rflag, ref_rflag, ref_Sflag;
volatile sig_atomic_t ref_info;

int ref_setfile(struct stat *, int, bool);
int ref_preserve_fd_acls(int, int);
int ref_preserve_dir_acls(const char *, const char *);
void ref_usage(void);
int ref_copy(char *[], enum ref_op, int, struct stat *);
void ref_siginfo(int sig);
int ref_ftscmp(const FTSENT * const *, const FTSENT * const *);

#define to ref_to
#define Nflag ref_Nflag
#define fflag ref_fflag
#define iflag ref_iflag
#define lflag ref_lflag
#define nflag ref_nflag
#define pflag ref_pflag
#define sflag ref_sflag
#define vflag ref_vflag
#define Hflag ref_Hflag
#define Lflag ref_Lflag
#define Pflag ref_Pflag
#define Rflag ref_Rflag
#define rflag ref_rflag
#define Sflag ref_Sflag
#define info ref_info

#define copy_fallback ref_copy_fallback
#define copy_file ref_copy_file
#define copy_link ref_copy_link
#define copy_fifo ref_copy_fifo
#define copy_special ref_copy_special
#define setfile ref_setfile
#define preserve_fd_acls ref_preserve_fd_acls
#define preserve_dir_acls ref_preserve_dir_acls
#define usage ref_usage
#define main ref_main
#define ftscmp ref_ftscmp
#define copy ref_copy
#define siginfo ref_siginfo

'''

utils_p = transform_port_utils(strip_includes(utils_src))
cp_p = transform_port_cp(strip_includes(cp_src))

port_preamble = r'''module;

#ifndef __dead2
#define __dead2 __attribute__((__noreturn__))
#endif
''' + COMMON_INCLUDES + r'''

export module pbsd.bin.cp.b0191;

export namespace pbsd::bin_cp::b0191 {

#ifndef MAXPHYS
#define MAXPHYS 65536
#endif

#ifndef AT_RESOLVE_BENEATH
#define AT_RESOLVE_BENEATH 0
#endif
#ifndef O_RESOLVE_BENEATH
#define O_RESOLVE_BENEATH 0
#endif
#ifndef ENOTCAPABLE
#define ENOTCAPABLE EACCES
#endif

typedef void *acl_t;
typedef unsigned int acl_type_t;
#ifndef ACL_TYPE_NFS4
#define ACL_TYPE_NFS4 0x00000004
#endif
#ifndef ACL_TYPE_ACCESS
#define ACL_TYPE_ACCESS 0x00000002
#endif
#ifndef _PC_ACL_NFS4
#define _PC_ACL_NFS4 64
#endif
#ifndef _PC_ACL_EXTENDED
#define _PC_ACL_EXTENDED 65
#endif

acl_t acl_get_fd_np(int, acl_type_t);
int acl_is_trivial_np(acl_t, int *);
int acl_set_fd_np(int, acl_t, acl_type_t);
int acl_free(acl_t);
int fchflags(int, unsigned long);
int chflagsat(int, const char *, unsigned long, int);

typedef struct {
	int		 dir;
	char		 base[PATH_MAX + 1];
	char		*end;
	char		 path[PATH_MAX];
} PATH_T;

enum op { FILE_TO_FILE, FILE_TO_DIR, DIR_TO_DNE };

PATH_T to;
bool Nflag, fflag, iflag, lflag, nflag, pflag, sflag, vflag;
bool Hflag, Lflag, Pflag, Rflag, rflag, Sflag;
volatile sig_atomic_t info;

int setfile(struct stat *, int, bool);
int preserve_fd_acls(int, int);
int preserve_dir_acls(const char *, const char *);
void usage(void);
int copy(char *[], enum op, int, struct stat *);
void siginfo(int sig __unused);

'''

(OUT / 'oracle.c').write_text(oracle_preamble + '\n/* --- utils.c --- */\n' + utils_o + '\n/* --- cp.c --- */\n' + cp_o)
(OUT / 'port.cppm').write_text(port_preamble + '\n/* --- utils.c --- */\n' + utils_p + '\n/* --- cp.c --- */\n' + cp_p + '\n} // namespace\n')
print('generated oracle.c and port.cppm')
