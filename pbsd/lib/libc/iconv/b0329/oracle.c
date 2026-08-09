
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

typedef struct {
	void	*spaceholder[64];
} iconv_allocation_t;

struct iconv_hooks {
	void (*uc_hook)(unsigned int, void *);
	void (*wc_hook)(wchar_t, void *);
	void *data;
};

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

#ifndef ICONV_TRIVIALP
#define ICONV_TRIVIALP		0
#define ICONV_GET_TRANSLITERATE	1
#define ICONV_SET_TRANSLITERATE	2
#define ICONV_GET_DISCARD_ILSEQ	3
#define ICONV_SET_DISCARD_ILSEQ	4
#define ICONV_SET_HOOKS		5
#define ICONV_SET_FALLBACKS	6
#define ICONV_GET_ILSEQ_INVALID	128
#define ICONV_SET_ILSEQ_INVALID	129
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
#define _stdenc_state_desc	_citrus_stdenc_state_desc
#define _STDENC_SDID_GENERIC	_CITRUS_STDENC_SDID_GENERIC
#define _STDENC_SDGEN_INITIAL	1

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

#define _db_factory		_citrus_db_factory
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

int _citrus_stdenc_open(struct _citrus_stdenc * __restrict * __restrict,
    char const * __restrict, const void * __restrict, size_t);
void _citrus_stdenc_close(struct _citrus_stdenc *);

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
#define	_citrus_NONE_stdenc_init	ref__citrus_NONE_stdenc_init
#define	_citrus_NONE_stdenc_uninit	ref__citrus_NONE_stdenc_uninit
#define	_citrus_NONE_stdenc_init_state	ref__citrus_NONE_stdenc_init_state
#define	_citrus_NONE_stdenc_mbtocs	ref__citrus_NONE_stdenc_mbtocs
#define	_citrus_NONE_stdenc_cstomb	ref__citrus_NONE_stdenc_cstomb
#define	_citrus_NONE_stdenc_mbtowc	ref__citrus_NONE_stdenc_mbtowc
#define	_citrus_NONE_stdenc_wctomb	ref__citrus_NONE_stdenc_wctomb
#define	_citrus_NONE_stdenc_put_state_reset	ref__citrus_NONE_stdenc_put_state_reset
#define	_citrus_NONE_stdenc_get_state_desc	ref__citrus_NONE_stdenc_get_state_desc
#define	_citrus_stdenc_open	ref__citrus_stdenc_open
#define	_citrus_stdenc_close	ref__citrus_stdenc_close
#define	find_src	ref_find_src
#define	free_src	ref_free_src
#define	convert_line	ref_convert_line
#define	dump_db	ref_dump_db
#define	_citrus_pivot_factory_convert	ref__citrus_pivot_factory_convert
#define	__bsd___iconv_open	ref___bsd___iconv_open
#define	__bsd_iconv_open	ref___bsd_iconv_open
#define	__bsd_iconv_open_into	ref___bsd_iconv_open_into
#define	__bsd_iconv_close	ref___bsd_iconv_close
#define	__bsd_iconv	ref___bsd_iconv
#define	__bsd___iconv	ref___bsd___iconv
#define	__bsd___iconv_get_list	ref___bsd___iconv_get_list
#define	__bsd___iconv_free_list	ref___bsd___iconv_free_list
#define	qsort_helper	ref_qsort_helper
#define	__bsd_iconvlist	ref___bsd_iconvlist
#define	__bsd_iconv_canonicalize	ref___bsd_iconv_canonicalize
#define	__bsd_iconvctl	ref___bsd_iconvctl
#define	__bsd_iconv_set_relocation_prefix	ref___bsd_iconv_set_relocation_prefix
/* ===== citrus_none.c ===== */
/* $NetBSD: citrus_none.c,v 1.18 2008/06/14 16:01:07 tnozaki Exp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Citrus Project,
 * Copyright (c) 2010 Gabor Kovesdan <gabor@FreeBSD.org>,
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

#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <iconv.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

struct _citrus_stdenc_traits _citrus_NONE_stdenc_traits = {
	0,	/* et_state_size */
	1,	/* mb_cur_max */
};

