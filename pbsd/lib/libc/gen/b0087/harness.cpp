/*
 * harness.cpp -- differential test for batch b0087.
 *
 * Every function in port.cppm is driven side by side with the corresponding
 * ref_ function from oracle.c and every observable is compared: the return
 * value, the arguments that reached the underlying kernel entry points, and
 * the complete contents of the caller-supplied buffers (guard bytes included).
 *
 * inotify_add_watch()/inotify_init1()/inotify_init() bottom out in
 * inotify_add_watch_at() and __sys___specialfd().  Those two symbols are
 * defined here as recording test doubles shared by BOTH the port and the
 * oracle: each records exactly what it was handed and returns a value that is
 * an injective function of every argument, so any divergence in the arguments
 * the port computes shows up both in the recorded state and in the return
 * value.
 */

import pbsd.lib.libc.gen.b0087;

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

namespace port = pbsd::lib_libc_gen::b0087;

extern "C" {
int ref_inotify_add_watch(int fd, const char *pathname, uint32_t mask);
int ref_inotify_init1(int flags);
int ref_inotify_init(void);
}

/* ------------------------------------------------------------------ doubles */

#define	MOCK_PATHCOPY	80u

struct mock_state {
	long		specialfd_calls;
	int		specialfd_type;
	int		specialfd_flags;
	unsigned long	specialfd_argsize;
	long		iaw_calls;
	int		iaw_fd;
	int		iaw_dfd;
	const char     *iaw_path;
	uint32_t	iaw_mask;
	unsigned long	iaw_pathlen;
	char		iaw_pathcopy[MOCK_PATHCOPY];
};

static mock_state g;

static void
mock_reset(void)
{
	memset(&g, 0, sizeof(g));
}

static inline uint32_t
fnv_step(uint32_t h, uint32_t v)
{
	/* xor-then-multiply-by-odd is a bijection, so distinct v => distinct h */
	return (h ^ v) * 16777619u;
}

extern "C" int
__sys___specialfd(int type, const void *arg, size_t argsize)
{
	int fl;
	uint32_t h;

	g.specialfd_calls++;
	g.specialfd_type = type;
	g.specialfd_argsize = (unsigned long)argsize;
	fl = 0;
	if (arg != nullptr && argsize >= sizeof(int))
		memcpy(&fl, arg, sizeof(int));
	g.specialfd_flags = fl;

	h = 0x811c9dc5u;
	h = fnv_step(h, (uint32_t)type);
	h = fnv_step(h, (uint32_t)fl);
	h = fnv_step(h, (uint32_t)argsize);
	return ((int)h);
}

extern "C" int
inotify_add_watch_at(int fd, int dfd, const char *pathname, uint32_t mask)
{
	uint32_t h;
	size_t n, c;

	g.iaw_calls++;
	g.iaw_fd = fd;
	g.iaw_dfd = dfd;
	g.iaw_path = pathname;
	g.iaw_mask = mask;

	h = 0x811c9dc5u;
	h = fnv_step(h, (uint32_t)fd);
	h = fnv_step(h, (uint32_t)dfd);
	h = fnv_step(h, mask);
	n = 0;
	if (pathname != nullptr) {
		n = strlen(pathname);
		for (size_t i = 0; i < n; i++)
			h = fnv_step(h, (uint32_t)(unsigned char)pathname[i]);
		c = n < MOCK_PATHCOPY - 1 ? n : MOCK_PATHCOPY - 1;
		memcpy(g.iaw_pathcopy, pathname, c);
		g.iaw_pathcopy[c] = '\0';
	} else {
		h = fnv_step(h, 0xdeadbeefu);
	}
	g.iaw_pathlen = (unsigned long)n;
	return ((int)h);
}

/* ------------------------------------------------------------------ capture */

static const long OFF_NULL = -999999L;

struct capture {
	int		rv;
	long		specialfd_calls;
	int		specialfd_type;
	int		specialfd_flags;
	unsigned long	specialfd_argsize;
	long		iaw_calls;
	int		iaw_fd;
	int		iaw_dfd;
	long		iaw_off;
	uint32_t	iaw_mask;
	unsigned long	iaw_pathlen;
	char		iaw_pathcopy[MOCK_PATHCOPY];
};

