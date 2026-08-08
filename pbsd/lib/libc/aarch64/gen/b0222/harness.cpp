/*
 * Differential harness for batch b0222: pbsd.lib.libc.aarch64.gen.b0222 against
 * the unmodified C oracle in oracle.c.
 */

#include <cerrno>
#include <climits>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

import pbsd.lib.libc.aarch64.gen.b0222;

namespace port = pbsd::lib_libc_aarch64_gen::b0222;

namespace ref_abi {

using __register_t = std::int64_t;
using __uint64_t = std::uint64_t;
using __uint32_t = std::uint32_t;

typedef struct {
	__uint32_t __bits[4];
} ref_sigset_t;

struct gpregs {
	__register_t	gp_x[30];
	__register_t	gp_lr;
	__register_t	gp_sp;
	__register_t	gp_elr;
	__uint64_t	gp_spsr;
};

struct fpregs {
	unsigned __int128	fp_q[32];
	__uint32_t	fp_sr;
	__uint32_t	fp_cr;
	int		fp_flags;
	int		fp_pad;
};

struct mcontext {
	struct gpregs	mc_gpregs;
	struct fpregs	mc_fpregs;
	int		mc_flags;
	int		mc_pad;
	__uint64_t	mc_ptr;
	__uint64_t	mc_spare[7];
};

struct stack_t {
	void		*ss_sp;
	std::size_t	ss_size;
	int		ss_flags;
};

struct ucontext {
	ref_sigset_t	uc_sigmask;
	struct mcontext	uc_mcontext;
	struct ucontext	*uc_link;
	struct stack_t	uc_stack;
	int		uc_flags;
	int		__spare__[4];
};

} /* namespace ref_abi */

static_assert(sizeof(ref_abi::ucontext) == sizeof(port::ucontext_t));
static_assert(offsetof(ref_abi::ucontext, uc_mcontext.mc_ptr) ==
    offsetof(port::ucontext_t, uc_mcontext.mc_ptr));
static_assert(offsetof(ref_abi::ucontext, uc_link) ==
    offsetof(port::ucontext_t, uc_link));

extern "C" {
int ref___getcontextx_size(void);
int ref___fillcontextx2(char *ctx);
int ref___fillcontextx(char *ctx);
ref_abi::ucontext *ref___getcontextx(void);
void ref_ctx_done(ref_abi::ucontext *ucp);
void ref___makecontext(ref_abi::ucontext *ucp, void (*func)(void), int argc, ...);
}

enum {
	F_GETCONTEXTX_SIZE,
	F_FILLCONTEXTX2,
	F_FILLCONTEXTX,
	F_GETCONTEXTX,
	F_MAKECONTEXT,
	F_CTX_DONE,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"__getcontextx_size",
	"__fillcontextx2",
	"__fillcontextx",
	"__getcontextx",
	"__makecontext",
	"ctx_done",
};

static long long ncases[NFUNC];
static long long nfails[NFUNC];
static int nreported;
static const int MAXREPORT = 20;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-20s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

static int g_getcontext_fail;
static int g_getcontext_errno = EINTR;
static unsigned g_getcontext_seq;
static int g_malloc_fail;

struct DoneLog {
	int			exit_called;
	int			exit_status;
	ref_abi::ucontext	*setcontext_arg;
	int			abort_called;
};

static DoneLog g_done_log;

extern "C" void
_ctx_start(void)
{
}

extern "C" int
getcontext(void *ucp)
{
	auto *u = (ref_abi::ucontext *)ucp;

	if (g_getcontext_fail) {
		errno = g_getcontext_errno;
		return (-1);
	}
	std::memset(u, 0xa5, sizeof(*u));
	u->uc_flags = (int)(0xbe000000u | (++g_getcontext_seq));
	u->uc_mcontext.mc_gpregs.gp_x[0] = (ref_abi::__register_t)g_getcontext_seq;
	return (0);
}

extern "C" void
__wrap_exit(int status)
{
	g_done_log.exit_called = 1;
	g_done_log.exit_status = status;
}

extern "C" int
setcontext(const ref_abi::ucontext *ucp)
{
	g_done_log.setcontext_arg = (ref_abi::ucontext *)ucp;
	return (0);
}

