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
 * oracle.c -- reference implementation of mknodes.c for batch b0230.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef ALIGNBYTES
#define ALIGNBYTES (sizeof(long) - 1)
#endif
#ifndef ALIGN
#define ALIGN(p) (((unsigned long)(p) + ALIGNBYTES) & ~ALIGNBYTES)
#endif

#define __printf0like(...)
#define __dead2

#define MAXTYPES 50
#define MAXFIELDS 20
#define BUFLEN 100

#define T_NODE 1
#define T_NODELIST 2
#define T_STRING 3
#define T_INT 4
#define T_OTHER 5
#define T_TEMP 6

struct field {
	char *name;
	int type;
	char *decl;
};

struct str {
	char *tag;
	int nfields;
	struct field field[MAXFIELDS];
	int done;
};

static int ntypes;
static char *nodename[MAXTYPES];
static struct str *nodestr[MAXTYPES];
static int nstr;
static struct str str[MAXTYPES];
static struct str *curstr;
static char line[1024];
static int linno;
static char *linep;

#define savestr ref_mknodes_savestr
#define error ref_mknodes_error
#define main ref_mknodes_main
#define parsenode ref_parsenode
#define parsefield ref_parsefield
#define output ref_mknodes_output
#define outsizes ref_outsizes
#define outfunc ref_outfunc
#define indent ref_indent
#define nextfield ref_nextfield
#define skipbl ref_skipbl
#define readline ref_readline

void ref_parsenode(void);
void ref_parsefield(void);
void ref_mknodes_output(char *);
void ref_outsizes(FILE *);
void ref_outfunc(FILE *, int);
void ref_indent(int, FILE *);
int ref_nextfield(char *);
void ref_skipbl(void);
int ref_readline(FILE *);
void ref_mknodes_error(const char *, ...) __printf0like(1, 2) __dead2;
char *ref_mknodes_savestr(const char *);

int
ref_mknodes_main(int argc, char *argv[])
{
	FILE *infp;

	if (argc != 3)
		error("usage: mknodes file");
	if ((infp = fopen(argv[1], "r")) == NULL)
		error("Can't open %s: %s", argv[1], strerror(errno));
	while (readline(infp)) {
		if (line[0] == ' ' || line[0] == '\t')
			parsefield();
		else if (line[0] != '\0')
			parsenode();
	}
	fclose(infp);
	output(argv[2]);
	exit(0);
}

void
ref_parsenode(void)
{
	char name[BUFLEN];
	char tag[BUFLEN];
	struct str *sp;

	if (curstr && curstr->nfields > 0)
		curstr->done = 1;
	nextfield(name);
	if (! nextfield(tag))
		error("Tag expected");
	if (*linep != '\0')
		error("Garbage at end of line");
	nodename[ntypes] = savestr(name);
	for (sp = str ; sp < str + nstr ; sp++) {
		if (strcmp(sp->tag, tag) == 0)
			break;
	}
	if (sp >= str + nstr) {
		sp->tag = savestr(tag);
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
		error("No current structure to add field to");
	if (! nextfield(name))
		error("No field name");
	if (! nextfield(type))
		error("No field type");
	fp = &curstr->field[curstr->nfields];
	fp->name = savestr(name);
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
		error("Unknown type %s", type);
	}
	if (fp->type == T_OTHER || fp->type == T_TEMP) {
		skipbl();
		fp->decl = savestr(linep);
	} else {
		if (*linep)
			error("Garbage at end of line");
		fp->decl = savestr(decl);
	}
	curstr->nfields++;
}

static const char writer[] = "\
/*\n\
 * This file was generated by the mknodes program.\n\
 */\n\
\n";

void
ref_mknodes_output(char *file)
{
	FILE *hfile;
	FILE *cfile;
	FILE *patfile;
	int i;
	struct str *sp;
	struct field *fp;
	char *p;

	if ((patfile = fopen(file, "r")) == NULL)
		error("Can't open %s: %s", file, strerror(errno));
	if ((hfile = fopen("nodes.h", "w")) == NULL)
		error("Can't create nodes.h: %s", strerror(errno));
	if ((cfile = fopen("nodes.c", "w")) == NULL)
		error("Can't create nodes.c");
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
		error("Can't write to nodes.h");
	if (fclose(hfile))
		error("Can't close nodes.h");

	fputs(writer, cfile);
	while (fgets(line, sizeof line, patfile) != NULL) {
		for (p = line ; *p == ' ' || *p == '\t' ; p++);
		if (strcmp(p, "%SIZES\n") == 0)
			outsizes(cfile);
		else if (strcmp(p, "%CALCSIZE\n") == 0)
			outfunc(cfile, 1);
		else if (strcmp(p, "%COPY\n") == 0)
			outfunc(cfile, 0);
		else
			fputs(line, cfile);
	}
	fclose(patfile);
	if (ferror(cfile))
		error("Can't write to nodes.c");
	if (fclose(cfile))
		error("Can't close nodes.c");
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
					indent(12, cfile);
					fprintf(cfile, "calcsize(n->%s.%s, result);\n",
						sp->tag, fp->name);
				} else {
					indent(12, cfile);
					fprintf(cfile, "new->%s.%s = copynode(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_NODELIST:
				if (calcsize) {
					indent(12, cfile);
					fprintf(cfile, "sizenodelist(n->%s.%s, result);\n",
						sp->tag, fp->name);
				} else {
					indent(12, cfile);
					fprintf(cfile, "new->%s.%s = copynodelist(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_STRING:
				if (calcsize) {
					indent(12, cfile);
					fprintf(cfile, "result->stringsize += strlen(n->%s.%s) + 1;\n",
						sp->tag, fp->name);
				} else {
					indent(12, cfile);
					fprintf(cfile, "new->%s.%s = nodesavestr(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_INT:
			case T_OTHER:
				if (! calcsize) {
					indent(12, cfile);
					fprintf(cfile, "new->%s.%s = n->%s.%s;\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			}
		}
		indent(12, cfile);
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
		error("Line too long");
	return 1;
}

void
ref_mknodes_error(const char *msg, ...)
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
ref_mknodes_savestr(const char *s)
{
	char *p;

	if ((p = malloc(strlen(s) + 1)) == NULL)
		error("Out of space");
	(void) strcpy(p, s);
	return p;
}

void oracle_mknodes_reset(void)
{
	int i, j;

	for (i = 0; i < ntypes; i++) {
		free(nodename[i]);
		nodename[i] = NULL;
	}
	for (i = 0; i < nstr; i++) {
		free(str[i].tag);
		str[i].tag = NULL;
		for (j = 0; j < str[i].nfields; j++) {
			free(str[i].field[j].name);
			free(str[i].field[j].decl);
			str[i].field[j].name = NULL;
			str[i].field[j].decl = NULL;
		}
		str[i].nfields = 0;
		str[i].done = 0;
	}
	ntypes = 0;
	nstr = 0;
	curstr = NULL;
	linno = 0;
	line[0] = '\0';
	linep = line;
}

void oracle_mknodes_set_line(const char *s)
{
	strncpy(line, s, sizeof(line) - 1);
	line[sizeof(line) - 1] = '\0';
	linep = line;
}

int oracle_mknodes_get_linno(void) { return linno; }
