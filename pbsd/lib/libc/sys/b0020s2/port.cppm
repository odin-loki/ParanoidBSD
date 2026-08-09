/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Kyle Evans <kevans@FreeBSD.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

export module pbsd.lib.libc.sys.b0020s2;

/*
 * In libc this prototype comes from "libc_private.h"; it is the raw
 * close_range(2) system call stub and is deliberately left as an external
 * dependency so the caller supplies it.  C language linkage keeps the symbol
 * unmangled and shared with the C oracle's declaration.
 */
extern "C" int __sys_close_range(unsigned int lowfd, unsigned int highfd,
    int flags);

export namespace pbsd::lib_libc_sys::b0020s2 {

/*
 * <sys/param.h>: #define MAX(a,b) (((a)>(b))?(a):(b))
 * The comparison is done in int, and the int result is then converted to the
 * unsigned int parameter type.
 */
void
closefrom(int lowfd)
{
	__sys_close_range((((0) > (lowfd)) ? (0) : (lowfd)), ~0U, 0);
}

} // namespace pbsd::lib_libc_sys::b0020s2