extern "C" void
__wrap_abort(void)
{
	g_done_log.abort_called = 1;
}

extern "C" void *
__real_malloc(size_t);

extern "C" void *
__wrap_malloc(size_t sz)
{
	if (g_malloc_fail)
		return (NULL);
	return (__real_malloc(sz));
}

static void
mock_reset(void)
{
	g_getcontext_fail = 0;
	g_getcontext_errno = EINTR;
	g_getcontext_seq = 0;
	g_malloc_fail = 0;
	std::memset(&g_done_log, 0, sizeof g_done_log);
}

static uint64_t rng_state = 0x243f6a8885a308d3ULL;

static uint64_t
nextr(void)
{
	uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return (x * 0x2545F4914F6CDD1DULL);
}

static uint64_t
rand_u64(void)
{
	return (nextr());
}

static int
rand_argc(void)
{
	return ((int)((long long)(nextr() % 13) - 2));
}

static const unsigned char GUARD_BYTE = 0x7f;
static const size_t GUARD = 64;

struct CtxBuf {
	unsigned char	*base;
	size_t		total;
	size_t		ctxsz;
	char		*ctx;

	void init(size_t ctx_size)
	{
		ctxsz = ctx_size;
		total = GUARD + ctxsz + GUARD;
		base = (unsigned char *)std::malloc(total);
		std::memset(base, GUARD_BYTE, total);
		ctx = (char *)(base + GUARD);
	}

	void fill_random(void)
	{
		for (size_t i = 0; i < ctxsz; i++)
			ctx[i] = (char)(rand_u64() & 0xff);
	}

	void fill_pattern(unsigned char b)
	{
		std::memset(ctx, (int)b, ctxsz);
	}

	bool guards_ok(void) const
	{
		for (size_t i = 0; i < GUARD; i++) {
			if (base[i] != GUARD_BYTE)
				return (false);
			if (base[GUARD + ctxsz + i] != GUARD_BYTE)
				return (false);
		}
		return (true);
	}

	void freebuf(void)
	{
		std::free(base);
		base = nullptr;
		ctx = nullptr;
	}
};

static bool
ctx_same(char *ctx_a, char *ctx_b, size_t sz, char *msg, size_t msgsz)
{
	std::vector<unsigned char> a(sz), b(sz);
	auto *ua = (ref_abi::ucontext *)a.data();
	auto *ub = (ref_abi::ucontext *)b.data();
	const uint64_t base_a = (uint64_t)(uintptr_t)ctx_a;
	const uint64_t base_b = (uint64_t)(uintptr_t)ctx_b;
	const uint64_t end_a = base_a + sz;
	const uint64_t end_b = base_b + sz;

	std::memcpy(a.data(), ctx_a, sz);
	std::memcpy(b.data(), ctx_b, sz);

	if (ua->uc_mcontext.mc_ptr >= base_a && ua->uc_mcontext.mc_ptr < end_a)
		ua->uc_mcontext.mc_ptr -= base_a;
	if (ub->uc_mcontext.mc_ptr >= base_b && ub->uc_mcontext.mc_ptr < end_b)
		ub->uc_mcontext.mc_ptr -= base_b;
	if (ua->uc_mcontext.mc_gpregs.gp_x[20] == base_a)
		ua->uc_mcontext.mc_gpregs.gp_x[20] = 0;
	if (ub->uc_mcontext.mc_gpregs.gp_x[20] == base_b)
		ub->uc_mcontext.mc_gpregs.gp_x[20] = 0;

	if (std::memcmp(a.data(), b.data(), sz) != 0) {
		for (size_t i = 0; i < sz; i++) {
			if (a[i] != b[i]) {
				std::snprintf(msg, msgsz,
				    "byte[%zu] port=%02x ref=%02x", i, a[i], b[i]);
				return (false);
			}
		}
	}
	return (true);
}

static void dummy_func0(void) {}
static void dummy_func1(void) {}

static void (*const dummy_funcs[])(void) = {
	dummy_func0, dummy_func1, dummy_func0, dummy_func1
};

