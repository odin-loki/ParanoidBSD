/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2013 Peter Wemm
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Batch b0063 oracle: the original HardenedBSD sources concatenated, with each
 * function renamed with a ref_ prefix.  Function bodies are unmodified.
 *
 *	lib/libc/iconv/iconv_canonicalize.c
 *	lib/libc/iconv/__iconv_get_list.c
 *	lib/libc/iconv/iconvlist.c
 *	lib/libc/iconv/iconv_set_relocation_prefix.c
 *
 * The batch sources include <sys/types.h>, <iconv.h> and "iconv-internal.h".
 * Those headers supply size_t, __iconv_bool and the __bsd_* prototypes; the
 * equivalents are provided below so that this file is self contained.
 */

#include <stddef.h>
#include <string.h>

/*
 * <iconv.h>: __iconv_bool is _Bool for C99 and later.
 */
typedef _Bool __iconv_bool;

/*
 * "iconv-internal.h": prototypes for the renamed citrus entry points.
 */
const char *__bsd_iconv_canonicalize(const char *);
int __bsd___iconv_get_list(char ***, size_t *, __iconv_bool);
void __bsd_iconvlist(int (*)(unsigned int, const char *const *, void *), void *);
void __bsd_iconv_set_relocation_prefix(const char *, const char *);

/* lib/libc/iconv/iconv_canonicalize.c */
const char *
ref_iconv_canonicalize(const char *a)
{
	return __bsd_iconv_canonicalize(a);
}

/* lib/libc/iconv/__iconv_get_list.c */
int
ref___iconv_get_list(char ***a, size_t *b, __iconv_bool c)
{
	return __bsd___iconv_get_list(a, b, c);
}

/* lib/libc/iconv/iconvlist.c */
void
ref_iconvlist(int (*a) (unsigned int, const char * const *, void *), void *b)
{
	return __bsd_iconvlist(a, b);
}

/* lib/libc/iconv/iconv_set_relocation_prefix.c */
void
ref_iconv_set_relocation_prefix(const char *a, const char *b)
{
	return __bsd_iconv_set_relocation_prefix(a, b);
}

/* ------------------------------------------------------------------ */
/* instrumented __bsd_* mocks for the differential harness              */
/* ------------------------------------------------------------------ */

#define	B0063_CANON_BUF	512

typedef struct B0063MockSnap {
	unsigned long long	ncalls;
	const char		*canonicalize_arg;
	const char		*canonicalize_ret;
	int			get_list_ret;
	char			***get_list_a;
	size_t			*get_list_b;
	__iconv_bool		get_list_c;
	char			**get_list_out;
	size_t			get_list_count;
	int			(*iconvlist_fn)(unsigned int, const char *const *, void *);
	void			*iconvlist_data;
	unsigned int		iconvlist_cb_count;
	unsigned int		iconvlist_cb_arg0;
	const char		*iconvlist_cb_names[4];
	void			*iconvlist_cb_user;
	int			iconvlist_cb_rets[4];
	const char		*reloc_a;
	const char		*reloc_b;
} B0063MockSnap;

static B0063MockSnap b0063_snap;

void
b0063_mock_reset(void)
{

	memset(&b0063_snap, 0, sizeof(b0063_snap));
}

void
b0063_mock_snap(B0063MockSnap *dst)
{

	*dst = b0063_snap;
}

const char *
__bsd_iconv_canonicalize(const char *a)
{
	static char out[B0063_CANON_BUF];
	size_t i;

	b0063_snap.ncalls++;
	b0063_snap.canonicalize_arg = a;
	if (a == NULL) {
		b0063_snap.canonicalize_ret = NULL;
		return (NULL);
	}

	for (i = 0; a[i] != '\0' && i + 1 < B0063_CANON_BUF; i++) {
		unsigned char c = (unsigned char)a[i];

		if (c >= 'A' && c <= 'Z')
			out[i] = (char)(c - 'A' + 'a');
		else
			out[i] = (char)c;
	}
	out[i] = '\0';
	b0063_snap.canonicalize_ret = out;
	return (out);
}

int
__bsd___iconv_get_list(char ***a, size_t *b, __iconv_bool c)
{
	static char name0[] = "utf-8";
	static char name1[] = "iso8859-1";
	static char *list[3];

	b0063_snap.ncalls++;
	b0063_snap.get_list_a = a;
	b0063_snap.get_list_b = b;
	b0063_snap.get_list_c = c;

	if (a != NULL) {
		list[0] = name0;
		list[1] = name1;
		list[2] = NULL;
		*a = list;
		b0063_snap.get_list_out = list;
	}
	if (b != NULL) {
		*b = c ? 2u : 0u;
		b0063_snap.get_list_count = *b;
	}

	b0063_snap.get_list_ret = c ? 0 : -1;
	return (b0063_snap.get_list_ret);
}

void
__bsd_iconvlist(int (*fn)(unsigned int, const char *const *, void *), void *arg)
{
	static const char *const names[] = {"UTF-8", "ASCII", "UCS-2"};
	unsigned int i, count = 3u;
	int cb_ret;

	b0063_snap.ncalls++;
	b0063_snap.iconvlist_fn = fn;
	b0063_snap.iconvlist_data = arg;
	if (fn == NULL)
		return;

	cb_ret = fn(count, names, arg);
	b0063_snap.iconvlist_cb_count = 1;
	b0063_snap.iconvlist_cb_arg0 = count;
	b0063_snap.iconvlist_cb_user = arg;
	for (i = 0; i < 4; i++)
		b0063_snap.iconvlist_cb_names[i] = NULL;
	for (i = 0; i < count && i < 4; i++)
		b0063_snap.iconvlist_cb_names[i] = names[i];
	b0063_snap.iconvlist_cb_rets[0] = cb_ret;
}

void
__bsd_iconv_set_relocation_prefix(const char *a, const char *b)
{

	b0063_snap.ncalls++;
	b0063_snap.reloc_a = a;
	b0063_snap.reloc_b = b;
}
