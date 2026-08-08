/*
 * harness.cpp -- differential test for PBSD batch b0029s4.
 *
 * Compares pbsd::sys_security_mac_veriexec::b0029s4::SHA1_Final against
 * ref_SHA1_Final from oracle.c.
 *
 * SHA1_Final(unsigned char *buf, void *ctx) returns void; its entire
 * observable effect is what sha1_result() writes through the two pointers it
 * is handed.  So every case compares, byte for byte:
 *   - the ENTIRE output buffer, including the bytes past the 20-byte digest
 *     window, so a digest written at the wrong offset is caught;
 *   - the ENTIRE context region, which is several times sizeof(SHA1_CTX) and
 *     holds the context at a non-zero offset, so a context pointer that is
 *     off by +/- one struct or one byte lands on guard bytes that are also
 *     compared.
 * Both sides are given freshly guard-filled (0x7f) regions and identical
 * input bytes.  Only offsets from the region bases are ever used, never raw
 * addresses.  Neither function returns a pointer and neither is a stateful
 * iterator, so those comparison modes do not apply here.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.sys.security.mac.veriexec.b0029s4;

namespace prt = pbsd::sys_security_mac_veriexec::b0029s4;

extern "C" {

struct sha1_ctxt {
	union {
		std::uint8_t	b8[20];
		std::uint32_t	b32[5];
	} h;
	union {
		std::uint8_t	b8[8];
		std::uint64_t	b64[1];
	} c;
	union {
		std::uint8_t	b8[64];
		std::uint32_t	b32[16];
	} m;
	std::uint8_t	count;
};

void sha1_init(struct sha1_ctxt *);
void sha1_loop(struct sha1_ctxt *, const std::uint8_t *, std::size_t);
void ref_SHA1_Final(unsigned char *buf, void *ctx);

} /* extern "C" */

