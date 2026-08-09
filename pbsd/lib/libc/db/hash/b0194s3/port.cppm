/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
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
 * Batch b0194s3: hbsd/src/lib/libc/db/hash/hash.c
 *
 * The functions of hash.c that are not present here are listed, with the
 * reason for their absence, in skipped.txt.
 */

module;

#include <stdlib.h>
#include <string.h>

export module pbsd.lib.libc.db.hash.b0194s3;

export namespace pbsd::lib_libc_db_hash::b0194s3 {

/*
 * SEGMENT is declared by hash.h as "BUFHEAD **", BUFHEAD being page.h's
 * struct _bufhead.  Neither header belongs to this batch; the ported code
 * below only ever moves the pointer around, so the pointee stays incomplete.
 */
struct _bufhead;
using SEGMENT = _bufhead **;

/*
 * If realloc guarantees that the pointer is not destroyed if the realloc
 * fails, then this routine can go away.
 */
void *
hash_realloc(SEGMENT **p_ptr, int oldsize, int newsize)
{
	void *p;

	if ( (p = malloc(newsize)) ) {
		memmove(p, *p_ptr, oldsize);
		memset((char *)p + oldsize, 0, newsize - oldsize);
		free(*p_ptr);
		*p_ptr = (SEGMENT *)p;
	}
	return (p);
}

} /* namespace pbsd::lib_libc_db_hash::b0194s3 */
