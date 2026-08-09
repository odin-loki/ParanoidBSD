/*
 * Batch b0196 differential test.
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

import pbsd.lib.libc.rpc.b0196;

extern "C" {
struct rpc_createerr;
struct desparams;

struct rpc_createerr *ref___rpc_createerr(void);
int ref__des_crypt_call(char *buf, int len, struct desparams *dparms);
int ref_pmap_set(unsigned long program, unsigned long version, int protocol,
    int port);
int ref_pmap_unset(unsigned long program, unsigned long version);
int ref_cbc_crypt(char *key, char *buf, unsigned len, unsigned mode, char *ivec);
int ref_ecb_crypt(char *key, char *buf, unsigned len, unsigned mode);

extern int (*__des_crypt_LOCAL)(char *, unsigned, struct desparams *);
int ref_des_crypt_local_hook(char *buf, unsigned len, struct desparams *desp);

void mock_reset_b0196(void);

extern int mock_thr_main_result;
extern int mock_thr_once_result;
extern int mock_thr_keycreate_result;
extern int mock_thr_setspecific_result;
extern int mock_thr_getspecific_null;
extern int mock_malloc_fail;
extern int mock_thr_setspecific_fail_on;
extern int mock_getconfip_udp_null;
extern int mock_getconfip_tcp_null;
extern int mock_uaddr2taddr_null;
extern int mock_rpcb_set_result;
extern int mock_rpcb_unset_udp_result;
extern int mock_rpcb_unset_tcp_result;
extern int mock_getnetconfig_calls;
extern int mock_crypt_no_loopback;
extern int mock_crypt_no_clnt;
extern int mock_crypt_null_result;
extern int mock_crypt_result_stat;
extern int mock_des_local_return;
extern int mock_des_local_xor;
}

namespace port = pbsd::lib_libc_rpc::b0196;

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
	{ "__rpc_createerr", 0, 0 },
	{ "_des_crypt_call", 0, 0 },
	{ "pmap_set", 0, 0 },
	{ "pmap_unset", 0, 0 },
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

unsigned
rand_u32()
{
	return static_cast<unsigned>(next_u64());
}

void
check_rpc_createerr(const char *origin)
{
	tbl[0].cases++;

	mock_reset_b0196();
	mock_thr_main_result = origin[0] == 'm' ? 1 : 0;
	if (origin[0] == 'o')
		mock_thr_once_result = -1;
	if (origin[0] == 'k')
		mock_thr_keycreate_result = -1;
	if (origin[0] == 's')
		mock_thr_setspecific_fail_on = 1;
	if (origin[0] == 'a')
		mock_malloc_fail = 1;
	if (origin[0] == 't') {
		mock_thr_main_result = 0;
		mock_thr_getspecific_null = 1;
	}

	port::rpc_createerr *pgot = port::__rpc_createerr();
	struct rpc_createerr *rwant = ref___rpc_createerr();

	bool ok = true;
	if (pgot == nullptr || rwant == nullptr)
		ok = (pgot == nullptr && rwant == nullptr);
	else {
		enum clnt_stat ps = pgot->cf_stat;
		enum clnt_stat rs = rwant->cf_stat;
		pgot->cf_stat = static_cast<enum clnt_stat>(42);
		rwant->cf_stat = static_cast<enum clnt_stat>(42);
		ok = (pgot->cf_stat == static_cast<enum clnt_stat>(42)) &&
		    (rwant->cf_stat == static_cast<enum clnt_stat>(42));
		pgot->cf_stat = ps;
		rwant->cf_stat = rs;
	}

	if (!ok) {
		tbl[0].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL __rpc_createerr [%s]\n", origin);
		}
	}
}

void
check_des_crypt_call(int len, const char *origin)
{
	tbl[1].cases++;

	const size_t cap = static_cast<size_t>(len) + 16;
	std::vector<unsigned char> refbuf(cap, GUARD);
	std::vector<unsigned char> portbuf(cap, GUARD);
	std::vector<unsigned char> refivec(16, GUARD);
	std::vector<unsigned char> portivec(16, GUARD);

	for (int i = 0; i < len; i++) {
		refbuf[i] = static_cast<unsigned char>(0x10 + i);
		portbuf[i] = refbuf[i];
	}
	for (int i = 0; i < 8; i++) {
		refivec[i] = static_cast<unsigned char>(0x20 + i);
		portivec[i] = refivec[i];
	}

	mock_reset_b0196();
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

	port::desparams pd{};
	port::desparams rd{};
	std::memcpy(pd.des_ivec, portivec.data(), 8);
	std::memcpy(rd.des_ivec, refivec.data(), 8);
	pd.des_dir = port::ENCRYPT;
	rd.des_dir = port::ENCRYPT;
	pd.des_mode = port::CBC;
	rd.des_mode = port::CBC;

	int got = port::_des_crypt_call(reinterpret_cast<char *>(portbuf.data()),
	    len, &pd);
	int want = ref__des_crypt_call(reinterpret_cast<char *>(refbuf.data()),
	    len, &rd);

	bool ok = got == want &&
	    bufs_equal(portbuf.data(), refbuf.data(), cap) &&
	    bufs_equal(portivec.data(), refivec.data(), 16) &&
	    std::memcmp(pd.des_ivec, rd.des_ivec, 8) == 0;

	if (!ok) {
		tbl[1].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL _des_crypt_call [%s] len=%d got=%d want=%d\n",
			    origin, len, got, want);
		}
	}
}

void
check_pmap_set(unsigned long program, unsigned long version, int protocol,
    int portnum, const char *origin)
{
	tbl[2].cases++;

	mock_reset_b0196();
	if (origin[0] == 'u')
		mock_getconfip_udp_null = 1;
	if (origin[0] == 't')
		mock_getconfip_tcp_null = 1;
	if (origin[0] == 'a')
		mock_uaddr2taddr_null = 1;
	if (origin[0] == 'f')
		mock_rpcb_set_result = 0;

	port::bool_t got = port::pmap_set(program, version, protocol, portnum);
	port::bool_t want = ref_pmap_set(program, version, protocol, portnum);

	if (got != want) {
		tbl[2].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL pmap_set [%s] prog=%lu ver=%lu proto=%d "
			    "port=%d got=%d want=%d\n",
			    origin, program, version, protocol, portnum, got,
			    want);
		}
	}
}

void
check_pmap_unset(unsigned long program, unsigned long version,
    const char *origin)
{
	tbl[3].cases++;

	mock_reset_b0196();
	if (origin[0] == 'u')
		mock_rpcb_unset_udp_result = 1;
	if (origin[0] == 't')
		mock_rpcb_unset_tcp_result = 1;
	if (origin[0] == 'b') {
		mock_rpcb_unset_udp_result = 1;
		mock_rpcb_unset_tcp_result = 1;
	}
	if (origin[0] == 'n') {
		mock_getconfip_udp_null = 1;
		mock_getconfip_tcp_null = 1;
	}

	port::bool_t got = port::pmap_unset(program, version);
	port::bool_t want = ref_pmap_unset(program, version);

	if (got != want) {
		tbl[3].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL pmap_unset [%s] prog=%lu ver=%lu "
			    "got=%d want=%d\n",
			    origin, program, version, got, want);
		}
	}
}

void
check_crypt(const char *which, unsigned len, unsigned mode,
    const char *origin)
{
	stats &st = which[0] == 'c' ? tbl[4] : tbl[5];
	st.cases++;

	const size_t cap = len + 16;
	std::vector<unsigned char> refkey(16, GUARD);
	std::vector<unsigned char> portkey(16, GUARD);
	std::vector<unsigned char> refbuf(cap, GUARD);
	std::vector<unsigned char> portbuf(cap, GUARD);
	std::vector<unsigned char> refivec(16, GUARD);
	std::vector<unsigned char> portivec(16, GUARD);

	for (unsigned i = 0; i < 8; i++) {
		refkey[i] = static_cast<unsigned char>(0x30 + i);
		portkey[i] = refkey[i];
	}
	for (unsigned i = 0; i < len; i++) {
		refbuf[i] = static_cast<unsigned char>((0x40 + i) & 0xff);
		portbuf[i] = refbuf[i];
	}
	for (int i = 0; i < 8; i++) {
		refivec[i] = static_cast<unsigned char>(0x50 + i);
		portivec[i] = refivec[i];
	}

	mock_reset_b0196();
	__des_crypt_LOCAL = ref_des_crypt_local_hook;
	if (origin[0] == 'l')
		__des_crypt_LOCAL = nullptr;
	if (origin[0] == 'f')
		mock_des_local_return = 0;
	if (origin[0] == 'x')
		mock_des_local_xor = 0x99;

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
	check_rpc_createerr("main");
	check_rpc_createerr("once");
	check_rpc_createerr("key");
	check_rpc_createerr("setspec");
	check_rpc_createerr("alloc");
	check_rpc_createerr("tls");

	for (int len : {0, 8, 16, 24, 32, 64, 128, 256, 512, 1024, 8192}) {
		check_des_crypt_call(len, "ok");
		check_des_crypt_call(len, "no_lb");
		check_des_crypt_call(len, "clnt");
		check_des_crypt_call(len, "null");
		check_des_crypt_call(len, "hw");
		check_des_crypt_call(len, "nodev");
	}

	for (int proto : {0, 1, 6, 17, 255, -1, 256, 0x7fff, 0x8000, 0xffff}) {
		check_pmap_set(1, 1, proto, 8080, "ok");
	}
	check_pmap_set(100029, 2, 17, 0, "udp");
	check_pmap_set(100029, 2, 6, 0xff, "tcp");
	check_pmap_set(100029, 2, 17, 0x8080, "udp");
	check_pmap_set(100029, 2, 6, 0x0102, "tcp");
	check_pmap_set(1, 1, 17, 8080, "noudp");
	check_pmap_set(1, 1, 6, 8080, "notcp");
	check_pmap_set(1, 1, 17, 8080, "addr");
	check_pmap_set(1, 1, 17, 8080, "fail");

	check_pmap_unset(1, 1, "none");
	check_pmap_unset(1, 1, "udp");
	check_pmap_unset(1, 1, "tcp");
	check_pmap_unset(1, 1, "both");
	check_pmap_unset(1, 1, "nonet");

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

	for (unsigned len : {8, 16, 24, 32, 64, 128, 256, 512, 1024, 8192}) {
		for (unsigned mode : modes) {
			check_crypt("cbc_crypt", len, mode, "local");
			check_crypt("ecb_crypt", len, mode, "local");
		}
	}

	for (unsigned badlen : {1, 2, 3, 4, 5, 6, 7, 9, 15, 8193, 8200,
	    0xffff}) {
		for (unsigned mode : modes) {
			check_crypt("cbc_crypt", badlen, mode, "local");
			check_crypt("ecb_crypt", badlen, mode, "local");
		}
	}

	check_crypt("cbc_crypt", 16, DES_ENCRYPT | DES_SW, "local");
	check_crypt("cbc_crypt", 16, DES_ENCRYPT | DES_SW, "fail");
	check_crypt("cbc_crypt", 16, DES_ENCRYPT | DES_SW, "xor");
	check_crypt("cbc_crypt", 16, DES_ENCRYPT | DES_HW, "rpc");
	check_crypt("ecb_crypt", 16, DES_ENCRYPT | DES_HW, "rpc");
}

void
random_sweep()
{
	const unsigned long long iterations = 200000;

	for (unsigned long long i = 0; i < iterations; i++) {
		std::uint64_t r = next_u64();

		switch (static_cast<unsigned>(r % 6)) {
		case 0:
			check_rpc_createerr(
			    (r & 1) ? "main" : "tls");
			break;
		case 1: {
			int len = static_cast<int>((r >> 4) & 0x3ff);
			len = (len / 8) * 8;
			check_des_crypt_call(len, "ok");
			break;
		}
		case 2:
			check_pmap_set(r & 0xffff, (r >> 16) & 0xff,
			    (r & 2) ? 6 : 17, static_cast<int>(r & 0xffff),
			    "ok");
			break;
		case 3:
			check_pmap_unset(r & 0xffff, (r >> 16) & 0xff, "none");
			break;
		case 4: {
			unsigned len = static_cast<unsigned>((r >> 8) & 0x3ff);
			len = ((len / 8) + 1) * 8;
			if (len > DES_MAXDATA)
				len = DES_MAXDATA;
			unsigned mode = static_cast<unsigned>(r & 0xff);
			check_crypt("cbc_crypt", len, mode, "local");
			break;
		}
		default: {
			unsigned len = static_cast<unsigned>((r >> 8) & 0x3ff);
			len = ((len / 8) + 1) * 8;
			if (len > DES_MAXDATA)
				len = DES_MAXDATA;
			unsigned mode = static_cast<unsigned>(r & 0xff);
			check_crypt("ecb_crypt", len, mode, "local");
			break;
		}
		}
	}
}

} /* namespace */

int
main()
{
	edge_cases();
	random_sweep();

	std::printf("\n=== b0196 differential test results ===\n");
	unsigned long long total_fail = 0;
	for (const stats &s : tbl) {
		std::printf("%-16s  cases=%llu  failures=%llu\n",
		    s.name, s.cases, s.failures);
		total_fail += s.failures;
	}
	std::printf("TOTAL failures: %llu\n", total_fail);
	return total_fail == 0 ? 0 : 1;
}
