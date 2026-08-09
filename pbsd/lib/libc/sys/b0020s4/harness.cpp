/*
 * Differential test for pbsd/lib/libc/sys/b0020s4 (getdents).
 *
 * getdents() is a pure forwarding wrapper around the libc-internal syscall
 * stub __sys_getdirentries().  Its entire observable behaviour is
 *
 *   (a) the return value it hands back, and
 *   (b) the exact argument tuple (fd, buf, nbytes, basep) it forwards, and
 *   (c) whatever the kernel wrote through the buffer pointer it forwarded.
 *
 * So the syscall stub is replaced here by a deterministic mock that is linked
 * against BOTH the C++ port and the ref_ oracle.  The mock records the whole
 * argument tuple, derives its return value from (fd, nbytes), and fills the
 * caller's buffer with a pattern derived from (fd, nbytes, index).  Any
 * divergence in a forwarded argument therefore shows up in the recorded
 * tuple, in the return value and in the buffer bytes at once.
 *
 * Buffers are compared in full, including the guard bytes past the nominal
 * [buf, buf + nbytes) write window, so a port that hands the kernel a shifted
 * pointer or a stretched length is caught.  Pointers are compared as offsets
 * from each side's own buffer base, never as raw addresses.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/types.h>

import pbsd.lib.libc.sys.b0020s4;

extern "C" ssize_t ref_getdents(int fd, char *buf, size_t nbytes);

/* ------------------------------------------------------------------ */
/* Deterministic mock of the syscall stub, shared by port and oracle. */
/* ------------------------------------------------------------------ */

struct syscall_record {
	unsigned long	calls;
	int		fd;
	char		*buf;
	size_t		nbytes;
	off_t		*basep;
};

static syscall_record g_rec;

/* Bounds of the allocation the current call is allowed to scribble on. */
static char *g_alloc_base;
static size_t g_alloc_size;

static inline uint64_t
mix64(uint64_t z)
{
	z += 0x9E3779B97F4A7C15ull;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
	return (z ^ (z >> 31));
}

static inline ssize_t
synth_ret(int fd, size_t nbytes)
{
	uint64_t h;

	h = mix64((uint64_t)(uint32_t)fd * 0x100000001B3ull + (uint64_t)nbytes);
	switch (h % 8u) {
	case 0:
		return ((ssize_t)-1);
	case 1:
		return ((ssize_t)0);
	case 2:
		return ((ssize_t)(nbytes & 0x3ffu));
	case 3:
		return ((ssize_t)(h & 0x7fffffffu));
	case 4:
		return (-(ssize_t)(h & 0x7fu) - 1);
	case 5:
		/* Wider than int, so a truncating return is observable. */
		return ((ssize_t)((h & 0x00ffffffffffffffull) |
		    0x0000010000000000ull));
	case 6:
		return (-(ssize_t)((h & 0x00ffffffffffffffull) |
		    0x0000010000000000ull));
	default:
		/* Low 32 bits carry no information at all. */
		return ((ssize_t)((h & 0x7fffffff00000000ull) | 1ull));
	}
}

static inline char
synth_byte(int fd, size_t nbytes, size_t i)
{
	uint64_t h;

	h = mix64(((uint64_t)(uint32_t)fd << 32) ^ (uint64_t)nbytes ^
	    (uint64_t)i * 0x9E3779B1u);
	return ((char)(unsigned char)(h & 0xffu));
}

extern "C" ssize_t
__sys_getdirentries(int fd, char *buf, size_t nbytes, off_t *basep)
{
	size_t avail, n, i;

	g_rec.calls++;
	g_rec.fd = fd;
	g_rec.buf = buf;
	g_rec.nbytes = nbytes;
	g_rec.basep = basep;

	/*
	 * basep is deliberately never dereferenced: a mutated port may hand us
	 * a bogus non-null pointer and the recorded value is what is compared.
	 */
	if (buf != nullptr && g_alloc_base != nullptr &&
	    buf >= g_alloc_base && buf <= g_alloc_base + g_alloc_size) {
		avail = (size_t)(g_alloc_base + g_alloc_size - buf);
		n = (nbytes < avail) ? nbytes : avail;
		for (i = 0; i < n; i++)
			buf[i] = synth_byte(fd, nbytes, i);
	}
	return (synth_ret(fd, nbytes));
}

