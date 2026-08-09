/*
 * PBSD batch b0229 -- oracle (specification).
 *
 * The original HardenedBSD C sources of this batch, concatenated, with every
 * function renamed with a "ref_" prefix.  The function bodies are unmodified;
 * only the names of the functions -- and the names used to call them -- carry
 * the prefix.  Functions that were "static" (or "static __inline") in the
 * original have had the storage class removed so that the differential
 * harness can call them; their bodies are untouched.
 *
 * Batch sources (in this order):
 *	lib/libc/iconv/citrus_db.c
 *	lib/libc/iconv/citrus_db_factory.c
 *	lib/libc/iconv/citrus_lookup.c
 *	lib/libc/iconv/citrus_mapper.c
 *
 * The support layer at the top of this file (citrus_region.h,
 * citrus_memstream.[ch], citrus_bcs.[ch], citrus_mmap.c, citrus_db_hash.c and
 * the citrus_namespace.h shorthands) is reproduced under its original names.
 * It is not part of the batch -- the batch sources simply do not compile
 * without it.
 */

/*
 * strict -std=c11 hides the POSIX/BSD interfaces (open(2), mmap(2), strdup(3),
 * be32toh(3), ...) that the batch sources use.
 */
#ifndef _GNU_SOURCE
#define	_GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Defines this (non-FreeBSD) toolchain does not provide. */
#ifndef EFTYPE
#define	EFTYPE		79		/* Inappropriate file type or format */
#endif
#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * 8)
#endif
#ifndef PATH_MAX
#define	PATH_MAX	1024
#endif
#ifndef __DECONST
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif
#ifndef __packed
#define	__packed	__attribute__((__packed__))
#endif
#ifndef __inline
#define	__inline	inline
#endif

/*
 * ------------------------------------------------------------------------
 * support layer: citrus_namespace.h shorthands
 * ------------------------------------------------------------------------
 */

/* citrus_bcs */
#define	_bcs_isblank		_citrus_bcs_isblank
#define	_bcs_iseol		_citrus_bcs_iseol
#define	_bcs_isspace		_citrus_bcs_isspace
#define	_bcs_islower		_citrus_bcs_islower
#define	_bcs_isupper		_citrus_bcs_isupper
#define	_bcs_skip_nonws_len	_citrus_bcs_skip_nonws_len
#define	_bcs_skip_ws_len	_citrus_bcs_skip_ws_len
#define	_bcs_strncasecmp	_citrus_bcs_strncasecmp
#define	_bcs_tolower		_citrus_bcs_tolower
#define	_bcs_toupper		_citrus_bcs_toupper
#define	_bcs_trunc_rws_len	_citrus_bcs_trunc_rws_len
#define	_bcs_convert_to_lower	_citrus_bcs_convert_to_lower

/* citrus_db */
#define	_db_open		_citrus_db_open
#define	_db_close		_citrus_db_close
#define	_db_lookup_by_s		_citrus_db_lookup_by_string
#define	_db_hash_std		_citrus_db_hash_std
#define	_db_get_num_entries	_citrus_db_get_number_of_entries
#define	_db_get_entry		_citrus_db_get_entry
#define	_db_locator		_citrus_db_locator
#define	_db_locator_init	_citrus_db_locator_init

/* citrus_db_factory */
#define	_db_factory		_citrus_db_factory

/* citrus_memstream */
#define	_memstream		_citrus_memory_stream
#define	_memstream_getln	_citrus_memory_stream_getln
#define	_memstream_matchline	_citrus_memory_stream_matchline
#define	_memstream_bind		_citrus_memory_stream_bind
#define	_memstream_seek		_citrus_memory_stream_seek
#define	_memstream_remainder	_citrus_memory_stream_remainder
#define	_memstream_getregion	_citrus_memory_stream_getregion

/* citrus_mmap */
#define	_map_file		_citrus_map_file
#define	_unmap_file		_citrus_unmap_file

/* citrus_region.h */
#define	_region			_citrus_region
#define	_region_init		_citrus_region_init
#define	_region_head		_citrus_region_head
#define	_region_size		_citrus_region_size
#define	_region_offset		_citrus_region_offset

/*
 * ------------------------------------------------------------------------
 * support layer: citrus_region.h
 * ------------------------------------------------------------------------
 */

struct _citrus_region {
	void	*r_head;
	size_t	 r_size;
};

static __inline void
_citrus_region_init(struct _citrus_region *r, void *h, size_t sz)
{

	r->r_head = h;
	r->r_size = sz;
}

static __inline void *
_citrus_region_head(const struct _citrus_region *r)
{

	return (r->r_head);
}

static __inline size_t
_citrus_region_size(const struct _citrus_region *r)
{

	return (r->r_size);
}

static __inline void *
_citrus_region_offset(const struct _citrus_region *r, size_t pos)
{

	return ((void *)((uint8_t *)r->r_head + pos));
}

/*
 * ------------------------------------------------------------------------
 * support layer: citrus_bcs.h / citrus_bcs.c
 * ------------------------------------------------------------------------
 */

#define _CITRUS_BCS_PRED(_name_, _cond_) \
static __inline int _citrus_bcs_##_name_(uint8_t c) { return (_cond_); }

_CITRUS_BCS_PRED(isblank, c == ' ' || c == '\t')
_CITRUS_BCS_PRED(iseol, c == '\n' || c == '\r')
_CITRUS_BCS_PRED(isspace, _citrus_bcs_isblank(c) || _citrus_bcs_iseol(c) ||
    c == '\f' || c == '\v')
_CITRUS_BCS_PRED(isupper, c >= 'A' && c <= 'Z')
_CITRUS_BCS_PRED(islower, c >= 'a' && c <= 'z')

static __inline uint8_t
_citrus_bcs_toupper(uint8_t c)
{

	return (_citrus_bcs_islower(c) ? (c - 'a' + 'A') : c);
}

static __inline uint8_t
_citrus_bcs_tolower(uint8_t c)
{

	return (_citrus_bcs_isupper(c) ? (c - 'A' + 'a') : c);
}