int
ref__citrus_NONE_stdenc_init(struct _citrus_stdenc * __restrict ce,
    const void *var __unused, size_t lenvar __unused,
    struct _citrus_stdenc_traits * __restrict et)
{

	et->et_state_size = 0;
	et->et_mb_cur_max = 1;

	ce->ce_closure = NULL;

	return (0);
}

void
ref__citrus_NONE_stdenc_uninit(struct _citrus_stdenc *ce __unused)
{

}

int
ref__citrus_NONE_stdenc_init_state(struct _citrus_stdenc * __restrict ce __unused,
    void * __restrict ps __unused)
{

	return (0);
}

int
ref__citrus_NONE_stdenc_mbtocs(struct _citrus_stdenc * __restrict ce __unused,
    _csid_t *csid, _index_t *idx, char **s, size_t n,
    void *ps __unused, size_t *nresult, struct iconv_hooks *hooks)
{

	if (n < 1) {
		*nresult = (size_t)-2;
		return (0);
	}

	*csid = 0;
	*idx = (_index_t)(unsigned char)*(*s)++;
	*nresult = *idx == 0 ? 0 : 1;

	if ((hooks != NULL) && (hooks->uc_hook != NULL))
		hooks->uc_hook((unsigned int)*idx, hooks->data);

	return (0);
}

int
ref__citrus_NONE_stdenc_cstomb(struct _citrus_stdenc * __restrict ce __unused,
    char *s, size_t n, _csid_t csid, _index_t idx, void *ps __unused,
    size_t *nresult, struct iconv_hooks *hooks __unused)
{

	if (csid == _CITRUS_CSID_INVALID) {
		*nresult = 0;
		return (0);
	}
	if (csid != 0)
		return (EILSEQ);

	if ((idx & 0x000000FF) == idx) {
		if (n < 1) {
			*nresult = (size_t)-1;
			return (E2BIG);
		}
		*s = (char)idx;
		*nresult = 1;
	} else if ((idx & 0x0000FFFF) == idx) {
		if (n < 2) {
			*nresult = (size_t)-1;
			return (E2BIG);
		}
		s[0] = (char)idx;
		/* XXX: might be endian dependent */
		s[1] = (char)(idx >> 8);
		*nresult = 2;
	} else if ((idx & 0x00FFFFFF) == idx) {
		if (n < 3) {
			*nresult = (size_t)-1;
			return (E2BIG);
		}
		s[0] = (char)idx;
		/* XXX: might be endian dependent */
		s[1] = (char)(idx >> 8);
		s[2] = (char)(idx >> 16);
		*nresult = 3;
	} else {
		if (n < 4) {
			*nresult = (size_t)-1;
			return (E2BIG);
		}
		s[0] = (char)idx;
		/* XXX: might be endian dependent */
		s[1] = (char)(idx >> 8);
		s[2] = (char)(idx >> 16);
		s[3] = (char)(idx >> 24);
		*nresult = 4;
	}
		
	return (0);
}

int
ref__citrus_NONE_stdenc_mbtowc(struct _citrus_stdenc * __restrict ce __unused,
    _wc_t * __restrict pwc, char ** __restrict s, size_t n,
    void * __restrict pspriv __unused, size_t * __restrict nresult,
    struct iconv_hooks *hooks)
{

	if (*s == NULL) {
		*nresult = 0;
		return (0);
	}
	if (n == 0) {
		*nresult = (size_t)-2;
		return (0);
	}

	if (pwc != NULL)
		*pwc = (_wc_t)(unsigned char) **s;

	*nresult = **s == '\0' ? 0 : 1;

	if ((hooks != NULL) && (hooks->wc_hook != NULL))
		hooks->wc_hook(*pwc, hooks->data);

	return (0);
}

int
ref__citrus_NONE_stdenc_wctomb(struct _citrus_stdenc * __restrict ce __unused,
    char * __restrict s, size_t n, _wc_t wc,
    void * __restrict pspriv __unused, size_t * __restrict nresult,
    struct iconv_hooks *hooks __unused)
{

	if ((wc & ~0xFFU) != 0) {
		*nresult = (size_t)-1;
		return (EILSEQ);
	}
	if (n == 0) {
		*nresult = (size_t)-1;
		return (E2BIG);
	}