static void
invoke_makecontext_ref(ref_abi::ucontext *ucp, void (*func)(void), int argc,
    const uint64_t args[8])
{
	if (argc < 0 || argc > 8) {
		ref___makecontext(ucp, func, argc);
		return;
	}
	switch (argc) {
	case 0: ref___makecontext(ucp, func, 0); break;
	case 1: ref___makecontext(ucp, func, 1, args[0]); break;
	case 2: ref___makecontext(ucp, func, 2, args[0], args[1]); break;
	case 3: ref___makecontext(ucp, func, 3, args[0], args[1], args[2]); break;
	case 4: ref___makecontext(ucp, func, 4, args[0], args[1], args[2], args[3]); break;
	case 5: ref___makecontext(ucp, func, 5, args[0], args[1], args[2], args[3], args[4]); break;
	case 6: ref___makecontext(ucp, func, 6, args[0], args[1], args[2], args[3], args[4], args[5]); break;
	case 7: ref___makecontext(ucp, func, 7, args[0], args[1], args[2], args[3], args[4], args[5], args[6]); break;
	default: ref___makecontext(ucp, func, 8, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]); break;
	}
}

static void
invoke_makecontext_port(port::ucontext_t *ucp, void (*func)(void), int argc,
    const uint64_t args[8])
{
	if (argc < 0 || argc > 8) {
		port::__makecontext(ucp, func, argc);
		return;
	}
	switch (argc) {
	case 0: port::__makecontext(ucp, func, 0); break;
	case 1: port::__makecontext(ucp, func, 1, args[0]); break;
	case 2: port::__makecontext(ucp, func, 2, args[0], args[1]); break;
	case 3: port::__makecontext(ucp, func, 3, args[0], args[1], args[2]); break;
	case 4: port::__makecontext(ucp, func, 4, args[0], args[1], args[2], args[3]); break;
	case 5: port::__makecontext(ucp, func, 5, args[0], args[1], args[2], args[3], args[4]); break;
	case 6: port::__makecontext(ucp, func, 6, args[0], args[1], args[2], args[3], args[4], args[5]); break;
	case 7: port::__makecontext(ucp, func, 7, args[0], args[1], args[2], args[3], args[4], args[5], args[6]); break;
	default: port::__makecontext(ucp, func, 8, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]); break;
	}
}

static void
setup_ucp_port(port::ucontext_t *ucp, uintptr_t sp_base, size_t ss_size,
    int ss_flags)
{
	std::memset(ucp, 0, sizeof(*ucp));
	ucp->uc_stack.ss_sp = (void *)sp_base;
	ucp->uc_stack.ss_size = ss_size;
	ucp->uc_stack.ss_flags = ss_flags;
}

static void
setup_ucp_ref(ref_abi::ucontext *ucp, uintptr_t sp_base, size_t ss_size,
    int ss_flags)
{
	std::memset(ucp, 0, sizeof(*ucp));
	ucp->uc_stack.ss_sp = (void *)sp_base;
	ucp->uc_stack.ss_size = ss_size;
	ucp->uc_stack.ss_flags = ss_flags;
}

static void
op_getcontextx_size(const char *ctx)
{
	char msg[256];

	ncases[F_GETCONTEXTX_SIZE]++;
	int a = port::__getcontextx_size();
	mock_reset();
	int b = ref___getcontextx_size();
	if (a != b) {
		std::snprintf(msg, sizeof msg, "return port=%d ref=%d", a, b);
		report(F_GETCONTEXTX_SIZE, ctx, msg);
	}
}

static void
op_fillcontextx2(size_t sz, int fill_mode, const char *ctx)
{
	char msg[256];
	CtxBuf pa, pb;

	ncases[F_FILLCONTEXTX2]++;
	pa.init(sz);
	pb.init(sz);
	if (fill_mode == 0)
		pa.fill_pattern(0x00);
	else if (fill_mode == 1)
		pa.fill_pattern(0xff);
	else if (fill_mode == 2)
		pa.fill_pattern(0x80);
	else
		pa.fill_random();
	std::memcpy(pb.base, pa.base, pa.total);

	mock_reset();
	int ra = port::__fillcontextx2(pa.ctx);
	mock_reset();
	int rb = ref___fillcontextx2(pb.ctx);

	if (ra != rb) {
		std::snprintf(msg, sizeof msg, "return port=%d ref=%d", ra, rb);
		report(F_FILLCONTEXTX2, ctx, msg);
		goto done;
	}
	if (!pa.guards_ok() || !pb.guards_ok()) {
		report(F_FILLCONTEXTX2, ctx, "guard corruption");
		goto done;
	}
	if (!ctx_same(pa.ctx, pb.ctx, pa.ctxsz, msg, sizeof msg))
		report(F_FILLCONTEXTX2, ctx, msg);

done:
	pa.freebuf();
	pb.freebuf();
}

