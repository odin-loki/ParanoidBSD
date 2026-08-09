#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

import pbsd.lib.libc.secure.b0240;

namespace port = pbsd::lib_libc_secure::b0240;

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t BUF_CAP = 256;
constexpr unsigned RANDOM_ITERS = 200000;

extern "C" {
extern volatile int ref_chk_fail_called;
extern volatile int ref_stack_chk_fail_called;

size_t ref___strlcpy_chk(char *dst, const char *src, size_t dsize,
    size_t dbufsize);
void *ref___mempcpy_chk(void *dst, const void *src, size_t len, size_t slen);
void *ref___memset_chk(void *dst, int val, size_t len, size_t slen);
void ref___stack_chk_fail_local_hidden(void);
int __ssp_overlap(const void *s1, const void *s2, size_t n);
}

struct Stats {
	const char *name;
	unsigned long cases = 0;
	unsigned long failures = 0;
};

Stats g_stats[4];
int g_stat_idx = 0;

Stats &cur_stats(const char *name)
{
	g_stats[g_stat_idx].name = name;
	return g_stats[g_stat_idx++];
}

void fill_guard(unsigned char *p, size_t n)
{
	for (size_t i = 0; i < n; i++)
		p[i] = GUARD;
}

bool buffers_equal(const unsigned char *a, const unsigned char *b, size_t n)
{
	for (size_t i = 0; i < n; i++)
		if (a[i] != b[i])
			return false;
	return true;
}

struct Rng {
	uint32_t s;

	explicit Rng(uint32_t seed) : s(seed) {}

	uint32_t next()
	{
		s ^= s << 13;
		s ^= s >> 17;
		s ^= s << 5;
		return s;
	}

	size_t next_size(size_t maxv)
	{
		if (maxv == 0)
			return 0;
		return static_cast<size_t>(next() % (maxv + 1));
	}

	int next_int()
	{
		return static_cast<int>(next());
	}
};

void reset_chk_flags()
{
	ref_chk_fail_called = 0;
	ref_stack_chk_fail_called = 0;
}

bool check_chk_fail(bool expect, int got_ref, int got_port, Stats &st)
{
	st.cases++;
	if (got_ref != (expect ? 1 : 0) || got_port != (expect ? 1 : 0)) {
		st.failures++;
		return false;
	}
	return true;
}

void test_strlcpy_chk_hand(Stats &st)
{
	struct Case {
		size_t dsize;
		size_t dbufsize;
		const char *src;
		bool expect_fail;
	};

	const Case hand[] = {
		{0, 0, "", false},
		{0, 0, "a", false},
		{0, 1, "hello", false},
		{1, 1, "", false},
		{1, 1, "a", false},
		{1, 1, "ab", false},
		{2, 2, "a", false},
		{2, 2, "ab", false},
		{2, 2, "abc", false},
		{8, 8, "", false},
		{8, 8, "x", false},
		{8, 8, "abcdefgh", false},
		{8, 8, "abcdefghi", false},
		{8, 8, "\xff\xfe\xfd", false},
		{16, 16, "\x80\x81\x82\x83", false},
		{4, 8, "test", false},
		{8, 4, "short", true},
		{5, 4, "fail", true},
		{1, 0, "", true},
		{2, 1, "a", true},
		{BUF_CAP, 4, "overflow", true},
	};

	for (const auto &c : hand) {
		unsigned char dst_ref[BUF_CAP];
		unsigned char dst_port[BUF_CAP];
		char src_buf[BUF_CAP];

		fill_guard(dst_ref, BUF_CAP);
		fill_guard(dst_port, BUF_CAP);
		strncpy(src_buf, c.src, BUF_CAP - 1);
		src_buf[BUF_CAP - 1] = '\0';

		reset_chk_flags();
		size_t rret = ref___strlcpy_chk(reinterpret_cast<char *>(dst_ref),
		    src_buf, c.dsize, c.dbufsize);
		int rfail = ref_chk_fail_called;

		reset_chk_flags();
		size_t pret = port::__strlcpy_chk(reinterpret_cast<char *>(dst_port),
		    src_buf, c.dsize, c.dbufsize);
		int pfail = ref_chk_fail_called;

		st.cases++;
		if (rret != pret || !buffers_equal(dst_ref, dst_port, BUF_CAP) ||
		    rfail != pfail || rfail != (c.expect_fail ? 1 : 0))
			st.failures++;
	}
}

