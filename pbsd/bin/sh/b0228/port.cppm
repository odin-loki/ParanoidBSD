/*
 * PBSD batch b0228 -- C++23 module port of:
 *	hbsd/src/bin/sh/arith_yacc.c
 *	hbsd/src/bin/sh/mksyntax.c
 *
 * hbsd/src/bin/sh/main.c and hbsd/src/bin/sh/redir.c are not ported; see
 * skipped.txt.
 *
 * The ports below are literal transcriptions.  Behaviour, including integer
 * signedness, evaluation order, wrap-around and the original bugs, is
 * preserved.  The only deviations, all of them mechanical and forced by the
 * language or by the need to link a differential harness against the code:
 *
 *   - file scope `static' is dropped from the ported functions (and from
 *     last_token / arith_startbuf / cfile / hfile) so the harness can call
 *     and observe them.  `inline' is dropped for the same reason.
 *   - arith_yacc.c's and() and or() are named arith_and() and arith_or():
 *     `and' and `or' are alternative tokens for && and || in C++.
 *   - mksyntax.c's main() is named mksyntax_main().
 *   - the out-of-batch environment (error, lookupvar, setvar, uflag, yylex,
 *     strtoarith_t, setstackmark, popstackmark, out1fmt and the memalloc.h
 *     stack-string macros) is declared with C language linkage and supplied
 *     by the link, exactly as the shell supplies it in the real build.
 */

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

module;

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* from shell.h */
#define	ARITH_FORMAT_STR  "%" PRIdMAX
#define	ARITH_MIN INTMAX_MIN
#define	ARITH_MAX INTMAX_MAX

/* from arith.h */
#define DIGITS(var) (3 + (2 + CHAR_BIT * sizeof((var))) / 3)

/* from arith_yacc.h */
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

/*
 * Stand-ins for the memalloc.h stack-string macros.  The shell's stack
 * allocator is not part of this batch; the harness provides the storage.
 * The character sequence produced by the ported code is unaffected.
 */
#define STARTSTACKSTR(p)	((p) = pbsd_b0228_stackstr_start())
#define STPUTC(c, p)		(*(p)++ = (char)(c))

/* from sys/cdefs.h */
#define __unused __attribute__((__unused__))

export module pbsd.bin.sh.b0228;

/*
 * The environment that lives outside this batch.  C language linkage, so
 * both this module and the reference C translation unit bind to the same
 * definitions.
 */
extern "C" {

struct stackmark {
	void *stackp;
	int nleft;
};

[[noreturn]] void error(const char *fmt, ...);
const char *lookupvar(const char *name);
void setvar(const char *name, const char *val, int flags);
extern int uflag;
intmax_t strtoarith_t(const char *__restrict nptr, char **__restrict endptr);
int yylex(void);
void setstackmark(struct stackmark *mark);
void popstackmark(struct stackmark *mark);
void out1fmt(const char *fmt, ...);
char *pbsd_b0228_stackstr_start(void);
char *grabstackstr(char *p);

}

