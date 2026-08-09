#!/usr/bin/env python3
"""Generate oracle.c and port.cppm for batch b0329. Temporary; delete after use."""

import re
from pathlib import Path

ROOT = Path("/home/odin/pbsd")
HBSD = ROOT / "hbsd/src/lib/libc/iconv"
OUT = ROOT / "pbsd/lib/libc/iconv/b0329"

SOURCES = [
    ("citrus_none.c", (HBSD / "citrus_none.c").read_text()),
    ("citrus_stdenc.c", (HBSD / "citrus_stdenc.c").read_text()),
    ("citrus_pivot_factory.c", (HBSD / "citrus_pivot_factory.c").read_text()),
    ("bsd_iconv.c", (HBSD / "bsd_iconv.c").read_text()),
]

BATCH_FUNCS = [
    "_citrus_NONE_stdenc_init", "_citrus_NONE_stdenc_uninit",
    "_citrus_NONE_stdenc_init_state", "_citrus_NONE_stdenc_mbtocs",
    "_citrus_NONE_stdenc_cstomb", "_citrus_NONE_stdenc_mbtowc",
    "_citrus_NONE_stdenc_wctomb", "_citrus_NONE_stdenc_put_state_reset",
    "_citrus_NONE_stdenc_get_state_desc",
    "_citrus_stdenc_open", "_citrus_stdenc_close",
    "find_src", "free_src", "convert_line", "dump_db",
    "_citrus_pivot_factory_convert",
    "__bsd___iconv_open", "__bsd_iconv_open", "__bsd_iconv_open_into",
    "__bsd_iconv_close", "__bsd_iconv", "__bsd___iconv",
    "__bsd___iconv_get_list", "__bsd___iconv_free_list",
    "qsort_helper", "__bsd_iconvlist", "__bsd_iconv_canonicalize",
    "__bsd_iconvctl", "__bsd_iconv_set_relocation_prefix",
]

INTERNAL_MACROS = "\n".join(f"#define\t{fn}\tref_{fn}" for fn in BATCH_FUNCS)

STRIP_INCLUDES = re.compile(
    r'#include\s+"(?:citrus_[^"]+|iconv-internal\.h)"\s*\n')