/* ------------------------------------------------------------------ */
/* Test driver.                                                       */
/* ------------------------------------------------------------------ */

static const size_t ALLOC = 320;
static const unsigned char GUARD = 0x7f;

static char bufA[ALLOC];
static char bufB[ALLOC];

static unsigned long long g_cases;
static unsigned long long g_failures;
static unsigned g_reported;

enum {
	PAT_GUARD_ONLY = 0,
	PAT_ZERO,
	PAT_HIGH_BIT,
	PAT_ALTERNATING,
	PAT_ASCII,
	PAT_NUL_HEAVY,
	PAT_RAMP,
	PAT_COUNT
};

static void
fill_input(char *p, unsigned pat, size_t inlen)
{
	size_t i;

	memset(p, (int)GUARD, ALLOC);
	if (inlen > ALLOC)
		inlen = ALLOC;
	for (i = 0; i < inlen; i++) {
		switch (pat) {
		case PAT_ZERO:
			p[i] = (char)0x00;
			break;
		case PAT_HIGH_BIT:
			p[i] = (char)(unsigned char)(0x80u + (i % 0x80u));
			break;
		case PAT_ALTERNATING:
			p[i] = (char)(unsigned char)((i & 1u) ? 0x80u : 0x7fu);
			break;
		case PAT_ASCII:
			p[i] = (char)('a' + (int)(i % 26u));
			break;
		case PAT_NUL_HEAVY:
			p[i] = (char)((i % 3u == 0u) ? 0 :
			    (char)(unsigned char)0xffu);
			break;
		case PAT_RAMP:
			p[i] = (char)(unsigned char)(i & 0xffu);
			break;
		case PAT_GUARD_ONLY:
		default:
			break;
		}
	}
}

static void
report(int fd, size_t nbytes, size_t off, unsigned pat, size_t inlen,
    int use_null, const char *what)
{
	g_failures++;
	if (g_reported < 20u) {
		g_reported++;
		fprintf(stderr, "FAIL getdents: %s  (fd=%d nbytes=%zu off=%zu "
		    "pat=%u inlen=%zu null=%d)\n", what, fd, nbytes, off, pat,
		    inlen, use_null);
	}
}

static void
run_case(int fd, size_t nbytes, size_t off, unsigned pat, size_t inlen,
    int use_null)
{
	syscall_record recP, recR;
	ssize_t retP, retR;
	char *pP, *pR;
	ptrdiff_t offP, offR;

	g_cases++;

	if (off > ALLOC)
		off = ALLOC;

	fill_input(bufA, pat, inlen);
	fill_input(bufB, pat, inlen);

	pP = use_null ? nullptr : bufA + off;
	pR = use_null ? nullptr : bufB + off;

	memset(&g_rec, 0, sizeof(g_rec));
	g_alloc_base = use_null ? nullptr : bufA;
	g_alloc_size = ALLOC;
	retP = pbsd::lib_libc_sys::b0020s4::getdents(fd, pP, nbytes);
	recP = g_rec;

	memset(&g_rec, 0, sizeof(g_rec));
	g_alloc_base = use_null ? nullptr : bufB;
	g_alloc_size = ALLOC;
	retR = ref_getdents(fd, pR, nbytes);
	recR = g_rec;

	g_alloc_base = nullptr;
	g_alloc_size = 0;

	if (retP != retR)
		report(fd, nbytes, off, pat, inlen, use_null, "return value");
	if (recP.calls != recR.calls || recP.calls != 1ul)
		report(fd, nbytes, off, pat, inlen, use_null, "call count");
	if (recP.fd != recR.fd || recP.fd != fd)
		report(fd, nbytes, off, pat, inlen, use_null, "fd argument");
	if (recP.nbytes != recR.nbytes || recP.nbytes != nbytes)
		report(fd, nbytes, off, pat, inlen, use_null,
		    "nbytes argument");
	if (recP.basep != nullptr || recR.basep != nullptr)
		report(fd, nbytes, off, pat, inlen, use_null,
		    "basep argument");

	if (use_null) {
		if (recP.buf != nullptr || recR.buf != nullptr)
			report(fd, nbytes, off, pat, inlen, use_null,
			    "buf argument (expected null)");
	} else {
		if (recP.buf == nullptr || recR.buf == nullptr) {
			report(fd, nbytes, off, pat, inlen, use_null,
			    "buf argument (unexpected null)");
		} else {
			offP = recP.buf - bufA;
			offR = recR.buf - bufB;
			if (offP != offR || offP != (ptrdiff_t)off)
				report(fd, nbytes, off, pat, inlen, use_null,
				    "buf offset");
		}
		if (memcmp(bufA, bufB, ALLOC) != 0)
			report(fd, nbytes, off, pat, inlen, use_null,
			    "buffer contents");
	}
}

