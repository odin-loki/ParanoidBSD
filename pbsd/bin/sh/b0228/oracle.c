/*
 * oracle.c -- reference implementations for batch b0228.
 */

#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

typedef intmax_t arith_t;
#define ARITH_FORMAT_STR "%" PRIdMAX
#define ARITH_MIN INTMAX_MIN
#define ARITH_MAX INTMAX_MAX

#define ARITH_ASS 1
#define ARITH_OR 2
#define ARITH_AND 3
#define ARITH_BAD 4
#define ARITH_NUM 5
#define ARITH_VAR 6
#define ARITH_NOT 7
#define ARITH_BINOP_MIN 8
#define ARITH_LE 8
#define ARITH_GE 9
#define ARITH_LT 10
#define ARITH_GT 11
#define ARITH_EQ 12
#define ARITH_REM 13
#define ARITH_BAND 14
#define ARITH_LSHIFT 15
#define ARITH_RSHIFT 16
#define ARITH_MUL 17
#define ARITH_ADD 18
#define ARITH_BOR 19
#define ARITH_SUB 20
#define ARITH_BXOR 21
#define ARITH_DIV 22
#define ARITH_NE 23
#define ARITH_BINOP_MAX 24
#define ARITH_ASS_MIN 24
#define ARITH_REMASS 24
#define ARITH_BANDASS 25
#define ARITH_LSHIFTASS 26
#define ARITH_RSHIFTASS 27
#define ARITH_MULASS 28
#define ARITH_ADDASS 29
#define ARITH_BORASS 30
#define ARITH_SUBASS 31
#define ARITH_BXORASS 32
#define ARITH_DIVASS 33
#define ARITH_ASS_MAX 34
#define ARITH_LPAREN 34
#define ARITH_RPAREN 35
#define ARITH_BNOT 36
#define ARITH_QMARK 37
#define ARITH_COLON 38

#define DIGITS(var) (3 + (2 + CHAR_BIT * sizeof((var))) / 3)

union yystype {
	arith_t val;
	char *name;
};

#define __dead2
#define __unused

jmp_buf b0228_jmp;
int b0228_err_exit;

static int oracle_uflag;
static char oracle_varbuf[256];
static char oracle_outbuf[512];
static int oracle_outlen;

struct stackmark { char dummy; };
static char oracle_stack[65536];
static size_t oracle_stack_off;

void oracle_reset_state(void)
{
	oracle_uflag = 0;
	oracle_varbuf[0] = '\0';
	oracle_outlen = 0;
	oracle_outbuf[0] = '\0';
	oracle_stack_off = 0;
	b0228_err_exit = 0;
}

void oracle_set_var(const char *name, const char *val)
{
	(void)name;
	strncpy(oracle_varbuf, val, sizeof(oracle_varbuf) - 1);
	oracle_varbuf[sizeof(oracle_varbuf) - 1] = '\0';
}

void oracle_set_uflag(int v) { oracle_uflag = v; }

const char *lookupvar(const char *name)
{
	(void)name;
	if (oracle_varbuf[0] == '\0')
		return NULL;
	return oracle_varbuf;
}

void setvar(const char *name, const char *val, int flags)
{
	(void)flags;
	oracle_set_var(name, val);
}

void error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	b0228_err_exit = 1;
	longjmp(b0228_jmp, 1);
}

void out1fmt(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	oracle_outlen += vsnprintf(oracle_outbuf + oracle_outlen,
	    (int)sizeof(oracle_outbuf) - oracle_outlen, fmt, ap);
	va_end(ap);
}

void setstackmark(struct stackmark *mark) { (void)mark; oracle_stack_off = 0; }
void popstackmark(struct stackmark *mark) { (void)mark; }

#define STARTSTACKSTR(p) char *p = oracle_stack + oracle_stack_off
#define STPUTC(c, p) do { *(p)++ = (char)(c); } while (0)
char *grabstackstr(char *p)
{
	*p = '\0';
	oracle_stack_off = (size_t)(p + 1 - oracle_stack);
	return p;
}

