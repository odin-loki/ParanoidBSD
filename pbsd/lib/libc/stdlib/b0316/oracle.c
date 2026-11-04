/*
 * oracle.c -- reference implementation for PBSD batch b0316.
 *
 * The original HardenedBSD C sources of the batch, concatenated, with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 * Stubs and macros route libc-internal calls to observable test hooks.
 */

#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

typedef void (*constraint_handler_t)(const char *msg, void *ptr, errno_t error);

/* ------------------------------------------------------------------ */
/* observable stubs (specification for side effects)		      */
/* ------------------------------------------------------------------ */

enum stub_op {
	STUB_OP_CLEANUP,
	STUB_OP_ONCE,
	STUB_OP_MUTEX_LOCK,
	STUB_OP_MUTEX_UNLOCK,
	STUB_OP_CXA_THREAD_DTORS,
	STUB_OP_CXA_FINALIZE,
	STUB_OP_EXIT,
	STUB_OP_SIGPROCMASK,
	STUB_OP_RAISE,
	STUB_OP_SIGACTION,
	STUB_OP_WRITE,
	STUB_OP_MUTEX_INIT,
	STUB_OP_MUTEXATTR_INIT,
	STUB_OP_MUTEXATTR_SETTYPE,
	STUB_OP_MUTEXATTR_DESTROY,
};

struct stub_trace_entry {
	enum stub_op op;
	int a;
	int b;
};

#define STUB_TRACE_MAX 64

static struct stub_trace_entry stub_trace[STUB_TRACE_MAX];
static int stub_trace_len;

static unsigned char stub_write_buf[512];
static size_t stub_write_len;

static int stub_malloc_fail;

void
stub_reset(void)
{
	stub_trace_len = 0;
	stub_write_len = 0;
	memset(stub_write_buf, 0, sizeof(stub_write_buf));
	stub_malloc_fail = 0;
}

void
stub_set_malloc_fail(int fail)
{
	stub_malloc_fail = fail;
}

size_t
stub_trace_count(void)
{
	return (size_t)stub_trace_len;
}

const struct stub_trace_entry *
stub_trace_get(void)
{
	return (stub_trace);
}

const unsigned char *
stub_write_data(void)
{
	return (stub_write_buf);
}

size_t
stub_write_length(void)
{
	return (stub_write_len);
}

static void
stub_push(enum stub_op op, int a, int b)
{
	if (stub_trace_len < STUB_TRACE_MAX)
		stub_trace[stub_trace_len++] = (struct stub_trace_entry){ op, a, b };
}

void *
stub_malloc(size_t sz)
{
	if (stub_malloc_fail) {
		stub_malloc_fail = 0;
		return (NULL);
	}
	return (malloc(sz));
}

void
stub_free(void *p)
{
	free(p);
}

ssize_t
stub__write(int fd, const void *buf, size_t n)
{
	size_t room;

	stub_push(STUB_OP_WRITE, fd, (int)n);
	room = sizeof(stub_write_buf) - stub_write_len;
	if (n > room)
		n = room;
	if (n > 0) {
		memcpy(stub_write_buf + stub_write_len, buf, n);
		stub_write_len += n;
	}
	return ((ssize_t)n);
}

int
stub___libc_sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	(void)set;
	(void)oset;
	stub_push(STUB_OP_SIGPROCMASK, how, 0);
	return (0);
}

int
stub___libc_sigaction(int sig, const struct sigaction *act,
    struct sigaction *oact)
{
	(void)act;
	(void)oact;
	stub_push(STUB_OP_SIGACTION, sig, 0);
	return (0);
}

int
stub_raise(int sig)
{
	stub_push(STUB_OP_RAISE, sig, 0);
	return (0);
}

void
stub__exit(int status)
{
	stub_push(STUB_OP_EXIT, status, 0);
}

void
stub___cxa_thread_call_dtors(void)
{
	stub_push(STUB_OP_CXA_THREAD_DTORS, 0, 0);
}

void
stub___cxa_finalize(void *d)
{
	stub_push(STUB_OP_CXA_FINALIZE, (int)(intptr_t)d, 0);
}

void
stub__once(pthread_once_t *control, void (*init)(void))
{
	stub_push(STUB_OP_ONCE, 0, 0);
	if (control != NULL && *control == PTHREAD_ONCE_INIT) {
		init();
		*control = (pthread_once_t)1;
	}
}

int
stub__pthread_mutex_lock(pthread_mutex_t *m)
{
	(void)m;
	stub_push(STUB_OP_MUTEX_LOCK, 0, 0);
	return (0);
}

int
stub__pthread_mutex_unlock(pthread_mutex_t *m)
{
	(void)m;
	stub_push(STUB_OP_MUTEX_UNLOCK, 0, 0);
	return (0);
}

int
stub__pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *a)
{
	(void)m;
	(void)a;
	stub_push(STUB_OP_MUTEX_INIT, 0, 0);
	return (0);
}

int
stub__pthread_mutexattr_init(pthread_mutexattr_t *a)
{
	(void)a;
	stub_push(STUB_OP_MUTEXATTR_INIT, 0, 0);
	return (0);
}

int
stub__pthread_mutexattr_settype(pthread_mutexattr_t *a, int t)
{
	(void)a;
	stub_push(STUB_OP_MUTEXATTR_SETTYPE, t, 0);
	return (0);
}

int
stub__pthread_mutexattr_destroy(pthread_mutexattr_t *a)
{
	(void)a;
	stub_push(STUB_OP_MUTEXATTR_DESTROY, 0, 0);
	return (0);
}

