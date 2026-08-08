/*-
 * PBSD batch b0156 -- C++23 module port of
 *
 *	hbsd/src/lib/libc/locale/table.c
 *	hbsd/src/lib/libc/locale/utf8.c
 *	hbsd/src/lib/libc/locale/euc.c
 *	hbsd/src/lib/libc/locale/collate.c
 */

module;

#include <cassert>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef SIZE_T_MAX
#define SIZE_T_MAX	SIZE_MAX
#endif

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#ifndef MB_CUR_MAX
#define MB_CUR_MAX	4
#endif

#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif

#ifndef SS2
#define SS2	0x8e
#endif

#ifndef SS3
#define SS3	0x8f
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC	0
#endif

#ifndef isascii
#define isascii(c)	((unsigned)(c) <= 0x7f)
#endif

export module pbsd.lib.libc.locale.b0156;

extern "C" {
int asprintf(char **, const char *, ...);
size_t strlcat(char *, const char *, size_t);
wchar_t *wcsdup(const wchar_t *);
}

extern "C" {
typedef struct {
	int		open_ret;
	int		fstat_ret;
	off_t		st_size;
	const void	*map_data;
	size_t		map_len;
	unsigned int	call_count;
} pbsd_collate_file_hook_t;

extern pbsd_collate_file_hook_t pbsd_collate_file_hook;
extern const char *pbsd_PathLocale;

void pbsd_reset_collate_hooks(void);
}

export namespace pbsd::lib_libc_locale::b0156 {

using __ct_rune_t = int;
using __rune_t = __ct_rune_t;

#ifndef __unused
#define __unused	[[maybe_unused]]
#endif

#ifdef COLL_WEIGHTS_MAX
#undef COLL_WEIGHTS_MAX
#endif

#define _RUNE_MAGIC_1	{'R','u','n','e','M','a','g','1'}
#define _CTYPE_A	0x00000100
#define _CTYPE_C	0x00000200
#define _CTYPE_D	0x00000400
#define _CTYPE_G	0x00000800
#define _CTYPE_L	0x00001000
#define _CTYPE_P	0x00002000
#define _CTYPE_R	0x00004000
#define _CTYPE_S	0x00008000
#define _CTYPE_U	0x00010000
#define _CTYPE_X	0x00020000
#define _CTYPE_B	0x00040000
#define _CTYPE_N	0x01000000

struct _RuneEntry {
	__rune_t	__min;
	__rune_t	__max;
	__rune_t	__map;
	unsigned long	*__types;
};

struct _RuneRange {
	int		__nranges;
	_RuneEntry	*__ranges;
};

struct _RuneLocale {
	char		__magic[8];
	char		__encoding[32];
	__rune_t	(*__sgetrune)(const char *, std::size_t, char const **);
	int		(*__sputrune)(__rune_t, char *, std::size_t, char **);
	__rune_t	__invalid_rune;
	unsigned long	__runetype[256];
	__rune_t	__maplower[256];
	__rune_t	__mapupper[256];
	_RuneRange	__runetype_ext;
	_RuneRange	__maplower_ext;
	_RuneRange	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
};

union mbstate_t {
	char		__mbstate8[128];
	long long	_mbstateL;
};

struct xlocale_ctype {
	std::size_t	(*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
		    std::size_t, mbstate_t * __restrict);
	std::size_t	(*__wcrtomb)(char * __restrict, wchar_t, mbstate_t * __restrict);
	int		(*__mbsinit)(const mbstate_t *);
	std::size_t	(*__mbsnrtowcs)(wchar_t * __restrict,
		    const char ** __restrict, std::size_t, std::size_t,
		    mbstate_t * __restrict);
	std::size_t	(*__wcsnrtombs)(char * __restrict,
		    const wchar_t ** __restrict, std::size_t, std::size_t,
		    mbstate_t * __restrict);
	_RuneLocale	*runes;
	int		__mb_cur_max;
	int		__mb_sb_limit;
};

struct xlocale {
	void		*components[8];
};

using locale_t = xlocale *;

enum { XLC_COLLATE = 0, XLC_CTYPE = 1 };

#define FIX_LOCALE(loc)	if ((loc) == NULL) (loc) = port_get_locale()
#define XLOCALE_CTYPE(l)	((xlocale_ctype *)(l)->components[XLC_CTYPE])

using mbrtowc_pfn_t = std::size_t (*)(wchar_t * __restrict,
    const char * __restrict, std::size_t, mbstate_t * __restrict);
using wcrtomb_pfn_t = std::size_t (*)(char * __restrict, wchar_t,
    mbstate_t * __restrict);

extern int __mb_sb_limit;

#define COLLATE_FMT_VERSION		"FreeBSD-1.0"
#define COLLATE_FMT_VERSION_LEN		(sizeof(COLLATE_FMT_VERSION) - 1)
#define XLOCALE_DEF_VERSION_LEN		16
#define _LDP_LOADED			1
#define _LDP_ERROR			(-1)
#define _LDP_CACHE			2
#define COLLATE_WEIGHTS_MAX		10
#define COLL_WEIGHTS_MAX		COLLATE_WEIGHTS_MAX
#define COLLATE_STR_LEN			32
#define DIRECTIVE_FORWARD		0x01
#define DIRECTIVE_BACKWARD		0x02
#define DIRECTIVE_POSITION		0x04
#define DIRECTIVE_UNDEFINED		0x08
#define COLLATE_SUBST_PRIORITY		0x80000000
#define COLLATE_MAX_PRIORITY		0x7fffffff
#define IGNORE_EQUIV_CLASS		(-2)

struct collate_info_t {
	int directive_count;
	int chain_count;
	int large_count;
	int subst_count[COLLATE_WEIGHTS_MAX];
	int directive[COLLATE_WEIGHTS_MAX];
	int undef_pri[COLLATE_WEIGHTS_MAX];
	uint32_t pri_count[COLLATE_WEIGHTS_MAX];
	int chain_max_len;
};

struct collate_char_t {
	int pri[COLLATE_WEIGHTS_MAX];
};

struct collate_chain_t {
	wchar_t str[COLLATE_STR_LEN];
	int pri[COLLATE_WEIGHTS_MAX];
};

struct collate_large_t {
	wchar_t val;
	struct { int pri[COLLATE_WEIGHTS_MAX]; } pri;
};

struct collate_subst_t {
	wchar_t key;
	int32_t pri[16];
};

struct xlocale_component_header {
	void		(*destructor)(void *);
};

struct xlocale_collate {
	struct {
		xlocale_component_header header;
		char		version[XLOCALE_DEF_VERSION_LEN];
		char		name[32];
	} header;
	int		__collate_load_error;
	char		*map;
	std::size_t	maplen;
	collate_info_t	*info;
	collate_char_t	*char_pri_table;
	collate_subst_t	*subst_table[COLLATE_WEIGHTS_MAX];
	collate_chain_t	*chain_pri_table;
	collate_large_t	*large_pri_table;
};

xlocale	port___xlocale_global_locale;

#define _PathLocale	pbsd_PathLocale

locale_t port_get_locale()
{
	return (&port___xlocale_global_locale);
}

#define __get_locale()	port_get_locale()

void xlocale_release(void *t)
{
	std::free(t);
}

int port_open(const char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	pbsd_collate_file_hook.call_count++;
	return (pbsd_collate_file_hook.open_ret);
}

int port_close(int fd)
{
	(void)fd;
	return (0);
}

int port_fstat(int fd, struct stat *sb)
{
	(void)fd;
	pbsd_collate_file_hook.call_count++;
	if (pbsd_collate_file_hook.fstat_ret < 0)
		return (-1);
	sb->st_size = pbsd_collate_file_hook.st_size;
	return (0);
}

void *port_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	void *p;
	(void)addr;
	(void)prot;
	(void)flags;
	(void)fd;
	(void)offset;
	if (pbsd_collate_file_hook.map_data == NULL || len > pbsd_collate_file_hook.map_len)
		return ((void *)MAP_FAILED);
	p = std::malloc(len);
	if (p == NULL)
		return ((void *)MAP_FAILED);
	std::memcpy(p, pbsd_collate_file_hook.map_data, len);
	return (p);
}

int port_munmap(void *addr, size_t len)
{
	(void)len;
	std::free(addr);
	return (0);
}

#define _open	port_open
#define _close	port_close
#define _fstat	port_fstat
#define mmap	port_mmap
#define munmap	port_munmap

