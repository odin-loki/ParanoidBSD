/*
 * Batch b0196s3 differential test.
 */

#include <cstdint>
#include <cstdio>

import pbsd.lib.libc.rpc.b0196s3;

extern "C" {
int ref_pmap_set(unsigned long program, unsigned long version, int protocol,
    int port);
int ref_pmap_unset(unsigned long program, unsigned long version);

void mock_reset_b0196s3(void);

extern int mock_getconfip_udp_null;
extern int mock_getconfip_tcp_null;
extern int mock_uaddr2taddr_null;
extern int mock_rpcb_set_result;
extern int mock_rpcb_unset_udp_result;
extern int mock_rpcb_unset_tcp_result;
}

namespace port = pbsd::lib_libc_rpc::b0196s3;

namespace {

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "pmap_set", 0, 0 },
	{ "pmap_unset", 0, 0 },
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

void
apply_pmap_set_mocks(const char *origin)
{
	mock_reset_b0196s3();
	if (origin[0] == 'u')
		mock_getconfip_udp_null = 1;
	if (origin[0] == 't')
		mock_getconfip_tcp_null = 1;
	if (origin[0] == 'a')
		mock_uaddr2taddr_null = 1;
	if (origin[0] == 'f')
		mock_rpcb_set_result = 0;
}

void
check_pmap_set(unsigned long program, unsigned long version, int protocol,
    int portnum, const char *origin)
{
	tbl[0].cases++;

	apply_pmap_set_mocks(origin);

	port::bool_t got = port::pmap_set(program, version, protocol, portnum);
	port::bool_t want = ref_pmap_set(program, version, protocol, portnum);

	if (got != want) {
		tbl[0].failures++;
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
apply_pmap_unset_mocks(const char *origin)
{
	mock_reset_b0196s3();
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
}

void
check_pmap_unset(unsigned long program, unsigned long version,
    const char *origin)
{
	tbl[1].cases++;

	apply_pmap_unset_mocks(origin);

	port::bool_t got = port::pmap_unset(program, version);
	port::bool_t want = ref_pmap_unset(program, version);

	if (got != want) {
		tbl[1].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL pmap_unset [%s] prog=%lu ver=%lu "
			    "got=%d want=%d\n",
			    origin, program, version, got, want);
		}
	}
}

void
edge_cases()
{
	static const int bad_protos[] = {
		0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		18, 19, 20, 255, -1, -17, 256, 0x7fff, 0x8000, 0xffff,
		0x7ffe, 0x8001,
	};
	static const int good_protos[] = { 6, 17 };
	static const int ports[] = {
		0, 1, 0xff, 0x100, 0x0102, 0x7f7f, 0x8080, 8080,
		0x1f90, 0x8000, 0xffff, -1, 0x7fffffff,
		static_cast<int>(0x80000000U),
	};

	for (int proto : bad_protos)
		check_pmap_set(1, 1, proto, 8080, "ok");

	for (int proto : good_protos) {
		for (int p : ports)
			check_pmap_set(1, 1, proto, p, "ok");
	}

	check_pmap_set(100029, 2, 17, 0, "ok");
	check_pmap_set(100029, 2, 6, 0xff, "ok");
	check_pmap_set(100029, 2, 17, 0x8080, "ok");
	check_pmap_set(100029, 2, 6, 0x0102, "ok");
	check_pmap_set(0, 0, 17, 0, "ok");
	check_pmap_set(0xffffffffU, 0xffffffffU, 6, 0xffff, "ok");

	check_pmap_set(1, 1, 17, 8080, "udp_miss");
	check_pmap_set(1, 1, 6, 8080, "tcp_miss");
	check_pmap_set(1, 1, 17, 8080, "addr");
	check_pmap_set(1, 1, 6, 8080, "addr");
	check_pmap_set(1, 1, 17, 8080, "fail");
	check_pmap_set(1, 1, 6, 8080, "fail");

	for (int proto : good_protos) {
		if (proto == 17)
			check_pmap_set(1, 1, proto, 0x8080, "udp_miss");
		else
			check_pmap_set(1, 1, proto, 0x0102, "tcp_miss");
		check_pmap_set(1, 1, proto, 0x7f00, "addr");
		check_pmap_set(1, 1, proto, 0x00ff, "fail");
	}

	check_pmap_unset(0, 0, "none");
	check_pmap_unset(1, 1, "none");
	check_pmap_unset(100029, 2, "none");
	check_pmap_unset(0xffffffffU, 0xffffffffU, "none");

	check_pmap_unset(1, 1, "udp");
	check_pmap_unset(1, 1, "tcp");
	check_pmap_unset(1, 1, "both");
	check_pmap_unset(1, 1, "nonet");

	check_pmap_unset(100029, 2, "udp");
	check_pmap_unset(100029, 2, "tcp");
	check_pmap_unset(100029, 2, "both");
	check_pmap_unset(100029, 2, "nonet");
}

void
random_sweep()
{
	const unsigned long long iterations = 200000;

	for (unsigned long long i = 0; i < iterations; i++) {
		std::uint64_t r = next_u64();
		const unsigned which = static_cast<unsigned>(r % 8);

		if (which < 5) {
			unsigned long program = static_cast<unsigned long>(r & 0xffffffffU);
			unsigned long version =
			    static_cast<unsigned long>((r >> 32) & 0xffU);
			int protocol;
			int portnum;

			switch (which) {
			case 0:
				protocol = 17;
				portnum = static_cast<int>(r & 0xffff);
				check_pmap_set(program, version, protocol,
				    portnum, "ok");
				break;
			case 1:
				protocol = 6;
				portnum = static_cast<int>((r >> 16) & 0xffff);
				check_pmap_set(program, version, protocol,
				    portnum, "ok");
				break;
			case 2:
				protocol = static_cast<int>((r >> 8) & 0xff);
				portnum = static_cast<int>(r & 0xffff);
				check_pmap_set(program, version, protocol,
				    portnum, "ok");
				break;
			case 3:
				protocol = 17;
				portnum = static_cast<int>(r & 0xffff);
				check_pmap_set(program, version, protocol,
				    portnum, "udp_miss");
				break;
			default:
				protocol = 6;
				portnum = static_cast<int>(r & 0xffff);
				check_pmap_set(program, version, protocol,
				    portnum, "fail");
				break;
			}
		} else {
			unsigned long program = static_cast<unsigned long>(r & 0xffffffffU);
			unsigned long version =
			    static_cast<unsigned long>((r >> 32) & 0xffU);
			const char *origin;

			switch (which) {
			case 5:
				origin = "none";
				break;
			case 6:
				origin = (r & 1) ? "udp" : "tcp";
				break;
			case 7:
				origin = (r & 2) ? "both" : "nonet";
				break;
			default:
				origin = "none";
				break;
			}
			check_pmap_unset(program, version, origin);
		}
	}
}

} /* namespace */

int
main()
{
	edge_cases();
	random_sweep();

	std::printf("\n=== b0196s3 differential test results ===\n");
	unsigned long long total_fail = 0;
	for (const stats &s : tbl) {
		std::printf("%-16s  cases=%llu  failures=%llu\n",
		    s.name, s.cases, s.failures);
		total_fail += s.failures;
	}
	std::printf("TOTAL failures: %llu\n", total_fail);
	return total_fail == 0 ? 0 : 1;
}
