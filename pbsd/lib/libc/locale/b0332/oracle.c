/*
 * PBSD batch b0332 -- reference oracle.
 *
 * Original HardenedBSD sources concatenated, every function renamed with
 * a ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/locale/lmonetary.c
 *   hbsd/src/lib/libc/locale/rune.c
 *   hbsd/src/lib/libc/locale/setlocale.c
 *   hbsd/src/lib/libc/locale/xlocale.c
 */

#define _GNU_SOURCE 1


#ifndef SIZE_T_MAX
#define SIZE_T_MAX	((size_t)-1)
#endif

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#ifndef PATH_MAX
#define PATH_MAX	4096
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC	0
#endif

#ifndef EFTYPE
#define EFTYPE		79
#endif

#ifndef ENAMETOOLONG
#define ENAMETOOLONG	63
#endif

#define _LDP_LOADED	0
#define _LDP_ERROR	(-1)
#define _LDP_CACHE	1

#define _CACHED_RUNES	(1 << 8)
#define _RUNE_MAGIC_1	"RuneMagi"
#define _FILE_RUNE_MAGIC_1	"RuneMag1"

#define ENCODING_LEN	31
#define CATEGORY_LEN	11
#define XLOCALE_DEF_VERSION_LEN 12

#define LC_COLLATE_MASK	(1<<0)
#define LC_CTYPE_MASK	(1<<1)
#define LC_MONETARY_MASK (1<<2)
#define LC_NUMERIC_MASK	(1<<3)
#define LC_TIME_MASK	(1<<4)
#define LC_MESSAGES_MASK (1<<5)
#define LC_ALL_MASK	(LC_COLLATE_MASK | LC_CTYPE_MASK | LC_MESSAGES_MASK | \
			 LC_MONETARY_MASK | LC_NUMERIC_MASK | LC_TIME_MASK)
#define LC_VERSION_MASK	(1<<6)

#define _PATH_LOCALE	"/usr/share/locale"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#define LC_ALL		0
#define LC_COLLATE	1
#define LC_CTYPE	2
#define LC_MONETARY	3
#define LC_NUMERIC	4
#define LC_TIME		5
#define LC_MESSAGES	6
#define _LC_LAST	7

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef int32_t __rune_t;
typedef __rune_t __ct_rune_t;

typedef struct {
	__rune_t	__min;
	__rune_t	__max;
	__rune_t	__map;
	unsigned long	*__types;
} _RuneEntry;

typedef struct {
	int		__nranges;
	_RuneEntry	*__ranges;
} _RuneRange;

typedef struct {
	char		__magic[8];
	char		__encoding[32];
	__rune_t	(*__sgetrune)(const char *, size_t, char const **);
	int		(*__sputrune)(__rune_t, char *, size_t, char **);
	__rune_t	__invalid_rune;
	unsigned long	__runetype[_CACHED_RUNES];
	__rune_t	__maplower[_CACHED_RUNES];
	__rune_t	__mapupper[_CACHED_RUNES];
	_RuneRange	__runetype_ext;
	_RuneRange	__maplower_ext;
	_RuneRange	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
} _RuneLocale;

typedef struct {
	int32_t		min;
	int32_t		max;
	int32_t		map;
} _FileRuneEntry;

typedef struct {
	char		magic[8];
	char		encoding[32];
	uint32_t	runetype[_CACHED_RUNES];
	int32_t		maplower[_CACHED_RUNES];
	int32_t		mapupper[_CACHED_RUNES];
	int32_t		runetype_ext_nranges;
	int32_t		maplower_ext_nranges;
	int32_t		mapupper_ext_nranges;
	int32_t		variable_len;
} _FileRuneLocale;

struct lconv {
	char	*decimal_point;
	char	*thousands_sep;
	char	*grouping;
	char	*int_curr_symbol;
	char	*currency_symbol;
	char	*mon_decimal_point;
	char	*mon_thousands_sep;
	char	*mon_grouping;
	char	*positive_sign;
	char	*negative_sign;
	char	int_frac_digits;
	char	frac_digits;
	char	p_cs_precedes;
	char	p_sep_by_space;
	char	n_cs_precedes;
	char	n_sep_by_space;
	char	p_sign_posn;
	char	n_sign_posn;
	char	int_p_cs_precedes;
	char	int_n_cs_precedes;
	char	int_p_sep_by_space;
	char	int_n_sep_by_space;
	char	int_p_sign_posn;
	char	int_n_sign_posn;
};

struct lc_monetary_T {
	const char	*int_curr_symbol;
	const char	*currency_symbol;
	const char	*mon_decimal_point;
	const char	*mon_thousands_sep;
	const char	*mon_grouping;
	const char	*positive_sign;
	const char	*negative_sign;
	const char	*int_frac_digits;
	const char	*frac_digits;
	const char	*p_cs_precedes;
	const char	*p_sep_by_space;
	const char	*n_cs_precedes;
	const char	*n_sep_by_space;
	const char	*p_sign_posn;
	const char	*n_sign_posn;
	const char	*int_p_cs_precedes;
	const char	*int_n_cs_precedes;
	const char	*int_p_sep_by_space;
	const char	*int_n_sep_by_space;
	const char	*int_p_sign_posn;
	const char	*int_n_sign_posn;
};

struct xlocale_refcounted {
	long		retain_count;
	void		(*destructor)(void *);
};

struct xlocale_component {
	struct xlocale_refcounted header;
	char		locale[ENCODING_LEN + 1];
	char		version[XLOCALE_DEF_VERSION_LEN];
};

enum {
	XLC_COLLATE = 0,
	XLC_CTYPE,
	XLC_MONETARY,
	XLC_NUMERIC,
	XLC_TIME,
	XLC_MESSAGES,
	XLC_LAST
};

