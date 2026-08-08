/*
 * Differential test for pbsd.lib.libc.rpc.b0196s4 (des_crypt.c).
 *
 * Both the C oracle (ref_*) and the C++23 port are driven through the same
 * instrumented DES back end, which together with the caller's buffers is
 * everything either of them can observe or mutate.  Every case compares:
 *
 *   - the return value,
 *   - the ENTIRE data buffer of both sides, guard bytes included,
 *   - the ENTIRE key buffer and, for cbc_crypt, the ENTIRE ivec buffer,
 *   - which back end was selected and how many times it was entered,
 *   - the exact struct desparams it was handed: key bytes, direction, mode,
 *     length, and the input vector both on the way in and on the way out,
 *   - the OFFSET of the data pointer from that side's own buffer base.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

import pbsd.lib.libc.rpc.b0196s4;

namespace port = pbsd::lib_libc_rpc::b0196s4;

/*
 * The oracle side.  struct desparams comes from the module; the C
 * translation unit declares the same layout, and these symbols have C
 * linkage, so only the layout matters.
 */
extern "C" {
int ref_cbc_crypt(char *, char *, unsigned, unsigned, char *);
int ref_ecb_crypt(char *, char *, unsigned, unsigned);
extern int (*__des_crypt_LOCAL)(char *, unsigned, struct desparams *);
}

/* ------------------------------------------------------------------ */
/* Instrumented DES back end                                          */
/* ------------------------------------------------------------------ */

struct CallRec {
	int backend;		/* 0 none, 1 _des_crypt_call, 2 __des_crypt_LOCAL */
	int ncalls;
	int len;
	int dir;
	int mode;
	int ret;
	unsigned char key[8];
	unsigned char ivec_in[8];
	unsigned char ivec_out[8];
	const char *bufptr;
};

static CallRec g_rec;

static void
rec_reset(void)
{
	std::memset(&g_rec, 0, sizeof(g_rec));
}

/*
 * A deterministic stand-in for the real DES transform.  It is a pure
 * function of the bytes it is handed, so the oracle and the port have to
 * produce bit-identical results.  The transform folds in every key byte, the
 * direction, the mode, the byte position and, in CBC, the chaining state, so
 * a wrong key copy, a wrong direction or a wrong mode all show up in the
 * caller's buffer as well as in the recorded parameters.  The two back ends
 * use different biases so that picking the wrong one is visible too.
 *
 * des_ivec is only touched in CBC mode: ecb_crypt() leaves it
 * uninitialised, exactly as the original does.
 */
static int
run_backend(int which, char *buf, int len, struct desparams *desp)
{
	unsigned char st[8];
	int i, j;

	g_rec.backend = which;
	g_rec.ncalls++;
	g_rec.len = len;
	g_rec.dir = (int)desp->des_dir;
	g_rec.mode = (int)desp->des_mode;
	g_rec.bufptr = buf;
	std::memcpy(g_rec.key, desp->des_key, 8);

	const int cbc = (desp->des_mode == CBC);
	if (cbc) {
		std::memcpy(st, desp->des_ivec, 8);
		std::memcpy(g_rec.ivec_in, st, 8);
	} else {
		std::memset(st, 0, 8);
	}

	const unsigned char bias = (which == 1) ? 0x11 : 0xa7;
	const unsigned char dmix = (desp->des_dir == ENCRYPT) ? 0x5a : 0xc3;

	for (i = 0; i + 8 <= len; i += 8) {
		for (j = 0; j < 8; j++) {
			unsigned char v = (unsigned char)buf[i + j];
			v = (unsigned char)(v ^ st[j]);
			v = (unsigned char)(v + desp->des_key[j] + bias +
			    (unsigned char)j);
			v = (unsigned char)(v ^ dmix);
			buf[i + j] = (char)v;
		}
		if (cbc)
			std::memcpy(st, buf + i, 8);
	}
	if (cbc) {
		std::memcpy(desp->des_ivec, st, 8);
		std::memcpy(g_rec.ivec_out, st, 8);
	}

	/* Odd keys succeed, even keys report a driver failure. */
	g_rec.ret = (desp->des_key[0] & 1) ? 1 : 0;
	return g_rec.ret;
}