	*nresult = 1;
	if (s != NULL && n > 0)
		*s = (char)wc;

	return (0);
}

int
ref__citrus_NONE_stdenc_put_state_reset(struct _citrus_stdenc * __restrict ce __unused,
    char * __restrict s __unused, size_t n __unused,
    void * __restrict pspriv __unused, size_t * __restrict nresult)
{

	*nresult = 0;

	return (0);
}

int
ref__citrus_NONE_stdenc_get_state_desc(struct _stdenc * __restrict ce __unused,
    void * __restrict ps __unused, int id,
    struct _stdenc_state_desc * __restrict d)
{
	int ret = 0;

	switch (id) {
	case _STDENC_SDID_GENERIC:
		d->u.generic.state = _STDENC_SDGEN_INITIAL;
		break;
	default:
		ret = EOPNOTSUPP;
	}

	return (ret);
}

_CITRUS_STDENC_DEF_OPS(NONE);

/* ===== citrus_stdenc.c ===== */
/*	$NetBSD: citrus_stdenc.c,v 1.4 2011/11/19 18:39:58 tnozaki Exp $	*/

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


#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct _citrus_stdenc _citrus_stdenc_default = {
	&_citrus_NONE_stdenc_ops,	/* ce_ops */
	NULL,				/* ce_closure */
	NULL,				/* ce_module */
	&_citrus_NONE_stdenc_traits,	/* ce_traits */
};

int
ref__citrus_stdenc_open(struct _citrus_stdenc * __restrict * __restrict rce,
    char const * __restrict encname, const void * __restrict variable,
    size_t lenvar)
{
	struct _citrus_stdenc *ce;
	_citrus_module_t handle;
	_citrus_stdenc_getops_t getops;
	int ret;

	if (!strcmp(encname, _CITRUS_DEFAULT_STDENC_NAME)) {
		*rce = &_citrus_stdenc_default;
		return (0);
	}
	ce = malloc(sizeof(*ce));
	if (ce == NULL) {
		ret = errno;
		goto bad;
	}
	ce->ce_ops = NULL;
	ce->ce_closure = NULL;
	ce->ce_module = NULL;
	ce->ce_traits = NULL;

	ret = _citrus_load_module(&handle, encname);
	if (ret)
		goto bad;

	ce->ce_module = handle;

	getops = (_citrus_stdenc_getops_t)_citrus_find_getops(ce->ce_module,
	    encname, "stdenc");
	if (getops == NULL) {
		ret = EINVAL;
		goto bad;
	}

	ce->ce_ops = (struct _citrus_stdenc_ops *)malloc(sizeof(*ce->ce_ops));
	if (ce->ce_ops == NULL) {
		ret = errno;
		goto bad;
	}

	ret = (*getops)(ce->ce_ops, sizeof(*ce->ce_ops));
	if (ret)
		goto bad;

	/* validation check */
	if (ce->ce_ops->eo_init == NULL ||
	    ce->ce_ops->eo_uninit == NULL ||
	    ce->ce_ops->eo_init_state == NULL ||
	    ce->ce_ops->eo_mbtocs == NULL ||
	    ce->ce_ops->eo_cstomb == NULL ||
	    ce->ce_ops->eo_mbtowc == NULL ||
	    ce->ce_ops->eo_wctomb == NULL ||
	    ce->ce_ops->eo_get_state_desc == NULL) {
		ret = EINVAL;
		goto bad;
	}

	/* allocate traits */
	ce->ce_traits = malloc(sizeof(*ce->ce_traits));
	if (ce->ce_traits == NULL) {
		ret = errno;
		goto bad;
	}
	/* init and get closure */
	ret = (*ce->ce_ops->eo_init)(ce, variable, lenvar, ce->ce_traits);
	if (ret)
		goto bad;

	*rce = ce;

	return (0);

bad:
	ref__citrus_stdenc_close(ce);
	return (ret);
}

