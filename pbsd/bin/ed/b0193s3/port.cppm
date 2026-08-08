// PBSD port of HardenedBSD ed batch b0193s3 (main.c + supporting ed routines).

module;

#include <cctype>
#include <cerrno>
#include <climits>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <regex.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <unistd.h>

export module pbsd.bin.ed.b0193s3;

export namespace pbsd::bin_ed::b0193s3 {

#define ERR		(-2)
#define EMOD		(-3)
#define FATAL		(-4)

#define MINBUFSZ 512
#define SE_MAX 30
#ifdef INT_MAX
# define LINECHARS INT_MAX
#else
# define LINECHARS MAXINT
#endif

#define GLB 001
#define GPR 002
#define GLS 004
#define GNP 010
#define GSG 020

using pattern_t = regex_t;

struct line_t {
	line_t	*q_forw;
	line_t	*q_back;
	off_t	seek;
	int	len;
};

struct undo_t {
#define UADD	0
#define UDEL 	1
#define UMOV	2
#define VMOV	3
	int type;
	line_t	*h;
	line_t	*t;
};

#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
# define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define INC_MOD(l, k)	((l) + 1 > (k) ? 0 : (l) + 1)
#define DEC_MOD(l, k)	((l) - 1 < 0 ? (k) : (l) - 1)

int &mutex_ref();
int &sigflags_ref();

#define SPL1() mutex_ref()++

#define SPL0() \
if (--mutex_ref() == 0) { \
	if (sigflags_ref() & (1 << (SIGHUP - 1))) handle_hup(SIGHUP); \
	if (sigflags_ref() & (1 << (SIGINT - 1))) handle_int(SIGINT); \
}

#define REALLOC(b,n,i,err) \
if ((i) > (n)) { \
	size_t ti = (n); \
	char *ts; \
	SPL1(); \
	if ((ts = (char *) realloc((b), ti += max((i), MINBUFSZ))) == NULL) { \
		fprintf(stderr, "%s\n", strerror(errno)); \
		errmsg = "out of memory"; \
		SPL0(); \
		return err; \
	} \
	(n) = ti; \
	(b) = ts; \
	SPL0(); \
}

#define REQUE(pred, succ) (pred)->q_forw = (succ), (succ)->q_back = (pred)

#define INSQUE(elem, pred) \
{ \
	REQUE((elem), (pred)->q_forw); \
	REQUE((pred), elem); \
}

#define REMQUE(elem) REQUE((elem)->q_back, (elem)->q_forw);

#define NUL_TO_NEWLINE(s, l) translit_text(s, l, '\0', '\n')
#define NEWLINE_TO_NUL(s, l) translit_text(s, l, '\n', '\0')

#define STRTOL(i, p) { \
	if (((i = strtol(p, &p, 10)) == LONG_MIN || i == LONG_MAX) && \
	    errno == ERANGE) { \
		errmsg = "number out of range"; \
		i = 0; \
		return ERR; \
	} \
}

char stdinbuf_arr[1];
char ibuf_storage[65536];
char *ibuf = ibuf_storage;
char *ibufp;
int ibufsz;

int isbinary;
int isglobal;
int modified;
int mutex;
int sigflags;

long addr_last;
long current_addr;
long first_addr;
long second_addr;
long u_addr_last = -1;
long u_current_addr = -1;
int lineno;
int newline_added;
int scripted;
int patlock;

const char *errmsg = "";

static const char *prompt;
static const char *dps = "*";
char *optarg = nullptr;
static int garrulous = 0;
static int red = 0;
static char old_filename[PATH_MAX] = "";
static char *shcmd;
static int shcmdsz;
static int shcmdi;
static long addr_cnt;
static int sigactive = 0;
long rows = 22;
int cols = 72;
#ifdef _POSIX_SOURCE
static sigjmp_buf env;
#else
static jmp_buf env;
#endif


int &mutex_ref() { return mutex; }
int &sigflags_ref() { return sigflags; }

int malloc_fail_at = 0;
int malloc_calls = 0;
int quit_status = 0;
int quit_called = 0;
jmp_buf quit_jmp;

void *port_malloc(std::size_t n)
{
	void *p;
	malloc_calls++;
	if (malloc_fail_at != 0 && malloc_calls >= malloc_fail_at)
		return nullptr;
	p = std::malloc(n);
	return p;
}

void reset_hooks()
{
	malloc_fail_at = 0;
	malloc_calls = 0;
	quit_status = 0;
	quit_called = 0;
}

void reset_globals()
{
	ibufp = ibuf;
	ibufsz = (int)sizeof(ibuf_storage);
	isbinary = 0;
	isglobal = 0;
	modified = 0;
	mutex = 0;
	sigflags = 0;
	addr_last = 0;
	current_addr = 0;
	first_addr = 1;
	second_addr = 1;
	u_addr_last = -1;
	u_current_addr = -1;
	lineno = 0;
	newline_added = 0;
	scripted = 0;
	patlock = 0;
	rows = 22;
	cols = 72;
	errmsg = "";
	garrulous = 0;
	red = 0;
	old_filename[0] = '\0';
	addr_cnt = 0;
	sigactive = 0;
	reset_hooks();
}

void quit_enter(void)
{
	if (setjmp(quit_jmp) != 0)
		return;
}

#define port_exit(n) \
	do { \
		quit_status = (n); \
		quit_called = 1; \
		longjmp(quit_jmp, 1); \
	} while (0)


pattern_t *get_compiled_pattern(void);
char *extract_pattern(int delimiter);
char *parse_char_class(char *s);
undo_t *push_undo_stack(int type, long from, long to);
int pop_undo_stack(void);
void clear_undo_stack(void);
int build_active_list(int isgcmd);
long exec_global(int interact, int gflag);
int set_active_node(line_t *lp);
void unset_active_nodes(line_t *np, line_t *mp);
line_t *next_active_node(void);
void clear_active_list(void);
char *get_sbuf_line(line_t *lp);
const char *put_sbuf_line(const char *cs);
void add_line_node(line_t *lp);
long get_line_node_addr(line_t *lp);
line_t *get_addressed_line_node(long n);
int open_sbuf(void);
int close_sbuf(void);
void quit(int n);
void init_buffers(void);
char *translit_text(char *s, int len, int from, int to);
int extract_addr_range(void);
long next_addr(void);
int exec_command(void);
int check_addr_range(long n, long m);
long get_matching_node_addr(pattern_t *pat, int dir);
char *get_filename(void);
int get_shell_command(void);
int append_lines(long n);
int join_lines(long from, long to);
int move_lines(long addr);
int copy_lines(long addr);
int delete_lines(long from, long to);
int display_lines(long from, long to, int gflag);
int mark_line_node(line_t *lp, int n);
long get_marked_node_addr(int n);
void unmark_line_node(line_t *lp);
line_t *dup_line_node(line_t *lp);
int has_trailing_escape(char *s, char *t);
char *strip_escapes(char *s);
void signal_hup(int signo);
void signal_int(int signo);
void handle_hup(int signo);
void handle_int(int signo);
void handle_winch(int signo);
int is_legal_filename(char *s);
int extract_subst_tail(int *flagp, long *np);
char *extract_subst_template(void);
int search_and_replace(pattern_t *pat, int gflag, int kth);
int substitute_matching_text(pattern_t *pat, line_t *lp, int gflag, int kth);
int apply_subst_template(const char *boln, regmatch_t *rm, int off, int re_nsub);
long read_file(char *fn, long n);
long read_stream(FILE *fp, long n);
int get_stream_line(FILE *fp);
long write_file(char *fn, const char *mode, long n, long m);
long write_stream(FILE *fp, long n, long m);
int put_stream_line(FILE *fp, const char *s, int len);
char *get_extended_line(int *sizep, int nonl);
int get_tty_line(void);
int put_tty_line(const char *s, int l, long n, int gflag);
void clear_marks(void);


/* re.c: This file contains the regular expression interface routines for
   the ed line editor. */
/*-
 * Copyright (c) 1993 Andrew Moore, Talke Studio.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
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

pattern_t *
get_compiled_pattern(void)
{
	static pattern_t *expr = NULL;
	static char error[1024];

	char *exprs;
	char delimiter;
	int n;

	if ((delimiter = *ibufp) == ' ') {
		errmsg = "invalid pattern delimiter";
		return NULL;
	} else if (delimiter == '\n' || *++ibufp == '\n' || *ibufp == delimiter) {
		if (!expr)
			errmsg = "no previous pattern";
		return expr;
	} else if ((exprs = extract_pattern(delimiter)) == NULL)
		return NULL;
	/* buffer alloc'd && not reserved */
	if (expr && !patlock)
		regfree(expr);
	else if ((expr = (pattern_t *) port_malloc(sizeof(pattern_t))) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "out of memory";
		return NULL;
	}
	patlock = 0;
	if ((n = regcomp(expr, exprs, 0))) {
		regerror(n, expr, error, sizeof error);
		errmsg = error;
		free(expr);
		return expr = NULL;
	}
	return expr;
}