int
_citrus_bcs_strncasecmp(const char * __restrict str1,
    const char * __restrict str2, size_t sz)
{
	int c1, c2;

	c1 = c2 = 1;

	while (c1 && c2 && c1 == c2 && sz != 0) {
		c1 = _bcs_toupper(*str1++);
		c2 = _bcs_toupper(*str2++);
		sz--;
	}

	return ((c1 == c2) ? 0 : ((c1 > c2) ? 1 : -1));
}

const char *
_citrus_bcs_skip_ws_len(const char * __restrict p, size_t * __restrict len)
{

	while (*len > 0 && *p && _bcs_isspace(*p)) {
		p++;
		(*len)--;
	}

	return (p);
}

const char *
_citrus_bcs_skip_nonws_len(const char * __restrict p, size_t * __restrict len)
{

	while (*len > 0 && *p && !_bcs_isspace(*p)) {
		p++;
		(*len)--;
	}

	return (p);
}

void
_citrus_bcs_trunc_rws_len(const char * __restrict p, size_t * __restrict len)
{

	while (*len > 0 && _bcs_isspace(p[*len - 1]))
		(*len)--;
}

void
_citrus_bcs_convert_to_lower(char *s)
{

	while (*s) {
		*s = _bcs_tolower(*s);
		s++;
	}
}

/*
 * ------------------------------------------------------------------------
 * support layer: citrus_memstream.h / citrus_memstream.c
 * ------------------------------------------------------------------------
 */

struct _citrus_memory_stream {
	struct _citrus_region	ms_region;
	size_t			ms_pos;
};

static __inline void
_citrus_memory_stream_bind(struct _citrus_memory_stream * __restrict ms,
    const struct _citrus_region * __restrict r)
{

	ms->ms_region = *r;
	ms->ms_pos = 0;
}

static __inline size_t
_citrus_memory_stream_remainder(struct _citrus_memory_stream *ms)
{
	size_t sz;

	sz = _citrus_region_size(&ms->ms_region);
	if (ms->ms_pos>sz)
		return (0);
	return (sz-ms->ms_pos);
}

static __inline int
_citrus_memory_stream_seek(struct _citrus_memory_stream *ms, size_t pos, int w)
{
	size_t sz;

	sz = _citrus_region_size(&ms->ms_region);

	switch (w) {
	case SEEK_SET:
		if (pos >= sz)
			return (-1);
		ms->ms_pos = pos;
		break;
	case SEEK_CUR:
		pos += (ssize_t)ms->ms_pos;
		if (pos >= sz)
			return (-1);
		ms->ms_pos = pos;
		break;
	case SEEK_END:
		if (sz < pos)
			return (-1);
		ms->ms_pos = sz - pos;
		break;
	}
	return (0);
}

static __inline void *
_citrus_memory_stream_getregion(struct _citrus_memory_stream *ms,
    struct _citrus_region *r, size_t sz)
{
	void *ret;

	if (ms->ms_pos + sz > _citrus_region_size(&ms->ms_region))
		return (NULL);

	ret = _citrus_region_offset(&ms->ms_region, ms->ms_pos);
	ms->ms_pos += sz;
	if (r)
		_citrus_region_init(r, ret, sz);

	return (ret);
}

const char *
_citrus_memory_stream_getln(struct _citrus_memory_stream * __restrict ms,
    size_t * __restrict rlen)
{
	const uint8_t *h, *p;
	size_t i, ret;

	if (ms->ms_pos>=_region_size(&ms->ms_region))
		return (NULL);

	h = p = (uint8_t *)_region_offset(&ms->ms_region, ms->ms_pos);
	ret = 0;
	for (i = _region_size(&ms->ms_region) - ms->ms_pos; i > 0; i--) {
		ret++;
		if (_bcs_iseol(*p))
			break;
		p++;
	}

	ms->ms_pos += ret;
	*rlen = ret;
	return ((const char *)h);
}

#define T_COMM	'#'

const char *
_citrus_memory_stream_matchline(struct _citrus_memory_stream * __restrict ms,
    const char * __restrict key, size_t * __restrict rlen, int iscasesensitive)
{
	const char *p, *q;
	size_t keylen, len;

	keylen = strlen(key);
	for(;;) {
		p = _citrus_memory_stream_getln(ms, &len);
		if (p == NULL)
			return (NULL);

		/* ignore comment */
		q = memchr(p, T_COMM, len);
		if (q) {
			len = q - p;
		}
		/* ignore trailing white space and newline */
		_bcs_trunc_rws_len(p, &len);
		if (len == 0)
			continue; /* ignore null line */

		/* skip white spaces at the head of the line */
		p = _bcs_skip_ws_len(p, &len);
		q = _bcs_skip_nonws_len(p, &len);

		if ((size_t)(q - p) == keylen) {
			if (iscasesensitive) {
				if (memcmp(key, p, keylen) == 0)
					break; /* match */
			} else {
				if (_bcs_strncasecmp(key, p, keylen) == 0)
					break; /* match */
			}
		}
	}

	p = _bcs_skip_ws_len(q, &len);
	*rlen = len;

	return (p);
}

#undef T_COMM

/*
 * ------------------------------------------------------------------------
 * support layer: citrus_mmap.c
 * ------------------------------------------------------------------------
 */

int
_citrus_map_file(struct _citrus_region * __restrict r,
    const char * __restrict path)
{
	struct stat st;
	void *head;
	int fd, ret;

	ret = 0;

	_region_init(r, NULL, 0);

	if ((fd = open(path, O_RDONLY | O_CLOEXEC)) == -1)
		return (errno);

	if (fstat(fd, &st)  == -1) {
		ret = errno;
		goto error;
	}
	if (!S_ISREG(st.st_mode)) {
		ret = EOPNOTSUPP;
		goto error;
	}

	head = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_FILE|MAP_PRIVATE,
	    fd, (off_t)0);
	if (head == MAP_FAILED) {
		ret = errno;
		goto error;
	}
	_region_init(r, head, (size_t)st.st_size);

error:
	(void)close(fd);
	return (ret);
}

void
_citrus_unmap_file(struct _citrus_region *r)
{

	if (_region_head(r) != NULL) {
		(void)munmap(_region_head(r), _region_size(r));
		_region_init(r, NULL, 0);
	}
}

/*
 * ------------------------------------------------------------------------
 * support layer: citrus_db_file.h, citrus_db.h, citrus_db_factory.h,
 * citrus_lookup_file.h, citrus_db_hash.c
 * ------------------------------------------------------------------------
 */