/* ------------------------------------------------------------------ */

static const size_t edge_nbytes[] = {
	0u, 1u, 2u, 3u, 4u, 5u, 7u, 8u, 9u, 15u, 16u, 17u, 31u, 32u, 33u,
	63u, 64u, 65u, 127u, 128u, 129u, 255u, 256u, 257u,
	ALLOC - 2u, ALLOC - 1u, ALLOC, ALLOC + 1u, ALLOC + 2u,
	511u, 512u, 513u, 4095u, 4096u, 4097u,
	0x7fffu, 0x8000u, 0x8001u, 0xffffu, 0x10000u,
	(size_t)INT_MAX - 1u, (size_t)INT_MAX, (size_t)INT_MAX + 1u,
	(size_t)0x80000000u, (size_t)0xffffffffu, (size_t)0x100000000ull,
	(size_t)SSIZE_MAX - 1u, (size_t)SSIZE_MAX, (size_t)SSIZE_MAX + 1u,
	SIZE_MAX / 2u, SIZE_MAX - 2u, SIZE_MAX - 1u, SIZE_MAX
};

static const size_t edge_off[] = {
	0u, 1u, 2u, 3u, 7u, 8u, 15u, 16u, 31u, 32u, 63u, 64u,
	ALLOC / 2u, ALLOC - 65u, ALLOC - 64u, ALLOC - 33u, ALLOC - 32u,
	ALLOC - 2u, ALLOC - 1u, ALLOC
};

static const int edge_fd[] = {
	INT_MIN, INT_MIN + 1, -0x10000, -257, -256, -129, -128, -127,
	-3, -2, -1, 0, 1, 2, 3, 4, 5, 0x7f, 0x80, 0x81, 0xfe, 0xff,
	0x100, 0x7fff, 0x8000, 0xffff, 0x10000, 0x7ffffffe, INT_MAX
};

static const size_t edge_inlen[] = {
	0u, 1u, 2u, 3u, 8u, 31u, 32u, 64u, 128u, ALLOC - 1u, ALLOC
};

