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

module;
#include <stddef.h>
#include <string.h>

struct netconfig {
	char *nc_protofmly;
};

struct desparams {
	int des_dir;
	int des_mode;
	char des_ivec[8];
	char des_key[8];
};

struct CLIENT {
	int _dummy;
};

typedef struct CLIENT CLIENT;
typedef int (*xdrproc_t)(void *, void *);

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

extern "C" {
extern void *setnetconfig(void);
extern struct netconfig *getnetconfig(void *);
extern void endnetconfig(void *);
extern const char *nc_sperror(void);
extern CLIENT *clnt_tp_create(void *, unsigned, unsigned,
    struct netconfig *);
extern struct desresp *des_crypt_1(struct desargs *, CLIENT *);
extern void clnt_freeres(CLIENT *, xdrproc_t, void *);
extern void clnt_destroy(CLIENT *);
extern int xdr_desresp(void *, struct desresp *);
}

export module pbsd.lib.libc.rpc.b0196s2;

namespace pbsd::lib_libc_rpc::b0196s2 {

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

export using desparams = ::desparams;

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

export int
_des_crypt_call(char *buf, int len, struct ::desparams *dparms)
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
		     strcmp(nconf->nc_protofmly, NC_LOOPBACK) != 0)
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

} /* namespace pbsd::lib_libc_rpc::b0196s2 */