void
ref__citrus_stdenc_close(struct _citrus_stdenc *ce)
{

	if (ce == &_citrus_stdenc_default)
		return;

	if (ce->ce_module) {
		if (ce->ce_ops) {
			if (ce->ce_closure && ce->ce_ops->eo_uninit)
				(*ce->ce_ops->eo_uninit)(ce);
			free(ce->ce_ops);
		}
		free(ce->ce_traits);
		_citrus_unload_module(ce->ce_module);
	}
	free(ce);
}

/* ===== citrus_pivot_factory.c ===== */
/* $NetBSD: citrus_pivot_factory.c,v 1.7 2009/04/12 14:20:19 lukem Exp $ */

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

#include <sys/queue.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct src_entry {
	char				*se_name;
	struct _citrus_db_factory	*se_df;
	STAILQ_ENTRY(src_entry)		 se_entry;
};
STAILQ_HEAD(src_head, src_entry);

int
ref_find_src(struct src_head *sh, struct src_entry **rse, const char *name)
{
	int ret;
	struct src_entry *se;

	STAILQ_FOREACH(se, sh, se_entry) {
		if (_bcs_strcasecmp(se->se_name, name) == 0) {
			*rse = se;
			return (0);
		}
	}
	se = malloc(sizeof(*se));
	if (se == NULL)
		return (errno);
	se->se_name = strdup(name);
	if (se->se_name == NULL) {
		ret = errno;
		free(se);
		return (ret);
	}
	ret = _db_factory_create(&se->se_df, &_db_hash_std, NULL);
	if (ret) {
		free(se->se_name);
		free(se);
		return (ret);
	}
	STAILQ_INSERT_TAIL(sh, se, se_entry);
	*rse = se;

	return (0);
}

void
ref_free_src(struct src_head *sh)
{
	struct src_entry *se;

	while ((se = STAILQ_FIRST(sh)) != NULL) {
		STAILQ_REMOVE_HEAD(sh, se_entry);
		_db_factory_free(se->se_df);
		free(se->se_name);
		free(se);
	}
}


#define T_COMM '#'
int
ref_convert_line(struct src_head *sh, const char *line, size_t len)
{
	struct src_entry *se;
	const char *p;
	char key1[LINE_MAX], key2[LINE_MAX], data[LINE_MAX];
	char *ep;
	uint32_t val;
	int ret;

	se = NULL;

	/* cut off trailing comment */
	p = memchr(line, T_COMM, len);
	if (p)
		len = p - line;

	/* key1 */
	line = _bcs_skip_ws_len(line, &len);
	if (len == 0)
		return (0);
	p = _bcs_skip_nonws_len(line, &len);
	if (p == line)
		return (0);
	snprintf(key1, sizeof(key1), "%.*s", (int)(p - line), line);

	/* key2 */
	line = _bcs_skip_ws_len(p, &len);
	if (len == 0)
		return (0);
	p = _bcs_skip_nonws_len(line, &len);
	if (p == line)
		return (0);
	snprintf(key2, sizeof(key2), "%.*s", (int)(p - line), line);

	/* data */
	line = _bcs_skip_ws_len(p, &len);
	_bcs_trunc_rws_len(line, &len);
	snprintf(data, sizeof(data), "%.*s", (int)len, line);
	val = strtoul(data, &ep, 0);
	if (*ep != '\0')
		return (EFTYPE);

	/* insert to DB */
	ret = ref_find_src(sh, &se, key1);
	if (ret)
		return (ret);

	return (_db_factory_add32_by_s(se->se_df, key2, val));
}

int
ref_dump_db(struct src_head *sh, struct _region *r)
{
	struct _db_factory *df;
	struct src_entry *se;
	struct _region subr;
	void *ptr;
	size_t size;
	int ret;

	ret = _db_factory_create(&df, &_db_hash_std, NULL);
	if (ret)
		return (ret);

	STAILQ_FOREACH(se, sh, se_entry) {
		size = _db_factory_calc_size(se->se_df);
		ptr = malloc(size);
		if (ptr == NULL)
			goto quit;
		_region_init(&subr, ptr, size);
		ret = _db_factory_serialize(se->se_df, _CITRUS_PIVOT_SUB_MAGIC,
		    &subr);
		if (ret)
			goto quit;
		ret = _db_factory_add_by_s(df, se->se_name, &subr, 1);
		if (ret)
			goto quit;
	}

	size = _db_factory_calc_size(df);
	ptr = malloc(size);
	if (ptr == NULL)
		goto quit;
	_region_init(r, ptr, size);

	ret = _db_factory_serialize(df, _CITRUS_PIVOT_MAGIC, r);
	ptr = NULL;

quit:
	free(ptr);
	_db_factory_free(df);
	return (ret);
}