ORACLE_SUPPORT = r'''
/*
 * PBSD batch b0329 -- oracle (specification).
 *
 * Batch sources (in this order):
 *   citrus_none.c, citrus_stdenc.c, citrus_pivot_factory.c, bsd_iconv.c
 *
 * Every batch function is renamed with a ref_ prefix.  Bodies are unmodified.
 * static was removed from batch-local functions so the harness can call them.
 */

#ifndef _GNU_SOURCE
#define	_GNU_SOURCE
#endif

#include <sys/queue.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <endian.h>
#include <errno.h>
#include <iconv.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#ifndef EFTYPE
#define	EFTYPE		79
#endif
#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * 8)
#endif
#ifndef LINE_MAX
#define	LINE_MAX	2048
#endif
#ifndef __DECONST
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif
#ifndef __inline
#define	__inline	inline
#endif
#ifndef __unused
#define	__unused	__attribute__((__unused__))
#endif
#ifndef __BEGIN_DECLS
#define	__BEGIN_DECLS
#define	__END_DECLS
#endif

#define _CITRUS_HASH_ENTRY(type)	LIST_ENTRY(type)

typedef uint32_t _citrus_wc_t;
typedef uint32_t _citrus_index_t;
typedef uint32_t _citrus_csid_t;
#define _CITRUS_CSID_INVALID	((_citrus_csid_t)-1)
#define _csid_t			_citrus_csid_t
#define _index_t		_citrus_index_t
#define _wc_t			_citrus_wc_t
#define _stdenc			_citrus_stdenc

#define _bcs_isblank		_citrus_bcs_isblank
#define _bcs_iseol		_citrus_bcs_iseol
#define _bcs_isspace		_citrus_bcs_isspace
#define _bcs_islower		_citrus_bcs_islower
#define _bcs_isupper		_citrus_bcs_isupper
#define _bcs_skip_nonws_len	_citrus_bcs_skip_nonws_len
#define _bcs_skip_ws_len	_citrus_bcs_skip_ws_len
#define _bcs_strcasecmp		_citrus_bcs_strcasecmp
#define _bcs_strncasecmp	_citrus_bcs_strncasecmp
#define _bcs_tolower		_citrus_bcs_tolower
#define _bcs_toupper		_citrus_bcs_toupper
#define _bcs_trunc_rws_len	_citrus_bcs_trunc_rws_len

#define _region			_citrus_region
#define _region_init		_citrus_region_init
#define _region_head		_citrus_region_head
#define _region_size		_citrus_region_size
#define _region_offset		_citrus_region_offset

#define _db_factory_create	_citrus_db_factory_create
#define _db_factory_free	_citrus_db_factory_free
#define _db_factory_add_by_s	_citrus_db_factory_add_by_string
#define _db_factory_add32_by_s	_citrus_db_factory_add32_by_string
#define _db_factory_calc_size	_citrus_db_factory_calc_size
#define _db_factory_serialize	_citrus_db_factory_serialize
#define _db_hash_std		_citrus_db_hash_std

#define _CITRUS_DEFAULT_STDENC_NAME	"NONE"
#define _CITRUS_PIVOT_MAGIC		"CSPIVOT\0"
#define _CITRUS_PIVOT_SUB_MAGIC	"CSPIVSUB"

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
_citrus_bcs_strcasecmp(const char * __restrict str1,
    const char * __restrict str2)
{
	int c1, c2;

	c1 = c2 = 1;
	while (c1 && c2 && c1 == c2) {
		c1 = _bcs_toupper(*str1++);
		c2 = _bcs_toupper(*str2++);
	}
	return ((c1 == c2) ? 0 : ((c1 > c2) ? 1 : -1));
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

#define _CITRUS_DB_MAGIC_SIZE	8
#define _CITRUS_DB_HEADER_SIZE	16
struct _citrus_db_header_x {
	char		dhx_magic[_CITRUS_DB_MAGIC_SIZE];
	uint32_t	dhx_num_entries;
	uint32_t	dhx_entry_offset;
} __attribute__((__packed__));

struct _citrus_db_entry_x {
	uint32_t	dex_hash_value;
	uint32_t	dex_next_offset;
	uint32_t	dex_key_offset;
	uint32_t	dex_key_size;
	uint32_t	dex_data_offset;
	uint32_t	dex_data_size;
} __attribute__((__packed__));
#define _CITRUS_DB_ENTRY_SIZE	24

typedef uint32_t (*_citrus_db_hash_func_t)(struct _citrus_region *);

uint32_t
_citrus_db_hash_std(struct _citrus_region *r)
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

struct _citrus_db_factory_entry {
	STAILQ_ENTRY(_citrus_db_factory_entry)	 de_entry;
	struct _citrus_db_factory_entry		*de_next;
	uint32_t				 de_hashvalue;
	struct _region				 de_key;
	int					 de_key_free;
	struct _region				 de_data;
	int					 de_data_free;
	int					 de_idx;
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

static __inline size_t
ceilto(size_t sz)
{
	return ((sz + DB_ALIGN - 1) & ~(DB_ALIGN - 1));
}

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
	while (*rofs < _CITRUS_DB_MAGIC_SIZE)
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
	if (df->df_num_entries == 0) {
		dump_header(r, magic, &ofs, 0);
		return (0);
	}
	depp = calloc(df->df_num_entries, sizeof(*depp));
	if (depp == NULL)
		return (-1);
	STAILQ_FOREACH(de, &df->df_entries, de_entry) {
		de->de_hashvalue %= df->df_num_entries;
		de->de_idx = -1;
		de->de_next = NULL;
		if (depp[de->de_hashvalue] == NULL) {
			depp[de->de_hashvalue] = de;
			de->de_idx = (int)de->de_hashvalue;
		}
	}
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
	    ceilto(df->df_num_entries * _CITRUS_DB_ENTRY_SIZE);
	dataofs = keyofs + ceilto(df->df_total_key_size);
	dump_header(r, magic, &ofs, df->df_num_entries);
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

typedef struct _citrus_module_rec {
	char	mr_name[64];
} *_citrus_module_t;

#define _CITRUS_STDENC_SDID_GENERIC		0
struct _citrus_stdenc_state_desc {
	union {
		struct {
			int	state;
#define _STDENC_SDGEN_INITIAL		1
		} generic;
	} u;
};

struct _citrus_stdenc_ops {
	int (*eo_init)(struct _citrus_stdenc * __restrict,
	    const void * __restrict, size_t,
	    struct _citrus_stdenc_traits * __restrict);
	void (*eo_uninit)(struct _citrus_stdenc *);
	int (*eo_init_state)(struct _citrus_stdenc * __restrict, void * __restrict);
	int (*eo_mbtocs)(struct _citrus_stdenc * __restrict,
	    _csid_t * __restrict, _index_t * __restrict,
	    char ** __restrict, size_t, void * __restrict, size_t * __restrict,
	    struct iconv_hooks *);
	int (*eo_cstomb)(struct _citrus_stdenc * __restrict,
	    char * __restrict, size_t, _csid_t, _index_t, void * __restrict,
	    size_t * __restrict, struct iconv_hooks *);
	int (*eo_mbtowc)(struct _citrus_stdenc * __restrict,
	    _wc_t * __restrict, char ** __restrict, size_t,
	    void * __restrict, size_t * __restrict, struct iconv_hooks *);
	int (*eo_wctomb)(struct _citrus_stdenc * __restrict,
	    char * __restrict, size_t, _wc_t, void * __restrict,
	    size_t * __restrict, struct iconv_hooks *);
	int (*eo_put_state_reset)(struct _citrus_stdenc * __restrict,
	    char * __restrict, size_t, void * __restrict, size_t * __restrict);
	int (*eo_get_state_desc)(struct _citrus_stdenc * __restrict,
	    void * __restrict, int, struct _citrus_stdenc_state_desc * __restrict);
};

struct _citrus_stdenc_traits {
	size_t	et_state_size;
	size_t	et_mb_cur_max;
};

struct _citrus_stdenc {
	struct _citrus_stdenc_ops	*ce_ops;
	void				*ce_closure;
	_citrus_module_t		 ce_module;
	struct _citrus_stdenc_traits	*ce_traits;
};

#define _CITRUS_STDENC_GETOPS_FUNC_BASE(n) \
   int n(struct _citrus_stdenc_ops *, size_t)
typedef _CITRUS_STDENC_GETOPS_FUNC_BASE((*_citrus_stdenc_getops_t));

#define _CITRUS_STDENC_DEF_OPS(_e_) \
extern struct _citrus_stdenc_ops _citrus_##_e_##_stdenc_ops; \
struct _citrus_stdenc_ops _citrus_##_e_##_stdenc_ops = { \
	&_citrus_##_e_##_stdenc_init, \
	&_citrus_##_e_##_stdenc_uninit, \
	&_citrus_##_e_##_stdenc_init_state, \
	&_citrus_##_e_##_stdenc_mbtocs, \
	&_citrus_##_e_##_stdenc_cstomb, \
	&_citrus_##_e_##_stdenc_mbtowc, \
	&_citrus_##_e_##_stdenc_wctomb, \
	&_citrus_##_e_##_stdenc_put_state_reset, \
	&_citrus_##_e_##_stdenc_get_state_desc \
}

struct _citrus_iconv_ops {
	int (*io_init_shared)(struct _citrus_iconv_shared * __restrict,
	    const char * __restrict, const char * __restrict);
	void (*io_uninit_shared)(struct _citrus_iconv_shared *);
	int (*io_init_context)(struct _citrus_iconv *);
	void (*io_uninit_context)(struct _citrus_iconv *);
	int (*io_convert)(struct _citrus_iconv * __restrict,
	    char * __restrict * __restrict, size_t * __restrict,
	    char * __restrict * __restrict, size_t * __restrict, uint32_t,
	    size_t * __restrict);
};

struct _citrus_iconv_shared {
	struct _citrus_iconv_ops	*ci_ops;
	void				*ci_closure;
	char				*ci_convname;
	bool				 ci_discard_ilseq;
	struct iconv_hooks		*ci_hooks;
	bool				 ci_ilseq_invalid;
};

struct _citrus_iconv {
	struct _citrus_iconv_shared	*cv_shared;
	void				*cv_closure;
};

static __inline int
_citrus_iconv_convert(struct _citrus_iconv * __restrict cv,
    char * __restrict * __restrict in, size_t * __restrict inbytes,
    char * __restrict * __restrict out, size_t * __restrict outbytes,
    uint32_t flags, size_t * __restrict nresults)
{
	return ((*cv->cv_shared->ci_ops->io_convert)(cv, in, inbytes, out,
	    outbytes, flags, nresults));
}

typedef struct {
	char			mock_modname[64];
	_citrus_stdenc_getops_t	mock_stdenc_getops;
	int			iconv_open_ret;
	struct _citrus_iconv	*iconv_open_handle;
	int			iconv_convert_ret;
	size_t			iconv_convert_nresults;
	char			**esdb_list;
	size_t			esdb_list_sz;
	bool			esdb_sorted;
	int			esdb_get_list_ret;
	const char		*canonicalize_ret;
	char			**fgetln_lines;
	size_t			*fgetln_lens;
	size_t			fgetln_count;
	size_t			fgetln_idx;
} B0329MockState;

static B0329MockState b0329_mock_state;

void
b0329_mock_reset(void)
{
	memset(&b0329_mock_state, 0, sizeof(b0329_mock_state));
}

void
b0329_mock_snap(B0329MockState *dst)
{
	*dst = b0329_mock_state;
}

void
b0329_mock_set_stdenc_module(const char *name,
    int (*getops)(struct _citrus_stdenc_ops *, size_t))
{
	strncpy(b0329_mock_state.mock_modname, name,
	    sizeof(b0329_mock_state.mock_modname) - 1);
	b0329_mock_state.mock_stdenc_getops = getops;
}

static int
mock_iconv_convert(struct _citrus_iconv * __restrict cv,
    char * __restrict * __restrict in, size_t * __restrict inbytes,
    char * __restrict * __restrict out, size_t * __restrict outbytes,
    uint32_t flags, size_t * __restrict nresults)
{
	(void)cv;
	(void)in;
	(void)inbytes;
	(void)out;
	(void)outbytes;
	(void)flags;
	if (nresults)
		*nresults = b0329_mock_state.iconv_convert_nresults;
	return (b0329_mock_state.iconv_convert_ret);
}

int
_citrus_load_module(_citrus_module_t *mod, const char *name)
{
	if (strcmp(name, b0329_mock_state.mock_modname) != 0)
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
	if (strcmp(name, b0329_mock_state.mock_modname) != 0)
		return (NULL);
	if (strcmp(kind, "stdenc") != 0)
		return (NULL);
	return ((void *)b0329_mock_state.mock_stdenc_getops);
}

int
_citrus_iconv_open(struct _citrus_iconv **handle, const char *in,
    const char *out)
{
	struct _citrus_iconv_shared *sh;
	struct _citrus_iconv *cv;
	char *cn;
	size_t len;

	(void)in;
	if (b0329_mock_state.iconv_open_ret)
		return (b0329_mock_state.iconv_open_ret);
	if (b0329_mock_state.iconv_open_handle) {
		*handle = b0329_mock_state.iconv_open_handle;
		return (0);
	}
	sh = calloc(1, sizeof(*sh));
	if (sh == NULL)
		return (errno);
	cv = calloc(1, sizeof(*cv));
	if (cv == NULL) {
		free(sh);
		return (errno);
	}
	len = strlen(in) + 1 + strlen(out) + 1;
	cn = malloc(len);
	if (cn == NULL) {
		free(cv);
		free(sh);
		return (errno);
	}
	snprintf(cn, len, "%s/%s", in, out);
	sh->ci_ops = calloc(1, sizeof(*sh->ci_ops));
	if (sh->ci_ops == NULL) {
		free(cn);
		free(cv);
		free(sh);
		return (errno);
	}
	sh->ci_ops->io_convert = mock_iconv_convert;
	sh->ci_convname = cn;
	cv->cv_shared = sh;
	*handle = cv;
	return (0);
}

void
_citrus_iconv_close(struct _citrus_iconv *cv)
{
	if (cv == NULL || cv == b0329_mock_state.iconv_open_handle)
		return;
	if (cv->cv_shared) {
		free(cv->cv_shared->ci_convname);
		free(cv->cv_shared->ci_ops);
		free(cv->cv_shared);
	}
	free(cv);
}

const char *
_citrus_iconv_canonicalize(const char *name)
{
	if (b0329_mock_state.canonicalize_ret)
		return (b0329_mock_state.canonicalize_ret);
	return (name);
}

int
_citrus_esdb_get_list(char ***rlist, size_t *rsz, bool sorted)
{
	(void)sorted;
	if (b0329_mock_state.esdb_get_list_ret)
		return (b0329_mock_state.esdb_get_list_ret);
	*rlist = b0329_mock_state.esdb_list;
	*rsz = b0329_mock_state.esdb_list_sz;
	return (0);
}

void
_citrus_esdb_free_list(char **list, size_t sz)
{
	(void)list;
	(void)sz;
}

char *
fgetln(FILE *fp, size_t *len)
{
	(void)fp;
	if (b0329_mock_state.fgetln_idx >= b0329_mock_state.fgetln_count)
		return (NULL);
	*len = b0329_mock_state.fgetln_lens[b0329_mock_state.fgetln_idx];
	return (b0329_mock_state.fgetln_lines[b0329_mock_state.fgetln_idx++]);
}

/*
 * ------------------------------------------------------------------------
 * Macro redirects for internal calls between batch functions.
 * ------------------------------------------------------------------------
 */
''' + INTERNAL_MACROS + "\n"


