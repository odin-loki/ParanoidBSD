/*
 * PBSD batch b0281 -- C++23 port of
 *     lib/libc/resolv/h_errno.c
 *     lib/libc/resolv/mtctxres.c
 *     lib/libc/resolv/res_state.c
 *     lib/libc/resolv/herror.c
 */

module;

#define __bits_pthreadtypes_common_h 1
#define _BITS_PTHREADTYPES_COMMON_H 1

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>

export module pbsd.lib.libc.resolv.b0281;

#define DO_PTHREADS 1

typedef struct {
	int id;
} pthread_key_t;

typedef struct {
	int locked;
} pthread_mutex_t;

extern "C" void *malloc(size_t);
extern "C" void free(void *);
extern "C" void *calloc(size_t, size_t);

#ifndef CLOCK_MONOTONIC_FAST
#define CLOCK_MONOTONIC_FAST CLOCK_MONOTONIC
#endif

#ifndef RES_INIT
#define RES_INIT 0x00000001u
#endif

#ifndef RES_TIMEOUT
#define RES_TIMEOUT 0x00000002u
#endif

#define _PATH_RESCONF "/etc/resolv.conf"

#define nitems(x) (sizeof(x) / sizeof((x)[0]))

#define DE_CONST(x, t) ((t) = (char *)(uintptr_t)(const void *)(x))

using thread_key_t = struct { long opaque; };
using once_t = struct { int state; };

#define ONCE_INITIALIZER {0}
#define PTHREAD_MUTEX_INITIALIZER {0}

export namespace pbsd::lib_libc_resolv::b0281 {

struct __res_state_ext {
	time_t conf_stat;
	struct timespec conf_mtim;
	unsigned int reload_period;
};

struct __res_state_layout {
	int res_h_errno;
	unsigned int options;
	struct {
		struct {
			struct __res_state_ext *ext;
		} _ext;
	} _u;
};

using res_state = __res_state_layout *;

struct mtctxres_t {
	unsigned char opaque[64];
};

extern "C" {
extern int mock_thr_main_ret;
extern int mock_thr_once_ret;
extern int mock_thr_keycreate_ret;
extern int mock_thr_setspecific_ret;
extern res_state mock_thr_getspecific_val;
extern int mock_calloc_fail;
extern int mock_clock_gettime_ret;
extern struct timespec mock_clock_now;
extern int mock_stat_ret;
extern struct stat mock_stat_sb;
extern void *mock_pthread_getspecific_val;
extern int mock_pthread_setspecific_ret;
extern int mock_pthread_key_create_ret;
extern int mock_malloc_fail;
extern int mock_writev_ret;
extern int mock_writev_calls;
extern int mock_writev_last_fd;
extern int mock_writev_last_count;
extern struct iovec mock_writev_last_iov[8];
extern int h_errno;
extern const char *h_errlist[];
extern const int h_nerr;

extern struct __res_state_layout _res;

int thr_main(void);
int thr_once(once_t *, void (*)(void));
int thr_keycreate(thread_key_t *, void (*)(void *));
void *thr_getspecific(thread_key_t);
int thr_setspecific(thread_key_t, void *);
void res_ndestroy(res_state);
int pthread_key_create(pthread_key_t *, void (*)(void *));
void *pthread_getspecific(pthread_key_t);
int pthread_setspecific(pthread_key_t, const void *);
int pthread_mutex_lock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);
int mock_writev(int, const struct iovec *, int);
}

#define _writev mock_writev

int *__h_errno(void);
void __h_errno_set(res_state res, int err);
res_state __res_state(void);

int *
__h_errno(void)
{
	return (&__res_state()->res_h_errno);
}

void
__h_errno_set(res_state res, int err)
{
	h_errno = res->res_h_errno = err;
}

static pthread_key_t	key;
static int		mt_key_initialized = 0;

static int		__res_init_ctx(void);
static void		__res_destroy_ctx(void *);

static mtctxres_t	sharedctx;

#ifdef DO_PTHREADS
static void
_mtctxres_init(void) {
	int pthread_keycreate_ret;

	pthread_keycreate_ret = pthread_key_create(&key, __res_destroy_ctx);
	if (pthread_keycreate_ret == 0)
		mt_key_initialized = 1;
}
#endif

#ifndef _LIBC
int
__res_enable_mt(void) {
	return (-1);
}

int
__res_disable_mt(void) {
	return (0);
}
#endif

