// Differential test for PBSD batch b0118s4 (fdopen.c).
//
// Every function of the batch is executed twice on identical inputs: once
// through the reference (ref_*, compiled from the unmodified C bodies in
// oracle.c) and once through the C++23 port.  Return values, errno, the
// fcntl call trace and the whole mock FILE state are compared.

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <climits>
#include <csignal>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0118s4;

namespace P = pbsd::lib_libc_stdio::b0118s4;

extern "C" {
pb_file_t *ref_fdopen(int, const char *);
void pb_fcntl_reset(void);

extern int pb_fcntl_getfl_ret;
extern int pb_fcntl_getfd_ret;
extern int pb_fcntl_setfd_ret;
extern int pb_fcntl_errno;
extern int pb_fcntl_log_n;
extern int pb_fcntl_log_fd[PB_FCNTL_LOG];
extern int pb_fcntl_log_cmd[PB_FCNTL_LOG];
extern int pb_fcntl_log_arg[PB_FCNTL_LOG];
extern int pb_sfp_fail;
extern pb_file_t *pb_sfp_last;
}

// ---------------------------------------------------------------- stats

struct Stat {
	const char *name;
	long cases;
	long fails;
	int printed;
};

static Stat st_fdopen = { "fdopen", 0, 0, 0 };

static void
fail(Stat &s, const char *fmt, ...)
{
	va_list ap;

	s.fails++;
	if (s.printed < 5) {
		s.printed++;
		fprintf(stderr, "FAIL %s: ", s.name);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fputc('\n', stderr);
	}
}

// ---------------------------------------------------------------- rng

static uint64_t rngstate;

static inline uint32_t
rnd(void)
{
	rngstate ^= rngstate << 13;
	rngstate ^= rngstate >> 7;
	rngstate ^= rngstate << 17;
	return (uint32_t)(rngstate >> 32);
}

static inline uint32_t
rndn(uint32_t n)
{
	return rnd() % n;
}

// ------------------------------------------------------- state compare

static long
poff(const unsigned char *p, const unsigned char *base)
{
	return p == nullptr ? -1L : (long)(p - base);
}

static bool
file_eq(const pb_file_t *a, const pb_file_t *b, char *why, size_t whysz)
{
#define	DIFF(field)							\
	do {								\
		if (a->field != b->field) {				\
			snprintf(why, whysz, "%s: ref=%lld port=%lld",	\
			    #field, (long long)a->field,		\
			    (long long)b->field);			\
			return (false);					\
		}							\
	} while (0)

	if (poff(a->_p, a->_buf) != poff(b->_p, b->_buf)) {
		snprintf(why, whysz, "_p: ref=%ld port=%ld",
		    poff(a->_p, a->_buf), poff(b->_p, b->_buf));
		return (false);
	}
	DIFF(_r);
	DIFF(_w);
	DIFF(_flags);
	DIFF(_file);
	DIFF(_orientation);
	DIFF(_flags2);
	DIFF(_mbstate.__want);
	DIFF(_mbstate.__ch);
	DIFF(_mbstate.__lbound);
	if (memcmp(a->_buf, b->_buf, sizeof(a->_buf)) != 0) {
		snprintf(why, whysz, "FILE buffer contents differ");
		return (false);
	}
	return (true);
#undef DIFF
}

// --------------------------------------------------------------- fdopen

