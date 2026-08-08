/*
 * Batch b0196 oracle: original HardenedBSD C sources concatenated.
 * Each function renamed with ref_ prefix; bodies otherwise unmodified.
 * Infrastructure (types, mocks, wrappers) added for standalone compilation.
 */

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef IPPROTO_UDP
#define IPPROTO_UDP 17
#endif
#ifndef IPPROTO_TCP
#define IPPROTO_TCP 6
#endif

typedef int bool_t;
#define TRUE 1
#define FALSE 0

typedef uint32_t rpcprog_t;
typedef uint32_t rpcvers_t;
typedef uint32_t u_long;
typedef unsigned char u_char;

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

typedef pthread_key_t thread_key_t;
typedef pthread_once_t once_t;
#define ONCE_INITIALIZER PTHREAD_ONCE_INITIALIZER

/* ---- mock control (harness-visible) ---- */

int mock_thr_main_result = 0;
int mock_thr_once_result = 0;
int mock_thr_keycreate_result = 0;
int mock_thr_setspecific_result = 0;
void *mock_thr_getspecific_result = NULL;
int mock_thr_getspecific_null = 0;
int mock_thr_once_invoked = 0;
int mock_malloc_fail = 0;
int mock_thr_setspecific_fail_on = 0;

int mock_getconfip_udp_null = 0;
int mock_getconfip_tcp_null = 0;
int mock_uaddr2taddr_null = 0;
bool_t mock_rpcb_set_result = TRUE;
bool_t mock_rpcb_unset_udp_result = FALSE;
bool_t mock_rpcb_unset_tcp_result = FALSE;

int mock_crypt_no_loopback = 0;
int mock_crypt_no_clnt = 0;
int mock_crypt_null_result = 0;
int mock_crypt_result_stat = DESERR_NONE;
int mock_crypt_copy_buf = 1;
char mock_crypt_result_ivec[8];

int mock_des_local_return = 1;
int mock_des_local_xor = 0x55;

void
mock_reset_b0196(void)
{
	mock_thr_main_result = 0;
	mock_thr_once_result = 0;
	mock_thr_keycreate_result = 0;
	mock_thr_setspecific_result = 0;
	mock_thr_getspecific_result = NULL;
	mock_thr_getspecific_null = 0;
	mock_thr_once_invoked = 0;
	mock_malloc_fail = 0;
	mock_thr_setspecific_fail_on = 0;

	mock_getconfip_udp_null = 0;
	mock_getconfip_tcp_null = 0;
	mock_uaddr2taddr_null = 0;
	mock_rpcb_set_result = TRUE;
	mock_rpcb_unset_udp_result = FALSE;
	mock_rpcb_unset_tcp_result = FALSE;

	mock_crypt_no_loopback = 0;
	mock_crypt_no_clnt = 0;
	mock_crypt_null_result = 0;
	mock_crypt_result_stat = DESERR_NONE;
	mock_crypt_copy_buf = 1;
	memset(mock_crypt_result_ivec, 0x42, 8);

	mock_des_local_return = 1;
	mock_des_local_xor = 0x55;
}

int
thr_main(void)
{
	return mock_thr_main_result;
}

int
thr_once(once_t *o, void (*init)(void))
{
	(void)o;
	if (mock_thr_once_result != 0)
		return mock_thr_once_result;
	if (!mock_thr_once_invoked) {
		mock_thr_once_invoked = 1;
		if (init != NULL)
			init();
	}
	return 0;
}

int
thr_keycreate(thread_key_t *k, void (*d)(void *))
{
	(void)d;
	if (mock_thr_keycreate_result != 0)
		return mock_thr_keycreate_result;
	*k = (thread_key_t)1;
	return 0;
}

int
thr_setspecific(thread_key_t k, void *p)
{
	(void)k;
	if (mock_thr_setspecific_fail_on)
		return -1;
	if (mock_thr_setspecific_result != 0)
		return mock_thr_setspecific_result;
	mock_thr_getspecific_result = p;
	mock_thr_getspecific_null = 0;
	return 0;
}

void *
thr_getspecific(thread_key_t k)
{
	(void)k;
	if (mock_thr_getspecific_null)
		return NULL;
	return mock_thr_getspecific_result;
}

void *
malloc(size_t n)
{
	if (mock_malloc_fail)
		return NULL;
	return __real_malloc(n);
}

extern void *__real_malloc(size_t);

struct netconfig *
__rpc_getconfip(const char *proto)
{
	static struct netconfig udp_nc;
	static struct netconfig tcp_nc;

	if (strcmp(proto, "udp") == 0) {
		if (mock_getconfip_udp_null)
			return NULL;
		udp_nc.nc_protofmly = NC_LOOPBACK;
		udp_nc.nc_proto = "udp";
		return &udp_nc;
	}
	if (strcmp(proto, "tcp") == 0) {
		if (mock_getconfip_tcp_null)
			return NULL;
		tcp_nc.nc_protofmly = NC_LOOPBACK;
		tcp_nc.nc_proto = "tcp";
		return &tcp_nc;
	}
	return NULL;
}

