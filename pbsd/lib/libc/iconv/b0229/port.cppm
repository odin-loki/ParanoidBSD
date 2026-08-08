module;

#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(__linux__)
#include <endian.h>
#elif defined(__APPLE__)
#include <machine/endian.h>
#else
#include <sys/endian.h>
#endif
#include <arpa/inet.h>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

export module pbsd.lib.libc.iconv.b0229;

export namespace pbsd::lib_libc_iconv::b0229 {


#define _DEFAULT_SOURCE

#if defined(__linux__)
#include <endian.h>
#elif defined(__APPLE__)
#include <machine/endian.h>
#else
#include <sys/endian.h>
#endif

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif
#ifndef __DECONST
#define __DECONST(type, var) ((type)(uintptr_t)(const void *)(var))
#endif
#ifndef __inline
#define __inline inline
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

static int __isthreaded = 0;
#define WLOCK(lock) if (__isthreaded) pthread_rwlock_wrlock(lock);
#define UNLOCK(lock) if (__isthreaded) pthread_rwlock_unlock(lock);

typedef uint32_t _citrus_index_t;

struct _citrus_region { void *r_head; size_t r_size; };
static __inline void _citrus_region_init(struct _citrus_region *r, void *h, size_t sz)
{ r->r_head = h; r->r_size = sz; }
static __inline void *_citrus_region_head(const struct _citrus_region *r) { return r->r_head; }
static __inline size_t _citrus_region_size(const struct _citrus_region *r) { return r->r_size; }
static __inline void *_citrus_region_offset(const _citrus_region *r, size_t pos)
{ return (void *)((uint8_t *)r->r_head + pos); }
static __inline uint8_t _citrus_region_peek8(const struct _citrus_region *r, size_t pos)
{ return *(uint8_t *)_citrus_region_offset(r, pos); }

#define _region _citrus_region
#define _region_init _citrus_region_init
#define _region_head _citrus_region_head
#define _region_size _citrus_region_size
#define _region_offset _citrus_region_offset
#define _region_peek8 _citrus_region_peek8

#define _CITRUS_BCS_PRED(_name_, _cond_) \
static __inline int _citrus_bcs_##_name_(uint8_t c) { return (_cond_); }
_CITRUS_BCS_PRED(isblank, c == ' ' || c == '\t')
_CITRUS_BCS_PRED(iseol, c == '\n' || c == '\r')
_CITRUS_BCS_PRED(isspace, _citrus_bcs_isblank(c) || _citrus_bcs_iseol(c) || c == '\f' || c == '\v')
_CITRUS_BCS_PRED(isdigit, c >= '0' && c <= '9')
_CITRUS_BCS_PRED(isupper, c >= 'A' && c <= 'Z')
_CITRUS_BCS_PRED(islower, c >= 'a' && c <= 'z')
_CITRUS_BCS_PRED(isalpha, _citrus_bcs_isupper(c) || _citrus_bcs_islower(c))
static __inline uint8_t _citrus_bcs_toupper(uint8_t c)
{ return (_citrus_bcs_islower(c) ? (c - 'a' + 'A') : c); }
static __inline uint8_t _citrus_bcs_tolower(uint8_t c)
{ return (_citrus_bcs_isupper(c) ? (c - 'A' + 'a') : c); }
#define _bcs_iseol _citrus_bcs_iseol
#define _bcs_isspace _citrus_bcs_isspace
#define _bcs_tolower _citrus_bcs_tolower
#define _bcs_toupper _citrus_bcs_toupper
#define _bcs_skip_ws_len _citrus_bcs_skip_ws_len
#define _bcs_skip_nonws_len _citrus_bcs_skip_nonws_len
#define _bcs_trunc_rws_len _citrus_bcs_trunc_rws_len
#define _bcs_convert_to_lower _citrus_bcs_convert_to_lower
#define _bcs_strncasecmp _citrus_bcs_strncasecmp

int _citrus_bcs_strncasecmp(const char * __restrict s1, const char * __restrict s2, size_t sz);
const char *_citrus_bcs_skip_ws_len(const char * __restrict p, size_t * __restrict len);
const char *_citrus_bcs_skip_nonws_len(const char * __restrict p, size_t * __restrict len);
void _citrus_bcs_trunc_rws_len(const char * __restrict p, size_t * __restrict len);
void _citrus_bcs_convert_to_lower(char *s);

int _citrus_bcs_strncasecmp(const char * __restrict s1, const char * __restrict s2, size_t sz)
{ int c1, c2; c1 = c2 = 1; while (c1 && c2 && c1 == c2 && sz) { c1 = _bcs_toupper(*s1++); c2 = _bcs_toupper(*s2++); sz--; }
  return ((c1 == c2) ? 0 : ((c1 > c2) ? 1 : -1)); }
const char *_citrus_bcs_skip_ws_len(const char * __restrict p, size_t * __restrict len)
{ while (*len && *p && _bcs_isspace(*p)) { p++; (*len)--; } return p; }
const char *_citrus_bcs_skip_nonws_len(const char * __restrict p, size_t * __restrict len)
{ while (*len && *p && !_bcs_isspace(*p)) { p++; (*len)--; } return p; }
void _citrus_bcs_trunc_rws_len(const char * __restrict p, size_t * __restrict len)
{ while (*len && _bcs_isspace(p[*len - 1])) (*len)--; }
void _citrus_bcs_convert_to_lower(char *s) { while (*s) { *s = _bcs_tolower(*s); s++; } }

struct _citrus_memory_stream { struct _citrus_region ms_region; size_t ms_pos; };
#define _memstream _citrus_memory_stream
static __inline void _citrus_memory_stream_bind(struct _citrus_memory_stream *ms, const struct _citrus_region *r)
{ ms->ms_region = *r; ms->ms_pos = 0; }
static __inline size_t _citrus_memory_stream_remainder(struct _citrus_memory_stream *ms)
{ size_t sz = _region_size(&ms->ms_region); return (ms->ms_pos > sz ? 0 : sz - ms->ms_pos); }
static __inline int _citrus_memory_stream_seek(struct _citrus_memory_stream *ms, size_t pos, int w)
{ size_t sz = _region_size(&ms->ms_region);
  switch (w) { case SEEK_SET: if (pos >= sz) return -1; ms->ms_pos = pos; break;
  case SEEK_CUR: pos += (ssize_t)ms->ms_pos; if (pos >= sz) return -1; ms->ms_pos = pos; break;
  case SEEK_END: if (sz < pos) return -1; ms->ms_pos = sz - pos; break; } return 0; }
#define _citrus_memory_stream_seek _citrus_memory_stream_seek
#define _memstream_seek _citrus_memory_stream_seek
#define _memstream_bind _citrus_memory_stream_bind
#define _memstream_remainder _citrus_memory_stream_remainder
static __inline void *_citrus_memory_stream_getregion(struct _citrus_memory_stream *ms, struct _citrus_region *r, size_t sz)
{ void *ret; if (ms->ms_pos + sz > _region_size(&ms->ms_region)) return NULL;
  ret = _region_offset(&ms->ms_region, ms->ms_pos); ms->ms_pos += sz;
  if (r) _region_init(r, ret, sz); return ret; }
#define _memstream_getregion _citrus_memory_stream_getregion
const char *_citrus_memory_stream_getln(struct _citrus_memory_stream * __restrict ms, size_t * __restrict rlen);
#define _memstream_getln _citrus_memory_stream_getln
const char *_citrus_memory_stream_matchline(struct _citrus_memory_stream * __restrict ms,
    const char * __restrict key, size_t * __restrict rlen, int ic);
#define _memstream_matchline _citrus_memory_stream_matchline

const char *_citrus_memory_stream_getln(struct _citrus_memory_stream * __restrict ms, size_t * __restrict rlen)
{ const uint8_t *h, *p; size_t i, ret;
  if (ms->ms_pos >= _region_size(&ms->ms_region)) return NULL;
  h = p = (uint8_t *)_region_offset(&ms->ms_region, ms->ms_pos); ret = 0;
  for (i = _region_size(&ms->ms_region) - ms->ms_pos; i; i--) { ret++; if (_bcs_iseol(*p)) break; p++; }
  ms->ms_pos += ret; *rlen = ret; return (const char *)h; }

const char *_citrus_memory_stream_matchline(struct _citrus_memory_stream * __restrict ms,
    const char * __restrict key, size_t * __restrict rlen, int ic)
{ const char *p, *q; size_t keylen, len; keylen = strlen(key);
  for (;;) { p = _memstream_getln(ms, &len); if (!p) return NULL;
    q = memchr(p, '#', len); if (q) len = (size_t)(q - p); _bcs_trunc_rws_len(p, &len);
    if (!len) continue; p = _bcs_skip_ws_len(p, &len); q = _bcs_skip_nonws_len(p, &len);
    if ((size_t)(q - p) == keylen) {
      if (ic) { if (!memcmp(key, p, keylen)) break; }
      else { if (!_bcs_strncasecmp(key, p, keylen)) break; }
    } }
  p = _bcs_skip_ws_len(q, &len); *rlen = len; return p; }

uint32_t _citrus_db_hash_std(struct _citrus_region *r);
#define _db_hash_std _citrus_db_hash_std
uint32_t _citrus_db_hash_std(struct _citrus_region *r)
{ const uint8_t *p; uint32_t hash = 0, tmp; size_t i;
  p = (const uint8_t *)_region_head(r);
  for (i = _region_size(r); i; i--) { hash = (hash << 4) + _bcs_tolower(*p++);
    tmp = hash & 0xF0000000; if (tmp) { hash ^= tmp; hash ^= tmp >> 24; } }
  return hash; }

#define _CITRUS_DB_MAGIC_SIZE 8
#define _CITRUS_DB_HEADER_SIZE 16
struct _citrus_db_header_x { char dhx_magic[8]; uint32_t dhx_num_entries; uint32_t dhx_entry_offset; } __attribute__((packed));
struct _citrus_db_entry_x { uint32_t dex_hash_value, dex_next_offset, dex_key_offset, dex_key_size, dex_data_offset, dex_data_size; } __attribute__((packed));
#define _CITRUS_DB_ENTRY_SIZE 24
typedef uint32_t (*_citrus_db_hash_func_t)(struct _citrus_region *);
struct _citrus_db_locator { uint32_t dl_hashval; size_t dl_offset; };
#define _db_locator _citrus_db_locator
static __inline void _citrus_db_locator_init(struct _citrus_db_locator *dl)
{ dl->dl_hashval = 0; dl->dl_offset = 0; }
#define _db_locator_init _citrus_db_locator_init
#define _CITRUS_LOOKUP_MAGIC "LOOKUP\0\0"
#define _db_open _citrus_db_open
#define _db_close _citrus_db_close
#define _db_lookup_by_s _citrus_db_lookup_by_string
#define _db_get_num_entries _citrus_db_get_number_of_entries
#define _db_get_entry _citrus_db_get_entry
#define _map_file _citrus_map_file
#define _unmap_file _citrus_unmap_file

int _citrus_map_file(struct _citrus_region * __restrict r, const char * __restrict path)
{ struct stat st; void *head; int fd, ret = 0;
  _region_init(r, NULL, 0);
  if ((fd = open(path, O_RDONLY | O_CLOEXEC)) == -1) return errno;
  if (fstat(fd, &st) == -1) { ret = errno; goto error; }
  if (!S_ISREG(st.st_mode)) { ret = EOPNOTSUPP; goto error; }
  head = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (head == MAP_FAILED) { ret = errno; goto error; }
  _region_init(r, head, (size_t)st.st_size);
error: (void)close(fd); return ret; }
void _citrus_unmap_file(struct _citrus_region *r)
{ if (_region_head(r)) { munmap(_region_head(r), _region_size(r)); _region_init(r, NULL, 0); } }

#define _CITRUS_HASH_ENTRY(type) LIST_ENTRY(type)
#define _CITRUS_HASH_HEAD(headname, type, hashsize) struct headname { LIST_HEAD(, type) chh_table[hashsize]; }
#define _CITRUS_HASH_INIT(head, hashsize) do { int _i; for (_i = 0; _i < hashsize; _i++) LIST_INIT(&(head)->chh_table[_i]); } while (0)
#define _CITRUS_HASH_REMOVE(elm, field) LIST_REMOVE(elm, field)
#define _CITRUS_HASH_INSERT(head, elm, field, hashval) LIST_INSERT_HEAD(&(head)->chh_table[hashval], elm, field)
#define _CITRUS_HASH_SEARCH(head, elm, field, matchfunc, key, hashval) \
  do { LIST_FOREACH((elm), &(head)->chh_table[hashval], field) if (matchfunc((elm), (key)) == 0) break; } while (0)

typedef struct _citrus_module_rec *_citrus_module_t;
struct _citrus_mapper_traits { size_t mt_state_size, mt_src_max, mt_dst_max; };
struct _citrus_mapper_ops {
  int (*mo_init)(struct _citrus_mapper_area *__restrict, struct _citrus_mapper *__restrict,
      const char *__restrict, const void *__restrict, size_t,
      struct _citrus_mapper_traits *__restrict, size_t);
  void (*mo_uninit)(struct _citrus_mapper *);
  int (*mo_convert)(struct _citrus_mapper *__restrict, _citrus_index_t *__restrict,
      _citrus_index_t, void *__restrict);
  void (*mo_init_state)(void);
};
typedef int (*_citrus_mapper_getops_t)(struct _citrus_mapper_ops *);
struct _citrus_mapper {
  struct _citrus_mapper_ops *cm_ops; void *cm_closure; _citrus_module_t cm_module;
  struct _citrus_mapper_traits *cm_traits; LIST_ENTRY(_citrus_mapper) cm_entry;
  int cm_refcount; char *cm_key;
};
#define _mapper_close _citrus_mapper_close

#define B0229_MAX_MOD 16
struct b0229_mod { char name[64]; _citrus_mapper_getops_t getops; };
static struct b0229_mod b0229_mods[B0229_MAX_MOD];
static int b0229_nmod;
static _citrus_module_t b0229_mod_id = (_citrus_module_t)(uintptr_t)0x1000;

void b0229_mock_reset(void) { b0229_nmod = 0; b0229_mod_id = (_citrus_module_t)(uintptr_t)0x1000; }
void b0229_mock_set_module(const char *name, _citrus_mapper_getops_t getops)
{ if (b0229_nmod < B0229_MAX_MOD) { strlcpy(b0229_mods[b0229_nmod].name, name, 64);
  b0229_mods[b0229_nmod].getops = getops; b0229_nmod++; } }

int _citrus_load_module(_citrus_module_t *mod, const char *name)
{ int i; for (i = 0; i < b0229_nmod; i++) if (!strcmp(b0229_mods[i].name, name)) {
  *mod = b0229_mod_id++; return 0; } return ENOENT; }
void _citrus_unload_module(_citrus_module_t mod) { (void)mod; }
void *_citrus_find_getops(_citrus_module_t mod, const char *name, const char *kind)
{ int i; (void)mod; (void)kind; for (i = 0; i < b0229_nmod; i++)
  if (!strcmp(b0229_mods[i].name, name)) return (void *)b0229_mods[i].getops; return NULL; }

int _citrus_string_hash_func(const char *key, int hashsize)
{ struct _citrus_region r; _region_init(&r, __DECONST(void *, key), strlen(key));
  return (int)(_db_hash_std(&r) % (uint32_t)hashsize); }
#define _string_hash_func _citrus_string_hash_func


/* ===== citrus_db_factory.c ===== */
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
_citrus_db_factory_create(struct _citrus_db_factory **rdf,
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
_citrus_db_factory_free(struct _citrus_db_factory *df)
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

static __inline size_t
ceilto(size_t sz)
{
	return ((sz + DB_ALIGN - 1) & ~(DB_ALIGN - 1));
}

int
_citrus_db_factory_add(struct _citrus_db_factory *df, struct _region *key,
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
_citrus_db_factory_add_by_string(struct _citrus_db_factory *df,
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
_citrus_db_factory_add8_by_string(struct _citrus_db_factory *df,
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
_citrus_db_factory_add16_by_string(struct _citrus_db_factory *df,
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
_citrus_db_factory_add32_by_string(struct _citrus_db_factory *df,
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
_citrus_db_factory_add_string_by_string(struct _citrus_db_factory *df,
    const char *key, const char *data)
{
	char *p;
	struct _region r;

	p = strdup(data);
	if (p == NULL)
		return (errno);
	_region_init(&r, p, strlen(p) + 1);
	return (_citrus_db_factory_add_by_string(df, key, &r, 1));
}

size_t
_citrus_db_factory_calc_size(struct _citrus_db_factory *df)
{
	size_t sz;

	sz = ceilto(_CITRUS_DB_HEADER_SIZE);
	sz += ceilto(_CITRUS_DB_ENTRY_SIZE * df->df_num_entries);
	sz += ceilto(df->df_total_key_size);
	sz += df->df_total_data_size;

	return (sz);
}

static __inline void
put8(struct _region *r, size_t *rofs, uint8_t val)
{

	*(uint8_t *)_region_offset(r, *rofs) = val;
	*rofs += 1;
}

static __inline void
put32(struct _region *r, size_t *rofs, uint32_t val)
{

	val = htonl(val);
	memcpy(_region_offset(r, *rofs), &val, 4);
	*rofs += 4;
}

static __inline void
putpad(struct _region *r, size_t *rofs)
{
	size_t i;

	for (i = ceilto(*rofs) - *rofs; i > 0; i--)
		put8(r, rofs, 0);
}

static __inline void
dump_header(struct _region *r, const char *magic, size_t *rofs,
    size_t num_entries)
{

	while (*rofs<_CITRUS_DB_MAGIC_SIZE)
		put8(r, rofs, *magic++);
	put32(r, rofs, num_entries);
	put32(r, rofs, _CITRUS_DB_HEADER_SIZE);
}

int
_citrus_db_factory_serialize(struct _citrus_db_factory *df, const char *magic,
    struct _region *r)
{
	struct _citrus_db_factory_entry *de, **depp, *det;
	size_t dataofs, i, keyofs, nextofs, ofs;

	ofs = 0;
	/* check whether more than 0 entries exist */
	if (df->df_num_entries == 0) {
		dump_header(r, magic, &ofs, 0);
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
	    ceilto(df->df_num_entries*_CITRUS_DB_ENTRY_SIZE);
	dataofs = keyofs + ceilto(df->df_total_key_size);

	/* dump header */
	dump_header(r, magic, &ofs, df->df_num_entries);

	/* dump entries */
	for (i = 0; i < df->df_num_entries; i++) {
		de = depp[i];
		nextofs = 0;
		if (de->de_next) {
			nextofs = _CITRUS_DB_HEADER_SIZE +
			    de->de_next->de_idx * _CITRUS_DB_ENTRY_SIZE;
		}
		put32(r, &ofs, de->de_hashvalue);
		put32(r, &ofs, nextofs);
		put32(r, &ofs, keyofs);
		put32(r, &ofs, _region_size(&de->de_key));
		put32(r, &ofs, dataofs);
		put32(r, &ofs, _region_size(&de->de_data));
		memcpy(_region_offset(r, keyofs),
		    _region_head(&de->de_key), _region_size(&de->de_key));
		keyofs += _region_size(&de->de_key);
		memcpy(_region_offset(r, dataofs),
		    _region_head(&de->de_data), _region_size(&de->de_data));
		dataofs += _region_size(&de->de_data);
		putpad(r, &dataofs);
	}
	putpad(r, &ofs);
	putpad(r, &keyofs);
	free(depp);

	return (0);
}

/* ===== citrus_db.c ===== */
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
_citrus_db_open(struct _citrus_db **rdb, struct _region *r, const char *magic,
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
_citrus_db_close(struct _citrus_db *db)
{

	free(db);
}

int
_citrus_db_lookup(struct _citrus_db *db, struct _citrus_region *key,
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
_citrus_db_lookup_by_string(struct _citrus_db *db, const char *key,
    struct _citrus_region *data, struct _citrus_db_locator *dl)
{
	struct _region r;

	_region_init(&r, __DECONST(void *, key), strlen(key));

	return (_citrus_db_lookup(db, &r, data, dl));
}

int
_citrus_db_lookup8_by_string(struct _citrus_db *db, const char *key,
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
_citrus_db_lookup16_by_string(struct _citrus_db *db, const char *key,
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
_citrus_db_lookup32_by_string(struct _citrus_db *db, const char *key,
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
_citrus_db_lookup_string_by_string(struct _citrus_db *db, const char *key,
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
_citrus_db_get_number_of_entries(struct _citrus_db *db)
{
	struct _citrus_db_header_x *dhx;
	struct _memstream ms;

	_memstream_bind(&ms, &db->db_region);

	dhx = _memstream_getregion(&ms, NULL, sizeof(*dhx));
	return ((int)be32toh(dhx->dhx_num_entries));
}

int
_citrus_db_get_entry(struct _citrus_db *db, int idx, struct _region *key,
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

/* ===== citrus_lookup.c ===== */
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

static int
seq_get_num_entries_db(struct _citrus_lookup *cl)
{

	return (cl->cl_dbnum);
}

static int
seq_next_db(struct _citrus_lookup *cl, struct _region *key,
    struct _region *data)
{

	if (cl->cl_key) {
		if (key)
			_region_init(key, cl->cl_key, cl->cl_keylen);
		return (_db_lookup_by_s(cl->cl_db, cl->cl_key, data,
		    &cl->cl_dblocator));
	}

	if (cl->cl_rewind) {
		cl->cl_dbidx = 0;
	}
	cl->cl_rewind = 0;
	if (cl->cl_dbidx >= cl->cl_dbnum)
		return (ENOENT);

	return (_db_get_entry(cl->cl_db, cl->cl_dbidx++, key, data));
}

static int
seq_lookup_db(struct _citrus_lookup *cl, const char *key, struct _region *data)
{

	cl->cl_rewind = 0;
	free(cl->cl_key);
	cl->cl_key = strdup(key);
	if (cl->cl_ignore_case)
		_bcs_convert_to_lower(cl->cl_key);
	cl->cl_keylen = strlen(cl->cl_key);
	_db_locator_init(&cl->cl_dblocator);
	return (_db_lookup_by_s(cl->cl_db, cl->cl_key, data,
	    &cl->cl_dblocator));
}

static void
seq_close_db(struct _citrus_lookup *cl)
{

	_db_close(cl->cl_db);
	_unmap_file(&cl->cl_dbfile);
}

static int
seq_open_db(struct _citrus_lookup *cl, const char *name)
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
static int
seq_next_plain(struct _citrus_lookup *cl, struct _region *key,
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

static int
seq_get_num_entries_plain(struct _citrus_lookup *cl)
{
	int num;

	num = 0;
	while (seq_next_plain(cl, NULL, NULL) == 0)
		num++;

	return (num);
}

static int
seq_lookup_plain(struct _citrus_lookup *cl, const char *key,
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

static void
seq_close_plain(struct _citrus_lookup *cl)
{

	_unmap_file(&cl->cl_plainr);
}

static int
seq_open_plain(struct _citrus_lookup *cl, const char *name)
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
_citrus_lookup_seq_open(struct _citrus_lookup **rcl, const char *name,
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
_citrus_lookup_seq_rewind(struct _citrus_lookup *cl)
{

	cl->cl_rewind = 1;
	free(cl->cl_key);
	cl->cl_key = NULL;
	cl->cl_keylen = 0;
}

int
_citrus_lookup_seq_next(struct _citrus_lookup *cl,
    struct _region *key, struct _region *data)
{

	return ((*cl->cl_next)(cl, key, data));
}

int
_citrus_lookup_seq_lookup(struct _citrus_lookup *cl, const char *key,
    struct _region *data)
{

	return ((*cl->cl_lookup)(cl, key, data));
}

int
_citrus_lookup_get_number_of_entries(struct _citrus_lookup *cl)
{

	return ((*cl->cl_num_entries)(cl));
}

void
_citrus_lookup_seq_close(struct _citrus_lookup *cl)
{

	free(cl->cl_key);
	(*cl->cl_close)(cl);
	free(cl);
}

char *
_citrus_lookup_simple(const char *name, const char *key,
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

/* ===== citrus_mapper.c ===== */
/*	$NetBSD: citrus_mapper.c,v 1.10 2012/06/08 07:49:42 martin Exp $	*/

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




#define _CITRUS_MAPPER_DIR	"mapper.dir"

#define CM_HASH_SIZE 101
#define REFCOUNT_PERSISTENT	-1

static pthread_rwlock_t		cm_lock = PTHREAD_RWLOCK_INITIALIZER;

struct _citrus_mapper_area {
	_CITRUS_HASH_HEAD(, _citrus_mapper, CM_HASH_SIZE)	 ma_cache;
	char							*ma_dir;
};

/*
 * _citrus_mapper_create_area:
 *	create mapper area
 */

int
_citrus_mapper_create_area(
    struct _citrus_mapper_area *__restrict *__restrict rma,
    const char *__restrict area)
{
	struct _citrus_mapper_area *ma;
	struct stat st;
	char path[PATH_MAX];
	int ret;

	WLOCK(&cm_lock);

	if (*rma != NULL) {
		ret = 0;
		goto quit;
	}

	snprintf(path, (size_t)PATH_MAX, "%s/%s", area, _CITRUS_MAPPER_DIR);

	ret = stat(path, &st);
	if (ret)
		goto quit;

	ma = malloc(sizeof(*ma));
	if (ma == NULL) {
		ret = errno;
		goto quit;
	}
	ma->ma_dir = strdup(area);
	if (ma->ma_dir == NULL) {
		ret = errno;
		free(ma);
		goto quit;
	}
	_CITRUS_HASH_INIT(&ma->ma_cache, CM_HASH_SIZE);

	*rma = ma;
	ret = 0;
quit:
	UNLOCK(&cm_lock);

	return (ret);
}


/*
 * lookup_mapper_entry:
 *	lookup mapper.dir entry in the specified directory.
 *
 * line format of iconv.dir file:
 *	mapper	module	arg
 * mapper : mapper name.
 * module : mapper module name.
 * arg    : argument for the module (generally, description file name)
 */

static int
lookup_mapper_entry(const char *dir, const char *mapname, void *linebuf,
    size_t linebufsize, const char **module, const char **variable)
{
	struct _region r;
	struct _memstream ms;
	const char *cp, *cq;
	char *p;
	char path[PATH_MAX];
	size_t len;
	int ret;

	/* create mapper.dir path */
	snprintf(path, (size_t)PATH_MAX, "%s/%s", dir, _CITRUS_MAPPER_DIR);

	/* open read stream */
	ret = _map_file(&r, path);
	if (ret)
		return (ret);

	_memstream_bind(&ms, &r);

	/* search the line matching to the map name */
	cp = _memstream_matchline(&ms, mapname, &len, 0);
	if (!cp) {
		ret = ENOENT;
		goto quit;
	}
	if (!len || len > linebufsize - 1) {
		ret = EINVAL;
		goto quit;
	}

	p = linebuf;
	/* get module name */
	*module = p;
	cq = _bcs_skip_nonws_len(cp, &len);
	strlcpy(p, cp, (size_t)(cq - cp + 1));
	p += cq - cp + 1;

	/* get variable */
	*variable = p;
	cp = _bcs_skip_ws_len(cq, &len);
	strlcpy(p, cp, len + 1);

	ret = 0;

quit:
	_unmap_file(&r);
	return (ret);
}

/*
 * mapper_close:
 *	simply close a mapper. (without handling hash)
 */
static void
mapper_close(struct _citrus_mapper *cm)
{
	if (cm->cm_module) {
		if (cm->cm_ops) {
			if (cm->cm_closure)
				(*cm->cm_ops->mo_uninit)(cm);
			free(cm->cm_ops);
		}
		_citrus_unload_module(cm->cm_module);
	}
	free(cm->cm_traits);
	free(cm);
}

/*
 * mapper_open:
 *	simply open a mapper. (without handling hash)
 */
static int
mapper_open(struct _citrus_mapper_area *__restrict ma,
    struct _citrus_mapper * __restrict * __restrict rcm,
    const char * __restrict module,
    const char * __restrict variable)
{
	struct _citrus_mapper *cm;
	_citrus_mapper_getops_t getops;
	int ret;

	/* initialize mapper handle */
	cm = malloc(sizeof(*cm));
	if (!cm)
		return (errno);

	cm->cm_module = NULL;
	cm->cm_ops = NULL;
	cm->cm_closure = NULL;
	cm->cm_traits = NULL;
	cm->cm_refcount = 0;
	cm->cm_key = NULL;

	/* load module */
	ret = _citrus_load_module(&cm->cm_module, module);
	if (ret)
		goto err;

	/* get operators */
	getops = (_citrus_mapper_getops_t)
	    _citrus_find_getops(cm->cm_module, module, "mapper");
	if (!getops) {
		ret = EOPNOTSUPP;
		goto err;
	}
	cm->cm_ops = malloc(sizeof(*cm->cm_ops));
	if (!cm->cm_ops) {
		ret = errno;
		goto err;
	}
	ret = (*getops)(cm->cm_ops);
	if (ret)
		goto err;

	if (!cm->cm_ops->mo_init ||
	    !cm->cm_ops->mo_uninit ||
	    !cm->cm_ops->mo_convert ||
	    !cm->cm_ops->mo_init_state) {
		ret = EINVAL;
		goto err;
	}

	/* allocate traits structure */
	cm->cm_traits = malloc(sizeof(*cm->cm_traits));
	if (cm->cm_traits == NULL) {
		ret = errno;
		goto err;
	}
	/* initialize the mapper */
	ret = (*cm->cm_ops->mo_init)(ma, cm, ma->ma_dir,
	    (const void *)variable, strlen(variable) + 1,
	    cm->cm_traits, sizeof(*cm->cm_traits));
	if (ret)
		goto err;

	*rcm = cm;

	return (0);

err:
	mapper_close(cm);
	return (ret);
}

/*
 * _citrus_mapper_open_direct:
 *	open a mapper.
 */
int
_citrus_mapper_open_direct(struct _citrus_mapper_area *__restrict ma,
    struct _citrus_mapper * __restrict * __restrict rcm,
    const char * __restrict module, const char * __restrict variable)
{

	return (mapper_open(ma, rcm, module, variable));
}

/*
 * hash_func
 */
static __inline int
hash_func(const char *key)
{

	return (_string_hash_func(key, CM_HASH_SIZE));
}

/*
 * match_func
 */
static __inline int
match_func(struct _citrus_mapper *cm, const char *key)
{

	return (strcmp(cm->cm_key, key));
}

/*
 * _citrus_mapper_open:
 *	open a mapper with looking up "mapper.dir".
 */
int
_citrus_mapper_open(struct _citrus_mapper_area *__restrict ma,
    struct _citrus_mapper * __restrict * __restrict rcm,
    const char * __restrict mapname)
{
	struct _citrus_mapper *cm;
	char linebuf[PATH_MAX];
	const char *module, *variable;
	int hashval, ret;

	variable = NULL;

	WLOCK(&cm_lock);

	/* search in the cache */
	hashval = hash_func(mapname);
	_CITRUS_HASH_SEARCH(&ma->ma_cache, cm, cm_entry, match_func, mapname,
	    hashval);
	if (cm) {
		/* found */
		cm->cm_refcount++;
		*rcm = cm;
		ret = 0;
		goto quit;
	}

	/* search mapper entry */
	ret = lookup_mapper_entry(ma->ma_dir, mapname, linebuf,
	    (size_t)PATH_MAX, &module, &variable);
	if (ret)
		goto quit;

	/* open mapper */
	UNLOCK(&cm_lock);
	ret = mapper_open(ma, &cm, module, variable);
	WLOCK(&cm_lock);
	if (ret)
		goto quit;
	cm->cm_key = strdup(mapname);
	if (cm->cm_key == NULL) {
		ret = errno;
		_mapper_close(cm);
		goto quit;	
	}

	/* insert to the cache */
	cm->cm_refcount = 1;
	_CITRUS_HASH_INSERT(&ma->ma_cache, cm, cm_entry, hashval);

	*rcm = cm;
	ret = 0;
quit:
	UNLOCK(&cm_lock);

	return (ret);
}

/*
 * _citrus_mapper_close:
 *	close the specified mapper.
 */
void
_citrus_mapper_close(struct _citrus_mapper *cm)
{

	if (cm) {
		WLOCK(&cm_lock);
		if (cm->cm_refcount == REFCOUNT_PERSISTENT)
			goto quit;
		if (cm->cm_refcount > 0) {
			if (--cm->cm_refcount > 0)
				goto quit;
			_CITRUS_HASH_REMOVE(cm, cm_entry);
			free(cm->cm_key);
		}
		UNLOCK(&cm_lock);
		mapper_close(cm);
		return;
quit:
		UNLOCK(&cm_lock);
	}
}

/*
 * _citrus_mapper_set_persistent:
 *	set persistent count.
 */
void
_citrus_mapper_set_persistent(struct _citrus_mapper * __restrict cm)
{

	WLOCK(&cm_lock);
	cm->cm_refcount = REFCOUNT_PERSISTENT;
	UNLOCK(&cm_lock);
}
} // namespace
