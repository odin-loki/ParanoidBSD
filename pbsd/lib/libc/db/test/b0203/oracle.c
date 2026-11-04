/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define index strchr

typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned long u_long;

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

void	 ref_compare(DBT *, DBT *);
DBTYPE	 ref_dbtype(char *);
void	 ref_dump(DB *, int);
void	 ref_err(const char *, ...) ;
void	 ref_get(DB *, DBT *);
void	 ref_getdata(DB *, DBT *, DBT *);
void	 ref_put(DB *, DBT *, DBT *);
void	 ref_rem(DB *, DBT *);
char	*ref_sflags(int);
void	 ref_synk(DB *);
void	*ref_rfile(char *, size_t *);
void	 ref_seq(DB *, DBT *);
u_int	 ref_setflags(char *);
void	*ref_setinfo(DBTYPE, char *);
void	 ref_usage(void);
void	*ref_xmalloc(char *, size_t);

DBTYPE type;				/* Database type. */
void *infop;				/* Iflags. */
u_long lineno;				/* Current line in test script. */
u_int flags;				/* Current DB flags. */
int ofd = STDOUT_FILENO;		/* Standard output fd. */

DB *XXdbp;				/* Global for gdb. */
int XXlineno;				/* Fast breakpoint for gdb. */

#define	NOOVERWRITE	"put failed, would overwrite key\n"

void
ref_compare(db1, db2)
	DBT *db1, *db2;
{
	size_t len;
	u_char *p1, *p2;

	if (db1->size != db2->size)
		printf("compare failed: key->data len %lu != data len %lu\n",
		    db1->size, db2->size);

	len = MIN(db1->size, db2->size);
	for (p1 = db1->data, p2 = db2->data; len--;)
		if (*p1++ != *p2++) {
			printf("compare failed at offset %d\n",
			    p1 - (u_char *)db1->data);
			break;
		}
}

void
ref_get(dbp, kp)
	DB *dbp;
	DBT *kp;
{
	DBT data;

	switch (dbp->get(dbp, kp, &data, flags)) {
	case 0:
		(void)write(ofd, data.data, data.size);
		if (ofd == STDOUT_FILENO)
			(void)write(ofd, "\n", 1);
		break;
	case -1:
		ref_err("line %lu: get: %s", lineno, strerror(errno));
		/* NOTREACHED */
	case 1:
#define	NOSUCHKEY	"get failed, no such key\n"
		if (ofd != STDOUT_FILENO)
			(void)write(ofd, NOSUCHKEY, sizeof(NOSUCHKEY) - 1);
		else
			(void)fprintf(stderr, "%d: %.*s: %s",
			    lineno, MIN(kp->size, 20), kp->data, NOSUCHKEY);
#undef	NOSUCHKEY
		break;
	}
}

void
ref_getdata(dbp, kp, dp)
	DB *dbp;
	DBT *kp, *dp;
{
	switch (dbp->get(dbp, kp, dp, flags)) {
	case 0:
		return;
	case -1:
		ref_err("line %lu: getdata: %s", lineno, strerror(errno));
		/* NOTREACHED */
	case 1:
		ref_err("line %lu: getdata failed, no such key", lineno);
		/* NOTREACHED */
	}
}

void
ref_put(dbp, kp, dp)
	DB *dbp;
	DBT *kp, *dp;
{
	switch (dbp->put(dbp, kp, dp, flags)) {
	case 0:
		break;
	case -1:
		ref_err("line %lu: put: %s", lineno, strerror(errno));
		/* NOTREACHED */
	case 1:
		(void)write(ofd, NOOVERWRITE, sizeof(NOOVERWRITE) - 1);
		break;
	}
}