const char *arith_buf;
union yystype yylval;

static int is_in_name(int c)
{
	return isalnum((unsigned char)c) || c == '_';
}

arith_t strtoarith_t(const char *restrict nptr, char **restrict endptr)
{
	arith_t val;

	while (isspace((unsigned char)*nptr))
		nptr++;
	switch (*nptr) {
		case '-':
			return strtoimax(nptr, endptr, 0);
		case '0':
			return (arith_t)strtoumax(nptr, endptr, 0);
		default:
			val = (arith_t)strtoumax(nptr, endptr, 0);
			if (val >= 0)
				return val;
			else if (val == ARITH_MIN) {
				errno = ERANGE;
				return ARITH_MIN;
			} else {
				errno = ERANGE;
				return ARITH_MAX;
			}
	}
}

int yylex(void)
{
	int value;
	const char *buf = arith_buf;
	char *end;
	const char *p;

	for (;;) {
		value = *buf;
		switch (value) {
		case ' ':
		case '\t':
		case '\n':
			buf++;
			continue;
		default:
			return ARITH_BAD;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			yylval.val = strtoarith_t(buf, &end);
			arith_buf = end;
			return ARITH_NUM;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
		case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
		case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
		case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case '_':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
		case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
		case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
		case 'v': case 'w': case 'x': case 'y': case 'z':
			p = buf;
			while (buf++, is_in_name(*buf))
				;
			yylval.name = malloc((size_t)(buf - p + 1));
			memcpy(yylval.name, p, (size_t)(buf - p));
			yylval.name[buf - p] = '\0';
			value = ARITH_VAR;
			goto out;
		case '=':
			value += ARITH_ASS - '=';
checkeq:
			buf++;
checkeqcur:
			if (*buf != '=')
				goto out;
			value += 11;
			break;
		case '>':
			switch (*++buf) {
			case '=':
				value += ARITH_GE - '>';
				break;
			case '>':
				value += ARITH_RSHIFT - '>';
				goto checkeq;
			default:
				value += ARITH_GT - '>';
				goto out;
			}
			break;
		case '<':
			switch (*++buf) {
			case '=':
				value += ARITH_LE - '<';
				break;
			case '<':
				value += ARITH_LSHIFT - '<';
				goto checkeq;
			default:
				value += ARITH_LT - '<';
				goto out;
			}
			break;
		case '|':
			if (*++buf != '|') {
				value += ARITH_BOR - '|';
				goto checkeqcur;
			}
			value += ARITH_OR - '|';
			break;
		case '&':
			if (*++buf != '&') {
				value += ARITH_BAND - '&';
				goto checkeqcur;
			}
			value += ARITH_AND - '&';
			break;
		case '!':
			if (*++buf != '=') {
				value += ARITH_NOT - '!';
				goto out;
			}
			value += ARITH_NE - '!';
			break;
		case 0:
			goto out;
		case '(':
			value += ARITH_LPAREN - '(';
			break;
		case ')':
			value += ARITH_RPAREN - ')';
			break;
		case '*':
			value += ARITH_MUL - '*';
			goto checkeq;
		case '/':
			value += ARITH_DIV - '/';
			goto checkeq;
		case '%':
			value += ARITH_REM - '%';
			goto checkeq;
		case '+':
			if (buf[1] == '+')
				return ARITH_BAD;
			value += ARITH_ADD - '+';
			goto checkeq;
		case '-':
			if (buf[1] == '-')
				return ARITH_BAD;
			value += ARITH_SUB - '-';
			goto checkeq;
		case '~':
			value += ARITH_BNOT - '~';
			break;
		case '^':
			value += ARITH_BXOR - '^';
			goto checkeq;
		case '?':
			value += ARITH_QMARK - '?';
			break;
		case ':':
			value += ARITH_COLON - ':';
			break;
		}
		break;
	}

	buf++;
out:
	arith_buf = buf;
	return value;
}