std::size_t port_mbrtowc(wchar_t *pwc, const char *s, std::size_t n, mbstate_t *ps)
{
	unsigned char c;
	(void)ps;
	if (s == NULL)
		return (0);
	if (n == 0)
		return ((std::size_t)-2);
	c = (unsigned char)*s;
	if (c < 0x80) {
		if (pwc)
			*pwc = c;
		return (c == 0 ? 0 : 1);
	}
	errno = EILSEQ;
	return ((std::size_t)-1);
}

#define mbrtowc	port_mbrtowc

std::size_t wcsnlen(const wchar_t *s, std::size_t maxlen)
{
	std::size_t i;
	for (i = 0; i < maxlen && s[i] != 0; i++)
		;
	return (i);
}

std::size_t
__mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
    std::size_t nms, std::size_t len, mbstate_t * __restrict ps,
    mbrtowc_pfn_t pmbrtowc)
{
	const char *s;
	std::size_t nchr;
	wchar_t wc;
	std::size_t nb;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		for (;;) {
			if ((nb = pmbrtowc(&wc, s, nms, ps)) == (std::size_t)-1)
				return ((std::size_t)-1);
			else if (nb == 0 || nb == (std::size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
	}

	while (len-- > 0) {
		if ((nb = pmbrtowc(dst, s, nms, ps)) == (std::size_t)-1) {
			*src = s;
			return ((std::size_t)-1);
		} else if (nb == (std::size_t)-2) {
			*src = s + nms;
			return (nchr);
		} else if (nb == 0) {
			*src = NULL;
			return (nchr);
		}
		s += nb;
		nms -= nb;
		nchr++;
		dst++;
	}
	*src = s;
	return (nchr);
}

std::size_t
__wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    std::size_t nwc, std::size_t len, mbstate_t * __restrict ps,
    wcrtomb_pfn_t pwcrtomb)
{
	mbstate_t mbsbak;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	std::size_t nbytes;
	std::size_t nb;

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if ((nb = pwcrtomb(buf, *s, ps)) == (std::size_t)-1)
				return ((std::size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (len > (std::size_t)MB_CUR_MAX) {
			if ((nb = pwcrtomb(dst, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
		} else {
			mbsbak = *ps;
			if ((nb = pwcrtomb(buf, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
			if (nb > (int)len) {
				*ps = mbsbak;
				break;
			}
			std::memcpy(dst, buf, nb);
		}
		if (*s == L'\0') {
			*src = NULL;
			return (nbytes + nb - 1);
		}
		s++;
		dst += nb;
		len -= nb;
		nbytes += nb;
	}
	*src = s;
	return (nbytes);
}



/* ----- hbsd/src/lib/libc/locale/table.c ----- */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
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


const _RuneLocale _DefaultRuneLocale = {
    _RUNE_MAGIC_1,
    "NONE",
    NULL,
    NULL,
    0xFFFD,

    {	/*00*/	_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
	/*08*/	_CTYPE_C,
		_CTYPE_C|_CTYPE_S|_CTYPE_B,
		_CTYPE_C|_CTYPE_S,
		_CTYPE_C|_CTYPE_S,
		_CTYPE_C|_CTYPE_S,
		_CTYPE_C|_CTYPE_S,
		_CTYPE_C,
		_CTYPE_C,
	/*10*/	_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
	/*18*/	_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
		_CTYPE_C,
	/*20*/	_CTYPE_S|_CTYPE_B|_CTYPE_R,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
	/*28*/	_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
	/*30*/	_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|0,
		_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|1,
		_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|2,
		_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|3,
		_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|4,
		_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|5,
		_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|6,
		_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|7,
	/*38*/	_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|8,
		_CTYPE_D|_CTYPE_R|_CTYPE_G|_CTYPE_X|_CTYPE_N|9,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
	/*40*/	_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_U|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|10,
		_CTYPE_U|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|11,
		_CTYPE_U|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|12,
		_CTYPE_U|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|13,
		_CTYPE_U|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|14,
		_CTYPE_U|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|15,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
	/*48*/	_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
	/*50*/	_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
	/*58*/	_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_U|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
	/*60*/	_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_L|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|10,
		_CTYPE_L|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|11,
		_CTYPE_L|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|12,
		_CTYPE_L|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|13,
		_CTYPE_L|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|14,
		_CTYPE_L|_CTYPE_X|_CTYPE_R|_CTYPE_G|_CTYPE_A|15,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
	/*68*/	_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
	/*70*/	_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
	/*78*/	_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_L|_CTYPE_R|_CTYPE_G|_CTYPE_A,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_P|_CTYPE_R|_CTYPE_G,
		_CTYPE_C,
    },
    {	0x00,	0x01,	0x02,	0x03,	0x04,	0x05,	0x06,	0x07,
     	0x08,	0x09,	0x0a,	0x0b,	0x0c,	0x0d,	0x0e,	0x0f,
	0x10,	0x11,	0x12,	0x13,	0x14,	0x15,	0x16,	0x17,
     	0x18,	0x19,	0x1a,	0x1b,	0x1c,	0x1d,	0x1e,	0x1f,
	0x20,	0x21,	0x22,	0x23,	0x24,	0x25,	0x26,	0x27,
     	0x28,	0x29,	0x2a,	0x2b,	0x2c,	0x2d,	0x2e,	0x2f,
	0x30,	0x31,	0x32,	0x33,	0x34,	0x35,	0x36,	0x37,
     	0x38,	0x39,	0x3a,	0x3b,	0x3c,	0x3d,	0x3e,	0x3f,
	0x40,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
     	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
     	'x',	'y',	'z',	0x5b,	0x5c,	0x5d,	0x5e,	0x5f,
	0x60,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
     	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
     	'x',	'y',	'z',	0x7b,	0x7c,	0x7d,	0x7e,	0x7f,
	0x80,	0x81,	0x82,	0x83,	0x84,	0x85,	0x86,	0x87,
     	0x88,	0x89,	0x8a,	0x8b,	0x8c,	0x8d,	0x8e,	0x8f,
	0x90,	0x91,	0x92,	0x93,	0x94,	0x95,	0x96,	0x97,
     	0x98,	0x99,	0x9a,	0x9b,	0x9c,	0x9d,	0x9e,	0x9f,
	0xa0,	0xa1,	0xa2,	0xa3,	0xa4,	0xa5,	0xa6,	0xa7,
     	0xa8,	0xa9,	0xaa,	0xab,	0xac,	0xad,	0xae,	0xaf,
	0xb0,	0xb1,	0xb2,	0xb3,	0xb4,	0xb5,	0xb6,	0xb7,
     	0xb8,	0xb9,	0xba,	0xbb,	0xbc,	0xbd,	0xbe,	0xbf,
	0xc0,	0xc1,	0xc2,	0xc3,	0xc4,	0xc5,	0xc6,	0xc7,
     	0xc8,	0xc9,	0xca,	0xcb,	0xcc,	0xcd,	0xce,	0xcf,
	0xd0,	0xd1,	0xd2,	0xd3,	0xd4,	0xd5,	0xd6,	0xd7,
     	0xd8,	0xd9,	0xda,	0xdb,	0xdc,	0xdd,	0xde,	0xdf,
	0xe0,	0xe1,	0xe2,	0xe3,	0xe4,	0xe5,	0xe6,	0xe7,
     	0xe8,	0xe9,	0xea,	0xeb,	0xec,	0xed,	0xee,	0xef,
	0xf0,	0xf1,	0xf2,	0xf3,	0xf4,	0xf5,	0xf6,	0xf7,
     	0xf8,	0xf9,	0xfa,	0xfb,	0xfc,	0xfd,	0xfe,	0xff,
    },
    {	0x00,	0x01,	0x02,	0x03,	0x04,	0x05,	0x06,	0x07,
     	0x08,	0x09,	0x0a,	0x0b,	0x0c,	0x0d,	0x0e,	0x0f,
	0x10,	0x11,	0x12,	0x13,	0x14,	0x15,	0x16,	0x17,
     	0x18,	0x19,	0x1a,	0x1b,	0x1c,	0x1d,	0x1e,	0x1f,
	0x20,	0x21,	0x22,	0x23,	0x24,	0x25,	0x26,	0x27,
     	0x28,	0x29,	0x2a,	0x2b,	0x2c,	0x2d,	0x2e,	0x2f,
	0x30,	0x31,	0x32,	0x33,	0x34,	0x35,	0x36,	0x37,
     	0x38,	0x39,	0x3a,	0x3b,	0x3c,	0x3d,	0x3e,	0x3f,
	0x40,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
     	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
     	'X',	'Y',	'Z',	0x5b,	0x5c,	0x5d,	0x5e,	0x5f,
	0x60,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
     	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
     	'X',	'Y',	'Z',	0x7b,	0x7c,	0x7d,	0x7e,	0x7f,
	0x80,	0x81,	0x82,	0x83,	0x84,	0x85,	0x86,	0x87,
     	0x88,	0x89,	0x8a,	0x8b,	0x8c,	0x8d,	0x8e,	0x8f,
	0x90,	0x91,	0x92,	0x93,	0x94,	0x95,	0x96,	0x97,
     	0x98,	0x99,	0x9a,	0x9b,	0x9c,	0x9d,	0x9e,	0x9f,
	0xa0,	0xa1,	0xa2,	0xa3,	0xa4,	0xa5,	0xa6,	0xa7,
     	0xa8,	0xa9,	0xaa,	0xab,	0xac,	0xad,	0xae,	0xaf,
	0xb0,	0xb1,	0xb2,	0xb3,	0xb4,	0xb5,	0xb6,	0xb7,
     	0xb8,	0xb9,	0xba,	0xbb,	0xbc,	0xbd,	0xbe,	0xbf,
	0xc0,	0xc1,	0xc2,	0xc3,	0xc4,	0xc5,	0xc6,	0xc7,
     	0xc8,	0xc9,	0xca,	0xcb,	0xcc,	0xcd,	0xce,	0xcf,
	0xd0,	0xd1,	0xd2,	0xd3,	0xd4,	0xd5,	0xd6,	0xd7,
     	0xd8,	0xd9,	0xda,	0xdb,	0xdc,	0xdd,	0xde,	0xdf,
	0xe0,	0xe1,	0xe2,	0xe3,	0xe4,	0xe5,	0xe6,	0xe7,
     	0xe8,	0xe9,	0xea,	0xeb,	0xec,	0xed,	0xee,	0xef,
	0xf0,	0xf1,	0xf2,	0xf3,	0xf4,	0xf5,	0xf6,	0xf7,
     	0xf8,	0xf9,	0xfa,	0xfb,	0xfc,	0xfd,	0xfe,	0xff,
    },
};

#undef _CurrentRuneLocale
const _RuneLocale *_CurrentRuneLocale = &_DefaultRuneLocale;

_RuneLocale *
__runes_for_locale(locale_t locale, int *mb_sb_limit)
{
	FIX_LOCALE(locale);
	struct xlocale_ctype *c = XLOCALE_CTYPE(locale);
	*mb_sb_limit = c->__mb_sb_limit;
	return c->runes;
}

/* ----- hbsd/src/lib/libc/locale/utf8.c ----- */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2011 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 2002-2004 Tim J. Robbins
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


extern int __mb_sb_limit;

static size_t	_UTF8_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static int	_UTF8_mbsinit(const mbstate_t *);
static size_t	_UTF8_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
static size_t	_UTF8_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
static size_t	_UTF8_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);

typedef struct {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
} _UTF8State;

int
_UTF8_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->__mbrtowc = _UTF8_mbrtowc;
	l->__wcrtomb = _UTF8_wcrtomb;
	l->__mbsinit = _UTF8_mbsinit;
	l->__mbsnrtowcs = _UTF8_mbsnrtowcs;
	l->__wcsnrtombs = _UTF8_wcsnrtombs;
	l->runes = rl;
	l->__mb_cur_max = 4;
	/*
	 * UCS-4 encoding used as the internal representation, so
	 * slots 0x0080-0x00FF are occuped and must be excluded
	 * from the single byte ctype by setting the limit.
	 */
	l->__mb_sb_limit = 128;

	return (0);
}

static int
_UTF8_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _UTF8State *)ps)->want == 0);
}

static size_t
_UTF8_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_UTF8State *us;
	int ch, i, mask, want;
	wchar_t lbound, wch;

	us = (_UTF8State *)ps;

	if (us->want < 0 || us->want > 6) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (us->want == 0) {
		/*
		 * Determine the number of octets that make up this character
		 * from the first octet, and a mask that extracts the
		 * interesting bits of the first octet. We already know
		 * the character is at least two bytes long.
		 *
		 * We also specify a lower bound for the character code to
		 * detect redundant, non-"shortest form" encodings. For
		 * example, the sequence C0 80 is _not_ a legal representation
		 * of the null character. This enforces a 1-to-1 mapping
		 * between character codes and their multibyte representations.
		 */
		ch = (unsigned char)*s;
		if ((ch & 0x80) == 0) {
			/* Fast path for plain ASCII characters. */
			if (pwc != NULL)
				*pwc = ch;
			return (ch != '\0' ? 1 : 0);
		}
		if ((ch & 0xe0) == 0xc0) {
			mask = 0x1f;
			want = 2;
			lbound = 0x80;
		} else if ((ch & 0xf0) == 0xe0) {
			mask = 0x0f;
			want = 3;
			lbound = 0x800;
		} else if ((ch & 0xf8) == 0xf0) {
			mask = 0x07;
			want = 4;
			lbound = 0x10000;
		} else {
			/*
			 * Malformed input; input is not UTF-8.
			 */
			errno = EILSEQ;
			return ((size_t)-1);
		}
	} else {
		want = us->want;
		lbound = us->lbound;
	}

	/*
	 * Decode the octet sequence representing the character in chunks
	 * of 6 bits, most significant first.
	 */
	if (us->want == 0)
		wch = (unsigned char)*s++ & mask;
	else
		wch = us->ch;

	for (i = (us->want == 0) ? 1 : 0; i < MIN(want, n); i++) {
		if ((*s & 0xc0) != 0x80) {
			/*
			 * Malformed input; bad characters in the middle
			 * of a character.
			 */
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wch <<= 6;
		wch |= *s++ & 0x3f;
	}
	if (i < want) {
		/* Incomplete multibyte sequence. */
		us->want = want - i;
		us->lbound = lbound;
		us->ch = wch;
		return ((size_t)-2);
	}
	if (wch < lbound) {
		/*
		 * Malformed input; redundant encoding.
		 */
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if ((wch >= 0xd800 && wch <= 0xdfff) || wch > 0x10ffff) {
		/*
		 * Malformed input; invalid code points.
		 */
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if (pwc != NULL)
		*pwc = wch;
	us->want = 0;
	return (wch == L'\0' ? 0 : want);
}

static size_t
_UTF8_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	const char *s;
	size_t nchr;
	wchar_t wc;
	size_t nb;

	us = (_UTF8State *)ps;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		/*
		 * The fast path in the loop below is not safe if an ASCII
		 * character appears as anything but the first byte of a
		 * multibyte sequence. Check now to avoid doing it in the loop.
		 */
		if (nms > 0 && us->want > 0 && (signed char)*s > 0) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		for (;;) {
			if (nms > 0 && (signed char)*s > 0)
				/*
				 * Fast path for plain ASCII characters
				 * excluding NUL.
				 */
				nb = 1;
			else if ((nb = _UTF8_mbrtowc(&wc, s, nms, ps)) ==
			    (size_t)-1)
				/* Invalid sequence - mbrtowc() sets errno. */
				return ((size_t)-1);
			else if (nb == 0 || nb == (size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
		/*NOTREACHED*/
	}

	/*
	 * The fast path in the loop below is not safe if an ASCII
	 * character appears as anything but the first byte of a
	 * multibyte sequence. Check now to avoid doing it in the loop.
	 */
	if (nms > 0 && len > 0 && us->want > 0 && (signed char)*s > 0) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	while (len-- > 0) {
		if (nms > 0 && (signed char)*s > 0) {
			/*
			 * Fast path for plain ASCII characters
			 * excluding NUL.
			 */
			*dst = (wchar_t)*s;
			nb = 1;
		} else if ((nb = _UTF8_mbrtowc(dst, s, nms, ps)) ==
		    (size_t)-1) {
			*src = s;
			return ((size_t)-1);
		} else if (nb == (size_t)-2) {
			*src = s + nms;
			return (nchr);
		} else if (nb == 0) {
			*src = NULL;
			return (nchr);
		}
		s += nb;
		nms -= nb;
		nchr++;
		dst++;
	}
	*src = s;
	return (nchr);
}

static size_t
_UTF8_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	unsigned char lead;
	int i, len;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);

	/*
	 * Determine the number of octets needed to represent this character.
	 * We always output the shortest sequence possible. Also specify the
	 * first few bits of the first octet, which contains the information
	 * about the sequence length.
	 */
	if ((wc & ~0x7f) == 0) {
		/* Fast path for plain ASCII characters. */
		*s = (char)wc;
		return (1);
	} else if ((wc & ~0x7ff) == 0) {
		lead = 0xc0;
		len = 2;
	} else if ((wc & ~0xffff) == 0) {
		if (wc >= 0xd800 && wc <= 0xdfff) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		lead = 0xe0;
		len = 3;
	} else if (wc >= 0 && wc <= 0x10ffff) {
		lead = 0xf0;
		len = 4;
	} else {
		errno = EILSEQ;
		return ((size_t)-1);
	}

	/*
	 * Output the octets representing the character in chunks
	 * of 6 bits, least significant last. The first octet is
	 * a special case because it contains the sequence length
	 * information.
	 */
	for (i = len - 1; i > 0; i--) {
		s[i] = (wc & 0x3f) | 0x80;
		wc >>= 6;
	}
	*s = (wc & 0xff) | lead;

	return (len);
}

static size_t
_UTF8_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	size_t nbytes;
	size_t nb;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if (0 <= *s && *s < 0x80)
				/* Fast path for plain ASCII characters. */
				nb = 1;
			else if ((nb = _UTF8_wcrtomb(buf, *s, ps)) ==
			    (size_t)-1)
				/* Invalid character - wcrtomb() sets errno. */
				return ((size_t)-1);
			if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (0 <= *s && *s < 0x80) {
			/* Fast path for plain ASCII characters. */
			nb = 1;
			*dst = *s;
		} else if (len > (size_t)MB_CUR_MAX) {
			/* Enough space to translate in-place. */
			if ((nb = _UTF8_wcrtomb(dst, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			/*
			 * May not be enough space; use temp. buffer.
			 */
			if ((nb = _UTF8_wcrtomb(buf, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > (int)len)
				/* MB sequence for character won't fit. */
				break;
			memcpy(dst, buf, nb);
		}
		if (*s == L'\0') {
			*src = NULL;
			return (nbytes + nb - 1);
		}
		s++;
		dst += nb;
		len -= nb;
		nbytes += nb;
	}
	*src = s;
	return (nbytes);
}

/* ----- hbsd/src/lib/libc/locale/euc.c ----- */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2011 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 2002-2004 Tim J. Robbins. All rights reserved.
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
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


extern int __mb_sb_limit;

static size_t	_EUC_mbrtowc_impl(wchar_t * __restrict, const char * __restrict,
    size_t, mbstate_t * __restrict, uint8_t, uint8_t, uint8_t, uint8_t);
static size_t	_EUC_wcrtomb_impl(char * __restrict, wchar_t,
    mbstate_t * __restrict, uint8_t, uint8_t, uint8_t, uint8_t);

static size_t	_EUC_CN_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static size_t	_EUC_JP_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static size_t	_EUC_KR_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static size_t	_EUC_TW_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);

static size_t	_EUC_CN_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
static size_t	_EUC_JP_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
static size_t	_EUC_KR_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
static size_t	_EUC_TW_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);

static size_t	_EUC_CN_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
static size_t	_EUC_JP_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
static size_t	_EUC_KR_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
static size_t	_EUC_TW_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);

static size_t	_EUC_CN_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
static size_t	_EUC_JP_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
static size_t	_EUC_KR_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
static size_t	_EUC_TW_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);

static int	_EUC_mbsinit(const mbstate_t *);

typedef struct {
	wchar_t	ch;
	int	set;
	int	want;
} _EucState;

static int
_EUC_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _EucState *)ps)->want == 0);
}

