// Differential test for PBSD batch b0118s2 (fwrite.c).
//
// Every function of the batch is executed twice on identical inputs: once
// through the reference (ref_*, compiled from the unmodified C bodies in
// oracle.c) and once through the C++23 port.  Return values, errno, the whole
// sink buffer including guard bytes beyond the nominal write window and the
// whole mock FILE state are compared.

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

import pbsd.lib.libc.stdio.b0118s2;

namespace P = pbsd::lib_libc_stdio::b0118s2;

extern "C" {
size_t ref_fwrite_unlocked(const void *, size_t, size_t, pb_file_t *);
size_t ref_fwrite(const void *, size_t, size_t, pb_file_t *);
void pb_file_init(pb_file_t *, unsigned char *, size_t, size_t);
}

// ---------------------------------------------------------------- stats

struct Stat {
	const char *name;
	long cases;
	long fails;
	int printed;
};

static Stat st_fwrite_unlocked = { "fwrite_unlocked", 0, 0, 0 };
static Stat st_fwrite = { "fwrite", 0, 0, 0 };

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

static void
hexdump(char *out, size_t outsz, const unsigned char *p, size_t n)
{
	size_t i, o = 0;

	if (n > 24)
		n = 24;
	for (i = 0; i < n && o + 3 < outsz; i++)
		o += (size_t)snprintf(out + o, outsz - o, "%02x", p[i]);
	out[o] = '\0';
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
	DIFF(sfvwrite_calls);
	DIFF(sink_len);
	if (memcmp(a->_buf, b->_buf, sizeof(a->_buf)) != 0) {
		snprintf(why, whysz, "FILE buffer contents differ");
		return (false);
	}
	if (a->sink != nullptr && b->sink != nullptr &&
	    a->sink_size == b->sink_size &&
	    memcmp(a->sink, b->sink, a->sink_size) != 0) {
		size_t i;
		for (i = 0; i < a->sink_size; i++)
			if (a->sink[i] != b->sink[i])
				break;
		snprintf(why, whysz,
		    "sink[%zu]: ref=%02x port=%02x", i, a->sink[i], b->sink[i]);
		return (false);
	}
	return (true);
#undef DIFF
}

// --------------------------------------------------------------- fwrite

#define	FW_SINK	4096

static pb_file_t fwA, fwB;
static unsigned char fwSinkA[FW_SINK], fwSinkB[FW_SINK];
static unsigned char fwSrc[FW_SINK];

static void
fill_fwsrc(size_t nbytes, unsigned char tag)
{
	size_t i;

	for (i = 0; i < FW_SINK; i++)
		fwSrc[i] = 0x7f;
	for (i = 0; i < nbytes && i < FW_SINK; i++) {
		if (tag == 0)
			fwSrc[i] = (unsigned char)(0x80 + (i % 0x80));
		else
			fwSrc[i] = (unsigned char)((tag + i) & 0xff);
	}
}

static void
check_fwrite(const void *src, size_t size, size_t count, size_t cap,
    short orient, short flags, int locked)
{
	Stat &st = locked ? st_fwrite : st_fwrite_unlocked;
	char why[160];
	size_t ra, rb;
	int ea, eb;

	if (cap > FW_SINK)
		cap = FW_SINK;
	pb_file_init(&fwA, fwSinkA, FW_SINK, cap);
	pb_file_init(&fwB, fwSinkB, FW_SINK, cap);
	fwA._flags = flags;
	fwB._flags = flags;
	fwA._orientation = orient;
	fwB._orientation = orient;

	errno = 0;
	ra = locked ? ref_fwrite(src, size, count, &fwA)
		    : ref_fwrite_unlocked(src, size, count, &fwA);
	ea = errno;
	errno = 0;
	rb = locked ? P::fwrite(src, size, count, &fwB)
		    : P::fwrite_unlocked(src, size, count, &fwB);
	eb = errno;

	st.cases++;
	if (ra != rb) {
		fail(st, "size=%zu count=%zu cap=%zu: ret ref=%zu port=%zu",
		    size, count, cap, ra, rb);
		return;
	}
	if (ea != eb) {
		fail(st, "size=%zu count=%zu cap=%zu: errno ref=%d port=%d",
		    size, count, cap, ea, eb);
		return;
	}
	if (!file_eq(&fwA, &fwB, why, sizeof(why)))
		fail(st, "size=%zu count=%zu cap=%zu: %s", size, count, cap,
		    why);
}

static void
check_fwrite(size_t size, size_t count, size_t cap, short orient, short flags,
    int locked)
{
	size_t n = size * count;
	fill_fwsrc(n, 0);
	check_fwrite(fwSrc, size, count, cap, orient, flags, locked);
}

