#!/usr/bin/env python3
"""One-shot generator for b0230 oracle.c and port.cppm — not part of deliverable."""
import re
import textwrap
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
HBSD = ROOT / "hbsd/src/bin/sh"
B0219 = ROOT / "pbsd/bin/sh/b0219"
OUT = Path(__file__).resolve().parent

SOURCES = [
    ("mknodes.c", {
        "savestr": "ref_mknodes_savestr",
        "error": "ref_mknodes_error",
        "main": "ref_mknodes_main",
        "parsenode": "ref_parsenode",
        "parsefield": "ref_parsefield",
        "output": "ref_mknodes_output",
        "outsizes": "ref_outsizes",
        "outfunc": "ref_outfunc",
        "indent": "ref_indent",
        "nextfield": "ref_nextfield",
        "skipbl": "ref_skipbl",
        "readline": "ref_readline",
    }),
    ("cd.c", {
        "cdcmd": "ref_cdcmd",
        "docd": "ref_docd",
        "cdlogical": "ref_cdlogical",
        "cdphysical": "ref_cdphysical",
        "getcomponent": "ref_getcomponent",
        "findcwd": "ref_findcwd",
        "updatepwd": "ref_updatepwd",
        "pwdcmd": "ref_pwdcmd",
        "getpwd": "ref_getpwd",
        "getpwd2": "ref_getpwd2",
        "pwd_init": "ref_pwd_init",
    }),
    ("trap.c", {
        "sigstring_to_signum": "ref_sigstring_to_signum",
        "printsignals": "ref_printsignals",
        "trapcmd": "ref_trapcmd",
        "clear_traps": "ref_clear_traps",
        "have_traps": "ref_have_traps",
        "setsignal": "ref_setsignal",
        "getsigaction": "ref_getsigaction",
        "ignoresig": "ref_ignoresig",
        "issigchldtrapped": "ref_issigchldtrapped",
        "onsig": "ref_onsig",
        "dotrap": "ref_dotrap",
        "trap_init": "ref_trap_init",
        "setinteractive": "ref_setinteractive",
        "exitshell": "ref_exitshell",
        "exitshell_savedstatus": "ref_exitshell_savedstatus",
    }),
    ("input.c", {
        "pgetc_macro": "ref_preadbuffer",
        "resetinput": "ref_resetinput",
        "pgetc": "ref_pgetc",
        "preadfd": "ref_preadfd",
        "preadbuffer": "ref_preadbuffer",
        "preadateof": "ref_preadateof",
        "pungetc": "ref_pungetc",
        "pushstring": "ref_pushstring",
        "popstring": "ref_popstring",
        "setinputfile": "ref_setinputfile",
        "setinputfd": "ref_setinputfd",
        "setinputstring": "ref_setinputstring",
        "pushfile": "ref_pushfile",
        "popfile": "ref_popfile",
        "getcurrentfile": "ref_getcurrentfile",
        "popfilesupto": "ref_popfilesupto",
        "popallfiles": "ref_popallfiles",
        "closescript": "ref_closescript",
    }),
]

MEMALLOC_START = 260
MEMALLOC_END = 606
OUTPUT_START = 608
OUTPUT_END = 999

def read_b0219_section(start, end):
    lines = (B0219 / "oracle.c").read_text().splitlines()
    return "\n".join(lines[start - 1:end]) + "\n"


def strip_source(text):
    out = []
    for line in text.splitlines():
        if line.startswith("#include"):
            continue
        # remove static from function definitions
        m = re.match(r'^static\s+(.*)$', line)
        if m and ('(' in m.group(1) or m.group(1).strip().endswith(')')):
            line = m.group(1)
        out.append(line)
    return "\n".join(out)


def apply_defines(text, mapping):
    for old, new in mapping.items():
        text = re.sub(rf'\b{old}\b', new, text)
    return text