static capture
snap(int rv, const unsigned char *base)
{
	capture c;

	memset(&c, 0, sizeof(c));
	c.rv = rv;
	c.specialfd_calls = g.specialfd_calls;
	c.specialfd_type = g.specialfd_type;
	c.specialfd_flags = g.specialfd_flags;
	c.specialfd_argsize = g.specialfd_argsize;
	c.iaw_calls = g.iaw_calls;
	c.iaw_fd = g.iaw_fd;
	c.iaw_dfd = g.iaw_dfd;
	/* pointers are compared as offsets from the buffer base, never raw */
	c.iaw_off = g.iaw_path == nullptr ? OFF_NULL :
	    (long)(g.iaw_path - (const char *)base);
	c.iaw_mask = g.iaw_mask;
	c.iaw_pathlen = g.iaw_pathlen;
	memcpy(c.iaw_pathcopy, g.iaw_pathcopy, MOCK_PATHCOPY);
	return (c);
}

static bool
cap_eq(const capture &a, const capture &b)
{
	return (a.rv == b.rv &&
	    a.specialfd_calls == b.specialfd_calls &&
	    a.specialfd_type == b.specialfd_type &&
	    a.specialfd_flags == b.specialfd_flags &&
	    a.specialfd_argsize == b.specialfd_argsize &&
	    a.iaw_calls == b.iaw_calls &&
	    a.iaw_fd == b.iaw_fd &&
	    a.iaw_dfd == b.iaw_dfd &&
	    a.iaw_off == b.iaw_off &&
	    a.iaw_mask == b.iaw_mask &&
	    a.iaw_pathlen == b.iaw_pathlen &&
	    memcmp(a.iaw_pathcopy, b.iaw_pathcopy, MOCK_PATHCOPY) == 0);
}

static void
cap_dump(const char *tag, const capture &c)
{
	printf("    %s: rv=%d sfd{calls=%ld type=%d flags=%#x size=%lu} "
	    "iaw{calls=%ld fd=%d dfd=%d off=%ld mask=%#x len=%lu}\n",
	    tag, c.rv, c.specialfd_calls, c.specialfd_type,
	    (unsigned)c.specialfd_flags, c.specialfd_argsize,
	    c.iaw_calls, c.iaw_fd, c.iaw_dfd, c.iaw_off,
	    (unsigned)c.iaw_mask, c.iaw_pathlen);
}

/* -------------------------------------------------------------------- table */

struct stat_row {
	const char     *name;
	long		cases;
	long		failures;
	long		printed;
};

static stat_row rows[] = {
	{ "inotify_add_watch", 0, 0, 0 },
	{ "inotify_init1", 0, 0, 0 },
	{ "inotify_init", 0, 0, 0 },
};

#define	R_ADD_WATCH	0
#define	R_INIT1		1
#define	R_INIT		2

#define	MAX_PRINT	12

/* ------------------------------------------------------------------ buffers */

#define	BUFSZ	512u
#define	GUARD	0x7f

static unsigned char *bufa;
static unsigned char *bufb;

static void
dump_buf_diff(const unsigned char *a, const unsigned char *b)
{
	for (unsigned i = 0; i < BUFSZ; i++) {
		if (a[i] != b[i]) {
			printf("    buffer diverges at [%u]: port=%#02x "
			    "oracle=%#02x\n", i, a[i], b[i]);
			return;
		}
	}
}

/* --------------------------------------------------------------- test cases */

static void
do_add_watch(int fd, uint32_t mask, size_t off, const unsigned char *content,
    size_t clen, bool use_null)
{
	stat_row &r = rows[R_ADD_WATCH];

	if (off + clen + 1 > BUFSZ) {
		fprintf(stderr, "internal: case does not fit buffer\n");
		abort();
	}
	r.cases++;

	memset(bufa, GUARD, BUFSZ);
	memset(bufb, GUARD, BUFSZ);
	if (!use_null) {
		memcpy(bufa + off, content, clen);
		bufa[off + clen] = '\0';
		memcpy(bufb + off, content, clen);
		bufb[off + clen] = '\0';
	}

	const char *pa = use_null ? nullptr : (const char *)(bufa + off);
	const char *pb = use_null ? nullptr : (const char *)(bufb + off);

	mock_reset();
	int rva = port::inotify_add_watch(fd, pa, mask);
	capture ca = snap(rva, bufa);

	mock_reset();
	int rvb = ref_inotify_add_watch(fd, pb, mask);
	capture cb = snap(rvb, bufb);

	bool same_buf = memcmp(bufa, bufb, BUFSZ) == 0;
	if (!cap_eq(ca, cb) || !same_buf) {
		r.failures++;
		if (r.printed < MAX_PRINT) {
			r.printed++;
			printf("FAIL inotify_add_watch fd=%d mask=%#x off=%zu "
			    "clen=%zu null=%d\n", fd, (unsigned)mask, off,
			    clen, (int)use_null);
			cap_dump("port  ", ca);
			cap_dump("oracle", cb);
			if (!same_buf)
				dump_buf_diff(bufa, bufb);
		}
	}
}

