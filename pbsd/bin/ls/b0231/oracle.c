/*
 * oracle.c -- reference implementation for batch b0231.
 *
 * Selected functions from the ls(1) sources, renamed with a ref_ prefix.
 * Function bodies are otherwise UNMODIFIED.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <fts.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#ifndef st_birthtim
#define st_birthtim st_mtim
#endif

#ifndef S_IFWHT
#define S_IFWHT 0
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN (-0x7fffffff - 1)
#endif

#define NO_PRINT 1

int f_samesort;
int f_octal;
int f_octal_escape;
int f_nonprint;
int f_slash;
int f_type;
int f_listdir;

enum {
	GRP_NONE = 0,
	GRP_DIR_FIRST = -1,
	GRP_DIR_LAST = 1
};

int f_groupdir = GRP_NONE;

int (*ref_sortfcn)(const FTSENT *, const FTSENT *);

int
ref_namecmp(const FTSENT *a, const FTSENT *b)
{

	return (strcoll(a->fts_name, b->fts_name));
}

int
ref_revnamecmp(const FTSENT *a, const FTSENT *b)
{

	return (strcoll(b->fts_name, a->fts_name));
}

int
ref_verscmp(const FTSENT *a, const FTSENT *b)
{

	return (strverscmp(a->fts_name, b->fts_name));
}

int
ref_revverscmp(const FTSENT *a, const FTSENT *b)
{

	return (strverscmp(b->fts_name, a->fts_name));
}

int
ref_modcmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_mtim.tv_sec >
	    a->fts_statp->st_mtim.tv_sec)
		return (1);
	if (b->fts_statp->st_mtim.tv_sec <
	    a->fts_statp->st_mtim.tv_sec)
		return (-1);
	if (b->fts_statp->st_mtim.tv_nsec >
	    a->fts_statp->st_mtim.tv_nsec)
		return (1);
	if (b->fts_statp->st_mtim.tv_nsec <
	    a->fts_statp->st_mtim.tv_nsec)
		return (-1);
	if (f_samesort)
		return (strcoll(b->fts_name, a->fts_name));
	else
		return (strcoll(a->fts_name, b->fts_name));
}

int
ref_revmodcmp(const FTSENT *a, const FTSENT *b)
{

	return (ref_modcmp(b, a));
}

int
ref_acccmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_atim.tv_sec >
	    a->fts_statp->st_atim.tv_sec)
		return (1);
	if (b->fts_statp->st_atim.tv_sec <
	    a->fts_statp->st_atim.tv_sec)
		return (-1);
	if (b->fts_statp->st_atim.tv_nsec >
	    a->fts_statp->st_atim.tv_nsec)
		return (1);
	if (b->fts_statp->st_atim.tv_nsec <
	    a->fts_statp->st_atim.tv_nsec)
		return (-1);
	if (f_samesort)
		return (strcoll(b->fts_name, a->fts_name));
	else
		return (strcoll(a->fts_name, b->fts_name));
}

int
ref_revacccmp(const FTSENT *a, const FTSENT *b)
{

	return (ref_acccmp(b, a));
}

int
ref_birthcmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_birthtim.tv_sec >
	    a->fts_statp->st_birthtim.tv_sec)
		return (1);
	if (b->fts_statp->st_birthtim.tv_sec <
	    a->fts_statp->st_birthtim.tv_sec)
		return (-1);
	if (b->fts_statp->st_birthtim.tv_nsec >
	    a->fts_statp->st_birthtim.tv_nsec)
		return (1);
	if (b->fts_statp->st_birthtim.tv_nsec <
	    a->fts_statp->st_birthtim.tv_nsec)
		return (-1);
	if (f_samesort)
		return (strcoll(b->fts_name, a->fts_name));
	else
		return (strcoll(a->fts_name, b->fts_name));
}

int
ref_revbirthcmp(const FTSENT *a, const FTSENT *b)
{

	return (ref_birthcmp(b, a));
}

int
ref_statcmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_ctim.tv_sec >
	    a->fts_statp->st_ctim.tv_sec)
		return (1);
	if (b->fts_statp->st_ctim.tv_sec <
	    a->fts_statp->st_ctim.tv_sec)
		return (-1);
	if (b->fts_statp->st_ctim.tv_nsec >
	    a->fts_statp->st_ctim.tv_nsec)
		return (1);
	if (b->fts_statp->st_ctim.tv_nsec <
	    a->fts_statp->st_ctim.tv_nsec)
		return (-1);
	if (f_samesort)
		return (strcoll(b->fts_name, a->fts_name));
	else
		return (strcoll(a->fts_name, b->fts_name));
}

int
ref_revstatcmp(const FTSENT *a, const FTSENT *b)
{

	return (ref_statcmp(b, a));
}

int
ref_sizecmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_size > a->fts_statp->st_size)
		return (1);
	if (b->fts_statp->st_size < a->fts_statp->st_size)
		return (-1);
	return (strcoll(a->fts_name, b->fts_name));
}

int
ref_revsizecmp(const FTSENT *a, const FTSENT *b)
{

	return (ref_sizecmp(b, a));
}

int
ref_prn_normal(const char *s)
{
	mbstate_t mbs;
	wchar_t wc;
	int i, n;
	size_t clen;

	memset(&mbs, 0, sizeof(mbs));
	n = 0;
	while ((clen = mbrtowc(&wc, s, MB_LEN_MAX, &mbs)) != 0) {
		if (clen == (size_t)-2) {
			n += printf("%s", s);
			break;
		}
		if (clen == (size_t)-1) {
			memset(&mbs, 0, sizeof(mbs));
			putchar((unsigned char)*s);
			s++;
			n++;
			continue;
		}
		for (i = 0; i < (int)clen; i++)
			putchar((unsigned char)s[i]);
		s += clen;
		if (iswprint(wc))
			n += wcwidth(wc);
	}
	return (n);
}

int
ref_prn_printable(const char *s)
{
	mbstate_t mbs;
	wchar_t wc;
	int i, n;
	size_t clen;

	memset(&mbs, 0, sizeof(mbs));
	n = 0;
	while ((clen = mbrtowc(&wc, s, MB_LEN_MAX, &mbs)) != 0) {
		if (clen == (size_t)-1) {
			putchar('?');
			s++;
			n++;
			memset(&mbs, 0, sizeof(mbs));
			continue;
		}
		if (clen == (size_t)-2) {
			putchar('?');
			n++;
			break;
		}
		if (!iswprint(wc)) {
			putchar('?');
			s += clen;
			n++;
			continue;
		}
		for (i = 0; i < (int)clen; i++)
			putchar((unsigned char)s[i]);
		s += clen;
		n += wcwidth(wc);
	}
	return (n);
}

size_t
ref_len_octal(const char *s, int len)
{
	mbstate_t mbs;
	wchar_t wc;
	size_t clen, r;

	memset(&mbs, 0, sizeof(mbs));
	r = 0;
	while (len != 0 && (clen = mbrtowc(&wc, s, len, &mbs)) != 0) {
		if (clen == (size_t)-1) {
			r += 4;
			s++;
			len--;
			memset(&mbs, 0, sizeof(mbs));
			continue;
		}
		if (clen == (size_t)-2) {
			r += 4 * len;
			break;
		}
		if (iswprint(wc))
			r++;
		else
			r += 4 * clen;
		s += clen;
	}
	return (r);
}

int
ref_prn_octal(const char *s)
{
	static const char esc[] = "\\\\\"\"\aa\bb\ff\nn\rr\tt\vv";
	const char *p;
	mbstate_t mbs;
	wchar_t wc;
	size_t clen;
	unsigned char ch;
	int goodchar, i, len, prtlen;

	memset(&mbs, 0, sizeof(mbs));
	len = 0;
	while ((clen = mbrtowc(&wc, s, MB_LEN_MAX, &mbs)) != 0) {
		goodchar = clen != (size_t)-1 && clen != (size_t)-2;
		if (goodchar && iswprint(wc) && wc != L'\"' && wc != L'\\') {
			for (i = 0; i < (int)clen; i++)
				putchar((unsigned char)s[i]);
			len += wcwidth(wc);
		} else if (goodchar && f_octal_escape &&
#if WCHAR_MIN < 0
                    wc >= 0 &&
#endif
		    wc <= (wchar_t)UCHAR_MAX &&
		    (p = strchr(esc, (char)wc)) != NULL) {
			putchar('\\');
			putchar(p[1]);
			len += 2;
		} else {
			if (goodchar)
				prtlen = clen;
			else if (clen == (size_t)-1)
				prtlen = 1;
			else
				prtlen = strlen(s);
			for (i = 0; i < prtlen; i++) {
				ch = (unsigned char)s[i];
				putchar('\\');
				putchar('0' + (ch >> 6));
				putchar('0' + ((ch >> 3) & 7));
				putchar('0' + (ch & 7));
				len += 4;
			}
		}
		if (clen == (size_t)-2)
			break;
		if (clen == (size_t)-1) {
			memset(&mbs, 0, sizeof(mbs));
			s++;
		} else
			s += clen;
	}
	return (len);
}

int
ref_printname(const char *name)
{
	if (f_octal || f_octal_escape)
		return ref_prn_octal(name);
	else if (f_nonprint)
		return ref_prn_printable(name);
	else
		return ref_prn_normal(name);
}

int
ref_printtype(u_int mode)
{

	if (f_slash) {
		if ((mode & S_IFMT) == S_IFDIR) {
			(void)putchar('/');
			return (1);
		}
		return (0);
	}

	switch (mode & S_IFMT) {
	case S_IFDIR:
		(void)putchar('/');
		return (1);
	case S_IFIFO:
		(void)putchar('|');
		return (1);
	case S_IFLNK:
		(void)putchar('@');
		return (1);
	case S_IFSOCK:
		(void)putchar('=');
		return (1);
	case S_IFWHT:
		(void)putchar('%');
		return (1);
	default:
		break;
	}
	if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
		(void)putchar('*');
		return (1);
	}
	return (0);
}

int
ref_mastercmp(const FTSENT * const *a, const FTSENT * const *b)
{
	int a_info, b_info, dir;

	a_info = (*a)->fts_info;
	if (a_info == FTS_ERR)
		return (0);
	b_info = (*b)->fts_info;
	if (b_info == FTS_ERR)
		return (0);

	if (a_info == FTS_NS || b_info == FTS_NS)
		return (ref_namecmp(*a, *b));

	if (a_info != b_info &&
	    (*a)->fts_level == FTS_ROOTLEVEL && !f_listdir) {
		if (a_info == FTS_D)
			return (1);
		if (b_info == FTS_D)
			return (-1);
	}

	if (f_groupdir != GRP_NONE)
		if ((dir = (a_info == FTS_D) - (b_info == FTS_D)) != 0)
			return (f_groupdir * dir);

	return (ref_sortfcn(*a, *b));
}