char *
extract_pattern(int delimiter)
{
	static char *lhbuf = NULL;	/* buffer */
	static int lhbufsz = 0;		/* buffer size */

	char *nd;
	int len;

	for (nd = ibufp; *nd != delimiter && *nd != '\n'; nd++)
		switch (*nd) {
		default:
			break;
		case '[':
			if ((nd = parse_char_class(nd + 1)) == NULL) {
				errmsg = "unbalanced brackets ([])";
				return NULL;
			}
			break;
		case '\\':
			if (*++nd == '\n') {
				errmsg = "trailing backslash (\\)";
				return NULL;
			}
			break;
		}
	len = nd - ibufp;
	REALLOC(lhbuf, lhbufsz, len + 1, NULL);
	memcpy(lhbuf, ibufp, len);
	lhbuf[len] = '\0';
	ibufp = nd;
	return (isbinary) ? NUL_TO_NEWLINE(lhbuf, len) : lhbuf;
}


char *
parse_char_class(char *s)
{
	int c, d;

	if (*s == '^')
		s++;
	if (*s == ']')
		s++;
	for (; *s != ']' && *s != '\n'; s++)
		if (*s == '[' && ((d = *(s+1)) == '.' || d == ':' || d == '='))
			for (s++, c = *++s; *s != ']' || c != d; s++)
				if ((c = *s) == '\n')
					return NULL;
	return  (*s == ']') ? s : NULL;
}

/* undo.c: This file contains the undo routines for the ed line editor */
/*-
 * Copyright (c) 1993 Andrew Moore, Talke Studio.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
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


#define USIZE 100				/* undo stack size */
undo_t *ustack = NULL;			/* undo stack */
long usize = 0;				/* stack size variable */
long u_p = 0;				/* undo stack pointer */

undo_t *
push_undo_stack(int type, long from, long to)
{
	undo_t *t;

#if defined(sun) || defined(NO_REALLOC_NULL)
	if (ustack == NULL &&
	    (ustack = (undo_t *) port_malloc((usize = USIZE) * sizeof(undo_t))) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "out of memory";
		return NULL;
	}
#endif
	t = ustack;
	if (u_p < usize ||
	    (t = (undo_t *) realloc(ustack, (usize += USIZE) * sizeof(undo_t))) != NULL) {
		ustack = t;
		ustack[u_p].type = type;
		ustack[u_p].t = get_addressed_line_node(to);
		ustack[u_p].h = get_addressed_line_node(from);
		return ustack + u_p++;
	}
	/* out of memory - release undo stack */
	fprintf(stderr, "%s\n", strerror(errno));
	errmsg = "out of memory";
	clear_undo_stack();
	free(ustack);
	ustack = NULL;
	usize = 0;
	return NULL;
}


#define USWAP(x,y) { \
	undo_t utmp; \
	utmp = x, x = y, y = utmp; \
}


/* u_current_addr and u_addr_last declared with globals */

int
pop_undo_stack(void)
{
	long n;
	long o_current_addr = current_addr;
	long o_addr_last = addr_last;

	if (u_current_addr == -1 || u_addr_last == -1) {
		errmsg = "nothing to undo";
		return ERR;
	} else if (u_p)
		modified = 1;
	get_addressed_line_node(0);	/* this get_addressed_line_node last! */
	SPL1();
	for (n = u_p; n-- > 0;) {
		switch(ustack[n].type) {
		case UADD:
			REQUE(ustack[n].h->q_back, ustack[n].t->q_forw);
			break;
		case UDEL:
			REQUE(ustack[n].h->q_back, ustack[n].h);
			REQUE(ustack[n].t, ustack[n].t->q_forw);
			break;
		case UMOV:
		case VMOV:
			REQUE(ustack[n - 1].h, ustack[n].h->q_forw);
			REQUE(ustack[n].t->q_back, ustack[n - 1].t);
			REQUE(ustack[n].h, ustack[n].t);
			n--;
			break;
		default:
			/*NOTREACHED*/
			;
		}
		ustack[n].type ^= 1;
	}
	/* reverse undo stack order */
	for (n = u_p; n-- > (u_p + 1)/ 2;)
		USWAP(ustack[n], ustack[u_p - 1 - n]);
	if (isglobal)
		clear_active_list();
	current_addr = u_current_addr, u_current_addr = o_current_addr;
	addr_last = u_addr_last, u_addr_last = o_addr_last;
	SPL0();
	return 0;
}


void
clear_undo_stack(void)
{
	line_t *lp, *ep, *tl;

	while (u_p--)
		if (ustack[u_p].type == UDEL) {
			ep = ustack[u_p].t->q_forw;
			for (lp = ustack[u_p].h; lp != ep; lp = tl) {
				unmark_line_node(lp);
				tl = lp->q_forw;
				free(lp);
			}
		}
	u_p = 0;
	u_current_addr = current_addr;
	u_addr_last = addr_last;
}

/* glob.c: This file contains the global command routines for the ed line
   editor */
/*-
 * Copyright (c) 1993 Andrew Moore, Talke Studio.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
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


int
build_active_list(int isgcmd)
{
	pattern_t *pat;
	line_t *lp;
	long n;
	char *s;
	char delimiter;

	if ((delimiter = *ibufp) == ' ' || delimiter == '\n') {
		errmsg = "invalid pattern delimiter";
		return ERR;
	} else if ((pat = get_compiled_pattern()) == NULL)
		return ERR;
	else if (*ibufp == delimiter)
		ibufp++;
	clear_active_list();
	lp = get_addressed_line_node(first_addr);
	for (n = first_addr; n <= second_addr; n++, lp = lp->q_forw) {
		if ((s = get_sbuf_line(lp)) == NULL)
			return ERR;
		if (isbinary)
			NUL_TO_NEWLINE(s, lp->len);
		if (!(regexec(pat, s, 0, NULL, 0) == isgcmd) &&
		    set_active_node(lp) >= 0)
			return ERR;
	}
	return 0;
}


long
exec_global(int interact, int gflag)
{
	static char *ocmd = NULL;
	static int ocmdsz = 0;

	line_t *lp = NULL;
	int status;
	int n;
	char *cmd = NULL;

#ifdef BACKWARDS
	if (!interact)
		if (!strcmp(ibufp, "\n"))
			cmd = "p\n";		/* null cmd-list == `p' */
		else if ((cmd = get_extended_line(&n, 0)) == NULL)
			return ERR;
#else
	if (!interact && (cmd = get_extended_line(&n, 0)) == NULL)
		return ERR;
#endif
	clear_undo_stack();
	while ((lp = next_active_node()) != NULL) {
		if ((current_addr = get_line_node_addr(lp)) < 0)
			return ERR;
		if (interact) {
			/* print current_addr; get a command in global syntax */
			if (display_lines(current_addr, current_addr, gflag) < 0)
				return ERR;
			while ((n = get_tty_line()) > 0 &&
			    ibuf[n - 1] != '\n')
				clearerr(stdin);
			if (n < 0)
				return ERR;
			else if (n == 0) {
				errmsg = "unexpected end-of-file";
				return ERR;
			} else if (n == 1 && !strcmp(ibuf, "\n"))
				continue;
			else if (n == 2 && !strcmp(ibuf, "&\n")) {
				if (cmd == NULL) {
					errmsg = "no previous command";
					return ERR;
				} else cmd = ocmd;
			} else if ((cmd = get_extended_line(&n, 0)) == NULL)
				return ERR;
			else {
				REALLOC(ocmd, ocmdsz, n + 1, ERR);
				memcpy(ocmd, cmd, n + 1);
				cmd = ocmd;
			}

		}
		ibufp = cmd;
		for (; *ibufp;)
			if ((status = extract_addr_range()) < 0 ||
			    (status = exec_command()) >= 0 ||
			    (status > 0 && (status = display_lines(
			    current_addr, current_addr, status)) < 0))
				return status;
	}
	return 0;
}


line_t **active_list;	/* list of lines active in a global command */
long active_last;	/* index of last active line in active_list */
long active_size;	/* size of active_list */
long active_ptr;		/* active_list index (non-decreasing) */
long active_ndx;		/* active_list index (modulo active_last) */

