/*	$NetBSD: test.c,v 1.21 1999/04/05 09:48:38 kleink Exp $	*/
#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif
#ifndef __nonstring
#define __nonstring __attribute__((__nonstring__))
#endif
#ifndef __dead2
#define __dead2 __attribute__((__noreturn__))
#endif
#ifndef __printf0like
#define __printf0like(x, y)
#endif
#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
enum token_types {
	UNOP = 0x100,
	BINOP = 0x200,
	BUNOP = 0x300,
	BBINOP = 0x400,
	PAREN = 0x500
};

enum token {
	EOI,
	OPERAND,
	FILRD = UNOP + 1,
	FILWR,
	FILEX,
	FILEXIST,
	FILREG,
	FILDIR,
	FILCDEV,
	FILBDEV,
	FILFIFO,
	FILSOCK,
	FILSYM,
	FILGZ,
	FILTT,
	FILSUID,
	FILSGID,
	FILSTCK,
	STREZ,
	STRNZ,
	FILUID,
	FILGID,
	FILNT = BINOP + 1,
	FILOT,
	FILEQ,
	STREQ,
	STRNE,
	STRLT,
	STRGT,
	INTEQ,
	INTNE,
	INTGE,
	INTGT,
	INTLE,
	INTLT,
	UNOT = BUNOP + 1,
	BAND = BBINOP + 1,
	BOR,
	LPAREN = PAREN + 1,
	RPAREN
};

#define TOKEN_TYPE(token) ((token) & 0xff00)

struct t_op {
	char op_text[2] __nonstring;
	short op_num;
};

const struct t_op ref_ops1[] = {
	{"=",	STREQ},
	{"<",	STRLT},
	{">",	STRGT},
	{"!",	UNOT},
	{"(",	LPAREN},
	{")",	RPAREN},
}, ref_opsm1[] = {
	{"r",	FILRD},
	{"w",	FILWR},
	{"x",	FILEX},
	{"e",	FILEXIST},
	{"f",	FILREG},
	{"d",	FILDIR},
	{"c",	FILCDEV},
	{"b",	FILBDEV},
	{"p",	FILFIFO},
	{"u",	FILSUID},
	{"g",	FILSGID},
	{"k",	FILSTCK},
	{"s",	FILGZ},
	{"t",	FILTT},
	{"z",	STREZ},
	{"n",	STRNZ},
	{"h",	FILSYM},		/* for backwards compat */
	{"O",	FILUID},
	{"G",	FILGID},
	{"L",	FILSYM},
	{"S",	FILSOCK},
	{"a",	BAND},
	{"o",	BOR},
}, ref_ops2[] = {
	{"==",	STREQ},
	{"!=",	STRNE},
}, ref_opsm2[] = {
	{"eq",	INTEQ},
	{"ne",	INTNE},
	{"ge",	INTGE},
	{"gt",	INTGT},
	{"le",	INTLE},
	{"lt",	INTLT},
	{"nt",	FILNT},
	{"ot",	FILOT},
	{"ef",	FILEQ},
};

const struct t_op *const ref_ops1_end = (&ref_ops1)[1];
const struct t_op *const ref_opsm1_end = (&ref_opsm1)[1];
const struct t_op *const ref_ops2_end = (&ref_ops2)[1];
const struct t_op *const ref_opsm2_end = (&ref_opsm2)[1];
int ref_nargc;
char **ref_t_wp;
int ref_parenlevel;