struct _xlocale {
	struct xlocale_refcounted header;
	struct xlocale_component *components[XLC_LAST];
	int		monetary_locale_changed;
	int		using_monetary_locale;
	int		numeric_locale_changed;
	int		using_numeric_locale;
	int		using_time_locale;
	int		using_messages_locale;
	struct lconv	lconv;
	char		*csym;
};

typedef struct _xlocale *pbsd_locale_t;

#include <pthread.h>
#undef pbsd_locale_t
typedef struct _xlocale *pbsd_locale_t;

#define LC_GLOBAL_LOCALE	((pbsd_locale_t)-1)

struct xlocale_monetary {
	struct xlocale_component header;
	char		*buffer;
	struct lc_monetary_T locale;
};

#define LCMONETARY_SIZE_FULL (sizeof(struct lc_monetary_T) / sizeof(char *))
#define LCMONETARY_SIZE_MIN \
		(offsetof(struct lc_monetary_T, int_p_cs_precedes) / \
		    sizeof(char *))

static inline void
atomic_store_rel_int(volatile int *p, int v)
{
	__atomic_store_n(p, v, __ATOMIC_RELEASE);
}

static inline long
atomic_fetchadd_long(volatile long *p, long v)
{
	return __atomic_fetch_add(p, v, __ATOMIC_SEQ_CST);
}

static inline void
atomic_add_long(volatile long *p, long v)
{
	__atomic_fetch_add(p, v, __ATOMIC_SEQ_CST);
}

static inline pbsd_locale_t
get_real_locale(pbsd_locale_t locale)
{
	switch ((intptr_t)locale) {
	case 0:
		return (&__xlocale_C_locale);
	case -1:
		return (&__xlocale_global_locale);
	default:
		return (locale);
	}
}

#define FIX_LOCALE(l)	((l) = get_real_locale(l))

static inline void *
xlocale_retain(void *val)
{
	struct xlocale_refcounted *obj = val;
	atomic_add_long(&(obj->retain_count), 1);
	return (val);
}

static inline void
xlocale_release(void *val)
{
	struct xlocale_refcounted *obj = val;
	long count;

	count = atomic_fetchadd_long(&(obj->retain_count), -1) - 1;
	if (count < 0 && obj->destructor != NULL)
		obj->destructor(obj);
}

static pbsd_locale_t
__get_locale(void)
{
	if (!__has_thread_locale || __thread_locale == NULL)
		return (&__xlocale_global_locale);
	return (__thread_locale);
}

/* harness-controllable hooks */
typedef struct {
	int		ret;
	int		using_locale;
	int		fail_malloc;
	int		line_count;
	const char	**lines;
	const char	*grouping_out;
} pbsd_part_load_hook_t;

typedef struct {
	int		open_fail;
	int		open_errno;
	int		fstat_fail;
	int		fstat_errno;
	off_t		file_size;
	const void	*file_data;
	int		mmap_fail;
	void		*mmap_addr;
	int		malloc_fail;
	int		close_count;
} pbsd_rune_hook_t;

typedef struct {
	int		ctype_ret;
	int		collate_ret;
	int		time_ret;
	int		numeric_ret;
	int		monetary_ret;
	int		messages_ret;
	int		detect_ret;
	int		detect_errno;
	char		*path_locale_dup;
	int		dup_fail;
	int		path_too_long;
} pbsd_setlocale_hook_t;

typedef struct {
	int		collate_fail;
	int		ctype_fail;
	int		monetary_fail;
	int		numeric_fail;
	int		time_fail;
	int		messages_fail;
	unsigned int	call_idx;
} pbsd_ctor_hook_t;

pbsd_part_load_hook_t	pbsd_part_load_hook;
pbsd_rune_hook_t	pbsd_rune_hook;
pbsd_setlocale_hook_t	pbsd_setlocale_hook;
pbsd_ctor_hook_t	pbsd_ctor_hook;

char			*_PathLocale = NULL;
int			__has_thread_locale = 0;
_Thread_local pbsd_locale_t	__thread_locale = NULL;

extern struct xlocale_component __xlocale_global_collate;
extern struct xlocale_component __xlocale_global_ctype;
extern struct xlocale_component __xlocale_global_numeric;
extern struct xlocale_component __xlocale_global_time;
extern struct xlocale_component __xlocale_global_messages;
extern struct xlocale_component __xlocale_C_collate;
extern struct xlocale_component __xlocale_C_ctype;
extern struct xlocale_monetary __xlocale_global_monetary;
extern struct _xlocale __xlocale_global_locale;
extern struct _xlocale __xlocale_C_locale;

void *
pbsd_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	(void)addr;
	(void)prot;
	(void)flags;
	(void)fd;
	(void)off;
	if (pbsd_rune_hook.mmap_fail) {
		errno = EINVAL;
		return (MAP_FAILED);
	}
	if (pbsd_rune_hook.mmap_addr != NULL)
		return (pbsd_rune_hook.mmap_addr);
	if (pbsd_rune_hook.file_data != NULL && len > 0) {
		void *p = malloc(len);
		if (p == NULL)
			return (MAP_FAILED);
		memcpy(p, pbsd_rune_hook.file_data, len);
		return (p);
	}
	return (MAP_FAILED);
}

int
pbsd_munmap(void *addr, size_t len)
{
	free(addr);
	(void)len;
	return (0);
}

