/*
 * oracle.c -- reference implementation for PBSD batch b0029.
 *
 * hbsd/src/sys/security/mac_veriexec/mac_veriexec_sha{1,256,384,512}.c
 * concatenated.  Functions are renamed with a ref_ prefix and lose "static"
 * so the harness can link against them; bodies are otherwise unmodified.
 * Kernel-only MAC_VERIEXEC_FPMOD registration is expanded via a no-op shim.
 * sha1_init/loop/result are linked from the userspace adaptation of
 * hbsd/src/sys/crypto/sha1.c below (not part of the batch; support only).
 */

#include <endian.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * 8)
#endif

#ifndef BYTE_ORDER
#define	BYTE_ORDER	__BYTE_ORDER
#define	LITTLE_ENDIAN	__LITTLE_ENDIAN
#define	BIG_ENDIAN	__BIG_ENDIAN
#endif

#define	bzero(x, y)	memset((x), 0, (y))
#define	bcopy(s, d, n)	memcpy((d), (s), (n))

typedef char *caddr_t;

struct sha1_ctxt {
	union {
		uint8_t		b8[20];
		uint32_t	b32[5];
	} h;
	union {
		uint8_t		b8[8];
		uint64_t	b64[1];
	} c;
	union {
		uint8_t		b8[64];
		uint32_t	b32[16];
	} m;
	uint8_t	count;
};
typedef struct sha1_ctxt SHA1_CTX;

#define	SHA1_RESULTLEN	(160 / 8)

/* --- userspace sha1.c (support; not batch source) --------------------- */

#if BYTE_ORDER != BIG_ENDIAN
# if BYTE_ORDER != LITTLE_ENDIAN
#  define unsupported 1
# endif
#endif

#ifndef unsupported

static uint32_t _K[] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };
#define	K(t)	_K[(t) / 20]

#define	F0(b, c, d)	(((b) & (c)) | ((~(b)) & (d)))
#define	F1(b, c, d)	(((b) ^ (c)) ^ (d))
#define	F2(b, c, d)	(((b) & (c)) | ((b) & (d)) | ((c) & (d)))
#define	F3(b, c, d)	(((b) ^ (c)) ^ (d))

#define	S(n, x)		(((x) << (n)) | ((x) >> (32 - n)))

#define	H(n)	(ctxt->h.b32[(n)])
#define	COUNT	(ctxt->count)
#define	BCOUNT	(ctxt->c.b64[0] / 8)
#define	W(n)	(ctxt->m.b32[(n)])

#define	PUTBYTE(x)	{ \
	ctxt->m.b8[(COUNT % 64)] = (x);		\
	COUNT++;				\
	COUNT %= 64;				\
	ctxt->c.b64[0] += 8;			\
	if (COUNT % 64 == 0)			\
		sha1_step(ctxt);		\
     }

#define	PUTPAD(x)	{ \
	ctxt->m.b8[(COUNT % 64)] = (x);		\
	COUNT++;				\
	COUNT %= 64;				\
	if (COUNT % 64 == 0)			\
		sha1_step(ctxt);		\
     }

static void sha1_step(struct sha1_ctxt *);