static void
do_init1(int flags)
{
	stat_row &r = rows[R_INIT1];

	r.cases++;

	memset(bufa, GUARD, BUFSZ);
	memset(bufb, GUARD, BUFSZ);

	mock_reset();
	int rva = port::inotify_init1(flags);
	capture ca = snap(rva, bufa);

	mock_reset();
	int rvb = ref_inotify_init1(flags);
	capture cb = snap(rvb, bufb);

	bool same_buf = memcmp(bufa, bufb, BUFSZ) == 0;
	if (!cap_eq(ca, cb) || !same_buf) {
		r.failures++;
		if (r.printed < MAX_PRINT) {
			r.printed++;
			printf("FAIL inotify_init1 flags=%#x (%d)\n",
			    (unsigned)flags, flags);
			cap_dump("port  ", ca);
			cap_dump("oracle", cb);
			if (!same_buf)
				dump_buf_diff(bufa, bufb);
		}
	}
}

static void
do_init(void)
{
	stat_row &r = rows[R_INIT];

	r.cases++;

	memset(bufa, GUARD, BUFSZ);
	memset(bufb, GUARD, BUFSZ);

	mock_reset();
	int rva = port::inotify_init();
	capture ca = snap(rva, bufa);

	mock_reset();
	int rvb = ref_inotify_init();
	capture cb = snap(rvb, bufb);

	bool same_buf = memcmp(bufa, bufb, BUFSZ) == 0;
	if (!cap_eq(ca, cb) || !same_buf) {
		r.failures++;
		if (r.printed < MAX_PRINT) {
			r.printed++;
			printf("FAIL inotify_init\n");
			cap_dump("port  ", ca);
			cap_dump("oracle", cb);
			if (!same_buf)
				dump_buf_diff(bufa, bufb);
		}
	}
}

/* ----------------------------------------------------------------- edge set */

/*
 * The four bits the flag translation in inotify_init1() cares about.  The
 * harness keeps its own copies purely to *construct* inputs; all comparisons
 * remain port-versus-oracle.
 */
#define	H_IN_NONBLOCK_OLD	0x00080000u
#define	H_IN_CLOEXEC_OLD	0x00000800u
#define	H_IN_NONBLOCK		0x00000004u
#define	H_IN_CLOEXEC		0x00100000u

static void
edge_init1(void)
{
	static const uint32_t bg[] = {
		0x00000000u, 0x00000001u, 0x00000002u, 0x00000003u,
		0x00000004u, 0x00000008u, 0x000007ffu, 0x00000800u,
		0x00000fffu, 0x0007ffffu, 0x00080000u, 0x00080001u,
		0x000fffffu, 0x00100000u, 0x00100001u, 0x001fffffu,
		0x0f0f0f0fu, 0xf0f0f0f0u, 0x7fffffffu, 0x80000000u,
		0xfffff7ffu, 0xfff7ffffu, 0xffffffffu, 0x55555555u,
		0xaaaaaaaau,
	};
	static const uint32_t sel[4] = {
		H_IN_NONBLOCK_OLD, H_IN_CLOEXEC_OLD,
		H_IN_NONBLOCK, H_IN_CLOEXEC,
	};

	/* every combination of the four interesting bits, on every background */
	for (unsigned combo = 0; combo < 16u; combo++) {
		uint32_t base = 0;
		for (unsigned b = 0; b < 4u; b++)
			if ((combo & (1u << b)) != 0)
				base |= sel[b];
		for (unsigned i = 0; i < sizeof(bg) / sizeof(bg[0]); i++) {
			do_init1((int)(base | bg[i]));
			/* and the background with the combo bits cleared */
			do_init1((int)((bg[i] & ~base) | base));
			do_init1((int)(bg[i] & ~base));
		}
	}

	/* every single bit, and every single bit cleared */
	for (unsigned i = 0; i < 32u; i++) {
		do_init1((int)(1u << i));
		do_init1((int)~(1u << i));
		do_init1((int)(H_IN_NONBLOCK_OLD | (1u << i)));
		do_init1((int)(H_IN_CLOEXEC_OLD | (1u << i)));
		do_init1((int)(H_IN_NONBLOCK_OLD | H_IN_CLOEXEC_OLD |
		    (1u << i)));
	}

	/* both sides of each boundary value */
	static const uint32_t bounds[] = {
		0u, 1u, 2u, 3u,
		H_IN_NONBLOCK_OLD - 1u, H_IN_NONBLOCK_OLD,
		H_IN_NONBLOCK_OLD + 1u,
		H_IN_CLOEXEC_OLD - 1u, H_IN_CLOEXEC_OLD, H_IN_CLOEXEC_OLD + 1u,
		H_IN_NONBLOCK - 1u, H_IN_NONBLOCK, H_IN_NONBLOCK + 1u,
		H_IN_CLOEXEC - 1u, H_IN_CLOEXEC, H_IN_CLOEXEC + 1u,
		0x7fffffffu, 0x80000000u, 0x80000001u, 0xfffffffeu,
		0xffffffffu,
	};
	for (unsigned i = 0; i < sizeof(bounds) / sizeof(bounds[0]); i++)
		do_init1((int)bounds[i]);

	do_init1(INT_MIN);
	do_init1(INT_MAX);
	do_init1(-1);
}

