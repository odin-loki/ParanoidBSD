/*
 * Batch b0196s4 differential test.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

import pbsd.lib.libc.rpc.b0196s4;

extern "C" {
struct desparams;

int ref_cbc_crypt(char *key, char *buf, unsigned len, unsigned mode, char *ivec);
int ref_ecb_crypt(char *key, char *buf, unsigned len, unsigned mode);

extern int (*__des_crypt_LOCAL)(char *, unsigned, struct desparams *);
int ref_des_crypt_local_hook(char *buf, unsigned len, struct desparams *desp);

void mock_reset_b0196s4(void);

extern int mock_crypt_no_loopback;
extern int mock_crypt_no_clnt;
extern int mock_crypt_null_result;
extern int mock_crypt_result_stat;
extern int mock_des_local_return;
extern int mock_des_local_xor;
}

namespace port = pbsd::lib_libc_rpc::b0196s4;

namespace {

inline constexpr unsigned char GUARD = 0x7f;
inline constexpr unsigned DES_MAXDATA = 8192;
inline constexpr unsigned DES_DIRMASK = (1 << 0);
inline constexpr unsigned DES_ENCRYPT = (0 * DES_DIRMASK);
inline constexpr unsigned DES_DECRYPT = (1 * DES_DIRMASK);
inline constexpr unsigned DES_DEVMASK = (1 << 1);
inline constexpr unsigned DES_HW = (0 * DES_DEVMASK);
inline constexpr unsigned DES_SW = (1 * DES_DEVMASK);
inline constexpr int DESERR_NONE = 0;
inline constexpr int DESERR_NOHWDEVICE = 1;
inline constexpr int DESERR_HWERROR = 2;
inline constexpr int DESERR_BADPARAM = 3;

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "cbc_crypt", 0, 0 },
	{ "ecb_crypt", 0, 0 },
};

unsigned long long reported = 0;
const unsigned long long report_limit = 30;

void
fill_guard(unsigned char *p, size_t n)
{
	for (size_t i = 0; i < n; i++)
		p[i] = GUARD;
}

bool
bufs_equal(const unsigned char *a, const unsigned char *b, size_t n)
{
	return std::memcmp(a, b, n) == 0;
}

std::uint64_t rng_state = 0x243F6A8885A308D3ULL;

std::uint64_t
next_u64()
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

void
apply_mock_origin(const char *origin)
{
	mock_reset_b0196s4();
	__des_crypt_LOCAL = ref_des_crypt_local_hook;

	if (origin[0] == 'l' && origin[1] != 'o')
		__des_crypt_LOCAL = nullptr;
	if (origin[0] == 'f')
		mock_des_local_return = 0;
	if (origin[0] == 'x')
		mock_des_local_xor = 0x99;
	if (origin[0] == 'n')
		mock_crypt_no_loopback = 1;
	if (origin[0] == 'c')
		mock_crypt_no_clnt = 1;
	if (origin[0] == 'r')
		mock_crypt_null_result = 1;
	if (origin[0] == 'h')
		mock_crypt_result_stat = DESERR_HWERROR;
	if (origin[0] == 'd')
		mock_crypt_result_stat = DESERR_NOHWDEVICE;
	if (origin[0] == 'z')
		mock_crypt_result_stat = DESERR_BADPARAM;
}

void
fill_pattern(unsigned char *p, size_t n, unsigned char base, unsigned mix)
{
	for (size_t i = 0; i < n; i++)
		p[i] = static_cast<unsigned char>((base + i + mix) & 0xff);
}

void
fill_nul_heavy(unsigned char *p, size_t n, unsigned char hi)
{
	for (size_t i = 0; i < n; i++)
		p[i] = (i % 3 == 0) ? 0 : static_cast<unsigned char>(hi | (i & 0x7f));
}

void
check_crypt(const char *which, unsigned len, unsigned mode, const char *origin,
    unsigned char key_base, unsigned char buf_base, unsigned char ivec_base)
{
	stats &st = which[0] == 'c' ? tbl[0] : tbl[1];
	st.cases++;

	const size_t cap = static_cast<size_t>(len) + 16;
	std::vector<unsigned char> refkey(16, GUARD);
	std::vector<unsigned char> portkey(16, GUARD);
	std::vector<unsigned char> refbuf(cap, GUARD);
	std::vector<unsigned char> portbuf(cap, GUARD);
	std::vector<unsigned char> refivec(16, GUARD);
	std::vector<unsigned char> portivec(16, GUARD);

	fill_pattern(refkey.data(), 8, key_base, len);
	fill_pattern(portkey.data(), 8, key_base, len);
	if (len > 0) {
		fill_pattern(refbuf.data(), len, buf_base, mode);
		fill_pattern(portbuf.data(), len, buf_base, mode);
	}
	fill_pattern(refivec.data(), 8, ivec_base, mode);
	fill_pattern(portivec.data(), 8, ivec_base, mode);

	apply_mock_origin(origin);

	int got;
	int want;
	if (which[0] == 'c') {
		got = port::cbc_crypt(reinterpret_cast<char *>(portkey.data()),
		    reinterpret_cast<char *>(portbuf.data()), len, mode,
		    reinterpret_cast<char *>(portivec.data()));
		want = ref_cbc_crypt(reinterpret_cast<char *>(refkey.data()),
		    reinterpret_cast<char *>(refbuf.data()), len, mode,
		    reinterpret_cast<char *>(refivec.data()));
	} else {
		got = port::ecb_crypt(reinterpret_cast<char *>(portkey.data()),
		    reinterpret_cast<char *>(portbuf.data()), len, mode);
		want = ref_ecb_crypt(reinterpret_cast<char *>(refkey.data()),
		    reinterpret_cast<char *>(refbuf.data()), len, mode);
	}

	bool ok = got == want &&
	    bufs_equal(portkey.data(), refkey.data(), 16) &&
	    bufs_equal(portbuf.data(), refbuf.data(), cap) &&
	    bufs_equal(portivec.data(), refivec.data(), 16);

	if (!ok) {
		st.failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL %s [%s] len=%u mode=%u got=%d want=%d\n",
			    which, origin, len, mode, got, want);
		}
	}
}

void
edge_cases()
{
	static const unsigned modes[] = {
		DES_ENCRYPT | DES_HW,
		DES_ENCRYPT | DES_SW,
		DES_DECRYPT | DES_HW,
		DES_DECRYPT | DES_SW,
		DES_ENCRYPT,
		DES_DECRYPT,
		DES_SW,
		DES_HW,
		0, 1, 2, 3, 4, 5, 6, 7,
		0x80, 0x81, 0xfe, 0xff,
	};

	static const char *origins[] = {
		"local", "fail", "xor", "ln", "lc", "lr", "lh", "ld",
	};

	for (unsigned len : {0, 8, 16, 24, 32, 64, 128, 256, 512, 1024,
	    8192}) {
		for (unsigned mode : modes) {
			for (const char *origin : origins) {
				check_crypt("cbc_crypt", len, mode, origin, 0x30,
				    0x40, 0x50);
				check_crypt("ecb_crypt", len, mode, origin, 0x30,
				    0x40, 0x50);
			}
		}
	}

	for (unsigned badlen : {1, 2, 3, 4, 5, 6, 7, 9, 15, 17, 31, 8193,
	    8200, 0xffff, 0xfffe}) {
		for (unsigned mode : modes) {
			check_crypt("cbc_crypt", badlen, mode, "local", 0x30,
			    0x40, 0x50);
			check_crypt("ecb_crypt", badlen, mode, "local", 0x30,
			    0x40, 0x50);
		}
	}

	for (unsigned len : {8, 16, 32, 64}) {
		tbl[0].cases++;
		tbl[1].cases++;

		const size_t cap = len + 16;
		std::vector<unsigned char> refkey(16, GUARD);
		std::vector<unsigned char> portkey(16, GUARD);
		std::vector<unsigned char> refbuf(cap, GUARD);
		std::vector<unsigned char> portbuf(cap, GUARD);
		std::vector<unsigned char> refivec(16, GUARD);
		std::vector<unsigned char> portivec(16, GUARD);

		fill_nul_heavy(refkey.data(), 8, 0x80);
		fill_nul_heavy(portkey.data(), 8, 0x80);
		fill_nul_heavy(refbuf.data(), len, 0xc0);
		fill_nul_heavy(portbuf.data(), len, 0xc0);
		fill_nul_heavy(refivec.data(), 8, 0xa0);
		fill_nul_heavy(portivec.data(), 8, 0xa0);

		apply_mock_origin("local");

		unsigned mode = DES_ENCRYPT | DES_SW;
		int got_cbc = port::cbc_crypt(
		    reinterpret_cast<char *>(portkey.data()),
		    reinterpret_cast<char *>(portbuf.data()), len, mode,
		    reinterpret_cast<char *>(portivec.data()));
		int want_cbc = ref_cbc_crypt(
		    reinterpret_cast<char *>(refkey.data()),
		    reinterpret_cast<char *>(refbuf.data()), len, mode,
		    reinterpret_cast<char *>(refivec.data()));

		int got_ecb = port::ecb_crypt(
		    reinterpret_cast<char *>(portkey.data()),
		    reinterpret_cast<char *>(portbuf.data()), len, mode);
		int want_ecb = ref_ecb_crypt(
		    reinterpret_cast<char *>(refkey.data()),
		    reinterpret_cast<char *>(refbuf.data()), len, mode);

		bool ok_cbc = got_cbc == want_cbc &&
		    bufs_equal(portkey.data(), refkey.data(), 16) &&
		    bufs_equal(portbuf.data(), refbuf.data(), cap) &&
		    bufs_equal(portivec.data(), refivec.data(), 16);
		bool ok_ecb = got_ecb == want_ecb &&
		    bufs_equal(portkey.data(), refkey.data(), 16) &&
		    bufs_equal(portbuf.data(), refbuf.data(), cap);

		if (!ok_cbc)
			tbl[0].failures++;
		if (!ok_ecb)
			tbl[1].failures++;
	}

	check_crypt("cbc_crypt", 16, DES_ENCRYPT | DES_SW, "local", 0xff, 0x80,
	    0xff);
	check_crypt("ecb_crypt", 16, DES_DECRYPT | DES_HW, "local", 0x80, 0xff,
	    0x80);
	check_crypt("cbc_crypt", 8, DES_ENCRYPT | DES_HW, "ln", 0x11, 0x22,
	    0x33);
	check_crypt("ecb_crypt", 8, DES_ENCRYPT | DES_HW, "lc", 0x44, 0x55,
	    0x66);
	check_crypt("cbc_crypt", 8, DES_ENCRYPT | DES_SW, "lr", 0x77, 0x88,
	    0x99);
	check_crypt("ecb_crypt", 8, DES_ENCRYPT | DES_SW, "lh", 0xaa, 0xbb,
	    0xcc);
}

void
random_sweep()
{
	const unsigned long long iterations = 200000;

	for (unsigned long long i = 0; i < iterations; i++) {
		const std::uint64_t r = next_u64();
		const unsigned mode = static_cast<unsigned>(r & 0xff);
		unsigned len = static_cast<unsigned>((r >> 8) & 0x1fff);
		const unsigned pick = static_cast<unsigned>((r >> 24) & 0xf);

		if (pick < 4) {
			len = static_cast<unsigned>((r >> 8) & 0x7);
			check_crypt("cbc_crypt", len, mode, "local",
			    static_cast<unsigned char>(r >> 32),
			    static_cast<unsigned char>(r >> 40),
			    static_cast<unsigned char>(r >> 48));
			continue;
		}
		if (pick < 8) {
			len = static_cast<unsigned>(((r >> 8) & 0x3ff) + 8193);
			check_crypt("ecb_crypt", len, mode, "local",
			    static_cast<unsigned char>(r >> 32),
			    static_cast<unsigned char>(r >> 40),
			    static_cast<unsigned char>(r >> 48));
			continue;
		}

		len = static_cast<unsigned>((r >> 8) & 0x3ff);
		len = ((len / 8) + 1) * 8;
		if (len > DES_MAXDATA)
			len = DES_MAXDATA;

		static const char *origins[] = {
			"local", "fail", "xor", "ln", "lc", "lr", "lh", "ld",
		};
		const char *origin = origins[pick % 8];

		if ((r & 1) == 0) {
			check_crypt("cbc_crypt", len, mode, origin,
			    static_cast<unsigned char>(r >> 32),
			    static_cast<unsigned char>(r >> 40),
			    static_cast<unsigned char>(r >> 48));
		} else {
			check_crypt("ecb_crypt", len, mode, origin,
			    static_cast<unsigned char>(r >> 32),
			    static_cast<unsigned char>(r >> 40),
			    static_cast<unsigned char>(r >> 48));
		}
	}
}

} /* namespace */

int
main()
{
	edge_cases();
	random_sweep();

	std::printf("\n=== b0196s4 differential test results ===\n");
	unsigned long long total_fail = 0;
	for (const stats &s : tbl) {
		std::printf("%-16s  cases=%llu  failures=%llu\n",
		    s.name, s.cases, s.failures);
		total_fail += s.failures;
	}
	std::printf("TOTAL failures: %llu\n", total_fail);
	return total_fail == 0 ? 0 : 1;
}
