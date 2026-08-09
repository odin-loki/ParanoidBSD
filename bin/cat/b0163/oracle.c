/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kevin Fall.
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

/*
 * oracle.c for batch b0163 -- the specification.
 *
 * Source: hbsd/src/bin/cat/cat.c
 *
 * Every ported function of cat.c appears below with a "ref_" prefix on its
 * name.  The function bodies are byte-for-byte the originals; nothing inside
 * a body has been touched.  What differs from cat.c:
 *
 *   - the #include list drops the FreeBSD-only headers (sys/capsicum.h,
 *     sys/param.h, capsicum_helpers.h, libcasper.h, casper/*) and adds the
 *     headers those pulled in indirectly;
 *   - the compatibility defines below supply the macros cat.c inherited from
 *     FreeBSD headers (MAXPHYS, MIN, MAX, iswascii, __dead2) and the
 *     fp->_mbstate member name, which FreeBSD stdio exposes and glibc does
 *     not;
 *   - the file-scope statics of cat.c are renamed with the ref_ prefix (via
 *     object-like macros, so the bodies still read "bflag", "rval", ...) and
 *     given external linkage so the harness can drive them;
 *   - function declarations lose "static" for the same reason.
 *
 * Functions of cat.c that cannot be built off FreeBSD are absent; see
 * skipped.txt.
 */

#define _GNU_SOURCE 1

#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

/*
 * Compatibility shims for the FreeBSD facilities cat.c uses.
 */

/* <sys/param.h>: FreeBSD 13+ userland value, and the min/max helpers. */
#ifndef MAXPHYS
#define	MAXPHYS (1024 * 1024)
#endif
#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

/* <sys/cdefs.h> */
#ifndef __dead2
#define	__dead2 __attribute__((__noreturn__))
#endif

/* <wctype.h>: FreeBSD spells this exactly like so. */
#ifndef iswascii
#define	iswascii(wc) (((wc) & ~0x7F) == 0)
#endif

/*
 * FreeBSD stdio keeps the per-stream conversion state in fp->_mbstate.
 * glibc's struct _IO_FILE has no such member, so point the name at the
 * stream's 20 bytes of unused trailing padding; that keeps the body of
 * cook_cat() unmodified and keeps the write in bounds.
 */
#define	_mbstate _unused2

/* cat.c's file-scope statics, exported for the differential harness. */
int ref_bflag, ref_eflag, ref_lflag, ref_nflag, ref_sflag, ref_tflag, ref_vflag;
int ref_rval;
const char *ref_filename;

#define	bflag ref_bflag
#define	eflag ref_eflag
#define	lflag ref_lflag
#define	nflag ref_nflag
#define	sflag ref_sflag
#define	tflag ref_tflag
#define	vflag ref_vflag
#define	rval ref_rval
#define	filename ref_filename

void ref_usage(void) __dead2;
void ref_cook_cat(FILE *);
ssize_t ref_in_kernel_copy(int);
void ref_raw_cat(int);

/*
 * Memory strategy threshold, in pages: if physmem is larger than this,
 * use a large buffer.
 */
#define	PHYSPAGES_THRESHOLD (32 * 1024)

/* Maximum buffer size in bytes - do not allow it to grow larger than this. */
#define	BUFSIZE_MAX (2 * 1024 * 1024)

/*
 * Small (default) buffer size in bytes. It's inefficient for this to be
 * smaller than MAXPHYS.
 */
#define	BUFSIZE_SMALL (MAXPHYS)

#define SUPPORTED_FLAGS "belnstuv"

void
ref_usage(void)
{

	fprintf(stderr, "usage: cat [-" SUPPORTED_FLAGS "] [file ...]\n");
	exit(1);
	/* NOTREACHED */
}