#define _CITRUS_DB_MAGIC_SIZE	8
#define _CITRUS_DB_HEADER_SIZE	16
struct _citrus_db_header_x {
	char		dhx_magic[_CITRUS_DB_MAGIC_SIZE];
	uint32_t	dhx_num_entries;
	uint32_t	dhx_entry_offset;
} __packed;

struct _citrus_db_entry_x {
	uint32_t	dex_hash_value;
	uint32_t	dex_next_offset;
	uint32_t	dex_key_offset;
	uint32_t	dex_key_size;
	uint32_t	dex_data_offset;
	uint32_t	dex_data_size;
} __packed;
#define _CITRUS_DB_ENTRY_SIZE	24

#define _CITRUS_LOOKUP_MAGIC	"LOOKUP\0\0"

struct _citrus_db_locator {
	uint32_t	dl_hashval;
	size_t		dl_offset;
};

typedef uint32_t (*_citrus_db_hash_func_t)(struct _citrus_region *);

static __inline void
_citrus_db_locator_init(struct _citrus_db_locator *dl)
{

	dl->dl_hashval = 0;
	dl->dl_offset = 0;
}

uint32_t
_citrus_db_hash_std(struct _region *r)
{
	const uint8_t *p;
	uint32_t hash, tmp;
	size_t i;

	hash = 0;
	p = _region_head(r);

	for (i = _region_size(r); i > 0; i--) {
		hash <<= 4;
		hash += _bcs_tolower(*p);
		tmp = hash & 0xF0000000;
		if (tmp != 0) {
			hash ^= tmp;
			hash ^= tmp >> 24;
		}
		p++;
	}
	return (hash);
}

/*
 * ------------------------------------------------------------------------
 * support layer: citrus_hash.h, citrus_module mock, citrus_lock.h, strlcpy
 * ------------------------------------------------------------------------
 */

static int __isthreaded = 0;
#define WLOCK(lock) if (__isthreaded) pthread_rwlock_wrlock(lock);
#define UNLOCK(lock) if (__isthreaded) pthread_rwlock_unlock(lock);

static size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	const char *s = src;
	size_t n = siz;

	if (n != 0) {
		while (--n != 0) {
			if ((*dst++ = *s++) == '\0')
				break;
		}
	}
	if (n == 0) {
		if (siz != 0)
			*dst = '\0';
		while (*s++)
			;
	}
	return (s - src - 1);
}

#define _CITRUS_HASH_ENTRY(type) LIST_ENTRY(type)
#define _CITRUS_HASH_HEAD(headname, type, hashsize) \
	struct headname { LIST_HEAD(, type) chh_table[hashsize]; }
#define _CITRUS_HASH_INIT(head, hashsize) \
	do { int _i; for (_i = 0; _i < (hashsize); _i++) \
	    LIST_INIT(&(head)->chh_table[_i]); } while (0)
#define _CITRUS_HASH_REMOVE(elm, field) LIST_REMOVE(elm, field)
#define _CITRUS_HASH_INSERT(head, elm, field, hashval) \
	LIST_INSERT_HEAD(&(head)->chh_table[hashval], elm, field)
#define _CITRUS_HASH_SEARCH(head, elm, field, matchfunc, key, hashval) \
	do { LIST_FOREACH((elm), &(head)->chh_table[hashval], field) \
	    if ((matchfunc)((elm), (key)) == 0) break; } while (0)

typedef uint32_t _citrus_index_t;

typedef struct _citrus_module_rec {
	char	mr_name[64];
} *_citrus_module_t;

struct _citrus_mapper_traits {
	size_t	mt_state_size;
	size_t	mt_src_max;
	size_t	mt_dst_max;
};

struct _citrus_mapper_ops {
	int (*mo_init)(struct _citrus_mapper_area *__restrict,
	    struct _citrus_mapper *__restrict, const char *__restrict,
	    const void *__restrict, size_t, struct _citrus_mapper_traits *__restrict,
	    size_t);
	void (*mo_uninit)(struct _citrus_mapper *);
	int (*mo_convert)(struct _citrus_mapper *__restrict,
	    _citrus_index_t *__restrict, _citrus_index_t, void *__restrict);
	void (*mo_init_state)(void);
};

typedef int (*_citrus_mapper_getops_t)(struct _citrus_mapper_ops *);

struct _citrus_mapper {
	struct _citrus_mapper_ops	*cm_ops;
	void				*cm_closure;
	_citrus_module_t		 cm_module;
	struct _citrus_mapper_traits	*cm_traits;
	LIST_ENTRY(_citrus_mapper)	 cm_entry;
	int				 cm_refcount;
	char				*cm_key;
};

struct _citrus_mapper_area;

static struct {
	char			 mock_modname[64];
	_citrus_mapper_getops_t	 mock_getops;
} b0229_mock_state;

void
b0229_mock_reset(void)
{

	memset(&b0229_mock_state, 0, sizeof(b0229_mock_state));
}

void
b0229_mock_set_module(const char *name,
    int (*getops)(struct _citrus_mapper_ops *))
{

	strncpy(b0229_mock_state.mock_modname, name,
	    sizeof(b0229_mock_state.mock_modname) - 1);
	b0229_mock_state.mock_getops = getops;
}

int
_citrus_load_module(_citrus_module_t *mod, const char *name)
{

	if (strcmp(name, b0229_mock_state.mock_modname) != 0)
		return (ENOENT);
	*mod = malloc(sizeof(**mod));
	if (*mod == NULL)
		return (errno);
	strncpy((*mod)->mr_name, name, sizeof((*mod)->mr_name) - 1);
	return (0);
}

void
_citrus_unload_module(_citrus_module_t mod)
{

	free(mod);
}

void *
_citrus_find_getops(_citrus_module_t mod, const char *name, const char *kind)
{

	(void)mod;
	if (strcmp(name, b0229_mock_state.mock_modname) != 0)
		return (NULL);
	if (strcmp(kind, "mapper") != 0)
		return (NULL);
	return ((void *)b0229_mock_state.mock_getops);
}

int
_citrus_string_hash_func(const char *key, int hashsize)
{
	struct _citrus_region r;

	_region_init(&r, __DECONST(void *, key), strlen(key));
	return ((int)(_db_hash_std(&r) % (uint32_t)hashsize));
}

