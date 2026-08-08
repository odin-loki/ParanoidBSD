module;

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

export module pbsd.lib.libc.db.test.b0203;

export namespace pbsd::lib_libc_db_test::b0203 {

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct {
	void *data;
	size_t size;
} DBT;

#define	R_CURSOR	1
#define	R_FIRST		3
#define	R_IAFTER	4
#define	R_IBEFORE	5
#define	R_LAST		6
#define	R_NEXT		7
#define	R_NOOVERWRITE	8
#define	R_PREV		9
#define	R_SETCURSOR	10

typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

typedef struct __db {
	DBTYPE type;
	int (*close)(struct __db *);
	int (*del)(const struct __db *, const DBT *, unsigned int);
	int (*get)(const struct __db *, const DBT *, DBT *, unsigned int);
	int (*put)(struct __db *, DBT *, const DBT *, unsigned int);
	int (*seq)(struct __db *, DBT *, DBT *, unsigned int);
	int (*sync)(struct __db *, unsigned int);
	void *internal;
	int (*fd)(const struct __db *);
} DB;

typedef struct {
	unsigned long flags;
	unsigned int cachesize;
	int maxkeypage;
	int minkeypage;
	unsigned int psize;
	int (*compare)(const DBT *, const DBT *);
	size_t (*prefix)(const DBT *, const DBT *);
	int lorder;
} BTREEINFO;

typedef struct {
	unsigned int bsize;
	unsigned int ffactor;
	unsigned int nelem;
	unsigned int cachesize;
	uint32_t (*hash)(const void *, size_t);
	int lorder;
} HASHINFO;

typedef struct {
	unsigned long flags;
	unsigned int cachesize;
	unsigned int psize;
	int lorder;
	size_t reclen;
	unsigned char bval;
	char *bfname;
} RECNOINFO;

enum S { COMMAND, COMPARE, GET, PUT, REMOVE, SEQ, SEQFLAG, KEY, DATA };

inline DBTYPE type{};
inline void *infop{nullptr};
inline unsigned long lineno{};
inline unsigned int flags{};
inline int ofd{STDOUT_FILENO};
inline DB *XXdbp{nullptr};
inline int XXlineno{};

#define	NOOVERWRITE	"put failed, would overwrite key\n"

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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

char *sflags(int);
void err(const char *, ...);

void compare(DBT *db1, DBT *db2)
{
	size_t len;
	unsigned char *p1, *p2;

	if (db1->size != db2->size)
		std::printf("compare failed: key->data len %lu != data len %lu\n",
		    db1->size, db2->size);

	len = MIN(db1->size, db2->size);
	for (p1 = (unsigned char *)db1->data, p2 = (unsigned char *)db2->data;
	    len--;)
		if (*p1++ != *p2++) {
			std::printf("compare failed at offset %d\n",
			    (int)(p1 - (unsigned char *)db1->data));
			break;
		}
}

void get(DB *dbp, DBT *kp)
{
	DBT data;

	switch (dbp->get(dbp, kp, &data, flags)) {
	case 0:
		(void)write(ofd, data.data, data.size);
		if (ofd == STDOUT_FILENO)
			(void)write(ofd, "\n", 1);
		break;
	case -1:
		err("line %lu: get: %s", lineno, strerror(errno));
	case 1:
#define	NOSUCHKEY	"get failed, no such key\n"
		if (ofd != STDOUT_FILENO)
			(void)write(ofd, NOSUCHKEY, sizeof(NOSUCHKEY) - 1);
		else
			(void)std::fprintf(stderr, "%d: %.*s: %s",
			    (int)lineno, (int)MIN(kp->size, 20),
			    (const char *)kp->data, NOSUCHKEY);
#undef	NOSUCHKEY
		break;
	}
}

void getdata(DB *dbp, DBT *kp, DBT *dp)
{
	switch (dbp->get(dbp, kp, dp, flags)) {
	case 0:
		return;
	case -1:
		err("line %lu: getdata: %s", lineno, strerror(errno));
	case 1:
		err("line %lu: getdata failed, no such key", lineno);
	}
}

void put(DB *dbp, DBT *kp, DBT *dp)
{
	switch (dbp->put(dbp, kp, dp, flags)) {
	case 0:
		break;
	case -1:
		err("line %lu: put: %s", lineno, strerror(errno));
	case 1:
		(void)write(ofd, NOOVERWRITE, sizeof(NOOVERWRITE) - 1);
		break;
	}
}

void rem(DB *dbp, DBT *kp)
{
	switch (dbp->del(dbp, kp, flags)) {
	case 0:
		break;
	case -1:
		err("line %lu: rem: %s", lineno, strerror(errno));
	case 1:
#define	NOSUCHKEY	"rem failed, no such key\n"
		if (ofd != STDOUT_FILENO)
			(void)write(ofd, NOSUCHKEY, sizeof(NOSUCHKEY) - 1);
		else if (flags != R_CURSOR)
			(void)std::fprintf(stderr, "%d: %.*s: %s",
			    (int)lineno, (int)MIN(kp->size, 20),
			    (const char *)kp->data, NOSUCHKEY);
		else
			(void)std::fprintf(stderr,
			    "%d: rem of cursor failed\n", (int)lineno);
#undef	NOSUCHKEY
		break;
	}
}

void synk(DB *dbp)
{
	switch (dbp->sync(dbp, flags)) {
	case 0:
		break;
	case -1:
		err("line %lu: synk: %s", lineno, strerror(errno));
	}
}

void seq(DB *dbp, DBT *kp)
{
	DBT data;

	switch (dbp->seq(dbp, kp, &data, flags)) {
	case 0:
		(void)write(ofd, data.data, data.size);
		if (ofd == STDOUT_FILENO)
			(void)write(ofd, "\n", 1);
		break;
	case -1:
		err("line %lu: seq: %s", lineno, strerror(errno));
	case 1:
#define	NOSUCHKEY	"seq failed, no such key\n"
		if (ofd != STDOUT_FILENO)
			(void)write(ofd, NOSUCHKEY, sizeof(NOSUCHKEY) - 1);
		else if (flags == R_CURSOR)
			(void)std::fprintf(stderr, "%d: %.*s: %s",
			    (int)lineno, (int)MIN(kp->size, 20),
			    (const char *)kp->data, NOSUCHKEY);
		else
			(void)std::fprintf(stderr,
			    "%d: seq (%s) failed\n", (int)lineno, sflags(flags));
#undef	NOSUCHKEY
		break;
	}
}

void dump(DB *dbp, int rev)
{
	DBT key, data;
	int seqflags, nflags;

	if (rev) {
		seqflags = R_LAST;
		nflags = R_PREV;
	} else {
		seqflags = R_FIRST;
		nflags = R_NEXT;
	}
	for (;; seqflags = nflags)
		switch (dbp->seq(dbp, &key, &data, seqflags)) {
		case 0:
			(void)write(ofd, data.data, data.size);
			if (ofd == STDOUT_FILENO)
				(void)write(ofd, "\n", 1);
			break;
		case 1:
			goto done;
		case -1:
			err("line %lu: (dump) seq: %s",
			    lineno, strerror(errno));
		}
done:	return;
}

unsigned int setflags(char *s)
{
	char *p;

	for (; isspace((unsigned char)*s); ++s);
	if (*s == '\n' || *s == '\0')
		return (0);
	if ((p = strchr(s, '\n')) != NULL)
		*p = '\0';
	if (!std::strcmp(s, "R_CURSOR"))		return (R_CURSOR);
	if (!std::strcmp(s, "R_FIRST"))		return (R_FIRST);
	if (!std::strcmp(s, "R_IAFTER")) 		return (R_IAFTER);
	if (!std::strcmp(s, "R_IBEFORE")) 		return (R_IBEFORE);
	if (!std::strcmp(s, "R_LAST")) 		return (R_LAST);
	if (!std::strcmp(s, "R_NEXT")) 		return (R_NEXT);
	if (!std::strcmp(s, "R_NOOVERWRITE"))	return (R_NOOVERWRITE);
	if (!std::strcmp(s, "R_PREV"))		return (R_PREV);
	if (!std::strcmp(s, "R_SETCURSOR"))		return (R_SETCURSOR);

	err("line %lu: %s: unknown flag", lineno, s);
}

char *sflags(int seqflags)
{
	switch (seqflags) {
	case R_CURSOR:		return ("R_CURSOR");
	case R_FIRST:		return ("R_FIRST");
	case R_IAFTER:		return ("R_IAFTER");
	case R_IBEFORE:		return ("R_IBEFORE");
	case R_LAST:		return ("R_LAST");
	case R_NEXT:		return ("R_NEXT");
	case R_NOOVERWRITE:	return ("R_NOOVERWRITE");
	case R_PREV:		return ("R_PREV");
	case R_SETCURSOR:	return ("R_SETCURSOR");
	}

	return ("UNKNOWN!");
}

DBTYPE dbtype(char *s)
{
	if (!std::strcmp(s, "btree"))
		return (DB_BTREE);
	if (!std::strcmp(s, "hash"))
		return (DB_HASH);
	if (!std::strcmp(s, "recno"))
		return (DB_RECNO);
	err("%s: unknown type (use btree, hash or recno)", s);
}

void *setinfo(DBTYPE dbtype_arg, char *s)
{
	static BTREEINFO ib;
	static HASHINFO ih;
	static RECNOINFO rh;
	char *eq;

	if ((eq = strchr(s, '=')) == NULL)
		err("%s: illegal structure set statement", s);
	*eq++ = '\0';
	if (!isdigit((unsigned char)*eq))
		err("%s: structure set statement must be a number", s);

	switch (dbtype_arg) {
	case DB_BTREE:
		if (!std::strcmp("flags", s)) {
			ib.flags = atoi(eq);
			return (&ib);
		}
		if (!std::strcmp("cachesize", s)) {
			ib.cachesize = atoi(eq);
			return (&ib);
		}
		if (!std::strcmp("maxkeypage", s)) {
			ib.maxkeypage = atoi(eq);
			return (&ib);
		}
		if (!std::strcmp("minkeypage", s)) {
			ib.minkeypage = atoi(eq);
			return (&ib);
		}
		if (!std::strcmp("lorder", s)) {
			ib.lorder = atoi(eq);
			return (&ib);
		}
		if (!std::strcmp("psize", s)) {
			ib.psize = atoi(eq);
			return (&ib);
		}
		break;
	case DB_HASH:
		if (!std::strcmp("bsize", s)) {
			ih.bsize = atoi(eq);
			return (&ih);
		}
		if (!std::strcmp("ffactor", s)) {
			ih.ffactor = atoi(eq);
			return (&ih);
		}
		if (!std::strcmp("nelem", s)) {
			ih.nelem = atoi(eq);
			return (&ih);
		}
		if (!std::strcmp("cachesize", s)) {
			ih.cachesize = atoi(eq);
			return (&ih);
		}
		if (!std::strcmp("lorder", s)) {
			ih.lorder = atoi(eq);
			return (&ih);
		}
		break;
	case DB_RECNO:
		if (!std::strcmp("flags", s)) {
			rh.flags = atoi(eq);
			return (&rh);
		}
		if (!std::strcmp("cachesize", s)) {
			rh.cachesize = atoi(eq);
			return (&rh);
		}
		if (!std::strcmp("lorder", s)) {
			rh.lorder = atoi(eq);
			return (&rh);
		}
		if (!std::strcmp("reclen", s)) {
			rh.reclen = atoi(eq);
			return (&rh);
		}
		if (!std::strcmp("bval", s)) {
			rh.bval = atoi(eq);
			return (&rh);
		}
		if (!std::strcmp("psize", s)) {
			rh.psize = atoi(eq);
			return (&rh);
		}
		break;
	}
	err("%s: unknown structure value", s);
}

void *rfile(char *name, size_t *lenp)
{
	struct stat sb;
	void *p;
	int fd;
	char *np;

	for (; isspace((unsigned char)*name); ++name);
	if ((np = strchr(name, '\n')) != NULL)
		*np = '\0';
	if ((fd = open(name, O_RDONLY, 0)) < 0 ||
	    fstat(fd, &sb))
		err("%s: %s\n", name, strerror(errno));
	if ((p = (void *)malloc((unsigned int)sb.st_size)) == NULL)
		err("%s", strerror(errno));
	(void)read(fd, p, (int)sb.st_size);
	*lenp = sb.st_size;
	(void)close(fd);
	return (p);
}

void *xmalloc(char *text, size_t len)
{
	void *p;

	if ((p = (void *)malloc(len)) == NULL)
		err("%s", strerror(errno));
	memmove(p, text, len);
	return (p);
}

void usage()
{
	(void)std::fprintf(stderr,
	    "usage: dbtest [-l] [-f file] [-i info] [-o file] type script\n");
	std::exit(1);
}

void err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)std::fprintf(stderr, "dbtest: ");
	(void)std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)std::fprintf(stderr, "\n");
	std::exit(1);
}

} /* namespace */