int	ref_aexpr(enum token);
int	ref_binop(enum token);
int	ref_equalf(const char *, const char *);
int	ref_filstat(char *, enum token);
int	ref_getn(const char *);
intmax_t	ref_getq(const char *);
int	ref_intcmp(const char *, const char *);
int	ref_isunopoperand(void);
int	ref_islparenoperand(void);
int	ref_isrparenoperand(void);
int	ref_newerf(const char *, const char *);
int	ref_nexpr(enum token);
int	ref_oexpr(enum token);
int	ref_olderf(const char *, const char *);
int	ref_primary(enum token);
void	ref_syntax(const char *, const char *);
enum token ref_t_lex(char *);
void ref_error(const char *msg, ...) __dead2 __printf0like(1, 2);
void ref_error(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	verrx(2, msg, ap);
	va_end(ap);
}
int ref_main(int argc, char **argv)
{
	int	res;
	char	*p;

	if ((p = strrchr(argv[0], '/')) == NULL)
		p = argv[0];
	else
		p++;
	if (strcmp(p, "[") == 0) {
		if (strcmp(argv[--argc], "]") != 0)
			ref_error("missing ']'");
		argv[argc] = NULL;
	}

	/* no expression => false */
	if (--argc <= 0)
		return 1;

	(void)setlocale(LC_CTYPE, "");
	ref_nargc = argc;
	ref_t_wp = &argv[1];
	ref_parenlevel = 0;
	if (ref_nargc == 4 && strcmp(*ref_t_wp, "!") == 0) {
		/* Things like ! "" -o x do not fit in the normal grammar. */
		--ref_nargc;
		++ref_t_wp;
		res = ref_oexpr(ref_t_lex(*ref_t_wp));
	} else
		res = !ref_oexpr(ref_t_lex(*ref_t_wp));

	if (--ref_nargc > 0)
		ref_syntax(*ref_t_wp, "unexpected operator");

	return res;
}

void
ref_syntax(const char *op, const char *msg)
{

	if (op && *op)
		ref_error("%s: %s", op, msg);
	else
		ref_error("%s", msg);
}

int
ref_oexpr(enum token n)
{
	int res;

	res = ref_aexpr(n);
	if (ref_t_lex(ref_nargc > 0 ? (--ref_nargc, *++ref_t_wp) : NULL) == BOR)
		return ref_oexpr(ref_t_lex(ref_nargc > 0 ? (--ref_nargc, *++ref_t_wp) : NULL)) ||
		    res;
	ref_t_wp--;
	ref_nargc++;
	return res;
}

int
ref_aexpr(enum token n)
{
	int res;

	res = ref_nexpr(n);
	if (ref_t_lex(ref_nargc > 0 ? (--ref_nargc, *++ref_t_wp) : NULL) == BAND)
		return ref_aexpr(ref_t_lex(ref_nargc > 0 ? (--ref_nargc, *++ref_t_wp) : NULL)) &&
		    res;
	ref_t_wp--;
	ref_nargc++;
	return res;
}

int
ref_nexpr(enum token n)
{
	if (n == UNOT)
		return !ref_nexpr(ref_t_lex(ref_nargc > 0 ? (--ref_nargc, *++ref_t_wp) : NULL));
	return ref_primary(n);
}

int
ref_primary(enum token n)
{
	enum token nn;
	int res;

	if (n == EOI)
		return 0;		/* missing expression */
	if (n == LPAREN) {
		ref_parenlevel++;
		if ((nn = ref_t_lex(ref_nargc > 0 ? (--ref_nargc, *++ref_t_wp) : NULL)) ==
		    RPAREN) {
			ref_parenlevel--;
			return 0;	/* missing expression */
		}
		res = ref_oexpr(nn);
		if (ref_t_lex(ref_nargc > 0 ? (--ref_nargc, *++ref_t_wp) : NULL) != RPAREN)
			ref_syntax(NULL, "closing paren expected");
		ref_parenlevel--;
		return res;
	}
	if (TOKEN_TYPE(n) == UNOP) {
		/* unary expression */
		if (--ref_nargc == 0)
			ref_syntax(NULL, "argument expected"); /* impossible */
		switch (n) {
		case STREZ:
			return strlen(*++ref_t_wp) == 0;
		case STRNZ:
			return strlen(*++ref_t_wp) != 0;
		case FILTT:
			return isatty(ref_getn(*++ref_t_wp));
		default:
			return ref_filstat(*++ref_t_wp, n);
		}
	}

	nn = ref_t_lex(ref_nargc > 0 ? ref_t_wp[1] : NULL);
	if (TOKEN_TYPE(nn) == BINOP)
		return ref_binop(nn);

	return strlen(*ref_t_wp) > 0;
}