#ifdef DO_PTHREADS
static int
__res_init_ctx(void) {

	mtctxres_t	*mt;
	int		ret;


	if (pthread_getspecific(key) != 0) {
		/* Already exists */
		return (0);
	}

	if ((mt = (mtctxres_t *)malloc(sizeof(mtctxres_t))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	memset(mt, 0, sizeof (mtctxres_t));

	if ((ret = pthread_setspecific(key, mt)) != 0) {
		free(mt);
		errno = ret;
		return (-1);
	}

	return (0);
}

static void
__res_destroy_ctx(void *value) {

	free(value);
}
#endif

mtctxres_t *
___mtctxres(void) {
#ifdef DO_PTHREADS
	mtctxres_t	*mt;

	if (!mt_key_initialized) {
		static pthread_mutex_t keylock = PTHREAD_MUTEX_INITIALIZER;
                if (pthread_mutex_lock(&keylock) == 0) {
			_mtctxres_init();
			(void) pthread_mutex_unlock(&keylock);
		}
	}

	if (mt_key_initialized) {
		if (((mt = (mtctxres_t *)pthread_getspecific(key)) != NULL) ||
		    (__res_init_ctx() == 0 &&
		     (mt = (mtctxres_t *)pthread_getspecific(key)) != NULL)) {
			return (mt);
		}
	}
#endif
	return (&sharedctx);
}

static thread_key_t res_key;
static once_t res_init_once = ONCE_INITIALIZER;
static int res_thr_keycreated = 0;

static void
free_res(void *ptr)
{
	res_state statp = (res_state)ptr;

	if (statp->_u._ext.ext != NULL)
		res_ndestroy(statp);
	free(statp);
}

static void
res_keycreate(void)
{
	res_thr_keycreated = thr_keycreate(&res_key, free_res) == 0;
}

static res_state
res_check_reload(res_state statp)
{
	struct timespec now;
	struct stat sb;
	struct __res_state_ext *ext;

	if ((statp->options & RES_INIT) == 0) {
		return (statp);
	}

	ext = statp->_u._ext.ext;
	if (ext == NULL || ext->reload_period == 0) {
		return (statp);
	}

	if (clock_gettime(CLOCK_MONOTONIC_FAST, &now) != 0 ||
	    (now.tv_sec - ext->conf_stat) < ext->reload_period) {
		return (statp);
	}

	ext->conf_stat = now.tv_sec;
	if (stat(_PATH_RESCONF, &sb) == 0 &&
	    (sb.st_mtim.tv_sec  != ext->conf_mtim.tv_sec ||
	     sb.st_mtim.tv_nsec != ext->conf_mtim.tv_nsec)) {
		statp->options &= ~RES_INIT;
	}

	return (statp);
}

res_state
__res_state(void)
{
	res_state statp;

	if (thr_main() != 0)
		return res_check_reload(&_res);

	if (thr_once(&res_init_once, res_keycreate) != 0 ||
	    !res_thr_keycreated)
		return (&_res);

	statp = (res_state)thr_getspecific(res_key);
	if (statp != NULL)
		return res_check_reload(statp);
	statp = (res_state)calloc(1, sizeof(*statp));
	if (statp == NULL)
		return (&_res);
#ifdef __BIND_RES_TEXT
	statp->options = RES_TIMEOUT;
#endif
	if (thr_setspecific(res_key, statp) == 0)
		return (statp);
	free(statp);
	return (&_res);
}

const char *
hstrerror(int err) {
	if (err < 0)
		return ("Resolver internal error");
	else if (err < h_nerr)
		return (h_errlist[err]);
	return ("Unknown resolver error");
}

void
herror(const char *s) {
	struct iovec iov[4], *v = iov;
	char *t;

	if (s != NULL && *s != '\0') {
		DE_CONST(s, t);
		v->iov_base = t;
		v->iov_len = strlen(t);
		v++;
		DE_CONST(": ", t);
		v->iov_base = t;
		v->iov_len = 2;
		v++;
	}
	DE_CONST(hstrerror(*__h_errno()), t);
	v->iov_base = t;
	v->iov_len = strlen((const char *)v->iov_base);
	v++;
	DE_CONST("\n", t);
	v->iov_base = t;
	v->iov_len = 1;
	_writev(STDERR_FILENO, iov, (v - iov) + 1);
}

} /* namespace */