int
ref__citrus_pivot_factory_convert(FILE *out, FILE *in)
{
	struct src_head sh;
	struct _region r;
	char *line;
	size_t size;
	int ret;

	STAILQ_INIT(&sh);

	while ((line = fgetln(in, &size)) != NULL)
		if ((ret = ref_convert_line(&sh, line, size))) {
			ref_free_src(&sh);
			return (ret);
		}

	ret = ref_dump_db(&sh, &r);
	ref_free_src(&sh);
	if (ret)
		return (ret);

	if (fwrite(_region_head(&r), _region_size(&r), 1, out) != 1)
		return (errno);

	return (0);
}

/* ===== bsd_iconv.c ===== */
/* $NetBSD: iconv.c,v 1.11 2009/03/03 16:22:33 explorer Exp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2003 Citrus Project,
 * Copyright (c) 2009, 2010 Gabor Kovesdan <gabor@FreeBSD.org>,
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

#include <sys/queue.h>
#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <iconv.h>
#include <limits.h>
#include <paths.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define ISBADF(_h_)	(!(_h_) || (_h_) == (iconv_t)-1)

iconv_t
ref___bsd___iconv_open(const char *out, const char *in, struct _citrus_iconv *handle)
{
	int ret;

	/*
	 * Remove anything following a //, as these are options (like
	 * //ignore, //translate, etc) and we just don't handle them.
	 * This is for compatibility with software that uses these
	 * blindly.
	 */
	ret = _citrus_iconv_open(&handle, in, out);
	if (ret) {
		errno = ret == ENOENT ? EINVAL : ret;
		return ((iconv_t)-1);
	}

	handle->cv_shared->ci_discard_ilseq = strcasestr(out, "//IGNORE");

	return ((iconv_t)(void *)handle);
}

iconv_t
ref___bsd_iconv_open(const char *out, const char *in)
{

	return (ref___bsd___iconv_open(out, in, NULL));
}

int
ref___bsd_iconv_open_into(const char *out, const char *in, iconv_allocation_t *ptr)
{
	struct _citrus_iconv *handle;

	handle = (struct _citrus_iconv *)ptr;
	return ((ref___bsd___iconv_open(out, in, handle) == (iconv_t)-1) ? -1 : 0);
}

int
ref___bsd_iconv_close(iconv_t handle)
{

	if (ISBADF(handle)) {
		errno = EBADF;
		return (-1);
	}

	_citrus_iconv_close((struct _citrus_iconv *)(void *)handle);

	return (0);
}

size_t
ref___bsd_iconv(iconv_t handle, char **in, size_t *szin, char **out, size_t *szout)
{
	size_t ret;
	int err;

	if (ISBADF(handle)) {
		errno = EBADF;
		return ((size_t)-1);
	}

	err = _citrus_iconv_convert((struct _citrus_iconv *)(void *)handle,
	    in, szin, out, szout, 0, &ret);
	if (err) {
		errno = err;
		ret = (size_t)-1;
	}

	return (ret);
}

size_t
ref___bsd___iconv(iconv_t handle, char **in, size_t *szin, char **out,
    size_t *szout, uint32_t flags, size_t *invalids)
{
	size_t ret;
	int err;

	if (ISBADF(handle)) {
		errno = EBADF;
		return ((size_t)-1);
	}

	err = _citrus_iconv_convert((struct _citrus_iconv *)(void *)handle,
	    in, szin, out, szout, flags, &ret);
	if (invalids)
		*invalids = ret;
	if (err) {
		errno = err;
		ret = (size_t)-1;
	}

	return (ret);
}

int
ref___bsd___iconv_get_list(char ***rlist, size_t *rsz, bool sorted)
{
	int ret;

	ret = _citrus_esdb_get_list(rlist, rsz, sorted);
	if (ret) {
		errno = ret;
		return (-1);
	}

	return (0);
}