/*
 * EUC-CN uses CS0, CS1 and CS2 (4 bytes).
 */
int
_EUC_CN_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = _EUC_CN_mbrtowc;
	l->__wcrtomb = _EUC_CN_wcrtomb;
	l->__mbsnrtowcs = _EUC_CN_mbsnrtowcs;
	l->__wcsnrtombs = _EUC_CN_wcsnrtombs;
	l->__mbsinit = _EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 4;
	l->__mb_sb_limit = 128;
	return (0);
}

static size_t
_EUC_CN_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

static size_t
_EUC_CN_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _EUC_CN_mbrtowc));
}

static size_t
_EUC_CN_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
}

static size_t
_EUC_CN_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _EUC_CN_wcrtomb));
}

/*
 * EUC-KR uses only CS0 and CS1.
 */
int
_EUC_KR_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = _EUC_KR_mbrtowc;
	l->__wcrtomb = _EUC_KR_wcrtomb;
	l->__mbsnrtowcs = _EUC_KR_mbsnrtowcs;
	l->__wcsnrtombs = _EUC_KR_wcsnrtombs;
	l->__mbsinit = _EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 128;
	return (0);
}

static size_t
_EUC_KR_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, 0, 0, 0, 0));
}

static size_t
_EUC_KR_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _EUC_KR_mbrtowc));
}

