/*
 * PBSD batch b0122 -- reference oracle.
 *
 * The original HardenedBSD sources for
 *	lib/libc/uuid/uuid_create.c
 *	lib/libc/uuid/uuid_create_nil.c
 *	lib/libc/uuid/uuid_hash.c
 * concatenated, with every function renamed with a "ref_" prefix.  The
 * function bodies are UNMODIFIED.  Only the declarations that <uuid.h> and
 * <sys/uuid.h> would have supplied on FreeBSD (struct uuid, uuid_t,
 * uuid_s_ok, the uuidgen() system call prototype) have been added, because
 * those headers do not exist on the host this oracle is compiled for.
 */

#define _DEFAULT_SOURCE 1

#include <stdint.h>
#include <strings.h>

/* From <sys/uuid.h>: */
#define	_UUID_NODE_LEN	6

struct uuid {
	uint32_t	time_low;
	uint16_t	time_mid;
	uint16_t	time_hi_and_version;
	uint8_t		clock_seq_hi_and_reserved;
	uint8_t		clock_seq_low;
	uint8_t		node[_UUID_NODE_LEN];
};

/* From <uuid.h>: */
typedef struct uuid uuid_t;

#define	uuid_s_ok			0
#define	uuid_s_bad_version		1
#define	uuid_s_invalid_string_uuid	2
#define	uuid_s_no_memory		3

/*
 * From <unistd.h> on FreeBSD: the uuidgen(2) system call.  Declared with a
 * void * first parameter so that the C, the C++ module and the harness all
 * agree on one prototype for the shared symbol; the call in the unmodified
 * body below converts implicitly.
 */
int uuidgen(void *store, int count);

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Marcel Moolenaar
 * Copyright (c) 2002 Hiten Mahesh Pandya
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

/* #include <uuid.h> */

/*
 * uuid_create() - create an UUID.
 * See also:
 *	http://www.opengroup.org/onlinepubs/009629399/uuid_create.htm
 */
void
ref_uuid_create(uuid_t *u, uint32_t *status)
{

	if (status)
		*status = uuid_s_ok;

	uuidgen(u, 1);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Marcel Moolenaar
 * Copyright (c) 2002 Hiten Mahesh Pandya
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

/* #include <strings.h> */
/* #include <uuid.h> */

/*
 * uuid_create_nil() - create a nil UUID.
 * See also:
 *	http://www.opengroup.org/onlinepubs/009629399/uuid_create_nil.htm
 */
void
ref_uuid_create_nil(uuid_t *u, uint32_t *status)
{

	if (status)
		*status = uuid_s_ok;

	bzero(u, sizeof(*u));
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002,2005 Marcel Moolenaar
 * Copyright (c) 2002 Hiten Mahesh Pandya
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

/* #include <uuid.h> */

/*
 * uuid_hash() - generate a hash value.
 * See also:
 *	http://www.opengroup.org/onlinepubs/009629399/uuid_hash.htm
 */
uint16_t
ref_uuid_hash(const uuid_t *u, uint32_t *status)
{

	if (status)
		*status = uuid_s_ok;

	/*
	 * Use the most frequently changing bits in the UUID as the hash
	 * value. This should yield a good enough distribution...
	 */
	return ((u) ? u->time_low & 0xffff : 0);
}
