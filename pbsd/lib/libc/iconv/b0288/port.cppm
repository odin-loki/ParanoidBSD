module;

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

export module pbsd.lib.libc.iconv.b0288;

export namespace pbsd::lib_libc_iconv::b0288 {

#ifndef __inline
#define __inline inline
#endif

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

#define _bcs_iseol _citrus_bcs_iseol
#define _bcs_isspace _citrus_bcs_isspace
#define _bcs_tolower _citrus_bcs_tolower
#define _bcs_toupper _citrus_bcs_toupper
#define _bcs_skip_ws_len _citrus_bcs_skip_ws_len
#define _bcs_skip_nonws_len _citrus_bcs_skip_nonws_len
#define _bcs_trunc_rws_len _citrus_bcs_trunc_rws_len
#define _bcs_strncasecmp _citrus_bcs_strncasecmp

struct _citrus_region {
	void	*r_head;
	size_t	 r_size;
};

__inline void
_citrus_region_init(struct _citrus_region *r, void *h, size_t sz)
{
	r->r_head = h;
	r->r_size = sz;
}

__inline void *
_citrus_region_head(const struct _citrus_region *r)
{
	return (r->r_head);
}

__inline size_t
_citrus_region_size(const struct _citrus_region *r)
{
	return (r->r_size);
}

__inline void *
_citrus_region_offset(const struct _citrus_region *r, size_t pos)
{
	return ((void *)((uint8_t *)r->r_head + pos));
}

#define _region _citrus_region
#define _region_init _citrus_region_init
#define _region_head _citrus_region_head
#define _region_size _citrus_region_size
#define _region_offset _citrus_region_offset

struct _citrus_memory_stream {
	struct _citrus_region	ms_region;
	size_t			ms_pos;
};

static __inline int
_citrus_memory_stream_iseof(struct _citrus_memory_stream *ms)
{
	return (ms->ms_pos >= _citrus_region_size(&ms->ms_region));
}

__inline void
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
_citrus_memory_stream_getc(struct _citrus_memory_stream *ms)
{
	if (_citrus_memory_stream_iseof(ms))
		return (EOF);
	return (*(uint8_t *)_citrus_region_offset(&ms->ms_region, ms->ms_pos++));
}

static __inline int
_citrus_memory_stream_peek(struct _citrus_memory_stream *ms)
{
	if (_citrus_memory_stream_iseof(ms))
		return (EOF);
	return (*(uint8_t *)_citrus_region_offset(&ms->ms_region, ms->ms_pos));
}

#define _memstream_remainder _citrus_memory_stream_remainder

/* $NetBSD: citrus_bcs.c,v 1.5 2005/05/14 17:55:42 tshiozak Exp $ */

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
	return ((c1 == c2) ? 0 : ((c1 <= c2) ? 1 : -1));
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
_citrus_bcs_skip_ws(const char *p)
{
	while (*p && _bcs_isspace(*p))
		p++;
	return (p);
}

const char *
_citrus_bcs_skip_nonws(const char *p)
{
	while (*p && !_bcs_isspace(*p))
		p++;
	return (p);
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

void
_citrus_bcs_convert_to_upper(char *s)
{
	while (*s) {
		*s = _bcs_toupper(*s);
		s++;
	}
}

/*	$NetBSD: citrus_memstream.c,v 1.5 2012/03/13 21:13:31 christos Exp $	*/

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

		q = (const char *)memchr(p, T_COMM, len);
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
_citrus_memory_stream_chr(struct _citrus_memory_stream *ms,
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
_citrus_memory_stream_skip_ws(struct _citrus_memory_stream *ms)
{
	int ch;

	while ((ch = _citrus_memory_stream_peek(ms)) != EOF) {
		if (!_bcs_isspace(ch))
			break;
		_citrus_memory_stream_getc(ms);
	}
}

} /* namespace */