static size_t
_EUC_KR_wcrtomb(char * __restrict s, wchar_t wc,
	mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, 0, 0, 0, 0));
}

static size_t
_EUC_KR_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _EUC_KR_wcrtomb));
}

/*
 * EUC-JP uses CS0, CS1, CS2, and CS3.
 */
int
_EUC_JP_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = _EUC_JP_mbrtowc;
	l->__wcrtomb = _EUC_JP_wcrtomb;
	l->__mbsnrtowcs = _EUC_JP_mbsnrtowcs;
	l->__wcsnrtombs = _EUC_JP_wcsnrtombs;
	l->__mbsinit = _EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 3;
	l->__mb_sb_limit = 128;
	return (0);
}

static size_t
_EUC_JP_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 2, SS3, 3));
}

static size_t
_EUC_JP_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _EUC_JP_mbrtowc));
}

static size_t
_EUC_JP_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 2, SS3, 3));
}

static size_t
_EUC_JP_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _EUC_JP_wcrtomb));
}

/*
 * EUC-TW uses CS0, CS1, and CS2.
 */
int
_EUC_TW_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = _EUC_TW_mbrtowc;
	l->__wcrtomb = _EUC_TW_wcrtomb;
	l->__mbsnrtowcs = _EUC_TW_mbsnrtowcs;
	l->__wcsnrtombs = _EUC_TW_wcsnrtombs;
	l->__mbsinit = _EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 4;
	l->__mb_sb_limit = 128;
	return (0);
}

static size_t
_EUC_TW_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
	size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

static size_t
_EUC_TW_mbsnrtowcs(wchar_t * __restrict dst,
	const char ** __restrict src,
	size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _EUC_TW_mbrtowc));
}

static size_t
_EUC_TW_wcrtomb(char * __restrict s, wchar_t wc,
	mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
}

static size_t
_EUC_TW_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _EUC_TW_wcrtomb));
}

/*
 * Common EUC code.
 */

static size_t
_EUC_mbrtowc_impl(wchar_t * __restrict pwc, const char * __restrict s,
	size_t n, mbstate_t * __restrict ps,
	uint8_t cs2, uint8_t cs2width, uint8_t cs3, uint8_t cs3width)
{
	_EucState *es;
	int i, want;
	wchar_t wc = 0;
	unsigned char ch, chs;

	es = (_EucState *)ps;

	if (es->want < 0 || es->want > MB_CUR_MAX) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (es->want == 0) {
		/* Fast path for plain ASCII (CS0) */
		if (((ch = (unsigned char)*s) & 0x80) == 0) {
			if (pwc != NULL)
				*pwc = ch;
			return (ch != '\0' ? 1 : 0);
		}

		if (ch >= 0xa1) {
			/* CS1 */
			want = 2;
		} else if (ch == cs2) {
			want = cs2width;
		} else if (ch == cs3) {
			want = cs3width;
		} else {
			errno = EILSEQ;
			return ((size_t)-1);
		}


		es->want = want;
		es->ch = 0;
	} else {
		want = es->want;
		wc = es->ch;
	}

	for (i = 0; i < MIN(want, n); i++) {
		wc <<= 8;
		chs = *s;
		wc |= chs;
		s++;
	}
	if (i < want) {
		/* Incomplete multibyte sequence */
		es->want = want - i;
		es->ch = wc;
		errno = EILSEQ;
		return ((size_t)-2);
	}
	if (pwc != NULL)
		*pwc = wc;
	es->want = 0;
	return (wc == L'\0' ? 0 : want);
}

