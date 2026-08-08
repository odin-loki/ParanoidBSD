/*
 * Batch b0196s4 oracle: original HardenedBSD C sources concatenated.
 * Each function renamed with ref_ prefix; bodies otherwise unmodified.
 * Infrastructure (types, mocks, wrappers) added for standalone compilation.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint32_t rpcprog_t;
typedef uint32_t rpcvers_t;
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

#define DES_MAXDATA 8192
#define DES_DIRMASK (1 << 0)
#define DES_ENCRYPT (0 * DES_DIRMASK)
#define DES_DECRYPT (1 * DES_DIRMASK)
#define DES_DEVMASK (1 << 1)
#define DES_HW (0 * DES_DEVMASK)
#define DES_SW (1 * DES_DEVMASK)
#define DESERR_NONE 0
#define DESERR_NOHWDEVICE 1
#define DESERR_HWERROR 2
#define DESERR_BADPARAM 3

#define NC_LOOPBACK "loopback"
#define CRYPT_PROG 600100029U
#define CRYPT_VERS 1U

typedef void *CLIENT;
typedef int (*xdrproc_t)(void *, void *, unsigned int);

enum des_dir_rpc { ENCRYPT_DES, DECRYPT_DES };
enum des_mode_rpc { CBC_DES, ECB_DES };

typedef struct {
	unsigned int desbuf_len;
	char *desbuf_val;
} opaque_buf;

struct desargs {
	u_char des_key[8];
	enum des_dir_rpc des_dir;
	enum des_mode_rpc des_mode;
	u_char des_ivec[8];
	opaque_buf desbuf;
};

struct desresp {
	opaque_buf desbuf;
	u_char des_ivec[8];
	int stat;
};

typedef struct desargs desargs;
typedef struct desresp desresp;

struct netconfig {
	char *nc_netid;
	unsigned long nc_semantics;
	unsigned long nc_flag;
	char *nc_protofmly;
	char *nc_proto;
	char *nc_device;
	unsigned long nc_nlookups;
	char **nc_lookups;
	unsigned long nc_unused[9];
};

/* ---- mock control (harness-visible) ---- */

int mock_getnetconfig_calls = 0;
int mock_crypt_no_loopback = 0;
int mock_crypt_no_clnt = 0;
int mock_crypt_null_result = 0;
int mock_crypt_result_stat = DESERR_NOHWDEVICE;
int mock_crypt_copy_buf = 1;
char mock_crypt_result_ivec[8];

int mock_des_local_return = 1;
int mock_des_local_xor = 0x55;

void
mock_reset_b0196s4(void)
{
	mock_getnetconfig_calls = 0;
	mock_crypt_no_loopback = 0;
	mock_crypt_no_clnt = 0;
	mock_crypt_null_result = 0;
	mock_crypt_result_stat = DESERR_NOHWDEVICE;
	mock_crypt_copy_buf = 1;
	memset(mock_crypt_result_ivec, 0x42, 8);

	mock_des_local_return = 1;
	mock_des_local_xor = 0x55;

	__des_crypt_LOCAL = 0;
}

void *
setnetconfig(void)
{
	return (void *)0xABCD;
}

struct netconfig *
getnetconfig(void *handle)
{
	static struct netconfig loop_nc;

	(void)handle;
	if (mock_crypt_no_loopback)
		return NULL;
	mock_getnetconfig_calls++;
	if (mock_getnetconfig_calls == 1) {
		loop_nc.nc_protofmly = NC_LOOPBACK;
		loop_nc.nc_proto = "loopback";
		return &loop_nc;
	}
	return NULL;
}

void
endnetconfig(void *handle)
{
	(void)handle;
}

const char *
nc_sperror(void)
{
	return "mock nc error";
}

void
warnx(const char *fmt, ...)
{
	(void)fmt;
}

CLIENT *
clnt_tp_create(const char *host, rpcprog_t prog, rpcvers_t vers,
    struct netconfig *nconf)
{
	(void)host;
	(void)prog;
	(void)vers;
	(void)nconf;
	if (mock_crypt_no_clnt)
		return NULL;
	return (CLIENT *)0x5678;
}

void
clnt_destroy(CLIENT *clnt)
{
	(void)clnt;
}

desresp *
des_crypt_1(desargs *arg, CLIENT *clnt)
{
	static desresp resp;
	(void)clnt;
	if (mock_crypt_null_result)
		return NULL;
	resp.stat = mock_crypt_result_stat;
	resp.desbuf.desbuf_len = arg->desbuf.desbuf_len;
	resp.desbuf.desbuf_val = arg->desbuf.desbuf_val;
	if (mock_crypt_copy_buf && arg->desbuf.desbuf_val != NULL) {
		unsigned int i;
		for (i = 0; i < arg->desbuf.desbuf_len; i++)
			arg->desbuf.desbuf_val[i] ^= (char)mock_des_local_xor;
	}
	memcpy(resp.des_ivec, mock_crypt_result_ivec, 8);
	return &resp;
}

int
clnt_freeres(CLIENT *clnt, xdrproc_t xdr, void *res)
{
	(void)clnt;
	(void)xdr;
	(void)res;
	return 1;
}

void
bcopy(const void *src, void *dst, size_t n)
{
	memcpy(dst, src, n);
}

int
xdr_desresp(void *xdrs, void *objp, unsigned int opt)
{
	(void)xdrs;
	(void)objp;
	(void)opt;
	return 1;
}

