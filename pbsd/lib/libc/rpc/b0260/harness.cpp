/*
 * Batch b0260 differential test.
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.lib.libc.rpc.b0260;

namespace port = pbsd::lib_libc_rpc::b0260;

extern "C" {
typedef int bool_t;

struct opaque_auth {
	int oa_flavor;
	char *oa_base;
	unsigned int oa_length;
};

struct pmap {
	std::uint32_t pm_prog;
	std::uint32_t pm_vers;
	std::uint32_t pm_prot;
	std::uint32_t pm_port;
};

enum xdr_op { XDR_ENCODE = 0, XDR_DECODE = 1, XDR_FREE = 2 };

typedef struct XDR {
	enum xdr_op x_op;
	void *x_private;
} XDR;

typedef bool_t (*xdrproc_t)(XDR *, void *, unsigned int);

struct AUTH;

bool_t ref___rpc_gss_wrap_stub(AUTH *auth, void *header, size_t headerlen,
    XDR *xdrs, xdrproc_t xdr_args, void *args_ptr);
bool_t ref___rpc_gss_unwrap_stub(AUTH *auth, XDR *xdrs, xdrproc_t xdr_args,
    void *args_ptr);
bool_t ref_xdr_pmap(XDR *xdrs, struct pmap *regs);
int ref__rpc_dtablesize(void);

extern struct opaque_auth ref__null_auth;
extern fd_set ref_svc_fdset;
extern int ref_svc_maxfd;

void mock_reset_b0260(void);
extern int mock_getdtablesize_return;
extern int mock_getdtablesize_calls;
extern int mock_xdr_fail_on_call;
extern int mock_xdr_call_count;
extern std::uint32_t mock_xdr_slot[4];
}

namespace {

inline constexpr unsigned char GUARD = 0x7f;
inline constexpr std::size_t PRE = 32;
inline constexpr std::size_t POST = 32;

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "rpc_commondata globals", 0, 0 },
	{ "__rpc_gss_wrap_stub", 0, 0 },
	{ "__rpc_gss_unwrap_stub", 0, 0 },
	{ "xdr_pmap", 0, 0 },
	{ "_rpc_dtablesize", 0, 0 },
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

unsigned
rand_u32()
{
	return static_cast<unsigned>(next_u64());
}

void
fill_guard(unsigned char *p, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		p[i] = GUARD;
}

bool
bufs_equal(const unsigned char *a, const unsigned char *b, std::size_t n)
{
	return std::memcmp(a, b, n) == 0;
}

struct pmap_arena {
	unsigned char *refbuf;
	unsigned char *portbuf;
	std::size_t total;
	std::size_t body_off;
};

void
pmap_arena_init(pmap_arena &ar)
{
	ar.body_off = PRE;
	ar.total = PRE + sizeof(struct pmap) + POST;
	ar.refbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	ar.portbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	if (ar.refbuf == nullptr || ar.portbuf == nullptr) {
		std::fprintf(stderr, "out of memory\n");
		std::exit(2);
	}
}

void
pmap_arena_free(pmap_arena &ar)
{
	std::free(ar.refbuf);
	std::free(ar.portbuf);
}

struct pmap *
ref_pmap_ptr(pmap_arena &ar)
{
	return reinterpret_cast<struct pmap *>(ar.refbuf + ar.body_off);
}

port::pmap *
port_pmap_ptr(pmap_arena &ar)
{
	return reinterpret_cast<port::pmap *>(ar.portbuf + ar.body_off);
}

void
pmap_arena_prepare(pmap_arena &ar, std::uint32_t prog, std::uint32_t vers,
    std::uint32_t prot, std::uint32_t port)
{
	fill_guard(ar.refbuf, ar.total);
	fill_guard(ar.portbuf, ar.total);
	struct pmap *r = ref_pmap_ptr(ar);
	port::pmap *p = port_pmap_ptr(ar);
	r->pm_prog = prog;
	r->pm_vers = vers;
	r->pm_prot = prot;
	r->pm_port = port;
	p->pm_prog = prog;
	p->pm_vers = vers;
	p->pm_prot = prot;
	p->pm_port = port;
}

bool
check_commondata(const char *origin)
{
	tbl[0].cases++;

	bool ok = true;
	if (ref_svc_maxfd != port::svc_maxfd)
		ok = false;
	if (std::memcmp(&ref__null_auth, &port::_null_auth,
		sizeof(ref__null_auth)) != 0)
		ok = false;
	if (std::memcmp(&ref_svc_fdset, &port::svc_fdset,
		sizeof(ref_svc_fdset)) != 0)
		ok = false;

	if (!ok) {
		tbl[0].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL rpc_commondata globals [%s]\n", origin);
		}
	}
	return ok;
}

bool
check_wrap_stub(size_t headerlen, unsigned char header_byte,
    unsigned char xdr_byte, const char *origin)
{
	tbl[1].cases++;

	unsigned char header[16];
	unsigned char args[16];
	for (int i = 0; i < 16; i++) {
		header[i] = static_cast<unsigned char>(header_byte + i);
		args[i] = static_cast<unsigned char>(0xa0 + i);
	}

	alignas(16) unsigned char auth_storage[64]{};
	XDR xdr{};
	xdr.x_op = XDR_ENCODE;
	xdr.x_private = &xdr_byte;

	bool_t r = ref___rpc_gss_wrap_stub(
	    reinterpret_cast<AUTH *>(auth_storage),
	    headerlen == 0 ? nullptr : header, headerlen, &xdr,
	    reinterpret_cast<xdrproc_t>(1), args);
	bool_t p = port::__rpc_gss_wrap_stub(
	    reinterpret_cast<port::AUTH *>(auth_storage),
	    headerlen == 0 ? nullptr : header, headerlen,
	    reinterpret_cast<port::XDR *>(&xdr),
	    reinterpret_cast<port::xdrproc_t>(1), args);

	bool ok = (r == p);
	if (!ok) {
		tbl[1].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL __rpc_gss_wrap_stub [%s] len=%zu ret=%d/%d\n",
			    origin, headerlen, (int)r, (int)p);
		}
	}
	return ok;
}

bool
check_unwrap_stub(unsigned char xdr_byte, const char *origin)
{
	tbl[2].cases++;

	unsigned char args[16];
	for (int i = 0; i < 16; i++)
		args[i] = static_cast<unsigned char>(0xb0 + i);

	alignas(16) unsigned char auth_storage[64]{};
	XDR xdr{};
	xdr.x_op = XDR_DECODE;
	xdr.x_private = &xdr_byte;

	bool_t r = ref___rpc_gss_unwrap_stub(
	    reinterpret_cast<AUTH *>(auth_storage), &xdr,
	    reinterpret_cast<xdrproc_t>(2), args);
	bool_t p = port::__rpc_gss_unwrap_stub(
	    reinterpret_cast<port::AUTH *>(auth_storage),
	    reinterpret_cast<port::XDR *>(&xdr),
	    reinterpret_cast<port::xdrproc_t>(2), args);

	bool ok = (r == p);
	if (!ok) {
		tbl[2].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL __rpc_gss_unwrap_stub [%s] ret=%d/%d\n",
			    origin, (int)r, (int)p);
		}
	}
	return ok;
}

bool
check_xdr_pmap(pmap_arena &ar, enum xdr_op op, int fail_on,
    std::uint32_t prog, std::uint32_t vers, std::uint32_t prot,
    std::uint32_t port, const char *origin)
{
	tbl[3].cases++;

	pmap_arena_prepare(ar, prog, vers, prot, port);

	mock_reset_b0260();
	mock_xdr_fail_on_call = fail_on;
	mock_xdr_slot[0] = prog;
	mock_xdr_slot[1] = vers;
	mock_xdr_slot[2] = prot;
	mock_xdr_slot[3] = port;

	XDR rx{};
	port::XDR px{};
	rx.x_op = op;
	px.x_op = static_cast<port::xdr_op>(static_cast<int>(op));
	rx.x_private = nullptr;
	px.x_private = nullptr;

	bool_t r = ref_xdr_pmap(&rx, ref_pmap_ptr(ar));
	const int ref_calls = mock_xdr_call_count;
	mock_xdr_call_count = 0;

	bool_t p = port::xdr_pmap(&px, port_pmap_ptr(ar));
	const int port_calls = mock_xdr_call_count;
	const int want_calls =
	    (fail_on >= 1 && fail_on <= 4) ? fail_on : 4;

	bool ok = (r == p);
	if (ok)
		ok = bufs_equal(ar.refbuf, ar.portbuf, ar.total);
	if (ok)
		ok = (ref_calls == want_calls) && (port_calls == want_calls);

	if (!ok) {
		tbl[3].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL xdr_pmap [%s] op=%d fail=%d ret=%d/%d"
			    " calls=%d/%d want=%d\n",
			    origin, (int)op, fail_on, (int)r, (int)p,
			    ref_calls, port_calls, want_calls);
		}
	}
	return ok;
}

int
cap_dtable(int value)
{
	if (value > FD_SETSIZE)
		return (FD_SETSIZE);
	return (value);
}

int
run_dtablesize_child(int mock_return, int do_second, int second_mock)
{
	mock_reset_b0260();
	mock_getdtablesize_return = mock_return;

	int r1 = ref__rpc_dtablesize();
	int p1 = port::_rpc_dtablesize();
	int calls_after_first = mock_getdtablesize_calls;

	int r2 = r1;
	int p2 = p1;
	if (do_second) {
		mock_getdtablesize_return = second_mock;
		r2 = ref__rpc_dtablesize();
		p2 = port::_rpc_dtablesize();
	}

	const int want_calls = do_second ?
	    (cap_dtable(mock_return) != 0 ? 2 : 4) : 2;

	if (r1 != p1 || r2 != p2)
		return 1;
	if (calls_after_first != 2)
		return 1;
	if (mock_getdtablesize_calls != want_calls)
		return 1;
	return 0;
}

bool
check_dtablesize_fresh(int mock_return, int do_second, int second_mock,
    const char *origin)
{
	tbl[4].cases++;

	pid_t pid = fork();
	if (pid < 0) {
		tbl[4].failures++;
		return false;
	}
	if (pid == 0)
		_exit(run_dtablesize_child(mock_return, do_second, second_mock));

	int status = 0;
	if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status) ||
	    WEXITSTATUS(status) != 0) {
		tbl[4].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL _rpc_dtablesize [%s] mock=%d second=%d\n",
			    origin, mock_return, second_mock);
		}
		return false;
	}
	return true;
}

bool
check_dtablesize_cached(const char *origin)
{
	tbl[4].cases++;

	const int before = mock_getdtablesize_calls;
	mock_getdtablesize_return += 1000;
	int r1 = ref__rpc_dtablesize();
	int p1 = port::_rpc_dtablesize();
	const int after_first = mock_getdtablesize_calls;

	mock_getdtablesize_return += 1000;
	int r2 = ref__rpc_dtablesize();
	int p2 = port::_rpc_dtablesize();

	bool ok = (r1 == p1) && (r2 == p2) && (r1 == r2) &&
	    (after_first == before) && (mock_getdtablesize_calls == before);
	if (!ok) {
		tbl[4].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL _rpc_dtablesize cached [%s] %d/%d then %d/%d"
			    " calls=%d before=%d\n",
			    origin, r1, p1, r2, p2, mock_getdtablesize_calls, before);
		}
	}
	return ok;
}

} // namespace

int
main()
{
	pmap_arena ar{};
	pmap_arena_init(ar);

	check_commondata("init");

	const size_t wrap_lens[] = {
		0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256,
		1023, 1024, 4095, 4096, 65535, 65536,
		SIZE_MAX
	};
	const unsigned char header_bytes[] = {
		0x00, 0x01, 0x7f, 0x80, 0xff
	};
	const unsigned char xdr_bytes[] = { 0x00, 0x55, 0x80, 0xff };

	for (size_t len : wrap_lens) {
		for (unsigned char hb : header_bytes) {
			for (unsigned char xb : xdr_bytes) {
				check_wrap_stub(len, hb, xb, "hand-wrap");
				check_unwrap_stub(xb, "hand-unwrap");
			}
		}
	}

	const int fail_points[] = { 0, 1, 2, 3, 4, 5 };
	const std::uint32_t pmap_vals[] = {
		0, 1, 0x7f, 0x80, 0xff, 0x100, 0x7fff, 0x8000, 0xffff,
		0x10000, 0x7fffffff, 0x80000000, 0xffffffff
	};
	const std::uint32_t pmap_edge[][4] = {
		{ 0, 0, 0, 0 },
		{ 1, 1, 1, 1 },
		{ 0x7f, 0x80, 0xff, 0x100 },
		{ 0xffff, 0x10000, 0x7fffffff, 0x80000000 },
		{ 0xffffffff, 0xdeadbeef, 0x80808080, 0x01020304 },
	};
	const int n_pmap_edge =
	    static_cast<int>(sizeof(pmap_edge) / sizeof(pmap_edge[0]));

	const enum xdr_op xdr_ops[] = { XDR_ENCODE, XDR_DECODE, XDR_FREE };

	for (int fail_on : fail_points) {
		for (enum xdr_op op : xdr_ops) {
			for (int i = 0; i < n_pmap_edge; i++) {
				check_xdr_pmap(ar, op, fail_on, pmap_edge[i][0],
				    pmap_edge[i][1], pmap_edge[i][2],
				    pmap_edge[i][3], "hand-pmap");
			}
			for (std::uint32_t prog : pmap_vals) {
				check_xdr_pmap(ar, op, fail_on, prog, 0, 0, 0,
				    "hand-pmap-prog");
				check_xdr_pmap(ar, op, fail_on, 0, prog, 0, 0,
				    "hand-pmap-vers");
				check_xdr_pmap(ar, op, fail_on, 0, 0, prog, 0,
				    "hand-pmap-prot");
				check_xdr_pmap(ar, op, fail_on, 0, 0, 0, prog,
				    "hand-pmap-port");
			}
		}
	}

	const int dtable_mocks[] = {
		0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256,
		FD_SETSIZE - 1, FD_SETSIZE, FD_SETSIZE + 1, FD_SETSIZE + 2,
		1023, 1024, 2047, 4095, 8191, 16383, 32767, 65535, 131071,
		262143, 1048575
	};

	for (int mock : dtable_mocks) {
		check_dtablesize_fresh(mock, 0, 0, "hand-fresh");
		check_dtablesize_fresh(mock, 1, mock + 9999, "hand-cache");
	}

	for (long it = 0; it < 200000; it++) {
		check_commondata("rand-globals");

		size_t len = static_cast<size_t>(rand_u32());
		if ((rand_u32() & 3u) == 0u)
			len &= 0xffu;
		unsigned char hb = static_cast<unsigned char>(rand_u32());
		unsigned char xb = static_cast<unsigned char>(rand_u32() >> 8);
		check_wrap_stub(len, hb, xb, "rand-wrap");
		check_unwrap_stub(xb, "rand-unwrap");

		enum xdr_op op = static_cast<enum xdr_op>(rand_u32() % 3u);
		int fail_on = static_cast<int>(rand_u32() % 6u);
		std::uint32_t prog = rand_u32();
		std::uint32_t vers = rand_u32();
		std::uint32_t prot = rand_u32();
		std::uint32_t port = rand_u32();
		check_xdr_pmap(ar, op, fail_on, prog, vers, prot, port,
		    "rand-pmap");

		if ((rand_u32() & 0xffu) == 0u) {
			int mock = static_cast<int>(rand_u32() % 2000000u);
			check_dtablesize_fresh(mock, rand_u32() & 1u,
			    static_cast<int>(rand_u32()), "rand-dtable-fork");
		}
	}

	mock_reset_b0260();
	mock_getdtablesize_return = 256;
	ref__rpc_dtablesize();
	port::_rpc_dtablesize();

	for (long it = 0; it < 200000; it++)
		check_dtablesize_cached("rand-dtable-cache");

	pmap_arena_free(ar);

	unsigned long long cases = 0;
	unsigned long long failures = 0;
	for (const stats &s : tbl) {
		cases += s.cases;
		failures += s.failures;
	}

	std::printf("\n");
	std::printf("%-38s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------------------------\n");
	for (const stats &s : tbl)
		std::printf("%-38s %12llu %12llu\n", s.name, s.cases, s.failures);
	std::printf("--------------------------------------------------------------\n");
	std::printf("%-38s %12llu %12llu\n", "TOTAL", cases, failures);
	std::printf("\n%s\n", failures == 0 ? "PASS" : "FAIL");

	return failures == 0 ? 0 : 1;
}