void test_strlcpy_chk_random(Stats &st, Rng &rng)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++) {
		size_t dbufsize = rng.next_size(64);
		size_t dsize = rng.next_size(64);
		if ((rng.next() & 7u) == 0u)
			dsize = dbufsize + rng.next_size(8);
		if ((rng.next() & 7u) == 1u && dbufsize > 0)
			dsize = dbufsize - 1;
		if ((rng.next() & 7u) == 2u)
			dsize = dbufsize;

		unsigned char dst_ref[BUF_CAP];
		unsigned char dst_port[BUF_CAP];
		char src_buf[64];
		size_t slen = rng.next_size(63);
		for (size_t j = 0; j < slen; j++)
			src_buf[j] = static_cast<char>(rng.next() & 0xffu);
		src_buf[slen] = '\0';

		fill_guard(dst_ref, BUF_CAP);
		fill_guard(dst_port, BUF_CAP);

		bool expect_fail = dsize > dbufsize;

		reset_chk_flags();
		size_t rret = ref___strlcpy_chk(reinterpret_cast<char *>(dst_ref),
		    src_buf, dsize, dbufsize);
		int rfail = ref_chk_fail_called;

		reset_chk_flags();
		size_t pret = port::__strlcpy_chk(reinterpret_cast<char *>(dst_port),
		    src_buf, dsize, dbufsize);
		int pfail = ref_chk_fail_called;

		st.cases++;
		if (rret != pret || !buffers_equal(dst_ref, dst_port, BUF_CAP) ||
		    rfail != pfail || rfail != (expect_fail ? 1 : 0))
			st.failures++;
	}
}

void test_mempcpy_chk_hand(Stats &st)
{
	struct Case {
		size_t len;
		size_t slen;
		bool overlap;
		bool expect_fail;
	};

	const Case hand[] = {
		{0, 0, false, false},
		{0, 8, false, false},
		{1, 1, false, false},
		{4, 4, false, false},
		{8, 8, false, false},
		{16, 16, false, false},
		{32, 32, false, false},
		{8, 16, false, false},
		{16, 8, false, true},
		{9, 8, false, true},
		{1, 0, false, true},
		{4, 3, false, true},
	};

	for (const auto &c : hand) {
		unsigned char dst_ref[BUF_CAP];
		unsigned char dst_port[BUF_CAP];
		unsigned char src_buf[BUF_CAP];

		for (size_t j = 0; j < BUF_CAP; j++)
			src_buf[j] = static_cast<unsigned char>(0x80u + (j & 0x7fu));

		fill_guard(dst_ref, BUF_CAP);
		fill_guard(dst_port, BUF_CAP);

		const void *src_ref = src_buf;
		const void *src_port = src_buf;
		if (c.overlap) {
			for (size_t j = 0; j < BUF_CAP; j++) {
				unsigned char v =
				    static_cast<unsigned char>(0xa0u + (j & 0x3fu));
				dst_ref[j] = v;
				dst_port[j] = v;
			}
			src_ref = dst_ref + 2;
			src_port = dst_port + 2;
		}

		reset_chk_flags();
		void *rptr = ref___mempcpy_chk(dst_ref, src_ref, c.len, c.slen);
		size_t roff = static_cast<unsigned char *>(rptr) - dst_ref;
		int rfail = ref_chk_fail_called;

		reset_chk_flags();
		void *pptr = port::__mempcpy_chk(dst_port, src_port, c.len, c.slen);
		size_t poff = static_cast<unsigned char *>(pptr) - dst_port;
		int pfail = ref_chk_fail_called;

		st.cases++;
		if (roff != poff || !buffers_equal(dst_ref, dst_port, BUF_CAP) ||
		    rfail != pfail || rfail != (c.expect_fail ? 1 : 0))
			st.failures++;
	}

	/* explicit overlap boundary cases */
	for (size_t off = 0; off < 8; off++) {
		for (size_t len = 0; len < 8; len++) {
			unsigned char dst_ref[BUF_CAP];
			unsigned char dst_port[BUF_CAP];

			for (size_t j = 0; j < BUF_CAP; j++) {
				unsigned char v =
				    static_cast<unsigned char>(0xa0u + (j & 0x3fu));
				dst_ref[j] = v;
				dst_port[j] = v;
			}

			const void *src_ref = dst_ref + off;
			const void *src_port = dst_port + off;
			size_t slen = BUF_CAP;
			bool expect_fail = __ssp_overlap(src_ref, dst_ref, len);

			reset_chk_flags();
			void *rptr = ref___mempcpy_chk(dst_ref, src_ref, len, slen);
			size_t roff = static_cast<unsigned char *>(rptr) - dst_ref;
			int rfail = ref_chk_fail_called;

			reset_chk_flags();
			void *pptr = port::__mempcpy_chk(dst_port, src_port, len, slen);
			size_t poff = static_cast<unsigned char *>(pptr) - dst_port;
			int pfail = ref_chk_fail_called;

			st.cases++;
			if (roff != poff || !buffers_equal(dst_ref, dst_port, BUF_CAP) ||
			    rfail != pfail || rfail != (expect_fail ? 1 : 0))
				st.failures++;
		}
	}
}

