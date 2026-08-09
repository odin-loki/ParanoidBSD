/*
 * oracle.c -- PBSD batch b0230 reference oracle.
 *
 * The original HardenedBSD C sources for the functions covered by this batch,
 * concatenated.  Every function carries a "ref_" prefix; the bodies are
 * otherwise unmodified.  Globals that the differential harness has to observe
 * had their "static" storage class removed; nothing else was touched.
 *
 * Sources:
 *   hbsd/src/bin/sh/cd.c
 *   hbsd/src/bin/sh/mknodes.c
 *   hbsd/src/bin/sh/trap.c
 *   hbsd/src/bin/sh/input.c
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>

/* Defines the BSD sources get from <sys/cdefs.h> / <limits.h>. */
#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif
#ifndef NSIG
#define NSIG		65
#endif
#ifndef __unused
#define __unused
#endif
#ifndef __dead2
#define __dead2
#endif
#ifndef __printf0like
#define __printf0like(a, b)
#endif

/*
 * ---------------------------------------------------------------------------
 * Test dependencies.  These are NOT part of the batch; they stand in for
 * symbols that live in other translation units of bin/sh (mystring.c,
 * output.c) or in libc on FreeBSD.  Both the oracle and the C++23 port link
 * against these exact objects, so they can never be a source of divergence.
 * ---------------------------------------------------------------------------
 */

/* syntax.h */
#define is_digit(c)	((unsigned)((c) - '0') <= 9)

/* mystring.c */
int
is_number(const char *p)
{
	do {
		if (! is_digit(*p))
			return 0;
	} while (*++p != '\0');
	return 1;
}

/* libc signal name table, made writable so the harness can vary it. */
const char *dep_signame[128];
const char **sys_signame = dep_signame;
int sys_nsig = 0;

/* output.c: capture out1 into a buffer the harness can inspect. */
char shim_out1[262144];
size_t shim_out1len = 0;

static void
shim_put(const char *s, size_t n)
{
	if (shim_out1len + n <= sizeof shim_out1) {
		memcpy(shim_out1 + shim_out1len, s, n);
		shim_out1len += n;
	}
}

void
out1str(const char *s)
{
	shim_put(s, strlen(s));
}

void
out1c(int c)
{
	char ch = (char)c;

	shim_put(&ch, 1);
}