int
set_active_node(line_t *lp)
{
	if (active_last + 1 > active_size) {
		size_t ti = active_size;
		line_t **ts;
		SPL1();
#if defined(sun) || defined(NO_REALLOC_NULL)
		if (active_list != NULL) {
#endif
			if ((ts = (line_t **) realloc(active_list,
			    (ti += MINBUFSZ) * sizeof(line_t *))) == NULL) {
				fprintf(stderr, "%s\n", strerror(errno));
				errmsg = "out of memory";
				SPL0();
				return ERR;
			}
#if defined(sun) || defined(NO_REALLOC_NULL)
		} else {
			if ((ts = (line_t **) port_malloc((ti += MINBUFSZ) *
			    sizeof(line_t **))) == NULL) {
				fprintf(stderr, "%s\n", strerror(errno));
				errmsg = "out of memory";
				SPL0();
				return ERR;
			}
		}
#endif
		active_size = ti;
		active_list = ts;
		SPL0();
	}
	active_list[active_last++] = lp;
	return 0;
}


void
unset_active_nodes(line_t *np, line_t *mp)
{
	line_t *lp;
	long i;

	for (lp = np; lp != mp; lp = lp->q_forw)
		for (i = 0; i < active_last; i++)
			if (active_list[active_ndx] == lp) {
				active_list[active_ndx] = NULL;
				active_ndx = INC_MOD(active_ndx, active_last - 1);
				break;
			} else	active_ndx = INC_MOD(active_ndx, active_last - 1);
}


line_t *
next_active_node(void)
{
	while (active_ptr < active_last && active_list[active_ptr] == NULL)
		active_ptr++;
	return (active_ptr < active_last) ? active_list[active_ptr++] : NULL;
}


void
clear_active_list(void)
{
	SPL1();
	active_size = active_last = active_ptr = active_ndx = 0;
	free(active_list);
	active_list = NULL;
	SPL0();
}

/* buf.c: This file contains the scratch-file buffer routines for the
   ed line editor. */
/*-
 * Copyright (c) 1993 Andrew Moore, Talke Studio.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
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


FILE *sfp;			/* scratch file pointer */
off_t sfseek;			/* scratch file position */
int seek_write;			/* seek before writing */
line_t buffer_head;		/* incore buffer */

char *
get_sbuf_line(line_t *lp)
{
	static char *sfbuf = NULL;	/* buffer */
	static size_t sfbufsz;		/* buffer size */

	size_t len;

	if (lp == &buffer_head)
		return NULL;
	seek_write = 1;				/* force seek on write */
	/* out of position */
	if (sfseek != lp->seek) {
		sfseek = lp->seek;
		if (fseeko(sfp, sfseek, SEEK_SET) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			errmsg = "cannot seek temp file";
			return NULL;
		}
	}
	len = lp->len;
	REALLOC(sfbuf, sfbufsz, len + 1, NULL);
	if (fread(sfbuf, sizeof(char), len, sfp) != len) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "cannot read temp file";
		return NULL;
	}
	sfseek += len;				/* update file position */
	sfbuf[len] = '\0';
	return sfbuf;
}


const char *
put_sbuf_line(const char *cs)
{
	line_t *lp;
	size_t len;
	const char *s;

	if ((lp = (line_t *) port_malloc(sizeof(line_t))) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "out of memory";
		return NULL;
	}
	/* assert: cs is '\n' terminated */
	for (s = cs; *s != '\n'; s++)
		;
	if (s - cs >= LINECHARS) {
		errmsg = "line too long";
		free(lp);
		return NULL;
	}
	len = s - cs;
	/* out of position */
	if (seek_write) {
		if (fseeko(sfp, (off_t)0, SEEK_END) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			errmsg = "cannot seek temp file";
			free(lp);
			return NULL;
		}
		sfseek = ftello(sfp);
		seek_write = 0;
	}
	/* assert: SPL1() */
	if (fwrite(cs, sizeof(char), len, sfp) != len) {
		sfseek = -1;
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "cannot write temp file";
		free(lp);
		return NULL;
	}
	lp->len = len;
	lp->seek  = sfseek;
	add_line_node(lp);
	sfseek += len;			/* update file position */
	return ++s;
}


void
add_line_node(line_t *lp)
{
	line_t *cp;

	cp = get_addressed_line_node(current_addr);				/* this get_addressed_line_node last! */
	INSQUE(lp, cp);
	addr_last++;
	current_addr++;
}


long
get_line_node_addr(line_t *lp)
{
	line_t *cp = &buffer_head;
	long n = 0;

	while (cp != lp && (cp = cp->q_forw) != &buffer_head)
		n++;
	if (n && cp == &buffer_head) {
		errmsg = "invalid address";
		return ERR;
	 }
	 return n;
}


line_t *
get_addressed_line_node(long n)
{
	static line_t *lp = &buffer_head;
	static long on = 0;

	SPL1();
	if (n > on)
		if (n <= (on + addr_last) >> 1)
			for (; on < n; on++)
				lp = lp->q_forw;
		else {
			lp = buffer_head.q_back;
			for (on = addr_last; on > n; on--)
				lp = lp->q_back;
		}
	else
		if (n >= on >> 1)
			for (; on > n; on--)
				lp = lp->q_back;
		else {
			lp = &buffer_head;
			for (on = 0; on < n; on++)
				lp = lp->q_forw;
		}
	SPL0();
	return lp;
}

char sfn[15] = "";			/* scratch file name */

int
open_sbuf(void)
{
	int fd;
	int u;

	isbinary = newline_added = 0;
	u = umask(077);
	strcpy(sfn, "/tmp/ed.XXXXXX");
	if ((fd = mkstemp(sfn)) == -1 ||
	    (sfp = fdopen(fd, "w+")) == NULL) {
		if (fd != -1)
			close(fd);
		perror(sfn);
		errmsg = "cannot open temp file";
		umask(u);
		return ERR;
	}
	umask(u);
	return 0;
}


int
close_sbuf(void)
{
	if (sfp) {
		if (fclose(sfp) < 0) {
			fprintf(stderr, "%s: %s\n", sfn, strerror(errno));
			errmsg = "cannot close temp file";
			return ERR;
		}
		sfp = NULL;
		unlink(sfn);
	}
	sfseek = seek_write = 0;
	return 0;
}


void
quit(int n)
{
	if (sfp) {
		fclose(sfp);
		unlink(sfn);
	}
	port_exit(n);
}


unsigned char ctab[256];		/* character translation table */

void
init_buffers(void)
{
	int i = 0;

	/* Read stdin one character at a time to avoid i/o contention
	   with shell escapes invoked by nonterminal input, e.g.,
	   ed - <<EOF
	   !cat
	   hello, world
	   EOF */
	setbuffer(stdin, stdinbuf_arr, 1);

	/* Ensure stdout is line buffered. This avoids bogus delays
	   of output if stdout is piped through utilities to a terminal. */
	setvbuf(stdout, NULL, _IOLBF, 0);
	if (open_sbuf() < 0)
		quit(2);
	REQUE(&buffer_head, &buffer_head);
	for (i = 0; i < 256; i++)
		ctab[i] = i;
}


char *
translit_text(char *s, int len, int from, int to)
{
	static int i = 0;

	unsigned char *us;

	ctab[i] = i;			/* restore table to initial state */
	ctab[i = from] = to;
	for (us = (unsigned char *) s; len-- > 0; us++)
		*us = ctab[*us];
	return s;
}

/* ----- sub.c ----- */
static char *rhbuf;		/* rhs substitution buffer */
static int rhbufsz;		/* rhs substitution buffer size */
static int rhbufi;		/* rhs substitution buffer index */
static char *rbuf;		/* substitute_matching_text buffer */
static int rbufsz;		/* substitute_matching_text buffer size */
/* extract_subst_tail: extract substitution tail from the command buffer */
int
extract_subst_tail(int *flagp, long *np)
{
	char delimiter;

	*flagp = *np = 0;
	if ((delimiter = *ibufp) == '\n') {
		rhbufi = 0;
		*flagp = GPR;
		return 0;
	} else if (extract_subst_template() == nullptr)
		return  ERR;
	else if (*ibufp == '\n') {
		*flagp = GPR;
		return 0;
	} else if (*ibufp == delimiter)
		ibufp++;
	if ('1' <= *ibufp && *ibufp <= '9') {
		STRTOL(*np, ibufp);
		return 0;
	} else if (*ibufp == 'g') {
		ibufp++;
		*flagp = GSG;
		return 0;
	}
	return 0;
}

/* extract_subst_template: return pointer to copy of substitution template
   in the command buffer */
char *
extract_subst_template(void)
{
	int n = 0;
	int i = 0;
	char c;
	char delimiter = *ibufp++;

	if (*ibufp == '%' && *(ibufp + 1) == delimiter) {
		ibufp++;
		if (!rhbuf)
			errmsg = "no previous substitution";
		return rhbuf;
	}
	while (*ibufp != delimiter) {
		REALLOC(rhbuf, rhbufsz, i + 2, nullptr);
		if ((c = rhbuf[i++] = *ibufp++) == '\n' && *ibufp == '\0') {
			i--, ibufp--;
			break;
		} else if (c != '\\')
			;
		else if ((rhbuf[i++] = *ibufp++) != '\n')
			;
		else if (!isglobal) {
			while ((n = get_tty_line()) == 0 ||
			    (n > 0 && ibuf[n - 1] != '\n'))
				clearerr(stdin);
			if (n < 0)
				return nullptr;
		}
	}
	REALLOC(rhbuf, rhbufsz, i + 1, nullptr);
	rhbuf[rhbufi = i] = '\0';
	return  rhbuf;
}
/* search_and_replace: for each line in a range, change text matching a pattern
   according to a substitution template; return status  */
