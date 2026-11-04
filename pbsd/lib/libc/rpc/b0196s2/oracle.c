/*
 * Batch b0196s2 oracle: original HardenedBSD C sources concatenated.
 * Each function renamed with ref_ prefix; bodies otherwise unmodified.
 * Infrastructure (types, mocks, wrappers) added for standalone compilation.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- RPC / DES types and constants (standalone stubs) ---- */

#define NC_LOOPBACK		"loopback"
#define CRYPT_PROG		100024
#define CRYPT_VERS		1

#define DESERR_NONE		0
#define DESERR_NOKEY		1
#define DESERR_HWERROR		2
#define DESERR_BADMODE		3
#define DESERR_NOHWDEVICE	4

#define ENCRYPT			0
#define DECRYPT			1
#define CBC			0
#define ECB			1

#define ENCRYPT_DES		0
#define DECRYPT_DES		1
#define CBC_DES			0
#define ECB_DES			1

typedef struct CLIENT CLIENT;
typedef int (*xdrproc_t)(void *, void *);

struct CLIENT {
	int _dummy;
};

struct netconfig {
	char *nc_protofmly;
};

struct desparams {
	int des_dir;
	int des_mode;
	char des_ivec[8];
	char des_key[8];
};

struct desargs {
	struct {
		unsigned desbuf_len;
		char *desbuf_val;
	} desbuf;
	int des_dir;
	int des_mode;
	char des_ivec[8];
	char des_key[8];
};

struct desresp {
	int stat;
	struct {
		unsigned desbuf_len;
		char *desbuf_val;
	} desbuf;
	char des_ivec[8];
};

typedef struct desargs desargs;
typedef struct desresp desresp;

int
xdr_desresp(void *xdrs, struct desresp *objp)
{
	(void)xdrs;
	(void)objp;
	return 1;
}

static void
bcopy(const void *src, void *dst, size_t len)
{
	memcpy(dst, src, len);
}

static void
warnx(const char *fmt, ...)
{
	(void)fmt;
}

/* ---- mock control (harness-visible) ---- */

int mock_nc_always_null = 0;
int mock_nc_nonloop_count = 0;
int mock_nc_null_protofmly = 0;
char mock_nc_protofmly_other[16] = "inet";
int mock_clnt_create_fail = 0;
int mock_des_crypt_fail = 0;
int mock_des_crypt_stat = DESERR_NONE;

static int nc_call_idx;
static struct netconfig mock_nc_other;
static struct netconfig mock_nc_loopback;
static CLIENT mock_clnt;
static struct desresp mock_desresp;
static char mock_desbuf[256];

void
mock_reset_b0196s2(void)
{
	mock_nc_always_null = 0;
	mock_nc_nonloop_count = 0;
	mock_nc_null_protofmly = 0;
	strcpy(mock_nc_protofmly_other, "inet");
	mock_clnt_create_fail = 0;
	mock_des_crypt_fail = 0;
	mock_des_crypt_stat = DESERR_NONE;
	nc_call_idx = 0;
	memset(&mock_nc_other, 0, sizeof(mock_nc_other));
	memset(&mock_nc_loopback, 0, sizeof(mock_nc_loopback));
	memset(&mock_desresp, 0, sizeof(mock_desresp));
	memset(mock_desbuf, 0, sizeof(mock_desbuf));
}

const char *
nc_sperror(void)
{
	return "mock nc error";
}

void *
setnetconfig(void)
{
	return (void *)1;
}

struct netconfig *
getnetconfig(void *localhandle)
{
	(void)localhandle;

	if (mock_nc_always_null)
		return NULL;
	if (nc_call_idx < mock_nc_nonloop_count) {
		nc_call_idx++;
		if (mock_nc_null_protofmly)
			mock_nc_other.nc_protofmly = NULL;
		else
			mock_nc_other.nc_protofmly = mock_nc_protofmly_other;
		return &mock_nc_other;
	}
	nc_call_idx++;
	mock_nc_loopback.nc_protofmly = NC_LOOPBACK;
	return &mock_nc_loopback;
}

void
endnetconfig(void *localhandle)
{
	(void)localhandle;
}

CLIENT *
clnt_tp_create(void *addr, unsigned prog, unsigned vers, struct netconfig *nconf)
{
	(void)addr;
	(void)prog;
	(void)vers;
	(void)nconf;

	if (mock_clnt_create_fail)
		return (CLIENT *)NULL;
	return &mock_clnt;
}

struct desresp *
des_crypt_1(struct desargs *arg, CLIENT *clnt)
{
	unsigned i;
	(void)clnt;

	if (mock_des_crypt_fail)
		return (struct desresp *)NULL;

	mock_desresp.stat = mock_des_crypt_stat;
	for (i = 0; i < arg->desbuf.desbuf_len; i++) {
		mock_desbuf[i] = (char)(arg->desbuf.desbuf_val[i] +
		    arg->des_dir + arg->des_mode + i);
	}
	for (i = 0; i < 8; i++)
		mock_desresp.des_ivec[i] = (char)(arg->des_ivec[i] ^
		    arg->des_key[i] ^ (char)i);
	mock_desresp.desbuf.desbuf_val = mock_desbuf;
	return &mock_desresp;
}

void
clnt_freeres(CLIENT *clnt, xdrproc_t xdr_func, void *res)
{
	(void)clnt;
	(void)xdr_func;
	(void)res;
}

void
clnt_destroy(CLIENT *clnt)
{
	(void)clnt;
}

/* ===== crypt_client.c ===== */

/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 1996
 *	Bill Paul <wpaul@ctr.columbia.edu>.  All rights reserved.
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
 *	This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

int
ref_des_crypt_call(char *buf, int len, struct desparams *dparms)
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