int
ref_binop(enum token n)
{
	const char *opnd1, *op, *opnd2;

	opnd1 = *ref_t_wp;
	op = ref_nargc > 0 ? (--ref_nargc, *++ref_t_wp) : NULL;

	if ((opnd2 = ref_nargc > 0 ? (--ref_nargc, *++ref_t_wp) : NULL) == NULL)
		ref_syntax(op, "argument expected");

	switch (n) {
	case STREQ:
		return strcmp(opnd1, opnd2) == 0;
	case STRNE:
		return strcmp(opnd1, opnd2) != 0;
	case STRLT:
		return strcmp(opnd1, opnd2) < 0;
	case STRGT:
		return strcmp(opnd1, opnd2) > 0;
	case INTEQ:
		return ref_intcmp(opnd1, opnd2) == 0;
	case INTNE:
		return ref_intcmp(opnd1, opnd2) != 0;
	case INTGE:
		return ref_intcmp(opnd1, opnd2) >= 0;
	case INTGT:
		return ref_intcmp(opnd1, opnd2) > 0;
	case INTLE:
		return ref_intcmp(opnd1, opnd2) <= 0;
	case INTLT:
		return ref_intcmp(opnd1, opnd2) < 0;
	case FILNT:
		return ref_newerf(opnd1, opnd2);
	case FILOT:
		return ref_olderf(opnd1, opnd2);
	case FILEQ:
		return ref_equalf(opnd1, opnd2);
	default:
		abort();
		/* NOTREACHED */
	}
}