int
ref_des_crypt_local_hook(char *buf, unsigned len, struct desparams *desp)
{
	unsigned int i;

	if (mock_des_local_return == 0)
		return 0;
	if (buf != NULL) {
		for (i = 0; i < len; i++)
			buf[i] ^= (char)mock_des_local_xor;
	}
	if (desp != NULL)
		memset(desp->des_ivec, 0x33, 8);
	return mock_des_local_return;
}

int
ref__des_crypt_call(char *buf, int len, struct desparams *dparms)
{
	CLIENT *clnt;
	desresp  *result_1;
	desargs  des_crypt_1_arg;
	struct netconfig *nconf;
	void *localhandle;
	int stat;

	nconf = NULL;
	localhandle = setnetconfig();
	while ((nconf = getnetconfig(localhandle)) != NULL) {
		if (nconf->nc_protofmly != NULL &&
		     strcmp(nconf->nc_protofmly, NC_LOOPBACK) == 0)
			break;
	}
	if (nconf == NULL) {
		warnx("getnetconfig: %s", nc_sperror());
		endnetconfig(localhandle);
		return(DESERR_HWERROR);
	}
	clnt = clnt_tp_create(NULL, CRYPT_PROG, CRYPT_VERS, nconf);
	if (clnt == (CLIENT *) NULL) {
		endnetconfig(localhandle);
		return(DESERR_HWERROR);
	}
	endnetconfig(localhandle);

	des_crypt_1_arg.desbuf.desbuf_len = len;
	des_crypt_1_arg.desbuf.desbuf_val = buf;
	des_crypt_1_arg.des_dir = (dparms->des_dir == ENCRYPT) ? ENCRYPT_DES : DECRYPT_DES;
	des_crypt_1_arg.des_mode = (dparms->des_mode == CBC) ? CBC_DES : ECB_DES;
	bcopy(dparms->des_ivec, des_crypt_1_arg.des_ivec, 8);
	bcopy(dparms->des_key, des_crypt_1_arg.des_key, 8);

	result_1 = des_crypt_1(&des_crypt_1_arg, clnt);
	if (result_1 == (desresp *) NULL) {
		clnt_destroy(clnt);
		return(DESERR_HWERROR);
	}

	stat = result_1->stat;

	if (result_1->stat == DESERR_NONE ||
	    result_1->stat == DESERR_NOHWDEVICE) {
		bcopy(result_1->desbuf.desbuf_val, buf, len);
		bcopy(result_1->des_ivec, dparms->des_ivec, 8);
	}

	clnt_freeres(clnt, (xdrproc_t)xdr_desresp, result_1);
	clnt_destroy(clnt);

	return(stat);
}

int
_des_crypt_call(char *buf, int len, struct desparams *dparms)
{
	return ref__des_crypt_call(buf, len, dparms);
}

/* ===== des_crypt.c ===== */

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

int (*__des_crypt_LOCAL)(char *, unsigned, struct desparams *) = 0;
extern int ref__des_crypt_call(char *, int, struct desparams *);
#define _des_crypt_call ref__des_crypt_call

static int ref_common_crypt(char *, char *, unsigned, unsigned,
    struct desparams *);

/*
 * Copy 8 bytes
 */
#define COPY8(src, dst) { \
	char *a = (char *) dst; \
	char *b = (char *) src; \
	*a++ = *b++; *a++ = *b++; *a++ = *b++; *a++ = *b++; \
	*a++ = *b++; *a++ = *b++; *a++ = *b++; *a++ = *b++; \
}
 
/*
 * Copy multiple of 8 bytes
 */
#define DESCOPY(src, dst, len) { \
	char *a = (char *) dst; \
	char *b = (char *) src; \
	int i; \
	for (i = (int) len; i > 0; i -= 8) { \
		*a++ = *b++; *a++ = *b++; *a++ = *b++; *a++ = *b++; \
		*a++ = *b++; *a++ = *b++; *a++ = *b++; *a++ = *b++; \
	} \
}

/*
 * CBC mode encryption
 */
int
ref_cbc_crypt(char *key, char *buf, unsigned len, unsigned mode, char *ivec)
{
	int err;
	struct desparams dp;

#ifdef BROKEN_DES
	dp.UDES.UDES_buf = buf;
	dp.des_mode = ECB;
#else
	dp.des_mode = CBC;
#endif
	COPY8(ivec, dp.des_ivec);
	err = ref_common_crypt(key, buf, len, mode, &dp);
	COPY8(dp.des_ivec, ivec);
	return(err);
}


/*
 * ECB mode encryption
 */
int
ref_ecb_crypt(char *key, char *buf, unsigned len, unsigned mode)
{
	struct desparams dp;

#ifdef BROKEN_DES
	dp.UDES.UDES_buf = buf;
	dp.des_mode = CBC;
#else
	dp.des_mode = ECB;
#endif
	return(ref_common_crypt(key, buf, len, mode, &dp));
}



/*
 * Common code to cbc_crypt() & ecb_crypt()
 */
static int
ref_common_crypt(char *key, char *buf, unsigned len, unsigned mode,
    struct desparams *desp)
{
	int desdev;

	if ((len % 8) != 0 || len > DES_MAXDATA) {
		return(DESERR_BADPARAM);
	}
	desp->des_dir =
		((mode & DES_DIRMASK) == DES_ENCRYPT) ? ENCRYPT : DECRYPT;

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

#undef _des_crypt_call
