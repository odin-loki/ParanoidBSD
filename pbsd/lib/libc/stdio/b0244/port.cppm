/*
 * PBSD batch b0244 -- C++23 port of
 *     lib/libc/stdio/fcloseall.c
 *     lib/libc/stdio/getline.c
 *     lib/libc/stdio/dprintf.c
 *     lib/libc/stdio/fwide.c
 *
 * Faithful translation: behaviour, signedness, evaluation order and pointer
 * arithmetic are preserved exactly.  Original copyright headers are retained
 * with each function.
 */

module;

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <wchar.h>

/*
 * Declared by the libc-internal "local.h" in the original tree.
 */
extern "C" int _fwalk(int (*)(FILE *));

export module pbsd.lib.libc.stdio.b0244;

export namespace pbsd::lib_libc_stdio::b0244 {

/*
 * FreeBSD's struct __sFILE carries the wide-orientation flag that fwide()
 * manipulates; glibc's FILE has no such member, so the single field the
 * function touches is modelled here with the layout the oracle uses.  The
 * guard arrays and the lock counters let the differential harness observe
 * out-of-object writes and the FLOCKFILE/FUNLOCKFILE pairing.
 */
struct __pbsd_sFILE {
	unsigned char _pbsd_guard_lo[8];
	int _orientation;
	int _pbsd_lockdepth;
	int _pbsd_lockseq;
	unsigned char _pbsd_guard_hi[8];
};

} /* namespace pbsd::lib_libc_stdio::b0244 */

/* From "libc_private.h": no-ops when the process is single threaded. */
#define	FLOCKFILE(fp)	do {						\
		(fp)->_pbsd_lockdepth++;				\
		(fp)->_pbsd_lockseq++;					\
	} while (0)
#define	FUNLOCKFILE(fp)	do {						\
		(fp)->_pbsd_lockdepth--;				\
		(fp)->_pbsd_lockseq++;					\
	} while (0)

export namespace pbsd::lib_libc_stdio::b0244 {

/* ====================================================================== */
/* lib/libc/stdio/fcloseall.c                                             */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2006 Daniel M. Eischen.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * __weak_reference(__fcloseall, fcloseall) in the original is a linker
 * directive, not translatable code; the alias is provided by the link editor.
 */

void
__fcloseall(void)
{

	(void)::_fwalk(::fclose);
}

/* ====================================================================== */
/* lib/libc/stdio/getline.c                                               */
/* ====================================================================== */

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

::ssize_t
getline(char ** __restrict linep, ::size_t * __restrict linecapp,
	::FILE * __restrict fp)
{

	return (::getdelim(linep, linecapp, '\n', fp));
}

/* ====================================================================== */
/* lib/libc/stdio/dprintf.c                                               */
/* ====================================================================== */

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

int
dprintf(int fd, const char * __restrict fmt, ...)
{
	::va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = ::vdprintf(fd, fmt, ap);
	va_end(ap);
	return (ret);
}

/* ====================================================================== */
/* lib/libc/stdio/fwide.c                                                 */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins.
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
fwide(struct __pbsd_sFILE *fp, int mode)
{
	int m;

	FLOCKFILE(fp);
	/* Only change the orientation if the stream is not oriented yet. */
	if (mode != 0 && fp->_orientation == 0)
		fp->_orientation = mode > 0 ? 1 : -1;
	m = fp->_orientation;
	FUNLOCKFILE(fp);

	return (m);
}

} /* namespace pbsd::lib_libc_stdio::b0244 */