ORACLE_HEADER = r'''
/*
 * oracle.c -- reference implementations for batch b0230.
 */

#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef ALIGNBYTES
#define ALIGNBYTES (sizeof(long) - 1)
#endif
#ifndef ALIGN
#define ALIGN(p) (((unsigned long)(p) + ALIGNBYTES) & ~ALIGNBYTES)
#endif

#define __unused
#define NO_HISTORY 1
#define JOBS 1
#define DEBUG 1
#define __printf0like(...)
#define __dead2

typedef void *pointer;
typedef void (*sig_t)(int);

#define equal(s1, s2) (strcmp((s1), (s2)) == 0)

#define out1c(c) outc((c), out1)
#define out2c(c) outcslow((c), out2)

struct oracle_fwopen_cookie {
	void *cookie;
	int (*writefn)(void *, const char *, int);
};

static ssize_t
oracle_fwcookie_write(void *c, const char *buf, size_t size)
{
	struct oracle_fwopen_cookie *fc = (struct oracle_fwopen_cookie *)c;
	int r = fc->writefn(fc->cookie, buf, (int)size);
	return (r < 0) ? -1 : (ssize_t)r;
}

static FILE *
oracle_fwopen(void *cookie, int (*writefn)(void *, const char *, int))
{
	static struct oracle_fwopen_cookie fc;
	static cookie_io_functions_t io = { NULL, oracle_fwcookie_write, NULL, NULL };

	fc.cookie = cookie;
	fc.writefn = writefn;
	return fopencookie(&fc, "w", io);
}

#define fwopen oracle_fwopen

#define ALIASINUSE 1
#define VEXPORT 0x01
#define PEOF 0

struct alias {
	struct alias *next;
	char *name;
	char *val;
	int flag;
};

struct output {
	char *nextc;
	char *bufend;
	char *buf;
	int bufsize;
	short fd;
	short flags;
};

#define OUTBUFSIZ BUFSIZ
#define MEM_OUT -2
#define OUTPUT_ERR 01

struct stack_block {
	struct stack_block *prev;
};
#define SPACE(sp) ((char *)(sp) + ALIGN(sizeof(struct stack_block)))

struct stackmark {
	struct stack_block *stackp;
	char *stacknxt;
	int stacknleft;
};

#define stackblock() stacknxt
#define stackblocksize() stacknleft

#define CHECKSTRSPACE(n, p) \
	{ if ((size_t)(sstrend - p) < (size_t)(n)) p = makestrspace((n), (p)); }

#define STARTSTACKSTR(p) { p = stackblock(); STACKSTRNUL(p); }
#define STPUTC(c, p) do { CHECKSTRSPACE(1, p); *(p)++ = (c); } while (0)
#define STPUTS(s, p) (p = stputs((s), (p)))
#define STACKSTRNUL(p) do { CHECKSTRSPACE(1, p); *(p) = '\0'; } while (0)
#define STTOPC(p) ((p) > stackblock() ? (p)[-1] : '\0')
#define STUNPUTC(p) (--(p))

#define INTOFF oracle_suppressint++
#define INTON do { if (--oracle_suppressint == 0 && oracle_intpending) oracle_onint(); } while (0)
#define is_int_on() oracle_suppressint
#define int_pending() oracle_intpending
#define FORCEINTON do { oracle_suppressint = 0; } while (0)

#define TRACE(x)

static volatile sig_atomic_t oracle_suppressint = 1;
static volatile sig_atomic_t oracle_intpending = 0;

static void oracle_onint(void) {}

static int oracle_error_flag = 0;

void error(const char *fmt, ...)
{
	oracle_error_flag = 1;
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	abort();
}

void warning(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

void errorwithstatus(int status, const char *fmt, ...)
{
	(void)status;
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	abort();
}

char **oracle_argptr;
char *oracle_nextopt_optptr;
char *oracle_shoptarg;

int oracle_nextopt(const char *optstring)
{
	char *p;
	const char *q;
	char c;

	if ((p = oracle_nextopt_optptr) == NULL || *p == '\0') {
		p = *oracle_argptr;
		if (p == NULL || *p != '-' || *++p == '\0')
			return '\0';
		oracle_argptr++;
		if (p[0] == '-' && p[1] == '\0')
			return '\0';
	}
	c = *p++;
	for (q = optstring; *q != c;) {
		if (*q == '\0')
			error("Illegal option -%c", c);
		if (*++q == ':')
			q++;
	}
	if (*++q == ':') {
		if (*p == '\0' && (p = *oracle_argptr++) == NULL)
			error("No arg for -%c option", c);
		oracle_shoptarg = p;
		p = NULL;
	}
	if (p != NULL && *p != '\0')
		oracle_nextopt_optptr = p;
	else
		oracle_nextopt_optptr = NULL;
	return c;
}

#define nextopt oracle_nextopt
#define argptr oracle_argptr
#define shoptarg oracle_shoptarg
#define nextopt_optptr oracle_nextopt_optptr

struct output output = {NULL, NULL, NULL, OUTBUFSIZ, 1, 0};
struct output errout = {NULL, NULL, NULL, 256, 2, 0};
struct output memout = {NULL, NULL, NULL, 64, MEM_OUT, 0};
struct output *out1 = &output;
struct output *out2 = &errout;

char nullstr[1] = {0};

char *stacknxt;
int stacknleft;
char *sstrend;
static struct stack_block *stackp;

#define outc(c, file) \
	((file)->nextc == (file)->bufend ? (emptyoutbuf(file), *(file)->nextc++ = (c)) : (*(file)->nextc++ = (c)))

void oracle_reset_state(void);
struct output *oracle_get_memout(void);
void oracle_set_out1_memout(void);
void oracle_restore_out1(void);

/* shell globals */
int Pflag = 0;
int iflag = 0;
int mflag = 0;
int debug = 0;
int rootshell = 1;
int verifyflag = 0;
int vflag = 0;
int whichprompt = 1;
volatile sig_atomic_t suppressint = 0;
int evalskip = 0;
int skipcount = 0;
int exitstatus = 0;
int oexitstatus = 0;

#define SET_PENDING_INT oracle_intpending = 1

struct jmploc {
	void *loc;
};
struct jmploc *handler = NULL;

int setjmp(struct jmploc *loc) { (void)loc; return 0; }

void onint(void) {}

void evalstring(char *s, int flags) { (void)s; (void)flags; }
void setjobctl(int on) { (void)on; }
void histsave(void) {}
void forcealias(void) {}

#ifndef NSIG
#define NSIG 64
#endif

static const char *oracle_sys_signame[NSIG];
#define sys_signame oracle_sys_signame
int sys_nsig = NSIG;

static int is_number(const char *p)
{
	if (*p == '\0')
		return 0;
	while (*p) {
		if (!isdigit((unsigned char)*p))
			return 0;
		p++;
	}
	return 1;
}

'''