extern "C" int
_des_crypt_call(char *buf, int len, struct desparams *desp)
{
	return run_backend(1, buf, len, desp);
}

static int
harness_local_crypt(char *buf, unsigned len, struct desparams *desp)
{
	return run_backend(2, buf, (int)len, desp);
}

/* ------------------------------------------------------------------ */
/* Guarded buffer pairs                                               */
/* ------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;
static const std::size_t PRE = 64;
static const std::size_t POST = 64;

struct Arena {
	unsigned char *a;
	unsigned char *b;
	std::size_t cap;
	std::size_t total;
};

static void
arena_init(Arena &ar, std::size_t cap)
{
	ar.cap = cap;
	ar.total = PRE + cap + POST;
	ar.a = (unsigned char *)std::malloc(ar.total);
	ar.b = (unsigned char *)std::malloc(ar.total);
	if (ar.a == NULL || ar.b == NULL) {
		std::fprintf(stderr, "out of memory\n");
		std::exit(2);
	}
	std::memset(ar.a, GUARD, ar.total);
	std::memset(ar.b, GUARD, ar.total);
}

static char *
base_a(Arena &ar)
{
	return (char *)(ar.a + PRE);
}

static char *
base_b(Arena &ar)
{
	return (char *)(ar.b + PRE);
}

/*
 * Guard byte 0x7f everywhere, then identical pseudo-random input into the
 * data window of both buffers.
 */
static void
arena_fill(Arena &ar, std::uint32_t seed)
{
	std::memset(ar.a, GUARD, ar.total);
	std::uint32_t s = seed * 2654435761u + 1u;
	unsigned char *p = ar.a + PRE;
	std::size_t n = ar.cap;
	std::size_t i = 0;
	while (i + 4 <= n) {
		s = s * 1664525u + 1013904223u;
		std::uint32_t v = s ^ (s >> 13);
		p[i + 0] = (unsigned char)(v);
		p[i + 1] = (unsigned char)(v >> 8);
		p[i + 2] = (unsigned char)(v >> 16);
		p[i + 3] = (unsigned char)(v >> 24);
		i += 4;
	}
	while (i < n) {
		s = s * 1664525u + 1013904223u;
		p[i] = (unsigned char)(s >> 24);
		i++;
	}
	std::memcpy(ar.b, ar.a, ar.total);
}

/* Guard byte 0x7f everywhere, then the same 8 input bytes into both. */
static void
arena_set8(Arena &ar, const unsigned char *v8)
{
	std::memset(ar.a, GUARD, ar.total);
	std::memcpy(ar.a + PRE, v8, 8);
	std::memcpy(ar.b, ar.a, ar.total);
}

static bool
arena_equal(const Arena &ar)
{
	return std::memcmp(ar.a, ar.b, ar.total) == 0;
}

/* ------------------------------------------------------------------ */
/* Case driver                                                        */
/* ------------------------------------------------------------------ */

struct Stats {
	long cases;
	long fails;
};

static Stats st_cbc = { 0, 0 };
static Stats st_ecb = { 0, 0 };
static long reports_left = 20;

static Arena g_key;
static Arena g_ivec;

static void
hexdump8(const char *label, const unsigned char *p)
{
	std::printf("%s", label);
	for (int i = 0; i < 8; i++)
		std::printf(" %02x", p[i]);
	std::printf("\n");
}

