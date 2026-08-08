// PBSD port of HardenedBSD ed batch b0148s3 (glbl.c).

module;

#include <cerrno>
#include <climits>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <regex.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

export module pbsd.bin.ed.b0148s3;

export namespace pbsd::bin_ed::b0148s3 {

#define ERR		(-2)
#define EMOD		(-3)
#define FATAL		(-4)

#define MINBUFSZ 512
#ifdef INT_MAX
# define LINECHARS INT_MAX
#else
# define LINECHARS MAXINT
#endif

using pattern_t = regex_t;

struct line_t {
	line_t	*q_forw;
	line_t	*q_back;
	off_t	seek;
	int	len;
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

#define NUL_TO_NEWLINE(s, l) translit_text(s, l, '\0', '\n')
#define NEWLINE_TO_NUL(s, l) translit_text(s, l, '\n', '\0')

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
int lineno;
int newline_added;
int scripted;
int patlock;

const char *errmsg = "";

int extract_addr_range_result;
int exec_command_result;
int display_lines_result;
char *extended_line;
int extended_line_len;
int get_tty_line_result = 1;
char tty_line[4096] = "\n";
int malloc_fail_at;
int malloc_calls;

int &mutex_ref() { return mutex; }
int &sigflags_ref() { return sigflags; }

void reset_hooks()
{
	extract_addr_range_result = 0;
	exec_command_result = 0;
	display_lines_result = 0;
	extended_line = nullptr;
	extended_line_len = 0;
	get_tty_line_result = 1;
	std::strcpy(tty_line, "\n");
	malloc_fail_at = 0;
	malloc_calls = 0;
}

void *port_malloc(std::size_t n)
{
	void *p;

	malloc_calls++;
	if (malloc_fail_at != 0 && malloc_calls >= malloc_fail_at)
		return (nullptr);
	p = std::malloc(n);
	return (p);
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
	lineno = 0;
	newline_added = 0;
	scripted = 0;
	patlock = 0;
	errmsg = "";
	reset_hooks();
}

int extract_addr_range(void)
{
	return (extract_addr_range_result);
}

int exec_command(void)
{
	return (exec_command_result);
}

int display_lines(long a, long b, int f)
{
	(void)a;
	(void)b;
	(void)f;
	return (display_lines_result);
}

char *get_extended_line(int *n, int f)
{
	(void)f;
	if (extended_line == nullptr)
		return (nullptr);
	*n = extended_line_len;
	return (extended_line);
}

int get_tty_line(void)
{
	if (get_tty_line_result < 0)
		return (-1);
	if (get_tty_line_result == 0)
		return (0);
	std::strcpy(ibuf, tty_line);
	return ((int)std::strlen(ibuf));
}

void unmark_line_node(line_t *lp)
{
	(void)lp;
}

void handle_hup(int s)
{
	(void)s;
}

void handle_int(int s)
{
	(void)s;
}

static long u_p;

void clear_undo_stack(void)
{
	u_p = 0;
}

pattern_t *get_compiled_pattern(void);
char *extract_pattern(int delimiter);
char *parse_char_class(char *s);
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
char *translit_text(char *s, int len, int from, int to);

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
	static char *lhbuf = NULL;
	static int lhbufsz = 0;

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

static FILE *sfp;
static off_t sfseek;
static int seek_write;
static line_t buffer_head;

char *
get_sbuf_line(line_t *lp)
{
	static char *sfbuf = NULL;
	static size_t sfbufsz;

	size_t len;

	if (lp == &buffer_head)
		return NULL;
	seek_write = 1;
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
	sfseek += len;
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
	for (s = cs; *s != '\n'; s++)
		;
	if (s - cs >= LINECHARS) {
		errmsg = "line too long";
		free(lp);
		return NULL;
	}
	len = s - cs;
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
	sfseek += len;
	return ++s;
}


void
add_line_node(line_t *lp)
{
	line_t *cp;

	cp = get_addressed_line_node(current_addr);
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

static char sfn[15] = "";

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


static unsigned char ctab[256];

char *
translit_text(char *s, int len, int from, int to)
{
	static int i = 0;

	unsigned char *us;

	ctab[i] = i;
	ctab[i = from] = to;
	for (us = (unsigned char *) s; len-- > 0; us++)
		*us = ctab[*us];
	return s;
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
		    set_active_node(lp) < 0)
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
			    (status = exec_command()) < 0 ||
			    (status > 0 && (status = display_lines(
			    current_addr, current_addr, status)) < 0))
				return status;
	}
	return 0;
}


line_t **active_list;
long active_last;
long active_size;
long active_ptr;
long active_ndx;

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

void reset_batch(void)
{
	clear_active_list();
	clear_undo_stack();
	close_sbuf();
	reset_globals();
	REQUE(&buffer_head, &buffer_head);
}

long port_active_last(void) { return active_last; }
long port_active_ptr(void) { return active_ptr; }
long port_active_ndx(void) { return active_ndx; }
long port_active_size(void) { return active_size; }
line_t *port_active_at(long i) { return active_list[i]; }

} /* namespace pbsd::bin_ed::b0148s3 */