#define _string_hash_func _citrus_string_hash_func

/*
 * ------------------------------------------------------------------------
 * Every function of the batch is defined below under its original name with
 * a "ref_" prefix.  Calls between batch functions are redirected to the
 * prefixed definitions by the macros below, so that the function bodies
 * themselves are byte-for-byte the originals.
 * ------------------------------------------------------------------------
 */

/* citrus_db.c */
#define	_citrus_db_open			ref__citrus_db_open
#define	_citrus_db_close		ref__citrus_db_close
#define	_citrus_db_lookup		ref__citrus_db_lookup
#define	_citrus_db_lookup_by_string	ref__citrus_db_lookup_by_string
#define	_citrus_db_lookup8_by_string	ref__citrus_db_lookup8_by_string
#define	_citrus_db_lookup16_by_string	ref__citrus_db_lookup16_by_string
#define	_citrus_db_lookup32_by_string	ref__citrus_db_lookup32_by_string
#define	_citrus_db_lookup_string_by_string \
					ref__citrus_db_lookup_string_by_string
#define	_citrus_db_get_number_of_entries \
					ref__citrus_db_get_number_of_entries
#define	_citrus_db_get_entry		ref__citrus_db_get_entry

/* citrus_db_factory.c */
#define	ceilto				ref_ceilto
#define	put8				ref_put8
#define	put32				ref_put32
#define	putpad				ref_putpad
#define	dump_header			ref_dump_header
#define	_citrus_db_factory_create	ref__citrus_db_factory_create
#define	_citrus_db_factory_free		ref__citrus_db_factory_free
#define	_citrus_db_factory_add		ref__citrus_db_factory_add
#define	_citrus_db_factory_add_by_string \
					ref__citrus_db_factory_add_by_string
#define	_citrus_db_factory_add8_by_string \
					ref__citrus_db_factory_add8_by_string
#define	_citrus_db_factory_add16_by_string \
					ref__citrus_db_factory_add16_by_string
#define	_citrus_db_factory_add32_by_string \
					ref__citrus_db_factory_add32_by_string
#define	_citrus_db_factory_add_string_by_string \
				ref__citrus_db_factory_add_string_by_string
#define	_citrus_db_factory_calc_size	ref__citrus_db_factory_calc_size
#define	_citrus_db_factory_serialize	ref__citrus_db_factory_serialize

/* citrus_lookup.c */
#define	seq_get_num_entries_db		ref_seq_get_num_entries_db
#define	seq_next_db			ref_seq_next_db
#define	seq_lookup_db			ref_seq_lookup_db
#define	seq_close_db			ref_seq_close_db
#define	seq_open_db			ref_seq_open_db
#define	seq_next_plain			ref_seq_next_plain
#define	seq_get_num_entries_plain	ref_seq_get_num_entries_plain
#define	seq_lookup_plain		ref_seq_lookup_plain
#define	seq_close_plain			ref_seq_close_plain
#define	seq_open_plain			ref_seq_open_plain
#define	_citrus_lookup_seq_open		ref__citrus_lookup_seq_open
#define	_citrus_lookup_seq_rewind	ref__citrus_lookup_seq_rewind
#define	_citrus_lookup_seq_next		ref__citrus_lookup_seq_next
#define	_citrus_lookup_seq_lookup	ref__citrus_lookup_seq_lookup
#define	_citrus_lookup_get_number_of_entries \
				ref__citrus_lookup_get_number_of_entries
#define	_citrus_lookup_seq_close	ref__citrus_lookup_seq_close
#define	_citrus_lookup_simple		ref__citrus_lookup_simple

/* citrus_mapper.c */
#define	lookup_mapper_entry		ref_lookup_mapper_entry
#define	mapper_close			ref_mapper_close
#define	mapper_open			ref_mapper_open
#define	hash_func			ref_hash_func
#define	match_func			ref_match_func
#define	_mapper_close			ref__citrus_mapper_close
#define	_citrus_mapper_create_area	ref__citrus_mapper_create_area
#define	_citrus_mapper_open_direct	ref__citrus_mapper_open_direct
#define	_citrus_mapper_open		ref__citrus_mapper_open
#define	_citrus_mapper_close		ref__citrus_mapper_close
#define	_citrus_mapper_set_persistent	ref__citrus_mapper_set_persistent

/*
 * ========================================================================
 * lib/libc/iconv/citrus_db.c
 * ========================================================================
 */

/* $NetBSD: citrus_db.c,v 1.5 2008/02/09 14:56:20 junyoung Exp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c)2003 Citrus Project,
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

struct _citrus_db {
	struct _region		 db_region;
	_citrus_db_hash_func_t	 db_hashfunc;
	void			*db_hashfunc_closure;
};

int
ref__citrus_db_open(struct _citrus_db **rdb, struct _region *r, const char *magic,
    _citrus_db_hash_func_t hashfunc, void *hashfunc_closure)
{
	struct _citrus_db *db;
	struct _citrus_db_header_x *dhx;
	struct _memstream ms;

	_memstream_bind(&ms, r);

	/* sanity check */
	dhx = _memstream_getregion(&ms, NULL, sizeof(*dhx));
	if (dhx == NULL)
		return (EFTYPE);
	if (strncmp(dhx->dhx_magic, magic, _CITRUS_DB_MAGIC_SIZE) != 0)
		return (EFTYPE);
	if (_memstream_seek(&ms, be32toh(dhx->dhx_entry_offset), SEEK_SET))
		return (EFTYPE);

	if (be32toh(dhx->dhx_num_entries)*_CITRUS_DB_ENTRY_SIZE >
	    _memstream_remainder(&ms))
		return (EFTYPE);

	db = malloc(sizeof(*db));
	if (db == NULL)
		return (errno);
	db->db_region = *r;
	db->db_hashfunc = hashfunc;
	db->db_hashfunc_closure = hashfunc_closure;
	*rdb = db;

	return (0);
}

void
ref__citrus_db_close(struct _citrus_db *db)
{

	free(db);
}

int
ref__citrus_db_lookup(struct _citrus_db *db, struct _citrus_region *key,
    struct _citrus_region *data, struct _citrus_db_locator *dl)
{
	struct _citrus_db_entry_x *dex;
	struct _citrus_db_header_x *dhx;
	struct _citrus_region r;
	struct _memstream ms;
	uint32_t hashval, num_entries;
	size_t offset;