static void
edge_pass(void)
{
	size_t i, j, k, m;
	unsigned p;

	/* fd sweep against a couple of representative lengths. */
	for (i = 0; i < sizeof(edge_fd) / sizeof(edge_fd[0]); i++) {
		for (j = 0; j < sizeof(edge_nbytes) / sizeof(edge_nbytes[0]);
		    j++) {
			run_case(edge_fd[i], edge_nbytes[j], 0u,
			    PAT_GUARD_ONLY, 0u, 0);
			run_case(edge_fd[i], edge_nbytes[j], 1u, PAT_RAMP,
			    ALLOC, 0);
			run_case(edge_fd[i], edge_nbytes[j], 0u, PAT_HIGH_BIT,
			    ALLOC, 1);
		}
	}

	/* offset x length x pattern cross product. */
	for (j = 0; j < sizeof(edge_nbytes) / sizeof(edge_nbytes[0]); j++) {
		for (k = 0; k < sizeof(edge_off) / sizeof(edge_off[0]); k++) {
			for (p = 0; p < (unsigned)PAT_COUNT; p++) {
				run_case(3, edge_nbytes[j], edge_off[k], p,
				    ALLOC, 0);
				run_case(-1, edge_nbytes[j], edge_off[k], p,
				    0u, 0);
			}
		}
	}

	/* input-length sweep: NUL-heavy and high-bit content. */
	for (m = 0; m < sizeof(edge_inlen) / sizeof(edge_inlen[0]); m++) {
		for (k = 0; k < sizeof(edge_off) / sizeof(edge_off[0]); k++) {
			run_case(0, 0u, edge_off[k], PAT_NUL_HEAVY,
			    edge_inlen[m], 0);
			run_case(0, 1u, edge_off[k], PAT_HIGH_BIT,
			    edge_inlen[m], 0);
			run_case(0, ALLOC, edge_off[k], PAT_ZERO,
			    edge_inlen[m], 0);
			run_case(0, SIZE_MAX, edge_off[k], PAT_ALTERNATING,
			    edge_inlen[m], 0);
		}
	}

	/* null buffer with every length. */
	for (j = 0; j < sizeof(edge_nbytes) / sizeof(edge_nbytes[0]); j++)
		run_case(7, edge_nbytes[j], 0u, PAT_GUARD_ONLY, 0u, 1);
}

static void
random_pass(void)
{
	uint64_t s = 0x0DDBA11C0FFEE123ull;
	unsigned long i;
	uint64_t r;
	int fd;
	size_t nbytes, off, inlen;
	unsigned pat;
	int use_null;

	for (i = 0; i < 220000ul; i++) {
		r = mix64(s);
		s = r;

		switch ((unsigned)(r & 7u)) {
		case 0:
			fd = (int)(int32_t)(r >> 8);
			break;
		case 1:
			fd = -1;
			break;
		case 2:
			fd = (int)((r >> 8) % 64u);
			break;
		case 3:
			fd = -(int)((r >> 8) % 512u);
			break;
		case 4:
			fd = (int)(unsigned)((r >> 8) & 0xffu);
			break;
		case 5:
			fd = (int)(signed char)((r >> 8) & 0xffu);
			break;
		case 6:
			fd = INT_MAX - (int)((r >> 8) % 4u);
			break;
		default:
			fd = INT_MIN + (int)((r >> 8) % 4u);
			break;
		}

		r = mix64(s);
		s = r;
		switch ((unsigned)(r & 7u)) {
		case 0:
			nbytes = (size_t)((r >> 8) % (ALLOC + 3u));
			break;
		case 1:
			nbytes = (size_t)((r >> 8) % 8u);
			break;
		case 2:
			nbytes = ALLOC - (size_t)((r >> 8) % 4u);
			break;
		case 3:
			nbytes = (size_t)((r >> 8) % 0x10000u);
			break;
		case 4:
			nbytes = SIZE_MAX - (size_t)((r >> 8) % 4u);
			break;
		case 5:
			nbytes = (size_t)SSIZE_MAX + (size_t)((r >> 8) % 3u) -
			    1u;
			break;
		case 6:
			nbytes = (size_t)(r >> 8);
			break;
		default:
			nbytes = (size_t)1u << ((r >> 8) % 64u);
			break;
		}

		r = mix64(s);
		s = r;
		off = (size_t)(r % (ALLOC + 1u));
		pat = (unsigned)((r >> 20) % (unsigned)PAT_COUNT);
		inlen = (size_t)((r >> 32) % (ALLOC + 1u));
		use_null = (int)(((r >> 48) % 64u) == 0u);

		run_case(fd, nbytes, off, pat, inlen, use_null);
	}
}

int
main(void)
{
	edge_pass();
	random_pass();

	printf("%-16s %12s %12s\n", "function", "cases", "failures");
	printf("%-16s %12llu %12llu\n", "getdents", g_cases, g_failures);
	printf("%-16s %12llu %12llu\n", "TOTAL", g_cases, g_failures);
	printf("%s\n", g_failures == 0ull ? "PASS" : "FAIL");

	return (g_failures == 0ull ? 0 : 1);
}
