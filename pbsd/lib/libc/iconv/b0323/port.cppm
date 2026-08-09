module;

#include <assert.h>
#include <cerrno>
#include <climits>
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

export module pbsd.lib.libc.iconv.b0323;

export namespace pbsd::lib_libc_iconv::b0323 {

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif
#ifndef __DECONST
#define __DECONST(type, var) ((type)(uintptr_t)(const void *)(var))
#endif
#ifndef __inline
#define __inline inline
#endif
#ifndef __BEGIN_DECLS
#define __BEGIN_DECLS
#define __END_DECLS
#endif

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

static __inline int
_citrus_region_check(const struct _citrus_region *r, size_t ofs, size_t sz)
{

	return (r->r_size >= ofs + sz ? 0 : -1);
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

#define _region			_citrus_region
#define _region_init		_citrus_region_init
#define _region_head		_citrus_region_head
#define _region_size		_citrus_region_size
#define _region_check		_citrus_region_check
#define _region_offset		_citrus_region_offset
#define _region_peek8		_citrus_region_peek8

#define _CITRUS_BCS_PRED(_name_, _cond_) \
static __inline int _citrus_bcs_##_name_(uint8_t c) { return (_cond_); }

_CITRUS_BCS_PRED(isblank, c == ' ' || c == '\t')
_CITRUS_BCS_PRED(iseol, c == '\n' || c == '\r')
_CITRUS_BCS_PRED(isspace, _citrus_bcs_isblank(c) || _citrus_bcs_iseol(c) ||
    c == '\f' || c == '\v')
_CITRUS_BCS_PRED(isdigit, c >= '0' && c <= '9')
_CITRUS_BCS_PRED(isupper, c >= 'A' && c <= 'Z')
_CITRUS_BCS_PRED(islower, c >= 'a' && c <= 'z')
_CITRUS_BCS_PRED(isalpha, _citrus_bcs_isupper(c) || _citrus_bcs_islower(c))
_CITRUS_BCS_PRED(isalnum, _citrus_bcs_isdigit(c) || _citrus_bcs_isalpha(c))
_CITRUS_BCS_PRED(isxdigit, _citrus_bcs_isdigit(c) ||
    (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))

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

#define _bcs_isalnum		_citrus_bcs_isalnum
#define _bcs_isalpha		_citrus_bcs_isalpha
#define _bcs_isblank		_citrus_bcs_isblank
#define _bcs_isdigit		_citrus_bcs_isdigit
#define _bcs_islower		_citrus_bcs_islower
#define _bcs_iseol		_citrus_bcs_iseol
#define _bcs_isspace		_citrus_bcs_isspace
#define _bcs_isupper		_citrus_bcs_isupper
#define _bcs_isxdigit		_citrus_bcs_isxdigit
#define _bcs_tolower		_citrus_bcs_tolower
#define _bcs_toupper		_citrus_bcs_toupper

static int
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

#define _bcs_strcasecmp		_citrus_bcs_strcasecmp

struct _citrus_memory_stream {
	struct _citrus_region	ms_region;
	size_t			ms_pos;
};

static __inline int
_citrus_memory_stream_iseof(struct _citrus_memory_stream *ms)
{

	return (ms->ms_pos >= _citrus_region_size(&ms->ms_region));
}

static __inline void
_citrus_memory_stream_bind_ptr(struct _citrus_memory_stream * __restrict ms,
    void *ptr, size_t sz)
{
	struct _citrus_region r;

	_citrus_region_init(&r, ptr, sz);
	ms->ms_region = r;
	ms->ms_pos = 0;
}

static __inline int
_citrus_memory_stream_getc(struct _citrus_memory_stream *ms)
{

	if (_citrus_memory_stream_iseof(ms))
		return (EOF);
	return (_citrus_region_peek8(&ms->ms_region, ms->ms_pos++));
}

static __inline void
_citrus_memory_stream_ungetc(struct _citrus_memory_stream *ms, int ch)
{

	if (ch != EOF && ms->ms_pos > 0)
		ms->ms_pos--;
}

static __inline int
_citrus_memory_stream_peek(struct _citrus_memory_stream *ms)
{

	if (_citrus_memory_stream_iseof(ms))
		return (EOF);
	return (_citrus_region_peek8(&ms->ms_region, ms->ms_pos));
}

#define _memstream		_citrus_memory_stream
#define _memstream_bind_ptr	_citrus_memory_stream_bind_ptr
#define _memstream_getc		_citrus_memory_stream_getc
#define _memstream_ungetc	_citrus_memory_stream_ungetc
#define _memstream_peek		_citrus_memory_stream_peek

static void
_citrus_memory_stream_skip_ws(struct _citrus_memory_stream *ms)
{
	int ch;

	while ((ch = _memstream_peek(ms)) != EOF) {
		if (!_bcs_isspace(ch))
			break;
		_memstream_getc(ms);
	}
}

#define _memstream_skip_ws	_citrus_memory_stream_skip_ws

typedef enum {
	_CITRUS_PROP_BOOL = 0,
	_CITRUS_PROP_STR  = 1,
	_CITRUS_PROP_CHR  = 2,
	_CITRUS_PROP_NUM  = 3,
} _citrus_prop_type_t;

typedef struct _citrus_prop_hint_t _citrus_prop_hint_t;

#define _CITRUS_PROP_CB0_T(_func_, _type_) \
typedef int (*_citrus_prop_##_func_##_cb_func_t) \
    (void * __restrict, const char *, _type_); \
typedef struct { \
	_citrus_prop_##_func_##_cb_func_t func; \
} _citrus_prop_##_func_##_cb_t;
_CITRUS_PROP_CB0_T(boolean, int)
_CITRUS_PROP_CB0_T(str, const char *)
#undef _CITRUS_PROP_CB0_T

#define _CITRUS_PROP_CB1_T(_func_, _type_) \
typedef int (*_citrus_prop_##_func_##_cb_func_t) \
    (void * __restrict, const char *, _type_, _type_); \
typedef struct { \
	_citrus_prop_##_func_##_cb_func_t func; \
} _citrus_prop_##_func_##_cb_t;
_CITRUS_PROP_CB1_T(chr, int)
_CITRUS_PROP_CB1_T(num, uint64_t)
#undef _CITRUS_PROP_CB1_T

struct _citrus_prop_hint_t {
	const char *name;
	_citrus_prop_type_t type;
#define _CITRUS_PROP_CB_T_OPS(_name_) \
	_citrus_prop_##_name_##_cb_t _name_
	union {
		_CITRUS_PROP_CB_T_OPS(boolean);
		_CITRUS_PROP_CB_T_OPS(str);
		_CITRUS_PROP_CB_T_OPS(chr);
		_CITRUS_PROP_CB_T_OPS(num);
	} cb;
};

#define _CITRUS_PROP_HINT_BOOL(name, cb) \
    { name, _CITRUS_PROP_BOOL, { .boolean = { cb } } }
#define _CITRUS_PROP_HINT_STR(name, cb) \
    { name, _CITRUS_PROP_STR, { .str = { cb } } }
#define _CITRUS_PROP_HINT_CHR(name, cb) \
    { name, _CITRUS_PROP_CHR, { .chr = { cb } } }
#define _CITRUS_PROP_HINT_NUM(name, cb) \
    { name, _CITRUS_PROP_NUM, { .num = { cb } } }
#define _CITRUS_PROP_HINT_END \
    { .name = NULL }


/* $NetBSD: citrus_prop.c,v 1.4 2011/03/30 08:22:01 jruoho Exp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c)2006 Citrus Project,
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
 *
 */




typedef struct {
	_citrus_prop_type_t type;
	union {
		const char *str;
		int chr;
		bool boolean;
		uint64_t num;
	} u;
} _citrus_prop_object_t;

void
_citrus_prop_object_init(_citrus_prop_object_t *obj, _citrus_prop_type_t type)
{

	obj->type = type;
	memset(&obj->u, 0, sizeof(obj->u));
}

void
_citrus_prop_object_uninit(_citrus_prop_object_t *obj)
{

	if (obj->type == _CITRUS_PROP_STR)
		free(__DECONST(void *, obj->u.str));
}

static const char *xdigit = "0123456789ABCDEF";

#define _CITRUS_PROP_READ_UINT_COMMON(_func_, _type_, _max_)		\
int									\
_citrus_prop_read_##_func_##_common(struct _memstream * __restrict ms,	\
    _type_ * __restrict result, int base, int neg)			\
{									\
	_type_ acc, cutoff;						\
	int ch, cutlim, n;						\
	const char *p;							\
									\
	acc = (_type_)0;						\
	cutoff = _max_ / base;						\
	cutlim = _max_ % base;						\
	for (;;) {							\
		ch = _memstream_getc(ms);				\
		p = strchr(xdigit, _bcs_toupper(ch));			\
		if (p == NULL || (n = (p - xdigit)) >= base)		\
			break;						\
		if (acc > cutoff || (acc == cutoff && n > cutlim))	\
			break;						\
		acc *= base;						\
		acc += n;						\
	}								\
	_memstream_ungetc(ms, ch);					\
	*result = neg ? -acc : acc;					\
	return (0);							\
}
_CITRUS_PROP_READ_UINT_COMMON(chr, int, UCHAR_MAX)
_CITRUS_PROP_READ_UINT_COMMON(num, uint64_t, UINT64_MAX)
#undef _CITRUS_PROP_READ_UINT_COMMON

#define _CITRUS_PROP_READ_INT(_func_, _type_)			\
int								\
_citrus_prop_read_##_func_(struct _memstream * __restrict ms,	\
    _citrus_prop_object_t * __restrict obj)			\
{								\
	int base, ch, neg;					\
								\
	_memstream_skip_ws(ms);					\
	ch = _memstream_getc(ms);				\
	neg = 0;						\
	switch (ch) {						\
	case '-':						\
		neg = 1;					\
	case '+':						\
		ch = _memstream_getc(ms);			\
	}							\
	base = 10;						\
	if (ch == '0') {					\
		base -= 2;					\
		ch = _memstream_getc(ms);			\
		if (ch == 'x' || ch == 'X') {			\
			ch = _memstream_getc(ms);		\
			if (_bcs_isxdigit(ch) == 0) {		\
				_memstream_ungetc(ms, ch);	\
				obj->u._func_ = 0;		\
				return (0);			\
			}					\
			base += 8;				\
		}						\
	} else if (_bcs_isdigit(ch) == 0)			\
		return (EINVAL);				\
	_memstream_ungetc(ms, ch);				\
	return (_citrus_prop_read_##_func_##_common		\
	    (ms, &obj->u._func_, base, neg));			\
}
_CITRUS_PROP_READ_INT(chr, int)
_CITRUS_PROP_READ_INT(num, uint64_t)
#undef _CITRUS_PROP_READ_INT

int
_citrus_prop_read_character_common(struct _memstream * __restrict ms,
    int * __restrict result)
{
	int base, ch;

	ch = _memstream_getc(ms);
	if (ch != '\\')
		*result = ch;
	else {
		ch = _memstream_getc(ms);
		base = 16;
		switch (ch) {
		case 'a':
			*result = '\a';
			break;
		case 'b':
			*result = '\b';
			break;
		case 'f':
			*result = '\f';
			break;
		case 'n':
			*result = '\n';
			break;
		case 'r':
			*result = '\r';
			break;
		case 't':
			*result = '\t';
			break;
		case 'v':
			*result = '\v';
			break;
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			_memstream_ungetc(ms, ch);
			base -= 8;
			/*FALLTHROUGH*/
		case 'x':
			return (_citrus_prop_read_chr_common(ms, result,
			    base, 0));
			/*NOTREACHED*/
		default:
			/* unknown escape */
			*result = ch;
		}
	}
	return (0);
}

int
_citrus_prop_read_character(struct _memstream * __restrict ms,
    _citrus_prop_object_t * __restrict obj)
{
	int ch, errnum;

	_memstream_skip_ws(ms);
	ch = _memstream_getc(ms);
	if (ch != '\'') {
		_memstream_ungetc(ms, ch);
		return (_citrus_prop_read_chr(ms, obj));
	}
	errnum = _citrus_prop_read_character_common(ms, &ch);
	if (errnum != 0)
		return (errnum);
	obj->u.chr = ch;
	ch = _memstream_getc(ms);
	if (ch != '\'')
		return (EINVAL);
	return (0);
}

int
_citrus_prop_read_bool(struct _memstream * __restrict ms,
    _citrus_prop_object_t * __restrict obj)
{

	_memstream_skip_ws(ms);
	switch (_bcs_tolower(_memstream_getc(ms))) {
	case 't':
		if (_bcs_tolower(_memstream_getc(ms)) == 'r' &&
		    _bcs_tolower(_memstream_getc(ms)) == 'u' &&
		    _bcs_tolower(_memstream_getc(ms)) == 'e') {
			obj->u.boolean = true;
			return (0);
		}
		break;
	case 'f':
		if (_bcs_tolower(_memstream_getc(ms)) == 'a' &&
		    _bcs_tolower(_memstream_getc(ms)) == 'l' &&
		    _bcs_tolower(_memstream_getc(ms)) == 's' &&
		    _bcs_tolower(_memstream_getc(ms)) == 'e') {
			obj->u.boolean = false;
			return (0);
		}
	}
	return (EINVAL);
}

int
_citrus_prop_read_str(struct _memstream * __restrict ms,
    _citrus_prop_object_t * __restrict obj)
{
	int ch, errnum, quot;
	char *s, *t;
#define _CITRUS_PROP_STR_BUFSIZ	512
	size_t m, n;

	m = _CITRUS_PROP_STR_BUFSIZ;
	s = (char *)malloc(m);
	if (s == NULL)
		return (ENOMEM);
	n = 0;
	_memstream_skip_ws(ms);
	quot = _memstream_getc(ms);
	switch (quot) {
	case EOF:
		goto done;
		/*NOTREACHED*/
	case '\\':
		_memstream_ungetc(ms, quot);
		quot = EOF;
		/*FALLTHROUGH*/
	case '\"': case '\'':
		break;
	default:
		s[n] = quot;
		++n, --m;
		quot = EOF;
	}
	for (;;) {
		if (m < 1) {
			m = _CITRUS_PROP_STR_BUFSIZ;
			t = (char *)realloc(s, n + m);
			if (t == NULL) {
				free(s);
				return (ENOMEM);
			}
			s = t;
		}
		ch = _memstream_getc(ms);
		if (quot == ch || (quot == EOF &&
		    (ch == ';' || _bcs_isspace(ch)))) {
done:
			s[n] = '\0';
			obj->u.str = (const char *)s;
			return (0);
		}
		_memstream_ungetc(ms, ch);
		errnum = _citrus_prop_read_character_common(ms, &ch);
		if (errnum != 0) {
			free(s);
			return (errnum);
		}
		s[n] = ch;
		++n, --m;
	}
	free(s);
	return (EINVAL);
#undef _CITRUS_PROP_STR_BUFSIZ
}

typedef int (*_citrus_prop_read_type_t)(struct _memstream * __restrict,
    _citrus_prop_object_t * __restrict);

const _citrus_prop_read_type_t readers[] = {
	_citrus_prop_read_bool,
	_citrus_prop_read_str,
	_citrus_prop_read_character,
	_citrus_prop_read_num,
};

int
_citrus_prop_read_symbol(struct _memstream * __restrict ms,
    char * __restrict s, size_t n)
{
	int ch;
	size_t m;

	for (m = 0; m < n; ++m) {
		ch = _memstream_getc(ms);
		if (ch != '_' && _bcs_isalnum(ch) == 0)
			goto name_found;
		s[m] = ch;
	}
	ch = _memstream_getc(ms);
	if (ch == '_' || _bcs_isalnum(ch) != 0)
		return (EINVAL);

name_found:
	_memstream_ungetc(ms, ch);
	s[m] = '\0';

	return (0);
}

int
_citrus_prop_parse_element(struct _memstream * __restrict ms,
    const _citrus_prop_hint_t * __restrict hints, void * __restrict context)
{
	int ch, errnum;
#define _CITRUS_PROP_HINT_NAME_LEN_MAX	255
	char name[_CITRUS_PROP_HINT_NAME_LEN_MAX + 1];
	const _citrus_prop_hint_t *hint;
	_citrus_prop_object_t ostart, oend;

	errnum = _citrus_prop_read_symbol(ms, name, sizeof(name));
	if (errnum != 0)
		return (errnum);
	for (hint = hints; hint->name != NULL; ++hint)
		if (_citrus_bcs_strcasecmp(name, hint->name) == 0)
			goto hint_found;
	return (EINVAL);

hint_found:
	_memstream_skip_ws(ms);
	ch = _memstream_getc(ms);
	if (ch != '=' && ch != ':')
		_memstream_ungetc(ms, ch);
	do {
		_citrus_prop_object_init(&ostart, hint->type);
		_citrus_prop_object_init(&oend, hint->type);
		errnum = (*readers[hint->type])(ms, &ostart);
		if (errnum != 0)
			return (errnum);
		_memstream_skip_ws(ms);
		ch = _memstream_getc(ms);
		switch (hint->type) {
		case _CITRUS_PROP_BOOL:
			/*FALLTHROUGH*/
		case _CITRUS_PROP_STR:
			break;
		default:
			if (ch != '-')
				break;
			errnum = (*readers[hint->type])(ms, &oend);
			if (errnum != 0)
				return (errnum);
			_memstream_skip_ws(ms);
			ch = _memstream_getc(ms);
		}
#define CALL0(_func_)					\
do {							\
	errnum = (*hint->cb._func_.func)(context,	\
	    hint->name,	ostart.u._func_);		\
} while (0)
#define CALL1(_func_)					\
do {							\
	errnum = (*hint->cb._func_.func)(context,	\
	    hint->name,	ostart.u._func_, oend.u._func_);\
} while (0)
		switch (hint->type) {
		case _CITRUS_PROP_BOOL:
			CALL0(boolean);
			break;
		case _CITRUS_PROP_STR:
			CALL0(str);
			break;
		case _CITRUS_PROP_CHR:
			CALL1(chr);
			break;
		case _CITRUS_PROP_NUM:
			CALL1(num);
			break;
		default:
			abort();
			/*NOTREACHED*/
		}
#undef CALL0
#undef CALL1
		_citrus_prop_object_uninit(&ostart);
		_citrus_prop_object_uninit(&oend);
		if (errnum != 0)
			return (errnum);
	} while (ch == ',');
	if (ch != ';')
		_memstream_ungetc(ms, ch);
	return (0);
}

int
_citrus_prop_parse_variable(const _citrus_prop_hint_t * __restrict hints,
    void * __restrict context, const void *var, size_t lenvar)
{
	struct _memstream ms;
	int ch, errnum;

	_memstream_bind_ptr(&ms, __DECONST(void *, var), lenvar);
	for (;;) {
		_memstream_skip_ws(&ms);
		ch = _memstream_getc(&ms);
		if (ch == EOF || ch == '\0')
			break;
		_memstream_ungetc(&ms, ch);
		errnum = _citrus_prop_parse_element(&ms, hints, context);
		if (errnum != 0)
			return (errnum);
	}
	return (0);
}

} // namespace
