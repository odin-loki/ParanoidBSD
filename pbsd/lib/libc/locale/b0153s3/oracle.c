/*
 * Reference oracle for batch b0153s3.
 *
 * Original HardenedBSD source concatenated, every function renamed with a
 * ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Source:
 *   hbsd/src/lib/libc/locale/ldpart.c
 */

#ifndef SIZE_T_MAX
#define SIZE_T_MAX	((size_t)-1)
#endif

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#ifndef PATH_MAX
#define PATH_MAX	4096
#endif

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC	0
#endif

#ifndef EFTYPE
#define EFTYPE		79
#endif

#define _LDP_LOADED	0
#define _LDP_ERROR	(-1)
#define _LDP_CACHE	1

#define FAKE_FD		100

typedef struct {
	int			open_fail;
	int			open_errno;
	int			fstat_fail;
	int			fstat_errno;
	off_t			file_size;
	const char		*file_content;
	int			read_fail;
	int			read_errno;
	int			malloc_fail;
	int			close_count;
} pbsd_ldpart_hook_t;

pbsd_ldpart_hook_t	pbsd_ldpart_hook;
char			*_PathLocale = "/locale";

void
pbsd_reset_hooks(void)
{
	memset(&pbsd_ldpart_hook, 0, sizeof(pbsd_ldpart_hook));
}

pbsd_ldpart_hook_t *
pbsd_get_ldpart_hook(void)
{
	return (&pbsd_ldpart_hook);
}

static void __attribute__((constructor))
ref_oracle_init(void)
{
	pbsd_reset_hooks();
}

void *__real_malloc(size_t);

void *
__wrap_malloc(size_t n)
{
	if (pbsd_ldpart_hook.malloc_fail != 0)
		return (NULL);
	return (__real_malloc(n));
}

int
_open(const char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	if (pbsd_ldpart_hook.open_fail != 0) {
		errno = pbsd_ldpart_hook.open_errno;
		return (-1);
	}
	return (FAKE_FD);
}

int
_fstat(int fd, struct stat *st)
{
	(void)fd;
	if (pbsd_ldpart_hook.fstat_fail != 0) {
		errno = pbsd_ldpart_hook.fstat_errno;
		return (-1);
	}
	memset(st, 0, sizeof(*st));
	st->st_size = pbsd_ldpart_hook.file_size;
	return (0);
}

ssize_t
_read(int fd, void *buf, size_t nbytes)
{
	size_t n;

	(void)fd;
	if (pbsd_ldpart_hook.read_fail != 0) {
		errno = pbsd_ldpart_hook.read_errno;
		return (-1);
	}
	n = (size_t)pbsd_ldpart_hook.file_size;
	if (nbytes < n)
		n = nbytes;
	if (pbsd_ldpart_hook.file_content != NULL && n > 0)
		memcpy(buf, pbsd_ldpart_hook.file_content, n);
	return ((ssize_t)n);
}

int
_close(int fd)
{
	(void)fd;
	pbsd_ldpart_hook.close_count++;
	return (0);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/locale/ldpart.c					*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2000, 2001 Alexey Zelkin <phantom@FreeBSD.org>
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

int ref_split_lines(char *, const char *);

int
ref___part_load_locale(const char *name,
		int *using_locale,
		char **locale_buf,
		const char *category_filename,
		int locale_buf_size_max,
		int locale_buf_size_min,
		const char **dst_localebuf)
{
	int		saverr, fd, i, num_lines;
	char		*lbuf, *p;
	const char	*plim;
	char		filename[PATH_MAX];
	struct stat	st;
	size_t		namesize, bufsize;

	/* 'name' must be already checked. */
	if (strcmp(name, "C") == 0 || strcmp(name, "POSIX") == 0 ||
	    strncmp(name, "C.", 2) == 0) {
		*using_locale = 0;
		return (_LDP_CACHE);
	}

	/*
	 * If the locale name is the same as our cache, use the cache.
	 */
	if (*locale_buf != NULL && strcmp(name, *locale_buf) == 0) {
		*using_locale = 1;
		return (_LDP_CACHE);
	}

	/*
	 * Slurp the locale file into the cache.
	 */
	namesize = strlen(name) + 1;

	/* 'PathLocale' must be already set & checked. */

	/* Range checking not needed, 'name' size is limited */
	strcpy(filename, _PathLocale);
	strcat(filename, "/");
	strcat(filename, name);
	strcat(filename, "/");
	strcat(filename, category_filename);
	if ((fd = _open(filename, O_RDONLY | O_CLOEXEC)) < 0)
		return (_LDP_ERROR);
	if (_fstat(fd, &st) != 0)
		goto bad_locale;
	if (st.st_size <= 0) {
		errno = EFTYPE;
		goto bad_locale;
	}
	bufsize = namesize + st.st_size;
	if ((lbuf = malloc(bufsize)) == NULL) {
		errno = ENOMEM;
		goto bad_locale;
	}
	(void)strcpy(lbuf, name);
	p = lbuf + namesize;
	plim = p + st.st_size;
	if (_read(fd, p, (size_t) st.st_size) != st.st_size)
		goto bad_lbuf;
	/*
	 * Parse the locale file into localebuf.
	 */
	if (plim[-1] != '\n') {
		errno = EFTYPE;
		goto bad_lbuf;
	}
	num_lines = ref_split_lines(p, plim);
	if (num_lines >= locale_buf_size_max)
		num_lines = locale_buf_size_max;
	else if (num_lines >= locale_buf_size_min)
		num_lines = locale_buf_size_min;
	else {
		errno = EFTYPE;
		goto bad_lbuf;
	}
	(void)_close(fd);
	/*
	 * Record the successful parse in the cache.
	 */
	if (*locale_buf != NULL)
		free(*locale_buf);
	*locale_buf = lbuf;
	for (p = *locale_buf, i = 0; i < num_lines; i++)
		dst_localebuf[i] = (p += strlen(p) + 1);
	for (i = num_lines; i < locale_buf_size_max; i++)
		dst_localebuf[i] = NULL;
	*using_locale = 1;

	return (_LDP_LOADED);

bad_lbuf:
	saverr = errno;
	free(lbuf);
	errno = saverr;
bad_locale:
	saverr = errno;
	(void)_close(fd);
	errno = saverr;

	return (_LDP_ERROR);
}

int
ref_split_lines(char *p, const char *plim)
{
	int i;

	i = 0;
	while (p < plim) {
		if (*p == '\n') {
			*p = '\0';
			i++;
		}
		p++;
	}
	return (i);
}