	_memstream_bind(&ms, &db->db_region);

	dhx = _memstream_getregion(&ms, NULL, sizeof(*dhx));
	num_entries = be32toh(dhx->dhx_num_entries);
	if (num_entries == 0)
		return (ENOENT);

	if (dl != NULL && dl->dl_offset>0) {
		hashval = dl->dl_hashval;
		offset = dl->dl_offset;
		if (offset >= _region_size(&db->db_region))
			return (ENOENT);
	} else {
		hashval = db->db_hashfunc(key)%num_entries;
		offset = be32toh(dhx->dhx_entry_offset) +
		    hashval * _CITRUS_DB_ENTRY_SIZE;
		if (dl)
			dl->dl_hashval = hashval;
	}
	do {
		/* seek to the next entry */
		if (_citrus_memory_stream_seek(&ms, offset, SEEK_SET))
			return (EFTYPE);
		/* get the entry record */
		dex = _memstream_getregion(&ms, NULL, _CITRUS_DB_ENTRY_SIZE);
		if (dex == NULL)
			return (EFTYPE);

		/* jump to next entry having the same hash value. */
		offset = be32toh(dex->dex_next_offset);

		/* save the current position */
		if (dl) {
			dl->dl_offset = offset;
			if (offset == 0)
				dl->dl_offset = _region_size(&db->db_region);
		}

		/* compare hash value. */
		if (be32toh(dex->dex_hash_value) != hashval)
			/* not found */
			break;
		/* compare key length */
		if (be32toh(dex->dex_key_size) == _region_size(key)) {
			/* seek to the head of the key. */
			if (_memstream_seek(&ms, be32toh(dex->dex_key_offset),
			    SEEK_SET))
				return (EFTYPE);
			/* get the region of the key */
			if (_memstream_getregion(&ms, &r,
			    _region_size(key)) == NULL)
				return (EFTYPE);
			/* compare key byte stream */
			if (memcmp(_region_head(&r), _region_head(key),
			    _region_size(key)) == 0) {
				/* match */
				if (_memstream_seek(
				    &ms, be32toh(dex->dex_data_offset),
				    SEEK_SET))
					return (EFTYPE);
				if (_memstream_getregion(
				    &ms, data,
				    be32toh(dex->dex_data_size)) == NULL)
					return (EFTYPE);
				return (0);
			}
		}
	} while (offset != 0);

	return (ENOENT);
}

int
ref__citrus_db_lookup_by_string(struct _citrus_db *db, const char *key,
    struct _citrus_region *data, struct _citrus_db_locator *dl)
{
	struct _region r;

	_region_init(&r, __DECONST(void *, key), strlen(key));

	return (_citrus_db_lookup(db, &r, data, dl));
}

int
ref__citrus_db_lookup8_by_string(struct _citrus_db *db, const char *key,
    uint8_t *rval, struct _citrus_db_locator *dl)
{
	struct _region r;
	int ret;

	ret = _citrus_db_lookup_by_string(db, key, &r, dl);
	if (ret)
		return (ret);

	if (_region_size(&r) != 1)
		return (EFTYPE);

	if (rval)
		memcpy(rval, _region_head(&r), 1);

	return (0);
}

int
ref__citrus_db_lookup16_by_string(struct _citrus_db *db, const char *key,
    uint16_t *rval, struct _citrus_db_locator *dl)
{
	struct _region r;
	int ret;
	uint16_t val;

	ret = _citrus_db_lookup_by_string(db, key, &r, dl);
	if (ret)
		return (ret);

	if (_region_size(&r) != 2)
		return (EFTYPE);

	if (rval) {
		memcpy(&val, _region_head(&r), 2);
		*rval = be16toh(val);
	}

	return (0);
}

int
ref__citrus_db_lookup32_by_string(struct _citrus_db *db, const char *key,
    uint32_t *rval, struct _citrus_db_locator *dl)
{
	struct _region r;
	uint32_t val;
	int ret;

	ret = _citrus_db_lookup_by_string(db, key, &r, dl);
	if (ret)
		return (ret);

	if (_region_size(&r) != 4)
		return (EFTYPE);

	if (rval) {
		memcpy(&val, _region_head(&r), 4);
		*rval = be32toh(val);
	}

	return (0);
}

int
ref__citrus_db_lookup_string_by_string(struct _citrus_db *db, const char *key,
    const char **rdata, struct _citrus_db_locator *dl)
{
	struct _region r;
	int ret;

	ret = _citrus_db_lookup_by_string(db, key, &r, dl);
	if (ret)
		return (ret);

	/* check whether the string is null terminated */
	if (_region_size(&r) == 0)
		return (EFTYPE);
	if (*((const char*)_region_head(&r)+_region_size(&r)-1) != '\0')
		return (EFTYPE);

	if (rdata)
		*rdata = _region_head(&r);

	return (0);
}

int
ref__citrus_db_get_number_of_entries(struct _citrus_db *db)
{
	struct _citrus_db_header_x *dhx;
	struct _memstream ms;

	_memstream_bind(&ms, &db->db_region);

	dhx = _memstream_getregion(&ms, NULL, sizeof(*dhx));
	return ((int)be32toh(dhx->dhx_num_entries));
}

int
ref__citrus_db_get_entry(struct _citrus_db *db, int idx, struct _region *key,
    struct _region *data)
{
	struct _citrus_db_entry_x *dex;
	struct _citrus_db_header_x *dhx;
	struct _memstream ms;
	uint32_t num_entries;
	size_t offset;

	_memstream_bind(&ms, &db->db_region);

	dhx = _memstream_getregion(&ms, NULL, sizeof(*dhx));
	num_entries = be32toh(dhx->dhx_num_entries);
	if (idx < 0 || (uint32_t)idx >= num_entries)
		return (EINVAL);