FILE *cfile;
FILE *hfile;

void oracle_set_cfile(FILE *f) { cfile = f; }
void oracle_set_hfile(FILE *f) { hfile = f; }


/* ========== mksyntax.c ========== */
#define static
#define main ref_main
#define init ref_init
#define add_one ref_add_one
#define add_default ref_add_default
#define finish ref_finish
#define add ref_add
#define output_type_macros ref_output_type_macros
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
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
 * This program creates syntax.h and syntax.c.
 */



struct synclass {
	const char *name;
	const char *comment;
};

/* Syntax classes */
static const struct synclass synclass[] = {
	{ "CWORD",	"character is nothing special" },
	{ "CNL",	"newline character" },
	{ "CQNL",	"newline character in quotes" },
	{ "CBACK",	"a backslash character" },
	{ "CSBACK",	"a backslash character in single quotes" },
	{ "CSQUOTE",	"single quote" },
	{ "CDQUOTE",	"double quote" },
	{ "CENDQUOTE",	"a terminating quote" },
	{ "CBQUOTE",	"backwards single quote" },
	{ "CVAR",	"a dollar sign" },
	{ "CENDVAR",	"a '}' character" },
	{ "CLP",	"a left paren in arithmetic" },
	{ "CRP",	"a right paren in arithmetic" },
	{ "CEOF",	"end of file" },
	{ "CCTL",	"like CWORD, except it must be escaped" },
	{ "CSPCL",	"these terminate a word" },
	{ "CIGN",       "character should be ignored" },
	{ NULL,		NULL }
};


/*
 * Syntax classes for is_ functions.  Warning:  if you add new classes
 * you may have to change the definition of the is_in_name macro.
 */
static const struct synclass is_entry[] = {
	{ "ISDIGIT",	"a digit" },
	{ "ISUPPER",	"an upper case letter" },
	{ "ISLOWER",	"a lower case letter" },
	{ "ISUNDER",	"an underscore" },
	{ "ISSPECL",	"the name of a special parameter" },
	{ NULL, 	NULL }
};

static const char writer[] = "\
/*\n\
 * This file was generated by the mksyntax program.\n\
 */\n\
\n";


static FILE *cfile;
static FILE *hfile;

static void add_default(void);
static void finish(void);
static void init(const char *);
static void add(const char *, const char *);
static void output_type_macros(void);

