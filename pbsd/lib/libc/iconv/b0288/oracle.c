/*
 * PBSD batch b0288 -- oracle (specification).
 *
 * citrus_lookup_factory.c and iconv_compat.c are not part of this file;
 * see skipped.txt.
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif
#ifndef __inline
#define __inline inline
#endif

#define _bcs_iseol		_citrus_bcs_iseol
#define _bcs_isspace		_citrus_bcs_isspace
#define _bcs_tolower		_citrus_bcs_tolower
#define _bcs_toupper		_citrus_bcs_toupper
#define _bcs_skip_ws_len	_citrus_bcs_skip_ws_len
#define _bcs_skip_nonws_len	_citrus_bcs_skip_nonws_len
#define _bcs_strncasecmp	_citrus_bcs_strncasecmp
#define _bcs_trunc_rws_len	_citrus_bcs_trunc_rws_len
#define _region			_citrus_region
#define _region_init		_citrus_region_init
#define _region_size		_citrus_region_size
#define _region_offset		_citrus_region_offset
#define _region_peek8		_citrus_region_peek8
#define _memstream_remainder	_citrus_memory_stream_remainder

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

static __inline uint8_t
_citrus_region_peek8(const struct _citrus_region *r, size_t pos)
{
	return (*(uint8_t *)_citrus_region_offset(r, pos));
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

struct _citrus_memory_stream {
	struct _citrus_region	ms_region;
	size_t			ms_pos;
};

static __inline int
_citrus_memory_stream_iseof(struct _citrus_memory_stream *ms)
{
	return (ms->ms_pos >= _citrus_region_size(&ms->ms_region));
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
_citrus_memory_stream_getc(struct _citrus_memory_stream *ms)
{
	if (_citrus_memory_stream_iseof(ms))
		return (EOF);
	return (_citrus_region_peek8(&ms->ms_region, ms->ms_pos++));
}

static __inline int
_citrus_memory_stream_peek(struct _citrus_memory_stream *ms)
{
	if (_citrus_memory_stream_iseof(ms))
		return (EOF);
	return (_citrus_region_peek8(&ms->ms_region, ms->ms_pos));
}

/* citrus_bcs.c */

int
ref__citrus_bcs_strcasecmp(const char * __restrict str1,
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
ref__citrus_bcs_strncasecmp(const char * __restrict str1,
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
ref__citrus_bcs_skip_ws(const char *p)
{
	while (*p && _bcs_isspace(*p))
		p++;
	return (p);
}

const char *
ref__citrus_bcs_skip_nonws(const char *p)
{
	while (*p && !_bcs_isspace(*p))
		p++;
	return (p);
}

const char *
ref__citrus_bcs_skip_ws_len(const char * __restrict p, size_t * __restrict len)
{
	while (*len > 0 && *p && _bcs_isspace(*p)) {
		p++;
		(*len)--;
	}
	return (p);
}

const char *
ref__citrus_bcs_skip_nonws_len(const char * __restrict p, size_t * __restrict len)
{
	while (*len > 0 && *p && !_bcs_isspace(*p)) {
		p++;
		(*len)--;
	}
	return (p);
}

void
ref__citrus_bcs_trunc_rws_len(const char * __restrict p, size_t * __restrict len)
{
	while (*len > 0 && _bcs_isspace(p[*len - 1]))
		(*len)--;
}

void
ref__citrus_bcs_convert_to_lower(char *s)
{
	while (*s) {
		*s = _bcs_tolower(*s);
		s++;
	}
}

void
ref__citrus_bcs_convert_to_upper(char *s)
{
	while (*s) {
		*s = _bcs_toupper(*s);
		s++;
	}
}

/* citrus_memstream.c */

const char *
ref__citrus_memory_stream_getln(struct _citrus_memory_stream * __restrict ms,
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
ref__citrus_memory_stream_matchline(struct _citrus_memory_stream * __restrict ms,
    const char * __restrict key, size_t * __restrict rlen, int iscasesensitive)
{
	const char *p, *q;
	size_t keylen, len;

	keylen = strlen(key);
	for(;;) {
		p = ref__citrus_memory_stream_getln(ms, &len);
		if (p == NULL)
			return (NULL);

		q = memchr(p, T_COMM, len);
		if (q)
			len = q - p;
		_bcs_trunc_rws_len(p, &len);
		if (len == 0)
			continue;

		p = _bcs_skip_ws_len(p, &len);
		q = _bcs_skip_nonws_len(p, &len);

		if ((size_t)(q - p) == keylen) {
			if (iscasesensitive) {
				if (memcmp(key, p, keylen) == 0)
					break;
			} else {
				if (_bcs_strncasecmp(key, p, keylen) == 0)
					break;
			}
		}
	}

	p = _bcs_skip_ws_len(q, &len);
	*rlen = len;
	return (p);
}

void *
ref__citrus_memory_stream_chr(struct _citrus_memory_stream *ms,
    struct _citrus_region *r, char ch)
{
	void *chr, *head;
	size_t sz;

	if (ms->ms_pos >= _region_size(&ms->ms_region))
		return (NULL);

	head = _region_offset(&ms->ms_region, ms->ms_pos);
	chr = memchr(head, ch, _memstream_remainder(ms));
	if (chr == NULL) {
		_region_init(r, head, _memstream_remainder(ms));
		ms->ms_pos = _region_size(&ms->ms_region);
		return (NULL);
	}
	sz = (char *)chr - (char *)head;

	_region_init(r, head, sz);
	ms->ms_pos += sz + 1;

	return (chr);
}

void
ref__citrus_memory_stream_skip_ws(struct _citrus_memory_stream *ms)
{
	int ch;

	while ((ch = _citrus_memory_stream_peek(ms)) != EOF) {
		if (!_bcs_isspace(ch))
			break;
		_citrus_memory_stream_getc(ms);
	}
}