static size_t
_EUC_wcrtomb_impl(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps,
    uint8_t cs2, uint8_t cs2width, uint8_t cs3, uint8_t cs3width)
{
	_EucState *es;
	int i, len;
	wchar_t nm;

	es = (_EucState *)ps;

	if (es->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);

	if ((wc & ~0x7f) == 0) {
		/* Fast path for plain ASCII (CS0) */
		*s = (char)wc;
		return (1);
	}

	/* Determine the "length" */
	if ((unsigned)wc > 0xffffff) {
		len = 4;
	} else if ((unsigned)wc > 0xffff) {
		len = 3;
	} else if ((unsigned)wc > 0xff) {
		len = 2;
	} else {
		len = 1;
	}

	if (len > MB_CUR_MAX) {
		errno = EILSEQ;
		return ((size_t)-1);
	}

	/* This first check excludes CS1, which is implicitly valid. */
	if ((wc < 0xa100) || (wc > 0xffff)) {
		/* Check for valid CS2 or CS3 */
		nm = (wc >> ((len - 1) * 8)) & 0xff;
		if (nm == cs2) {
			if (len != cs2width) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		} else if (nm == cs3) {
			if (len != cs3width) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		} else {
			errno = EILSEQ;
			return ((size_t)-1);
		}
	}

	/* Stash the bytes, least significant last */
	for (i = len - 1; i >= 0; i--) {
		s[i] = (wc & 0xff);
		wc >>= 8;
	}
	return (len);
}

/* ----- hbsd/src/lib/libc/locale/collate.c ----- */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2014 Garrett D'Amore <garrett@damore.org>
 * Copyright 2010 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 1995 Alex Tatmanjants <alex@elvisti.kiev.ua>
 *		at Electronni Visti IA, Kiev, Ukraine.
 *			All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Adapted to xlocale by John Marino <draco@marino.st>
 */





struct xlocale_collate __xlocale_global_collate = {
	{{0}, "C"}, 1, 0, 0, 0
};

struct xlocale_collate __xlocale_C_collate = {
	{{0}, "C"}, 1, 0, 0, 0
};

struct xlocale_collate __xlocale_POSIX_collate = {
	{{0}, "POSIX"}, 1, 0, 0, 0
};

struct xlocale_collate __xlocale_CUTF8_collate = {
	{{0}, "C.UTF-8"}, 1, 0, 0, 0
};

static int
__collate_load_tables_l(const char *encoding, struct xlocale_collate *table);

static void
destruct_collate(void *t)
{
	struct xlocale_collate *table = (struct xlocale_collate *)t;
	if (table->map && (table->maplen > 0)) {
		(void) munmap(table->map, table->maplen);
	}
	free(t);
}

void *
__collate_load(const char *encoding, __unused locale_t unused)
{
	if (strcmp(encoding, "C") == 0)
		return (&__xlocale_C_collate);
	else if (strcmp(encoding, "POSIX") == 0)
		return (&__xlocale_POSIX_collate);
	else if (strcmp(encoding, "C.UTF-8") == 0)
		return (&__xlocale_CUTF8_collate);

	struct xlocale_collate *table = (struct xlocale_collate *)calloc(sizeof(struct xlocale_collate),
	    1);
	if (table == NULL)
		return (NULL);
	table->header.header.destructor = destruct_collate;

	/*
	 * FIXME: Make sure that _LDP_CACHE is never returned.  We
	 * should be doing the caching outside of this section.
	 */
	if (__collate_load_tables_l(encoding, table) != _LDP_LOADED) {
		xlocale_release(table);
		return (NULL);
	}
	return (table);
}

/**
 * Load the collation tables for the specified encoding into the global table.
 */
int
__collate_load_tables(const char *encoding)
{

	return (__collate_load_tables_l(encoding, &__xlocale_global_collate));
}

static int
__collate_load_tables_l(const char *encoding, struct xlocale_collate *table)
{
	int i, chains, z;
	char *buf;
	char *TMP;
	char *map;
	collate_info_t *info;
	struct stat sbuf;
	int fd;

	table->__collate_load_error = 1;

	/* 'encoding' must be already checked. */
	if (strcmp(encoding, "C") == 0 || strcmp(encoding, "POSIX") == 0 ||
	    strncmp(encoding, "C.", 2) == 0) {
		return (_LDP_CACHE);
	}

	if (asprintf(&buf, "%s/%s/LC_COLLATE", _PathLocale, encoding) == -1)
		return (_LDP_ERROR);

	if ((fd = _open(buf, O_RDONLY | O_CLOEXEC)) < 0) {
		free(buf);
		return (_LDP_ERROR);
	}
	free(buf);
	if (_fstat(fd, &sbuf) < 0) {
		(void) _close(fd);
		return (_LDP_ERROR);
	}
	if (sbuf.st_size < (COLLATE_FMT_VERSION_LEN +
			    XLOCALE_DEF_VERSION_LEN +
			    sizeof (*info))) {
		(void) _close(fd);
		errno = EINVAL;
		return (_LDP_ERROR);
	}
	map = (char *)mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	(void) _close(fd);
	if ((TMP = map) == MAP_FAILED) {
		return (_LDP_ERROR);
	}

	if (strncmp(TMP, COLLATE_FMT_VERSION, COLLATE_FMT_VERSION_LEN) != 0) {
		(void) munmap(map, sbuf.st_size);
		errno = EINVAL;
		return (_LDP_ERROR);
	}
	TMP += COLLATE_FMT_VERSION_LEN;
	strlcat(table->header.version, TMP, sizeof (table->header.version));
	TMP += XLOCALE_DEF_VERSION_LEN;

	info = (collate_info_t *)TMP;
	TMP += sizeof (*info);

	if ((info->directive_count < 1) ||
	    (info->directive_count >= COLL_WEIGHTS_MAX) ||
	    ((chains = info->chain_count) < 0)) {
		(void) munmap(map, sbuf.st_size);
		errno = EINVAL;
		return (_LDP_ERROR);
	}

	i = (sizeof (collate_char_t) * (UCHAR_MAX + 1)) +
	    (sizeof (collate_chain_t) * chains) +
	    (sizeof (collate_large_t) * info->large_count);
	for (z = 0; z < info->directive_count; z++) {
		i += sizeof (collate_subst_t) * info->subst_count[z];
	}
	if (i != (sbuf.st_size - (TMP - map))) {
		(void) munmap(map, sbuf.st_size);
		errno = EINVAL;
		return (_LDP_ERROR);
	}

	if (table->map && (table->maplen > 0)) {
		(void) munmap(table->map, table->maplen);
	}
	table->map = map;
	table->maplen = sbuf.st_size;
	table->info = info;
	table->char_pri_table = (collate_char_t *)TMP;
	TMP += sizeof (collate_char_t) * (UCHAR_MAX + 1);

	for (z = 0; z < info->directive_count; z++) {
		if (info->subst_count[z] > 0) {
			table->subst_table[z] = (collate_subst_t *)TMP;
			TMP += info->subst_count[z] * sizeof (collate_subst_t);
		} else {
			table->subst_table[z] = NULL;
		}
	}

	if (chains > 0) {
		table->chain_pri_table = (collate_chain_t *)TMP;
		TMP += chains * sizeof (collate_chain_t);
	} else
		table->chain_pri_table = NULL;
	if (info->large_count > 0)
		table->large_pri_table = (collate_large_t *)TMP;
	else
		table->large_pri_table = NULL;

	table->__collate_load_error = 0;
	return (_LDP_LOADED);
}

static const int32_t *
substsearch(struct xlocale_collate *table, const wchar_t key, int pass)
{
	const collate_subst_t *p;
	int n = table->info->subst_count[pass];

	if (n == 0)
		return (NULL);

	if (pass >= table->info->directive_count)
		return (NULL);

	if (!(key & COLLATE_SUBST_PRIORITY))
		return (NULL);

	p = table->subst_table[pass] + (key & ~COLLATE_SUBST_PRIORITY);
	assert(p->key == key);

	return (p->pri);
}