static void
sha1_step(struct sha1_ctxt *ctxt)
{
	uint32_t	a, b, c, d, e;
	size_t t, s;
	uint32_t	tmp;

#if BYTE_ORDER == LITTLE_ENDIAN
	struct sha1_ctxt tctxt;
	bcopy(&ctxt->m.b8[0], &tctxt.m.b8[0], 64);
	ctxt->m.b8[0] = tctxt.m.b8[3]; ctxt->m.b8[1] = tctxt.m.b8[2];
	ctxt->m.b8[2] = tctxt.m.b8[1]; ctxt->m.b8[3] = tctxt.m.b8[0];
	ctxt->m.b8[4] = tctxt.m.b8[7]; ctxt->m.b8[5] = tctxt.m.b8[6];
	ctxt->m.b8[6] = tctxt.m.b8[5]; ctxt->m.b8[7] = tctxt.m.b8[4];
	ctxt->m.b8[8] = tctxt.m.b8[11]; ctxt->m.b8[9] = tctxt.m.b8[10];
	ctxt->m.b8[10] = tctxt.m.b8[9]; ctxt->m.b8[11] = tctxt.m.b8[8];
	ctxt->m.b8[12] = tctxt.m.b8[15]; ctxt->m.b8[13] = tctxt.m.b8[14];
	ctxt->m.b8[14] = tctxt.m.b8[13]; ctxt->m.b8[15] = tctxt.m.b8[12];
	ctxt->m.b8[16] = tctxt.m.b8[19]; ctxt->m.b8[17] = tctxt.m.b8[18];
	ctxt->m.b8[18] = tctxt.m.b8[17]; ctxt->m.b8[19] = tctxt.m.b8[16];
	ctxt->m.b8[20] = tctxt.m.b8[23]; ctxt->m.b8[21] = tctxt.m.b8[22];
	ctxt->m.b8[22] = tctxt.m.b8[21]; ctxt->m.b8[23] = tctxt.m.b8[20];
	ctxt->m.b8[24] = tctxt.m.b8[27]; ctxt->m.b8[25] = tctxt.m.b8[26];
	ctxt->m.b8[26] = tctxt.m.b8[25]; ctxt->m.b8[27] = tctxt.m.b8[24];
	ctxt->m.b8[28] = tctxt.m.b8[31]; ctxt->m.b8[29] = tctxt.m.b8[30];
	ctxt->m.b8[30] = tctxt.m.b8[29]; ctxt->m.b8[31] = tctxt.m.b8[28];
	ctxt->m.b8[32] = tctxt.m.b8[35]; ctxt->m.b8[33] = tctxt.m.b8[34];
	ctxt->m.b8[34] = tctxt.m.b8[33]; ctxt->m.b8[35] = tctxt.m.b8[32];
	ctxt->m.b8[36] = tctxt.m.b8[39]; ctxt->m.b8[37] = tctxt.m.b8[38];
	ctxt->m.b8[38] = tctxt.m.b8[37]; ctxt->m.b8[39] = tctxt.m.b8[36];
	ctxt->m.b8[40] = tctxt.m.b8[43]; ctxt->m.b8[41] = tctxt.m.b8[42];
	ctxt->m.b8[42] = tctxt.m.b8[41]; ctxt->m.b8[43] = tctxt.m.b8[40];
	ctxt->m.b8[44] = tctxt.m.b8[47]; ctxt->m.b8[45] = tctxt.m.b8[46];
	ctxt->m.b8[46] = tctxt.m.b8[45]; ctxt->m.b8[47] = tctxt.m.b8[44];
	ctxt->m.b8[48] = tctxt.m.b8[51]; ctxt->m.b8[49] = tctxt.m.b8[50];
	ctxt->m.b8[50] = tctxt.m.b8[49]; ctxt->m.b8[51] = tctxt.m.b8[48];
	ctxt->m.b8[52] = tctxt.m.b8[55]; ctxt->m.b8[53] = tctxt.m.b8[54];
	ctxt->m.b8[54] = tctxt.m.b8[53]; ctxt->m.b8[55] = tctxt.m.b8[52];
	ctxt->m.b8[56] = tctxt.m.b8[59]; ctxt->m.b8[57] = tctxt.m.b8[58];
	ctxt->m.b8[58] = tctxt.m.b8[57]; ctxt->m.b8[59] = tctxt.m.b8[56];
	ctxt->m.b8[60] = tctxt.m.b8[63]; ctxt->m.b8[61] = tctxt.m.b8[62];
	ctxt->m.b8[62] = tctxt.m.b8[61]; ctxt->m.b8[63] = tctxt.m.b8[60];
#endif

	a = H(0); b = H(1); c = H(2); d = H(3); e = H(4);

	for (t = 0; t < 20; t++) {
		s = t & 0x0f;
		if (t >= 16) {
			W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
		}
		tmp = S(5, a) + F0(b, c, d) + e + W(s) + K(t);
		e = d; d = c; c = S(30, b); b = a; a = tmp;
	}
	for (t = 20; t < 40; t++) {
		s = t & 0x0f;
		W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
		tmp = S(5, a) + F1(b, c, d) + e + W(s) + K(t);
		e = d; d = c; c = S(30, b); b = a; a = tmp;
	}
	for (t = 40; t < 60; t++) {
		s = t & 0x0f;
		W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
		tmp = S(5, a) + F2(b, c, d) + e + W(s) + K(t);
		e = d; d = c; c = S(30, b); b = a; a = tmp;
	}
	for (t = 60; t < 80; t++) {
		s = t & 0x0f;
		W(s) = S(1, W((s+13) & 0x0f) ^ W((s+8) & 0x0f) ^ W((s+2) & 0x0f) ^ W(s));
		tmp = S(5, a) + F3(b, c, d) + e + W(s) + K(t);
		e = d; d = c; c = S(30, b); b = a; a = tmp;
	}

	H(0) = H(0) + a;
	H(1) = H(1) + b;
	H(2) = H(2) + c;
	H(3) = H(3) + d;
	H(4) = H(4) + e;

	bzero(&ctxt->m.b8[0], 64);
}