PORT_HEADER = r'''
module;

#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1

#include <ctype.h>
#include <cstdarg>
#include <cerrno>
#include <cinttypes>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef ALIGNBYTES
#define ALIGNBYTES (sizeof(long) - 1)
#endif
#ifndef ALIGN
#define ALIGN(p) (((unsigned long)(p) + ALIGNBYTES) & ~ALIGNBYTES)
#endif

#ifndef O_VERIFY
#define O_VERIFY 0
#endif

export module pbsd.bin.sh.b0230;

export namespace pbsd::bin_sh::b0230 {

#define __unused
#define NO_HISTORY 1
#define JOBS 1
#define DEBUG 1
#define __printf0like(...)
#define __dead2

typedef void *pointer;
typedef void (*sig_t)(int);

#define equal(s1, s2) (std::strcmp((s1), (s2)) == 0)

struct alias {
	struct alias *next;
	char *name;
	char *val;
	int flag;
};

struct output {
	char *nextc;
	char *bufend;
	char *buf;
	int bufsize;
	short fd;
	short flags;
};

#define OUTBUFSIZ BUFSIZ
#define MEM_OUT -2
#define OUTPUT_ERR 01
#define ALIASINUSE 1
#define VEXPORT 0x01
#define PEOF 0

struct stack_block {
	struct stack_block *prev;
};
#define SPACE(sp) ((char *)(sp) + ALIGN(sizeof(struct stack_block)))

struct stackmark {
	struct stack_block *stackp;
	char *stacknxt;
	int stacknleft;
};

#define stackblock() stacknxt
#define stackblocksize() stacknleft

#define CHECKSTRSPACE(n, p) \
	{ if ((size_t)(sstrend - p) < (size_t)(n)) p = makestrspace((n), (p)); }

#define STARTSTACKSTR(p) { p = stackblock(); STACKSTRNUL(p); }
#define STPUTC(c, p) do { CHECKSTRSPACE(1, p); *(p)++ = (c); } while (0)
#define STPUTS(s, p) (p = stputs((s), (p)))
#define STACKSTRNUL(p) do { CHECKSTRSPACE(1, p); *(p) = '\0'; } while (0)
#define STTOPC(p) ((p) > stackblock() ? (p)[-1] : '\0')
#define STUNPUTC(p) (--(p))

static volatile sig_atomic_t port_suppressint = 1;
static volatile sig_atomic_t port_intpending = 0;

static void port_onint(void) {}

#define INTOFF port_suppressint++
#define INTON do { if (--port_suppressint == 0 && port_intpending) port_onint(); } while (0)
#define is_int_on() port_suppressint
#define int_pending() port_intpending
#define FORCEINTON do { port_suppressint = 0; } while (0)

#define TRACE(x)

static int port_error_flag = 0;

void error(const char *fmt, ...)
{
	port_error_flag = 1;
	va_list ap;
	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fprintf(stderr, "\n");
	std::abort();
}

void warning(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fprintf(stderr, "\n");
}

void errorwithstatus(int status, const char *fmt, ...)
{
	(void)status;
	va_list ap;
	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fprintf(stderr, "\n");
	std::abort();
}

char **port_argptr;
char *port_nextopt_optptr;
char *port_shoptarg;

int port_nextopt(const char *optstring)
{
	char *p;
	const char *q;
	char c;

	if ((p = port_nextopt_optptr) == NULL || *p == '\0') {
		p = *port_argptr;
		if (p == NULL || *p != '-' || *++p == '\0')
			return '\0';
		port_argptr++;
		if (p[0] == '-' && p[1] == '\0')
			return '\0';
	}
	c = *p++;
	for (q = optstring; *q != c;) {
		if (*q == '\0')
			error("Illegal option -%c", c);
		if (*++q == ':')
			q++;
	}
	if (*++q == ':') {
		if (*p == '\0' && (p = *port_argptr++) == NULL)
			error("No arg for -%c option", c);
		port_shoptarg = p;
		p = NULL;
	}
	if (p != NULL && *p != '\0')
		port_nextopt_optptr = p;
	else
		port_nextopt_optptr = NULL;
	return c;
}

#define nextopt port_nextopt
#define argptr port_argptr
#define shoptarg port_shoptarg
#define nextopt_optptr port_nextopt_optptr

static struct output output = {NULL, NULL, NULL, OUTBUFSIZ, 1, 0};
static struct output errout = {NULL, NULL, NULL, 256, 2, 0};
static struct output memout = {NULL, NULL, NULL, 64, MEM_OUT, 0};
static struct output *out1 = &output;
static struct output *out2 = &errout;

char port_nullstr[1] = {0};

static char *stacknxt;
static int stacknleft;
static char *sstrend;
static struct stack_block *stackp;

struct fwopen_cookie {
	void *cookie;
	int (*writefn)(void *, const char *, int);
};

static ssize_t
port_fwcookie_write(void *c, const char *buf, size_t size)
{
	struct fwopen_cookie *fc = (struct fwopen_cookie *)c;
	int r = fc->writefn(fc->cookie, buf, (int)size);
	return (r < 0) ? -1 : (ssize_t)r;
}

static FILE *
port_fwopen(void *cookie, int (*writefn)(void *, const char *, int))
{
	static struct fwopen_cookie fc;
	static cookie_io_functions_t io = { NULL, port_fwcookie_write, NULL, NULL };

	fc.cookie = cookie;
	fc.writefn = writefn;
	return fopencookie(&fc, "w", io);
}

#define fwopen port_fwopen

void emptyoutbuf(struct output *dest);
void outstr(const char *p, struct output *file);
void outqstr(const char *p, struct output *file);
void outbin(const void *data, size_t len, struct output *file);
void outcslow(int c, struct output *file);
void flushout(struct output *dest);
int xwrite(int fd, const char *buf, int nbytes);
void doformat(struct output *dest, const char *f, va_list ap);

#define outc(c, file) \
	((file)->nextc == (file)->bufend ? (emptyoutbuf(file), *(file)->nextc++ = (c)) : (*(file)->nextc++ = (c)))

#define out1c(c) outc((c), out1)
#define out2c(c) outcslow((c), out2)

void port_reset_state(void);

int Pflag = 0;
int iflag = 0;
int mflag = 0;
int debug = 0;
int rootshell = 1;
int verifyflag = 0;
int vflag = 0;
volatile sig_atomic_t suppressint = 0;
int evalskip = 0;
int skipcount = 0;
int exitstatus = 0;
int oexitstatus = 0;

#define SET_PENDING_INT port_intpending = 1

struct jmploc {
	void *loc;
};
struct jmploc *handler = NULL;

int setjmp(struct jmploc *loc) { (void)loc; return 0; }

void onint(void) {}

void evalstring(char *s, int flags) { (void)s; (void)flags; }
void setjobctl(int on) { (void)on; }
void histsave(void) {}
void forcealias(void) {}

#ifndef NSIG
#define NSIG 64
#endif

static const char *port_sys_signame[NSIG];
#define sys_signame port_sys_signame
int sys_nsig = NSIG;

static int is_number(const char *p)
{
	if (*p == '\0')
		return 0;
	while (*p) {
		if (!isdigit((unsigned char)*p))
			return 0;
		p++;
	}
	return 1;
}

void port_set_out1_memout(void);
void port_restore_out1(void);
struct output *port_get_memout(void);

'''