static collate_chain_t *
chainsearch(struct xlocale_collate *table, const wchar_t *key, int *len)
{
	int low = 0;
	int high = table->info->chain_count - 1;
	int next, compar, l;
	collate_chain_t *p;
	collate_chain_t *tab = table->chain_pri_table;

	if (high < 0)
		return (NULL);

	while (low <= high) {
		next = (low + high) / 2;
		p = tab + next;
		compar = *key - *p->str;
		if (compar == 0) {
			l = wcsnlen(p->str, COLLATE_STR_LEN);
			compar = wcsncmp(key, p->str, l);
			if (compar == 0) {
				*len = l;
				return (p);
			}
		}
		if (compar > 0)
			low = next + 1;
		else
			high = next - 1;
	}
	return (NULL);
}

static collate_large_t *
largesearch(struct xlocale_collate *table, const wchar_t key)
{
	int low = 0;
	int high = table->info->large_count - 1;
	int next, compar;
	collate_large_t *p;
	collate_large_t *tab = table->large_pri_table;

	if (high < 0)
		return (NULL);

	while (low <= high) {
		next = (low + high) / 2;
		p = tab + next;
		compar = key - p->val;
		if (compar == 0)
			return (p);
		if (compar > 0)
			low = next + 1;
		else
			high = next - 1;
	}
	return (NULL);
}

void
_collate_lookup(struct xlocale_collate *table, const wchar_t *t, int *len,
    int *pri, int which, const int **state)
{
	collate_chain_t *p2;
	collate_large_t *match;
	int p, l;
	const int *sptr;

	/*
	 * If this is the "last" pass for the UNDEFINED, then
	 * we just return the priority itself.
	 */
	if (which >= table->info->directive_count) {
		*pri = *t;
		*len = 1;
		*state = NULL;
		return;
	}

	/*
	 * If we have remaining substitution data from a previous
	 * call, consume it first.
	 */
	if ((sptr = *state) != NULL) {
		*pri = *sptr;
		sptr++;
		if ((sptr == *state) || (sptr == NULL))
			*state = NULL;
		else
			*state = sptr;
		*len = 0;
		return;
	}

	/* No active substitutions */
	*len = 1;

	/*
	 * Check for composites such as diphthongs that collate as a
	 * single element (aka chains or collating-elements).
	 */
	if (((p2 = chainsearch(table, t, &l)) != NULL) &&
	    ((p = p2->pri[which]) >= 0)) {

		*len = l;
		*pri = p;

	} else if (*t <= UCHAR_MAX) {

		/*
		 * Character is a small (8-bit) character.
		 * We just look these up directly for speed.
		 */
		*pri = table->char_pri_table[*t].pri[which];

	} else if ((table->info->large_count > 0) &&
	    ((match = largesearch(table, *t)) != NULL)) {

		/*
		 * Character was found in the extended table.
		 */
		*pri = match->pri.pri[which];

	} else {
		/*
		 * Character lacks a specific definition.
		 */
		if (table->info->directive[which] & DIRECTIVE_UNDEFINED) {
			/* Mask off sign bit to prevent ordering confusion. */
			*pri = (*t & COLLATE_MAX_PRIORITY);
		} else {
			*pri = table->info->undef_pri[which];
		}
		/* No substitutions for undefined characters! */
		return;
	}

	/*
	 * Try substituting (expanding) the character.  We are
	 * currently doing this *after* the chain compression.  I
	 * think it should not matter, but this way might be slightly
	 * faster.
	 *
	 * We do this after the priority search, as this will help us
	 * to identify a single key value.  In order for this to work,
	 * its important that the priority assigned to a given element
	 * to be substituted be unique for that level.  The localedef
	 * code ensures this for us.
	 */
	if ((sptr = substsearch(table, *pri, which)) != NULL) {
		if ((*pri = *sptr) > 0) {
			sptr++;
			*state = *sptr ? sptr : NULL;
		}
	}

}

/*
 * This is the meaty part of wcsxfrm & strxfrm.  Note that it does
 * NOT NULL terminate.  That is left to the caller.
 */
size_t
_collate_wxfrm(struct xlocale_collate *table, const wchar_t *src, wchar_t *xf,
    size_t room)
{
	int		pri;
	int		len;
	const wchar_t	*t;
	wchar_t		*tr = NULL;
	int		direc;
	int		pass;
	const int32_t 	*state;
	size_t		want = 0;
	size_t		need = 0;
	int		ndir = table->info->directive_count;

	assert(src);

	for (pass = 0; pass <= ndir; pass++) {

		state = NULL;

		if (pass != 0) {
			/* insert level separator from the previous pass */
			if (room) {
				*xf++ = 1;
				room--;
			}
			want++;
		}

		/* special pass for undefined */
		if (pass == ndir) {
			direc = DIRECTIVE_FORWARD | DIRECTIVE_UNDEFINED;
		} else {
			direc = table->info->directive[pass];
		}

		t = src;

		if (direc & DIRECTIVE_BACKWARD) {
			wchar_t *bp, *fp, c;
			free(tr);
			if ((tr = wcsdup(t)) == NULL) {
				errno = ENOMEM;
				goto fail;
			}
			bp = tr;
			fp = tr + wcslen(tr) - 1;
			while (bp < fp) {
				c = *bp;
				*bp++ = *fp;
				*fp-- = c;
			}
			t = (const wchar_t *)tr;
		}

		if (direc & DIRECTIVE_POSITION) {
			while (*t || state) {
				_collate_lookup(table, t, &len, &pri, pass, &state);
				t += len;
				if (pri <= 0) {
					if (pri < 0) {
						errno = EINVAL;
						goto fail;
					}
					state = NULL;
					pri = COLLATE_MAX_PRIORITY;
				}
				if (room) {
					*xf++ = pri;
					room--;
				}
				want++;
				need = want;
			}
		} else {
			while (*t || state) {
				_collate_lookup(table, t, &len, &pri, pass, &state);
				t += len;
				if (pri <= 0) {
					if (pri < 0) {
						errno = EINVAL;
						goto fail;
					}
					state = NULL;
					continue;
				}
				if (room) {
					*xf++ = pri;
					room--;
				}
				want++;
				need = want;
			}
		}
	}
	free(tr);
	return (need);

fail:
	free(tr);
	return ((size_t)(-1));
}

/*
 * In the non-POSIX case, we transform each character into a string of
 * characters representing the character's priority.  Since char is usually
 * signed, we are limited by 7 bits per byte.  To avoid zero, we need to add
 * XFRM_OFFSET, so we can't use a full 7 bits.  For simplicity, we choose 6
 * bits per byte.
 *
 * It turns out that we sometimes have real priorities that are
 * 31-bits wide.  (But: be careful using priorities where the high
 * order bit is set -- i.e. the priority is negative.  The sort order
 * may be surprising!)
 *
 * TODO: This would be a good area to optimize somewhat.  It turns out
 * that real prioririties *except for the last UNDEFINED pass* are generally
 * very small.  We need the localedef code to precalculate the max
 * priority for us, and ideally also give us a mask, and then we could
 * severely limit what we expand to.
 */
#define	XFRM_BYTES	6
#define	XFRM_OFFSET	('0')	/* make all printable characters */
#define	XFRM_SHIFT	6
#define	XFRM_MASK	((1 << XFRM_SHIFT) - 1)
#define	XFRM_SEP	('.')	/* chosen to be less than XFRM_OFFSET */

static int
xfrm(struct xlocale_collate *table, unsigned char *p, int pri, int pass)
{
	/* we use unsigned to ensure zero fill on right shift */
	uint32_t val = (uint32_t)table->info->pri_count[pass];
	int nc = 0;

	while (val) {
		*p = (pri & XFRM_MASK) + XFRM_OFFSET;
		pri >>= XFRM_SHIFT;
		val >>= XFRM_SHIFT;
		p++;
		nc++;
	}
	return (nc);
}