void test_mempcpy_chk_random(Stats &st, Rng &rng)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++) {
		size_t slen = rng.next_size(64);
		size_t len = rng.next_size(64);
		if ((rng.next() & 7u) == 0u)
			len = slen + rng.next_size(8);
		if ((rng.next() & 7u) == 1u && slen > 0)
			len = slen - 1;
		if ((rng.next() & 7u) == 2u)
			len = slen;

		unsigned char dst_ref[BUF_CAP];
		unsigned char dst_port[BUF_CAP];
		unsigned char src_buf[BUF_CAP];

		for (size_t j = 0; j < BUF_CAP; j++)
			src_buf[j] = static_cast<unsigned char>(rng.next() & 0xffu);

		fill_guard(dst_ref, BUF_CAP);
		fill_guard(dst_port, BUF_CAP);

		const void *src_ref = src_buf;
		const void *src_port = src_buf;
		bool overlap = false;
		if ((rng.next() & 3u) == 0u) {
			size_t off = rng.next_size(32);
			for (size_t j = 0; j < BUF_CAP; j++) {
				unsigned char v =
				    static_cast<unsigned char>(rng.next() & 0xffu);
				dst_ref[j] = v;
				dst_port[j] = v;
			}
			src_ref = dst_ref + off;
			src_port = dst_port + off;
			overlap = __ssp_overlap(src_ref, dst_ref, len);
		}

		bool expect_fail = (len > slen) || overlap;

		reset_chk_flags();
		void *rptr = ref___mempcpy_chk(dst_ref, src_ref, len, slen);
		size_t roff = static_cast<unsigned char *>(rptr) - dst_ref;
		int rfail = ref_chk_fail_called;

		reset_chk_flags();
		void *pptr = port::__mempcpy_chk(dst_port, src_port, len, slen);
		size_t poff = static_cast<unsigned char *>(pptr) - dst_port;
		int pfail = ref_chk_fail_called;

		st.cases++;
		if (roff != poff || !buffers_equal(dst_ref, dst_port, BUF_CAP) ||
		    rfail != pfail || rfail != (expect_fail ? 1 : 0))
			st.failures++;
	}
}