	/* seek to the next entry */
	offset = be32toh(dhx->dhx_entry_offset) + idx * _CITRUS_DB_ENTRY_SIZE;
	if (_citrus_memory_stream_seek(&ms, offset, SEEK_SET))
		return (EFTYPE);
	/* get the entry record */
	dex = _memstream_getregion(&ms, NULL, _CITRUS_DB_ENTRY_SIZE);
	if (dex == NULL)
		return (EFTYPE);
	/* seek to the head of the key. */
	if (_memstream_seek(&ms, be32toh(dex->dex_key_offset), SEEK_SET))
		return (EFTYPE);
	/* get the region of the key. */
	if (_memstream_getregion(&ms, key, be32toh(dex->dex_key_size))==NULL)
		return (EFTYPE);
	/* seek to the head of the data. */
	if (_memstream_seek(&ms, be32toh(dex->dex_data_offset), SEEK_SET))
		return (EFTYPE);
	/* get the region of the data. */
	if (_memstream_getregion(&ms, data, be32toh(dex->dex_data_size))==NULL)
		return (EFTYPE);

	return (0);
}

/*
 * ========================================================================
 * lib/libc/iconv/citrus_db_factory.c
 * ========================================================================
 */

/*	$NetBSD: citrus_db_factory.c,v 1.10 2013/09/14 13:05:51 joerg Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c)2003 Citrus Project,
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

struct _citrus_db_factory_entry {
	STAILQ_ENTRY(_citrus_db_factory_entry)	 de_entry;
	struct _citrus_db_factory_entry		*de_next;
	uint32_t				 de_hashvalue;
	struct _region				 de_key;
	int					 de_key_free;
	struct _region				 de_data;
	int					 de_data_free;
	int					de_idx;
};

struct _citrus_db_factory {
	size_t					 df_num_entries;
	STAILQ_HEAD(, _citrus_db_factory_entry)	 df_entries;
	size_t					 df_total_key_size;
	size_t					 df_total_data_size;
	uint32_t (*df_hashfunc)(struct _citrus_region *);
	void					*df_hashfunc_closure;
};

#define DB_ALIGN 16

int
ref__citrus_db_factory_create(struct _citrus_db_factory **rdf,
    _citrus_db_hash_func_t hashfunc, void *hashfunc_closure)
{
	struct _citrus_db_factory *df;

	df = malloc(sizeof(*df));
	if (df == NULL)
		return (errno);
	df->df_num_entries = 0;
	df->df_total_key_size = df->df_total_data_size = 0;
	STAILQ_INIT(&df->df_entries);
	df->df_hashfunc = hashfunc;
	df->df_hashfunc_closure = hashfunc_closure;

	*rdf = df;

	return (0);
}

void
ref__citrus_db_factory_free(struct _citrus_db_factory *df)
{
	struct _citrus_db_factory_entry *de;

	while ((de = STAILQ_FIRST(&df->df_entries)) != NULL) {
		STAILQ_REMOVE_HEAD(&df->df_entries, de_entry);
		if (de->de_key_free)
			free(_region_head(&de->de_key));
		if (de->de_data_free)
			free(_region_head(&de->de_data));
		free(de);
	}
	free(df);
}

size_t
ref_ceilto(size_t sz)
{
	return ((sz + DB_ALIGN - 1) & ~(DB_ALIGN - 1));
}

int
ref__citrus_db_factory_add(struct _citrus_db_factory *df, struct _region *key,
    int keyfree, struct _region *data, int datafree)
{
	struct _citrus_db_factory_entry *de;

	de = malloc(sizeof(*de));
	if (de == NULL)
		return (-1);

	de->de_hashvalue = df->df_hashfunc(key);
	de->de_key = *key;
	de->de_key_free = keyfree;
	de->de_data = *data;
	de->de_data_free = datafree;
	de->de_idx = -1;

	STAILQ_INSERT_TAIL(&df->df_entries, de, de_entry);
	df->df_total_key_size += _region_size(key);
	df->df_total_data_size += ceilto(_region_size(data));
	df->df_num_entries++;

	return (0);

}

int
ref__citrus_db_factory_add_by_string(struct _citrus_db_factory *df,
    const char *key, struct _citrus_region *data, int datafree)
{
	struct _region r;
	char *tmp;

	tmp = strdup(key);
	if (tmp == NULL)
		return (errno);
	_region_init(&r, tmp, strlen(key));
	return _citrus_db_factory_add(df, &r, 1, data, datafree);
}

int
ref__citrus_db_factory_add8_by_string(struct _citrus_db_factory *df,
    const char *key, uint8_t val)
{
	struct _region r;
	uint8_t *p;

	p = malloc(sizeof(*p));
	if (p == NULL)
		return (errno);
	*p = val;
	_region_init(&r, p, 1);
	return (_citrus_db_factory_add_by_string(df, key, &r, 1));
}

int
ref__citrus_db_factory_add16_by_string(struct _citrus_db_factory *df,
    const char *key, uint16_t val)
{
	struct _region r;
	uint16_t *p;

	p = malloc(sizeof(*p));
	if (p == NULL)
		return (errno);
	*p = htons(val);
	_region_init(&r, p, 2);
	return (_citrus_db_factory_add_by_string(df, key, &r, 1));
}

int
ref__citrus_db_factory_add32_by_string(struct _citrus_db_factory *df,
    const char *key, uint32_t val)
{
	struct _region r;
	uint32_t *p;

	p = malloc(sizeof(*p));
	if (p == NULL)
		return (errno);
	*p = htonl(val);
	_region_init(&r, p, 4);
	return (_citrus_db_factory_add_by_string(df, key, &r, 1));
}

int
ref__citrus_db_factory_add_string_by_string(struct _citrus_db_factory *df,
    const char *key, const char *data)
{
	char *p;
	struct _region r;

	p = strdup(data);
	if (p == NULL)
		return (errno);
	_region_init(&r, p, strlen(p) + 1);
	return (ref__citrus_db_factory_add_by_string(df, key, &r, 1));
}

size_t
ref__citrus_db_factory_calc_size(struct _citrus_db_factory *df)
{
	size_t sz;

	sz = ref_ceilto(_CITRUS_DB_HEADER_SIZE);
	sz += ref_ceilto(_CITRUS_DB_ENTRY_SIZE * df->df_num_entries);
	sz += ref_ceilto(df->df_total_key_size);
	sz += df->df_total_data_size;

	return (sz);
}

void
ref_put8(struct _region *r, size_t *rofs, uint8_t val)
{

	*(uint8_t *)_region_offset(r, *rofs) = val;
	*rofs += 1;
}

void
ref_put32(struct _region *r, size_t *rofs, uint32_t val)
{

	val = htonl(val);
	memcpy(_region_offset(r, *rofs), &val, 4);
	*rofs += 4;
}

void
ref_putpad(struct _region *r, size_t *rofs)
{
	size_t i;

	for (i = ref_ceilto(*rofs) - *rofs; i > 0; i--)
		ref_put8(r, rofs, 0);
}

void
ref_dump_header(struct _region *r, const char *magic, size_t *rofs,
    size_t num_entries)
{

	while (*rofs<_CITRUS_DB_MAGIC_SIZE)
		ref_put8(r, rofs, *magic++);
	ref_put32(r, rofs, num_entries);
	ref_put32(r, rofs, _CITRUS_DB_HEADER_SIZE);
}

int
ref__citrus_db_factory_serialize(struct _citrus_db_factory *df, const char *magic,
    struct _region *r)
{
	struct _citrus_db_factory_entry *de, **depp, *det;
	size_t dataofs, i, keyofs, nextofs, ofs;

	ofs = 0;
	/* check whether more than 0 entries exist */
	if (df->df_num_entries == 0) {
		ref_dump_header(r, magic, &ofs, 0);
		return (0);
	}
	/* allocate hash table */
	depp = calloc(df->df_num_entries, sizeof(*depp));
	if (depp == NULL)
		return (-1);

	/* step1: store the entries which are not conflicting */
	STAILQ_FOREACH(de, &df->df_entries, de_entry) {
		de->de_hashvalue %= df->df_num_entries;
		de->de_idx = -1;
		de->de_next = NULL;
		if (depp[de->de_hashvalue] == NULL) {
			depp[de->de_hashvalue] = de;
			de->de_idx = (int)de->de_hashvalue;
		}
	}

	/* step2: resolve conflicts */
	i = 0;
	STAILQ_FOREACH(de, &df->df_entries, de_entry) {
		if (de->de_idx == -1) {
			det = depp[de->de_hashvalue];
			while (det->de_next != NULL)
				det = det->de_next;
			det->de_next = de;
			while (depp[i] != NULL)
				i++;
			depp[i] = de;
			de->de_idx = (int)i;
		}
	}

	keyofs = _CITRUS_DB_HEADER_SIZE +
	    ref_ceilto(df->df_num_entries*_CITRUS_DB_ENTRY_SIZE);
	dataofs = keyofs + ref_ceilto(df->df_total_key_size);

	/* dump header */
	ref_dump_header(r, magic, &ofs, df->df_num_entries);

	/* dump entries */
	for (i = 0; i < df->df_num_entries; i++) {
		de = depp[i];
		nextofs = 0;
		if (de->de_next) {
			nextofs = _CITRUS_DB_HEADER_SIZE +
			    de->de_next->de_idx * _CITRUS_DB_ENTRY_SIZE;
		}
		ref_put32(r, &ofs, de->de_hashvalue);
		ref_put32(r, &ofs, nextofs);
		ref_put32(r, &ofs, keyofs);
		ref_put32(r, &ofs, _region_size(&de->de_key));
		ref_put32(r, &ofs, dataofs);
		ref_put32(r, &ofs, _region_size(&de->de_data));
		memcpy(_region_offset(r, keyofs),
		    _region_head(&de->de_key), _region_size(&de->de_key));
		keyofs += _region_size(&de->de_key);
		memcpy(_region_offset(r, dataofs),
		    _region_head(&de->de_data), _region_size(&de->de_data));
		dataofs += _region_size(&de->de_data);
		ref_putpad(r, &dataofs);
	}
	ref_putpad(r, &ofs);
	ref_putpad(r, &keyofs);
	free(depp);

	return (0);
}

