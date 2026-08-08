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
 * des_crypt.c, DES encryption library routines
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

module;

#include <cstddef>
#include <cstdint>

typedef unsigned char u_char;

enum desdir { ENCRYPT, DECRYPT };
enum desmode { CBC, ECB };

struct desparams {
	u_char des_key[8];
	enum desdir des_dir;
	enum desmode des_mode;
	u_char des_ivec[8];
	unsigned des_len;
	union {
		u_char UDES_data[16];
		u_char *UDES_buf;
	} UDES;
};

extern "C" {
extern int (*__des_crypt_LOCAL)(char *, unsigned, struct desparams *);
extern int _des_crypt_call(char *, int, struct desparams *);
}

export module pbsd.lib.libc.rpc.b0196s4;

namespace pbsd::lib_libc_rpc::b0196s4 {

inline constexpr unsigned DES_MAXDATA = 8192;
inline constexpr unsigned DES_DIRMASK = (1 << 0);
inline constexpr unsigned DES_ENCRYPT = (0 * DES_DIRMASK);
inline constexpr unsigned DES_DECRYPT = (1 * DES_DIRMASK);
inline constexpr unsigned DES_DEVMASK = (1 << 1);
inline constexpr unsigned DES_HW = (0 * DES_DEVMASK);
inline constexpr unsigned DES_SW = (1 * DES_DEVMASK);
inline constexpr int DESERR_NONE = 0;
inline constexpr int DESERR_NOHWDEVICE = 1;
inline constexpr int DESERR_HWERROR = 2;
inline constexpr int DESERR_BADPARAM = 3;

#define COPY8(src, dst) { \
	char *a = (char *) dst; \
	char *b = (char *) src; \
	*a++ = *b++; *a++ = *b++; *a++ = *b++; *a++ = *b++; \
	*a++ = *b++; *a++ = *b++; *a++ = *b++; *a++ = *b++; \
}

#define DESCOPY(src, dst, len) { \
	char *a = (char *) dst; \
	char *b = (char *) src; \
	int i; \
	for (i = (int) len; i > 0; i -= 8) { \
		*a++ = *b++; *a++ = *b++; *a++ = *b++; *a++ = *b++; \
		*a++ = *b++; *a++ = *b++; *a++ = *b++; *a++ = *b++; \
	} \
}

static int
common_crypt(char *key, char *buf, unsigned len, unsigned mode,
    struct desparams *desp)
{
	int desdev;

	if ((len % 8) != 0 || len > DES_MAXDATA) {
		return(DESERR_BADPARAM);
	}
	desp->des_dir =
		((mode & DES_DIRMASK) == DES_ENCRYPT) ? ::ENCRYPT : ::DECRYPT;

	desdev = mode & DES_DEVMASK;
	COPY8(key, desp->des_key);
	/* 
	 * software
	 */
	if (__des_crypt_LOCAL != NULL) {
		if (!__des_crypt_LOCAL(buf, len, desp)) {
			return (DESERR_HWERROR);
		}
	} else {
		if (!_des_crypt_call(buf, len, desp)) {
			return (DESERR_HWERROR);
		}
	}
	return(desdev == DES_SW ? DESERR_NONE : DESERR_NOHWDEVICE);
}

export int
cbc_crypt(char *key, char *buf, unsigned len, unsigned mode, char *ivec)
{
	int err;
	struct desparams dp;

#ifdef BROKEN_DES
	dp.UDES.UDES_buf = buf;
	dp.des_mode = ::ECB;
#else
	dp.des_mode = ::CBC;
#endif
	COPY8(ivec, dp.des_ivec);
	err = common_crypt(key, buf, len, mode, &dp);
	COPY8(dp.des_ivec, ivec);
	return(err);
}

export int
ecb_crypt(char *key, char *buf, unsigned len, unsigned mode)
{
	struct desparams dp;

#ifdef BROKEN_DES
	dp.UDES.UDES_buf = buf;
	dp.des_mode = ::CBC;
#else
	dp.des_mode = ::ECB;
#endif
	return(common_crypt(key, buf, len, mode, &dp));
}

} /* namespace pbsd::lib_libc_rpc::b0196s4 */