void
out1fmt(const char *fmt, ...)
{
	char b[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(b, sizeof b, fmt, ap);
	va_end(ap);
	shim_put(b, strlen(b));
}

/* Lets the harness assert that the port saw the same NSIG. */
const int ref_nsig_value = NSIG;

/*
 * ---------------------------------------------------------------------------
 * bin/sh/cd.c
 * ---------------------------------------------------------------------------
 */

/*-
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
 * Get the next component of the path name pointed to by *path.
 * This routine overwrites *path and the string pointed to by it.
 */
char *
ref_getcomponent(char **path)
{
	char *p;
	char *start;

	if ((p = *path) == NULL)
		return NULL;
	start = *path;
	while (*p != '/' && *p != '\0')
		p++;
	if (*p == '\0') {
		*path = NULL;
	} else {
		*p++ = '\0';
		*path = p;
	}
	return start;
}

/*
 * ---------------------------------------------------------------------------
 * bin/sh/mknodes.c
 * ---------------------------------------------------------------------------
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

/*
 * This program reads the nodetypes file and nodes.c.pat file.  It generates
 * the files nodes.h and nodes.c.
 */

#define MAXTYPES 50		/* max number of node types */
#define MAXFIELDS 20		/* max fields in a structure */
#define BUFLEN 100		/* size of character buffers */

/* field types */
#define T_NODE 1		/* union node *field */
#define T_NODELIST 2		/* struct nodelist *field */
#define T_STRING 3
#define T_INT 4			/* int field */
#define T_OTHER 5		/* other */
#define T_TEMP 6		/* don't copy this field */


struct field {			/* a structure field */
	char *name;		/* name of field */
	int type;			/* type of field */
	char *decl;		/* declaration of field */
};


struct str {			/* struct representing a node structure */
	char *tag;		/* structure tag */
	int nfields;		/* number of fields in the structure */
	struct field field[MAXFIELDS];	/* the fields of the structure */
	int done;			/* set if fully parsed */
};


int ntypes;			/* number of node types */
char *nodename[MAXTYPES];	/* names of the nodes */
struct str *nodestr[MAXTYPES];	/* type of structure used by the node */
int nstr;			/* number of structures */
struct str str[MAXTYPES];	/* the structures */
struct str *curstr;		/* current structure */
char line[1024];
int linno;
char *linep;

void ref_parsenode(void);
void ref_parsefield(void);
void ref_output(char *);
void ref_outsizes(FILE *);
void ref_outfunc(FILE *, int);
void ref_indent(int, FILE *);
int ref_nextfield(char *);
void ref_skipbl(void);
int ref_readline(FILE *);
void ref_error(const char *, ...) __printf0like(1, 2) __dead2;
char *ref_savestr(const char *);


void
ref_parsenode(void)
{
	char name[BUFLEN];
	char tag[BUFLEN];
	struct str *sp;

	if (curstr && curstr->nfields > 0)
		curstr->done = 1;
	ref_nextfield(name);
	if (! ref_nextfield(tag))
		ref_error("Tag expected");
	if (*linep != '\0')
		ref_error("Garbage at end of line");
	nodename[ntypes] = ref_savestr(name);
	for (sp = str ; sp < str + nstr ; sp++) {
		if (strcmp(sp->tag, tag) == 0)
			break;
	}
	if (sp >= str + nstr) {
		sp->tag = ref_savestr(tag);
		sp->nfields = 0;
		curstr = sp;
		nstr++;
	}
	nodestr[ntypes] = sp;
	ntypes++;
}


void
ref_parsefield(void)
{
	char name[BUFLEN];
	char type[BUFLEN];
	char decl[2 * BUFLEN];
	struct field *fp;

	if (curstr == NULL || curstr->done)
		ref_error("No current structure to add field to");
	if (! ref_nextfield(name))
		ref_error("No field name");
	if (! ref_nextfield(type))
		ref_error("No field type");
	fp = &curstr->field[curstr->nfields];
	fp->name = ref_savestr(name);
	if (strcmp(type, "nodeptr") == 0) {
		fp->type = T_NODE;
		sprintf(decl, "union node *%s", name);
	} else if (strcmp(type, "nodelist") == 0) {
		fp->type = T_NODELIST;
		sprintf(decl, "struct nodelist *%s", name);
	} else if (strcmp(type, "string") == 0) {
		fp->type = T_STRING;
		sprintf(decl, "char *%s", name);
	} else if (strcmp(type, "int") == 0) {
		fp->type = T_INT;
		sprintf(decl, "int %s", name);
	} else if (strcmp(type, "other") == 0) {
		fp->type = T_OTHER;
	} else if (strcmp(type, "temp") == 0) {
		fp->type = T_TEMP;
	} else {
		ref_error("Unknown type %s", type);
	}
	if (fp->type == T_OTHER || fp->type == T_TEMP) {
		ref_skipbl();
		fp->decl = ref_savestr(linep);
	} else {
		if (*linep)
			ref_error("Garbage at end of line");
		fp->decl = ref_savestr(decl);
	}
	curstr->nfields++;
}


static const char writer[] = "\
/*\n\
 * This file was generated by the mknodes program.\n\
 */\n\
\n";

void
ref_output(char *file)
{
	FILE *hfile;
	FILE *cfile;
	FILE *patfile;
	int i;
	struct str *sp;
	struct field *fp;
	char *p;

	if ((patfile = fopen(file, "r")) == NULL)
		ref_error("Can't open %s: %s", file, strerror(errno));
	if ((hfile = fopen("nodes.h", "w")) == NULL)
		ref_error("Can't create nodes.h: %s", strerror(errno));
	if ((cfile = fopen("nodes.c", "w")) == NULL)
		ref_error("Can't create nodes.c");
	fputs(writer, hfile);
	for (i = 0 ; i < ntypes ; i++)
		fprintf(hfile, "#define %s %d\n", nodename[i], i);
	fputs("\n\n\n", hfile);
	for (sp = str ; sp < &str[nstr] ; sp++) {
		fprintf(hfile, "struct %s {\n", sp->tag);
		for (i = sp->nfields, fp = sp->field ; --i >= 0 ; fp++) {
			fprintf(hfile, "      %s;\n", fp->decl);
		}
		fputs("};\n\n\n", hfile);
	}
	fputs("union node {\n", hfile);
	fprintf(hfile, "      int type;\n");
	for (sp = str ; sp < &str[nstr] ; sp++) {
		fprintf(hfile, "      struct %s %s;\n", sp->tag, sp->tag);
	}
	fputs("};\n\n\n", hfile);
	fputs("struct nodelist {\n", hfile);
	fputs("\tstruct nodelist *next;\n", hfile);
	fputs("\tunion node *n;\n", hfile);
	fputs("};\n\n\n", hfile);
	fputs("struct funcdef;\n", hfile);
	fputs("struct funcdef *copyfunc(union node *);\n", hfile);
	fputs("union node *getfuncnode(struct funcdef *);\n", hfile);
	fputs("void reffunc(struct funcdef *);\n", hfile);
	fputs("void unreffunc(struct funcdef *);\n", hfile);
	if (ferror(hfile))
		ref_error("Can't write to nodes.h");
	if (fclose(hfile))
		ref_error("Can't close nodes.h");

	fputs(writer, cfile);
	while (fgets(line, sizeof line, patfile) != NULL) {
		for (p = line ; *p == ' ' || *p == '\t' ; p++);
		if (strcmp(p, "%SIZES\n") == 0)
			ref_outsizes(cfile);
		else if (strcmp(p, "%CALCSIZE\n") == 0)
			ref_outfunc(cfile, 1);
		else if (strcmp(p, "%COPY\n") == 0)
			ref_outfunc(cfile, 0);
		else
			fputs(line, cfile);
	}
	fclose(patfile);
	if (ferror(cfile))
		ref_error("Can't write to nodes.c");
	if (fclose(cfile))
		ref_error("Can't close nodes.c");
}



void
ref_outsizes(FILE *cfile)
{
	int i;

	fprintf(cfile, "static const short nodesize[%d] = {\n", ntypes);
	for (i = 0 ; i < ntypes ; i++) {
		fprintf(cfile, "      ALIGN(sizeof (struct %s)),\n", nodestr[i]->tag);
	}
	fprintf(cfile, "};\n");
}


void
ref_outfunc(FILE *cfile, int calcsize)
{
	struct str *sp;
	struct field *fp;
	int i;

	fputs("      if (n == NULL)\n", cfile);
	if (calcsize)
		fputs("	    return;\n", cfile);
	else
		fputs("	    return NULL;\n", cfile);
	if (calcsize)
		fputs("      result->blocksize += nodesize[n->type];\n", cfile);
	else {
		fputs("      new = state->block;\n", cfile);
		fputs("      state->block = (char *)state->block + nodesize[n->type];\n", cfile);
	}
	fputs("      switch (n->type) {\n", cfile);
	for (sp = str ; sp < &str[nstr] ; sp++) {
		for (i = 0 ; i < ntypes ; i++) {
			if (nodestr[i] == sp)
				fprintf(cfile, "      case %s:\n", nodename[i]);
		}
		for (i = sp->nfields ; --i >= 1 ; ) {
			fp = &sp->field[i];
			switch (fp->type) {
			case T_NODE:
				if (calcsize) {
					ref_indent(12, cfile);
					fprintf(cfile, "calcsize(n->%s.%s, result);\n",
						sp->tag, fp->name);
				} else {
					ref_indent(12, cfile);
					fprintf(cfile, "new->%s.%s = copynode(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_NODELIST:
				if (calcsize) {
					ref_indent(12, cfile);
					fprintf(cfile, "sizenodelist(n->%s.%s, result);\n",
						sp->tag, fp->name);
				} else {
					ref_indent(12, cfile);
					fprintf(cfile, "new->%s.%s = copynodelist(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_STRING:
				if (calcsize) {
					ref_indent(12, cfile);
					fprintf(cfile, "result->stringsize += strlen(n->%s.%s) + 1;\n",
						sp->tag, fp->name);
				} else {
					ref_indent(12, cfile);
					fprintf(cfile, "new->%s.%s = nodesavestr(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_INT:
			case T_OTHER:
				if (! calcsize) {
					ref_indent(12, cfile);
					fprintf(cfile, "new->%s.%s = n->%s.%s;\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			}
		}
		ref_indent(12, cfile);
		fputs("break;\n", cfile);
	}
	fputs("      };\n", cfile);
	if (! calcsize)
		fputs("      new->type = n->type;\n", cfile);
}


void
ref_indent(int amount, FILE *fp)
{
	while (amount >= 8) {
		putc('\t', fp);
		amount -= 8;
	}
	while (--amount >= 0) {
		putc(' ', fp);
	}
}


int
ref_nextfield(char *buf)
{
	char *p, *q;

	p = linep;
	while (*p == ' ' || *p == '\t')
		p++;
	q = buf;
	while (*p != ' ' && *p != '\t' && *p != '\0')
		*q++ = *p++;
	*q = '\0';
	linep = p;
	return (q > buf);
}


void
ref_skipbl(void)
{
	while (*linep == ' ' || *linep == '\t')
		linep++;
}


int
ref_readline(FILE *infp)
{
	char *p;

	if (fgets(line, 1024, infp) == NULL)
		return 0;
	for (p = line ; *p != '#' && *p != '\n' && *p != '\0' ; p++);
	while (p > line && (p[-1] == ' ' || p[-1] == '\t'))
		p--;
	*p = '\0';
	linep = line;
	linno++;
	if (p - line > BUFLEN)
		ref_error("Line too long");
	return 1;
}



void
ref_error(const char *msg, ...)
{
	va_list va;
	va_start(va, msg);

	(void) fprintf(stderr, "line %d: ", linno);
	(void) vfprintf(stderr, msg, va);
	(void) fputc('\n', stderr);

	va_end(va);

	exit(2);
}



char *
ref_savestr(const char *s)
{
	char *p;

	if ((p = malloc(strlen(s) + 1)) == NULL)
		ref_error("Out of space");
	(void) strcpy(p, s);
	return p;
}

/*
 * ---------------------------------------------------------------------------
 * bin/sh/trap.c
 * ---------------------------------------------------------------------------
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

char *volatile trap[NSIG];	/* trap handler commands */

/*
 * Map a string to a signal number.
 *
 * Note: the signal number may exceed NSIG.
 */
int
ref_sigstring_to_signum(char *sig)
{

	if (is_number(sig)) {
		int signo;

		signo = atoi(sig);
		return ((signo >= 0 && signo < NSIG) ? signo : (-1));
	} else if (strcasecmp(sig, "EXIT") == 0) {
		return (0);
	} else {
		int n;

		if (strncasecmp(sig, "SIG", 3) == 0)
			sig += 3;
		for (n = 1; n < sys_nsig; n++)
			if (sys_signame[n] &&
			    strcasecmp(sys_signame[n], sig) == 0)
				return (n);
	}
	return (-1);
}


/*
 * Print a list of valid signal names.
 */
void
ref_printsignals(void)
{
	int n, outlen;

	outlen = 0;
	for (n = 1; n < sys_nsig; n++) {
		if (sys_signame[n]) {
			out1fmt("%s", sys_signame[n]);
			outlen += strlen(sys_signame[n]);
		} else {
			out1fmt("%d", n);
			outlen += 3;	/* good enough */
		}
		++outlen;
		if (outlen > 71 || n == sys_nsig - 1) {
			out1str("\n");
			outlen = 0;
		} else {
			out1c(' ');
		}
	}
}


/*
 * Check if we have any traps enabled.
 */
int
ref_have_traps(void)
{
	char *volatile *tp;

	for (tp = trap ; tp <= &trap[NSIG - 1] ; tp++) {
		if (*tp && **tp)	/* trap not NULL or SIG_IGN */
			return 1;
	}
	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * bin/sh/input.c
 * ---------------------------------------------------------------------------
 */

/*-
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

#define EOF_NLEFT -99		/* value of parsenleft when EOF pushed back */

struct alias;

struct strpush {
	struct strpush *prev;	/* preceding string on stack */
	const char *prevstring;
	int prevnleft;
	int prevlleft;
	struct alias *ap;	/* if push was associated with an alias */
};

/*
 * The parsefile structure pointed to by the global variable parsefile
 * contains information about the current file being read.
 */

struct parsefile {
	struct parsefile *prev;	/* preceding file on stack */
	int linno;		/* current line */
	int fd;			/* file descriptor (or -1 if string) */
	int nleft;		/* number of chars left in this line */
	int lleft;		/* number of lines left in this buffer */
	const char *nextc;	/* next char in buffer */
	char *buf;		/* input buffer */
	struct strpush *strpush; /* for pushing strings at this level */
	struct strpush basestrpush; /* so pushing one is fast */
};


int plinno = 1;			/* input line number */
int parsenleft;			/* copy of parsefile->nleft */
int parselleft;			/* copy of parsefile->lleft */
const char *parsenextc;		/* copy of parsefile->nextc */
char basebuf[BUFSIZ + 1];/* buffer for top level input file */
struct parsefile basepf = {	/* top level input file */
	.nextc = basebuf,
	.buf = basebuf
};
struct parsefile *parsefile = &basepf;	/* current input file */

/*
 * Returns if we are certain we are at EOF. Does not cause any more input
 * to be read from the outside world.
 */

int
ref_preadateof(void)
{
	if (parsenleft > 0)
		return 0;
	if (parsefile->strpush)
		return 0;
	if (parsenleft == EOF_NLEFT || parsefile->buf == NULL)
		return 1;
	return 0;
}

/*
 * Undo the last call to pgetc.  Only one character may be pushed back.
 * PEOF may be pushed back.
 */

void
ref_pungetc(void)
{
	parsenleft++;
	parsenextc--;
}

/*
 * Harness helpers (not in the original sources).
 */
void
oracle_mknodes_reset(void)
{
	int i, j;

	for (i = 0; i < ntypes; i++)
		free(nodename[i]);
	for (i = 0; i < nstr; i++) {
		free(str[i].tag);
		for (j = 0; j < str[i].nfields; j++) {
			free(str[i].field[j].name);
			free(str[i].field[j].decl);
		}
	}
	ntypes = 0;
	nstr = 0;
	curstr = NULL;
	linno = 0;
	line[0] = '\0';
	linep = line;
}

void
oracle_mknodes_set_line(const char *s)
{
	(void)strncpy(line, s, sizeof(line) - 1);
	line[sizeof(line) - 1] = '\0';
	linep = line;
}

int
oracle_mknodes_get_linno(void)
{
	return linno;
}

char *
ref_mknodes_savestr(const char *s)
{
	return ref_savestr(s);
}

void
ref_mknodes_output(char *file)
{
	ref_output(file);
}

void
ref_mknodes_read_input(FILE *infp)
{
	while (ref_readline(infp)) {
		if (line[0] == ' ' || line[0] == '\t')
			ref_parsefield();
		else if (line[0] != '\0')
			ref_parsenode();
	}
}

int
ref_mknodes_main(int argc, char **argv)
{
	FILE *infp;

	if (argc != 3)
		ref_error("usage: mknodes file");
	if ((infp = fopen(argv[1], "r")) == NULL)
		ref_error("Can't open %s: %s", argv[1], strerror(errno));
	ref_mknodes_read_input(infp);
	fclose(infp);
	ref_output(argv[2]);
	exit(0);
}

void
oracle_out1_reset(void)
{
	shim_out1len = 0;
	shim_out1[0] = '\0';
}

const char *
oracle_out1_get(void)
{
	return shim_out1;
}

size_t
oracle_out1_len(void)
{
	return shim_out1len;
}

void
oracle_input_reset(void)
{
	parsefile = &basepf;
	parsenleft = 0;
	parselleft = 0;
	parsenextc = basebuf;
	plinno = 1;
	basepf.prev = NULL;
	basepf.linno = 1;
	basepf.fd = 0;
	basepf.nleft = 0;
	basepf.lleft = 0;
	basepf.nextc = basebuf;
	basepf.buf = basebuf;
	basepf.strpush = NULL;
	basepf.basestrpush.prev = NULL;
	basepf.basestrpush.ap = NULL;
}

void
oracle_input_set_parsenleft(int n)
{
	parsenleft = n;
}

void
oracle_input_set_parselleft(int n)
{
	parselleft = n;
}

void
oracle_input_set_nextc(const char *s)
{
	parsenextc = s;
}

void
oracle_input_set_buf_null(int null)
{
	parsefile->buf = null ? NULL : basebuf;
}

void
oracle_input_set_strpush(int on)
{
	if (on)
		parsefile->strpush = &basepf.basestrpush;
	else
		parsefile->strpush = NULL;
}

void
oracle_trap_reset(void)
{
	int i;

	for (i = 0; i < NSIG; i++) {
		free(trap[i]);
		trap[i] = NULL;
	}
}

void
oracle_trap_set(int signo, const char *cmd)
{
	free(trap[signo]);
	trap[signo] = cmd ? ref_savestr(cmd) : NULL;
}
