/*
 * oracle.c -- reference implementation for PBSD batch b0029s1.
 *
 * hbsd/src/sys/security/mac_veriexec/mac_veriexec_sha384.c concatenated.
 * Functions are renamed with a ref_ prefix and lose "static" so the harness
 * can link against them; bodies are otherwise unmodified.  Kernel-only
 * MAC_VERIEXEC_FPMOD registration is expanded via a no-op shim.
 *
 * The batch source defines no functions; see skipped.txt.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * 8)
#endif

/* --- shims for kernel headers referenced by the batch source ---------- */

typedef void (*mac_veriexec_fpop_init_t)(void *);
typedef void (*mac_veriexec_fpop_update_t)(void *, const uint8_t *, size_t);
typedef void (*mac_veriexec_fpop_final_t)(uint8_t *, void *);

#define	MAC_VERIEXEC_FPMOD(_name, _digest_len, _context_size, _init,	\
	    _update, _final, _vers)					\
	static int pbsd_mac_veriexec_fpmod_##_name##_unused		\
	    __attribute__((unused)) = 0

struct SHA384_CTX { char opaque[216]; };

void SHA384_Init(struct SHA384_CTX *);
void SHA384_Update(struct SHA384_CTX *, const uint8_t *, size_t);
void SHA384_Final(uint8_t *, struct SHA384_CTX *);

/* --- mac_veriexec_sha384.c -------------------------------------------- */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2011, 2012, 2013, 2015, 2016, Juniper Networks, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define	SHA384_DIGEST_LENGTH	48

MAC_VERIEXEC_FPMOD(SHA384, SHA384_DIGEST_LENGTH, sizeof(SHA384_CTX),
    (mac_veriexec_fpop_init_t) SHA384_Init,
    (mac_veriexec_fpop_update_t) SHA384_Update,
    (mac_veriexec_fpop_final_t) SHA384_Final, 1);