/*
 * ========================================================================
 * lib/libc/iconv/citrus_lookup.c
 * ========================================================================
 */

/*	$NetBSD: citrus_lookup.c,v 1.7 2012/05/04 16:45:05 joerg Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c)2003 Citrus Project,
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

struct _citrus_lookup {
	union {
		struct {
			struct _citrus_db *db;
			struct _citrus_region file;
			int num, idx;
			struct _db_locator locator;
		} db;
		struct {
			struct _region r;
			struct _memstream ms;
		} plain;
	} u;
#define cl_db		u.db.db
#define cl_dbidx	u.db.idx
#define cl_dbfile	u.db.file
#define cl_dbnum	u.db.num
#define cl_dblocator	u.db.locator
#define cl_plainr	u.plain.r
#define cl_plainms	u.plain.ms
	int cl_ignore_case;
	int cl_rewind;
	char *cl_key;
	size_t cl_keylen;
	int (*cl_next)(struct _citrus_lookup *, struct _region *,
		       struct _region *);
	int (*cl_lookup)(struct _citrus_lookup *, const char *,
			 struct _region *);
	int (*cl_num_entries)(struct _citrus_lookup *);
	void (*cl_close)(struct _citrus_lookup *);
};

int
ref_seq_get_num_entries_db(struct _citrus_lookup *cl)
{

	return (cl->cl_dbnum);
}

int
ref_seq_next_db(struct _citrus_lookup *cl, struct _region *key,
    struct _region *data)
{

	if (cl->cl_key) {
		if (key)
			_region_init(key, cl->cl_key, cl->cl_keylen);
		return (ref__citrus_db_lookup_by_string(cl->cl_db, cl->cl_key, data,
		    &cl->cl_dblocator));
	}

	if (cl->cl_rewind) {
		cl->cl_dbidx = 0;
	}
	cl->cl_rewind = 0;
	if (cl->cl_dbidx >= cl->cl_dbnum)
		return (ENOENT);

	return (ref__citrus_db_get_entry(cl->cl_db, cl->cl_dbidx++, key, data));
}

int
ref_seq_lookup_db(struct _citrus_lookup *cl, const char *key, struct _region *data)
{

	cl->cl_rewind = 0;
	free(cl->cl_key);
	cl->cl_key = strdup(key);
	if (cl->cl_ignore_case)
		_bcs_convert_to_lower(cl->cl_key);
	cl->cl_keylen = strlen(cl->cl_key);
	_citrus_db_locator_init(&cl->cl_dblocator);
	return (ref__citrus_db_lookup_by_string(cl->cl_db, cl->cl_key, data,
	    &cl->cl_dblocator));
}

void
ref_seq_close_db(struct _citrus_lookup *cl)
{

	ref__citrus_db_close(cl->cl_db);
	_unmap_file(&cl->cl_dbfile);
}

int
ref_seq_open_db(struct _citrus_lookup *cl, const char *name)
{
	struct _region r;
	char path[PATH_MAX];
	int ret;

	snprintf(path, sizeof(path), "%s.db", name);
	ret = _map_file(&r, path);
	if (ret)
		return (ret);

	ret = _db_open(&cl->cl_db, &r, _CITRUS_LOOKUP_MAGIC,
	    _db_hash_std, NULL);
	if (ret) {
		_unmap_file(&r);
		return (ret);
	}

	cl->cl_dbfile = r;
	cl->cl_dbnum = _db_get_num_entries(cl->cl_db);
	cl->cl_dbidx = 0;
	cl->cl_rewind = 1;
	cl->cl_lookup = &seq_lookup_db;
	cl->cl_next = &seq_next_db;
	cl->cl_num_entries = &seq_get_num_entries_db;
	cl->cl_close = &seq_close_db;

	return (0);
}

#define T_COMM '#'
int
ref_seq_next_plain(struct _citrus_lookup *cl, struct _region *key,
	       struct _region *data)
{
	const char *p, *q;
	size_t len;

	if (cl->cl_rewind)
		_memstream_bind(&cl->cl_plainms, &cl->cl_plainr);
	cl->cl_rewind = 0;

retry:
	p = _memstream_getln(&cl->cl_plainms, &len);
	if (p == NULL)
		return (ENOENT);
	/* ignore comment */
	q = memchr(p, T_COMM, len);
	if (q) {
		len = q - p;
	}
	/* ignore trailing spaces */
	_bcs_trunc_rws_len(p, &len);
	p = _bcs_skip_ws_len(p, &len);
	q = _bcs_skip_nonws_len(p, &len);
	if (p == q)
		goto retry;
	if (cl->cl_key && ((size_t)(q - p) != cl->cl_keylen ||
	    memcmp(p, cl->cl_key, (size_t)(q - p)) != 0))
		goto retry;

	/* found a entry */
	if (key)
		_region_init(key, __DECONST(void *, p), (size_t)(q - p));
	p = _bcs_skip_ws_len(q, &len);
	if (data)
		_region_init(data, len ? __DECONST(void *, p) : NULL, len);

	return (0);
}