int
main(int argc __unused, char **argv __unused)
{
	int i;
	char buf[80];
	int pos;

	/* Create output files */
	if ((cfile = fopen("syntax.c", "w")) == NULL) {
		perror("syntax.c");
		exit(2);
	}
	if ((hfile = fopen("syntax.h", "w")) == NULL) {
		perror("syntax.h");
		exit(2);
	}
	fputs(writer, hfile);
	fputs(writer, cfile);

	fputs("#include <sys/cdefs.h>\n", hfile);
	fputs("#include <limits.h>\n\n", hfile);

	/* Generate the #define statements in the header file */
	fputs("/* Syntax classes */\n", hfile);
	for (i = 0 ; synclass[i].name ; i++) {
		sprintf(buf, "#define %s %d", synclass[i].name, i);
		fputs(buf, hfile);
		for (pos = strlen(buf) ; pos < 32 ; pos = (pos + 8) & ~07)
			putc('\t', hfile);
		fprintf(hfile, "/* %s */\n", synclass[i].comment);
	}
	putc('\n', hfile);
	fputs("/* Syntax classes for is_ functions */\n", hfile);
	for (i = 0 ; is_entry[i].name ; i++) {
		sprintf(buf, "#define %s %#o", is_entry[i].name, 1 << i);
		fputs(buf, hfile);
		for (pos = strlen(buf) ; pos < 32 ; pos = (pos + 8) & ~07)
			putc('\t', hfile);
		fprintf(hfile, "/* %s */\n", is_entry[i].comment);
	}
	putc('\n', hfile);
	fputs("#define SYNBASE (1 - CHAR_MIN)\n", hfile);
	fputs("#define PEOF -SYNBASE\n\n", hfile);
	putc('\n', hfile);
	fputs("#define BASESYNTAX (basesyntax + SYNBASE)\n", hfile);
	fputs("#define DQSYNTAX (dqsyntax + SYNBASE)\n", hfile);
	fputs("#define SQSYNTAX (sqsyntax + SYNBASE)\n", hfile);
	fputs("#define ARISYNTAX (arisyntax + SYNBASE)\n", hfile);
	putc('\n', hfile);
	output_type_macros();		/* is_digit, etc. */
	putc('\n', hfile);

	/* Generate the syntax tables. */
	fputs("#include \"parser.h\"\n", cfile);
	fputs("#include \"shell.h\"\n", cfile);
	fputs("#include \"syntax.h\"\n\n", cfile);

	fputs("/* syntax table used when not in quotes */\n", cfile);
	init("basesyntax");
	add_default();
	add("\n", "CNL");
	add("\\", "CBACK");
	add("'", "CSQUOTE");
	add("\"", "CDQUOTE");
	add("`", "CBQUOTE");
	add("$", "CVAR");
	add("}", "CENDVAR");
	add("<>();&| \t", "CSPCL");
	finish();

	fputs("\n/* syntax table used when in double quotes */\n", cfile);
	init("dqsyntax");
	add_default();
	add("\n", "CQNL");
	add("\\", "CBACK");
	add("\"", "CENDQUOTE");
	add("`", "CBQUOTE");
	add("$", "CVAR");
	add("}", "CENDVAR");
	/* ':/' for tilde expansion, '-^]' for [a\-x] pattern ranges */
	add("!*?[]=~:/-^", "CCTL");
	finish();

	fputs("\n/* syntax table used when in single quotes */\n", cfile);
	init("sqsyntax");
	add_default();
	add("\n", "CQNL");
	add("\\", "CSBACK");
	add("'", "CENDQUOTE");
	/* ':/' for tilde expansion, '-^]' for [a\-x] pattern ranges */
	add("!*?[]=~:/-^", "CCTL");
	finish();

	fputs("\n/* syntax table used when in arithmetic */\n", cfile);
	init("arisyntax");
	add_default();
	add("\n", "CQNL");
	add("\\", "CBACK");
	add("`", "CBQUOTE");
	add("\"", "CIGN");
	add("$", "CVAR");
	add("}", "CENDVAR");
	add("(", "CLP");
	add(")", "CRP");
	finish();

	fputs("\n/* character classification table */\n", cfile);
	init("is_type");
	add("0123456789", "ISDIGIT");
	add("abcdefghijklmnopqrstuvwxyz", "ISLOWER");
	add("ABCDEFGHIJKLMNOPQRSTUVWXYZ", "ISUPPER");
	add("_", "ISUNDER");
	add("#?$!-*@", "ISSPECL");
	finish();

	exit(0);
}


/*
 * Output the header and declaration of a syntax table.
 */

static void
init(const char *name)
{
	fprintf(hfile, "extern const char %s[];\n", name);
	fprintf(cfile, "const char %s[SYNBASE + CHAR_MAX + 1] = {\n", name);
}


static void
add_one(const char *key, const char *type)
{
	fprintf(cfile, "\t[SYNBASE + %s] = %s,\n", key, type);
}


/*
 * Add default values to the syntax table.
 */

static void
add_default(void)
{
	add_one("PEOF",                "CEOF");
	add_one("CTLESC",              "CCTL");
	add_one("CTLVAR",              "CCTL");
	add_one("CTLENDVAR",           "CCTL");
	add_one("CTLBACKQ",            "CCTL");
	add_one("CTLBACKQ + CTLQUOTE", "CCTL");
	add_one("CTLARI",              "CCTL");
	add_one("CTLENDARI",           "CCTL");
	add_one("CTLQUOTEMARK",        "CCTL");
	add_one("CTLQUOTEEND",         "CCTL");
}