static void
op_fillcontextx(size_t sz, int fill_mode, int gc_fail, int gc_errno,
    const char *ctx)
{
	char msg[256];
	CtxBuf pa, pb;

	ncases[F_FILLCONTEXTX]++;
	pa.init(sz);
	pb.init(sz);
	if (fill_mode == 0)
		pa.fill_pattern(0x80);
	else
		pa.fill_random();
	std::memcpy(pb.base, pa.base, pa.total);

	mock_reset();
	g_getcontext_fail = gc_fail;
	g_getcontext_errno = gc_errno;
	int ra = port::__fillcontextx(pa.ctx);

	mock_reset();
	g_getcontext_fail = gc_fail;
	g_getcontext_errno = gc_errno;
	int rb = ref___fillcontextx(pb.ctx);

	if (ra != rb) {
		std::snprintf(msg, sizeof msg, "return port=%d ref=%d", ra, rb);
		report(F_FILLCONTEXTX, ctx, msg);
		goto done;
	}
	if (!pa.guards_ok() || !pb.guards_ok()) {
		report(F_FILLCONTEXTX, ctx, "guard corruption");
		goto done;
	}
	if (!ctx_same(pa.ctx, pb.ctx, pa.ctxsz, msg, sizeof msg))
		report(F_FILLCONTEXTX, ctx, msg);

done:
	pa.freebuf();
	pb.freebuf();
}

static void
op_getcontextx(int malloc_fail, int gc_fail, int gc_errno, const char *ctx)
{
	char msg[256];
	int save_errno = errno;

	ncases[F_GETCONTEXTX]++;

	mock_reset();
	g_malloc_fail = malloc_fail;
	g_getcontext_fail = gc_fail;
	g_getcontext_errno = gc_errno;
	errno = save_errno;
	port::ucontext_t *pa = port::__getcontextx();
	int err_a = errno;
	size_t sz = (size_t)port::__getcontextx_size();

	mock_reset();
	g_malloc_fail = malloc_fail;
	g_getcontext_fail = gc_fail;
	g_getcontext_errno = gc_errno;
	errno = save_errno;
	ref_abi::ucontext *pb = ref___getcontextx();
	int err_b = errno;

	if ((pa == nullptr) != (pb == nullptr)) {
		std::snprintf(msg, sizeof msg, "null mismatch port=%p ref=%p",
		    (const void *)pa, (const void *)pb);
		report(F_GETCONTEXTX, ctx, msg);
		goto cleanup;
	}
	if (err_a != err_b) {
		std::snprintf(msg, sizeof msg, "errno port=%d ref=%d", err_a, err_b);
		report(F_GETCONTEXTX, ctx, msg);
		goto cleanup;
	}
	if (pa != nullptr) {
		char msg[256];
		if (!ctx_same((char *)pa, (char *)pb, sz, msg, sizeof msg))
			report(F_GETCONTEXTX, ctx, msg);
	}

cleanup:
	std::free(pa);
	std::free(pb);
}