namespace {

constexpr std::size_t CTXSZ = sizeof(struct sha1_ctxt);
constexpr std::size_t CTXREG = 8 * CTXSZ;
constexpr std::size_t BUFSZ = 64;
constexpr std::size_t DIGESTLEN = 20;
constexpr unsigned char GUARD = 0x7f;

/* Context offsets are multiples of 8 so the b64 member stays aligned, and are
 * bounded so that a context pointer off by +/- two whole structs still lands
 * inside the region instead of running off the end. */
constexpr std::size_t CTX_OFF_MIN = 2 * CTXSZ;
constexpr std::size_t CTX_OFF_MAX = CTXREG - 3 * CTXSZ;
constexpr std::size_t BUF_OFF_MAX = BUFSZ - DIGESTLEN;

alignas(16) unsigned char regA[CTXREG];
alignas(16) unsigned char regB[CTXREG];
unsigned char bufA[BUFSZ];
unsigned char bufB[BUFSZ];

struct Group {
	const char	*name;
	unsigned long	 cases;
	unsigned long	 failures;
};

Group g_edge = { "hand-written edge cases", 0, 0 };
Group g_count = { "context count sweep 0..255", 0, 0 };
Group g_len = { "message length sweep 0..320", 0, 0 };
Group g_offs = { "buffer/context offset sweep", 0, 0 };
Group g_kat = { "known-answer digests", 0, 0 };
Group g_rand = { "randomised sweep", 0, 0 };

Group *const g_all[] = { &g_edge, &g_count, &g_len, &g_offs, &g_kat, &g_rand };

unsigned long g_reports;
unsigned long g_wrote;		/* cases where the digest window changed */
unsigned long g_distinct_marker;/* running mix of every digest produced */

void
dump(const char *tag, const unsigned char *p, std::size_t n, std::size_t from,
    std::size_t to)
{
	std::printf("      %s[%zu..%zu]:", tag, from, to);
	for (std::size_t i = from; i < to && i < n; i++)
		std::printf(" %02x", p[i]);
	std::printf("\n");
}

void
report(const char *label, std::size_t ctx_off, std::size_t buf_off,
    bool buf_bad, bool reg_bad)
{
	if (++g_reports > 12) {
		if (g_reports == 13)
			std::printf("    ... further mismatch details "
			    "suppressed\n");
		return;
	}
	std::printf("  MISMATCH %s (ctx_off=%zu buf_off=%zu)%s%s\n", label,
	    ctx_off, buf_off, buf_bad ? " [output buffer]" : "",
	    reg_bad ? " [context region]" : "");
	if (buf_bad) {
		std::size_t i = 0;
		while (i < BUFSZ && bufA[i] == bufB[i])
			i++;
		std::printf("    first differing output byte at offset %zu "
		    "(port=%02x ref=%02x)\n", i, bufA[i], bufB[i]);
		dump("port", bufA, BUFSZ, 0, BUFSZ);
		dump("ref ", bufB, BUFSZ, 0, BUFSZ);
	}
	if (reg_bad) {
		std::size_t i = 0;
		while (i < CTXREG && regA[i] == regB[i])
			i++;
		std::printf("    first differing context byte at region "
		    "offset %zu (%+lld from ctx, port=%02x ref=%02x)\n", i,
		    (long long)i - (long long)ctx_off, regA[i], regB[i]);
		std::size_t from = i >= 8 ? i - 8 : 0;
		dump("port", regA, CTXREG, from, from + 24);
		dump("ref ", regB, CTXREG, from, from + 24);
	}
}

/*
 * One differential case: identical context bytes at ctx_off in a guard-filled
 * region, identical guard-filled output buffers, digest written at buf_off.
 */
bool
run_case(Group &grp, const char *label, const unsigned char *ctx_bytes,
    std::size_t ctx_off, std::size_t buf_off)
{
	std::memset(regA, GUARD, CTXREG);
	std::memset(regB, GUARD, CTXREG);
	std::memcpy(regA + ctx_off, ctx_bytes, CTXSZ);
	std::memcpy(regB + ctx_off, ctx_bytes, CTXSZ);
	std::memset(bufA, GUARD, BUFSZ);
	std::memset(bufB, GUARD, BUFSZ);

	prt::SHA1_Final(bufA + buf_off, static_cast<void *>(regA + ctx_off));
	ref_SHA1_Final(bufB + buf_off, static_cast<void *>(regB + ctx_off));

	bool buf_bad = std::memcmp(bufA, bufB, BUFSZ) != 0;
	bool reg_bad = std::memcmp(regA, regB, CTXREG) != 0;

	grp.cases++;
	if (buf_bad || reg_bad) {
		grp.failures++;
		report(label, ctx_off, buf_off, buf_bad, reg_bad);
		return false;
	}

	for (std::size_t i = 0; i < DIGESTLEN; i++) {
		if (bufB[buf_off + i] != GUARD) {
			g_wrote++;
			break;
		}
	}
	for (std::size_t i = 0; i < DIGESTLEN; i++)
		g_distinct_marker = g_distinct_marker * 1000003u +
		    bufB[buf_off + i];
	return true;
}

/* Every case is run at several buffer offsets, including 0 and the last
 * offset at which the digest still fits, so an output pointer nudged either
 * way is observable. */
const std::size_t buf_offs[] = { 0, 1, 2, 3, 4, 7, 8, 13, 16, 31,
	BUF_OFF_MAX - 1, BUF_OFF_MAX };
const std::size_t ctx_offs[] = { CTX_OFF_MIN, CTX_OFF_MIN + 8,
	CTX_OFF_MIN + CTXSZ, CTX_OFF_MAX - 8, CTX_OFF_MAX };

void
run_all_offsets(Group &grp, const char *label, const unsigned char *ctx_bytes)
{
	for (std::size_t bo : buf_offs)
		for (std::size_t co : ctx_offs)
			run_case(grp, label, ctx_bytes, co, bo);
}

void
make_ctx_message(unsigned char *out, const unsigned char *data,
    std::size_t len)
{
	struct sha1_ctxt c;

	sha1_init(&c);
	if (len != 0)
		sha1_loop(&c, reinterpret_cast<const std::uint8_t *>(data),
		    len);
	std::memcpy(out, &c, CTXSZ);
}

std::uint64_t g_seed = 0x9e3779b97f4a7c15ull;

std::uint64_t
rnd(void)
{
	std::uint64_t z = (g_seed += 0x9e3779b97f4a7c15ull);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return z ^ (z >> 31);
}

std::size_t
rnd_below(std::size_t n)
{
	return static_cast<std::size_t>(rnd() % n);
}

void
hexof(char *dst, const unsigned char *p, std::size_t n)
{
	static const char hx[] = "0123456789abcdef";

	for (std::size_t i = 0; i < n; i++) {
		dst[2 * i] = hx[p[i] >> 4];
		dst[2 * i + 1] = hx[p[i] & 0x0f];
	}
	dst[2 * n] = '\0';
}

/*
 * Known-answer check: proves the contexts fed to the port are real SHA-1
 * states and that the port's output is the true digest, not merely equal to
 * whatever the oracle produced.
 */
void
run_kat(const char *msg, std::size_t len, const char *expect)
{
	unsigned char ctx[CTXSZ];
	char got[2 * DIGESTLEN + 1];

	make_ctx_message(ctx, reinterpret_cast<const unsigned char *>(msg),
	    len);
	std::size_t bo = 5, co = CTX_OFF_MIN + 8;
	bool ok = run_case(g_kat, "kat", ctx, co, bo);

	g_kat.cases++;
	hexof(got, bufA + bo, DIGESTLEN);
	if (!ok || std::strcmp(got, expect) != 0) {
		g_kat.failures++;
		std::printf("  MISMATCH kat len=%zu: port digest %s, "
		    "expected %s\n", len, got, expect);
	}
}

void
edge_cases(void)
{
	unsigned char ctx[CTXSZ];

	/* Uniform-byte contexts, including all high-bit and all NUL. */
	static const unsigned char fills[] = { 0x00, 0x01, 0x7f, 0x80, 0x81,
	    0xfe, 0xff };
	for (unsigned char f : fills) {
		std::memset(ctx, f, CTXSZ);
		run_all_offsets(g_edge, "uniform-fill ctx", ctx);
	}

	/* Ascending and descending byte ladders. */
	for (std::size_t i = 0; i < CTXSZ; i++)
		ctx[i] = static_cast<unsigned char>(i);
	run_all_offsets(g_edge, "ascending ctx", ctx);
	for (std::size_t i = 0; i < CTXSZ; i++)
		ctx[i] = static_cast<unsigned char>(0xff - i);
	run_all_offsets(g_edge, "descending ctx", ctx);
	for (std::size_t i = 0; i < CTXSZ; i++)
		ctx[i] = static_cast<unsigned char>(0x80 | (i & 0x7f));
	run_all_offsets(g_edge, "high-bit ctx", ctx);

	/* A freshly initialised context: the empty-message case. */
	make_ctx_message(ctx, nullptr, 0);
	run_all_offsets(g_edge, "sha1_init only", ctx);

	/* Single byte messages at both ends of the byte range. */
	static const unsigned char one[][1] = { { 0x00 }, { 0x01 }, { 0x7f },
	    { 0x80 }, { 0xff } };
	for (const auto &b : one) {
		make_ctx_message(ctx, b, 1);
		run_all_offsets(g_edge, "single byte message", ctx);
	}

	/* NUL-heavy and high-bit messages at the padding boundaries. */
	static const std::size_t bounds[] = { 0, 1, 54, 55, 56, 57, 63, 64,
	    65, 111, 112, 119, 120, 127, 128, 129, 255, 256 };
	unsigned char msg[320];
	static const unsigned char pats[] = { 0x00, 0x80, 0xff, 0x7f };
	for (unsigned char p : pats) {
		std::memset(msg, p, sizeof msg);
		for (std::size_t n : bounds) {
			make_ctx_message(ctx, msg, n);
			run_all_offsets(g_edge, "boundary-length message",
			    ctx);
		}
	}

	/* Counter field extremes with an otherwise fixed context: the bit
	 * count feeds the padded tail, so its endianness handling shows up in
	 * the digest. */
	static const std::uint64_t bits[] = { 0, 1, 7, 8, 0xffull, 0x100ull,
	    0xffffffffull, 0x100000000ull, 0x00ffffffffffff00ull,
	    0xffffffffffffffffull };
	for (std::uint64_t bc : bits) {
		std::memset(ctx, 0xa5, CTXSZ);
		struct sha1_ctxt *cp =
		    reinterpret_cast<struct sha1_ctxt *>(ctx);
		cp->c.b64[0] = bc;
		cp->count = 0;
		run_all_offsets(g_edge, "bit-count extreme", ctx);
		cp->count = 63;
		run_all_offsets(g_edge, "bit-count extreme, count=63", ctx);
	}
}

/*
 * Sweep every value of the one-byte count field.  count % 64 in 57..63 takes
 * the short-tail path inside the padding, count % 64 == 0 takes the aligned
 * path, so both sides of that boundary are covered for every offset pair.
 */
void
count_sweep(void)
{
	unsigned char ctx[CTXSZ];

	for (unsigned v = 0; v < 256; v++) {
		for (std::size_t i = 0; i < CTXSZ; i++)
			ctx[i] = static_cast<unsigned char>(
			    (i * 7u + v * 31u) & 0xff);
		struct sha1_ctxt *cp =
		    reinterpret_cast<struct sha1_ctxt *>(ctx);
		cp->count = static_cast<std::uint8_t>(v);
		cp->c.b64[0] = static_cast<std::uint64_t>(v) * 8u;
		for (std::size_t bo : buf_offs)
			run_case(g_count, "count sweep", ctx,
			    CTX_OFF_MIN + 8, bo);
		run_case(g_count, "count sweep", ctx, CTX_OFF_MAX, 3);
	}
}

void
length_sweep(void)
{
	unsigned char ctx[CTXSZ];
	unsigned char msg[321];

	for (std::size_t i = 0; i < sizeof msg; i++)
		msg[i] = static_cast<unsigned char>((i * 251u + 0x80u) & 0xff);
	for (std::size_t n = 0; n <= 320; n++) {
		make_ctx_message(ctx, msg, n);
		run_case(g_len, "length sweep", ctx, CTX_OFF_MIN, 0);
		run_case(g_len, "length sweep", ctx, CTX_OFF_MIN + 8, 1);
		run_case(g_len, "length sweep", ctx, CTX_OFF_MAX, BUF_OFF_MAX);
	}
}

/* Exhaustive pass over the offset pairs, so any misuse of either pointer is
 * visible at every alignment the digest can land on. */
void
offset_sweep(void)
{
	unsigned char ctx[CTXSZ];

	for (std::size_t i = 0; i < CTXSZ; i++)
		ctx[i] = static_cast<unsigned char>((i * 173u + 11u) & 0xff);
	for (std::size_t bo = 0; bo <= BUF_OFF_MAX; bo++) {
		for (std::size_t co = CTX_OFF_MIN; co <= CTX_OFF_MAX;
		    co += 8)
			run_case(g_offs, "offset sweep", ctx, co, bo);
	}
}

void
random_sweep(unsigned long iters)
{
	unsigned char ctx[CTXSZ];
	unsigned char msg[512];

	for (unsigned long it = 0; it < iters; it++) {
		unsigned mode = static_cast<unsigned>(rnd() & 3);

		if (mode == 0) {
			/* Wholly random context bytes. */
			for (std::size_t i = 0; i < CTXSZ; i++)
				ctx[i] = static_cast<unsigned char>(rnd());
		} else if (mode == 1) {
			/* Random context, then a random count/bit-count so
			 * every padding path is reached often. */
			for (std::size_t i = 0; i < CTXSZ; i++)
				ctx[i] = static_cast<unsigned char>(rnd());
			struct sha1_ctxt *cp =
			    reinterpret_cast<struct sha1_ctxt *>(ctx);
			cp->count = static_cast<std::uint8_t>(rnd_below(256));
			cp->c.b64[0] = rnd();
		} else {
			/* Real states from random messages: uniform random
			 * bytes, or biased to NUL and to the 0x80..0xff
			 * high-bit range. */
			std::size_t n = rnd_below(sizeof msg + 1);
			unsigned bias = static_cast<unsigned>(rnd() % 4);
			for (std::size_t i = 0; i < n; i++) {
				unsigned char b =
				    static_cast<unsigned char>(rnd());
				if (bias == 1)
					b = 0x00;
				else if (bias == 2)
					b |= 0x80;
				else if (bias == 3)
					b = (b & 1) ? 0x00 : 0xff;
				msg[i] = b;
			}
			make_ctx_message(ctx, msg, n);
		}

		std::size_t co = CTX_OFF_MIN +
		    8 * rnd_below((CTX_OFF_MAX - CTX_OFF_MIN) / 8 + 1);
		std::size_t bo = rnd_below(BUF_OFF_MAX + 1);
		run_case(g_rand, "random", ctx, co, bo);
	}
}

} /* namespace */