struct xlocale_component __xlocale_global_collate = {{0, NULL}, "C", "BSD 1.0\n"};
struct xlocale_component __xlocale_global_ctype = {{0, NULL}, "C", "BSD 1.0\n"};
struct xlocale_component __xlocale_global_numeric = {{0, NULL}, "C", "BSD 1.0\n"};
struct xlocale_component __xlocale_global_time = {{0, NULL}, "C", "BSD 1.0\n"};
struct xlocale_component __xlocale_global_messages = {{0, NULL}, "C", "BSD 1.0\n"};
struct xlocale_component __xlocale_C_collate = {{0, NULL}, "C", ""};
struct xlocale_component __xlocale_C_ctype = {{0, NULL}, "C", ""};

void
pbsd_reset_hooks(void)
{
	memset(&pbsd_part_load_hook, 0, sizeof(pbsd_part_load_hook));
	memset(&pbsd_rune_hook, 0, sizeof(pbsd_rune_hook));
	memset(&pbsd_setlocale_hook, 0, sizeof(pbsd_setlocale_hook));
	memset(&pbsd_ctor_hook, 0, sizeof(pbsd_ctor_hook));
	pbsd_part_load_hook.ret = _LDP_LOADED;
	pbsd_setlocale_hook.ctype_ret = _LDP_CACHE;
	pbsd_setlocale_hook.collate_ret = _LDP_CACHE;
	pbsd_setlocale_hook.time_ret = _LDP_CACHE;
	pbsd_setlocale_hook.numeric_ret = _LDP_CACHE;
	pbsd_setlocale_hook.monetary_ret = _LDP_CACHE;
	pbsd_setlocale_hook.messages_ret = _LDP_CACHE;
}

static void __attribute__((constructor))
ref_oracle_init(void)
{
	pbsd_reset_hooks();
}

int
_once(pthread_once_t *o, void (*fn)(void))
{
	return (pthread_once(o, fn));
}

const char *
__fix_locale_grouping_str(const char *s)
{
	if (pbsd_part_load_hook.grouping_out != NULL)
		return (pbsd_part_load_hook.grouping_out);
	return (s);
}

int
__part_load_locale(const char *name, int *using_locale, char **locale_buf,
    const char *category_filename, int locale_buf_size_max,
    int locale_buf_size_min, const char **dst_localebuf)
{
	int i;

	(void)name;
	(void)category_filename;
	(void)locale_buf_size_max;
	(void)locale_buf_size_min;
	if (pbsd_part_load_hook.fail_malloc) {
		errno = ENOMEM;
		return (_LDP_ERROR);
	}
	if (pbsd_part_load_hook.ret == _LDP_ERROR)
		return (_LDP_ERROR);
	*using_locale = pbsd_part_load_hook.using_locale;
	if (pbsd_part_load_hook.ret == _LDP_CACHE)
		return (_LDP_CACHE);
	for (i = 0; i < pbsd_part_load_hook.line_count; i++)
		dst_localebuf[i] = pbsd_part_load_hook.lines[i];
	return (pbsd_part_load_hook.ret);
}

void
__set_thread_rune_locale(pbsd_locale_t loc)
{
	(void)loc;
}

int
__wrap_setrunelocale(const char *name)
{
	(void)name;
	return (pbsd_setlocale_hook.ctype_ret);
}

int
__collate_load_tables(const char *name)
{
	(void)name;
	return (pbsd_setlocale_hook.collate_ret);
}

int
__time_load_locale(const char *name)
{
	(void)name;
	return (pbsd_setlocale_hook.time_ret);
}

int
__numeric_load_locale(const char *name)
{
	(void)name;
	return (pbsd_setlocale_hook.numeric_ret);
}

int
__messages_load_locale(const char *name)
{
	(void)name;
	return (pbsd_setlocale_hook.messages_ret);
}

static struct xlocale_component *
pbsd_make_component(const char *name, int fail)
{
	struct xlocale_component *c;

	if (fail)
		return (NULL);
	c = calloc(1, sizeof(*c));
	if (c == NULL)
		return (NULL);
	strncpy(c->locale, name, ENCODING_LEN);
	strncpy(c->version, "BSD 1.0\n", XLOCALE_DEF_VERSION_LEN - 1);
	return (c);
}

void *
__collate_load(const char *name, pbsd_locale_t l)
{
	(void)l;
	pbsd_ctor_hook.call_idx++;
	return (pbsd_make_component(name, pbsd_ctor_hook.collate_fail));
}

void *
__ctype_load(const char *name, pbsd_locale_t l)
{
	(void)l;
	pbsd_ctor_hook.call_idx++;
	return (pbsd_make_component(name, pbsd_ctor_hook.ctype_fail));
}

void *
__numeric_load(const char *name, pbsd_locale_t l)
{
	(void)l;
	pbsd_ctor_hook.call_idx++;
	return (pbsd_make_component(name, pbsd_ctor_hook.numeric_fail));
}

void *
__time_load(const char *name, pbsd_locale_t l)
{
	(void)l;
	pbsd_ctor_hook.call_idx++;
	return (pbsd_make_component(name, pbsd_ctor_hook.time_fail));
}

void *
__messages_load(const char *name, pbsd_locale_t l)
{
	(void)l;
	pbsd_ctor_hook.call_idx++;
	return (pbsd_make_component(name, pbsd_ctor_hook.messages_fail));
}

int
_open(const char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	if (pbsd_rune_hook.open_fail) {
		errno = pbsd_rune_hook.open_errno;
		return (-1);
	}
	return (100);
}

int
_fstat(int fd, struct stat *st)
{
	(void)fd;
	if (pbsd_rune_hook.fstat_fail) {
		errno = pbsd_rune_hook.fstat_errno;
		return (-1);
	}
	memset(st, 0, sizeof(*st));
	st->st_size = pbsd_rune_hook.file_size;
	return (0);
}

int
_close(int fd)
{
	(void)fd;
	pbsd_rune_hook.close_count++;
	return (0);
}

