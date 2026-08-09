/*-
 * Copyright (c) 2012 Gleb Kurtsou <gleb@FreeBSD.org>
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

/*
 * Port of lib/libc/sys/getdents.c.
 *
 * The libc-internal syscall stub is declared here rather than pulled in from
 * "namespace.h"/"libc_private.h"; the call itself is unchanged.
 */

module;

#include <sys/types.h>
#include <stddef.h>

export module pbsd.lib.libc.sys.b0020s4;

extern "C" ssize_t __sys_getdirentries(int fd, char *buf, size_t nbytes,
    off_t *basep);

export namespace pbsd::lib_libc_sys::b0020s4 {

ssize_t
getdents(int fd, char *buf, size_t nbytes)
{

	return (__sys_getdirentries(fd, buf, nbytes, NULL));
}

} // namespace pbsd::lib_libc_sys::b0020s4
