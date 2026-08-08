/*
 * Batch b0196s1 oracle: original HardenedBSD C sources concatenated.
 * Each function renamed with ref_ prefix; bodies otherwise unmodified.
 * Infrastructure (types, mocks, wrappers) added for standalone compilation.
 */

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum clnt_stat { RPC_SUCCESS = 0 };
struct rpc_err { int re_status; };

struct rpc_createerr {
	enum clnt_stat cf_stat;
	struct rpc_err cf_error;
};

typedef pthread_key_t thread_key_t;
typedef pthread_once_t once_t;
#ifndef PTHREAD_ONCE_INITIALIZER
#define PTHREAD_ONCE_INITIALIZER PTHREAD_ONCE_INIT
#endif
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

void
mock_reset_b0196s1(void)
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
	if (p == NULL)
		return -1;
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

extern void *__real_malloc(size_t);

void *
__wrap_malloc(size_t n)
{
	if (mock_malloc_fail)
		return NULL;
	return __real_malloc(n);
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

/* protects the services list (svc.c) */
pthread_rwlock_t	svc_lock = PTHREAD_RWLOCK_INITIALIZER;

/* protects svc_fdset and the xports[] array */
pthread_rwlock_t	svc_fd_lock = PTHREAD_RWLOCK_INITIALIZER;

/* protects the RPCBIND address cache */
pthread_rwlock_t	rpcbaddr_cache_lock = PTHREAD_RWLOCK_INITIALIZER;

/* serializes authdes ops initializations */
pthread_mutex_t authdes_ops_lock = PTHREAD_MUTEX_INITIALIZER;

/* protects des stats list */
pthread_mutex_t svcauthdesstats_lock = PTHREAD_MUTEX_INITIALIZER;

/* auth_none.c serialization */
pthread_mutex_t	authnone_lock = PTHREAD_MUTEX_INITIALIZER;

/* protects the Auths list (svc_auth.c) */
pthread_mutex_t	authsvc_lock = PTHREAD_MUTEX_INITIALIZER;

/* protects client-side fd lock array */
pthread_mutex_t	clnt_fd_lock = PTHREAD_MUTEX_INITIALIZER;

/* clnt_raw.c serialization */
pthread_mutex_t	clntraw_lock = PTHREAD_MUTEX_INITIALIZER;

/* dupreq variables (svc_dg.c) */
pthread_mutex_t	dupreq_lock = PTHREAD_MUTEX_INITIALIZER;

/* loopnconf (rpcb_clnt.c) */
pthread_mutex_t	loopnconf_lock = PTHREAD_MUTEX_INITIALIZER;

/* serializes ops initializations */
pthread_mutex_t	ops_lock = PTHREAD_MUTEX_INITIALIZER;

/* protects proglst list (svc_simple.c) */
pthread_mutex_t	proglst_lock = PTHREAD_MUTEX_INITIALIZER;

/* serializes clnt_com_create() (rpc_soc.c) */
pthread_mutex_t	rpcsoc_lock = PTHREAD_MUTEX_INITIALIZER;

/* svc_raw.c serialization */
pthread_mutex_t	svcraw_lock = PTHREAD_MUTEX_INITIALIZER;

/* xprtlist (svc_generic.c) */
pthread_mutex_t	xprtlist_lock = PTHREAD_MUTEX_INITIALIZER;

#undef	rpc_createerr

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