/*
 * Output the footer of a syntax table.
 */

static void
finish(void)
{
	fputs("};\n", cfile);
}


/*
 * Add entries to the syntax table.
 */

static void
add(const char *p, const char *type)
{
	for (; *p; ++p) {
		char c = *p;
		switch (c) {
		case '\t': c = 't';  break;
		case '\n': c = 'n';  break;
		case '\'': c = '\''; break;
		case '\\': c = '\\'; break;

		default:
			fprintf(cfile, "\t[SYNBASE + '%c'] = %s,\n", c, type);
			continue;
		}
		fprintf(cfile, "\t[SYNBASE + '\\%c'] = %s,\n", c, type);
	}
}


/*
 * Output character classification macros (e.g. is_digit).  If digits are
 * contiguous, we can test for them quickly.
 */

static const char *macro[] = {
	"#define is_digit(c)\t((unsigned int)((c) - '0') <= 9)",
	"#define is_eof(c)\t((c) == PEOF)",
	"#define is_alpha(c)\t((is_type+SYNBASE)[(int)c] & (ISUPPER|ISLOWER))",
	"#define is_name(c)\t((is_type+SYNBASE)[(int)c] & (ISUPPER|ISLOWER|ISUNDER))",
	"#define is_in_name(c)\t((is_type+SYNBASE)[(int)c] & (ISUPPER|ISLOWER|ISUNDER|ISDIGIT))",
	"#define is_special(c)\t((is_type+SYNBASE)[(int)c] & (ISSPECL|ISDIGIT))",
	"#define digit_val(c)\t((c) - '0')",
	NULL
};

static void
output_type_macros(void)
{
	const char **pp;

	for (pp = macro ; *pp ; pp++)
		fprintf(hfile, "%s\n", *pp);
}

#undef static
#undef main
#undef init
#undef add_one
#undef add_default
#undef finish
#undef add
#undef output_type_macros

/* ========== arith_yacc.c ========== */
#define static
#define yyerror ref_yyerror
#define arith_lookupvarint ref_arith_lookupvarint
#define arith_prec ref_arith_prec
#define higher_prec ref_higher_prec
#define do_binop ref_do_binop
#define primary ref_primary
#define binop2 ref_binop2
#define binop ref_binop
#define and ref_and
#define or ref_or
#define cond ref_cond
#define assignment ref_assignment
#define arith ref_arith
#define letcmd ref_letcmd
/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 2007
 *	Herbert Xu <herbert@gondor.apana.org.au>.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
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


#if ARITH_BOR + 11 != ARITH_BORASS || ARITH_ASS + 11 != ARITH_EQ
#error Arithmetic tokens are out of order.
#endif

static const char *arith_startbuf;

const char *arith_buf;
union yystype yylval;

static int last_token;

#define ARITH_PRECEDENCE(op, prec) [op - ARITH_BINOP_MIN] = prec

static const char prec[ARITH_BINOP_MAX - ARITH_BINOP_MIN] = {
	ARITH_PRECEDENCE(ARITH_MUL, 0),
	ARITH_PRECEDENCE(ARITH_DIV, 0),
	ARITH_PRECEDENCE(ARITH_REM, 0),
	ARITH_PRECEDENCE(ARITH_ADD, 1),
	ARITH_PRECEDENCE(ARITH_SUB, 1),
	ARITH_PRECEDENCE(ARITH_LSHIFT, 2),
	ARITH_PRECEDENCE(ARITH_RSHIFT, 2),
	ARITH_PRECEDENCE(ARITH_LT, 3),
	ARITH_PRECEDENCE(ARITH_LE, 3),
	ARITH_PRECEDENCE(ARITH_GT, 3),
	ARITH_PRECEDENCE(ARITH_GE, 3),
	ARITH_PRECEDENCE(ARITH_EQ, 4),
	ARITH_PRECEDENCE(ARITH_NE, 4),
	ARITH_PRECEDENCE(ARITH_BAND, 5),
	ARITH_PRECEDENCE(ARITH_BXOR, 6),
	ARITH_PRECEDENCE(ARITH_BOR, 7),
};