static void
op_makecontext(port::ucontext_t *ucp_port, ref_abi::ucontext *ucp_ref,
    void (*func)(void), int argc, const uint64_t args[8], const char *ctx)
{
	char msg[256];

	ncases[F_MAKECONTEXT]++;

	if (ucp_port == nullptr && ucp_ref == nullptr) {
		mock_reset();
		invoke_makecontext_port(nullptr, func, argc, args);
		mock_reset();
		invoke_makecontext_ref(nullptr, func, argc, args);
		return;
	}
	if (ucp_port == nullptr || ucp_ref == nullptr)
		return;

	port::ucontext_t snap_port = *ucp_port;
	ref_abi::ucontext snap_ref = *ucp_ref;

	mock_reset();
	invoke_makecontext_port(ucp_port, func, argc, args);
	mock_reset();
	invoke_makecontext_ref(ucp_ref, func, argc, args);

	if (std::memcmp(&snap_port, &snap_ref, sizeof(snap_port)) != 0) {
		report(F_MAKECONTEXT, ctx, "input ucontext mismatch before call");
		return;
	}

	port::ucontext_t norm_port = *ucp_port;
	ref_abi::ucontext norm_ref = *ucp_ref;
	norm_port.uc_mcontext.mc_gpregs.gp_x[20] = 0;
	norm_ref.uc_mcontext.mc_gpregs.gp_x[20] = 0;

	if (std::memcmp(&norm_port, &norm_ref, sizeof(norm_port)) != 0) {
		for (size_t off = 0; off < sizeof(norm_port); off++) {
			const unsigned char *ap = (const unsigned char *)&norm_port;
			const unsigned char *bp = (const unsigned char *)&norm_ref;
			if (ap[off] != bp[off]) {
				std::snprintf(msg, sizeof msg,
				    "uc mismatch at %zu port=%02x ref=%02x",
				    off, ap[off], bp[off]);
				report(F_MAKECONTEXT, ctx, msg);
				return;
			}
		}
	}
}

[[gnu::noinline]] static void
op_ctx_done(port::ucontext_t *ucp_port, ref_abi::ucontext *ucp_ref,
    const char *ctx)
{
	char msg[256];
	DoneLog la, lb;

	ncases[F_CTX_DONE]++;

	std::fprintf(stderr, "op_ctx_done: port ctx_done\n");

	mock_reset();
	port::ctx_done(ucp_port);
	la = g_done_log;

	mock_reset();
	ref_ctx_done(ucp_ref);
	lb = g_done_log;

	if (la.exit_called != lb.exit_called) {
		std::snprintf(msg, sizeof msg, "exit_called port=%d ref=%d",
		    la.exit_called, lb.exit_called);
		report(F_CTX_DONE, ctx, msg);
		return;
	}
	if (la.exit_status != lb.exit_status) {
		std::snprintf(msg, sizeof msg, "exit_status port=%d ref=%d",
		    la.exit_status, lb.exit_status);
		report(F_CTX_DONE, ctx, msg);
		return;
	}
	if (la.abort_called != lb.abort_called) {
		std::snprintf(msg, sizeof msg, "abort_called port=%d ref=%d",
		    la.abort_called, lb.abort_called);
		report(F_CTX_DONE, ctx, msg);
		return;
	}
	if ((la.setcontext_arg == nullptr) != (lb.setcontext_arg == nullptr)) {
		report(F_CTX_DONE, ctx, "setcontext null mismatch");
		return;
	}
	if (la.setcontext_arg != nullptr &&
	    (void *)la.setcontext_arg != (void *)ucp_port->uc_link) {
		report(F_CTX_DONE, ctx, "port setcontext arg mismatch");
		return;
	}
	if (lb.setcontext_arg != nullptr &&
	    lb.setcontext_arg != ucp_ref->uc_link) {
		report(F_CTX_DONE, ctx, "ref setcontext arg mismatch");
		return;
	}
}

static void edge_getcontextx_size(void) { op_getcontextx_size("edge/size"); }

static void edge_fillcontextx2(void)
{
	size_t sz = (size_t)port::__getcontextx_size();
	op_fillcontextx2(sz, 0, "edge/fill2/zero");
	op_fillcontextx2(sz, 1, "edge/fill2/ff");
	op_fillcontextx2(sz, 3, "edge/fill2/random");
}

static void edge_fillcontextx(void)
{
	size_t sz = (size_t)port::__getcontextx_size();
	op_fillcontextx(sz, 0, 0, 0, "edge/fill/ok");
	op_fillcontextx(sz, 0, 1, EAGAIN, "edge/fill/fail");
}