def prepare_source(text: str) -> str:
    text = text.replace("_CITRUS_STDENC_DECLS(NONE);\n", "")
    return STRIP_INCLUDES.sub("", text)


def strip_static_defs(text: str) -> str:
    for fn in sorted(BATCH_FUNCS, key=len, reverse=True):
        text = re.sub(
            rf'\bstatic\s+(?=(?:__inline\s+)?)'
            rf'(?:int|void|size_t|iconv_t|const\s+char\s*\*)\s+{re.escape(fn)}\s*\(',
            '',
            text,
            flags=re.MULTILINE,
        )
    return text


def process_oracle_source(text: str) -> str:
    text = strip_static_defs(prepare_source(text))
    for fn in sorted(BATCH_FUNCS, key=len, reverse=True):
        text = re.sub(
            rf'(?<!ref_)\b{re.escape(fn)}\s*\(',
            f'ref_{fn}(',
            text,
        )
    return text


def process_port_source(text: str) -> str:
    text = strip_static_defs(prepare_source(text))
    if '_CITRUS_STDENC_DEF_OPS(NONE);' in text:
        text = text.replace('_CITRUS_STDENC_DEF_OPS(NONE);\n', '')
        text = text.rstrip() + '\n\n_CITRUS_STDENC_DEF_OPS(NONE);\n'
    return text