void
ref_rem(dbp, kp)
	DB *dbp;
	DBT *kp;
{
	switch (dbp->del(dbp, kp, flags)) {
	case 0:
		break;
	case -1:
		ref_err("line %lu: rem: %s", lineno, strerror(errno));
		/* NOTREACHED */
	case 1:
#define	NOSUCHKEY	"rem failed, no such key\n"
		if (ofd != STDOUT_FILENO)
			(void)write(ofd, NOSUCHKEY, sizeof(NOSUCHKEY) - 1);
		else if (flags != R_CURSOR)
			(void)fprintf(stderr, "%d: %.*s: %s", 
			    lineno, MIN(kp->size, 20), kp->data, NOSUCHKEY);
		else
			(void)fprintf(stderr,
			    "%d: rem of cursor failed\n", lineno);
#undef	NOSUCHKEY
		break;
	}
}

void
ref_synk(dbp)
	DB *dbp;
{
	switch (dbp->sync(dbp, flags)) {
	case 0:
		break;
	case -1:
		ref_err("line %lu: synk: %s", lineno, strerror(errno));
		/* NOTREACHED */
	}
}

void
ref_seq(dbp, kp)
	DB *dbp;
	DBT *kp;
{
	DBT data;

	switch (dbp->seq(dbp, kp, &data, flags)) {
	case 0:
		(void)write(ofd, data.data, data.size);
		if (ofd == STDOUT_FILENO)
			(void)write(ofd, "\n", 1);
		break;
	case -1:
		ref_err("line %lu: seq: %s", lineno, strerror(errno));
		/* NOTREACHED */
	case 1:
#define	NOSUCHKEY	"seq failed, no such key\n"
		if (ofd != STDOUT_FILENO)
			(void)write(ofd, NOSUCHKEY, sizeof(NOSUCHKEY) - 1);
		else if (flags == R_CURSOR)
			(void)fprintf(stderr, "%d: %.*s: %s", 
			    lineno, MIN(kp->size, 20), kp->data, NOSUCHKEY);
		else
			(void)fprintf(stderr,
			    "%d: seq (%s) failed\n", lineno, ref_sflags(flags));
#undef	NOSUCHKEY
		break;
	}
}

void
ref_dump(dbp, rev)
	DB *dbp;
	int rev;
{
	DBT key, data;
	int flags, nflags;

	if (rev) {
		flags = R_LAST;
		nflags = R_PREV;
	} else {
		flags = R_FIRST;
		nflags = R_NEXT;
	}
	for (;; flags = nflags)
		switch (dbp->seq(dbp, &key, &data, flags)) {
		case 0:
			(void)write(ofd, data.data, data.size);
			if (ofd == STDOUT_FILENO)
				(void)write(ofd, "\n", 1);
			break;
		case 1:
			goto done;
		case -1:
			ref_err("line %lu: (dump) seq: %s",
			    lineno, strerror(errno));
			/* NOTREACHED */
		}
done:	return;
}
	
u_int
ref_setflags(s)
	char *s;
{
	char *p, *index();

	for (; isspace(*s); ++s);
	if (*s == '\n' || *s == '\0')
		return (0);
	if ((p = index(s, '\n')) != NULL)
		*p = '\0';
	if (!strcmp(s, "R_CURSOR"))		return (R_CURSOR);
	if (!strcmp(s, "R_FIRST"))		return (R_FIRST);
	if (!strcmp(s, "R_IAFTER")) 		return (R_IAFTER);
	if (!strcmp(s, "R_IBEFORE")) 		return (R_IBEFORE);
	if (!strcmp(s, "R_LAST")) 		return (R_LAST);
	if (!strcmp(s, "R_NEXT")) 		return (R_NEXT);
	if (!strcmp(s, "R_NOOVERWRITE"))	return (R_NOOVERWRITE);
	if (!strcmp(s, "R_PREV"))		return (R_PREV);
	if (!strcmp(s, "R_SETCURSOR"))		return (R_SETCURSOR);

	ref_err("line %lu: %s: unknown flag", lineno, s);
	/* NOTREACHED */
}