static void
check_fdopen(int fd, const char *mode, int getfl, int getfd, int setfd,
    int sfpfail, int fcerr)
{
	int logA_fd[PB_FCNTL_LOG], logA_cmd[PB_FCNTL_LOG], logA_arg[PB_FCNTL_LOG];
	int nA, nB, i, ea, eb;
	pb_file_t *ra, *rb, *fpA, *fpB;
	char why[160];

	pb_fcntl_getfl_ret = getfl;
	pb_fcntl_getfd_ret = getfd;
	pb_fcntl_setfd_ret = setfd;
	pb_fcntl_errno = fcerr;
	pb_sfp_fail = sfpfail;

	pb_fcntl_reset();
	pb_sfp_last = nullptr;
	errno = 0;
	ra = ref_fdopen(fd, mode);
	ea = errno;
	fpA = pb_sfp_last;
	nA = pb_fcntl_log_n;
	for (i = 0; i < PB_FCNTL_LOG; i++) {
		logA_fd[i] = pb_fcntl_log_fd[i];
		logA_cmd[i] = pb_fcntl_log_cmd[i];
		logA_arg[i] = pb_fcntl_log_arg[i];
	}

	pb_fcntl_reset();
	pb_sfp_last = nullptr;
	errno = 0;
	rb = P::fdopen(fd, mode);
	eb = errno;
	fpB = pb_sfp_last;
	nB = pb_fcntl_log_n;

	st_fdopen.cases++;
	for (;;) {
		if ((ra == nullptr) != (rb == nullptr)) {
			fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: ret "
			    "ref=%s port=%s", fd, mode, getfl,
			    ra == nullptr ? "NULL" : "fp",
			    rb == nullptr ? "NULL" : "fp");
			break;
		}
		if (ea != eb) {
			fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: errno "
			    "ref=%d port=%d", fd, mode, getfl, ea, eb);
			break;
		}
		if (nA != nB) {
			fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: fcntl "
			    "calls ref=%d port=%d", fd, mode, getfl, nA, nB);
			break;
		}
		for (i = 0; i < nA && i < PB_FCNTL_LOG; i++) {
			if (logA_fd[i] != pb_fcntl_log_fd[i] ||
			    logA_cmd[i] != pb_fcntl_log_cmd[i] ||
			    logA_arg[i] != pb_fcntl_log_arg[i]) {
				fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: "
				    "fcntl[%d] ref=(%d,%d,%d) port=(%d,%d,%d)",
				    fd, mode, getfl, i, logA_fd[i],
				    logA_cmd[i], logA_arg[i],
				    pb_fcntl_log_fd[i], pb_fcntl_log_cmd[i],
				    pb_fcntl_log_arg[i]);
				goto done;
			}
		}
		if ((fpA == nullptr) != (fpB == nullptr)) {
			fail(st_fdopen, "fd=%d mode=\"%s\": __sfp ref=%s "
			    "port=%s", fd, mode, fpA ? "fp" : "NULL",
			    fpB ? "fp" : "NULL");
			break;
		}
		if (fpA != nullptr) {
			if ((ra == fpA) != (rb == fpB)) {
				fail(st_fdopen, "fd=%d mode=\"%s\": returned "
				    "pointer identity differs", fd, mode);
				break;
			}
			if ((fpA->_cookie == (void *)fpA) !=
			    (fpB->_cookie == (void *)fpB)) {
				fail(st_fdopen, "fd=%d mode=\"%s\": _cookie "
				    "differs", fd, mode);
				break;
			}
			if (fpA->_read != fpB->_read ||
			    fpA->_write != fpB->_write ||
			    fpA->_seek != fpB->_seek ||
			    fpA->_close != fpB->_close) {
				fail(st_fdopen, "fd=%d mode=\"%s\": I/O vector "
				    "differs", fd, mode);
				break;
			}
			fpA->_cookie = nullptr;
			fpB->_cookie = nullptr;
			if (!file_eq(fpA, fpB, why, sizeof(why))) {
				fail(st_fdopen, "fd=%d mode=\"%s\" getfl=%#x: "
				    "%s", fd, mode, getfl, why);
				break;
			}
		}
		break;
	}
done:
	free(fpA);
	free(fpB);
	pb_sfp_fail = 0;
}