int
search_and_replace(pattern_t *pat, int gflag, int kth)
{
	undo_t *up;
	const char *txt;
	const char *eot;
	long lc;
	long xa = current_addr;
	int nsubs = 0;
	line_t *lp;
	int len;

	current_addr = first_addr - 1;
	for (lc = 0; lc <= second_addr - first_addr; lc++) {
		lp = get_addressed_line_node(++current_addr);
		if ((len = substitute_matching_text(pat, lp, gflag, kth)) < 0)
			return ERR;
		else if (len) {
			up = nullptr;
			if (delete_lines(current_addr, current_addr) < 0)
				return ERR;
			txt = rbuf;
			eot = rbuf + len;
			SPL1();
			do {
				if ((txt = put_sbuf_line(txt)) == nullptr) {
					SPL0();
					return ERR;
				} else if (up)
					up->t = get_addressed_line_node(current_addr);
				else if ((up = push_undo_stack(UADD,
				    current_addr, current_addr)) == nullptr) {
					SPL0();
					return ERR;
				}
			} while (txt != eot);
			SPL0();
			nsubs++;
			xa = current_addr;
		}
	}
	current_addr = xa;
	if  (nsubs == 0 && !(gflag & GLB)) {
		errmsg = "no match";
		return ERR;
	} else if ((gflag & (GPR | GLS | GNP)) &&
	    display_lines(current_addr, current_addr, gflag) < 0)
		return ERR;
	return 0;
}
/* substitute_matching_text: replace text matched by a pattern according to
   a substitution template; return pointer to the modified text */
int
substitute_matching_text(pattern_t *pat, line_t *lp, int gflag, int kth)
{
	int off = 0;
	int changed = 0;
	int matchno = 0;
	int i = 0;
	regmatch_t rm[SE_MAX];
	char *txt;
	char *eot;

	if ((txt = get_sbuf_line(lp)) == nullptr)
		return ERR;
	if (isbinary)
		NUL_TO_NEWLINE(txt, lp->len);
	eot = txt + lp->len;
	if (!regexec(pat, txt, SE_MAX, rm, 0)) {
		do {
			if (!kth || kth == ++matchno) {
				changed++;
				i = rm[0].rm_so;
				REALLOC(rbuf, rbufsz, off + i, ERR);
				if (isbinary)
					NEWLINE_TO_NUL(txt, rm[0].rm_eo);
				memcpy(rbuf + off, txt, i);
				off += i;
				if ((off = apply_subst_template(txt, rm, off,
				    pat->re_nsub)) < 0)
					return ERR;
			} else {
				i = rm[0].rm_eo;
				REALLOC(rbuf, rbufsz, off + i, ERR);
				if (isbinary)
					NEWLINE_TO_NUL(txt, i);
				memcpy(rbuf + off, txt, i);
				off += i;
			}
			txt += rm[0].rm_eo;
		} while (*txt &&
                        (!changed || ((gflag & GSG) && rm[0].rm_eo)) &&
		        !regexec(pat, txt, SE_MAX, rm, REG_NOTBOL));
		i = eot - txt;
		REALLOC(rbuf, rbufsz, off + i + 2, ERR);
		if (i > 0 && !rm[0].rm_eo && (gflag & GSG)) {
			errmsg = "infinite substitution loop";
			return  ERR;
		}
		if (isbinary)
			NEWLINE_TO_NUL(txt, i);
		memcpy(rbuf + off, txt, i);
		memcpy(rbuf + off + i, "\n", 2);
	}
	return changed ? off + i + 1 : 0;
}
/* apply_subst_template: modify text according to a substitution template;
   return offset to end of modified text */