static void
test_fwrite_fixed(void)
{
	static const size_t sizes[] = { 0, 1, 2, 3, 4, 7, 8, 16, 0xFFFF,
	    0x10000 };
	static const size_t counts[] = { 0, 1, 2, 3, 5, 12, 100, 4096, 0xFFFF,
	    0x10000 };
	static const unsigned char src_empty[] = { 0 };
	static const unsigned char src_one[] = { 'x' };
	static const unsigned char src_nul[] = { 0, 0, 0, 0 };
	static const unsigned char src_hibit[] = { 0x80, 0xff, 0xfe, 0x81 };
	static const unsigned char src_mix[] = { 0, 0xff, '\n', 0x80, 0x7f };
	size_t si, ci, cap;
	int locked;

	for (locked = 0; locked < 2; locked++) {
		for (si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++)
			for (ci = 0; ci < sizeof(counts) / sizeof(counts[0]);
			    ci++) {
				size_t n = sizes[si] * counts[ci];
				static const size_t caps[] = { 0, 1, 2, 3, 5,
				    11, 12, 13, 100, 4096 };
				size_t k;

				for (k = 0; k < sizeof(caps) / sizeof(caps[0]);
				    k++)
					check_fwrite(sizes[si], counts[ci],
					    caps[k], 0, 0, locked);
				if (n <= FW_SINK) {
					for (cap = (n > 2 ? n - 2 : 0);
					    cap <= n + 2 && cap <= FW_SINK;
					    cap++)
						check_fwrite(sizes[si],
						    counts[ci], cap, 0, 0,
						    locked);
				}
			}

		check_fwrite(0x10000, SIZE_MAX / 0x10000, 64, 0, 0, locked);
		check_fwrite(0x10000, SIZE_MAX / 0x10000 + 1, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX / 0x10000, 0x10000, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX / 0x10000 + 1, 0x10000, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX, 1, 64, 0, 0, locked);
		check_fwrite(1, SIZE_MAX, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX, 2, 64, 0, 0, locked);
		check_fwrite(2, SIZE_MAX, 64, 0, 0, locked);
		check_fwrite(SIZE_MAX / 2, 2, 64, 0, 0, locked);
		check_fwrite(2, SIZE_MAX / 2, 64, 0, 0, locked);
		check_fwrite(0xFFFF, 0xFFFF, 64, 0, 0, locked);
		check_fwrite(0x10000, 1, 64, 0, 0, locked);
		check_fwrite(1, 0x10000, 64, 0, 0, locked);
		check_fwrite(0x10000, 1, 4096, 0, 0, locked);
		check_fwrite(0, 0x10000, 64, 0, 0, locked);
		check_fwrite(0x10000, 0, 64, 0, 0, locked);

		/* hand-written source buffers */
		check_fwrite(src_empty, 1, 1, 8, 0, 0, locked);
		check_fwrite(src_one, 1, 1, 8, 0, 0, locked);
		check_fwrite(src_nul, 4, 1, 8, 0, 0, locked);
		check_fwrite(src_hibit, 4, 1, 8, 0, 0, locked);
		check_fwrite(src_mix, 5, 1, 8, 0, 0, locked);
		check_fwrite(src_hibit, 2, 2, 3, 0, 0, locked);
		check_fwrite(src_hibit, 2, 2, 4, 0, 0, locked);
		check_fwrite(src_mix, 1, 5, 3, 0, 0, locked);
		check_fwrite(src_mix, 1, 5, 5, 0, 0, locked);

		for (int o = -1; o <= 1; o++)
			for (int f = 0; f < 3; f++) {
				static const short fl[] = { 0, 0x0008, 0x0040 };
				check_fwrite(0, 3, 8, (short)o, fl[f], locked);
				check_fwrite(3, 0, 8, (short)o, fl[f], locked);
				check_fwrite(3, 3, 8, (short)o, fl[f], locked);
				check_fwrite(3, 3, 9, (short)o, fl[f], locked);
				check_fwrite(3, 3, 64, (short)o, fl[f], locked);
				check_fwrite(src_hibit, 2, 2, 3,
				    (short)o, fl[f], locked);
			}
	}
}

static void
test_fwrite_random(void)
{
	static const size_t bigs[] = { 0xFFFF, 0x10000, 0x10001, SIZE_MAX,
	    SIZE_MAX / 2, SIZE_MAX / 0x10000, SIZE_MAX / 0x10000 + 1, 1, 0 };
	long it;

	for (it = 0; it < 200000; it++) {
		size_t size, count, cap;
		int locked = (int)rndn(2);
		short orient = (short)((int)rndn(3) - 1);
		short flags = (short)(rndn(2) ? 0 : 0x0040);
		unsigned char tag = (unsigned char)rnd();

		if (rndn(8) == 0) {
			size = bigs[rndn(9)];
			count = bigs[rndn(9)];
			cap = rndn(FW_SINK + 1);
		} else {
			size = rndn(8);
			count = rndn(300);
			cap = rndn((uint32_t)(size * count) + 4);
		}
		fill_fwsrc(size * count, tag);
		check_fwrite(fwSrc, size, count, cap, orient, flags, locked);
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
	Stat *all[] = { &st_fwrite_unlocked, &st_fwrite };
	long total_fail = 0;
	size_t i;

	signal(SIGALRM, on_alarm);
	alarm(300);

	rngstate = 0xfedcba9876543210ULL;
	test_fwrite_fixed();
	test_fwrite_random();

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