export namespace pbsd::bin_sh::b0228 {

/* from shell.h */
typedef intmax_t arith_t;

/* from arith_yacc.h */
union yystype {
	arith_t val;
	char *name;
};

/* ------------------------------------------------------------------ */
/* arith_yacc.c							      */
/* ------------------------------------------------------------------ */

#if ARITH_BOR + 11 != ARITH_BORASS || ARITH_ASS + 11 != ARITH_EQ
#error Arithmetic tokens are out of order.
#endif

const char *arith_startbuf;

const char *arith_buf;
union yystype yylval;

int last_token;

/*
 * The original writes this table with array designators:
 *	#define ARITH_PRECEDENCE(op, prec) [op - ARITH_BINOP_MIN] = prec
 * which C++ does not have.  The initialisers below are the same sixteen
 * values in index order:
 *	 0 ARITH_LE 3	 1 ARITH_GE 3	 2 ARITH_LT 3	 3 ARITH_GT 3
 *	 4 ARITH_EQ 4	 5 ARITH_REM 0	 6 ARITH_BAND 5	 7 ARITH_LSHIFT 2
 *	 8 ARITH_RSHIFT 2  9 ARITH_MUL 0  10 ARITH_ADD 1  11 ARITH_BOR 7
 *	12 ARITH_SUB 1	13 ARITH_BXOR 6	14 ARITH_DIV 0	15 ARITH_NE 4
 */
extern const char prec[ARITH_BINOP_MAX - ARITH_BINOP_MIN] = {
	3, 3, 3, 3, 4, 0, 5, 2, 2, 0, 1, 7, 1, 6, 0, 4,
};

#define ARITH_MAX_PREC 8

int letcmd(int, char **);

[[noreturn]] void yyerror(const char *s)
{
	error("arithmetic expression: %s: \"%s\"", s, arith_startbuf);
	/* NOTREACHED */
}

arith_t arith_lookupvarint(char *varname)
{
	const char *str;
	char *p;
	arith_t result;

	str = lookupvar(varname);
	if (uflag && str == NULL)
		yyerror("variable not set");
	if (str == NULL || *str == '\0')
		str = "0";
	errno = 0;
	result = strtoarith_t(str, &p);
	if (errno != 0 || *p != '\0')
		yyerror("variable conversion error");
	return result;
}

int arith_prec(int op)
{
	return prec[op - ARITH_BINOP_MIN];
}

int higher_prec(int op1, int op2)
{
	return arith_prec(op1) < arith_prec(op2);
}

arith_t do_binop(int op, arith_t a, arith_t b)
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

arith_t assignment(int var, int noeval);

arith_t primary(int token, union yystype *val, int op, int noeval)
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

arith_t binop2(arith_t a, int op, int precedence, int noeval)
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

arith_t binop(int token, union yystype *val, int op, int noeval)
{
	arith_t a = primary(token, val, op, noeval);

	op = last_token;
	if (op < ARITH_BINOP_MIN || op >= ARITH_BINOP_MAX)
		return a;

	return binop2(a, op, ARITH_MAX_PREC, noeval);
}

arith_t arith_and(int token, union yystype *val, int op, int noeval)
{
	arith_t a = binop(token, val, op, noeval);
	arith_t b;

	op = last_token;
	if (op != ARITH_AND)
		return a;

	token = yylex();
	*val = yylval;

	b = arith_and(token, val, yylex(), noeval | !a);

	return a && b;
}

arith_t arith_or(int token, union yystype *val, int op, int noeval)
{
	arith_t a = arith_and(token, val, op, noeval);
	arith_t b;

	op = last_token;
	if (op != ARITH_OR)
		return a;

	token = yylex();
	*val = yylval;

	b = arith_or(token, val, yylex(), noeval | !!a);

	return a || b;
}

arith_t cond(int token, union yystype *val, int op, int noeval)
{
	arith_t a = arith_or(token, val, op, noeval);
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

arith_t assignment(int var, int noeval)
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

/* ------------------------------------------------------------------ */
/* mksyntax.c							      */
/* ------------------------------------------------------------------ */

/*
 * This program creates syntax.h and syntax.c.
 */

struct synclass {
	const char *name;
	const char *comment;
};

/* Syntax classes */
extern const struct synclass synclass[] = {
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
extern const struct synclass is_entry[] = {
	{ "ISDIGIT",	"a digit" },
	{ "ISUPPER",	"an upper case letter" },
	{ "ISLOWER",	"a lower case letter" },
	{ "ISUNDER",	"an underscore" },
	{ "ISSPECL",	"the name of a special parameter" },
	{ NULL, 	NULL }
};

extern const char writer[] = "\
/*\n\
 * This file was generated by the mksyntax program.\n\
 */\n\
\n";


FILE *cfile;
FILE *hfile;

void add_default(void);
void finish(void);
void init(const char *);
void add(const char *, const char *);
void output_type_macros(void);

int
mksyntax_main(int argc __unused, char **argv __unused)
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

void
init(const char *name)
{
	fprintf(hfile, "extern const char %s[];\n", name);
	fprintf(cfile, "const char %s[SYNBASE + CHAR_MAX + 1] = {\n", name);
}


void
add_one(const char *key, const char *type)
{
	fprintf(cfile, "\t[SYNBASE + %s] = %s,\n", key, type);
}


/*
 * Add default values to the syntax table.
 */

void
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

void
finish(void)
{
	fputs("};\n", cfile);
}


/*
 * Add entries to the syntax table.
 */

void
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

const char *macro[] = {
	"#define is_digit(c)\t((unsigned int)((c) - '0') <= 9)",
	"#define is_eof(c)\t((c) == PEOF)",
	"#define is_alpha(c)\t((is_type+SYNBASE)[(int)c] & (ISUPPER|ISLOWER))",
	"#define is_name(c)\t((is_type+SYNBASE)[(int)c] & (ISUPPER|ISLOWER|ISUNDER))",
	"#define is_in_name(c)\t((is_type+SYNBASE)[(int)c] & (ISUPPER|ISLOWER|ISUNDER|ISDIGIT))",
	"#define is_special(c)\t((is_type+SYNBASE)[(int)c] & (ISSPECL|ISDIGIT))",
	"#define digit_val(c)\t((c) - '0')",
	NULL
};

void
output_type_macros(void)
{
	const char **pp;

	for (pp = macro ; *pp ; pp++)
		fprintf(hfile, "%s\n", *pp);
}

} /* namespace pbsd::bin_sh::b0228 */