#define	__libc_sigprocmask	stub___libc_sigprocmask
#define	__libc_sigaction	stub___libc_sigaction
#define	raise			stub_raise
#define	_exit			stub__exit
#define	__cxa_thread_call_dtors	stub___cxa_thread_call_dtors
#define	__cxa_finalize		stub___cxa_finalize
#define	_once			stub__once
#define	_pthread_mutex_lock	stub__pthread_mutex_lock
#define	_pthread_mutex_unlock	stub__pthread_mutex_unlock
#define	_pthread_mutex_init	stub__pthread_mutex_init
#define	_pthread_mutexattr_init	stub__pthread_mutexattr_init
#define	_pthread_mutexattr_settype	stub__pthread_mutexattr_settype
#define	_pthread_mutexattr_destroy	stub__pthread_mutexattr_destroy
#define	_write			stub__write
#define	malloc			stub_malloc
#define	free			stub_free

void (*__cleanup)(void);
int __isthreaded = 0;
int _thread_autoinit_dummy_decl;

void ref_exit(int);
void ref_abort(void);

#define	exit	ref_exit
#define	abort	ref_abort

/*
 * ===== hbsd/src/lib/libc/stdlib/set_constraint_handler_s.c =====
 */

static constraint_handler_t *_ch = NULL;
static pthread_mutex_t ch_lock = PTHREAD_MUTEX_INITIALIZER;

constraint_handler_t
ref_set_constraint_handler_s(constraint_handler_t handler)
{
	constraint_handler_t *new, *old, ret;

	new = malloc(sizeof(constraint_handler_t));
	if (new == NULL)
		return (NULL);
	*new = handler;
	if (__isthreaded)
		_pthread_mutex_lock(&ch_lock);
	old = _ch;
	_ch = new;
	if (__isthreaded)
		_pthread_mutex_unlock(&ch_lock);
	if (old == NULL) {
		ret = NULL;
	} else {
		ret = *old;
		free(old);
	}
	return (ret);
}

void
ref___throw_constraint_handler_s(const char * restrict msg, errno_t error)
{
	constraint_handler_t ch;

	if (__isthreaded)
		_pthread_mutex_lock(&ch_lock);
	ch = _ch != NULL ? *_ch : NULL;
	if (__isthreaded)
		_pthread_mutex_unlock(&ch_lock);
	if (ch != NULL)
		ch(msg, NULL, error);
}

void
ref_abort_handler_s(const char * restrict msg, void * restrict ptr __unused,
    errno_t error __unused)
{
	static const char ahs[] = "abort_handler_s : ";

	(void) _write(STDERR_FILENO, ahs, sizeof(ahs) - 1);
	(void) _write(STDERR_FILENO, msg, strlen(msg));
	(void) _write(STDERR_FILENO, "\n", 1);
	abort();
}

void
ref_ignore_handler_s(const char * restrict msg __unused,
    void * restrict ptr __unused, errno_t error __unused)
{
}

/*
 * ===== hbsd/src/lib/libc/stdlib/exit.c =====
 */

static pthread_mutex_t exit_mutex;
static pthread_once_t exit_mutex_once = PTHREAD_ONCE_INIT;

static void
exit_mutex_init_once(void)
{
	pthread_mutexattr_t ma;

	_pthread_mutexattr_init(&ma);
	_pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
	_pthread_mutex_init(&exit_mutex, &ma);
	_pthread_mutexattr_destroy(&ma);
}

void
ref_exit(int status)
{
	/* Ensure that the auto-initialization routine is linked in: */
	extern int _thread_autoinit_dummy_decl;

	_thread_autoinit_dummy_decl = 1;

	/* Make exit(3) thread-safe */
	if (__isthreaded) {
		_once(&exit_mutex_once, exit_mutex_init_once);
		_pthread_mutex_lock(&exit_mutex);
	}

	/*
	 * We're dealing with cleaning up thread_local destructors in the case of
	 * the process termination through main() exit.
	 * Other cases are handled elsewhere.
	 */
	__cxa_thread_call_dtors();
	__cxa_finalize(NULL);
	if (__cleanup)
		(*__cleanup)();
	_exit(status);
}

/*
 * ===== hbsd/src/lib/libc/stdlib/abort.c =====
 */

void
ref_abort(void)
{
	struct sigaction act;

	/*
	 * POSIX requires we flush stdio buffers on abort.
	 * XXX ISO C requires that abort() be async-signal-safe.
	 */
	if (__cleanup)
		(*__cleanup)();

	sigfillset(&act.sa_mask);
	/*
	 * Don't block SIGABRT to give any handler a chance; we ignore
	 * any errors -- ISO C doesn't allow abort to return anyway.
	 */
	sigdelset(&act.sa_mask, SIGABRT);
	(void)__libc_sigprocmask(SIG_SETMASK, &act.sa_mask, NULL);
	(void)raise(SIGABRT);

	/*
	 * If SIGABRT was ignored, or caught and the handler returns, do
	 * it again, only harder.
	 */
	act.sa_handler = SIG_DFL;
	act.sa_flags = 0;
	sigfillset(&act.sa_mask);
	(void)__libc_sigaction(SIGABRT, &act, NULL);
	sigdelset(&act.sa_mask, SIGABRT);
	(void)__libc_sigprocmask(SIG_SETMASK, &act.sa_mask, NULL);
	(void)raise(SIGABRT);
	exit(1);
}