int
ref_seq_get_num_entries_plain(struct _citrus_lookup *cl)
{
	int num;

	num = 0;
	while (seq_next_plain(cl, NULL, NULL) == 0)
		num++;

	return (num);
}

int
ref_seq_lookup_plain(struct _citrus_lookup *cl, const char *key,
    struct _region *data)
{
	size_t len;
	const char *p;

	cl->cl_rewind = 0;
	free(cl->cl_key);
	cl->cl_key = strdup(key);
	if (cl->cl_ignore_case)
		_bcs_convert_to_lower(cl->cl_key);
	cl->cl_keylen = strlen(cl->cl_key);
	_memstream_bind(&cl->cl_plainms, &cl->cl_plainr);
	p = _memstream_matchline(&cl->cl_plainms, cl->cl_key, &len, 0);
	if (p == NULL)
		return (ENOENT);
	if (data)
		_region_init(data, __DECONST(void *, p), len);

	return (0);
}

void
ref_seq_close_plain(struct _citrus_lookup *cl)
{

	_unmap_file(&cl->cl_plainr);
}

int
ref_seq_open_plain(struct _citrus_lookup *cl, const char *name)
{
	int ret;

	/* open read stream */
	ret = _map_file(&cl->cl_plainr, name);
	if (ret)
		return (ret);

	cl->cl_rewind = 1;
	cl->cl_next = &seq_next_plain;
	cl->cl_lookup = &seq_lookup_plain;
	cl->cl_num_entries = &seq_get_num_entries_plain;
	cl->cl_close = &seq_close_plain;

	return (0);
}

int
ref__citrus_lookup_seq_open(struct _citrus_lookup **rcl, const char *name,
    int ignore_case)
{
	int ret;
	struct _citrus_lookup *cl;

	cl = malloc(sizeof(*cl));
	if (cl == NULL)
		return (errno);

	cl->cl_key = NULL;
	cl->cl_keylen = 0;
	cl->cl_ignore_case = ignore_case;
	ret = seq_open_db(cl, name);
	if (ret == ENOENT)
		ret = seq_open_plain(cl, name);
	if (!ret)
		*rcl = cl;
	else
		free(cl);

	return (ret);
}

void
ref__citrus_lookup_seq_rewind(struct _citrus_lookup *cl)
{

	cl->cl_rewind = 1;
	free(cl->cl_key);
	cl->cl_key = NULL;
	cl->cl_keylen = 0;
}

int
ref__citrus_lookup_seq_next(struct _citrus_lookup *cl,
    struct _region *key, struct _region *data)
{

	return ((*cl->cl_next)(cl, key, data));
}

int
ref__citrus_lookup_seq_lookup(struct _citrus_lookup *cl, const char *key,
    struct _region *data)
{

	return ((*cl->cl_lookup)(cl, key, data));
}

int
ref__citrus_lookup_get_number_of_entries(struct _citrus_lookup *cl)
{

	return ((*cl->cl_num_entries)(cl));
}

void
ref__citrus_lookup_seq_close(struct _citrus_lookup *cl)
{

	free(cl->cl_key);
	(*cl->cl_close)(cl);
	free(cl);
}

char *
ref__citrus_lookup_simple(const char *name, const char *key,
    char *linebuf, size_t linebufsize, int ignore_case)
{
	struct _citrus_lookup *cl;
	struct _region data;
	int ret;

	ret = _citrus_lookup_seq_open(&cl, name, ignore_case);
	if (ret)
		return (NULL);

	ret = _citrus_lookup_seq_lookup(cl, key, &data);
	if (ret) {
		_citrus_lookup_seq_close(cl);
		return (NULL);
	}

	snprintf(linebuf, linebufsize, "%.*s", (int)_region_size(&data),
	    (const char *)_region_head(&data));

	_citrus_lookup_seq_close(cl);

	return (linebuf);
}
