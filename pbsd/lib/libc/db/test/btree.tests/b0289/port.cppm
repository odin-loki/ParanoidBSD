module;

#include <sys/param.h>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>

export module pbsd.lib.libc.db.test.btree.tests.b0289;

export namespace pbsd::lib_libc_db_test_btree_tests::b0289 {

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

inline int stopstop{};
inline DB *globaldb{};
inline int recno{};
inline char *dict = (char *)"words";
inline char *progname{};

void append(DB *, char **);
void cursor(DB *, char **);
void delcur(DB *, char **);
void delete_fn(DB *, char **);
void first(DB *, char **);
void get(DB *, char **);
void help(DB *, char **);
void iafter(DB *, char **);
void ibefore(DB *, char **);
void icursor(DB *, char **);
void insert(DB *, char **);
void keydata(DBT *, DBT *);
void last(DB *, char **);
void list(DB *, char **);
void next(DB *, char **);
void previous(DB *, char **);
void usage(void);

cmd_table commands[] = {
	(char *)"?",	0, 0, help, (char *)"help", NULL,
	(char *)"a",	2, 1, append, (char *)"append key def", (char *)"append key with data def",
	(char *)"b",	0, 0, NULL, (char *)"bstat", (char *)"stat btree",
	(char *)"c",	1, 1, cursor,  (char *)"cursor word", (char *)"move cursor to word",
	(char *)"delc",	0, 0, delcur, (char *)"delcur", (char *)"delete key the cursor references",
	(char *)"dele",	1, 1, delete_fn, (char *)"delete word", (char *)"delete word",
	(char *)"d",	0, 0, NULL, (char *)"dump", (char *)"dump database",
	(char *)"f",	0, 0, first, (char *)"first", (char *)"move cursor to first record",
	(char *)"g",	1, 1, get, (char *)"get key", (char *)"locate key",
	(char *)"h",	0, 0, help, (char *)"help", (char *)"print command summary",
	(char *)"ia",	2, 1, iafter, (char *)"iafter key data", (char *)"insert data after key",
	(char *)"ib",	2, 1, ibefore, (char *)"ibefore key data", (char *)"insert data before key",
	(char *)"ic",	2, 1, icursor, (char *)"icursor key data", (char *)"replace cursor",
	(char *)"in",	2, 1, insert, (char *)"insert key def", (char *)"insert key with data def",
	(char *)"la",	0, 0, last, (char *)"last", (char *)"move cursor to last record",
	(char *)"li",	1, 1, list, (char *)"list file", (char *)"list to a file",
	(char *)"loa",	1, 0, NULL, (char *)"load file", NULL,
	(char *)"loc",	1, 1, get, (char *)"get key", NULL,
	(char *)"m",	0, 0, NULL, (char *)"mstat", (char *)"stat memory pool",
	(char *)"n",	0, 0, next, (char *)"next", (char *)"move cursor forward one record",
	(char *)"p",	0, 0, previous, (char *)"previous", (char *)"move cursor back one record",
	(char *)"q",	0, 0, NULL, (char *)"quit", (char *)"quit",
	(char *)"sh",	1, 0, NULL, (char *)"show page", (char *)"dump a page",
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
parse(char *lbuf, char **argv, int maxargc)
{
	int argc = 0;
	char *c;

	c = lbuf;
	while (isspace((unsigned char)*c))
		c++;
	while (*c != '\0' && argc < maxargc) {
		*argv++ = c;
		argc++;
		while (!isspace((unsigned char)*c) && *c != '\0') {
			c++;
		}
		while (isspace((unsigned char)*c))
			*c++ = '\0';
	}
	return (argc);
}

void
append(DB *db, char **argv)
{
	DBT key, data;
	int status;

	if (!recno) {
		(void)std::fprintf(stderr,
		    "append only available for recno db's.\n");
		return;
	}
	key.data = argv[1];
	key.size = sizeof(recno_t);
	data.data = argv[2];
	data.size = std::strlen(data.data);
	status = (db->put)(db, &key, &data, R_APPEND);
	switch (status) {
	case RET_ERROR:
		perror("append/put");
		break;
	case RET_SPECIAL:
		(void)std::printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
cursor(DB *db, char **argv)
{
	DBT data, key;
	int status;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = std::strlen(argv[1]) + 1;
	status = (*db->seq)(db, &key, &data, R_CURSOR);
	switch (status) {
	case RET_ERROR:
		perror("cursor/seq");
		break;
	case RET_SPECIAL:
		(void)std::printf("key not found\n");
		break;
	case RET_SUCCESS:
		keydata(&key, &data);
		break;
	}
}

void
delcur(DB *db, char **argv)
{
	int status;

	status = (*db->del)(db, NULL, R_CURSOR);

	if (status == RET_ERROR)
		perror("delcur/del");
}

void
delete_fn(DB *db, char **argv)
{
	DBT key;
	int status;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = std::strlen(argv[1]) + 1;

	status = (*db->del)(db, &key, 0);
	switch (status) {
	case RET_ERROR:
		perror("delete/del");
		break;
	case RET_SPECIAL:
		(void)std::printf("key not found\n");
		break;
	case RET_SUCCESS:
		break;
	}
}

void
first(DB *db, char **argv)
{
	DBT data, key;
	int status;

	status = (*db->seq)(db, &key, &data, R_FIRST);

	switch (status) {
	case RET_ERROR:
		perror("first/seq");
		break;
	case RET_SPECIAL:
		(void)std::printf("no more keys\n");
		break;
	case RET_SUCCESS:
		keydata(&key, &data);
		break;
	}
}

void
get(DB *db, char **argv)
{
	DBT data, key;
	int status;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = std::strlen(argv[1]) + 1;

	status = (*db->get)(db, &key, &data, 0);

	switch (status) {
	case RET_ERROR:
		perror("get/get");
		break;
	case RET_SPECIAL:
		(void)std::printf("key not found\n");
		break;
	case RET_SUCCESS:
		keydata(&key, &data);
		break;
	}
}

void
help(DB *db, char **argv)
{
	int i;

	for (i = 0; commands[i].cmd; i++)
		if (commands[i].descrip)
			(void)std::printf("%s: %s\n",
			    commands[i].usage, commands[i].descrip);
}

void
iafter(DB *db, char **argv)
{
	DBT key, data;
	int status;

	if (!recno) {
		(void)std::fprintf(stderr,
		    "iafter only available for recno db's.\n");
		return;
	}
	key.data = argv[1];
	key.size = sizeof(recno_t);
	data.data = argv[2];
	data.size = std::strlen(data.data);
	status = (db->put)(db, &key, &data, R_IAFTER);
	switch (status) {
	case RET_ERROR:
		perror("iafter/put");
		break;
	case RET_SPECIAL:
		(void)std::printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
ibefore(DB *db, char **argv)
{
	DBT key, data;
	int status;

	if (!recno) {
		(void)std::fprintf(stderr,
		    "ibefore only available for recno db's.\n");
		return;
	}
	key.data = argv[1];
	key.size = sizeof(recno_t);
	data.data = argv[2];
	data.size = std::strlen(data.data);
	status = (db->put)(db, &key, &data, R_IBEFORE);
	switch (status) {
	case RET_ERROR:
		perror("ibefore/put");
		break;
	case RET_SPECIAL:
		(void)std::printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
icursor(DB *db, char **argv)
{
	int status;
	DBT data, key;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = std::strlen(argv[1]) + 1;
	data.data = argv[2];
	data.size = std::strlen(argv[2]) + 1;

	status = (*db->put)(db, &key, &data, R_CURSOR);
	switch (status) {
	case RET_ERROR:
		perror("icursor/put");
		break;
	case RET_SPECIAL:
		(void)std::printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
insert(DB *db, char **argv)
{
	int status;
	DBT data, key;

	key.data = argv[1];
	if (recno)
		key.size = sizeof(recno_t);
	else
		key.size = std::strlen(argv[1]) + 1;
	data.data = argv[2];
	data.size = std::strlen(argv[2]) + 1;

	status = (*db->put)(db, &key, &data, R_NOOVERWRITE);
	switch (status) {
	case RET_ERROR:
		perror("insert/put");
		break;
	case RET_SPECIAL:
		(void)std::printf("%s (duplicate key)\n", argv[1]);
		break;
	case RET_SUCCESS:
		break;
	}
}

void
last(DB *db, char **argv)
{
	DBT data, key;
	int status;

	status = (*db->seq)(db, &key, &data, R_LAST);

	switch (status) {
	case RET_ERROR:
		perror("last/seq");
		break;
	case RET_SPECIAL:
		(void)std::printf("no more keys\n");
		break;
	case RET_SUCCESS:
		keydata(&key, &data);
		break;
	}
}

void
list(DB *db, char **argv)
{
	DBT data, key;
	FILE *fp;
	int status;

	if ((fp = std::fopen(argv[1], "w")) == NULL) {
		(void)std::fprintf(stderr, "%s: %s\n", argv[1], strerror(errno));
		return;
	}
	status = (*db->seq)(db, &key, &data, R_FIRST);
	while (status == RET_SUCCESS) {
		(void)std::fprintf(fp, "%s\n", (char *)key.data);
		status = (*db->seq)(db, &key, &data, R_NEXT);
	}
	if (status == RET_ERROR)
		perror("list/seq");
	std::fclose(fp);
}

void
next(DB *db, char **argv)
{
	DBT data, key;
	int status;

	status = (*db->seq)(db, &key, &data, R_NEXT);

	switch (status) {
	case RET_ERROR:
		perror("next/seq");
		break;
	case RET_SPECIAL:
		(void)std::printf("no more keys\n");
		break;
	case RET_SUCCESS:
		keydata(&key, &data);
		break;
	}
}

void
previous(DB *db, char **argv)
{
	DBT data, key;
	int status;

	status = (*db->seq)(db, &key, &data, R_PREV);

	switch (status) {
	case RET_ERROR:
		perror("previous/seq");
		break;
	case RET_SPECIAL:
		(void)std::printf("no more keys\n");
		break;
	case RET_SUCCESS:
		keydata(&key, &data);
		break;
	}
}

void
keydata(DBT *key, DBT *data)
{
	if (!recno && key->size > 0)
		(void)std::printf("%s/", (char *)key->data);
	if (data->size > 0)
		(void)std::printf("%s", (char *)data->data);
	(void)std::printf("\n");
}

void
usage()
{
	(void)std::fprintf(stderr,
	    "usage: %s [-bdlu] [-c cache] [-i file] [-p page] [file]\n",
	    progname);
	std::exit(1);
}

} /* namespace */