void *
__real_malloc(size_t);

void *
__wrap_malloc(size_t n)
{
	if (pbsd_rune_hook.malloc_fail) {
		errno = ENOMEM;
		return (NULL);
	}
	return (__real_malloc(n));
}

/* --- lmonetary.c --- */
#define cnv ref_cnv
#define destruct_monetary ref_destruct_monetary
#define monetary_load_locale_l ref_monetary_load_locale_l
#define __monetary_load_locale ref___monetary_load_locale
#define __monetary_load ref___monetary_load
#define __get_current_monetary_locale ref___get_current_monetary_locale

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2000, 2001 Alexey Zelkin <phantom@FreeBSD.org>
 * All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
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





static char	empty[] = "";
static char	numempty[] = { CHAR_MAX, '\0'};

static const struct lc_monetary_T _C_monetary_locale = {
	empty,		/* int_curr_symbol */
	empty,		/* currency_symbol */
	empty,		/* mon_decimal_point */
	empty,		/* mon_thousands_sep */
	numempty,	/* mon_grouping */
	empty,		/* positive_sign */
	empty,		/* negative_sign */
	numempty,	/* int_frac_digits */
	numempty,	/* frac_digits */
	numempty,	/* p_cs_precedes */
	numempty,	/* p_sep_by_space */
	numempty,	/* n_cs_precedes */
	numempty,	/* n_sep_by_space */
	numempty,	/* p_sign_posn */
	numempty,	/* n_sign_posn */
	numempty,	/* int_p_cs_precedes */
	numempty,	/* int_n_cs_precedes */
	numempty,	/* int_p_sep_by_space */
	numempty,	/* int_n_sep_by_space */
	numempty,	/* int_p_sign_posn */
	numempty	/* int_n_sign_posn */
};

struct xlocale_monetary __xlocale_global_monetary;

static char
cnv(const char *str)
{
	int i = strtol(str, NULL, 10);

	if (i == -1)
		i = CHAR_MAX;
	return ((char)i);
}

static void
destruct_monetary(void *v)
{
	struct xlocale_monetary *l = v;
	if (l->buffer)
		free(l->buffer);
	free(l);
}