def cpp_fixup(text: str) -> str:
    text = text.replace('p = _region_head(r);', 'p = (const uint8_t *)_region_head(r);')
    subs = [
        (r'\bdf = malloc\b', 'df = (struct _citrus_db_factory *)malloc'),
        (r'\bde = malloc\b', 'de = (struct _citrus_db_factory_entry *)malloc'),
        (r'\bp = malloc\b', 'p = (uint32_t *)malloc'),
        (r'\bdepp = calloc\b', 'depp = (struct _citrus_db_factory_entry **)calloc'),
        (r'\btmp = strdup\b', 'tmp = (char *)strdup'),
        (r'\bptr = malloc\b', 'ptr = (void *)malloc'),
        (r'\bce = malloc\b', 'ce = (struct _citrus_stdenc *)malloc'),
        (r'\bse = malloc\b', 'se = (struct src_entry *)malloc'),
        (r'\bnames = malloc\b', 'names = (char **)malloc'),
        (r'\bcurkey = strndup\b', 'curkey = (char *)strndup'),
        (r'\bcuritem = strdup\b', 'curitem = (char *)strdup'),
        (r'\bce->ce_ops = \(struct _citrus_stdenc_ops \*\)malloc',
         'ce->ce_ops = (struct _citrus_stdenc_ops *)malloc'),
        (r'\bsh = calloc\b', 'sh = (struct _citrus_iconv_shared *)calloc'),
        (r'\bcv = calloc\b', 'cv = (struct _citrus_iconv *)calloc'),
        (r'\bcn = malloc\b', 'cn = (char *)malloc'),
        (r'\bsh->ci_ops = calloc\b', 'sh->ci_ops = (struct _citrus_iconv_ops *)calloc'),
        (r'\bgetops = \(_citrus_stdenc_getops_t\)_citrus_find_getops',
         'getops = (_citrus_stdenc_getops_t)_citrus_find_getops'),
        (r'__attribute__\(\(__packed__\)\)', '__attribute__((packed))'),
    ]
    for pat, rep in subs:
        text = re.sub(pat, rep, text)
    return text