static void edge_getcontextx(void)
{
	op_getcontextx(1, 0, 0, "edge/getx/malloc-fail");
	op_getcontextx(0, 1, ENOMEM, "edge/getx/gc-fail");
	op_getcontextx(0, 0, 0, "edge/getx/ok");
}

static void edge_makecontext(void)
{
	port::ucontext_t up;
	ref_abi::ucontext ur;
	unsigned char stackp[256];
	uint64_t args[8];
	char ctx[96];
	static const int argcs[] = { -1, 0, 1, 7, 8, 9 };
	static const size_t sizes[] = { 0, 1, 15, 16, 17, 127, 255 };

	for (int ai = 0; ai < (int)(sizeof argcs / sizeof argcs[0]); ai++) {
		int argc = argcs[ai];
		for (size_t si = 0; si < sizeof sizes / sizeof sizes[0]; si++) {
			for (int fi = 0; fi < (int)(sizeof dummy_funcs / sizeof dummy_funcs[0]); fi++) {
				std::memset(&up, 0, sizeof up);
				std::memset(&ur, 0, sizeof ur);
				std::memset(stackp, 0xa5, sizeof stackp);
				setup_ucp_port(&up, (uintptr_t)stackp, sizes[si], 0);
				setup_ucp_ref(&ur, (uintptr_t)stackp, sizes[si], 0);
				for (int k = 0; k < 8; k++)
					args[k] = (uint64_t)(0x80 + k) | ((uint64_t)k << 32);
				std::snprintf(ctx, sizeof ctx,
				    "edge/make/argc=%d/size=%zu/f=%d", argc, sizes[si], fi);
				op_makecontext(&up, &ur, dummy_funcs[fi], argc, args, ctx);
			}
		}
	}
	std::snprintf(ctx, sizeof ctx, "edge/make/null");
	op_makecontext(nullptr, nullptr, dummy_func0, 3, args, ctx);
	for (uintptr_t base = 1; base <= 32; base++) {
		std::memset(&up, 0, sizeof up);
		std::memset(&ur, 0, sizeof ur);
		setup_ucp_port(&up, base, 32, 0);
		setup_ucp_ref(&ur, base, 32, 0);
		for (int k = 0; k < 8; k++)
			args[k] = rand_u64();
		std::snprintf(ctx, sizeof ctx, "edge/make/align/base=%ju", (uintmax_t)base);
		op_makecontext(&up, &ur, dummy_func1, 8, args, ctx);
	}
}

static void edge_ctx_done(void)
{
	static port::ucontext_t up;
	static ref_abi::ucontext ur;
	static port::ucontext_t lp_port;
	static ref_abi::ucontext lr_ref;
	static port::ucontext_t up_link;
	static ref_abi::ucontext ur_link;

	std::memset(&up, 0, sizeof up);
	std::memset(&ur, 0, sizeof ur);
	up.uc_link = nullptr;
	ur.uc_link = nullptr;
	op_ctx_done(&up, &ur, "edge/done/no-link");

	std::memset(&lp_port, 0xcc, sizeof lp_port);
	std::memset(&lr_ref, 0xcc, sizeof lr_ref);
	std::memset(&up_link, 0, sizeof up_link);
	std::memset(&ur_link, 0, sizeof ur_link);
	up_link.uc_link = &lp_port;
	ur_link.uc_link = &lr_ref;
	op_ctx_done(&up_link, &ur_link, "edge/done/with-link");
}

static const long SWEEP = 200000;

static void sweep_getcontextx_size(void)
{
	char ctx[64];
	for (long i = 0; i < SWEEP; i++) {
		std::snprintf(ctx, sizeof ctx, "sweep/size[%ld]", i);
		op_getcontextx_size(ctx);
	}
}

static void sweep_fillcontextx2(void)
{
	size_t sz = (size_t)port::__getcontextx_size();
	char ctx[64];
	for (long i = 0; i < SWEEP; i++) {
		std::snprintf(ctx, sizeof ctx, "sweep/fill2[%ld]", i);
		op_fillcontextx2(sz, (int)(i % 4), ctx);
	}
}