static bool
run_case(Arena &data, const unsigned char *key8, const unsigned char *iv8,
    unsigned len, unsigned mode, bool use_local, bool is_cbc,
    std::uint32_t dataseed, const char *tag)
{
	arena_fill(data, dataseed);
	arena_set8(g_key, key8);
	arena_set8(g_ivec, iv8);

	if (use_local) {
		::__des_crypt_LOCAL = harness_local_crypt;
		port::__des_crypt_LOCAL = harness_local_crypt;
	} else {
		::__des_crypt_LOCAL = 0;
		port::__des_crypt_LOCAL = 0;
	}

	rec_reset();
	int rref = is_cbc
	    ? ref_cbc_crypt(base_a(g_key), base_a(data), len, mode,
	          base_a(g_ivec))
	    : ref_ecb_crypt(base_a(g_key), base_a(data), len, mode);
	CallRec rr = g_rec;
	long off_ref = (rr.bufptr != NULL)
	    ? (long)(rr.bufptr - base_a(data)) : -1L;

	rec_reset();
	int rprt = is_cbc
	    ? port::cbc_crypt(base_b(g_key), base_b(data), len, mode,
	          base_b(g_ivec))
	    : port::ecb_crypt(base_b(g_key), base_b(data), len, mode);
	CallRec rp = g_rec;
	long off_prt = (rp.bufptr != NULL)
	    ? (long)(rp.bufptr - base_b(data)) : -1L;

	bool ok = true;
	const char *why = "";

	if (rref != rprt) {
		ok = false; why = "return value";
	} else if (!arena_equal(data)) {
		ok = false; why = "data buffer";
	} else if (!arena_equal(g_key)) {
		ok = false; why = "key buffer";
	} else if (is_cbc && !arena_equal(g_ivec)) {
		ok = false; why = "ivec buffer";
	} else if (rr.backend != rp.backend) {
		ok = false; why = "backend selected";
	} else if (rr.ncalls != rp.ncalls) {
		ok = false; why = "backend call count";
	} else if (rr.backend != 0) {
		if (rr.len != rp.len) {
			ok = false; why = "desp len argument";
		} else if (rr.dir != rp.dir) {
			ok = false; why = "des_dir";
		} else if (rr.mode != rp.mode) {
			ok = false; why = "des_mode";
		} else if (rr.ret != rp.ret) {
			ok = false; why = "backend return";
		} else if (std::memcmp(rr.key, rp.key, 8) != 0) {
			ok = false; why = "des_key";
		} else if (off_ref != off_prt) {
			ok = false; why = "buf offset";
		} else if (is_cbc && rr.mode == CBC && rp.mode == CBC) {
			if (std::memcmp(rr.ivec_in, rp.ivec_in, 8) != 0) {
				ok = false; why = "des_ivec in";
			} else if (std::memcmp(rr.ivec_out, rp.ivec_out, 8) != 0) {
				ok = false; why = "des_ivec out";
			}
		}
	}

	Stats &st = is_cbc ? st_cbc : st_ecb;
	st.cases++;
	if (!ok) {
		st.fails++;
		if (reports_left > 0) {
			reports_left--;
			std::printf("FAIL [%s] %s len=%u mode=0x%08x local=%d: %s\n",
			    tag, is_cbc ? "cbc_crypt" : "ecb_crypt", len, mode,
			    (int)use_local, why);
			std::printf("      ret ref=%d port=%d  backend ref=%d port=%d"
			    "  ncalls ref=%d port=%d\n",
			    rref, rprt, rr.backend, rp.backend, rr.ncalls, rp.ncalls);
			std::printf("      dir ref=%d port=%d  mode ref=%d port=%d"
			    "  len ref=%d port=%d  off ref=%ld port=%ld\n",
			    rr.dir, rp.dir, rr.mode, rp.mode, rr.len, rp.len,
			    off_ref, off_prt);
			hexdump8("      key   ref:", rr.key);
			hexdump8("      key  port:", rp.key);
			if (is_cbc) {
				hexdump8("      ivin  ref:", rr.ivec_in);
				hexdump8("      ivin port:", rp.ivec_in);
				hexdump8("      ivout ref:", rr.ivec_out);
				hexdump8("      ivoutport:", rp.ivec_out);
			}
		}
	}
	return ok;
}

/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static void
rng_seed(std::uint64_t s)
{
	rng_state = s;
}

static std::uint32_t
rng_next(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return (std::uint32_t)(rng_state >> 32);
}