int
ref_filstat(char *nm, enum token mode)
{
	struct stat s;

	if (mode == FILSYM ? lstat(nm, &s) : stat(nm, &s))
		return 0;

	switch (mode) {
	case FILRD:
		return (eaccess(nm, R_OK) == 0);
	case FILWR:
		return (eaccess(nm, W_OK) == 0);
	case FILEX:
		/* XXX work around eaccess(2) false positives for superuser */
		if (eaccess(nm, X_OK) != 0)
			return 0;
		if (S_ISDIR(s.st_mode) || geteuid() != 0)
			return 1;
		return (s.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
	case FILEXIST:
		return (eaccess(nm, F_OK) == 0);
	case FILREG:
		return S_ISREG(s.st_mode);
	case FILDIR:
		return S_ISDIR(s.st_mode);
	case FILCDEV:
		return S_ISCHR(s.st_mode);
	case FILBDEV:
		return S_ISBLK(s.st_mode);
	case FILFIFO:
		return S_ISFIFO(s.st_mode);
	case FILSOCK:
		return S_ISSOCK(s.st_mode);
	case FILSYM:
		return S_ISLNK(s.st_mode);
	case FILSUID:
		return (s.st_mode & S_ISUID) != 0;
	case FILSGID:
		return (s.st_mode & S_ISGID) != 0;
	case FILSTCK:
		return (s.st_mode & S_ISVTX) != 0;
	case FILGZ:
		return s.st_size > (off_t)0;
	case FILUID:
		return s.st_uid == geteuid();
	case FILGID:
		return s.st_gid == getegid();
	default:
		return 1;
	}
}

int
ref_find_op_1char(const struct t_op *op, const struct t_op *end, const char *s)
{
	char c;

	c = s[0];
	while (op != end) {
		if (c == *op->op_text)
			return op->op_num;
		op++;
	}
	return OPERAND;
}

int
ref_find_op_2char(const struct t_op *op, const struct t_op *end, const char *s)
{
	while (op != end) {
		if (s[0] == op->op_text[0] && s[1] == op->op_text[1])
			return op->op_num;
		op++;
	}
	return OPERAND;
}

int
ref_find_op(const char *s)
{
	if (s[0] == '\0')
		return OPERAND;
	else if (s[1] == '\0')
		return ref_find_op_1char(ref_ops1, (&ref_ops1)[1], s);
	else if (s[2] == '\0')
		return s[0] == '-' ? ref_find_op_1char(ref_opsm1, (&ref_opsm1)[1], s + 1) :
		    ref_find_op_2char(ref_ops2, (&ref_ops2)[1], s);
	else if (s[3] == '\0')
		return s[0] == '-' ? ref_find_op_2char(ref_opsm2, (&ref_opsm2)[1], s + 1) :
		    OPERAND;
	else
		return OPERAND;
}

enum token ref_t_lex(char *s)
{
	int num;

	if (s == NULL) {
		return EOI;
	}
	num = ref_find_op(s);
	if (((TOKEN_TYPE(num) == UNOP || TOKEN_TYPE(num) == BUNOP)
				&& ref_isunopoperand()) ||
	    (num == LPAREN && ref_islparenoperand()) ||
	    (num == RPAREN && ref_isrparenoperand()))
		return OPERAND;
	return num;
}

int
ref_isunopoperand(void)
{
	char *s;
	char *t;
	int num;

	if (ref_nargc == 1)
		return 1;
	s = *(ref_t_wp + 1);
	if (ref_nargc == 2)
		return ref_parenlevel == 1 && strcmp(s, ")") == 0;
	t = *(ref_t_wp + 2);
	num = ref_find_op(s);
	return TOKEN_TYPE(num) == BINOP &&
	    (ref_parenlevel == 0 || t[0] != ')' || t[1] != '\0');
}

int
ref_islparenoperand(void)
{
	char *s;
	int num;

	if (ref_nargc == 1)
		return 1;
	s = *(ref_t_wp + 1);
	if (ref_nargc == 2)
		return ref_parenlevel == 1 && strcmp(s, ")") == 0;
	if (ref_nargc != 3)
		return 0;
	num = ref_find_op(s);
	return TOKEN_TYPE(num) == BINOP;
}

int
ref_isrparenoperand(void)
{
	char *s;

	if (ref_nargc == 1)
		return 0;
	s = *(ref_t_wp + 1);
	if (ref_nargc == 2)
		return ref_parenlevel == 1 && strcmp(s, ")") == 0;
	return 0;
}

/* atoi with error detection */
int
ref_getn(const char *s)
{
	char *p;
	long r;

	errno = 0;
	r = strtol(s, &p, 10);

	if (s == p)
		ref_error("%s: bad number", s);

	if (errno != 0)
		ref_error((errno == EINVAL) ? "%s: bad number" :
					  "%s: out of range", s);

	while (isspace((unsigned char)*p))
		p++;

	if (*p)
		ref_error("%s: bad number", s);

	return (int) r;
}

/* atoi with error detection and 64 bit range */
intmax_t
ref_getq(const char *s)
{
	char *p;
	intmax_t r;

	errno = 0;
	r = strtoimax(s, &p, 10);

	if (s == p)
		ref_error("%s: bad number", s);

	if (errno != 0)
		ref_error((errno == EINVAL) ? "%s: bad number" :
					  "%s: out of range", s);

	while (isspace((unsigned char)*p))
		p++;

	if (*p)
		ref_error("%s: bad number", s);

	return r;
}

int
ref_intcmp (const char *s1, const char *s2)
{
	intmax_t q1, q2;


	q1 = ref_getq(s1);
	q2 = ref_getq(s2);

	if (q1 > q2)
		return 1;

	if (q1 < q2)
		return -1;

	return 0;
}

int
ref_newerf (const char *f1, const char *f2)
{
	struct stat b1, b2;

	if (stat(f1, &b1) != 0 || stat(f2, &b2) != 0)
		return 0;

	if (b1.st_mtim.tv_sec > b2.st_mtim.tv_sec)
		return 1;
	if (b1.st_mtim.tv_sec < b2.st_mtim.tv_sec)
		return 0;

       return (b1.st_mtim.tv_nsec > b2.st_mtim.tv_nsec);
}

int
ref_olderf (const char *f1, const char *f2)
{
	return (ref_newerf(f2, f1));
}

int
ref_equalf (const char *f1, const char *f2)
{
	struct stat b1, b2;

	return (stat (f1, &b1) == 0 &&
		stat (f2, &b2) == 0 &&
		b1.st_dev == b2.st_dev &&
		b1.st_ino == b2.st_ino);
}
