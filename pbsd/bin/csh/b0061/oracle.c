/*
 * PBSD batch b0061 reference oracle.
 *
 * Source: hbsd/src/bin/csh/iconv_stub.c
 *
 * Every function of the original source is reproduced here verbatim, with only
 * the function name prefixed by "ref_".  Function bodies are UNMODIFIED.  The
 * only additions are the type/macro definitions that the original picked up
 * from bin/csh/iconv.h (which is not part of this batch) and a definition of
 * dlfunc(3) for hosts that lack that FreeBSD extension.
 */

/*-
 * Copyright (c) 2006 Hajimu UMEMOTO <ume@FreeBSD.org>
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

#include <dlfcn.h>
#include <stddef.h>

/*
 * Declarations supplied by bin/csh/iconv.h in the original build.  They are
 * reproduced here so that this translation unit is self contained.
 */
typedef void *iconv_t;
typedef size_t dl_iconv_t(iconv_t, const char **, size_t *, char **, size_t *);
typedef int dl_iconv_close_t(iconv_t);

#ifndef __FreeBSD__
/*
 * dlfunc(3) is a FreeBSD extension.  It is dlsym(3) with a return type that is
 * a function pointer rather than an object pointer.
 */
typedef void (*pbsd_dlfunc_t)(void);

static pbsd_dlfunc_t
pbsd_dlfunc(void *handle, const char *symbol)
{
	union {
		void *d;
		pbsd_dlfunc_t f;
	} u;

	u.d = dlsym(handle, symbol);
	return u.f;
}
#define dlfunc	pbsd_dlfunc
#endif

#undef iconv_open
#undef iconv
#undef iconv_close

#define ICONVLIB	"libiconv.so"
#define ICONV_ENGINE	"libiconv"
#define ICONV_OPEN	"libiconv_open"
#define ICONV_CLOSE	"libiconv_close"

typedef iconv_t iconv_open_t(const char *, const char *);

dl_iconv_t *dl_iconv;
dl_iconv_close_t *dl_iconv_close;

static int initialized;
static void *iconvlib;
static iconv_open_t *iconv_open;

iconv_t
ref_dl_iconv_open(const char *tocode, const char *fromcode)
{
	if (initialized) {
		if (iconvlib == NULL)
			return (iconv_t)-1;
	} else {
		initialized = 1;
		iconvlib = dlopen(ICONVLIB, RTLD_LAZY | RTLD_GLOBAL);
		if (iconvlib == NULL)
			return (iconv_t)-1;
		iconv_open = (iconv_open_t *)dlfunc(iconvlib, ICONV_OPEN);
		if (iconv_open == NULL)
			goto dlfunc_err;
		dl_iconv = (dl_iconv_t *)dlfunc(iconvlib, ICONV_ENGINE);
		if (dl_iconv == NULL)
			goto dlfunc_err;
		dl_iconv_close = (dl_iconv_close_t *)dlfunc(iconvlib,
		    ICONV_CLOSE);
		if (dl_iconv_close == NULL)
			goto dlfunc_err;
	}
	return iconv_open(tocode, fromcode);

dlfunc_err:
	dlclose(iconvlib);
	iconvlib = NULL;
	return (iconv_t)-1;
}