int
main(void)
{
	std::printf("PBSD b0029s4 differential test\n");
	std::printf("  port   : module pbsd.sys.security.mac.veriexec.b0029s4"
	    "\n");
	std::printf("  oracle : oracle.c ref_ functions\n");
	std::printf("  sizeof(SHA1_CTX)=%zu context region=%zu bytes "
	    "output buffer=%zu bytes guard=0x%02x\n\n", CTXSZ, CTXREG, BUFSZ,
	    GUARD);

	edge_cases();
	count_sweep();
	length_sweep();
	offset_sweep();
	run_kat("", 0, "da39a3ee5e6b4b0d3255bfef95601890afd80709");
	run_kat("abc", 3, "a9993e364706816aba3e25717850c26c9cd0d89d");
	run_kat("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
	    "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
	random_sweep(250000);

	unsigned long cases = 0, failures = 0;

	std::printf("group breakdown\n");
	for (const Group *g : g_all) {
		std::printf("  %-32s %9lu cases %9lu failures\n", g->name,
		    g->cases, g->failures);
		cases += g->cases;
		failures += g->failures;
	}

	std::printf("\n%-34s %9s %9s\n", "function", "cases", "failures");
	std::printf("%-34s %9s %9s\n", "----------------------------------",
	    "---------", "---------");
	std::printf("%-34s %9lu %9lu\n", "SHA1_Final", cases, failures);
	std::printf("%-34s %9lu %9lu\n", "TOTAL", cases, failures);
	std::printf("\noutput digest mix 0x%08lx, cases that wrote a digest "
	    "%lu\n", g_distinct_marker & 0xffffffffull, g_wrote);

	/* A run in which nothing was ever written would compare guard bytes
	 * against guard bytes and prove nothing. */
	if (cases == 0 || g_wrote == 0) {
		std::printf("\nFAIL: harness never observed a digest write\n");
		return 1;
	}
	if (failures != 0) {
		std::printf("\nFAIL: %lu of %lu cases diverged\n", failures,
		    cases);
		return 1;
	}
	std::printf("\nPASS: all %lu cases matched\n", cases);
	return 0;
}