PORT_FOOTER = "\n} // namespace pbsd::bin_sh::b0230\n"

ORACLE_SHELL_STUBS = r'''
void hashcd(void) {}

struct varpair { char *name; char *val; int flags; };
static struct varpair oracle_vars[128];
static int oracle_var_n = 0;

char *lookupvar(const char *name)
{
	int i;
	for (i = 0; i < oracle_var_n; i++)
		if (strcmp(oracle_vars[i].name, name) == 0)
			return oracle_vars[i].val;
	return NULL;
}

void setvar(const char *name, const char *val, int flags)
{
	int i;
	for (i = 0; i < oracle_var_n; i++) {
		if (strcmp(oracle_vars[i].name, name) == 0) {
			if (oracle_vars[i].val)
				free(oracle_vars[i].val);
			oracle_vars[i].val = val ? strdup(val) : NULL;
			oracle_vars[i].flags = flags;
			return;
		}
	}
	if (oracle_var_n < 128) {
		oracle_vars[oracle_var_n].name = strdup(name);
		oracle_vars[oracle_var_n].val = val ? strdup(val) : NULL;
		oracle_vars[oracle_var_n].flags = flags;
		oracle_var_n++;
	}
}

char *bltinlookup(const char *name, int remove)
{
	char *v = lookupvar(name);
	(void)remove;
	return v;
}

char *padvance(char **path, const char *dot, const char *dest)
{
	char *p, *q, *r;
	static char padbuf[PATH_MAX];

	(void)dot;
	p = *path;
	if (p == NULL)
		return NULL;
	if (*p == '\0') {
		*path = NULL;
		return stsavestr(dest);
	}
	q = padbuf;
	while (*p != '\0' && *p != ':') {
		if (q < padbuf + PATH_MAX - 1)
			*q++ = *p;
		p++;
	}
	if (*p == ':')
		p++;
	*path = p;
	if (q == padbuf) {
		r = stsavestr(dest);
		return r;
	}
	if (q[-1] != '/')
		*q++ = '/';
	r = dest;
	while (*r != '\0' && q < padbuf + PATH_MAX - 1)
		*q++ = *r++;
	*q = '\0';
	return stsavestr(padbuf);
}

void oracle_reset_state(void)
{
	int i;
	oracle_suppressint = 1;
	oracle_intpending = 0;
	oracle_error_flag = 0;
	oracle_argptr = NULL;
	oracle_nextopt_optptr = NULL;
	oracle_shoptarg = NULL;
	out1 = &output;
	out2 = &errout;
	if (output.buf) { free(output.buf); output.buf = NULL; }
	output.nextc = NULL;
	output.bufend = NULL;
	output.flags = 0;
	if (errout.buf) { free(errout.buf); errout.buf = NULL; }
	errout.nextc = NULL;
	errout.bufend = NULL;
	errout.flags = 0;
	if (memout.buf) { free(memout.buf); memout.buf = NULL; }
	memout.nextc = NULL;
	memout.bufend = NULL;
	memout.bufsize = 64;
	memout.flags = 0;
	while (stackp) {
		struct stack_block *sp = stackp;
		stackp = sp->prev;
		free(sp);
	}
	stacknxt = NULL;
	stacknleft = 0;
	sstrend = NULL;
	for (i = 0; i < oracle_var_n; i++) {
		free(oracle_vars[i].name);
		free(oracle_vars[i].val);
	}
	oracle_var_n = 0;
	Pflag = 0;
	iflag = 0;
	mflag = 0;
	debug = 0;
	rootshell = 1;
	verifyflag = 0;
	vflag = 0;
	whichprompt = 1;
	suppressint = 0;
	evalskip = 0;
	skipcount = 0;
	exitstatus = 0;
	oexitstatus = 0;
}

struct output *oracle_get_memout(void) { return &memout; }
void oracle_set_out1_memout(void) { out1 = &memout; }
void oracle_restore_out1(void) { out1 = &output; }

'''