#define ARITH_MAX_PREC 8

int letcmd(int, char **);

static __dead2 void yyerror(const char *s)
{
	error("arithmetic expression: %s: \"%s\"", s, arith_startbuf);
	/* NOTREACHED */
}

static arith_t arith_lookupvarint(char *varname)
{
	const char *str;
	char *p;
	arith_t result;

	str = lookupvar(varname);
	if (oracle_uflag && str == NULL)
		yyerror("variable not set");
	if (str == NULL || *str == '\0')
		str = "0";
	errno = 0;
	result = strtoarith_t(str, &p);
	if (errno != 0 || *p != '\0')
		yyerror("variable conversion error");
	return result;
}

static inline int arith_prec(int op)
{
	return prec[op - ARITH_BINOP_MIN];
}

static inline int higher_prec(int op1, int op2)
{
	return arith_prec(op1) < arith_prec(op2);
}

static arith_t do_binop(int op, arith_t a, arith_t b)
{

	switch (op) {
	default:
	case ARITH_REM:
	case ARITH_DIV:
		if (!b)
			yyerror("division by zero");
		if (a == ARITH_MIN && b == -1)
			yyerror("divide error");
		return op == ARITH_REM ? a % b : a / b;
	case ARITH_MUL:
		return (uintmax_t)a * (uintmax_t)b;
	case ARITH_ADD:
		return (uintmax_t)a + (uintmax_t)b;
	case ARITH_SUB:
		return (uintmax_t)a - (uintmax_t)b;
	case ARITH_LSHIFT:
		return (uintmax_t)a << (b & (sizeof(uintmax_t) * CHAR_BIT - 1));
	case ARITH_RSHIFT:
		return a >> (b & (sizeof(uintmax_t) * CHAR_BIT - 1));
	case ARITH_LT:
		return a < b;
	case ARITH_LE:
		return a <= b;
	case ARITH_GT:
		return a > b;
	case ARITH_GE:
		return a >= b;
	case ARITH_EQ:
		return a == b;
	case ARITH_NE:
		return a != b;
	case ARITH_BAND:
		return a & b;
	case ARITH_BXOR:
		return a ^ b;
	case ARITH_BOR:
		return a | b;
	}
}

static arith_t assignment(int var, int noeval);

static arith_t primary(int token, union yystype *val, int op, int noeval)
{
	arith_t result;

again:
	switch (token) {
	case ARITH_LPAREN:
		result = assignment(op, noeval);
		if (last_token != ARITH_RPAREN)
			yyerror("expecting ')'");
		last_token = yylex();
		return result;
	case ARITH_NUM:
		last_token = op;
		return val->val;
	case ARITH_VAR:
		last_token = op;
		return noeval ? val->val : arith_lookupvarint(val->name);
	case ARITH_ADD:
		token = op;
		*val = yylval;
		op = yylex();
		goto again;
	case ARITH_SUB:
		*val = yylval;
		return -primary(op, val, yylex(), noeval);
	case ARITH_NOT:
		*val = yylval;
		return !primary(op, val, yylex(), noeval);
	case ARITH_BNOT:
		*val = yylval;
		return ~primary(op, val, yylex(), noeval);
	default:
		yyerror("expecting primary");
	}
}

static arith_t binop2(arith_t a, int op, int precedence, int noeval)
{
	for (;;) {
		union yystype val;
		arith_t b;
		int op2;
		int token;

		token = yylex();
		val = yylval;

		b = primary(token, &val, yylex(), noeval);

		op2 = last_token;
		if (op2 >= ARITH_BINOP_MIN && op2 < ARITH_BINOP_MAX &&
		    higher_prec(op2, op)) {
			b = binop2(b, op2, arith_prec(op), noeval);
			op2 = last_token;
		}

		a = noeval ? b : do_binop(op, a, b);

		if (op2 < ARITH_BINOP_MIN || op2 >= ARITH_BINOP_MAX ||
		    arith_prec(op2) >= precedence)
			return a;

		op = op2;
	}
}