void
ref___bsd___iconv_free_list(char **list, size_t sz)
{

	_citrus_esdb_free_list(list, sz);
}

/*
 * GNU-compatibile non-standard interfaces.
 */
int
ref_qsort_helper(const void *first, const void *second)
{
	const char * const *s1;
	const char * const *s2;

	s1 = first;
	s2 = second;
	return (strcmp(*s1, *s2));
}

void
ref___bsd_iconvlist(int (*do_one) (unsigned int, const char * const *,
    void *), void *data)
{
	char **list, **names;
	const char * const *np;
	char *curitem, *curkey, *slashpos;
	size_t sz;
	unsigned int i, j, n;

	i = 0;
	names = NULL;

	if (ref___bsd___iconv_get_list(&list, &sz, true)) {
		list = NULL;
		goto out;
	}
	qsort((void *)list, sz, sizeof(char *), qsort_helper);
	while (i < sz) {
		j = 0;
		slashpos = strchr(list[i], '/');
		names = malloc(sz * sizeof(char *));
		if (names == NULL)
			goto out;
		curkey = strndup(list[i], slashpos - list[i]);
		if (curkey == NULL)
			goto out;
		names[j++] = curkey;
		for (; (i < sz) && (memcmp(curkey, list[i], strlen(curkey)) == 0); i++) {
			slashpos = strchr(list[i], '/');
			if (strcmp(curkey, &slashpos[1]) == 0)
				continue;
			curitem = strdup(&slashpos[1]);
			if (curitem == NULL)
				goto out;
			names[j++] = curitem;
		}
		np = (const char * const *)names;
		do_one(j, np, data);
		for (n = 0; n < j; n++)
			free(names[n]);
		free(names);
		names = NULL;
	}

out:
	if (names != NULL) {
		for (n = 0; n < j; n++)
			free(names[n]);
		free(names);
	}
	if (list != NULL)
		ref___bsd___iconv_free_list(list, sz);
}

__inline const char *
ref___bsd_iconv_canonicalize(const char *name)
{

	return (_citrus_iconv_canonicalize(name));
}

int
ref___bsd_iconvctl(iconv_t cd, int request, void *argument)
{
	struct _citrus_iconv *cv;
	struct iconv_hooks *hooks;
	const char *convname;
	char *dst;
	int *i;
	size_t srclen;

	cv = (struct _citrus_iconv *)(void *)cd;
	hooks = (struct iconv_hooks *)argument;
	i = (int *)argument;

	if (ISBADF(cd)) {
		errno = EBADF;
		return (-1);
	}

	switch (request) {
	case ICONV_TRIVIALP:
		convname = cv->cv_shared->ci_convname;
		dst = strchr(convname, '/');
		srclen = dst - convname;
		dst++;
		*i = (srclen == strlen(dst)) && !memcmp(convname, dst, srclen);
		return (0);
	case ICONV_GET_TRANSLITERATE:
		*i = 1;
		return (0);
	case ICONV_SET_TRANSLITERATE:
		return  ((*i == 1) ? 0 : -1);
	case ICONV_GET_DISCARD_ILSEQ:
		*i = cv->cv_shared->ci_discard_ilseq ? 1 : 0;
		return (0);
	case ICONV_SET_DISCARD_ILSEQ:
		cv->cv_shared->ci_discard_ilseq = *i;
		return (0);
	case ICONV_SET_HOOKS:
		cv->cv_shared->ci_hooks = hooks;
		return (0);
	case ICONV_SET_FALLBACKS:
		errno = EOPNOTSUPP;
		return (-1);
	case ICONV_GET_ILSEQ_INVALID:
		*i = cv->cv_shared->ci_ilseq_invalid ? 1 : 0;
		return (0);
	case ICONV_SET_ILSEQ_INVALID:
		cv->cv_shared->ci_ilseq_invalid = *i;
		return (0);
	default:
		errno = EINVAL;
		return (-1);
	}
}

void
ref___bsd_iconv_set_relocation_prefix(const char *orig_prefix __unused,
    const char *curr_prefix __unused)
{

}