void test_memset_chk_hand(Stats &st)
{
	struct Case {
		int val;
		size_t len;
		size_t slen;
		bool expect_fail;
	};

	const Case hand[] = {
		{0, 0, 0, false},
		{0, 0, 8, false},
		{'a', 1, 1, false},
		{0xff, 8, 8, false},
		{0x80, 16, 16, false},
		{-1, 4, 4, false},
		{127, 32, 32, false},
		{0, 8, 16, false},
		{0x55, 16, 8, true},
		{0, 9, 8, true},
		{0x7f, 1, 0, true},
		{-128, 4, 3, true},
	};

	for (const auto &c : hand) {
		unsigned char dst_ref[BUF_CAP];
		unsigned char dst_port[BUF_CAP];

		fill_guard(dst_ref, BUF_CAP);
		fill_guard(dst_port, BUF_CAP);

		reset_chk_flags();
		void *rptr = ref___memset_chk(dst_ref, c.val, c.len, c.slen);
		size_t roff = static_cast<unsigned char *>(rptr) - dst_ref;
		int rfail = ref_chk_fail_called;

		reset_chk_flags();
		void *pptr = port::__memset_chk(dst_port, c.val, c.len, c.slen);
		size_t poff = static_cast<unsigned char *>(pptr) - dst_port;
		int pfail = ref_chk_fail_called;

		st.cases++;
		if (roff != poff || !buffers_equal(dst_ref, dst_port, BUF_CAP) ||
		    rfail != pfail || rfail != (c.expect_fail ? 1 : 0))
			st.failures++;
	}
}

void test_memset_chk_random(Stats &st, Rng &rng)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++) {
		size_t slen = rng.next_size(64);
		size_t len = rng.next_size(64);
		if ((rng.next() & 7u) == 0u)
			len = slen + rng.next_size(8);
		if ((rng.next() & 7u) == 1u && slen > 0)
			len = slen - 1;
		if ((rng.next() & 7u) == 2u)
			len = slen;

		int val = rng.next_int();

		unsigned char dst_ref[BUF_CAP];
		unsigned char dst_port[BUF_CAP];

		fill_guard(dst_ref, BUF_CAP);
		fill_guard(dst_port, BUF_CAP);

		bool expect_fail = len > slen;

		reset_chk_flags();
		void *rptr = ref___memset_chk(dst_ref, val, len, slen);
		size_t roff = static_cast<unsigned char *>(rptr) - dst_ref;
		int rfail = ref_chk_fail_called;

		reset_chk_flags();
		void *pptr = port::__memset_chk(dst_port, val, len, slen);
		size_t poff = static_cast<unsigned char *>(pptr) - dst_port;
		int pfail = ref_chk_fail_called;

		st.cases++;
		if (roff != poff || !buffers_equal(dst_ref, dst_port, BUF_CAP) ||
		    rfail != pfail || rfail != (expect_fail ? 1 : 0))
			st.failures++;
	}
}

void test_stack_chk_fail_local_hidden(Stats &st)
{
	for (int i = 0; i < 16; i++) {
		reset_chk_flags();
		ref___stack_chk_fail_local_hidden();
		int rfail = ref_stack_chk_fail_called;

		reset_chk_flags();
		port::__stack_chk_fail_local_hidden();
		int pfail = ref_stack_chk_fail_called;

		st.cases++;
		if (rfail != 1 || pfail != 1 || rfail != pfail)
			st.failures++;
	}
}

} // namespace

int main()
{
	Rng rng(0xB0240u);

	auto &st_strlcpy = cur_stats("__strlcpy_chk");
	test_strlcpy_chk_hand(st_strlcpy);
	test_strlcpy_chk_random(st_strlcpy, rng);

	auto &st_mempcpy = cur_stats("__mempcpy_chk");
	test_mempcpy_chk_hand(st_mempcpy);
	test_mempcpy_chk_random(st_mempcpy, rng);

	auto &st_memset = cur_stats("__memset_chk");
	test_memset_chk_hand(st_memset);
	test_memset_chk_random(st_memset, rng);

	auto &st_stack = cur_stats("__stack_chk_fail_local_hidden");
	test_stack_chk_fail_local_hidden(st_stack);

	std::printf("function                      cases     failures\n");
	std::printf("---------------------------------------------\n");
	unsigned long total_fail = 0;
	for (int i = 0; i < g_stat_idx; i++) {
		std::printf("%-28s %8lu %10lu\n", g_stats[i].name, g_stats[i].cases,
		    g_stats[i].failures);
		total_fail += g_stats[i].failures;
	}
	std::printf("---------------------------------------------\n");
	std::printf("TOTAL                         %8lu %10lu\n",
	    st_strlcpy.cases + st_mempcpy.cases + st_memset.cases + st_stack.cases,
	    total_fail);

	return total_fail == 0 ? 0 : 1;
}
