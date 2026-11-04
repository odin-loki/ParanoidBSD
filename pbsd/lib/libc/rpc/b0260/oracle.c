/*
 * Batch b0260 oracle: original HardenedBSD C sources concatenated.
 * Each function renamed with ref_ prefix; bodies otherwise unmodified.
 * Infrastructure (types, mocks, wrappers) added for standalone compilation.
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (8 * (int)sizeof(long))
#endif

#define __weak_reference(sym, alias)

typedef int bool_t;
#define TRUE 1
#define FALSE 0

typedef uint32_t u_long;
typedef int32_t enum_t;
typedef char *caddr_t;
typedef unsigned int u_int;

struct opaque_auth {
	enum_t oa_flavor;
	caddr_t oa_base;
	u_int oa_length;
};

struct pmap {
	u_long pm_prog;
	u_long pm_vers;
	u_long pm_prot;
	u_long pm_port;
};

enum xdr_op { XDR_ENCODE = 0, XDR_DECODE = 1, XDR_FREE = 2 };

typedef struct XDR {
	enum xdr_op x_op;
	void *x_private;
} XDR;

typedef bool_t (*xdrproc_t)(XDR *, void *, unsigned int);

typedef struct AUTH AUTH;

int mock_getdtablesize_return = 256;
int mock_getdtablesize_calls = 0;

int mock_xdr_fail_on_call = 0;
int mock_xdr_call_count = 0;
u_long mock_xdr_slot[4];

void
mock_reset_b0260(void)
{
	mock_getdtablesize_return = 256;
	mock_getdtablesize_calls = 0;
	mock_xdr_fail_on_call = 0;
	mock_xdr_call_count = 0;
	memset(mock_xdr_slot, 0, sizeof(mock_xdr_slot));
}

int
getdtablesize(void)
{
	mock_getdtablesize_calls++;
	return (mock_getdtablesize_return);
}

bool_t
xdr_u_long(XDR *xdrs, u_long *ulp)
{
	int slot;

	mock_xdr_call_count++;
	if (mock_xdr_fail_on_call == mock_xdr_call_count)
		return (FALSE);
	slot = mock_xdr_call_count - 1;
	if (slot < 0 || slot >= 4)
		return (FALSE);
	if (xdrs->x_op == XDR_ENCODE)
		mock_xdr_slot[slot] = *ulp;
	else
		*ulp = mock_xdr_slot[slot];
	return (TRUE);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2006 Doug Rabson
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

bool_t
ref___rpc_gss_wrap_stub(AUTH *auth, void *header, size_t headerlen, XDR* xdrs,
    xdrproc_t xdr_args, void *args_ptr)
{

	return (FALSE);
}

bool_t
ref___rpc_gss_unwrap_stub(AUTH *auth, XDR* xdrs, xdrproc_t xdr_args, void *args_ptr)
{

	return (FALSE);
}

__weak_reference(ref___rpc_gss_wrap_stub,	__rpc_gss_wrap);
__weak_reference(ref___rpc_gss_unwrap_stub,	__rpc_gss_unwrap);

/*	$NetBSD: rpc_commondata.c,v 1.7 2000/06/02 23:11:13 fvdl Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file should only contain common data (global data) that is exported
 * by public interfaces 
 */
struct opaque_auth ref__null_auth;
fd_set ref_svc_fdset;
int ref_svc_maxfd = -1;

/*	$NetBSD: pmap_prot.c,v 1.10 2000/01/22 22:19:18 mycroft Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * pmap_prot.c
 * Protocol for the local binder service, or pmap.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

bool_t
ref_xdr_pmap(XDR *xdrs, struct pmap *regs)
{

	assert(xdrs != NULL);
	assert(regs != NULL);

	if (xdr_u_long(xdrs, &regs->pm_prog) &&
		xdr_u_long(xdrs, &regs->pm_vers) &&
		xdr_u_long(xdrs, &regs->pm_prot))
		return (xdr_u_long(xdrs, &regs->pm_port));
	return (FALSE);
}

/*	$NetBSD: rpc_dtablesize.c,v 1.14 1998/11/15 17:32:43 christos Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

int ref__rpc_dtablesize(void);	/* XXX */

/*
 * Cache the result of getdtablesize(), so we don't have to do an
 * expensive system call every time.
 */
/*
 * XXX In FreeBSD 2.x, you can have the maximum number of open file
 * descriptors be greater than FD_SETSIZE (which us 256 by default).
 *
 * Since old programs tend to use this call to determine the first arg
 * for _select(), having this return > FD_SETSIZE is a Bad Idea(TM)!
 */
int
ref__rpc_dtablesize(void)
{
	static int size;

	if (size == 0) {
		size = getdtablesize();
		if (size > FD_SETSIZE)
			size = FD_SETSIZE;
	}
	return (size);
}