size_t
_collate_sxfrm(struct xlocale_collate *table, const wchar_t *src, char *xf,
    size_t room)
{
	int		pri;
	int		len;
	const wchar_t	*t;
	wchar_t		*tr = NULL;
	int		direc;
	int		pass;
	const int32_t 	*state;
	size_t		want = 0;
	size_t		need = 0;
	int		b;
	uint8_t		buf[XFRM_BYTES];
	int		ndir = table->info->directive_count;

	assert(src);

	for (pass = 0; pass <= ndir; pass++) {

		state = NULL;

		if (pass != 0) {
			/* insert level separator from the previous pass */
			if (room) {
				*xf++ = XFRM_SEP;
				room--;
			}
			want++;
		}

		/* special pass for undefined */
		if (pass == ndir) {
			direc = DIRECTIVE_FORWARD | DIRECTIVE_UNDEFINED;
		} else {
			direc = table->info->directive[pass];
		}

		t = src;

		if (direc & DIRECTIVE_BACKWARD) {
			wchar_t *bp, *fp, c;
			free(tr);
			if ((tr = wcsdup(t)) == NULL) {
				errno = ENOMEM;
				goto fail;
			}
			bp = tr;
			fp = tr + wcslen(tr) - 1;
			while (bp < fp) {
				c = *bp;
				*bp++ = *fp;
				*fp-- = c;
			}
			t = (const wchar_t *)tr;
		}

		if (direc & DIRECTIVE_POSITION) {
			while (*t || state) {

				_collate_lookup(table, t, &len, &pri, pass, &state);
				t += len;
				if (pri <= 0) {
					if (pri < 0) {
						errno = EINVAL;
						goto fail;
					}
					state = NULL;
					pri = COLLATE_MAX_PRIORITY;
				}

				b = xfrm(table, buf, pri, pass);
				want += b;
				if (room) {
					while (b) {
						b--;
						if (room) {
							*xf++ = buf[b];
							room--;
						}
					}
				}
				need = want;
			}
		} else {
			while (*t || state) {
				_collate_lookup(table, t, &len, &pri, pass, &state);
				t += len;
				if (pri <= 0) {
					if (pri < 0) {
						errno = EINVAL;
						goto fail;
					}
					state = NULL;
					continue;
				}

				b = xfrm(table, buf, pri, pass);
				want += b;
				if (room) {

					while (b) {
						b--;
						if (room) {
							*xf++ = buf[b];
							room--;
						}
					}
				}
				need = want;
			}
		}
	}
	free(tr);
	return (need);

fail:
	free(tr);
	return ((size_t)(-1));
}

/*
 * __collate_equiv_value returns the primary collation value for the given
 * collating symbol specified by str and len.  Zero or negative is returned
 * if the collating symbol was not found.  This function is used by bracket
 * code in the TRE regex library.
 */
int
__collate_equiv_value(locale_t locale, const wchar_t *str, size_t len)
{
	int32_t e;

	if (len < 1 || len >= COLLATE_STR_LEN)
		return (-1);

	FIX_LOCALE(locale);
	struct xlocale_collate *table =
		(struct xlocale_collate*)locale->components[XLC_COLLATE];

	if (table->__collate_load_error)
		return ((len == 1 && *str <= UCHAR_MAX) ? *str : -1);

	if (len == 1) {
		e = -1;
		if (*str <= UCHAR_MAX)
			e = table->char_pri_table[*str].pri[0];
		else if (table->info->large_count > 0) {
			collate_large_t *match_large;
			match_large = largesearch(table, *str);
			if (match_large)
				e = match_large->pri.pri[0];
		}
		if (e == 0)
			return (1);
		return (e > 0 ? e : 0);
	}
	if (table->info->chain_count > 0) {
		wchar_t name[COLLATE_STR_LEN];
		collate_chain_t *match_chain;
		int clen;

		wcsncpy (name, str, len);
		name[len] = 0;
		match_chain = chainsearch(table, name, &clen);
		if (match_chain) {
			e = match_chain->pri[0];
			if (e == 0)
				return (1);
			return (e < 0 ? -e : e);
		}
	}
	return (0);
}

/*
 * __collate_collating_symbol takes the multibyte string specified by
 * src and slen, and using ps, converts that to a wide character.  Then
 * it is checked to verify it is a collating symbol, and then copies
 * it to the wide character string specified by dst and dlen (the
 * results are not null terminated).  The length of the wide characters
 * copied to dst is returned if successful.  Zero is returned if no such
 * collating symbol exists.  (size_t)-1 is returned if there are wide-character
 * conversion errors, if the length of the converted string is greater that
 * COLLATE_STR_LEN or if dlen is too small.  It is up to the calling routine to
 * preserve the mbstate_t structure as needed.
 */
size_t
__collate_collating_symbol(wchar_t *dst, size_t dlen, const char *src,
    size_t slen, mbstate_t *ps)
{
	wchar_t wname[COLLATE_STR_LEN];
	wchar_t w, *wp;
	struct xlocale_collate *table;
	size_t len, l;

	table =
	    (struct xlocale_collate *)__get_locale()->components[XLC_COLLATE];
	/* POSIX locale */
	if (table->__collate_load_error) {
		if (dlen < 1)
			return ((size_t)-1);
		if (slen != 1 || !isascii(*src))
			return (0);
		*dst = *src;
		return (1);
	}
	for (wp = wname, len = 0; slen > 0; len++) {
		l = mbrtowc(&w, src, slen, ps);
		if (l == (size_t)-1 || l == (size_t)-2)
			return ((size_t)-1);
		if (l == 0)
			break;
		if (len >= COLLATE_STR_LEN)
			return ((size_t)-1);
		*wp++ = w;
		src += l;
		slen -= l;
	}
	if (len == 0 || len > dlen)
		return ((size_t)-1);
	if (len == 1) {
		if (*wname <= UCHAR_MAX) {
			if (table->char_pri_table[*wname].pri[0] >= 0) {
				if (dlen > 0)
					*dst = *wname;
				return (1);
			}
			return (0);
		} else if (table->info->large_count > 0) {
			collate_large_t *match;
			match = largesearch(table, *wname);
			if (match && match->pri.pri[0] >= 0) {
				if (dlen > 0)
					*dst = *wname;
				return (1);
			}
		}
		return (0);
	}
	*wp = 0;
	if (table->info->chain_count > 0) {
		collate_chain_t *match;
		int ll;
		match = chainsearch(table, wname, &ll);
		if (match) {
			if (ll < dlen)
				dlen = ll;
			wcsncpy(dst, wname, dlen);
			return (dlen);
		}
	}
	return (0);
}

/*
 * __collate_equiv_class returns the equivalence class number for the symbol
 * specified by src and slen, using ps to convert from multi-byte to wide
 * character.  Zero is returned if the symbol is not in an equivalence
 * class.  -1 is returned if there are wide character conversion errors,
 * if there are any greater-than-8-bit characters or if a multi-byte symbol
 * is greater or equal to COLLATE_STR_LEN in length.  It is up to the calling
 * routine to preserve the mbstate_t structure as needed.
 */
int
__collate_equiv_class(const char *src, size_t slen, mbstate_t *ps)
{
	wchar_t wname[COLLATE_STR_LEN];
	wchar_t w, *wp;
	struct xlocale_collate *table;
	size_t len, l;
	int e;

	table =
	    (struct xlocale_collate *)__get_locale()->components[XLC_COLLATE];
	/* POSIX locale */
	if (table->__collate_load_error)
		return (0);
	for (wp = wname, len = 0; slen > 0; len++) {
		l = mbrtowc(&w, src, slen, ps);
		if (l == (size_t)-1 || l == (size_t)-2)
			return (-1);
		if (l == 0)
			break;
		if (len >= COLLATE_STR_LEN)
			return (-1);
		*wp++ = w;
		src += l;
		slen -= l;
	}
	if (len == 0)
		return (-1);
	if (len == 1) {
		e = -1;
		if (*wname <= UCHAR_MAX)
			e = table->char_pri_table[*wname].pri[0];
		else if (table->info->large_count > 0) {
			collate_large_t *match;
			match = largesearch(table, *wname);
			if (match)
				e = match->pri.pri[0];
		}
		if (e == 0)
			return (IGNORE_EQUIV_CLASS);
		return (e > 0 ? e : 0);
	}
	*wp = 0;
	if (table->info->chain_count > 0) {
		collate_chain_t *match;
		int ll;
		match = chainsearch(table, wname, &ll);
		if (match) {
			e = match->pri[0];
			if (e == 0)
				return (IGNORE_EQUIV_CLASS);
			return (e < 0 ? -e : e);
		}
	}
	return (0);
}


/*
 * __collate_equiv_match tries to match any single or multi-character symbol
 * in equivalence class equiv_class in the multi-byte string specified by src
 * and slen.  If start is non-zero, it is taken to be the first (pre-converted)
 * wide character.  Subsequence wide characters, if needed, will use ps in
 * the conversion.  On a successful match, the length of the matched string
 * is returned (including the start character).  If dst is non-NULL, the
 * matched wide-character string is copied to dst, a wide character array of
 * length dlen (the results are not zero-terminated).  If rlen is non-NULL,
 * the number of character in src actually used is returned.  Zero is
 * returned by __collate_equiv_match if there is no match.  (size_t)-1 is
 * returned on error: if there were conversion errors or if dlen is too small
 * to accept the results.  On no match or error, ps is restored to its incoming
 * state.
 */