static int
monetary_load_locale_l(struct xlocale_monetary *loc, int *using_locale,
    int *changed, const char *name)
{
	int ret;
	struct lc_monetary_T *l = &loc->locale;

	ret = __part_load_locale(name, using_locale,
	    &loc->buffer, "LC_MONETARY",
	    LCMONETARY_SIZE_FULL, LCMONETARY_SIZE_MIN,
	    (const char **)l);
	if (ret == _LDP_LOADED) {
		l->mon_grouping =
		     __fix_locale_grouping_str(l->mon_grouping);

#define M_ASSIGN_CHAR(NAME) (((char *)l->NAME)[0] = \
			     cnv(l->NAME))

		M_ASSIGN_CHAR(int_frac_digits);
		M_ASSIGN_CHAR(frac_digits);
		M_ASSIGN_CHAR(p_cs_precedes);
		M_ASSIGN_CHAR(p_sep_by_space);
		M_ASSIGN_CHAR(n_cs_precedes);
		M_ASSIGN_CHAR(n_sep_by_space);
		M_ASSIGN_CHAR(p_sign_posn);
		M_ASSIGN_CHAR(n_sign_posn);

		/*
		 * The six additional C99 international monetary formatting
		 * parameters default to the national parameters when
		 * reading FreeBSD LC_MONETARY data files.
		 */
#define	M_ASSIGN_ICHAR(NAME)						\
		do {							\
			if (l->int_##NAME == NULL)	\
				l->int_##NAME =		\
				    l->NAME;		\
			else						\
				M_ASSIGN_CHAR(int_##NAME);		\
		} while (0)

		M_ASSIGN_ICHAR(p_cs_precedes);
		M_ASSIGN_ICHAR(n_cs_precedes);
		M_ASSIGN_ICHAR(p_sep_by_space);
		M_ASSIGN_ICHAR(n_sep_by_space);
		M_ASSIGN_ICHAR(p_sign_posn);
		M_ASSIGN_ICHAR(n_sign_posn);
	}
	if (ret != _LDP_ERROR)
		atomic_store_rel_int(changed, 1);
	return (ret);
}

int
__monetary_load_locale(const char *name)
{
	return (monetary_load_locale_l(&__xlocale_global_monetary,
	    &__xlocale_global_locale.using_monetary_locale,
	    &__xlocale_global_locale.monetary_locale_changed, name));
}

void *
__monetary_load(const char *name, locale_t l)
{
	struct xlocale_monetary *new = calloc(sizeof(struct xlocale_monetary),
	    1);
	if (new == NULL)
		return (NULL);
	new->header.header.destructor = destruct_monetary;
	if (monetary_load_locale_l(new, &l->using_monetary_locale,
	    &l->monetary_locale_changed, name) == _LDP_ERROR) {
		xlocale_release(new);
		return (NULL);
	}
	return (new);
}

struct lc_monetary_T *
__get_current_monetary_locale(locale_t loc)
{
	return (loc->using_monetary_locale ?
	    &((struct xlocale_monetary*)loc->components[XLC_MONETARY])->locale :
	    (struct lc_monetary_T *)&_C_monetary_locale);
}


#undef __monetary_load_locale
#undef __monetary_load
#undef __get_current_monetary_locale

/* --- rune.c --- */
#define mmap pbsd_mmap
#define munmap pbsd_munmap
#define _Read_RuneMagi ref__Read_RuneMagi

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2014 Garrett D'Amore <garrett@damore.org>
 * Copyright 2010 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
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



_RuneLocale *
_Read_RuneMagi(const char *fname)
{
	char *fdata, *data;
	void *lastp;
	_FileRuneLocale *frl;
	_RuneLocale *rl;
	_FileRuneEntry *frr;
	_RuneEntry *rr;
	struct stat sb;
	int x, saverr;
	void *variable;
	_FileRuneEntry *runetype_ext_ranges;
	_FileRuneEntry *maplower_ext_ranges;
	_FileRuneEntry *mapupper_ext_ranges;
	int runetype_ext_len = 0;
	int fd;

	if ((fd = _open(fname, O_RDONLY | O_CLOEXEC)) < 0) {
		errno = EINVAL;
		return (NULL);
	}

	if (_fstat(fd, &sb) < 0) {
		(void) _close(fd);
		errno = EINVAL;
		return (NULL);
	}

	if ((size_t)sb.st_size < sizeof (_FileRuneLocale)) {
		(void) _close(fd);
		errno = EINVAL;
		return (NULL);
	}


	fdata = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	(void) _close(fd);
	if (fdata == MAP_FAILED) {
		errno = EINVAL;
		return (NULL);
	}

	frl = (_FileRuneLocale *)(void *)fdata;
	lastp = fdata + sb.st_size;

	variable = frl + 1;

	if (memcmp(frl->magic, _FILE_RUNE_MAGIC_1, sizeof (frl->magic))) {
		goto invalid;
	}

	runetype_ext_ranges = (_FileRuneEntry *)variable;
	variable = runetype_ext_ranges + frl->runetype_ext_nranges;
	if (variable > lastp) {
		goto invalid;
	}

	maplower_ext_ranges = (_FileRuneEntry *)variable;
	variable = maplower_ext_ranges + frl->maplower_ext_nranges;
	if (variable > lastp) {
		goto invalid;
	}

	mapupper_ext_ranges = (_FileRuneEntry *)variable;
	variable = mapupper_ext_ranges + frl->mapupper_ext_nranges;
	if (variable > lastp) {
		goto invalid;
	}

	frr = runetype_ext_ranges;
	for (x = 0; x < frl->runetype_ext_nranges; ++x) {
		uint32_t *types;

		if (frr[x].map == 0) {
			int len = frr[x].max - frr[x].min + 1;
			types = variable;
			variable = types + len;
			runetype_ext_len += len;
			if (variable > lastp) {
				goto invalid;
			}
		}
	}

	if ((char *)variable + frl->variable_len > (char *)lastp) {
		goto invalid;
	}

	/*
	 * Convert from disk format to host format.
	 */
	data = malloc(sizeof(_RuneLocale) +
	    (frl->runetype_ext_nranges + frl->maplower_ext_nranges +
	    frl->mapupper_ext_nranges) * sizeof(_RuneEntry) +
	    runetype_ext_len * sizeof(*rr->__types) + frl->variable_len);
	if (data == NULL) {
		saverr = errno;
		munmap(fdata, sb.st_size);
		errno = saverr;
		return (NULL);
	}

	rl = (_RuneLocale *)data;
	rl->__variable = rl + 1;

	memcpy(rl->__magic, _RUNE_MAGIC_1, sizeof(rl->__magic));
	memcpy(rl->__encoding, frl->encoding, sizeof(rl->__encoding));

	rl->__variable_len = frl->variable_len;
	rl->__runetype_ext.__nranges = frl->runetype_ext_nranges;
	rl->__maplower_ext.__nranges = frl->maplower_ext_nranges;
	rl->__mapupper_ext.__nranges = frl->mapupper_ext_nranges;

	for (x = 0; x < _CACHED_RUNES; ++x) {
		rl->__runetype[x] = frl->runetype[x];
		rl->__maplower[x] = frl->maplower[x];
		rl->__mapupper[x] = frl->mapupper[x];
	}

	rl->__runetype_ext.__ranges = (_RuneEntry *)rl->__variable;
	rl->__variable = rl->__runetype_ext.__ranges +
	    rl->__runetype_ext.__nranges;

	rl->__maplower_ext.__ranges = (_RuneEntry *)rl->__variable;
	rl->__variable = rl->__maplower_ext.__ranges +
	    rl->__maplower_ext.__nranges;

	rl->__mapupper_ext.__ranges = (_RuneEntry *)rl->__variable;
	rl->__variable = rl->__mapupper_ext.__ranges +
	    rl->__mapupper_ext.__nranges;

	variable = mapupper_ext_ranges + frl->mapupper_ext_nranges;
	frr = runetype_ext_ranges;
	rr = rl->__runetype_ext.__ranges;
	for (x = 0; x < rl->__runetype_ext.__nranges; ++x) {
		uint32_t *types;

		rr[x].__min = frr[x].min;
		rr[x].__max = frr[x].max;
		rr[x].__map = frr[x].map;
		if (rr[x].__map == 0) {
			int len = rr[x].__max - rr[x].__min + 1;
			types = variable;
			variable = types + len;
			rr[x].__types = rl->__variable;
			rl->__variable = rr[x].__types + len;
			while (len-- > 0)
				rr[x].__types[len] = types[len];
		} else
			rr[x].__types = NULL;
	}

	frr = maplower_ext_ranges;
	rr = rl->__maplower_ext.__ranges;
	for (x = 0; x < rl->__maplower_ext.__nranges; ++x) {
		rr[x].__min = frr[x].min;
		rr[x].__max = frr[x].max;
		rr[x].__map = frr[x].map;
	}

	frr = mapupper_ext_ranges;
	rr = rl->__mapupper_ext.__ranges;
	for (x = 0; x < rl->__mapupper_ext.__nranges; ++x) {
		rr[x].__min = frr[x].min;
		rr[x].__max = frr[x].max;
		rr[x].__map = frr[x].map;
	}

	memcpy(rl->__variable, variable, rl->__variable_len);
	munmap(fdata, sb.st_size);

	/*
	 * Go out and zero pointers that should be zero.
	 */
	if (!rl->__variable_len)
		rl->__variable = NULL;

	if (!rl->__runetype_ext.__nranges)
		rl->__runetype_ext.__ranges = NULL;

	if (!rl->__maplower_ext.__nranges)
		rl->__maplower_ext.__ranges = NULL;

	if (!rl->__mapupper_ext.__nranges)
		rl->__mapupper_ext.__ranges = NULL;

	return (rl);

invalid:
	munmap(fdata, sb.st_size);
	errno = EINVAL;
	return (NULL);
}

#undef _Read_RuneMagi

/* --- setlocale.c --- */
#define __monetary_load_locale ref___monetary_load_locale
#define currentlocale ref_currentlocale
#define loadlocale ref_loadlocale
#define setlocale ref_setlocale
#define __get_locale_env ref___get_locale_env
#define __detect_path_locale ref___detect_path_locale

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1996 - 2002 FreeBSD Project
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
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
 * Category names for getenv()
 */
static const char categories[_LC_LAST][12] = {
	"LC_ALL",
	"LC_COLLATE",
	"LC_CTYPE",
	"LC_MONETARY",
	"LC_NUMERIC",
	"LC_TIME",
	"LC_MESSAGES",
};

/*
 * Current locales for each category
 */
static char current_categories[_LC_LAST][ENCODING_LEN + 1] = {
	"C",
	"C",
	"C",
	"C",
	"C",
	"C",
	"C",
};

/*
 * Path to locale storage directory
 */

/*
 * The locales we are going to try and load
 */
static char new_categories[_LC_LAST][ENCODING_LEN + 1];
static char saved_categories[_LC_LAST][ENCODING_LEN + 1];

static char current_locale_string[_LC_LAST * (ENCODING_LEN + 1/*"/"*/ + 1)];

static char *currentlocale(void);
static char *loadlocale(int);
const char *__get_locale_env(int);

char *
setlocale(int category, const char *locale)
{
	int i, j, len, saverr;
	const char *env, *r;

	if (category < LC_ALL || category >= _LC_LAST) {
		errno = EINVAL;
		return (NULL);
	}
	if (locale == NULL)
		return (category != LC_ALL ?
		    current_categories[category] : currentlocale());

	/*
	 * Default to the current locale for everything.
	 */
	for (i = 1; i < _LC_LAST; ++i)
		(void)strcpy(new_categories[i], current_categories[i]);

	/*
	 * Now go fill up new_categories from the locale argument
	 */
	if (!*locale) {
		if (category == LC_ALL) {
			for (i = 1; i < _LC_LAST; ++i) {
				env = __get_locale_env(i);
				if (strlen(env) > ENCODING_LEN) {
					errno = EINVAL;
					return (NULL);
				}
				(void)strcpy(new_categories[i], env);
			}
		} else {
			env = __get_locale_env(category);
			if (strlen(env) > ENCODING_LEN) {
				errno = EINVAL;
				return (NULL);
			}
			(void)strcpy(new_categories[category], env);
		}
	} else if (category != LC_ALL) {
		if (strlen(locale) > ENCODING_LEN) {
			errno = EINVAL;
			return (NULL);
		}
		(void)strcpy(new_categories[category], locale);
	} else {
		if ((r = strchr(locale, '/')) == NULL) {
			if (strlen(locale) > ENCODING_LEN) {
				errno = EINVAL;
				return (NULL);
			}
			for (i = 1; i < _LC_LAST; ++i)
				(void)strcpy(new_categories[i], locale);
		} else {
			for (i = 1; r[1] == '/'; ++r)
				;
			if (!r[1]) {
				errno = EINVAL;
				return (NULL);	/* Hmm, just slashes... */
			}
			do {
				if (i == _LC_LAST)
					break;	/* Too many slashes... */
				if ((len = r - locale) > ENCODING_LEN) {
					errno = EINVAL;
					return (NULL);
				}
				(void)strlcpy(new_categories[i], locale,
				    len + 1);
				i++;
				while (*r == '/')
					r++;
				locale = r;
				while (*r && *r != '/')
					r++;
			} while (*locale);
			while (i < _LC_LAST) {
				(void)strcpy(new_categories[i],
				    new_categories[i - 1]);
				i++;
			}
		}
	}

	if (category != LC_ALL)
		return (loadlocale(category));

	for (i = 1; i < _LC_LAST; ++i) {
		(void)strcpy(saved_categories[i], current_categories[i]);
		if (loadlocale(i) == NULL) {
			saverr = errno;
			for (j = 1; j < i; j++) {
				(void)strcpy(new_categories[j],
				    saved_categories[j]);
				if (loadlocale(j) == NULL) {
					(void)strcpy(new_categories[j], "C");
					(void)loadlocale(j);
				}
			}
			errno = saverr;
			return (NULL);
		}
	}
	return (currentlocale());
}

static char *
currentlocale(void)
{
	int i;

	(void)strcpy(current_locale_string, current_categories[1]);

	for (i = 2; i < _LC_LAST; ++i)
		if (strcmp(current_categories[1], current_categories[i])) {
			for (i = 2; i < _LC_LAST; ++i) {
				(void)strcat(current_locale_string, "/");
				(void)strcat(current_locale_string,
				    current_categories[i]);
			}
			break;
		}
	return (current_locale_string);
}

static char *
loadlocale(int category)
{
	char *new = new_categories[category];
	char *old = current_categories[category];
	int (*func) (const char *);
	int saved_errno;

	if ((new[0] == '.' &&
	    (new[1] == '\0' || (new[1] == '.' && new[2] == '\0'))) ||
	    strchr(new, '/') != NULL) {
		errno = EINVAL;
		return (NULL);
	}
	saved_errno = errno;
	errno = __detect_path_locale();
	if (errno != 0)
		return (NULL);
	errno = saved_errno;

	switch (category) {
	case LC_CTYPE:
		func = __wrap_setrunelocale;
		break;
	case LC_COLLATE:
		func = __collate_load_tables;
		break;
	case LC_TIME:
		func = __time_load_locale;
		break;
	case LC_NUMERIC:
		func = __numeric_load_locale;
		break;
	case LC_MONETARY:
		func = __monetary_load_locale;
		break;
	case LC_MESSAGES:
		func = __messages_load_locale;
		break;
	default:
		errno = EINVAL;
		return (NULL);
	}

	if (strcmp(new, old) == 0)
		return (old);

	if (func(new) != _LDP_ERROR) {
		(void)strcpy(old, new);
		(void)strcpy(__xlocale_global_locale.components[category-1]->locale, new);
		return (old);
	}

	return (NULL);
}

const char *
__get_locale_env(int category)
{
	const char *env;

	/* 1. check LC_ALL. */
	env = getenv(categories[0]);

	/* 2. check LC_* */
	if (env == NULL || !*env)
		env = getenv(categories[category]);

	/* 3. check LANG */
	if (env == NULL || !*env)
		env = getenv("LANG");

	/* 4. if none is set, fall to "C" */
	if (env == NULL || !*env)
		env = "C";

	return (env);
}

/*
 * Detect locale storage location and store its value to _PathLocale variable
 */
int
__detect_path_locale(void)
{
	if (_PathLocale == NULL) {
		char *p = secure_getenv("PATH_LOCALE");

		if (p != NULL) {
			if (strlen(p) + 1/*"/"*/ + ENCODING_LEN +
			    1/*"/"*/ + CATEGORY_LEN >= PATH_MAX)
				return (ENAMETOOLONG);
			_PathLocale = strdup(p);
			if (_PathLocale == NULL)
				return (errno == 0 ? ENOMEM : errno);
		} else
			_PathLocale = _PATH_LOCALE;
	}
	return (0);
}

#undef setlocale
#undef __get_locale_env
#undef __detect_path_locale

/* --- xlocale.c --- */
#define __get_locale_env ref___get_locale_env
#define __detect_path_locale ref___detect_path_locale
#define __monetary_load ref___monetary_load
#define init_key ref_init_key
#define get_thread_locale ref_get_thread_locale
#define set_thread_locale ref_set_thread_locale
#define destruct_locale ref_destruct_locale
#define alloc_locale ref_alloc_locale
#define copyflags ref_copyflags
#define dupcomponent ref_dupcomponent
#define newlocale ref_newlocale
#define duplocale ref_duplocale
#define freelocale ref_freelocale
#define querylocale ref_querylocale
#define uselocale ref_uselocale

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * This software was developed by David Chisnall under sponsorship from
 * the FreeBSD Foundation.
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


/**
 * Each locale loader declares a global component.  This is used by setlocale()
 * and also by xlocale with LC_GLOBAL_LOCALE..
 */
extern struct xlocale_component __xlocale_global_collate;
extern struct xlocale_component __xlocale_global_ctype;
extern struct xlocale_component __xlocale_global_monetary;
extern struct xlocale_component __xlocale_global_numeric;
extern struct xlocale_component __xlocale_global_time;
extern struct xlocale_component __xlocale_global_messages;
/*
 * And another version for the statically-allocated C locale.  We only have
 * components for the parts that are expected to be sensible.
 */
extern struct xlocale_component __xlocale_C_collate;
extern struct xlocale_component __xlocale_C_ctype;

/*
 * The locale for this thread.
 */
_Thread_local locale_t __thread_locale;

/*
 * Flag indicating that one or more per-thread locales exist.
 */

/*
 * Private functions in setlocale.c.
 */
const char *__get_locale_env(int category);
int __detect_path_locale(void);

struct _xlocale __xlocale_global_locale = {
	{0},
	{
		&__xlocale_global_collate,
		&__xlocale_global_ctype,
		&__xlocale_global_monetary,
		&__xlocale_global_numeric,
		&__xlocale_global_time,
		&__xlocale_global_messages
	},
	1,
	0,
	1,
	0
};

struct _xlocale __xlocale_C_locale = {
	{0},
	{
		&__xlocale_C_collate,
		&__xlocale_C_ctype,
		0, 0, 0, 0
	},
	1,
	0,
	1,
	0
};

static void *(*constructors[])(const char *, locale_t) =
{
	__collate_load,
	__ctype_load,
	__monetary_load,
	__numeric_load,
	__time_load,
	__messages_load
};

static pthread_key_t locale_info_key;
static int fake_tls;
static locale_t thread_local_locale;

static void
init_key(void)
{
	int error;

	error = pthread_key_create(&locale_info_key, xlocale_release);
	if (error == 0) {
		pthread_setspecific(locale_info_key, (void*)42);
		if (pthread_getspecific(locale_info_key) == (void*)42) {
			pthread_setspecific(locale_info_key, 0);
		} else {
			fake_tls = 1;
		}
	} else {
		fake_tls = 1;
	}
	/* At least one per-thread locale has now been set. */
	__has_thread_locale = 1;
	__detect_path_locale();
}

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

static locale_t
get_thread_locale(void)
{

	_once(&once_control, init_key);
	
	return (fake_tls ? thread_local_locale :
	    pthread_getspecific(locale_info_key));
}

static void
set_thread_locale(locale_t loc)
{
	locale_t l = (loc == LC_GLOBAL_LOCALE) ? 0 : loc;

	_once(&once_control, init_key);
	
	if (NULL != l) {
		xlocale_retain((struct xlocale_refcounted*)l);
	}
	locale_t old = get_thread_locale();
	if ((NULL != old) && (l != old)) {
		xlocale_release((struct xlocale_refcounted*)old);
	}
	if (fake_tls) {
		thread_local_locale = l;
	} else {
		pthread_setspecific(locale_info_key, l);
	}
	__thread_locale = l;
	__set_thread_rune_locale(loc);
}

/**
 * Clean up a locale, once its reference count reaches zero.  This function is
 * called by xlocale_release(), it should not be called directly.
 */
static void
destruct_locale(void *l)
{
	locale_t loc = l;

	for (int type=0 ; type<XLC_LAST ; type++) {
		if (loc->components[type]) {
			xlocale_release(loc->components[type]);
		}
	}
	if (loc->csym) {
		free(loc->csym);
	}
	free(l);
}

/**
 * Allocates a new, uninitialised, locale.
 */
static locale_t
alloc_locale(void)
{
	locale_t new = calloc(sizeof(struct _xlocale), 1);

	if (new == NULL)
		return (NULL);

	new->header.destructor = destruct_locale;
	new->monetary_locale_changed = 1;
	new->numeric_locale_changed = 1;
	return (new);
}

static void
copyflags(locale_t new, locale_t old)
{
	new->using_monetary_locale = old->using_monetary_locale;
	new->using_numeric_locale = old->using_numeric_locale;
	new->using_time_locale = old->using_time_locale;
	new->using_messages_locale = old->using_messages_locale;
}

static int
dupcomponent(int type, locale_t base, locale_t new)
{
	/* Always copy from the global locale, since it has mutable components.
	 */
	struct xlocale_component *src = base->components[type];

	if (&__xlocale_global_locale == base) {
		new->components[type] = constructors[type](src->locale, new);
		if (new->components[type]) {
			strncpy(new->components[type]->locale, src->locale,
			    ENCODING_LEN);
			strncpy(new->components[type]->version, src->version,
			    XLOCALE_DEF_VERSION_LEN);
		}
	} else if (base->components[type]) {
		new->components[type] = xlocale_retain(base->components[type]);
	} else {
		/* If the component was NULL, return success - if base is a
		 * valid locale then the flag indicating that this isn't
		 * present should be set.  If it isn't a valid locale, then
		 * we're stuck anyway. */
		return 1;
	}
	return (0 != new->components[type]);
}

/*
 * Public interfaces.  These are the five public functions described by the
 * xlocale interface.  
 */

locale_t
newlocale(int mask, const char *locale, locale_t base)
{
	locale_t orig_base;
	int type;
	const char *realLocale = locale;
	int useenv = 0;
	int success = 1;

	locale_t new = alloc_locale();
	if (NULL == new) {
		return (NULL);
	}

	_once(&once_control, init_key);

	orig_base = base;
	FIX_LOCALE(base);
	copyflags(new, base);

	if (NULL == locale) {
		realLocale = "C";
	} else if ('\0' == locale[0]) {
		useenv = 1;
	}

	for (type=0 ; type<XLC_LAST ; type++) {
		if (mask & 1) {
			if (useenv) {
				realLocale = __get_locale_env(type + 1);
			}
			new->components[type] =
			     constructors[type](realLocale, new);
			if (new->components[type]) {
				strncpy(new->components[type]->locale,
				     realLocale, ENCODING_LEN);
			} else {
				success = 0;
				break;
			}
		} else {
			if (!dupcomponent(type, base, new)) {
				success = 0;
				break;
			}
		}
		mask >>= 1;
	}
	if (0 == success) {
		xlocale_release(new);
		new = NULL;
	} else if (base == orig_base) {
		xlocale_release(base);
	}

	return (new);
}

locale_t
duplocale(locale_t base)
{
	locale_t new = alloc_locale();
	int type;

	if (NULL == new) {
		return (NULL);
	}
	
	_once(&once_control, init_key);

	FIX_LOCALE(base);
	copyflags(new, base);

	for (type=0 ; type<XLC_LAST ; type++) {
		dupcomponent(type, base, new);
	}

	return (new);
}

/*
 * Free a locale_t.  This is quite a poorly named function.  It actually
 * disclaims a reference to a locale_t, rather than freeing it.  
 */
void
freelocale(locale_t loc)
{

	/*
	 * Fail if we're passed something that isn't a locale. If we're
	 * passed the global locale, pretend that we freed it but don't
	 * actually do anything.
	 */
	if (loc != NULL && loc != LC_GLOBAL_LOCALE &&
	    loc != &__xlocale_global_locale)
		xlocale_release(loc);
}

/*
 * Returns the name or version of the locale for a particular component of a
 * locale_t.
 */
const char *
querylocale(int mask, locale_t loc)
{
	int type = ffs(mask & ~LC_VERSION_MASK) - 1;
	FIX_LOCALE(loc);
	if (type >= XLC_LAST)
		return (NULL);
	if (mask & LC_VERSION_MASK) {
		if (loc->components[type])
			return (loc->components[type]->version);
		return ("");
	} else {
		if (loc->components[type])
			return (loc->components[type]->locale);
		return ("C");
	}
}

/*
 * Installs the specified locale_t as this thread's locale.
 */
locale_t
uselocale(locale_t loc)
{
	locale_t old = get_thread_locale();
	if (NULL != loc) {
		set_thread_locale(loc);
	}
	return (old ? old : LC_GLOBAL_LOCALE);
}


#undef newlocale
#undef duplocale
#undef freelocale
#undef querylocale
#undef uselocale