char *
ref_sflags(flags)
	int flags;
{
	switch (flags) {
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
	
DBTYPE
ref_dbtype(s)
	char *s;
{
	if (!strcmp(s, "btree"))
		return (DB_BTREE);
	if (!strcmp(s, "hash"))
		return (DB_HASH);
	if (!strcmp(s, "recno"))
		return (DB_RECNO);
	ref_err("%s: unknown type (use btree, hash or recno)", s);
	/* NOTREACHED */
}

void *
ref_setinfo(type, s)
	DBTYPE type;
	char *s;
{
	static BTREEINFO ib;
	static HASHINFO ih;
	static RECNOINFO rh;
	char *eq, *index();

	if ((eq = index(s, '=')) == NULL)
		ref_err("%s: illegal structure set statement", s);
	*eq++ = '\0';
	if (!isdigit(*eq))
		ref_err("%s: structure set statement must be a number", s);
		
	switch (type) {
	case DB_BTREE:
		if (!strcmp("flags", s)) {
			ib.flags = atoi(eq);
			return (&ib);
		}
		if (!strcmp("cachesize", s)) {
			ib.cachesize = atoi(eq);
			return (&ib);
		}
		if (!strcmp("maxkeypage", s)) {
			ib.maxkeypage = atoi(eq);
			return (&ib);
		}
		if (!strcmp("minkeypage", s)) {
			ib.minkeypage = atoi(eq);
			return (&ib);
		}
		if (!strcmp("lorder", s)) {
			ib.lorder = atoi(eq);
			return (&ib);
		}
		if (!strcmp("psize", s)) {
			ib.psize = atoi(eq);
			return (&ib);
		}
		break;
	case DB_HASH:
		if (!strcmp("bsize", s)) {
			ih.bsize = atoi(eq);
			return (&ih);
		}
		if (!strcmp("ffactor", s)) {
			ih.ffactor = atoi(eq);
			return (&ih);
		}
		if (!strcmp("nelem", s)) {
			ih.nelem = atoi(eq);
			return (&ih);
		}
		if (!strcmp("cachesize", s)) {
			ih.cachesize = atoi(eq);
			return (&ih);
		}
		if (!strcmp("lorder", s)) {
			ih.lorder = atoi(eq);
			return (&ih);
		}
		break;
	case DB_RECNO:
		if (!strcmp("flags", s)) {
			rh.flags = atoi(eq);
			return (&rh);
		}
		if (!strcmp("cachesize", s)) {
			rh.cachesize = atoi(eq);
			return (&rh);
		}
		if (!strcmp("lorder", s)) {
			rh.lorder = atoi(eq);
			return (&rh);
		}
		if (!strcmp("reclen", s)) {
			rh.reclen = atoi(eq);
			return (&rh);
		}
		if (!strcmp("bval", s)) {
			rh.bval = atoi(eq);
			return (&rh);
		}
		if (!strcmp("psize", s)) {
			rh.psize = atoi(eq);
			return (&rh);
		}
		break;
	}
	ref_err("%s: unknown structure value", s);
	/* NOTREACHED */
}

void *
ref_rfile(name, lenp)
	char *name;
	size_t *lenp;
{
	struct stat sb;
	void *p;
	int fd;
	char *np, *index();

	for (; isspace(*name); ++name);
	if ((np = index(name, '\n')) != NULL)
		*np = '\0';
	if ((fd = open(name, O_RDONLY, 0)) < 0 ||
	    fstat(fd, &sb))
		ref_err("%s: %s\n", name, strerror(errno));
#ifdef NOT_PORTABLE
	if (sb.st_size > (off_t)SIZE_T_MAX)
		ref_err("%s: %s\n", name, strerror(E2BIG));
#endif
	if ((p = (void *)malloc((u_int)sb.st_size)) == NULL)
		ref_err("%s", strerror(errno));
	(void)read(fd, p, (int)sb.st_size);
	*lenp = sb.st_size;
	(void)close(fd);
	return (p);
}

void *
ref_xmalloc(text, len)
	char *text;
	size_t len;
{
	void *p;

	if ((p = (void *)malloc(len)) == NULL)
		ref_err("%s", strerror(errno));
	memmove(p, text, len);
	return (p);
}

void
ref_usage()
{
	(void)fprintf(stderr,
	    "usage: dbtest [-l] [-f file] [-i info] [-o file] type script\n");
	exit(1);
}

void
ref_err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)fprintf(stderr, "dbtest: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	exit(1);
	/* NOTREACHED */
}