size_t
__collate_equiv_match(int equiv_class, wchar_t *dst, size_t dlen, wchar_t start,
    const char *src, size_t slen, mbstate_t *ps, size_t *rlen)
{
	wchar_t w;
	size_t len, l, clen;
	int i;
	wchar_t buf[COLLATE_STR_LEN], *wp;
	mbstate_t save;
	const char *s = src;
	struct xlocale_collate *table;
	size_t sl = slen;
	collate_chain_t *ch = NULL;

	table =
	    (struct xlocale_collate *)__get_locale()->components[XLC_COLLATE];
	/* POSIX locale */
	if (table->__collate_load_error)
		return ((size_t)-1);
	if (equiv_class == IGNORE_EQUIV_CLASS)
		equiv_class = 0;
	if (ps)
		save = *ps;
	wp = buf;
	len = clen = 0;
	if (start) {
		*wp++ = start;
		len = 1;
	}
	/* convert up to the max chain length */
	while (sl > 0 && len < table->info->chain_max_len) {
		l = mbrtowc(&w, s, sl, ps);
		if (l == (size_t)-1 || l == (size_t)-2 || l == 0)
			break;
		*wp++ = w;
		s += l;
		clen += l;
		sl -= l;
		len++;
	}
	*wp = 0;
	if (len > 1 && (ch = chainsearch(table, buf, &i)) != NULL) {
		int e = ch->pri[0];
		if (e < 0)
			e = -e;
		if (e == equiv_class)
			goto found;
	}
	/* try single character */
	i = 1;
	if (*buf <= UCHAR_MAX) {
		if (equiv_class == table->char_pri_table[*buf].pri[0])
			goto found;
	} else if (table->info->large_count > 0) {
		collate_large_t *match;
		match = largesearch(table, *buf);
		if (match && equiv_class == match->pri.pri[0])
			goto found;
	}
	/* no match */
	if (ps)
		*ps = save;
	return (0);
found:
	/*
	 * If we converted more than we used, restore to initial
	 * and reconvert up to what did match.
	 */
	if (i < len) {
		len = i;
		if (ps)
			*ps = save;
		if (start)
			i--;
		clen = 0;
		while (i-- > 0) {
			l = mbrtowc(&w, src, slen, ps);
			src += l;
			clen += l;
			slen -= l;
		}
	}
	if (dst) {
		if (dlen < len) {
			if (ps)
				*ps = save;
			return ((size_t)-1);
		}
		for (wp = buf; len > 0; len--)
		    *dst++ = *wp++;
	}
	if (rlen)
		*rlen = clen;
	return (len);
}

int UTF8_mbsinit(const mbstate_t *ps) { return _UTF8_mbsinit(ps); }
std::size_t UTF8_mbrtowc(wchar_t *pwc, const char *s, std::size_t n, mbstate_t *ps)
    { return _UTF8_mbrtowc(pwc, s, n, ps); }
std::size_t UTF8_mbsnrtowcs(wchar_t *dst, const char **src, std::size_t nms,
    std::size_t len, mbstate_t *ps)
    { return _UTF8_mbsnrtowcs(dst, src, nms, len, ps); }
std::size_t UTF8_wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
    { return _UTF8_wcrtomb(s, wc, ps); }
std::size_t UTF8_wcsnrtombs(char *dst, const wchar_t **src, std::size_t nwc,
    std::size_t len, mbstate_t *ps)
    { return _UTF8_wcsnrtombs(dst, src, nwc, len, ps); }

int EUC_mbsinit(const mbstate_t *ps) { return _EUC_mbsinit(ps); }
std::size_t EUC_mbrtowc_impl(wchar_t *pwc, const char *s, std::size_t n,
    mbstate_t *ps, uint8_t cs2, uint8_t cs2w, uint8_t cs3, uint8_t cs3w)
    { return _EUC_mbrtowc_impl(pwc, s, n, ps, cs2, cs2w, cs3, cs3w); }
std::size_t EUC_wcrtomb_impl(char *s, wchar_t wc, mbstate_t *ps,
    uint8_t cs2, uint8_t cs2w, uint8_t cs3, uint8_t cs3w)
    { return _EUC_wcrtomb_impl(s, wc, ps, cs2, cs2w, cs3, cs3w); }

std::size_t EUC_CN_mbrtowc(wchar_t *pwc, const char *s, std::size_t n, mbstate_t *ps)
    { return _EUC_CN_mbrtowc(pwc, s, n, ps); }
std::size_t EUC_CN_wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
    { return _EUC_CN_wcrtomb(s, wc, ps); }
std::size_t EUC_CN_mbsnrtowcs(wchar_t *dst, const char **src, std::size_t nms,
    std::size_t len, mbstate_t *ps)
    { return _EUC_CN_mbsnrtowcs(dst, src, nms, len, ps); }
std::size_t EUC_CN_wcsnrtombs(char *dst, const wchar_t **src, std::size_t nwc,
    std::size_t len, mbstate_t *ps)
    { return _EUC_CN_wcsnrtombs(dst, src, nwc, len, ps); }

std::size_t EUC_KR_mbrtowc(wchar_t *pwc, const char *s, std::size_t n, mbstate_t *ps)
    { return _EUC_KR_mbrtowc(pwc, s, n, ps); }
std::size_t EUC_KR_wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
    { return _EUC_KR_wcrtomb(s, wc, ps); }
std::size_t EUC_KR_mbsnrtowcs(wchar_t *dst, const char **src, std::size_t nms,
    std::size_t len, mbstate_t *ps)
    { return _EUC_KR_mbsnrtowcs(dst, src, nms, len, ps); }
std::size_t EUC_KR_wcsnrtombs(char *dst, const wchar_t **src, std::size_t nwc,
    std::size_t len, mbstate_t *ps)
    { return _EUC_KR_wcsnrtombs(dst, src, nwc, len, ps); }

std::size_t EUC_JP_mbrtowc(wchar_t *pwc, const char *s, std::size_t n, mbstate_t *ps)
    { return _EUC_JP_mbrtowc(pwc, s, n, ps); }
std::size_t EUC_JP_wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
    { return _EUC_JP_wcrtomb(s, wc, ps); }
std::size_t EUC_JP_mbsnrtowcs(wchar_t *dst, const char **src, std::size_t nms,
    std::size_t len, mbstate_t *ps)
    { return _EUC_JP_mbsnrtowcs(dst, src, nms, len, ps); }
std::size_t EUC_JP_wcsnrtombs(char *dst, const wchar_t **src, std::size_t nwc,
    std::size_t len, mbstate_t *ps)
    { return _EUC_JP_wcsnrtombs(dst, src, nwc, len, ps); }

std::size_t EUC_TW_mbrtowc(wchar_t *pwc, const char *s, std::size_t n, mbstate_t *ps)
    { return _EUC_TW_mbrtowc(pwc, s, n, ps); }
std::size_t EUC_TW_wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
    { return _EUC_TW_wcrtomb(s, wc, ps); }
std::size_t EUC_TW_mbsnrtowcs(wchar_t *dst, const char **src, std::size_t nms,
    std::size_t len, mbstate_t *ps)
    { return _EUC_TW_mbsnrtowcs(dst, src, nms, len, ps); }
std::size_t EUC_TW_wcsnrtombs(char *dst, const wchar_t **src, std::size_t nwc,
    std::size_t len, mbstate_t *ps)
    { return _EUC_TW_wcsnrtombs(dst, src, nwc, len, ps); }

const int32_t *collate_substsearch(xlocale_collate *t, const wchar_t key, int pass)
    { return substsearch(t, key, pass); }
collate_chain_t *collate_chainsearch(xlocale_collate *t, const wchar_t *key, int *len)
    { return chainsearch(t, key, len); }
collate_large_t *collate_largesearch(xlocale_collate *t, const wchar_t key)
    { return largesearch(t, key); }

const _RuneLocale *default_rune_locale() { return (&_DefaultRuneLocale); }
xlocale_collate *collate_c_table() { return (&__xlocale_C_collate); }

} // namespace pbsd::lib_libc_locale::b0156
