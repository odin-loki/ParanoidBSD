/*
 * Batch b0196s2 differential test.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

import pbsd.lib.libc.rpc.b0196s2;

extern "C" {
struct desparams {
	int des_dir;
	int des_mode;
	char des_ivec[8];
	char des_key[8];
};

int ref_des_crypt_call(char *buf, int len, struct desparams *dparms);

void mock_reset_b0196s2(void);

extern int mock_nc_always_null;
extern int mock_nc_nonloop_count;
extern int mock_nc_null_protofmly;
extern char mock_nc_protofmly_other[16];
extern int mock_clnt_create_fail;
extern int mock_des_crypt_fail;
extern int mock_des_crypt_stat;
}

namespace port = pbsd::lib_libc_rpc::b0196s2;

namespace {

constexpr int DESERR_NONE = 0;
constexpr int DESERR_NOKEY = 1;
constexpr int DESERR_HWERROR = 2;
constexpr int DESERR_BADMODE = 3;
constexpr int DESERR_NOHWDEVICE = 4;

constexpr int ENCRYPT = 0;
constexpr int DECRYPT = 1;
constexpr int CBC = 0;
constexpr int ECB = 1;

constexpr unsigned char GUARD_BYTE = 0x7f;
constexpr size_t BUF_GUARD = 8;
constexpr size_t MAX_LEN = 64;

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "_des_crypt_call", 0, 0 },
};

unsigned long long reported = 0;
const unsigned long long report_limit = 30;

std::uint64_t rng_state = 0x243F6A8885A308D3ULL;

std::uint64_t
next_u64()
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

struct guarded_dparms {
	unsigned char g1[8];
	port::desparams dp;
	unsigned char g2[8];
};

void
fill_dparms(port::desparams &dp, int dir, int mode,
    const unsigned char *ivec, const unsigned char *key)
{
	dp.des_dir = dir;
	dp.des_mode = mode;
	for (int i = 0; i < 8; i++) {
		dp.des_ivec[i] = static_cast<char>(ivec[i]);
		dp.des_key[i] = static_cast<char>(key[i]);
	}
}

struct mock_cfg {
	int nc_always_null = 0;
	int nc_nonloop_count = 0;
	int nc_null_protofmly = 0;
	const char *nc_other_proto = "inet";
	int clnt_create_fail = 0;
	int des_crypt_fail = 0;
	int des_crypt_stat = DESERR_NONE;
};

void
apply_mock_cfg(const mock_cfg &cfg)
{
	mock_nc_always_null = cfg.nc_always_null;
	mock_nc_nonloop_count = cfg.nc_nonloop_count;
	mock_nc_null_protofmly = cfg.nc_null_protofmly;
	std::strcpy(mock_nc_protofmly_other, cfg.nc_other_proto);
	mock_clnt_create_fail = cfg.clnt_create_fail;
	mock_des_crypt_fail = cfg.des_crypt_fail;
	mock_des_crypt_stat = cfg.des_crypt_stat;
}

bool
run_des_crypt_call(const char *origin, int len, int dir, int mode,
    const mock_cfg &cfg, const unsigned char *input)
{
	tbl[0].cases++;

	mock_reset_b0196s2();
	apply_mock_cfg(cfg);

	const size_t cap = BUF_GUARD + MAX_LEN + BUF_GUARD;
	const size_t off = BUF_GUARD;
	std::vector<unsigned char> pbuf(cap, GUARD_BYTE);
	std::vector<unsigned char> rbuf(cap, GUARD_BYTE);

	if (len > 0 && input != nullptr) {
		std::memcpy(pbuf.data() + off, input, static_cast<size_t>(len));
		std::memcpy(rbuf.data() + off, input, static_cast<size_t>(len));
	}

	static const unsigned char ivec_pat[] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
	};
	static const unsigned char key_pat[] = {
		0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
	};

	guarded_dparms pg{};
	guarded_dparms rg{};
	std::memset(&pg, GUARD_BYTE, sizeof(pg));
	std::memset(&rg, GUARD_BYTE, sizeof(rg));
	fill_dparms(pg.dp, dir, mode, ivec_pat, key_pat);
	fill_dparms(rg.dp, dir, mode, ivec_pat, key_pat);

	const int pr = port::_des_crypt_call(
	    reinterpret_cast<char *>(pbuf.data() + off), len, &pg.dp);
	const int rr = ref_des_crypt_call(
	    reinterpret_cast<char *>(rbuf.data() + off), len, &rg.dp);

	bool ok = pr == rr;
	ok = ok && std::memcmp(pbuf.data(), rbuf.data(), cap) == 0;
	ok = ok && std::memcmp(&pg, &rg, sizeof(pg)) == 0;

	if (!ok) {
		tbl[0].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL _des_crypt_call [%s] len=%d dir=%d "
			    "mode=%d stat=%d ret_p=%d ret_r=%d\n",
			    origin, len, dir, mode, cfg.des_crypt_stat, pr, rr);
		}
	}
	return ok;
}

void
edge_cases()
{
	static const unsigned char empty[] = { 0 };
	static const unsigned char one[] = { 0x41 };
	static const unsigned char nul_heavy[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	static const unsigned char highbit[] = {
		0x80, 0x81, 0xfe, 0xff, 0x7f, 0x00, 0xaa, 0x55,
		0x80, 0x81, 0xfe, 0xff, 0x7f, 0x00, 0xaa, 0x55
	};

	const mock_cfg ok{};
	mock_cfg nhw{};
	nhw.des_crypt_stat = DESERR_NOHWDEVICE;
	mock_cfg nky{};
	nky.des_crypt_stat = DESERR_NOKEY;
	mock_cfg bmd{};
	bmd.des_crypt_stat = DESERR_BADMODE;
	mock_cfg nc{};
	nc.nc_always_null = 1;
	mock_cfg loop{};
	loop.nc_nonloop_count = 3;
	mock_cfg nullpf{};
	nullpf.nc_null_protofmly = 1;
	mock_cfg wrongpf{};
	wrongpf.nc_other_proto = "tcp";
	mock_cfg clnt{};
	clnt.clnt_create_fail = 1;
	mock_cfg rpc{};
	rpc.des_crypt_fail = 1;

	run_des_crypt_call("ok0", 0, ENCRYPT, CBC, ok, empty);
	run_des_crypt_call("ok1", 1, ENCRYPT, CBC, ok, one);
	run_des_crypt_call("ok8", 8, ENCRYPT, CBC, ok, nul_heavy);
	run_des_crypt_call("ok16", 16, ENCRYPT, CBC, ok, highbit);
	run_des_crypt_call("ok32", 32, DECRYPT, ECB, ok, highbit);

	run_des_crypt_call("nhw", 8, ENCRYPT, CBC, nhw, highbit);
	run_des_crypt_call("nky", 8, ENCRYPT, CBC, nky, highbit);
	run_des_crypt_call("bmd", 8, ENCRYPT, CBC, bmd, highbit);

	run_des_crypt_call("nc", 8, ENCRYPT, CBC, nc, highbit);
	run_des_crypt_call("loop", 8, ENCRYPT, CBC, loop, highbit);
	run_des_crypt_call("nullpf", 8, ENCRYPT, CBC, nullpf, highbit);
	run_des_crypt_call("wrongpf", 8, ENCRYPT, CBC, wrongpf, highbit);

	run_des_crypt_call("clnt", 8, ENCRYPT, CBC, clnt, highbit);
	run_des_crypt_call("rpc", 8, ENCRYPT, CBC, rpc, highbit);

	for (int dir = ENCRYPT; dir <= DECRYPT; dir++) {
		for (int mode = CBC; mode <= ECB; mode++) {
			run_des_crypt_call("dm", 8, dir, mode, ok, highbit);
		}
	}

	run_des_crypt_call("ok0_nhw", 0, DECRYPT, ECB, nhw, empty);
	run_des_crypt_call("ok1_nhw", 1, DECRYPT, ECB, nhw, one);
}

void
random_sweep()
{
	const unsigned long long iterations = 200000;
	static const char *origins[] = {
		"ok", "nc", "loop", "nullpf", "wrongpf", "clnt", "rpc", "stat"
	};
	unsigned char input[MAX_LEN];

	for (unsigned long long i = 0; i < iterations; i++) {
		const int len = static_cast<int>(next_u64() % (MAX_LEN + 1));
		const int dir = static_cast<int>(next_u64() & 1U);
		const int mode = static_cast<int>((next_u64() >> 1) & 1U);
		const int stat_val = static_cast<int>(next_u64() % 6U);
		const unsigned pick = static_cast<unsigned>(
		    next_u64() % (sizeof(origins) / sizeof(origins[0])));

		for (int j = 0; j < len; j++)
			input[j] = static_cast<unsigned char>(next_u64());

		mock_cfg cfg{};
		switch (pick) {
		case 1:
			cfg.nc_always_null = 1;
			break;
		case 2:
			cfg.nc_nonloop_count =
			    static_cast<int>(next_u64() % 6U);
			break;
		case 3:
			cfg.nc_null_protofmly = 1;
			break;
		case 4:
			cfg.nc_other_proto = "tcp";
			break;
		case 5:
			cfg.clnt_create_fail = 1;
			break;
		case 6:
			cfg.des_crypt_fail = 1;
			break;
		case 7:
			cfg.des_crypt_stat = stat_val;
			break;
		default:
			break;
		}

		run_des_crypt_call(origins[pick], len, dir, mode, cfg, input);
	}
}

} /* namespace */

int
main()
{
	edge_cases();
	random_sweep();

	std::printf("\n=== b0196s2 differential test results ===\n");
	unsigned long long total_fail = 0;
	for (const stats &s : tbl) {
		std::printf("%-16s  cases=%llu  failures=%llu\n",
		    s.name, s.cases, s.failures);
		total_fail += s.failures;
	}
	std::printf("TOTAL failures: %llu\n", total_fail);
	return total_fail == 0 ? 0 : 1;
}