int
apply_subst_template(const char *boln, regmatch_t *rm, int off, int re_nsub)
{
	int j = 0;
	int k = 0;
	int n;
	char *sub = rhbuf;

	for (; sub - rhbuf < rhbufi; sub++)
		if (*sub == '&') {
			j = rm[0].rm_so;
			k = rm[0].rm_eo;
			REALLOC(rbuf, rbufsz, off + k - j, ERR);
			while (j < k)
				rbuf[off++] = boln[j++];
		} else if (*sub == '\\' && '1' <= *++sub && *sub <= '9' &&
		    (n = *sub - '0') <= re_nsub) {
			j = rm[n].rm_so;
			k = rm[n].rm_eo;
			REALLOC(rbuf, rbufsz, off + k - j, ERR);
			while (j < k)
				rbuf[off++] = boln[j++];
		} else {
			REALLOC(rbuf, rbufsz, off + 1, ERR);
			rbuf[off++] = *sub;
		}
	REALLOC(rbuf, rbufsz, off + 1, ERR);
	rbuf[off] = '\0';
	return off;
}
/* ----- io.c ----- */
static char *sbuf;		/* file i/o buffer */
static int sbufsz;		/* file i/o buffer size */
#define ESCAPES "\a\b\f\n\r\t\v\\"
#define ESCCHARS "abfnrtv\\"
long
read_file(char *fn, long n)
{
	FILE *fp;
	long size;
	int cs;

	fp = (*fn == '!') ? popen(fn + 1, "r") : fopen(strip_escapes(fn), "r");
	if (fp == nullptr) {
		fprintf(stderr, "%s: %s\n", fn, strerror(errno));
		errmsg = "cannot open input file";
		return ERR;
	}
	if ((size = read_stream(fp, n)) < 0) {
		fprintf(stderr, "%s: %s\n", fn, strerror(errno));
		errmsg = "error reading input file";
	}
	if ((cs = (*fn == '!') ?  pclose(fp) : fclose(fp)) < 0) {
		fprintf(stderr, "%s: %s\n", fn, strerror(errno));
		errmsg = "cannot close input file";
	}
	if (size < 0 || cs < 0)
		return ERR;
	if (!scripted)
		fprintf(stdout, "%lu\n", size);
	return current_addr - n;
}
long
read_stream(FILE *fp, long n)
{
	line_t *lp = get_addressed_line_node(n);
	undo_t *up = nullptr;
	unsigned long size = 0;
	int o_newline_added = newline_added;
	int o_isbinary = isbinary;
	int appended = (n == addr_last);
	int len;

	isbinary = newline_added = 0;
	for (current_addr = n; (len = get_stream_line(fp)) > 0; size += len) {
		SPL1();
		if (put_sbuf_line(sbuf) == nullptr) {
			SPL0();
			return ERR;
		}
		lp = lp->q_forw;
		if (up)
			up->t = lp;
		else if ((up = push_undo_stack(UADD, current_addr,
		    current_addr)) == nullptr) {
			SPL0();
			return ERR;
		}
		SPL0();
	}
	if (len < 0)
		return ERR;
	if (appended && size && o_isbinary && o_newline_added)
		fputs("newline inserted\n", stderr);
	else if (newline_added && (!appended || (!isbinary && !o_isbinary)))
		fputs("newline appended\n", stderr);
	if (isbinary && newline_added && !appended)
	    	size += 1;
	if (!size)
		newline_added = 1;
	newline_added = appended ? newline_added : o_newline_added;
	isbinary = isbinary | o_isbinary;
	return size;
}
int
get_stream_line(FILE *fp)
{
	int c;
	int i = 0;

	while (((c = getc(fp)) != EOF || (!feof(fp) && !ferror(fp))) &&
	    c != '\n') {
		REALLOC(sbuf, sbufsz, i + 1, ERR);
		if (!(sbuf[i++] = c))
			isbinary = 1;
	}
	REALLOC(sbuf, sbufsz, i + 2, ERR);
	if (c == '\n')
		sbuf[i++] = c;
	else if (ferror(fp)) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "cannot read input file";
		return ERR;
	} else if (i) {
		sbuf[i++] = '\n';
		newline_added = 1;
	}
	sbuf[i] = '\0';
	return (isbinary && newline_added && i) ? --i : i;
}
long
write_file(char *fn, const char *mode, long n, long m)
{
	FILE *fp;
	long size;
	int cs;

	fp = (*fn == '!') ? popen(fn+1, "w") : fopen(strip_escapes(fn), mode);
	if (fp == nullptr) {
		fprintf(stderr, "%s: %s\n", fn, strerror(errno));
		errmsg = "cannot open output file";
		return ERR;
	}
	if ((size = write_stream(fp, n, m)) < 0) {
		fprintf(stderr, "%s: %s\n", fn, strerror(errno));
		errmsg = "error writing output file";
	}
	if ((cs = (*fn == '!') ?  pclose(fp) : fclose(fp)) < 0) {
		fprintf(stderr, "%s: %s\n", fn, strerror(errno));
		errmsg = "cannot close output file";
	}
	if (size < 0 || cs < 0)
		return ERR;
	if (!scripted)
		fprintf(stdout, "%lu\n", size);
	return n ? m - n + 1 : 0;
}
long
write_stream(FILE *fp, long n, long m)
{
	line_t *lp = get_addressed_line_node(n);
	unsigned long size = 0;
	char *s;
	int len;

	for (; n && n <= m; n++, lp = lp->q_forw) {
		if ((s = get_sbuf_line(lp)) == nullptr)
			return ERR;
		len = lp->len;
		if (n != addr_last || !isbinary || !newline_added)
			s[len++] = '\n';
		if (put_stream_line(fp, s, len) < 0)
			return ERR;
		size += len;
	}
	return size;
}
int
put_stream_line(FILE *fp, const char *s, int len)
{
	while (len--)
		if (fputc(*s++, fp) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			errmsg = "cannot write file";
			return ERR;
		}
	return 0;
}
char *
get_extended_line(int *sizep, int nonl)
{
	static char *cvbuf = nullptr;		/* buffer */
	static int cvbufsz = 0;			/* buffer size */

	int l, n;
	char *t = ibufp;

	while (*t++ != '\n')
		;
	if ((l = t - ibufp) < 2 || !has_trailing_escape(ibufp, ibufp + l - 1)) {
		*sizep = l;
		return ibufp;
	}
	*sizep = -1;
	REALLOC(cvbuf, cvbufsz, l, nullptr);
	memcpy(cvbuf, ibufp, l);
	*(cvbuf + --l - 1) = '\n'; 	/* strip trailing esc */
	if (nonl) l--; 			/* strip newline */
	for (;;) {
		if ((n = get_tty_line()) < 0)
			return nullptr;
		else if (n == 0 || ibuf[n - 1] != '\n') {
			errmsg = "unexpected end-of-file";
			return nullptr;
		}
		REALLOC(cvbuf, cvbufsz, l + n, nullptr);
		memcpy(cvbuf + l, ibuf, n);
		l += n;
		if (n < 2 || !has_trailing_escape(cvbuf, cvbuf + l - 1))
			break;
		*(cvbuf + --l - 1) = '\n'; 	/* strip trailing esc */
		if (nonl) l--; 			/* strip newline */
	}
	REALLOC(cvbuf, cvbufsz, l + 1, nullptr);
	cvbuf[l] = '\0';
	*sizep = l;
	return cvbuf;
}
int
get_tty_line(void)
{
	int oi = 0;
	int i = 0;
	int c;

	for (;;)
		switch (c = getchar()) {
		default:
			oi = 0;
			REALLOC(ibuf, ibufsz, i + 2, ERR);
			if (!(ibuf[i++] = c)) isbinary = 1;
			if (c != '\n')
				continue;
			lineno++;
			ibuf[i] = '\0';
			ibufp = ibuf;
			return i;
		case EOF:
			if (ferror(stdin)) {
				fprintf(stderr, "stdin: %s\n", strerror(errno));
				errmsg = "cannot read stdin";
				clearerr(stdin);
				ibufp = nullptr;
				return ERR;
			} else {
				clearerr(stdin);
				if (i != oi) {
					oi = i;
					continue;
				} else if (i)
					ibuf[i] = '\0';
				ibufp = ibuf;
				return i;
			}
		}
}
int
put_tty_line(const char *s, int l, long n, int gflag)
{
	int col = 0;
	int lc = 0;
	char *cp;

	if (gflag & GNP) {
		printf("%ld\t", n);
		col = 8;
	}
	for (; l--; s++) {
		if ((gflag & GLS) && ++col > cols) {
			fputs("\\\n", stdout);
			col = 1;
#ifndef BACKWARDS
			if (!scripted && !isglobal && ++lc > rows) {
				lc = 0;
				fputs("Press <RETURN> to continue... ", stdout);
				fflush(stdout);
				if (get_tty_line() < 0)
					return ERR;
			}
#endif
		}
		if (gflag & GLS) {
			if (31 < *s && *s < 127 && *s != '\\')
				putchar(*s);
			else {
				putchar('\\');
				col++;
				if (*s && (cp = strchr((char *)ESCAPES, *s)) != nullptr)
					putchar(ESCCHARS[cp - ESCAPES]);
				else {
					putchar((((unsigned char) *s & 0300) >> 6) + '0');
					putchar((((unsigned char) *s & 070) >> 3) + '0');
					putchar(((unsigned char) *s & 07) + '0');
					col += 2;
				}
			}

		} else
			putchar(*s);
	}
#ifndef BACKWARDS
	if (gflag & GLS)
		putchar('$');
#endif
	putchar('\n');
	return 0;
}
/* ----- main.c ----- */
#define SKIP_BLANKS() while (isspace((unsigned char)*ibufp) && *ibufp != '\n') ibufp++

#define MUST_BE_FIRST() do {					\
	if (!first) {						\
		errmsg = "invalid address";			\
		return ERR;					\
	}							\
} while (0)


#ifdef BACKWARDS
/* GET_THIRD_ADDR: get a legal address from the command buffer */
#define GET_THIRD_ADDR(addr) \
{ \
	long ol1, ol2; \
\
	ol1 = first_addr, ol2 = second_addr; \
	if (extract_addr_range() < 0) \
		return ERR; \
	else if (addr_cnt == 0) { \
		errmsg = "destination expected"; \
		return ERR; \
	} else if (second_addr < 0 || addr_last < second_addr) { \
		errmsg = "invalid address"; \
		return ERR; \
	} \
	addr = second_addr; \
	first_addr = ol1, second_addr = ol2; \
}
#else	/* BACKWARDS */
/* GET_THIRD_ADDR: get a legal address from the command buffer */
#define GET_THIRD_ADDR(addr) \
{ \
	long ol1, ol2; \
\
	ol1 = first_addr, ol2 = second_addr; \
	if (extract_addr_range() < 0) \
		return ERR; \
	if (second_addr < 0 || addr_last < second_addr) { \
		errmsg = "invalid address"; \
		return ERR; \
	} \
	addr = second_addr; \
	first_addr = ol1, second_addr = ol2; \
}
#endif


/* GET_COMMAND_SUFFIX: verify the command suffix in the command buffer */
#define GET_COMMAND_SUFFIX() { \
	int done = 0; \
	do { \
		switch(*ibufp) { \
		case 'p': \
			gflag |= GPR, ibufp++; \
			break; \
		case 'l': \
			gflag |= GLS, ibufp++; \
			break; \
		case 'n': \
			gflag |= GNP, ibufp++; \
			break; \
		default: \
			done++; \
		} \
	} while (!done); \
	if (*ibufp++ != '\n') { \
		errmsg = "invalid command suffix"; \
		return ERR; \
	} \
}


/* sflags */
#define SGG 001		/* complement previous global substitute suffix */
#define SGP 002		/* complement previous print suffix */
#define SGR 004		/* use last regex instead of last pat */
#define SGF 010		/* repeat last substitution */
#define MAXMARK 26			/* max number of marks */

static line_t *mark[MAXMARK];		/* line markers */
static int markno;			/* line marker count */

void clear_marks(void)
{
	std::memset(mark, 0, sizeof(mark));
	markno = 0;
}

/* extract_addr_range: get line addresses from the command buffer until an
   illegal address is seen; return status */
