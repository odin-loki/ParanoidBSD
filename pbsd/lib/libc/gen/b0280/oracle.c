/*
 * oracle.c -- reference implementation for batch b0280.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/gen/stringlist.c
 *
 * Sources omitted (see skipped.txt):
 *   hbsd/src/lib/libc/gen/elf_utils.c
 *   hbsd/src/lib/libc/gen/nftw-compat11.c
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <err.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef _err
#define _err(status, ...) err(status, __VA_ARGS__)
#endif

static void *
reallocf(void *ptr, size_t size)
{
	void *n;

	n = realloc(ptr, size);
	if (n == NULL && ptr != NULL)
		free(ptr);
	return (n);
}

typedef struct _stringlist {
	char	**sl_str;
	size_t	  sl_max;
	size_t	  sl_cur;
} StringList;

#define _SL_CHUNKSIZE	20

/* ------------------------------------------------------------------ */
/* stringlist.c                                                       */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1994 Christos Zoulas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

StringList *
ref_sl_init(void)
{
	StringList *sl;

	sl = malloc(sizeof(StringList));
	if (sl == NULL)
		_err(1, "stringlist: %m");

	sl->sl_cur = 0;
	sl->sl_max = _SL_CHUNKSIZE;
	sl->sl_str = malloc(sl->sl_max * sizeof(char *));
	if (sl->sl_str == NULL)
		_err(1, "stringlist: %m");
	return sl;
}


/*
 * sl_add(): Add an item to the string list
 */
int
ref_sl_add(StringList *sl, char *name)
{
	if (sl->sl_cur == sl->sl_max - 1) {
		sl->sl_max += _SL_CHUNKSIZE;
		sl->sl_str = reallocf(sl->sl_str, sl->sl_max * sizeof(char *));
		if (sl->sl_str == NULL)
			return (-1);
	}
	sl->sl_str[sl->sl_cur++] = name;
	return (0);
}


/*
 * sl_free(): Free a stringlist
 */
void
ref_sl_free(StringList *sl, int all)
{
	size_t i;

	if (sl == NULL)
		return;
	if (sl->sl_str) {
		if (all)
			for (i = 0; i < sl->sl_cur; i++)
				free(sl->sl_str[i]);
		free(sl->sl_str);
	}
	free(sl);
}


/*
 * sl_find(): Find a name in the string list
 */
char *
ref_sl_find(StringList *sl, const char *name)
{
	size_t i;

	for (i = 0; i < sl->sl_cur; i++)
		if (strcmp(sl->sl_str[i], name) == 0)
			return sl->sl_str[i];

	return NULL;
}