static void
edge_add_watch(void)
{
	static const int fds[] = {
		INT_MIN, -101, -100, -99, -2, -1, 0, 1, 2, 3, 255, 256,
		65535, 0x7ffffffe, INT_MAX,
	};
	static const uint32_t masks[] = {
		0x00000000u, 0x00000001u, 0x00000002u, 0x00000004u,
		0x00000100u, 0x0000ffffu, 0x7fffffffu, 0x80000000u,
		0xfffffffeu, 0xffffffffu, 0x55555555u, 0xaaaaaaaau,
	};
	static const size_t offs[] = { 0, 1, 2, 3, 7, 8, 63, 64, 128, 300 };

	unsigned char c[300];

	/* empty string at every offset, for every fd */
	for (unsigned f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (unsigned o = 0; o < sizeof(offs) / sizeof(offs[0]); o++)
			do_add_watch(fds[f], 0x1234abcdu, offs[o], c, 0, false);

	/* every mask with a one-byte path */
	c[0] = 'a';
	for (unsigned m = 0; m < sizeof(masks) / sizeof(masks[0]); m++)
		for (unsigned f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
			do_add_watch(fds[f], masks[m], 5, c, 1, false);

	/* every single byte value 0x00..0xff as a one-byte path */
	for (unsigned v = 0; v < 256u; v++) {
		c[0] = (unsigned char)v;
		do_add_watch(3, 0xdeadbeefu, 9, c, 1, false);
		do_add_watch(-100, 0u, 0, c, 1, false);
	}

	/* high-bit runs */
	for (unsigned v = 0x80u; v <= 0xffu; v++) {
		for (unsigned i = 0; i < 16u; i++)
			c[i] = (unsigned char)v;
		do_add_watch(7, 0x0000ffffu, 3, c, 16, false);
	}

	/* NUL-heavy content: strlen stops early, guard bytes must still match */
	{
		static const unsigned char nulheavy[] = {
			'a', 0x00, 'b', 0x00, 'c', 0xff, 0x00, 0x80, 'd', 0x00,
		};
		for (size_t n = 0; n <= sizeof(nulheavy); n++)
			do_add_watch(11, 0x11223344u, 2, nulheavy, n, false);
		unsigned char allnul[32];
		memset(allnul, 0, sizeof(allnul));
		for (size_t n = 0; n <= sizeof(allnul); n++)
			do_add_watch(12, 0u, 4, allnul, n, false);
	}

	/* boundary lengths around the recording window and around powers of 2 */
	{
		static const size_t lens[] = {
			0, 1, 2, 3, 4, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63,
			64, 65, 78, 79, 80, 81, 127, 128, 129, 200, 255, 256,
			299, 300,
		};
		for (unsigned i = 0; i < sizeof(c); i++)
			c[i] = (unsigned char)(0x80u + (i % 0x80u));
		for (unsigned i = 0; i < sizeof(lens) / sizeof(lens[0]); i++) {
			do_add_watch(13, 0xcafebabeu, 0, c, lens[i], false);
			do_add_watch(13, 0xcafebabeu, 1, c, lens[i], false);
			do_add_watch(13, 0xcafebabeu,
			    BUFSZ - lens[i] - 1, c, lens[i], false);
		}
		/* the very last legal placement: path ends on the last byte */
		for (unsigned i = 0; i < sizeof(c); i++)
			c[i] = 'z';
		do_add_watch(14, 1u, BUFSZ - 1, c, 0, false);
		do_add_watch(14, 1u, BUFSZ - 2, c, 1, false);
	}

	/* NULL pathname is forwarded untouched */
	for (unsigned f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (unsigned m = 0; m < sizeof(masks) / sizeof(masks[0]); m++)
			do_add_watch(fds[f], masks[m], 0, c, 0, true);
}

/* ------------------------------------------------------------------- random */

static uint64_t rng_state = 0x00b0087c0ffeeULL;

static inline uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static inline uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

static int
rnd_flags(void)
{
	uint32_t v = rnd32();

	switch (rnd32() % 6u) {
	case 0:
		break;
	case 1:
		v &= 0x001fffffu;
		break;
	case 2: {
		uint32_t b = 0;
		unsigned n = (unsigned)(rnd32() % 4u) + 1u;
		for (unsigned i = 0; i < n; i++)
			b |= 1u << (rnd32() % 32u);
		v = b;
		break;
	}
	case 3:
		v = (rnd32() % 16u) * H_IN_CLOEXEC_OLD;
		break;
	case 4:
		v = ((rnd32() & 1u) != 0 ? H_IN_NONBLOCK_OLD : 0u) |
		    (rnd32() & 0x0000000fu);
		break;
	default:
		v = 0;
		if ((rnd32() & 1u) != 0)
			v |= H_IN_NONBLOCK_OLD;
		if ((rnd32() & 1u) != 0)
			v |= H_IN_CLOEXEC_OLD;
		if ((rnd32() & 1u) != 0)
			v |= H_IN_NONBLOCK;
		if ((rnd32() & 1u) != 0)
			v |= H_IN_CLOEXEC;
		if ((rnd32() & 1u) != 0)
			v |= rnd32() & 0x0f0f0f0fu;
		break;
	}
	return ((int)v);
}

static int
rnd_fd(void)
{
	switch (rnd32() % 5u) {
	case 0:
		return ((int)(rnd32() % 8u));
	case 1:
		return (-(int)(rnd32() % 128u) - 1);
	case 2:
		return (INT_MIN + (int)(rnd32() % 4u));
	case 3:
		return (INT_MAX - (int)(rnd32() % 4u));
	default:
		return ((int)rnd32());
	}
}

static unsigned char
rnd_byte(void)
{
	switch (rnd32() % 4u) {
	case 0:
		return (0u);
	case 1:
		return ((unsigned char)(0x80u + rnd32() % 0x80u));
	case 2:
		return ((unsigned char)(0x20u + rnd32() % 0x5fu));
	default:
		return ((unsigned char)(rnd32() & 0xffu));
	}
}

#define	SWEEP_ITERS	300000L

static void
sweep(void)
{
	unsigned char c[320];

	for (long it = 0; it < SWEEP_ITERS; it++) {
		do_init1(rnd_flags());

		size_t clen = (size_t)(rnd32() % 260u);
		size_t maxoff = BUFSZ - clen - 1u;
		size_t off;
		switch (rnd32() % 4u) {
		case 0:
			off = 0;
			break;
			/* hug both ends of the legal placement window */
		case 1:
			off = maxoff;
			break;
		case 2:
			off = maxoff > 0 ? maxoff - 1u : 0u;
			break;
		default:
			off = (size_t)(rnd32() % (uint32_t)(maxoff + 1u));
			break;
		}
		for (size_t i = 0; i < clen; i++)
			c[i] = rnd_byte();
		do_add_watch(rnd_fd(), rnd32(), off, c, clen,
		    (rnd32() % 64u) == 0u);

		if ((it % 977L) == 0L)
			do_init();
	}
}

/* --------------------------------------------------------------------- main */

int
main(void)
{
	bufa = (unsigned char *)malloc(BUFSZ);
	bufb = (unsigned char *)malloc(BUFSZ);
	if (bufa == nullptr || bufb == nullptr) {
		fprintf(stderr, "out of memory\n");
		return (1);
	}

	edge_init1();
	edge_add_watch();
	do_init();
	sweep();

	long total_cases = 0, total_failures = 0;

	printf("\n%-24s %10s %10s\n", "function", "cases", "failures");
	printf("--------------------------------------------------\n");
	for (unsigned i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
		printf("%-24s %10ld %10ld\n", rows[i].name, rows[i].cases,
		    rows[i].failures);
		total_cases += rows[i].cases;
		total_failures += rows[i].failures;
	}
	printf("--------------------------------------------------\n");
	printf("%-24s %10ld %10ld\n", "TOTAL", total_cases, total_failures);

	if (total_failures != 0) {
		printf("\nRESULT: FAIL (%ld mismatching cases)\n",
		    total_failures);
		return (1);
	}
	printf("\nRESULT: PASS\n");
	return (0);
}