static arith_t binop(int token, union yystype *val, int op, int noeval)
{
	arith_t a = primary(token, val, op, noeval);

	op = last_token;
	if (op < ARITH_BINOP_MIN || op >= ARITH_BINOP_MAX)
		return a;

	return binop2(a, op, ARITH_MAX_PREC, noeval);
}

static arith_t and(int token, union yystype *val, int op, int noeval)
{
	arith_t a = binop(token, val, op, noeval);
	arith_t b;

	op = last_token;
	if (op != ARITH_AND)
		return a;

	token = yylex();
	*val = yylval;

	b = and(token, val, yylex(), noeval | !a);

	return a && b;
}

static arith_t or(int token, union yystype *val, int op, int noeval)
{
	arith_t a = and(token, val, op, noeval);
	arith_t b;

	op = last_token;
	if (op != ARITH_OR)
		return a;

	token = yylex();
	*val = yylval;

	b = or(token, val, yylex(), noeval | !!a);

	return a || b;
}

static arith_t cond(int token, union yystype *val, int op, int noeval)
{
	arith_t a = or(token, val, op, noeval);
	arith_t b;
	arith_t c;

	if (last_token != ARITH_QMARK)
		return a;

	b = assignment(yylex(), noeval | !a);

	if (last_token != ARITH_COLON)
		yyerror("expecting ':'");

	token = yylex();
	*val = yylval;

	c = cond(token, val, yylex(), noeval | !!a);

	return a ? b : c;
}

static arith_t assignment(int var, int noeval)
{
	union yystype val = yylval;
	int op = yylex();
	arith_t result;
	char sresult[DIGITS(result) + 1];

	if (var != ARITH_VAR)
		return cond(var, &val, op, noeval);

	if (op != ARITH_ASS && (op < ARITH_ASS_MIN || op >= ARITH_ASS_MAX))
		return cond(var, &val, op, noeval);

	result = assignment(yylex(), noeval);
	if (noeval)
		return result;

	if (op != ARITH_ASS)
		result = do_binop(op - 11, arith_lookupvarint(val.name), result);
	snprintf(sresult, sizeof(sresult), ARITH_FORMAT_STR, result);
	setvar(val.name, sresult, 0);
	return result;
}

arith_t arith(const char *s)
{
	struct stackmark smark;
	arith_t result;

	setstackmark(&smark);

	arith_buf = arith_startbuf = s;

	result = assignment(yylex(), 0);

	if (last_token)
		yyerror("expecting EOF");

	popstackmark(&smark);

	return result;
}

/*
 *  The exp(1) builtin.
 */
int
letcmd(int argc, char **argv)
{
	const char *p;
	char *concat;
	char **ap;
	arith_t i;

	if (argc > 1) {
		p = argv[1];
		if (argc > 2) {
			/*
			 * Concatenate arguments.
			 */
			STARTSTACKSTR(concat);
			ap = argv + 2;
			for (;;) {
				while (*p)
					STPUTC(*p++, concat);
				if ((p = *ap++) == NULL)
					break;
				STPUTC(' ', concat);
			}
			STPUTC('\0', concat);
			p = grabstackstr(concat);
		}
	} else
		p = "";

	i = arith(p);

	out1fmt(ARITH_FORMAT_STR "\n", i);
	return !i;
}

#undef static
#undef yyerror
#undef arith_lookupvarint
#undef arith_prec
#undef higher_prec
#undef do_binop
#undef primary
#undef binop2
#undef binop
#undef and
#undef or
#undef cond
#undef assignment
#undef arith
#undef letcmd
