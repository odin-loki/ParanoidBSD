/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 */

#include <sys/param.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef R_APPEND
#define R_APPEND 12
#endif

#define	RET_ERROR	-1
#define	RET_SUCCESS	 0
#define	RET_SPECIAL	 1

#define	R_CURSOR	1
#define	R_FIRST		3
#define	R_IAFTER	4
#define	R_IBEFORE	5
#define	R_LAST		6
#define	R_NEXT		7
#define	R_NOOVERWRITE	8
#define	R_PREV		9

typedef uint32_t recno_t;

typedef struct {
	void *data;
	size_t size;
} DBT;

typedef struct __db {
	int (*close)(struct __db *);
	int (*del)(const struct __db *, const DBT *, unsigned int);
	int (*get)(const struct __db *, const DBT *, DBT *, unsigned int);
	int (*put)(struct __db *, DBT *, const DBT *, unsigned int);
	int (*seq)(struct __db *, DBT *, DBT *, unsigned int);
	int (*sync)(struct __db *);
	void *internal;
} DB;

typedef struct cmd_table {
	char *cmd;
	int nargs;
	int rconv;
	void (*func)(DB *, char **);
	char *usage, *descrip;
} cmd_table;

int ref_stopstop;
DB *ref_globaldb;

void ref_append(DB *, char **);
void ref_cursor(DB *, char **);
void ref_delcur(DB *, char **);
void ref_delete(DB *, char **);
void ref_first(DB *, char **);
void ref_get(DB *, char **);
void ref_help(DB *, char **);
void ref_iafter(DB *, char **);
void ref_ibefore(DB *, char **);
void ref_icursor(DB *, char **);
void ref_insert(DB *, char **);
void ref_keydata(DBT *, DBT *);
void ref_last(DB *, char **);
void ref_list(DB *, char **);
void ref_next(DB *, char **);
int  ref_parse(char *, char **, int);
void ref_previous(DB *, char **);
void ref_usage(void);

int recno;
char *dict = "words";
char *progname;

cmd_table commands[] = {
	"?",	0, 0, ref_help, "help", NULL,
	"a",	2, 1, ref_append, "append key def", "append key with data def",
	"b",	0, 0, NULL, "bstat", "stat btree",
	"c",	1, 1, ref_cursor,  "cursor word", "move cursor to word",
	"delc",	0, 0, ref_delcur, "delcur", "delete key the cursor references",
	"dele",	1, 1, ref_delete, "delete word", "delete word",
	"d",	0, 0, NULL, "dump", "dump database",
	"f",	0, 0, ref_first, "first", "move cursor to first record",
	"g",	1, 1, ref_get, "get key", "locate key",
	"h",	0, 0, ref_help, "help", "print command summary",
	"ia",	2, 1, ref_iafter, "iafter key data", "insert data after key",
	"ib",	2, 1, ref_ibefore, "ibefore key data", "insert data before key",
	"ic",	2, 1, ref_icursor, "icursor key data", "replace cursor",
	"in",	2, 1, ref_insert, "insert key def", "insert key with data def",
	"la",	0, 0, ref_last, "last", "move cursor to last record",
	"li",	1, 1, ref_list, "list file", "list to a file",
	"loa",	1, 0, NULL, "load file", NULL,
	"loc",	1, 1, ref_get, "get key", NULL,
	"m",	0, 0, NULL, "mstat", "stat memory pool",
	"n",	0, 0, ref_next, "next", "move cursor forward one record",
	"p",	0, 0, ref_previous, "previous", "move cursor back one record",
	"q",	0, 0, NULL, "quit", "quit",
	"sh",	1, 0, NULL, "show page", "dump a page",
	{ NULL },
};

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Olson.
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

int
ref_parse(lbuf, argv, maxargc)
	char *lbuf, **argv;
	int maxargc;
{
	int argc = 0;
	char *c;

	c = lbuf;
	while (isspace(*c))
		c++;
	while (*c != '\0' && argc < maxargc) {
		*argv++ = c;
		argc++;
		while (!isspace(*c) && *c != '\0') {
			c++;
		}
		while (isspace(*c))
			*c++ = '\0';
	}
	return (argc);
}

