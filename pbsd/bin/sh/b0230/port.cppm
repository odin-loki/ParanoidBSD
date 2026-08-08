module;

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef ALIGNBYTES
#define ALIGNBYTES (sizeof(long) - 1)
#endif
#ifndef ALIGN
#define ALIGN(p) (((unsigned long)(p) + ALIGNBYTES) & ~ALIGNBYTES)
#endif

export module pbsd.bin.sh.b0230;

export namespace pbsd::bin_sh::b0230 {

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

void parsenode(void);
void parsefield(void);
void mknodes_output(char *);
void outsizes(FILE *);
void outfunc(FILE *, int);
void indent(int, FILE *);
int nextfield(char *);
void skipbl(void);
int readline(FILE *);
void mknodes_error(const char *, ...) __printf0like(1, 2) __dead2;
char *mknodes_savestr(const char *);

#define savestr mknodes_savestr
#define error mknodes_error
#define output mknodes_output

int
mknodes_main(int argc, char *argv[])
{
	FILE *infp;

	if (argc != 3)
		error("usage: mknodes file");
	if ((infp = std::fopen(argv[1], "r")) == NULL)
		error("Can't open %s: %s", argv[1], strerror(errno));
	while (readline(infp)) {
		if (line[0] == ' ' || line[0] == '\t')
			parsefield();
		else if (line[0] != '\0')
			parsenode();
	}
	std::fclose(infp);
	output(argv[2]);
	std::exit(0);
}

void
parsenode(void)
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
		if (std::strcmp(sp->tag, tag) == 0)
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
parsefield(void)
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
	if (std::strcmp(type, "nodeptr") == 0) {
		fp->type = T_NODE;
		std::sprintf(decl, "union node *%s", name);
	} else if (std::strcmp(type, "nodelist") == 0) {
		fp->type = T_NODELIST;
		std::sprintf(decl, "struct nodelist *%s", name);
	} else if (std::strcmp(type, "string") == 0) {
		fp->type = T_STRING;
		std::sprintf(decl, "char *%s", name);
	} else if (std::strcmp(type, "int") == 0) {
		fp->type = T_INT;
		std::sprintf(decl, "int %s", name);
	} else if (std::strcmp(type, "other") == 0) {
		fp->type = T_OTHER;
	} else if (std::strcmp(type, "temp") == 0) {
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
mknodes_output(char *file)
{
	FILE *hfile;
	FILE *cfile;
	FILE *patfile;
	int i;
	struct str *sp;
	struct field *fp;
	char *p;

	if ((patfile = std::fopen(file, "r")) == NULL)
		error("Can't open %s: %s", file, strerror(errno));
	if ((hfile = std::fopen("nodes.h", "w")) == NULL)
		error("Can't create nodes.h: %s", strerror(errno));
	if ((cfile = std::fopen("nodes.c", "w")) == NULL)
		error("Can't create nodes.c");
	std::fputs(writer, hfile);
	for (i = 0 ; i < ntypes ; i++)
		std::fprintf(hfile, "#define %s %d\n", nodename[i], i);
	std::fputs("\n\n\n", hfile);
	for (sp = str ; sp < &str[nstr] ; sp++) {
		std::fprintf(hfile, "struct %s {\n", sp->tag);
		for (i = sp->nfields, fp = sp->field ; --i >= 0 ; fp++) {
			std::fprintf(hfile, "      %s;\n", fp->decl);
		}
		std::fputs("};\n\n\n", hfile);
	}
	std::fputs("union node {\n", hfile);
	std::fprintf(hfile, "      int type;\n");
	for (sp = str ; sp < &str[nstr] ; sp++) {
		std::fprintf(hfile, "      struct %s %s;\n", sp->tag, sp->tag);
	}
	std::fputs("};\n\n\n", hfile);
	std::fputs("struct nodelist {\n", hfile);
	std::fputs("\tstruct nodelist *next;\n", hfile);
	std::fputs("\tunion node *n;\n", hfile);
	std::fputs("};\n\n\n", hfile);
	std::fputs("struct funcdef;\n", hfile);
	std::fputs("struct funcdef *copyfunc(union node *);\n", hfile);
	std::fputs("union node *getfuncnode(struct funcdef *);\n", hfile);
	std::fputs("void reffunc(struct funcdef *);\n", hfile);
	std::fputs("void unreffunc(struct funcdef *);\n", hfile);
	if (std::ferror(hfile))
		error("Can't write to nodes.h");
	if (std::fclose(hfile))
		error("Can't close nodes.h");

	std::fputs(writer, cfile);
	while (std::fgets(line, sizeof line, patfile) != NULL) {
		for (p = line ; *p == ' ' || *p == '\t' ; p++);
		if (std::strcmp(p, "%SIZES\n") == 0)
			outsizes(cfile);
		else if (std::strcmp(p, "%CALCSIZE\n") == 0)
			outfunc(cfile, 1);
		else if (std::strcmp(p, "%COPY\n") == 0)
			outfunc(cfile, 0);
		else
			std::fputs(line, cfile);
	}
	std::fclose(patfile);
	if (std::ferror(cfile))
		error("Can't write to nodes.c");
	if (std::fclose(cfile))
		error("Can't close nodes.c");
}

void
outsizes(FILE *cfile)
{
	int i;

	std::fprintf(cfile, "static const short nodesize[%d] = {\n", ntypes);
	for (i = 0 ; i < ntypes ; i++) {
		std::fprintf(cfile, "      ALIGN(sizeof (struct %s)),\n", nodestr[i]->tag);
	}
	std::fprintf(cfile, "};\n");
}

void
outfunc(FILE *cfile, int calcsize)
{
	struct str *sp;
	struct field *fp;
	int i;

	std::fputs("      if (n == NULL)\n", cfile);
	if (calcsize)
		std::fputs("	    return;\n", cfile);
	else
		std::fputs("	    return NULL;\n", cfile);
	if (calcsize)
		std::fputs("      result->blocksize += nodesize[n->type];\n", cfile);
	else {
		std::fputs("      new = state->block;\n", cfile);
		std::fputs("      state->block = (char *)state->block + nodesize[n->type];\n", cfile);
	}
	std::fputs("      switch (n->type) {\n", cfile);
	for (sp = str ; sp < &str[nstr] ; sp++) {
		for (i = 0 ; i < ntypes ; i++) {
			if (nodestr[i] == sp)
				std::fprintf(cfile, "      case %s:\n", nodename[i]);
		}
		for (i = sp->nfields ; --i >= 1 ; ) {
			fp = &sp->field[i];
			switch (fp->type) {
			case T_NODE:
				if (calcsize) {
					indent(12, cfile);
					std::fprintf(cfile, "calcsize(n->%s.%s, result);\n",
						sp->tag, fp->name);
				} else {
					indent(12, cfile);
					std::fprintf(cfile, "new->%s.%s = copynode(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_NODELIST:
				if (calcsize) {
					indent(12, cfile);
					std::fprintf(cfile, "sizenodelist(n->%s.%s, result);\n",
						sp->tag, fp->name);
				} else {
					indent(12, cfile);
					std::fprintf(cfile, "new->%s.%s = copynodelist(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_STRING:
				if (calcsize) {
					indent(12, cfile);
					std::fprintf(cfile, "result->stringsize += strlen(n->%s.%s) + 1;\n",
						sp->tag, fp->name);
				} else {
					indent(12, cfile);
					std::fprintf(cfile, "new->%s.%s = nodesavestr(n->%s.%s, state);\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			case T_INT:
			case T_OTHER:
				if (! calcsize) {
					indent(12, cfile);
					std::fprintf(cfile, "new->%s.%s = n->%s.%s;\n",
						sp->tag, fp->name, sp->tag, fp->name);
				}
				break;
			}
		}
		indent(12, cfile);
		std::fputs("break;\n", cfile);
	}
	std::fputs("      };\n", cfile);
	if (! calcsize)
		std::fputs("      new->type = n->type;\n", cfile);
}

void
indent(int amount, FILE *fp)
{
	while (amount >= 8) {
		std::putc('\t', fp);
		amount -= 8;
	}
	while (--amount >= 0) {
		std::putc(' ', fp);
	}
}

int
nextfield(char *buf)
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
skipbl(void)
{
	while (*linep == ' ' || *linep == '\t')
		linep++;
}

int
readline(FILE *infp)
{
	char *p;

	if (std::fgets(line, 1024, infp) == NULL)
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
mknodes_error(const char *msg, ...)
{
	va_list va;
	va_start(va, msg);

	(void) std::fprintf(stderr, "line %d: ", linno);
	(void) std::vfprintf(stderr, msg, va);
	(void) std::fputc('\n', stderr);

	va_end(va);

	std::exit(2);
}

char *
mknodes_savestr(const char *s)
{
	char *p;

	if ((p = (char *)std::malloc(std::strlen(s) + 1)) == NULL)
		error("Out of space");
	(void) std::strcpy(p, s);
	return p;
}

void port_mknodes_reset(void)
{
	int i;

	ntypes = 0;
	nstr = 0;
	curstr = NULL;
	linno = 0;
	line[0] = '\0';
	linep = line;
	for (i = 0; i < MAXTYPES; i++) {
		nodename[i] = NULL;
		nodestr[i] = NULL;
		str[i].tag = NULL;
		str[i].nfields = 0;
		str[i].done = 0;
	}
}

void port_mknodes_set_line(const char *s)
{
	std::strncpy(line, s, sizeof(line) - 1);
	line[sizeof(line) - 1] = '\0';
	linep = line;
}

int port_mknodes_get_linno(void) { return linno; }

} // namespace pbsd::bin_sh::b0230
