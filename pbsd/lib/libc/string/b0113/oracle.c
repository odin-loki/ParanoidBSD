/*
 * Reference oracle for PBSD batch b0113.
 *
 * The original HardenedBSD C sources concatenated verbatim; every function has
 * been renamed with a `ref_` prefix and nothing else has been touched.  The
 * FreeBSD-internal <ssp/ssp.h> include is not available off-FreeBSD, so the
 * __ssp_real() name-mangling wrapper is replaced by the plain renamed symbol.
 *
 * Sources:
 *   hbsd/src/lib/libc/string/mempcpy.c
 *   hbsd/src/lib/libc/string/wcpncpy.c
 *   hbsd/src/lib/libc/string/wmempcpy.c
 */

#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <wchar.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/string/mempcpy.c                                        */
/* ------------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 The FreeBSD Foundation
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

void *
(ref_mempcpy)(void *__restrict dst, const void *__restrict src,
    size_t len)
{
	return ((char *)memcpy(dst, src, len) + len);
}

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/string/wcpncpy.c                                        */
/* ------------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2009 David Schultz <das@FreeBSD.org>
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

wchar_t *
ref_wcpncpy(wchar_t * __restrict dst, const wchar_t * __restrict src,
    size_t n)
{

	for (; n--; dst++, src++) {
		if (!(*dst = *src)) {
			wchar_t *ret = dst;
			while (n--)
				*++dst = L'\0';
			return (ret);
		}
	}
	return (dst);
}

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/string/wmempcpy.c                                       */
/* ------------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 The FreeBSD Foundation
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

wchar_t *
ref_wmempcpy(wchar_t *__restrict dst, const wchar_t *__restrict src,
    size_t len)
{
	return (wmemcpy(dst, src, len) + len);
}