void
ref_append(db, argv)
	DB *db;
	char **argv;
{
	DBT key, data;
	int status;

	if (!recno) {
		(void)fprintf(stderr,
		    "append only available for recno db's.\n");
		return;
	}
	key.data = argv[1];
	key.size = sizeof(recno_t);
	data.data = argv[2];
	data.size = strlen(data.data);
	status = (db->put)(db, &key, &data, R_APPEND);
	switch (status) {
	case RET_ERROR:
		perror("append/put");
		break;
	case RET_SPECIAL:
		(void)printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
ref_cursor(db, argv)
	DB *db;
	char **argv;
{
	DBT data, key;
	int status;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = strlen(argv[1]) + 1;
	status = (*db->seq)(db, &key, &data, R_CURSOR);
	switch (status) {
	case RET_ERROR:
		perror("cursor/seq");
		break;
	case RET_SPECIAL:
		(void)printf("key not found\n");
		break;
	case RET_SUCCESS:
		ref_keydata(&key, &data);
		break;
	}
}

void
ref_delcur(db, argv)
	DB *db;
	char **argv;
{
	int status;

	status = (*db->del)(db, NULL, R_CURSOR);

	if (status == RET_ERROR)
		perror("delcur/del");
}

void
ref_delete(db, argv)
	DB *db;
	char **argv;
{
	DBT key;
	int status;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = strlen(argv[1]) + 1;

	status = (*db->del)(db, &key, 0);
	switch (status) {
	case RET_ERROR:
		perror("delete/del");
		break;
	case RET_SPECIAL:
		(void)printf("key not found\n");
		break;
	case RET_SUCCESS:
		break;
	}
}

void
ref_first(db, argv)
	DB *db;
	char **argv;
{
	DBT data, key;
	int status;

	status = (*db->seq)(db, &key, &data, R_FIRST);

	switch (status) {
	case RET_ERROR:
		perror("first/seq");
		break;
	case RET_SPECIAL:
		(void)printf("no more keys\n");
		break;
	case RET_SUCCESS:
		ref_keydata(&key, &data);
		break;
	}
}

void
ref_get(db, argv)
	DB *db;
	char **argv;
{
	DBT data, key;
	int status;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = strlen(argv[1]) + 1;

	status = (*db->get)(db, &key, &data, 0);

	switch (status) {
	case RET_ERROR:
		perror("get/get");
		break;
	case RET_SPECIAL:
		(void)printf("key not found\n");
		break;
	case RET_SUCCESS:
		ref_keydata(&key, &data);
		break;
	}
}

void
ref_help(db, argv)
	DB *db;
	char **argv;
{
	int i;

	for (i = 0; commands[i].cmd; i++)
		if (commands[i].descrip)
			(void)printf("%s: %s\n",
			    commands[i].usage, commands[i].descrip);
}

void
ref_iafter(db, argv)
	DB *db;
	char **argv;
{
	DBT key, data;
	int status;

	if (!recno) {
		(void)fprintf(stderr,
		    "iafter only available for recno db's.\n");
		return;
	}
	key.data = argv[1];
	key.size = sizeof(recno_t);
	data.data = argv[2];
	data.size = strlen(data.data);
	status = (db->put)(db, &key, &data, R_IAFTER);
	switch (status) {
	case RET_ERROR:
		perror("iafter/put");
		break;
	case RET_SPECIAL:
		(void)printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
ref_ibefore(db, argv)
	DB *db;
	char **argv;
{
	DBT key, data;
	int status;

	if (!recno) {
		(void)fprintf(stderr,
		    "ibefore only available for recno db's.\n");
		return;
	}
	key.data = argv[1];
	key.size = sizeof(recno_t);
	data.data = argv[2];
	data.size = strlen(data.data);
	status = (db->put)(db, &key, &data, R_IBEFORE);
	switch (status) {
	case RET_ERROR:
		perror("ibefore/put");
		break;
	case RET_SPECIAL:
		(void)printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
ref_icursor(db, argv)
	DB *db;
	char **argv;
{
	int status;
	DBT data, key;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = strlen(argv[1]) + 1;
	data.data = argv[2];
	data.size = strlen(argv[2]) + 1;

	status = (*db->put)(db, &key, &data, R_CURSOR);
	switch (status) {
	case RET_ERROR:
		perror("icursor/put");
		break;
	case RET_SPECIAL:
		(void)printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
ref_insert(db, argv)
	DB *db;
	char **argv;
{
	int status;
	DBT data, key;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = strlen(argv[1]) + 1;
	data.data = argv[2];
	data.size = strlen(argv[2]) + 1;

	status = (*db->put)(db, &key, &data, R_NOOVERWRITE);
	switch (status) {
	case RET_ERROR:
		perror("insert/put");
		break;
	case RET_SPECIAL:
		(void)printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
ref_last(db, argv)
	DB *db;
	char **argv;
{
	DBT data, key;
	int status;

	status = (*db->seq)(db, &key, &data, R_LAST);

	switch (status) {
	case RET_ERROR:
		perror("last/seq");
		break;
	case RET_SPECIAL:
		(void)printf("no more keys\n");
		break;
	case RET_SUCCESS:
		ref_keydata(&key, &data);
		break;
	}
}

void
ref_list(db, argv)
	DB *db;
	char **argv;
{
	DBT data, key;
	FILE *fp;
	int status;

	if ((fp = fopen(argv[1], "w")) == NULL) {
		(void)fprintf(stderr, "%s: %s\n", argv[1], strerror(errno));
		return;
	}
	status = (*db->seq)(db, &key, &data, R_FIRST);
	while (status == RET_SUCCESS) {
		(void)fprintf(fp, "%s\n", key.data);
		status = (*db->seq)(db, &key, &data, R_NEXT);
	}
	if (status == RET_ERROR)
		perror("list/seq");
	(void)fclose(fp);
}

void
ref_next(db, argv)
	DB *db;
	char **argv;
{
	DBT data, key;
	int status;

	status = (*db->seq)(db, &key, &data, R_NEXT);

	switch (status) {
	case RET_ERROR:
		perror("next/seq");
		break;
	case RET_SPECIAL:
		(void)printf("no more keys\n");
		break;
	case RET_SUCCESS:
		ref_keydata(&key, &data);
		break;
	}
}

void
ref_previous(db, argv)
	DB *db;
	char **argv;
{
	DBT data, key;
	int status;

	status = (*db->seq)(db, &key, &data, R_PREV);

	switch (status) {
	case RET_ERROR:
		perror("previous/seq");
		break;
	case RET_SPECIAL:
		(void)printf("no more keys\n");
		break;
	case RET_SUCCESS:
		ref_keydata(&key, &data);
		break;
	}
}

void
ref_keydata(key, data)
	DBT *key, *data;
{
	if (!recno && key->size > 0)
		(void)printf("%s/", key->data);
	if (data->size > 0)
		(void)printf("%s", data->data);
	(void)printf("\n");
}

void
ref_usage()
{
	(void)fprintf(stderr,
	    "usage: %s [-bdlu] [-c cache] [-i file] [-p page] [file]\n",
	    progname);
	exit (1);
}