/* Zero, all ones, high bit set, both key parities. */
static const unsigned char hand_keys[][8] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
	{ 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
	{ 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 },
	{ 0x7f, 0x80, 0x00, 0xff, 0x01, 0xfe, 0x55, 0xaa },
	{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 },
};
static const int n_hand_keys = (int)(sizeof(hand_keys) / sizeof(hand_keys[0]));

static const unsigned char hand_ivecs[][8] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
	{ 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00 },
	{ 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f },
	{ 0xde, 0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67 },
};
static const int n_hand_ivecs = (int)(sizeof(hand_ivecs) / sizeof(hand_ivecs[0]));

/*
 * Lengths that fit the small arena.  Both sides of the "multiple of eight"
 * test are covered densely: 0 (empty), 1..7 (every non-zero remainder),
 * 8 (a single block), 9, then further multiples and near multiples.
 */
static const unsigned small_lens[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 16, 17, 23, 24, 25,
	31, 32, 33, 63, 64, 65, 127, 128, 129, 191, 192, 193,
	247, 248, 249, 255, 256
};
static const int n_small_lens = (int)(sizeof(small_lens) / sizeof(small_lens[0]));

/*
 * Lengths straddling DES_MAXDATA (8192) plus lengths whose conversion to int
 * is negative.  8192 must be accepted, 8200 must not; 8191 and 8193 are
 * rejected by the remainder test instead of by the size test.
 */
static const unsigned big_lens[] = {
	8176u, 8184u, 8191u, 8192u, 8193u, 8194u, 8199u, 8200u, 8208u,
	16384u, 32768u, 65528u, 65536u,
	0x80000000u, 0xfffffff8u, 0xffffffffu, 0xfffffff9u
};
static const int n_big_lens = (int)(sizeof(big_lens) / sizeof(big_lens[0]));

/*
 * Bit 0 is DES_DIRMASK and bit 1 is DES_DEVMASK; every other bit is ignored
 * by the original, so high garbage bits are covered as well.
 */
static const unsigned hand_modes[] = {
	0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u,
	0x80000000u, 0x80000001u, 0x80000002u, 0x80000003u,
	0xfffffffcu, 0xfffffffdu, 0xfffffffeu, 0xffffffffu
};
static const int n_hand_modes = (int)(sizeof(hand_modes) / sizeof(hand_modes[0]));