PORT_SHELL_STUBS = r'''
void hashcd(void) {}

struct varpair { char *name; char *val; int flags; };
static struct varpair port_vars[128];
static int port_var_n = 0;

char *lookupvar(const char *name)
{
	int i;
	for (i = 0; i < port_var_n; i++)
		if (strcmp(port_vars[i].name, name) == 0)
			return port_vars[i].val;
	return NULL;
}

void setvar(const char *name, const char *val, int flags)
{
	int i;
	for (i = 0; i < port_var_n; i++) {
		if (strcmp(port_vars[i].name, name) == 0) {
			if (port_vars[i].val)
				free(port_vars[i].val);
			port_vars[i].val = val ? strdup(val) : NULL;
			port_vars[i].flags = flags;
			return;
		}
	}
	if (port_var_n < 128) {
		port_vars[port_var_n].name = strdup(name);
		port_vars[port_var_n].val = val ? strdup(val) : NULL;
		port_vars[port_var_n].flags = flags;
		port_var_n++;
	}
}

char *bltinlookup(const char *name, int remove)
{
	char *v = lookupvar(name);
	(void)remove;
	return v;
}

char *padvance(char **path, const char *dot, const char *dest)
{
	char *p, *q, *r;
	static char padbuf[PATH_MAX];

	(void)dot;
	p = *path;
	if (p == NULL)
		return NULL;
	if (*p == '\0') {
		*path = NULL;
		return stsavestr(dest);
	}
	q = padbuf;
	while (*p != '\0' && *p != ':') {
		if (q < padbuf + PATH_MAX - 1)
			*q++ = *p;
		p++;
	}
	if (*p == ':')
		p++;
	*path = p;
	if (q == padbuf) {
		r = stsavestr(dest);
		return r;
	}
	if (q[-1] != '/')
		*q++ = '/';
	r = dest;
	while (*r != '\0' && q < padbuf + PATH_MAX - 1)
		*q++ = *r++;
	*q = '\0';
	return stsavestr(padbuf);
}

void port_reset_state(void)
{
	int i;
	port_suppressint = 1;
	port_intpending = 0;
	port_error_flag = 0;
	port_argptr = NULL;
	port_nextopt_optptr = NULL;
	port_shoptarg = NULL;
	out1 = &output;
	out2 = &errout;
	if (output.buf) { free(output.buf); output.buf = NULL; }
	output.nextc = NULL;
	output.bufend = NULL;
	output.flags = 0;
	if (errout.buf) { free(errout.buf); errout.buf = NULL; }
	errout.nextc = NULL;
	errout.bufend = NULL;
	errout.flags = 0;
	if (memout.buf) { free(memout.buf); memout.buf = NULL; }
	memout.nextc = NULL;
	memout.bufend = NULL;
	memout.bufsize = 64;
	memout.flags = 0;
	while (stackp) {
		struct stack_block *sp = stackp;
		stackp = sp->prev;
		free(sp);
	}
	stacknxt = NULL;
	stacknleft = 0;
	sstrend = NULL;
	for (i = 0; i < port_var_n; i++) {
		free(port_vars[i].name);
		free(port_vars[i].val);
	}
	port_var_n = 0;
	Pflag = 0;
	iflag = 0;
	mflag = 0;
	debug = 0;
	rootshell = 1;
	verifyflag = 0;
	vflag = 0;
	whichprompt = 1;
	suppressint = 0;
	evalskip = 0;
	skipcount = 0;
	exitstatus = 0;
	oexitstatus = 0;
}

void port_set_out1_memout(void) { out1 = &memout; }
void port_restore_out1(void) { out1 = &output; }
struct output *port_get_memout(void) { return &memout; }

'''