void
freenetconfigent(struct netconfig *nc)
{
	(void)nc;
}

struct netbuf *
uaddr2taddr(const struct netconfig *nc, const char *addr)
{
	static struct netbuf nb;
	(void)nc;
	(void)addr;
	if (mock_uaddr2taddr_null)
		return NULL;
	nb.maxlen = 16;
	nb.len = 4;
	nb.buf = (void *)0x1234;
	return &nb;
}

bool_t
rpcb_set(rpcprog_t program, rpcvers_t version,
    const struct netconfig *nconf, const struct netbuf *na)
{
	(void)program;
	(void)version;
	(void)nconf;
	(void)na;
	return mock_rpcb_set_result;
}

bool_t
rpcb_unset(rpcprog_t program, rpcvers_t version,
    const struct netconfig *nconf)
{
	(void)program;
	(void)version;
	if (nconf != NULL && nconf->nc_proto != NULL &&
	    strcmp(nconf->nc_proto, "udp") == 0)
		return mock_rpcb_unset_udp_result;
	if (nconf != NULL && nconf->nc_proto != NULL &&
	    strcmp(nconf->nc_proto, "tcp") == 0)
		return mock_rpcb_unset_tcp_result;
	return FALSE;
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
	static int returned = 0;

	(void)handle;
	if (mock_crypt_no_loopback)
		return NULL;
	if (returned) {
		returned = 0;
		return NULL;
	}
	returned = 1;
	loop_nc.nc_protofmly = NC_LOOPBACK;
	loop_nc.nc_proto = "loopback";
	return &loop_nc;
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

/* ===== mt_misc.c ===== */

/*	$NetBSD: mt_misc.c,v 1.1 2000/06/02 23:11:11 fvdl Exp $	*/

#define	svc_lock		__svc_lock
#define	svc_fd_lock		__svc_fd_lock
#define	rpcbaddr_cache_lock	__rpcbaddr_cache_lock
#define	authdes_ops_lock	__authdes_ops_lock
#define	authnone_lock		__authnone_lock
#define	authsvc_lock		__authsvc_lock
#define	clnt_fd_lock		__clnt_fd_lock
#define	clntraw_lock		__clntraw_lock
#define	dupreq_lock		__dupreq_lock
#define	loopnconf_lock		__loopnconf_lock
#define	ops_lock		__ops_lock
#define	proglst_lock		__proglst_lock
#define	rpcsoc_lock		__rpcsoc_lock
#define	svcraw_lock		__svcraw_lock
#define	xprtlist_lock		__xprtlist_lock

pthread_rwlock_t	svc_lock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t	svc_fd_lock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t	rpcbaddr_cache_lock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t authdes_ops_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t svcauthdesstats_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	authnone_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	authsvc_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	clnt_fd_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	clntraw_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	dupreq_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	loopnconf_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	ops_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	proglst_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	rpcsoc_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	svcraw_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	xprtlist_lock = PTHREAD_MUTEX_INITIALIZER;

struct rpc_createerr ref_rpc_createerr;
static thread_key_t rce_key;
static once_t rce_once = ONCE_INITIALIZER;
static int rce_key_error;

static void
ref_rce_key_init(void)
{

	rce_key_error = thr_keycreate(&rce_key, free);
}

struct rpc_createerr *
ref___rpc_createerr(void)
{
	struct rpc_createerr *rce_addr = NULL;

	if (thr_main())
		return (&ref_rpc_createerr);
	if (thr_once(&rce_once, ref_rce_key_init) != 0 || rce_key_error != 0)
		return (&ref_rpc_createerr);
	rce_addr = (struct rpc_createerr *)thr_getspecific(rce_key);
	if (!rce_addr) {
		rce_addr = (struct rpc_createerr *)
			malloc(sizeof (struct rpc_createerr));
		if (thr_setspecific(rce_key, (void *) rce_addr) != 0) {
			free(rce_addr);
			return (&ref_rpc_createerr);
		}
		memset(rce_addr, 0, sizeof (struct rpc_createerr));
		return (rce_addr);
	}
	return (rce_addr);
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

/* ===== pmap_clnt.c ===== */

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

bool_t
ref_pmap_set(u_long program, u_long version, int protocol, int port)
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

bool_t
ref_pmap_unset(u_long program, u_long version)
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

int
_des_crypt_call(char *buf, int len, struct desparams *dparms)
{
	return ref__des_crypt_call(buf, len, dparms);
}

int
xdr_desresp(void *xdrs, void *objp, unsigned int opt)
{
	(void)xdrs;
	(void)objp;
	(void)opt;
	return 1;
}