PORT_PREFIX = r'''module;

#include <sys/queue.h>
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
#include <iconv.h>
#include <cwchar>

export module pbsd.lib.libc.iconv.b0329;

export namespace pbsd::lib_libc_iconv::b0329 {

#ifndef EFTYPE
#define EFTYPE 79
#endif
#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif
#ifndef LINE_MAX
#define LINE_MAX 2048
#endif
#ifndef __DECONST
#define __DECONST(type, var) ((type)(uintptr_t)(const void *)(var))
#endif
#ifndef __inline
#define __inline inline
#endif

'''


PORT_MOCK_EXTERN = r'''
typedef struct {
	char			mock_modname[64];
	_citrus_stdenc_getops_t	mock_stdenc_getops;
	int			iconv_open_ret;
	struct _citrus_iconv	*iconv_open_handle;
	int			iconv_convert_ret;
	size_t			iconv_convert_nresults;
	char			**esdb_list;
	size_t			esdb_list_sz;
	bool			esdb_sorted;
	int			esdb_get_list_ret;
	const char		*canonicalize_ret;
	char			**fgetln_lines;
	size_t			*fgetln_lens;
	size_t			fgetln_count;
	size_t			fgetln_idx;
} B0329MockState;

extern "C" {
void b0329_mock_reset(void);
void b0329_mock_snap(B0329MockState *);
void b0329_mock_set_stdenc_module(const char *, int (*)(struct _citrus_stdenc_ops *, size_t));
int _citrus_load_module(_citrus_module_t *, const char *);
void _citrus_unload_module(_citrus_module_t);
void *_citrus_find_getops(_citrus_module_t, const char *, const char *);
int _citrus_iconv_open(struct _citrus_iconv **, const char *, const char *);
void _citrus_iconv_close(struct _citrus_iconv *);
const char *_citrus_iconv_canonicalize(const char *);
int _citrus_esdb_get_list(char ***, size_t *, bool);
void _citrus_esdb_free_list(char **, size_t);
char *fgetln(FILE *, size_t *);
}
'''


def port_support_text() -> str:
    end = ORACLE_SUPPORT.index(
        "/*\n * ------------------------------------------------------------------------\n * Macro redirects")
    text = ORACLE_SUPPORT[:end]
    cut = text.index("typedef struct {")
    return text[:cut] + PORT_MOCK_EXTERN


def main():
    oracle_parts = []
    port_parts = []
    for name, text in SOURCES:
        oracle_parts.append(f"/* ===== {name} ===== */\n" + process_oracle_source(text))
        port_parts.append(f"/* ===== {name} ===== */\n" + cpp_fixup(process_port_source(text)))

    oracle = ORACLE_SUPPORT + "\n".join(oracle_parts) + "\n"
    (OUT / "oracle.c").write_text(oracle)

    port_body = cpp_fixup(
        port_support_text().replace("static __inline", "static inline")
        + "\n".join(port_parts)
    )
    port = PORT_PREFIX + port_body + "\n} // namespace\n"
    (OUT / "port.cppm").write_text(port)
    print("wrote oracle.c and port.cppm")


if __name__ == "__main__":
    main()
