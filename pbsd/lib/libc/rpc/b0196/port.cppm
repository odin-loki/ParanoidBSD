/*	$NetBSD: mt_misc.c,v 1.1 2000/06/02 23:11:11 fvdl Exp $	*/

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

/*	$NetBSD: pmap_clnt.c,v 1.16 2000/07/06 03:10:34 christos Exp $	*/

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
 * pmap_clnt.c
 * Client interface to pmap rpc service.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

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

export module pbsd.lib.libc.rpc.b0196;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace pbsd::lib_libc_rpc::b0196 {

using std::size_t;

typedef int bool_t;
inline constexpr bool_t TRUE = 1;
inline constexpr bool_t FALSE = 0;

typedef std::uint32_t rpcprog_t;
typedef std::uint32_t rpcvers_t;
typedef std::uint32_t u_long;
typedef unsigned char u_char;
typedef std::uint32_t u_int32_t;

inline constexpr int IPPROTO_UDP = 17;
inline constexpr int IPPROTO_TCP = 6;

enum clnt_stat { RPC_SUCCESS = 0 };
struct rpc_err { int re_status; };

struct rpc_createerr {
	enum clnt_stat cf_stat;
	struct rpc_err cf_error;
};

struct netbuf {
	unsigned int maxlen;
	unsigned int len;
	void *buf;
};

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

typedef void *CLIENT;
typedef int (*xdrproc_t)(void *, void *, unsigned int);

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

inline constexpr const char *NC_LOOPBACK = "loopback";
inline constexpr rpcprog_t CRYPT_PROG = 600100029U;
inline constexpr rpcvers_t CRYPT_VERS = 1U;

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

typedef std::uintptr_t thread_key_t;
typedef int once_t;
inline constexpr once_t ONCE_INITIALIZER = 0;

extern "C" {
extern int (*__des_crypt_LOCAL)(char *, unsigned, struct desparams *);
extern int _des_crypt_call(char *, int, struct desparams *);
extern struct netconfig *__rpc_getconfip(const char *);
extern void freenetconfigent(struct netconfig *);
extern struct netbuf *uaddr2taddr(const struct netconfig *, const char *);
extern bool_t rpcb_set(rpcprog_t, rpcvers_t, const struct netconfig *,
    const struct netbuf *);
extern bool_t rpcb_unset(rpcprog_t, rpcvers_t, const struct netconfig *);
extern void *setnetconfig(void);
extern struct netconfig *getnetconfig(void *);
extern void endnetconfig(void *);
extern const char *nc_sperror(void);
extern void warnx(const char *, ...);
extern CLIENT *clnt_tp_create(const char *, rpcprog_t, rpcvers_t,
    struct netconfig *);
extern void clnt_destroy(CLIENT *);
extern desresp *des_crypt_1(desargs *, CLIENT *);
extern int clnt_freeres(CLIENT *, xdrproc_t, void *);
extern void bcopy(const void *, void *, size_t);
extern int thr_main(void);
extern int thr_once(once_t *, void (*)(void));
extern int thr_keycreate(thread_key_t *, void (*)(void *));
extern int thr_setspecific(thread_key_t, void *);
extern void *thr_getspecific(thread_key_t);
}

struct rpc_createerr port_rpc_createerr;
static thread_key_t rce_key;
static once_t rce_once = ONCE_INITIALIZER;
static int rce_key_error;

static void
rce_key_init(void)
{

	rce_key_error = thr_keycreate(&rce_key, free);
}

export struct rpc_createerr *
__rpc_createerr(void)
{
	struct rpc_createerr *rce_addr = NULL;

	if (thr_main())
		return (&port_rpc_createerr);
	if (thr_once(&rce_once, rce_key_init) != 0 || rce_key_error != 0)
		return (&port_rpc_createerr);
	rce_addr = (struct rpc_createerr *)thr_getspecific(rce_key);
	if (!rce_addr) {
		rce_addr = (struct rpc_createerr *)
			malloc(sizeof (struct rpc_createerr));
		if (thr_setspecific(rce_key, (void *) rce_addr) != 0) {
			free(rce_addr);
			return (&port_rpc_createerr);
		}
		memset(rce_addr, 0, sizeof (struct rpc_createerr));
		return (rce_addr);
	}
	return (rce_addr);
}

export int
_des_crypt_call(char *buf, int len, struct desparams *dparms)
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

export bool_t
pmap_set(u_long program, u_long version, int protocol, int port)
{
	bool_t rslt;
	struct netbuf *na;
	struct netconfig *nconf;
	char buf[32];

	if ((protocol != IPPROTO_UDP) && (protocol != IPPROTO_TCP)) {
		return (FALSE);
	}
	nconf = __rpc_getconfip(protocol == IPPROTO_UDP ? "udp" : "tcp");
	if (nconf == NULL) {
		return (FALSE);
	}
	snprintf(buf, sizeof buf, "0.0.0.0.%d.%d", 
	    (((u_int32_t)port) >> 8) & 0xff, port & 0xff);
	na = uaddr2taddr(nconf, buf);
	if (na == NULL) {
		freenetconfigent(nconf);
		return (FALSE);
	}
	rslt = rpcb_set((rpcprog_t)program, (rpcvers_t)version, nconf, na);
	free(na);
	freenetconfigent(nconf);
	return (rslt);
}

export bool_t
pmap_unset(u_long program, u_long version)
{
	struct netconfig *nconf;
	bool_t udp_rslt = FALSE;
	bool_t tcp_rslt = FALSE;

	nconf = __rpc_getconfip("udp");
	if (nconf != NULL) {
		udp_rslt = rpcb_unset((rpcprog_t)program, (rpcvers_t)version,
		    nconf);
		freenetconfigent(nconf);
	}
	nconf = __rpc_getconfip("tcp");
	if (nconf != NULL) {
		tcp_rslt = rpcb_unset((rpcprog_t)program, (rpcvers_t)version,
		    nconf);
		freenetconfigent(nconf);
	}
	/*
	 * XXX: The call may still succeed even if only one of the
	 * calls succeeded.  This was the best that could be
	 * done for backward compatibility.
	 */
	return (tcp_rslt || udp_rslt);
}

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

export int
cbc_crypt(char *key, char *buf, unsigned len, unsigned mode, char *ivec)
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
	dp.des_mode = CBC;
#else
	dp.des_mode = ECB;
#endif
	return(common_crypt(key, buf, len, mode, &dp));
}

extern "C" int xdr_desresp(void *, void *, unsigned int);

} /* namespace pbsd::lib_libc_rpc::b0196 */