int
main(void)
{
	Arena small;
	Arena big;

	arena_init(small, 256);
	arena_init(big, 65536);
	arena_init(g_key, 8);
	arena_init(g_ivec, 8);

	std::uint32_t seed = 0x1234u;

	/* ---- hand written cases, small lengths ---- */
	for (int k = 0; k < n_hand_keys; k++) {
		for (int v = 0; v < n_hand_ivecs; v++) {
			for (int l = 0; l < n_small_lens; l++) {
				for (int m = 0; m < n_hand_modes; m++) {
					for (int loc = 0; loc < 2; loc++) {
						for (int cbc = 0; cbc < 2; cbc++) {
							run_case(small,
							    hand_keys[k],
							    hand_ivecs[v],
							    small_lens[l],
							    hand_modes[m],
							    loc != 0, cbc != 0,
							    seed++,
							    "hand-small");
						}
					}
				}
			}
		}
	}

	/* ---- hand written cases, lengths around DES_MAXDATA ---- */
	for (int k = 0; k < n_hand_keys; k++) {
		for (int l = 0; l < n_big_lens; l++) {
			for (int m = 0; m < 8; m++) {
				for (int loc = 0; loc < 2; loc++) {
					for (int cbc = 0; cbc < 2; cbc++) {
						run_case(big, hand_keys[k],
						    hand_ivecs[k % n_hand_ivecs],
						    big_lens[l], hand_modes[m],
						    loc != 0, cbc != 0,
						    seed++, "hand-big");
					}
				}
			}
		}
	}

	/* ---- fixed seed randomised sweep ---- */
	rng_seed(0x9e3779b97f4a7c15ull);

	const long SWEEP = 220000;
	for (long it = 0; it < SWEEP; it++) {
		unsigned char key8[8];
		unsigned char iv8[8];
		std::uint32_t r;

		r = rng_next();
		for (int i = 0; i < 4; i++)
			key8[i] = (unsigned char)(r >> (8 * i));
		r = rng_next();
		for (int i = 0; i < 4; i++)
			key8[4 + i] = (unsigned char)(r >> (8 * i));
		r = rng_next();
		for (int i = 0; i < 4; i++)
			iv8[i] = (unsigned char)(r >> (8 * i));
		r = rng_next();
		for (int i = 0; i < 4; i++)
			iv8[4 + i] = (unsigned char)(r >> (8 * i));

		unsigned len;
		switch (rng_next() % 3u) {
		case 0:
			len = 8u * (rng_next() % 33u);	/* exact blocks */
			break;
		case 1:
			len = rng_next() % 257u;	/* any remainder */
			break;
		default:
			/* cluster tightly on both sides of a block boundary */
			len = 8u * (rng_next() % 32u) + (rng_next() % 9u);
			if (len > 256u)
				len = 256u;
			break;
		}

		unsigned mode;
		if ((rng_next() & 1u) != 0u)
			mode = rng_next() % 8u;
		else
			mode = rng_next();

		bool use_local = ((rng_next() & 1u) != 0u);
		bool is_cbc = ((rng_next() & 1u) != 0u);

		run_case(small, key8, iv8, len, mode, use_local, is_cbc,
		    rng_next(), "random-small");
	}

	/* ---- randomised sweep on the DES_MAXDATA boundary ---- */
	const long BIG_SWEEP = 4000;
	for (long it = 0; it < BIG_SWEEP; it++) {
		unsigned char key8[8];
		unsigned char iv8[8];
		std::uint32_t r;

		r = rng_next();
		for (int i = 0; i < 4; i++)
			key8[i] = (unsigned char)(r >> (8 * i));
		r = rng_next();
		for (int i = 0; i < 4; i++)
			key8[4 + i] = (unsigned char)(r >> (8 * i));
		r = rng_next();
		for (int i = 0; i < 4; i++)
			iv8[i] = (unsigned char)(r >> (8 * i));
		r = rng_next();
		for (int i = 0; i < 4; i++)
			iv8[4 + i] = (unsigned char)(r >> (8 * i));

		unsigned len;
		switch (rng_next() % 5u) {
		case 0:
			len = 8192u - 64u + (rng_next() % 129u);
			break;
		case 1:
			len = rng_next() % 8193u;
			break;
		case 2:
			len = 8192u + (rng_next() % 64u);
			break;
		case 3:
			len = 65536u - (rng_next() % 64u);
			break;
		default:
			len = 0xffffff00u + (rng_next() % 256u);
			break;
		}

		unsigned mode = ((rng_next() & 1u) != 0u)
		    ? (rng_next() % 8u) : rng_next();
		bool use_local = ((rng_next() & 1u) != 0u);
		bool is_cbc = ((rng_next() & 1u) != 0u);

		run_case(big, key8, iv8, len, mode, use_local, is_cbc,
		    rng_next(), "random-big");
	}

	long cases = st_cbc.cases + st_ecb.cases;
	long fails = st_cbc.fails + st_ecb.fails;

	std::printf("\n");
	std::printf("%-38s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------------"
	    "----------------\n");
	std::printf("%-38s %12ld %12ld\n", "cbc_crypt", st_cbc.cases, st_cbc.fails);
	std::printf("%-38s %12ld %12ld\n", "ecb_crypt", st_ecb.cases, st_ecb.fails);
	std::printf("%-38s %12ld %12ld\n",
	    "common_crypt (via cbc_crypt/ecb_crypt)", cases, fails);
	std::printf("--------------------------------------------------"
	    "----------------\n");
	std::printf("%-38s %12ld %12ld\n", "TOTAL", cases, fails);
	std::printf("\n%s\n", fails == 0 ? "PASS" : "FAIL");

	return fails == 0 ? 0 : 1;
}