void
sha1_init(struct sha1_ctxt *ctxt)
{
	bzero(ctxt, sizeof(struct sha1_ctxt));
	H(0) = 0x67452301;
	H(1) = 0xefcdab89;
	H(2) = 0x98badcfe;
	H(3) = 0x10325476;
	H(4) = 0xc3d2e1f0;
}

void
sha1_pad(struct sha1_ctxt *ctxt)
{
	size_t padlen;
	size_t padstart;

	PUTPAD(0x80);

	padstart = COUNT % 64;
	padlen = 64 - padstart;
	if (padlen < 8) {
		bzero(&ctxt->m.b8[padstart], padlen);
		COUNT += padlen;
		COUNT %= 64;
		sha1_step(ctxt);
		padstart = COUNT % 64;
		padlen = 64 - padstart;
	}
	bzero(&ctxt->m.b8[padstart], padlen - 8);
	COUNT += (padlen - 8);
	COUNT %= 64;
#if BYTE_ORDER == BIG_ENDIAN
	PUTPAD(ctxt->c.b8[0]); PUTPAD(ctxt->c.b8[1]);
	PUTPAD(ctxt->c.b8[2]); PUTPAD(ctxt->c.b8[3]);
	PUTPAD(ctxt->c.b8[4]); PUTPAD(ctxt->c.b8[5]);
	PUTPAD(ctxt->c.b8[6]); PUTPAD(ctxt->c.b8[7]);
#else
	PUTPAD(ctxt->c.b8[7]); PUTPAD(ctxt->c.b8[6]);
	PUTPAD(ctxt->c.b8[5]); PUTPAD(ctxt->c.b8[4]);
	PUTPAD(ctxt->c.b8[3]); PUTPAD(ctxt->c.b8[2]);
	PUTPAD(ctxt->c.b8[1]); PUTPAD(ctxt->c.b8[0]);
#endif
}

void
sha1_loop(struct sha1_ctxt *ctxt, const uint8_t *input, size_t len)
{
	size_t gaplen;
	size_t gapstart;
	size_t off;
	size_t copysiz;

	off = 0;

	while (off < len) {
		gapstart = COUNT % 64;
		gaplen = 64 - gapstart;

		copysiz = (gaplen < len - off) ? gaplen : len - off;
		bcopy(&input[off], &ctxt->m.b8[gapstart], copysiz);
		COUNT += copysiz;
		COUNT %= 64;
		ctxt->c.b64[0] += copysiz * 8;
		if (COUNT % 64 == 0)
			sha1_step(ctxt);
		off += copysiz;
	}
}

