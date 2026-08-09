/*	$NetBSD: mt_misc.c,v 1.1 2000/06/02 23:11:11 fvdl Exp $	*/

/*
 * PBSD batch b0196s1 oracle.
 *
 * Source: hbsd/src/lib/libc/rpc/mt_misc.c
 *
 * The function bodies below are the originals, unmodified.  Only the
 * externally visible function names carry a ref_ prefix (applied through a
 * #define for the static helper so that no body text changes), and the
 * declarations that libc's private "namespace.h" / "reentrant.h" /
 * "mt_misc.h" and <rpc/rpc.h> would have supplied are spelled out here so the
 * translation unit is self contained.
 */

#define _GNU_SOURCE

/* #include "namespace.h"	-- libc internal, not available here */
/* #include "reentrant.h"	-- substituted below */
/* #include <rpc/rpc.h>		-- substituted below */
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
/* #include "un-namespace.h"	-- libc internal, not available here */
/* #include "mt_misc.h"		-- declarations only */

#include <pthread.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

/* ---- from <rpc/clnt_stat.h> ------------------------------------------- */
enum clnt_stat {
	RPC_SUCCESS = 0,
	RPC_CANTENCODEARGS = 1,
	RPC_CANTDECODERES = 2,
	RPC_CANTSEND = 3,
	RPC_CANTRECV = 4,
	RPC_TIMEDOUT = 5,
	RPC_INTR = 18,
	RPC_UDERROR = 23,
	RPC_VERSMISMATCH = 6,
	RPC_AUTHERROR = 7,
	RPC_PROGUNAVAIL = 8,
	RPC_PROGVERSMISMATCH = 9,
	RPC_PROCUNAVAIL = 10,
	RPC_CANTDECODEARGS = 11,
	RPC_SYSTEMERROR = 12,
	RPC_UNKNOWNHOST = 13,
	RPC_UNKNOWNPROTO = 17,
	RPC_UNKNOWNADDR = 19,
	RPC_NOBROADCAST = 21,
	RPC_RPCBFAILURE = 14,
	RPC_PROGNOTREGISTERED = 15,
	RPC_N2AXLATEFAILURE = 22,
	RPC_TLIERROR = 20,
	RPC_FAILED = 16,
	RPC_INPROGRESS = 24,
	RPC_STALERACHANDLE = 25,
	RPC_CANTCONNECT = 26,
	RPC_XPRTFAILED = 27,
	RPC_CANTCREATESTREAM = 28
};

/* ---- from <rpc/auth.h> ------------------------------------------------ */
enum auth_stat {
	AUTH_OK = 0,
	AUTH_BADCRED = 1,
	AUTH_REJECTEDCRED = 2,
	AUTH_BADVERF = 3,
	AUTH_REJECTEDVERF = 4,
	AUTH_TOOWEAK = 5,
	AUTH_INVALIDRESP = 6,
	AUTH_FAILED = 7,
	AUTH_KERB_GENERIC = 8,
	AUTH_TIMEEXPIRE = 9,
	AUTH_TKT_FILE = 10,
	AUTH_DECODE = 11,
	AUTH_NET_ADDR = 12,
	RPCSEC_GSS_CREDPROBLEM = 13,
	RPCSEC_GSS_CTXPROBLEM = 14,
	RPCSEC_GSS_NODISPATCH = 15
};

/* ---- from <rpc/clnt.h> ------------------------------------------------ */
typedef uint32_t rpcvers_t;

struct rpc_err {
	enum clnt_stat re_status;
	union {
		int RE_errno;
		enum auth_stat RE_why;
		struct {
			rpcvers_t low;
			rpcvers_t high;
		} RE_vers;
		struct {
			int32_t s1;
			int32_t s2;
		} RE_lb;
	} ru;
};

struct rpc_createerr {
	enum clnt_stat cf_stat;
	struct rpc_err cf_error;
};

/* ---- from "reentrant.h" ----------------------------------------------- */
typedef pthread_key_t	thread_key_t;
typedef pthread_once_t	once_t;

#define	ONCE_INITIALIZER		PTHREAD_ONCE_INIT
#define	thr_keycreate(k, d)		pthread_key_create(k, d)
#define	thr_getspecific(k)		pthread_getspecific(k)
#define	thr_setspecific(k, p)		pthread_setspecific(k, p)
#define	thr_once(o, f)			pthread_once(o, f)
#define	thr_main()			(syscall(SYS_gettid) == (long)getpid())

/* ---- ref_ renaming of the static helper (leaves body text intact) ----- */
#define	rce_key_init			ref_rce_key_init

/* ======================================================================= */

/* Take these objects out of the application namespace. */
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

struct rpc_createerr rpc_createerr;
static thread_key_t rce_key;
static once_t rce_once = ONCE_INITIALIZER;
static int rce_key_error;

static void
rce_key_init(void)
{

	rce_key_error = thr_keycreate(&rce_key, free);
}

struct rpc_createerr *
ref___rpc_createerr(void)
{
	struct rpc_createerr *rce_addr = NULL;

	if (thr_main())
		return (&rpc_createerr);
	if (thr_once(&rce_once, rce_key_init) != 0 || rce_key_error != 0)
		return (&rpc_createerr);
	rce_addr = (struct rpc_createerr *)thr_getspecific(rce_key);
	if (!rce_addr) {
		rce_addr = (struct rpc_createerr *)
			malloc(sizeof (struct rpc_createerr));
		if (thr_setspecific(rce_key, (void *) rce_addr) != 0) {
			free(rce_addr);
			return (&rpc_createerr);
		}
		memset(rce_addr, 0, sizeof (struct rpc_createerr));
		return (rce_addr);
	}
	return (rce_addr);
}