def cppify(text):
    std_funcs = [
        'vfprintf', 'vsnprintf', 'fprintf', 'fputc', 'fputs', 'fgets',
        'fclose', 'fopen', 'ferror', 'memmove', 'memcpy', 'memset',
        'strlen', 'strcmp', 'strcpy', 'strncpy', 'strchr', 'strspn',
        'malloc', 'free', 'realloc', 'abort', 'exit', 'atoi', 'sprintf',
    ]
    posix_funcs = [
        'write', 'read', 'close', 'open', 'chdir', 'getcwd', 'lstat',
        'stat', 'fcntl', 'kill', 'getpid', '_exit', 'signal', 'sigaction',
        'sigemptyset', 'sigaddset', 'sigprocmask', 'strchrnul',
        'strncasecmp', 'strcasecmp',
    ]
    for f in std_funcs:
        text = re.sub(r'(?<![:\w])' + f + r'\(', 'std::' + f + '(', text)
    for f in posix_funcs:
        text = re.sub(r'(?<![:\w])' + f + r'\(', '::' + f + '(', text)
    return text


def portify_source(text, mapping):
    text = strip_source(text)
  # port uses original names, not ref_
    return text


def oracleify_source(text, mapping):
    text = strip_source(text)
    # add #defines before section
    defines = "\n".join(f"#define {k} {v}" for k, v in mapping.items())
    text = apply_defines(text, mapping)
    return defines + "\n\n" + text