void
sha1_result(struct sha1_ctxt *ctxt, char digest0[static SHA1_RESULTLEN])
{
	uint8_t *digest;

	digest = (uint8_t *)digest0;
	sha1_pad(ctxt);
#if BYTE_ORDER == BIG_ENDIAN
	bcopy(&ctxt->h.b8[0], digest, SHA1_RESULTLEN);
#else
	digest[0] = ctxt->h.b8[3]; digest[1] = ctxt->h.b8[2];
	digest[2] = ctxt->h.b8[1]; digest[3] = ctxt->h.b8[0];
	digest[4] = ctxt->h.b8[7]; digest[5] = ctxt->h.b8[6];
	digest[6] = ctxt->h.b8[5]; digest[7] = ctxt->h.b8[4];
	digest[8] = ctxt->h.b8[11]; digest[9] = ctxt->h.b8[10];
	digest[10] = ctxt->h.b8[9]; digest[11] = ctxt->h.b8[8];
	digest[12] = ctxt->h.b8[15]; digest[13] = ctxt->h.b8[14];
	digest[14] = ctxt->h.b8[13]; digest[15] = ctxt->h.b8[12];
	digest[16] = ctxt->h.b8[19]; digest[17] = ctxt->h.b8[18];
	digest[18] = ctxt->h.b8[17]; digest[19] = ctxt->h.b8[16];
#endif
}

#endif /* unsupported */

/* --- shims for kernel headers referenced by the batch sources ---------- */

typedef void (*mac_veriexec_fpop_init_t)(void *);
typedef void (*mac_veriexec_fpop_update_t)(void *, const uint8_t *, size_t);
typedef void (*mac_veriexec_fpop_final_t)(uint8_t *, void *);

#define	MAC_VERIEXEC_FPMOD(_name, _digest_len, _context_size, _init,	\
	    _update, _final, _vers)					\
	static int pbsd_mac_veriexec_fpmod_##_name##_unused		\
	    __attribute__((unused)) = 0

struct SHA256_CTX { char opaque[108]; };
struct SHA384_CTX { char opaque[216]; };
struct SHA512_CTX { char opaque[216]; };

void SHA256_Init(struct SHA256_CTX *);
void SHA256_Update(struct SHA256_CTX *, const uint8_t *, size_t);
void SHA256_Final(uint8_t *, struct SHA256_CTX *);
void SHA384_Init(struct SHA384_CTX *);
void SHA384_Update(struct SHA384_CTX *, const uint8_t *, size_t);
void SHA384_Final(uint8_t *, struct SHA384_CTX *);
void SHA512_Init(struct SHA512_CTX *);
void SHA512_Update(struct SHA512_CTX *, const uint8_t *, size_t);
void SHA512_Final(uint8_t *, struct SHA512_CTX *);

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

/* --- mac_veriexec_sha256.c -------------------------------------------- */

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

#define	SHA256_DIGEST_LENGTH	32

MAC_VERIEXEC_FPMOD(SHA256, SHA256_DIGEST_LENGTH, sizeof(SHA256_CTX),
    (mac_veriexec_fpop_init_t) SHA256_Init,
    (mac_veriexec_fpop_update_t) SHA256_Update,
    (mac_veriexec_fpop_final_t) SHA256_Final, 1);

/* --- mac_veriexec_sha512.c -------------------------------------------- */

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

#define	SHA512_DIGEST_LENGTH	64

MAC_VERIEXEC_FPMOD(SHA512, SHA512_DIGEST_LENGTH, sizeof(SHA512_CTX),
    (mac_veriexec_fpop_init_t) SHA512_Init,
    (mac_veriexec_fpop_update_t) SHA512_Update,
    (mac_veriexec_fpop_final_t) SHA512_Final, 1);

/* --- mac_veriexec_sha1.c ---------------------------------------------- */

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

#define	SHA1_Init	(mac_veriexec_fpop_init_t) sha1_init
#define	SHA1_Update	(mac_veriexec_fpop_update_t) sha1_loop

void
ref_SHA1_Final(unsigned char *buf, void *ctx)
{

	sha1_result((SHA1_CTX *) ctx, (caddr_t) buf);
}

MAC_VERIEXEC_FPMOD(SHA1, SHA1_RESULTLEN, sizeof(SHA1_CTX), SHA1_Init,
    SHA1_Update, ref_SHA1_Final, 1);