static void sweep_fillcontextx(void)
{
	size_t sz = (size_t)port::__getcontextx_size();
	char ctx[64];
	for (long i = 0; i < SWEEP; i++) {
		int fail = (int)(nextr() % 5 == 0);
		int err = (int)(nextr() & 0x7fffffff);
		if (err == 0)
			err = EIO;
		std::snprintf(ctx, sizeof ctx, "sweep/fill[%ld]", i);
		op_fillcontextx(sz, (int)(i & 1), fail, err, ctx);
	}
}

static void sweep_getcontextx(void)
{
	char ctx[64];
	for (long i = 0; i < SWEEP; i++) {
		int mf = (int)(nextr() % 17 == 0);
		int gf = mf ? 0 : (int)(nextr() % 11 == 0);
		int ge = (int)(nextr() & 0xffff);
		if (ge == 0)
			ge = EINVAL;
		std::snprintf(ctx, sizeof ctx, "sweep/getx[%ld]", i);
		op_getcontextx(mf, gf, ge, ctx);
	}
}

static void sweep_makecontext(void)
{
	port::ucontext_t up;
	ref_abi::ucontext ur;
	unsigned char stackp[512];
	uint64_t args[8];
	char ctx[64];

	for (long i = 0; i < SWEEP; i++) {
		int argc = rand_argc();
		if (i % 500 == 0)
			argc = -1;
		else if (i % 501 == 0)
			argc = 9;
		else if (i % 502 == 0)
			argc = 8;
		else if (argc < 0)
			argc = -1;
		else if (argc > 8)
			argc = 8;

		std::memset(&up, 0, sizeof up);
		std::memset(&ur, 0, sizeof ur);
		std::memset(stackp, (int)(0x80 | (rand_u64() & 0x7f)), sizeof stackp);
		size_t ss = (size_t)(nextr() % 400);
		uintptr_t off = (uintptr_t)(nextr() % 32);
		uintptr_t sp = (uintptr_t)(stackp + off);
		setup_ucp_port(&up, sp, ss, (int)(nextr() & 3));
		setup_ucp_ref(&ur, sp, ss, up.uc_stack.ss_flags);
		for (int k = 0; k < 8; k++)
			args[k] = rand_u64();
		void (*fn)(void) = dummy_funcs[(int)(nextr() % 4)];
		if (i % 997 == 0)
			fn = nullptr;
		std::snprintf(ctx, sizeof ctx, "sweep/make[%ld]", i);
		op_makecontext(&up, &ur, fn, argc, args, ctx);
	}
}

static void sweep_ctx_done(void)
{
	static port::ucontext_t lp_port;
	static ref_abi::ucontext lr_ref;
	port::ucontext_t up;
	ref_abi::ucontext ur;
	char ctx[64];

	for (long i = 0; i < SWEEP; i++) {
		std::memset(&up, 0, sizeof up);
		std::memset(&ur, 0, sizeof ur);
		std::memset(&lp_port, (int)(rand_u64() & 0xff), sizeof lp_port);
		std::memset(&lr_ref, (int)(rand_u64() & 0xff), sizeof lr_ref);
		if (nextr() & 1) {
			up.uc_link = nullptr;
			ur.uc_link = nullptr;
		} else {
			up.uc_link = &lp_port;
			ur.uc_link = &lr_ref;
		}
		std::snprintf(ctx, sizeof ctx, "sweep/done[%ld]", i);
		op_ctx_done(&up, &ur, ctx);
	}
}

int
main(void)
{
	long long total_cases = 0, total_fails = 0;

	edge_getcontextx_size();
	edge_fillcontextx2();
	edge_fillcontextx();
	edge_getcontextx();
	edge_makecontext();
	edge_ctx_done();

	sweep_getcontextx_size();
	sweep_fillcontextx2();
	sweep_fillcontextx();
	sweep_getcontextx();
	sweep_makecontext();
	sweep_ctx_done();

	std::printf("\n");
	std::printf("%-20s %10s %10s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------------\n");
	for (int f = 0; f < NFUNC; f++) {
		std::printf("%-20s %10lld %10lld\n", fname[f], ncases[f], nfails[f]);
		total_cases += ncases[f];
		total_fails += nfails[f];
	}
	std::printf("--------------------------------------------------\n");
	std::printf("%-20s %10lld %10lld\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