def main():
    oracle = ORACLE_HEADER
    oracle += read_b0219_section(MEMALLOC_START, MEMALLOC_END)
    oracle += ORACLE_SHELL_STUBS
    oracle += read_b0219_section(OUTPUT_START, OUTPUT_END)

    port = PORT_HEADER
    # port memalloc+output from b0219 port.cppm
    port_b0219 = (B0219 / "port.cppm").read_text()
    m = re.search(r'/\* --- memalloc\.c --- \*/', port_b0219)
    m2 = re.search(r'/\* --- alias\.c --- \*/', port_b0219)
    if m and m2:
        port += port_b0219[m.start():m2.start()]
    port += PORT_SHELL_STUBS

    for fname, mapping in SOURCES:
        src = (HBSD / fname).read_text()
        oracle += f"\n/* --- {fname} --- */\n"
        oracle += oracleify_source(src, mapping)
        port += f"\n/* --- {fname} --- */\n"
        if fname == "mknodes.c":
            port += "#define output mknodes_output\n"
            port += "#define main mknodes_main\n"
            port += "#define error mknodes_error\n"
            port += "#define savestr mknodes_savestr\n"
        if fname == "cd.c":
            port += "#define new cd_new_var\n"
        if fname == "input.c":
            port += "#define pgetc_macro preadbuffer\n"
        port += portify_source(src, mapping)
        if fname == "mknodes.c":
            port += "\n#undef output\n#undef main\n#undef error\n#undef savestr\n\n"
        if fname == "cd.c":
            port += "\n#undef new\n\n"

    oracle += "\n"
    port += PORT_FOOTER

    (OUT / "oracle.c").write_text(oracle)
    (OUT / "port.cppm").write_text(cppify(port))
    print("Generated oracle.c and port.cppm")


if __name__ == "__main__":
    main()