int
extract_addr_range(void)
{
	long addr;

	addr_cnt = 0;
	first_addr = second_addr = current_addr;
	while ((addr = next_addr()) >= 0) {
		addr_cnt++;
		first_addr = second_addr;
		second_addr = addr;
		if (*ibufp != ',' && *ibufp != ';')
			break;
		else if (*ibufp++ == ';')
			current_addr = addr;
	}
	if ((addr_cnt = min(addr_cnt, 2)) == 1 || second_addr != addr)
		first_addr = second_addr;
	return (addr == ERR) ? ERR : 0;
}
/*  next_addr: return the next line address in the command buffer */
long
next_addr(void)
{
	const char *hd;
	long addr = current_addr;
	long n;
	int first = 1;
	int c;

	SKIP_BLANKS();
	for (hd = ibufp;; first = 0)
		switch (c = *ibufp) {
		case '+':
		case '\t':
		case ' ':
		case '-':
		case '^':
			ibufp++;
			SKIP_BLANKS();
			if (isdigit((unsigned char)*ibufp)) {
				STRTOL(n, ibufp);
				addr += (c == '-' || c == '^') ? -n : n;
			} else if (!isspace((unsigned char)c))
				addr += (c == '-' || c == '^') ? -1 : 1;
			break;
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			MUST_BE_FIRST();
			STRTOL(addr, ibufp);
			break;
		case '.':
		case '$':
			MUST_BE_FIRST();
			ibufp++;
			addr = (c == '.') ? current_addr : addr_last;
			break;
		case '/':
		case '?':
			MUST_BE_FIRST();
			if ((addr = get_matching_node_addr(
			    get_compiled_pattern(), c == '/')) < 0)
				return ERR;
			else if (c == *ibufp)
				ibufp++;
			break;
		case '\'':
			MUST_BE_FIRST();
			ibufp++;
			if ((addr = get_marked_node_addr(*ibufp++)) < 0)
				return ERR;
			break;
		case '%':
		case ',':
		case ';':
			if (first) {
				ibufp++;
				addr_cnt++;
				second_addr = (c == ';') ? current_addr : 1;
				if ((addr = next_addr()) < 0)
					addr = addr_last;
				break;
			}
			/* FALLTHROUGH */
		default:
			if (ibufp == hd)
				return EOF;
			else if (addr < 0 || addr_last < addr) {
				errmsg = "invalid address";
				return ERR;
			} else
				return addr;
		}
	/* NOTREACHED */
}
int
exec_command(void)
{
	static pattern_t *pat = nullptr;
	static int sgflag = 0;
	static long sgnum = 0;

	pattern_t *tpat;
	char *fnp;
	int gflag = 0;
	int sflags = 0;
	long addr = 0;
	int n = 0;
	int c;

	SKIP_BLANKS();
	switch(c = *ibufp++) {
	case 'a':
		GET_COMMAND_SUFFIX();
		if (!isglobal) clear_undo_stack();
		if (append_lines(second_addr) < 0)
			return ERR;
		break;
	case 'c':
		if (check_addr_range(current_addr, current_addr) < 0)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (!isglobal) clear_undo_stack();
		if (delete_lines(first_addr, second_addr) < 0 ||
		    append_lines(current_addr) < 0)
			return ERR;
		break;
	case 'd':
		if (check_addr_range(current_addr, current_addr) < 0)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (!isglobal) clear_undo_stack();
		if (delete_lines(first_addr, second_addr) < 0)
			return ERR;
		else if ((addr = INC_MOD(current_addr, addr_last)) != 0)
			current_addr = addr;
		break;
	case 'e':
		if (modified && !scripted)
			return EMOD;
		/* FALLTHROUGH */
	case 'E':
		if (addr_cnt > 0) {
			errmsg = "unexpected address";
			return ERR;
		} else if (!isspace((unsigned char)*ibufp)) {
			errmsg = "unexpected command suffix";
			return ERR;
		} else if ((fnp = get_filename()) == nullptr)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (delete_lines(1, addr_last) < 0)
			return ERR;
		clear_undo_stack();
		if (close_sbuf() < 0)
			return ERR;
		else if (open_sbuf() < 0)
			return FATAL;
		if (*fnp && *fnp != '!')
			 strlcpy(old_filename, fnp, PATH_MAX);
#ifdef BACKWARDS
		if (*fnp == '\0' && *old_filename == '\0') {
			errmsg = "no current filename";
			return ERR;
		}
#endif
		if (read_file(*fnp ? fnp : old_filename, 0) < 0)
			return ERR;
		clear_undo_stack();
		modified = 0;
		u_current_addr = u_addr_last = -1;
		break;
	case 'f':
		if (addr_cnt > 0) {
			errmsg = "unexpected address";
			return ERR;
		} else if (!isspace((unsigned char)*ibufp)) {
			errmsg = "unexpected command suffix";
			return ERR;
		} else if ((fnp = get_filename()) == nullptr)
			return ERR;
		else if (*fnp == '!') {
			errmsg = "invalid redirection";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		if (*fnp)
			strlcpy(old_filename, fnp, PATH_MAX);
		printf("%s\n", strip_escapes(old_filename));
		break;
	case 'g':
	case 'v':
	case 'G':
	case 'V':
		if (isglobal) {
			errmsg = "cannot nest global commands";
			return ERR;
		} else if (check_addr_range(1, addr_last) < 0)
			return ERR;
		else if (build_active_list(c == 'g' || c == 'G') < 0)
			return ERR;
		else if ((n = (c == 'G' || c == 'V')))
			GET_COMMAND_SUFFIX();
		isglobal++;
		if (exec_global(n, gflag) < 0)
			return ERR;
		break;
	case 'h':
		if (addr_cnt > 0) {
			errmsg = "unexpected address";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		if (*errmsg) fprintf(stderr, "%s\n", errmsg);
		break;
	case 'H':
		if (addr_cnt > 0) {
			errmsg = "unexpected address";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		if ((garrulous = 1 - garrulous) && *errmsg)
			fprintf(stderr, "%s\n", errmsg);
		break;
	case 'i':
		if (second_addr == 0) {
			errmsg = "invalid address";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		if (!isglobal) clear_undo_stack();
		if (append_lines(second_addr - 1) < 0)
			return ERR;
		break;
	case 'j':
		if (check_addr_range(current_addr, current_addr + 1) < 0)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (!isglobal) clear_undo_stack();
		if (first_addr != second_addr &&
		    join_lines(first_addr, second_addr) < 0)
			return ERR;
		break;
	case 'k':
		c = *ibufp++;
		if (second_addr == 0) {
			errmsg = "invalid address";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		if (mark_line_node(get_addressed_line_node(second_addr), c) < 0)
			return ERR;
		break;
	case 'l':
		if (check_addr_range(current_addr, current_addr) < 0)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (display_lines(first_addr, second_addr, gflag | GLS) < 0)
			return ERR;
		gflag = 0;
		break;
	case 'm':
		if (check_addr_range(current_addr, current_addr) < 0)
			return ERR;
		GET_THIRD_ADDR(addr);
		if (first_addr <= addr && addr < second_addr) {
			errmsg = "invalid destination";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		if (!isglobal) clear_undo_stack();
		if (move_lines(addr) < 0)
			return ERR;
		break;
	case 'n':
		if (check_addr_range(current_addr, current_addr) < 0)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (display_lines(first_addr, second_addr, gflag | GNP) < 0)
			return ERR;
		gflag = 0;
		break;
	case 'p':
		if (check_addr_range(current_addr, current_addr) < 0)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (display_lines(first_addr, second_addr, gflag | GPR) < 0)
			return ERR;
		gflag = 0;
		break;
	case 'P':
		if (addr_cnt > 0) {
			errmsg = "unexpected address";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		prompt = prompt ? nullptr : optarg ? optarg : dps;
		break;
	case 'q':
	case 'Q':
		if (addr_cnt > 0) {
			errmsg = "unexpected address";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		gflag =  (modified && !scripted && c == 'q') ? EMOD : EOF;
		break;
	case 'r':
		if (!isspace((unsigned char)*ibufp)) {
			errmsg = "unexpected command suffix";
			return ERR;
		} else if (addr_cnt == 0)
			second_addr = addr_last;
		if ((fnp = get_filename()) == nullptr)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (!isglobal) clear_undo_stack();
		if (*old_filename == '\0' && *fnp != '!')
			strlcpy(old_filename, fnp, PATH_MAX);
#ifdef BACKWARDS
		if (*fnp == '\0' && *old_filename == '\0') {
			errmsg = "no current filename";
			return ERR;
		}
#endif
		if ((addr = read_file(*fnp ? fnp : old_filename, second_addr)) < 0)
			return ERR;
		else if (addr && addr != addr_last)
			modified = 1;
		break;
	case 's':
		do {
			switch(*ibufp) {
			case '\n':
				sflags |=SGF;
				break;
			case 'g':
				sflags |= SGG;
				ibufp++;
				break;
			case 'p':
				sflags |= SGP;
				ibufp++;
				break;
			case 'r':
				sflags |= SGR;
				ibufp++;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				STRTOL(sgnum, ibufp);
				sflags |= SGF;
				sgflag &= ~GSG;		/* override GSG */
				break;
			default:
				if (sflags) {
					errmsg = "invalid command suffix";
					return ERR;
				}
			}
		} while (sflags && *ibufp != '\n');
		if (sflags && !pat) {
			errmsg = "no previous substitution";
			return ERR;
		} else if (sflags & SGG)
			sgnum = 0;		/* override numeric arg */
		if (*ibufp != '\n' && *(ibufp + 1) == '\n') {
			errmsg = "invalid pattern delimiter";
			return ERR;
		}
		tpat = pat;
		SPL1();
		if ((!sflags || (sflags & SGR)) &&
		    (tpat = get_compiled_pattern()) == nullptr) {
		 	SPL0();
			return ERR;
		} else if (tpat != pat) {
			if (pat) {
				regfree(pat);
				free(pat);
			}
			pat = tpat;
			patlock = 1;		/* reserve pattern */
		}
		SPL0();
		if (!sflags && extract_subst_tail(&sgflag, &sgnum) < 0)
			return ERR;
		else if (isglobal)
			sgflag |= GLB;
		else
			sgflag &= ~GLB;
		if (sflags & SGG)
			sgflag ^= GSG;
		if (sflags & SGP)
			sgflag ^= GPR, sgflag &= ~(GLS | GNP);
		do {
			switch(*ibufp) {
			case 'p':
				sgflag |= GPR, ibufp++;
				break;
			case 'l':
				sgflag |= GLS, ibufp++;
				break;
			case 'n':
				sgflag |= GNP, ibufp++;
				break;
			default:
				n++;
			}
		} while (!n);
		if (check_addr_range(current_addr, current_addr) < 0)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (!isglobal) clear_undo_stack();
		if (search_and_replace(pat, sgflag, sgnum) < 0)
			return ERR;
		break;
	case 't':
		if (check_addr_range(current_addr, current_addr) < 0)
			return ERR;
		GET_THIRD_ADDR(addr);
		GET_COMMAND_SUFFIX();
		if (!isglobal) clear_undo_stack();
		if (copy_lines(addr) < 0)
			return ERR;
		break;
	case 'u':
		if (addr_cnt > 0) {
			errmsg = "unexpected address";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		if (pop_undo_stack() < 0)
			return ERR;
		break;
	case 'w':
	case 'W':
		if ((n = *ibufp) == 'q' || n == 'Q') {
			gflag = EOF;
			ibufp++;
		}
		if (!isspace((unsigned char)*ibufp)) {
			errmsg = "unexpected command suffix";
			return ERR;
		} else if ((fnp = get_filename()) == nullptr)
			return ERR;
		if (addr_cnt == 0 && !addr_last)
			first_addr = second_addr = 0;
		else if (check_addr_range(1, addr_last) < 0)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (*old_filename == '\0' && *fnp != '!')
			strlcpy(old_filename, fnp, PATH_MAX);
#ifdef BACKWARDS
		if (*fnp == '\0' && *old_filename == '\0') {
			errmsg = "no current filename";
			return ERR;
		}
#endif
		if ((addr = write_file(*fnp ? fnp : old_filename,
		    (c == 'W') ? "a" : "w", first_addr, second_addr)) < 0)
			return ERR;
		else if (addr == addr_last && *fnp != '!')
			modified = 0;
		else if (modified && !scripted && n == 'q')
			gflag = EMOD;
		break;
	case 'x':
		if (addr_cnt > 0) {
			errmsg = "unexpected address";
			return ERR;
		}
		GET_COMMAND_SUFFIX();
		errmsg = "crypt unavailable";
		return ERR;
	case 'z':
#ifdef BACKWARDS
		if (check_addr_range(first_addr = 1, current_addr + 1) < 0)
#else
		if (check_addr_range(first_addr = 1, current_addr + !isglobal) < 0)
#endif
			return ERR;
		else if ('0' < *ibufp && *ibufp <= '9')
			STRTOL(rows, ibufp);
		GET_COMMAND_SUFFIX();
		if (display_lines(second_addr, min(addr_last,
		    second_addr + rows), gflag) < 0)
			return ERR;
		gflag = 0;
		break;
	case '=':
		GET_COMMAND_SUFFIX();
		printf("%ld\n", addr_cnt ? second_addr : addr_last);
		break;
	case '!':
		if (addr_cnt > 0) {
			errmsg = "unexpected address";
			return ERR;
		} else if ((sflags = get_shell_command()) < 0)
			return ERR;
		GET_COMMAND_SUFFIX();
		if (sflags) printf("%s\n", shcmd + 1);
		system(shcmd + 1);
		if (!scripted) printf("!\n");
		break;
	case '\n':
#ifdef BACKWARDS
		if (check_addr_range(first_addr = 1, current_addr + 1) < 0
#else
		if (check_addr_range(first_addr = 1, current_addr + !isglobal) < 0
#endif
		 || display_lines(second_addr, second_addr, 0) < 0)
			return ERR;
		break;
	default:
		errmsg = "unknown command";
		return ERR;
	}
	return gflag;
}
int
check_addr_range(long n, long m)
{
	if (addr_cnt == 0) {
		first_addr = n;
		second_addr = m;
	}
	if (first_addr > second_addr || 1 > first_addr ||
	    second_addr > addr_last) {
		errmsg = "invalid address";
		return ERR;
	}
	return 0;
}
long
get_matching_node_addr(pattern_t *pat, int dir)
{
	char *s;
	long n = current_addr;
	line_t *lp;

	if (!pat) return ERR;
	do {
	       if ((n = dir ? INC_MOD(n, addr_last) : DEC_MOD(n, addr_last))) {
			lp = get_addressed_line_node(n);
			if ((s = get_sbuf_line(lp)) == nullptr)
				return ERR;
			if (isbinary)
				NUL_TO_NEWLINE(s, lp->len);
			if (!regexec(pat, s, 0, nullptr, 0))
				return n;
	       }
	} while (n != current_addr);
	errmsg = "no match";
	return  ERR;
}
char *
get_filename(void)
{
	static char *file = nullptr;
	static int filesz = 0;

	int n;

	if (*ibufp != '\n') {
		SKIP_BLANKS();
		if (*ibufp == '\n') {
			errmsg = "invalid filename";
			return nullptr;
		} else if ((ibufp = get_extended_line(&n, 1)) == nullptr)
			return nullptr;
		else if (*ibufp == '!') {
			ibufp++;
			if ((n = get_shell_command()) < 0)
				return nullptr;
			if (n)
				printf("%s\n", shcmd + 1);
			return shcmd;
		} else if (n > PATH_MAX - 1) {
			errmsg = "filename too long";
			return  nullptr;
		}
	}
#ifndef BACKWARDS
	else if (*old_filename == '\0') {
		errmsg = "no current filename";
		return  nullptr;
	}
#endif
	REALLOC(file, filesz, PATH_MAX, nullptr);
	for (n = 0; *ibufp != '\n';)
		file[n++] = *ibufp++;
	file[n] = '\0';
	return is_legal_filename(file) ? file : nullptr;
}
int
get_shell_command(void)
{
	static char *buf = nullptr;
	static int n = 0;

	char *s;			/* substitution char pointer */
	int i = 0;
	int j = 0;

	if (red) {
		errmsg = "shell access restricted";
		return ERR;
	} else if ((s = ibufp = get_extended_line(&j, 1)) == nullptr)
		return ERR;
	REALLOC(buf, n, j + 1, ERR);
	buf[i++] = '!';			/* prefix command w/ bang */
	while (*ibufp != '\n')
		switch (*ibufp) {
		default:
			REALLOC(buf, n, i + 2, ERR);
			buf[i++] = *ibufp;
			if (*ibufp++ == '\\')
				buf[i++] = *ibufp++;
			break;
		case '!':
			if (s != ibufp) {
				REALLOC(buf, n, i + 1, ERR);
				buf[i++] = *ibufp++;
			}
#ifdef BACKWARDS
			else if (shcmd == nullptr || *(shcmd + 1) == '\0')
#else
			else if (shcmd == nullptr)
#endif
			{
				errmsg = "no previous command";
				return ERR;
			} else {
				REALLOC(buf, n, i + shcmdi, ERR);
				for (s = shcmd + 1; s < shcmd + shcmdi;)
					buf[i++] = *s++;
				s = ibufp++;
			}
			break;
		case '%':
			if (*old_filename  == '\0') {
				errmsg = "no current filename";
				return ERR;
			}
			j = strlen(s = strip_escapes(old_filename));
			REALLOC(buf, n, i + j, ERR);
			while (j--)
				buf[i++] = *s++;
			s = ibufp++;
			break;
		}
	REALLOC(shcmd, shcmdsz, i + 1, ERR);
	memcpy(shcmd, buf, i);
	shcmd[shcmdi = i] = '\0';
	return *s == '!' || *s == '%';
}
int
append_lines(long n)
{
	int l;
	const char *lp = ibuf;
	const char *eot;
	undo_t *up = nullptr;

	for (current_addr = n;;) {
		if (!isglobal) {
			if ((l = get_tty_line()) < 0)
				return ERR;
			else if (l == 0 || ibuf[l - 1] != '\n') {
				clearerr(stdin);
				return  l ? EOF : 0;
			}
			lp = ibuf;
		} else if (*(lp = ibufp) == '\0')
			return 0;
		else {
			while (*ibufp++ != '\n')
				;
			l = ibufp - lp;
		}
		if (l == 2 && lp[0] == '.' && lp[1] == '\n') {
			return 0;
		}
		eot = lp + l;
		SPL1();
		do {
			if ((lp = put_sbuf_line(lp)) == nullptr) {
				SPL0();
				return ERR;
			} else if (up)
				up->t = get_addressed_line_node(current_addr);
			else if ((up = push_undo_stack(UADD, current_addr,
			    current_addr)) == nullptr) {
				SPL0();
				return ERR;
			}
		} while (lp != eot);
		modified = 1;
		SPL0();
	}
	/* NOTREACHED */
}
int
join_lines(long from, long to)
{
	static char *buf = nullptr;
	static int n;

	char *s;
	int size = 0;
	line_t *bp, *ep;

	ep = get_addressed_line_node(INC_MOD(to, addr_last));
	bp = get_addressed_line_node(from);
	for (; bp != ep; bp = bp->q_forw) {
		if ((s = get_sbuf_line(bp)) == nullptr)
			return ERR;
		REALLOC(buf, n, size + bp->len, ERR);
		memcpy(buf + size, s, bp->len);
		size += bp->len;
	}
	REALLOC(buf, n, size + 2, ERR);
	memcpy(buf + size, "\n", 2);
	if (delete_lines(from, to) < 0)
		return ERR;
	current_addr = from - 1;
	SPL1();
	if (put_sbuf_line(buf) == nullptr ||
	    push_undo_stack(UADD, current_addr, current_addr) == nullptr) {
		SPL0();
		return ERR;
	}
	modified = 1;
	SPL0();
	return 0;
}
int
move_lines(long addr)
{
	line_t *b1, *a1, *b2, *a2;
	long n = INC_MOD(second_addr, addr_last);
	long p = first_addr - 1;
	int done = (addr == first_addr - 1 || addr == second_addr);

	SPL1();
	if (done) {
		a2 = get_addressed_line_node(n);
		b2 = get_addressed_line_node(p);
		current_addr = second_addr;
	} else if (push_undo_stack(UMOV, p, n) == nullptr ||
	    push_undo_stack(UMOV, addr, INC_MOD(addr, addr_last)) == nullptr) {
		SPL0();
		return ERR;
	} else {
		a1 = get_addressed_line_node(n);
		if (addr < first_addr) {
			b1 = get_addressed_line_node(p);
			b2 = get_addressed_line_node(addr);
					/* this get_addressed_line_node last! */
		} else {
			b2 = get_addressed_line_node(addr);
			b1 = get_addressed_line_node(p);
					/* this get_addressed_line_node last! */
		}
		a2 = b2->q_forw;
		REQUE(b2, b1->q_forw);
		REQUE(a1->q_back, a2);
		REQUE(b1, a1);
		current_addr = addr + ((addr < first_addr) ?
		    second_addr - first_addr + 1 : 0);
	}
	if (isglobal)
		unset_active_nodes(b2->q_forw, a2);
	modified = 1;
	SPL0();
	return 0;
}
int
copy_lines(long addr)
{
	line_t *lp, *np = get_addressed_line_node(first_addr);
	undo_t *up = nullptr;
	long n = second_addr - first_addr + 1;
	long m = 0;

	current_addr = addr;
	if (first_addr <= addr && addr < second_addr) {
		n =  addr - first_addr + 1;
		m = second_addr - addr;
	}
	for (; n > 0; n=m, m=0, np = get_addressed_line_node(current_addr + 1))
		for (; n-- > 0; np = np->q_forw) {
			SPL1();
			if ((lp = dup_line_node(np)) == nullptr) {
				SPL0();
				return ERR;
			}
			add_line_node(lp);
			if (up)
				up->t = lp;
			else if ((up = push_undo_stack(UADD, current_addr,
			    current_addr)) == nullptr) {
				SPL0();
				return ERR;
			}
			modified = 1;
			SPL0();
		}
	return 0;
}
int
delete_lines(long from, long to)
{
	line_t *n, *p;

	SPL1();
	if (push_undo_stack(UDEL, from, to) == nullptr) {
		SPL0();
		return ERR;
	}
	n = get_addressed_line_node(INC_MOD(to, addr_last));
	p = get_addressed_line_node(from - 1);
					/* this get_addressed_line_node last! */
	if (isglobal)
		unset_active_nodes(p->q_forw, n);
	REQUE(p, n);
	addr_last -= to - from + 1;
	current_addr = from - 1;
	modified = 1;
	SPL0();
	return 0;
}
int
display_lines(long from, long to, int gflag)
{
	line_t *bp;
	line_t *ep;
	char *s;

	if (!from) {
		errmsg = "invalid address";
		return ERR;
	}
	ep = get_addressed_line_node(INC_MOD(to, addr_last));
	bp = get_addressed_line_node(from);
	for (; bp != ep; bp = bp->q_forw) {
		if ((s = get_sbuf_line(bp)) == nullptr)
			return ERR;
		if (put_tty_line(s, bp->len, current_addr = from++, gflag) < 0)
			return ERR;
	}
	return 0;
}
/* mark_line_node: set a line node mark */
int
mark_line_node(line_t *lp, int n)
{
	if (!islower((unsigned char)n)) {
		errmsg = "invalid mark character";
		return ERR;
	} else if (mark[n - 'a'] == nullptr)
		markno++;
	mark[n - 'a'] = lp;
	return 0;
}
/* get_marked_node_addr: return address of a marked line */
long
get_marked_node_addr(int n)
{
	if (!islower((unsigned char)n)) {
		errmsg = "invalid mark character";
		return ERR;
	}
	return get_line_node_addr(mark[n - 'a']);
}
/* unmark_line_node: clear line node mark */
void
unmark_line_node(line_t *lp)
{
	int i;

	for (i = 0; markno && i < MAXMARK; i++)
		if (mark[i] == lp) {
			mark[i] = nullptr;
			markno--;
		}
}
/* dup_line_node: return a pointer to a copy of a line node */
line_t *
dup_line_node(line_t *lp)
{
	line_t *np;

	if ((np = (line_t *) port_malloc(sizeof(line_t))) == nullptr) {
		fprintf(stderr, "%s\n", strerror(errno));
		errmsg = "out of memory";
		return nullptr;
	}
	np->seek = lp->seek;
	np->len = lp->len;
	return np;
}
/* has_trailing_escape:  return the parity of escapes preceding a character
   in a string */
int
has_trailing_escape(char *s, char *t)
{
    return (s == t || *(t - 1) != '\\') ? 0 : !has_trailing_escape(s, t - 1);
}
/* strip_escapes: return a copy of escaped string of at most length PATH_MAX */
char *
strip_escapes(char *s)
{
	static char *file = nullptr;
	static int filesz = 0;

	int i = 0;

	REALLOC(file, filesz, PATH_MAX, nullptr);
	while (i < filesz - 1	/* Worry about a possible trailing escape */
	       && (file[i++] = (*s == '\\') ? *++s : *s))
		s++;
	return file;
}
void
signal_hup(int signo)
{
	if (mutex)
		sigflags |= (1 << (signo - 1));
	else
		handle_hup(signo);
}
void
signal_int(int signo)
{
	if (mutex)
		sigflags |= (1 << (signo - 1));
	else
		handle_int(signo);
}
void
handle_hup(int signo)
{
	char *hup = nullptr;		/* hup filename */
	char *s;
	char ed_hup[] = "ed.hup";
	size_t n;

	if (!sigactive)
		quit(1);
	sigflags &= ~(1 << (signo - 1));
	if (addr_last && write_file(ed_hup, "w", 1, addr_last) < 0 &&
	    (s = getenv("HOME")) != nullptr &&
	    (n = strlen(s)) + 8 <= PATH_MAX &&	/* "ed.hup" + '/' */
	    (hup = (char *) port_malloc(n + 10)) != nullptr) {
		strcpy(hup, s);
		if (hup[n - 1] != '/')
			hup[n] = '/', hup[n+1] = '\0';
		strcat(hup, "ed.hup");
		write_file(hup, "w", 1, addr_last);
	}
	quit(2);
}
void
handle_int(int signo)
{
	if (!sigactive)
		quit(1);
	sigflags &= ~(1 << (signo - 1));
#ifdef _POSIX_SOURCE
	siglongjmp(env, -1);
#else
	longjmp(env, -1);
#endif
}
void
handle_winch(int signo)
{
	int save_errno = errno;

	struct winsize ws;		/* window size structure */

	sigflags &= ~(1 << (signo - 1));
	if (ioctl(0, TIOCGWINSZ, (char *) &ws) >= 0) {
		if (ws.ws_row > 2) rows = ws.ws_row - 2;
		if (ws.ws_col > 8) cols = ws.ws_col - 8;
	}
	errno = save_errno;
}
int
is_legal_filename(char *s)
{
	if (red && (*s == '!' || !strcmp(s, "..") || strchr(s, '/'))) {
		errmsg = "shell access restricted";
		return 0;
	}
	return 1;
}

void reset_batch(void)
{
	clear_active_list();
	clear_undo_stack();
	close_sbuf();
	clear_marks();
	reset_globals();
	REQUE(&buffer_head, &buffer_head);
	get_addressed_line_node(0);
}

} // namespace