void
ref_cook_cat(FILE *fp)
{
	int ch, gobble, line, prev;
	wint_t wch;

	/* Reset EOF condition on stdin. */
	if (fp == stdin && feof(stdin))
		clearerr(stdin);

	line = gobble = 0;
	for (prev = '\n'; (ch = getc(fp)) != EOF; prev = ch) {
		if (prev == '\n') {
			if (sflag) {
				if (ch == '\n') {
					if (gobble)
						continue;
					gobble = 1;
				} else
					gobble = 0;
			}
			if (nflag) {
				if (!bflag || ch != '\n') {
					(void)fprintf(stdout, "%6d\t", ++line);
					if (ferror(stdout))
						break;
				} else if (eflag) {
					(void)fprintf(stdout, "%6s\t", "");
					if (ferror(stdout))
						break;
				}
			}
		}
		if (ch == '\n') {
			if (eflag && putchar('$') == EOF)
				break;
		} else if (ch == '\t') {
			if (tflag) {
				if (putchar('^') == EOF || putchar('I') == EOF)
					break;
				continue;
			}
		} else if (vflag) {
			(void)ungetc(ch, fp);
			/*
			 * Our getwc(3) doesn't change file position
			 * on error.
			 */
			if ((wch = getwc(fp)) == WEOF) {
				if (ferror(fp) && errno == EILSEQ) {
					clearerr(fp);
					/* Resync attempt. */
					memset(&fp->_mbstate, 0, sizeof(mbstate_t));
					if ((ch = getc(fp)) == EOF)
						break;
					wch = ch;
					goto ilseq;
				} else
					break;
			}
			if (!iswascii(wch) && !iswprint(wch)) {
ilseq:
				if (putchar('M') == EOF || putchar('-') == EOF)
					break;
				wch = toascii(wch);
			}
			if (iswcntrl(wch)) {
				ch = toascii(wch);
				ch = (ch == '\177') ? '?' : (ch | 0100);
				if (putchar('^') == EOF || putchar(ch) == EOF)
					break;
				continue;
			}
			if (putwchar(wch) == WEOF)
				break;
			ch = -1;
			continue;
		}
		if (putchar(ch) == EOF)
			break;
	}
	if (ferror(fp)) {
		warn("%s", filename);
		rval = 1;
		clearerr(fp);
	}
	if (ferror(stdout))
		err(1, "stdout");
}

ssize_t
ref_in_kernel_copy(int rfd)
{
	int wfd;
	ssize_t ret;

	wfd = fileno(stdout);
	ret = 1;

	while (ret > 0)
		ret = copy_file_range(rfd, NULL, wfd, NULL, SSIZE_MAX, 0);

	return (ret);
}

void
ref_raw_cat(int rfd)
{
	long pagesize;
	int off, wfd;
	ssize_t nr, nw;
	static size_t bsize;
	static char *buf = NULL;
	struct stat sbuf;

	wfd = fileno(stdout);
	if (buf == NULL) {
		if (fstat(wfd, &sbuf))
			err(1, "stdout");
		if (S_ISREG(sbuf.st_mode)) {
			/* If there's plenty of RAM, use a large copy buffer */
			if (sysconf(_SC_PHYS_PAGES) > PHYSPAGES_THRESHOLD)
				bsize = MIN(BUFSIZE_MAX, MAXPHYS * 8);
			else
				bsize = BUFSIZE_SMALL;
		} else {
			bsize = sbuf.st_blksize;
			pagesize = sysconf(_SC_PAGESIZE);
			if (pagesize > 0)
				bsize = MAX(bsize, (size_t)pagesize);
		}
		if ((buf = malloc(bsize)) == NULL)
			err(1, "malloc() failure of IO buffer");
	}
	while ((nr = read(rfd, buf, bsize)) > 0)
		for (off = 0; nr; nr -= nw, off += nw)
			if ((nw = write(wfd, buf + off, (size_t)nr)) < 0)
				err(1, "stdout");
	if (nr < 0) {
		warn("%s", filename);
		rval = 1;
	}
}
