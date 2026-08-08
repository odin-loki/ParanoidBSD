
/*
 * Batch b0234 oracle: original HardenedBSD C sources concatenated.
 * Every function renamed with ref_ prefix; bodies otherwise unmodified.
 */

#include <stddef.h>

#ifdef PBSD_B0234_PORT_INCLUDE
#ifdef __cplusplus
extern "C" {
#endif
struct pthread {
	int error;
};
extern struct pthread *_thr_initial;
extern struct pthread *_get_curthread(void);
extern int __libsys_errno;
extern void b0234_set_curthread(struct pthread *cur);
#ifdef __cplusplus
}
#endif
#else /* PBSD_B0234_PORT_INCLUDE */

struct pthread {
	int error;
};

struct pthread *_thr_initial;
int __libsys_errno;

static struct pthread *b0234_curthread;

struct pthread *
_get_curthread(void)
{
	return (b0234_curthread);
}

void
b0234_set_curthread(struct pthread *cur)
{
	b0234_curthread = cur;
}

/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 1995 John Birrell <jb@cimlogic.com.au>.
 * Copyright (c) 1994 by Chris Provenzano, proven@mit.edu
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by John Birrell
 *  and Chris Provenzano.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JOHN BIRRELL AND CONTRIBUTORS ``AS IS'' AND
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

int *
ref___error_threaded(void)
{
	struct pthread *curthread;

	if (_thr_initial != NULL) {
		curthread = _get_curthread();
		if (curthread != NULL && curthread != _thr_initial)
			return (&curthread->error);
	}
	return (&__libsys_errno);
}

#endif /* PBSD_B0234_PORT_INCLUDE */