static const char *fd_modes[] = {
	"", "r", "w", "a", "r+", "w+", "a+", "rb", "wb", "ab", "r+b", "re",
	"we", "ae", "r+e", "rx", "wx", "ax", "wxe", "q", "+", "b", "e",
	"rbe+x", "rt", "a+e", "rw", "R", "aE", "abe", "web", "reb", "a+be",
	"rex", "wex"
};
#define	NMODES	((int)(sizeof(fd_modes) / sizeof(fd_modes[0])))

static const int fd_getfl[] = {
	0, 1, 2, 3, 0x0008, 0x0009, 0x000a, 0x000b, 0x00040000, 0x00040001,
	0x00040002, 0x00040008, 0x8000, 0x0200 | 0x0400 | 1, -1, -22,
	0x00100000, 0x00100002
};
#define	NGETFL	((int)(sizeof(fd_getfl) / sizeof(fd_getfl[0])))

static const int fd_fds[] = { -5, -1, 0, 1, 2, 3, 7, 255, 32766, 32767,
	32768, 40000, 65535, 2147483647 };
#define	NFDS	((int)(sizeof(fd_fds) / sizeof(fd_fds[0])))

static const int fd_getfd[] = { 0, 1, 2, 5, -1 };
static const int fd_setfd[] = { 0, -1, 3 };

static void
test_fdopen_fixed(void)
{
	int i, m, g, gd, sd, sf;

	for (i = 0; i < NFDS; i++)
		for (m = 0; m < NMODES; m++)
			for (g = 0; g < NGETFL; g++)
				check_fdopen(fd_fds[i], fd_modes[m], fd_getfl[g],
				    0, 0, 0, EBADF);

	for (m = 0; m < NMODES; m++)
		for (g = 0; g < NGETFL; g++)
			for (gd = 0; gd < 5; gd++)
				for (sd = 0; sd < 3; sd++)
					for (sf = 0; sf < 2; sf++)
						check_fdopen(3, fd_modes[m],
						    fd_getfl[g], fd_getfd[gd],
						    fd_setfd[sd], sf, EBADF);
}

static void
test_fdopen_random(void)
{
	char mode[8];
	static const char mchars[] = "rwa+bxeqt";
	long it;

	for (it = 0; it < 200000; it++) {
		int fd, getfl, getfd, setfd, sfpfail;
		const char *m;
		size_t k, mlen;

		fd = (int)rndn(4) == 0 ? fd_fds[rndn(NFDS)]
				       : (int)rndn(70000) - 2;
		if (rndn(2)) {
			m = fd_modes[rndn((uint32_t)NMODES)];
		} else {
			mlen = rndn(6);
			for (k = 0; k < mlen; k++)
				mode[k] = mchars[rndn(9)];
			mode[mlen] = '\0';
			m = mode;
		}
		getfl = rndn(4) == 0 ? (int)rnd() : fd_getfl[rndn(NGETFL)];
		getfd = fd_getfd[rndn(5)];
		setfd = fd_setfd[rndn(3)];
		sfpfail = rndn(16) == 0 ? 1 : 0;
		check_fdopen(fd, m, getfl, getfd, setfd, sfpfail, EBADF);
	}
}

// ----------------------------------------------------------------- main

static void
on_alarm(int)
{
	const char msg[] = "TIMEOUT: a call failed to terminate\n";

	(void)!write(2, msg, sizeof(msg) - 1);
	_exit(1);
}

int
main(void)
{
	Stat *all[] = { &st_fdopen };
	long total_fail = 0;
	size_t i;

	signal(SIGALRM, on_alarm);
	alarm(300);

	rngstate = 0x13198a2e03707344ULL;
	test_fdopen_fixed();
	test_fdopen_random();

	printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	printf("%-20s %12s %12s\n", "--------------------", "------------",
	    "------------");
	for (i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
		printf("%-20s %12ld %12ld\n", all[i]->name, all[i]->cases,
		    all[i]->fails);
		total_fail += all[i]->fails;
	}
	printf("%-20s %12s %12ld\n", "TOTAL", "", total_fail);
	return (total_fail == 0 ? 0 : 1);
}
