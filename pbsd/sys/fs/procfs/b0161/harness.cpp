// Differential test harness for PBSD batch b0161.

import pbsd.sys.fs.procfs.b0161;

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <pthread.h>

namespace P = pbsd::sys_fs_procfs::b0161;

#define GUARD     0x7f
#define PAD       32u
#define DATA_CAP  128u
#define ARENA_SZ  (PAD + DATA_CAP + PAD)
#define SWEEP     200000L
#define MAX_SHOW  8

#define EOPNOTSUPP 45
#define EINVAL     22

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int shown;
};

static Stat st_note = { "procfs_doprocnote", 0, 0, 0 };
static Stat st_osrel = { "procfs_doosrel", 0, 0, 0 };
static Stat st_mem = { "procfs_doprocmem", 0, 0, 0 };

extern "C" {
struct mtx {
	pthread_mutex_t lock;
};
struct thread {
	int dummy;
};
struct pfs_node {
	int dummy;
};
struct proc {
	struct mtx p_mtx;
	int p_osrel;
};
struct sbuf {
	char *s_buf;
	ssize_t s_size;
	ssize_t s_len;
	int s_flags;
	int s_error;
};
enum uio_rw { UIO_READ, UIO_WRITE };
struct uio {
	ssize_t uio_resid;
	enum uio_rw uio_rw;
};

#define PFS_FILL_ARGS \
	struct thread *td, struct proc *p, struct pfs_node *pn, \
	struct sbuf *sb, struct uio *uio

int ref_procfs_doprocnote(PFS_FILL_ARGS);
int ref_procfs_doosrel(PFS_FILL_ARGS);
int ref_procfs_doprocmem(PFS_FILL_ARGS);
void ref_stub_reset(void);

extern int ref_p_candebug_ret;
extern int ref_proc_rwmem_ret;
extern ssize_t ref_proc_rwmem_resid;
extern int ref_proc_rwmem_calls;
}

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	std::uint32_t below(std::uint32_t n) { return (std::uint32_t)(next() % n); }
	bool coin() { return (next() & 1) != 0; }
};

static Rng rng(0x00b0161faceULL);

static bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

static long
first_diff(const void *a, const void *b, size_t n)
{
	const unsigned char *pa = (const unsigned char *)a;
	const unsigned char *pb = (const unsigned char *)b;

	for (size_t i = 0; i < n; i++)
		if (pa[i] != pb[i])
			return (long)i;
	return -1;
}

static void
init_proc(struct proc *p, int osrel)
{
	std::memset(p, 0, sizeof(*p));
	pthread_mutex_init(&p->p_mtx.lock, nullptr);
	p->p_osrel = osrel;
}

template <typename Proc>
static void
init_proc_t(Proc *p, int osrel)
{
	std::memset(p, 0, sizeof(*p));
	pthread_mutex_init(&p->p_mtx.lock, nullptr);
	p->p_osrel = osrel;
}

static void
init_sbuf_data(struct sbuf *sb, char *data, ssize_t cap, const char *text)
{
	std::memset(sb, 0, sizeof(*sb));
	sb->s_buf = data;
	sb->s_size = cap;
	if (text != nullptr) {
		size_t n = std::strlen(text);
		if (n >= (size_t)cap)
			n = (size_t)cap - 1;
		std::memcpy(data, text, n);
		sb->s_len = (ssize_t)n;
	}
}

template <typename Sbuf>
static void
init_sbuf_data_t(Sbuf *sb, char *data, ssize_t cap, const char *text)
{
	std::memset(sb, 0, sizeof(*sb));
	sb->s_buf = data;
	sb->s_size = cap;
	if (text != nullptr) {
		size_t n = std::strlen(text);
		if (n >= (size_t)cap)
			n = (size_t)cap - 1;
		std::memcpy(data, text, n);
		sb->s_len = (ssize_t)n;
	}
}

static void
fill_arena(unsigned char *arena, const char *text, size_t text_len)
{
	std::memset(arena, GUARD, ARENA_SZ);
	if (text_len > DATA_CAP)
		text_len = DATA_CAP;
	std::memcpy(arena + PAD, text, text_len);
}

static void
fill_arena_bytes(unsigned char *arena, const unsigned char *bytes, size_t n)
{
	std::memset(arena, GUARD, ARENA_SZ);
	if (n > DATA_CAP)
		n = DATA_CAP;
	std::memcpy(arena + PAD, bytes, n);
}

static bool
cmp_arenas(Stat &st, const unsigned char *a, const unsigned char *b)
{
	long d = first_diff(a, b, ARENA_SZ);
	if (d >= 0) {
		std::printf("    arena diff at %ld: %02x vs %02x\n", d, a[d], b[d]);
		fail(st, "arena mismatch");
		return false;
	}
	return true;
}

static bool
cmp_sbuf_meta(Stat &st, const struct sbuf *pa, const P::sbuf *pb)
{
	if (pa->s_len != pb->s_len) {
		std::printf("    s_len %zd vs %zd\n", pa->s_len, pb->s_len);
		return fail(st, "s_len mismatch");
	}
	if (pa->s_flags != pb->s_flags) {
		std::printf("    s_flags %x vs %x\n", pa->s_flags, pb->s_flags);
		return fail(st, "s_flags mismatch");
	}
	if (pa->s_error != pb->s_error) {
		std::printf("    s_error %d vs %d\n", pa->s_error, pb->s_error);
		return fail(st, "s_error mismatch");
	}
	return true;
}

static void
stub_reset_both()
{
	P::stub_reset();
	ref_stub_reset();
}

static bool
run_note_case(const char *label, const char *text)
{
	st_note.cases++;

	unsigned char arena_r[ARENA_SZ];
	unsigned char arena_p[ARENA_SZ];
	size_t tlen = text ? std::strlen(text) : 0;

	fill_arena(arena_r, text, tlen);
	fill_arena(arena_p, text, tlen);

	struct thread td_r{};
	struct pfs_node pn_r{};
	struct proc pr_r{};
	struct sbuf sb_r{};
	P::thread td_p2{};
	P::pfs_node pn_p2{};
	P::proc pr_p2{};
	P::sbuf sb_p2{};
	struct uio uio_r{};
	P::uio uio_p2{};

	init_proc(&pr_r, 0);
	init_proc_t(&pr_p2, 0);

	init_sbuf_data(&sb_r, (char *)(arena_r + PAD), DATA_CAP, text);
	init_sbuf_data_t(&sb_p2, (char *)(arena_p + PAD), DATA_CAP, text);

	int ret_r = ref_procfs_doprocnote(&td_r, &pr_r, &pn_r, &sb_r, &uio_r);
	int ret_p = P::procfs_doprocnote(&td_p2, &pr_p2, &pn_p2, &sb_p2, &uio_p2);

	if (ret_r != ret_p) {
		std::printf("    ret %d vs %d (%s)\n", ret_r, ret_p, label);
		fail(st_note, label);
		return false;
	}
	if (!cmp_arenas(st_note, arena_r, arena_p))
		return false;
	if (!cmp_sbuf_meta(st_note, &sb_r, &sb_p2))
		return fail(st_note, label);
	return true;
}

static void
test_note_hand()
{
	run_note_case("empty", "");
	run_note_case("single", "a");
	run_note_case("digits", "12345");
	run_note_case("ws_tail", "hello   \t\n");
	run_note_case("nul_heavy", "a\0b\0c");
	run_note_case("high_bit", "\x80\xff\xfe");
	run_note_case("boundary", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
}

static void
test_note_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		unsigned char bytes[DATA_CAP];
		size_t n = rng.below(DATA_CAP + 1);
		for (size_t j = 0; j < n; j++) {
			if (rng.coin())
				bytes[j] = (unsigned char)(rng.below(256));
			else
				bytes[j] = (unsigned char)('0' + rng.below(10));
		}
		st_note.cases++;
		unsigned char arena_r[ARENA_SZ];
		unsigned char arena_p[ARENA_SZ];
		fill_arena_bytes(arena_r, bytes, n);
		fill_arena_bytes(arena_p, bytes, n);

		struct thread td_r{};
		struct pfs_node pn_r{};
		struct proc pr_r{};
		struct sbuf sb_r{};
		P::thread td_p2{};
		P::pfs_node pn_p2{};
		P::proc pr_p2{};
		P::sbuf sb_p2{};
		struct uio uio_r{};
		P::uio uio_p2{};

		init_proc(&pr_r, 0);
		init_proc_t(&pr_p2, 0);
		init_sbuf_data(&sb_r, (char *)(arena_r + PAD), DATA_CAP, nullptr);
		sb_r.s_len = (ssize_t)n;
		init_sbuf_data_t(&sb_p2, (char *)(arena_p + PAD), DATA_CAP, nullptr);
		sb_p2.s_len = (ssize_t)n;

		int ret_r = ref_procfs_doprocnote(&td_r, &pr_r, &pn_r, &sb_r, &uio_r);
		int ret_p = P::procfs_doprocnote(&td_p2, &pr_p2, &pn_p2, &sb_p2,
		    &uio_p2);

		if (ret_r != ret_p || first_diff(arena_r, arena_p, ARENA_SZ) >= 0 ||
		    sb_r.s_len != sb_p2.s_len || sb_r.s_flags != sb_p2.s_flags)
			fail(st_note, "sweep");
	}
}

static bool
run_osrel_read(int osrel)
{
	st_osrel.cases++;

	unsigned char arena_r[ARENA_SZ];
	unsigned char arena_p[ARENA_SZ];
	std::memset(arena_r, GUARD, ARENA_SZ);
	std::memset(arena_p, GUARD, ARENA_SZ);

	struct thread td_r{};
	struct pfs_node pn_r{};
	struct proc pr_r{};
	struct sbuf sb_r{};
	P::thread td_p2{};
	P::pfs_node pn_p2{};
	P::proc pr_p2{};
	P::sbuf sb_p2{};
	struct uio uio_r{};
	P::uio uio_p2{};

	init_proc(&pr_r, osrel);
	init_proc_t(&pr_p2, osrel);
	init_sbuf_data(&sb_r, (char *)(arena_r + PAD), DATA_CAP, "");
	init_sbuf_data_t(&sb_p2, (char *)(arena_p + PAD), DATA_CAP, "");
	uio_r.uio_rw = UIO_READ;
	uio_p2.uio_rw = P::UIO_READ;

	int ret_r = ref_procfs_doosrel(&td_r, &pr_r, &pn_r, &sb_r, &uio_r);
	int ret_p = P::procfs_doosrel(&td_p2, &pr_p2, &pn_p2, &sb_p2, &uio_p2);

	if (ret_r != ret_p) {
		std::printf("    read osrel=%d ret %d vs %d\n", osrel, ret_r, ret_p);
		return fail(st_osrel, "read ret");
	}
	if (!cmp_arenas(st_osrel, arena_r, arena_p))
		return false;
	if (!cmp_sbuf_meta(st_osrel, &sb_r, &sb_p2))
		return false;
	return true;
}

static bool
run_osrel_write(const char *label, const char *text, int expect_ret,
    int expect_osrel)
{
	st_osrel.cases++;

	unsigned char arena_r[ARENA_SZ];
	unsigned char arena_p[ARENA_SZ];
	size_t tlen = std::strlen(text);

	fill_arena(arena_r, text, tlen);
	fill_arena(arena_p, text, tlen);

	struct thread td_r{};
	struct pfs_node pn_r{};
	struct proc pr_r{};
	struct sbuf sb_r{};
	P::thread td_p2{};
	P::pfs_node pn_p2{};
	P::proc pr_p2{};
	P::sbuf sb_p2{};
	struct uio uio_r{};
	P::uio uio_p2{};

	init_proc(&pr_r, 99);
	init_proc_t(&pr_p2, 99);
	init_sbuf_data(&sb_r, (char *)(arena_r + PAD), DATA_CAP, text);
	init_sbuf_data_t(&sb_p2, (char *)(arena_p + PAD), DATA_CAP, text);
	uio_r.uio_rw = UIO_WRITE;
	uio_p2.uio_rw = P::UIO_WRITE;

	int ret_r = ref_procfs_doosrel(&td_r, &pr_r, &pn_r, &sb_r, &uio_r);
	int ret_p = P::procfs_doosrel(&td_p2, &pr_p2, &pn_p2, &sb_p2, &uio_p2);

	if (ret_r != ret_p || ret_r != expect_ret) {
		std::printf("    write %s ret %d/%d expect %d\n", label, ret_r,
		    ret_p, expect_ret);
		fail(st_osrel, label);
		return false;
	}
	if (pr_r.p_osrel != pr_p2.p_osrel || pr_r.p_osrel != expect_osrel) {
		std::printf("    write %s osrel %d/%d expect %d\n", label,
		    pr_r.p_osrel, pr_p2.p_osrel, expect_osrel);
		fail(st_osrel, label);
		return false;
	}
	if (!cmp_arenas(st_osrel, arena_r, arena_p))
		return false;
	if (!cmp_sbuf_meta(st_osrel, &sb_r, &sb_p2))
		return false;
	return true;
}

static bool
run_osrel_null_uio()
{
	st_osrel.cases++;

	struct thread td_r{};
	struct pfs_node pn_r{};
	struct proc pr_r{};
	struct sbuf sb_r{};
	P::thread td_p2{};
	P::pfs_node pn_p2{};
	P::proc pr_p2{};
	P::sbuf sb_p2{};
	unsigned char arena_r[ARENA_SZ];
	unsigned char arena_p[ARENA_SZ];

	std::memset(arena_r, GUARD, ARENA_SZ);
	std::memset(arena_p, GUARD, ARENA_SZ);
	init_proc(&pr_r, 0);
	init_proc_t(&pr_p2, 0);
	init_sbuf_data(&sb_r, (char *)(arena_r + PAD), DATA_CAP, "x");
	init_sbuf_data_t(&sb_p2, (char *)(arena_p + PAD), DATA_CAP, "x");

	int ret_r = ref_procfs_doosrel(&td_r, &pr_r, &pn_r, &sb_r, nullptr);
	int ret_p = P::procfs_doosrel(&td_p2, &pr_p2, &pn_p2, &sb_p2, nullptr);

	if (ret_r != EOPNOTSUPP || ret_p != EOPNOTSUPP)
		return fail(st_osrel, "null uio");
	return true;
}

static void
test_osrel_hand()
{
	run_osrel_null_uio();
	run_osrel_read(0);
	run_osrel_read(-1);
	run_osrel_read(INT_MAX);
	run_osrel_read(INT_MIN);
	run_osrel_write("zero", "0", 0, 0);
	run_osrel_write("digits", "12345", 0, 12345);
	run_osrel_write("ws_tail", "42  \n", 0, 42);
	run_osrel_write("ws_lead", "  42", EINVAL, 99);
	run_osrel_write("empty", "", 0, 0);
	run_osrel_write("max_ok", "2147483647", 0, 2147483647);
	run_osrel_write("overflow", "2147483648", EINVAL, 99);
	run_osrel_write("wrap_pos", "9999999999", 0, 1410065407);
	run_osrel_write("alpha", "abc", EINVAL, 99);
	run_osrel_write("mid_alpha", "12a34", EINVAL, 99);
	run_osrel_write("sign", "-1", EINVAL, 99);
	run_osrel_write("plus", "+1", EINVAL, 99);
	run_osrel_write("high", "\x80", EINVAL, 99);
	run_osrel_write("ff", "\xff", EINVAL, 99);
}

static bool
run_osrel_write_diff(const char *label, const char *text)
{
	st_osrel.cases++;

	unsigned char arena_r[ARENA_SZ];
	unsigned char arena_p[ARENA_SZ];
	size_t tlen = std::strlen(text);

	fill_arena(arena_r, text, tlen);
	fill_arena(arena_p, text, tlen);

	struct thread td_r{};
	struct pfs_node pn_r{};
	struct proc pr_r{};
	struct sbuf sb_r{};
	P::thread td_p2{};
	P::pfs_node pn_p2{};
	P::proc pr_p2{};
	P::sbuf sb_p2{};
	struct uio uio_r{};
	P::uio uio_p2{};

	init_proc(&pr_r, 99);
	init_proc_t(&pr_p2, 99);
	init_sbuf_data(&sb_r, (char *)(arena_r + PAD), DATA_CAP, text);
	init_sbuf_data_t(&sb_p2, (char *)(arena_p + PAD), DATA_CAP, text);
	uio_r.uio_rw = UIO_WRITE;
	uio_p2.uio_rw = P::UIO_WRITE;

	int ret_r = ref_procfs_doosrel(&td_r, &pr_r, &pn_r, &sb_r, &uio_r);
	int ret_p = P::procfs_doosrel(&td_p2, &pr_p2, &pn_p2, &sb_p2, &uio_p2);

	if (ret_r != ret_p) {
		std::printf("    write %s ret %d/%d\n", label, ret_r, ret_p);
		fail(st_osrel, label);
		return false;
	}
	if (pr_r.p_osrel != pr_p2.p_osrel) {
		std::printf("    write %s osrel %d/%d\n", label,
		    pr_r.p_osrel, pr_p2.p_osrel);
		fail(st_osrel, label);
		return false;
	}
	if (!cmp_arenas(st_osrel, arena_r, arena_p))
		return false;
	if (!cmp_sbuf_meta(st_osrel, &sb_r, &sb_p2))
		return false;
	return true;
}

static void
test_osrel_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		if (rng.coin()) {
			run_osrel_read((int)(rng.next() & 0x7fffffffu));
		} else {
			char buf[32];
			unsigned n = rng.below(11);
			size_t pos = 0;
			for (unsigned j = 0; j < n && pos + 1 < sizeof(buf); j++) {
				if (rng.coin() && j > 0)
					buf[pos++] = (char)(' ' + rng.below(5));
				else if (rng.coin())
					buf[pos++] = (char)(0x80 + rng.below(0x40));
				else
					buf[pos++] = (char)('0' + rng.below(10));
			}
			buf[pos] = '\0';
			run_osrel_write_diff("sweep", buf);
		}
	}
}

static bool
run_mem_case(ssize_t resid_in, int candebug_ret, int rwmem_ret,
    ssize_t rwmem_resid, int expect_ret, ssize_t expect_resid,
    int expect_rwmem_calls)
{
	st_mem.cases++;

	struct thread td_r{};
	struct pfs_node pn_r{};
	struct proc pr_r{};
	P::thread td_p2{};
	P::pfs_node pn_p2{};
	P::proc pr_p2{};
	struct uio uio_r{};
	P::uio uio_p2{};
	struct sbuf sb_r{};
	P::sbuf sb_p2{};

	init_proc(&pr_r, 0);
	init_proc_t(&pr_p2, 0);
	uio_r.uio_resid = resid_in;
	uio_p2.uio_resid = resid_in;

	stub_reset_both();
	ref_p_candebug_ret = candebug_ret;
	ref_proc_rwmem_ret = rwmem_ret;
	ref_proc_rwmem_resid = rwmem_resid;
	P::stub_p_candebug(candebug_ret);
	P::stub_proc_rwmem(rwmem_ret, rwmem_resid);

	int ret_r = ref_procfs_doprocmem(&td_r, &pr_r, &pn_r, &sb_r, &uio_r);
	int ret_p = P::procfs_doprocmem(&td_p2, &pr_p2, &pn_p2, &sb_p2, &uio_p2);

	if (ret_r != ret_p || ret_r != expect_ret) {
		std::printf("    mem ret %d/%d expect %d resid_in %zd\n", ret_r,
		    ret_p, expect_ret, (ssize_t)resid_in);
		fail(st_mem, "ret");
		return false;
	}
	if (uio_r.uio_resid != uio_p2.uio_resid ||
	    uio_r.uio_resid != expect_resid) {
		std::printf("    mem resid %zd/%zd expect %zd\n",
		    uio_r.uio_resid, uio_p2.uio_resid, expect_resid);
		fail(st_mem, "resid");
		return false;
	}
	if (ref_proc_rwmem_calls != P::proc_rwmem_calls() ||
	    ref_proc_rwmem_calls != expect_rwmem_calls) {
		std::printf("    mem rwmem_calls %d/%d expect %d\n",
		    ref_proc_rwmem_calls, P::proc_rwmem_calls(),
		    expect_rwmem_calls);
		fail(st_mem, "rwmem_calls");
		return false;
	}
	return true;
}

static void
test_mem_hand()
{
	run_mem_case(0, 0, 0, -1, 0, 0, 0);
	run_mem_case(1, 0, 0, 0, 0, 0, 1);
	run_mem_case(100, 0, 0, 50, 0, 50, 1);
	run_mem_case(10, 1, 0, -1, 1, 10, 0);
	run_mem_case(10, -1, 0, -1, -1, 10, 0);
	run_mem_case(10, 0, EINVAL, -1, EINVAL, 10, 1);
	run_mem_case(10, 0, EOPNOTSUPP, -1, EOPNOTSUPP, 10, 1);
	run_mem_case(1, 0, 0, -1, 0, 1, 1);
}

static void
test_mem_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		ssize_t resid = (ssize_t)(rng.below(256));
		int cdbg = 0;
		int rwret = 0;
		ssize_t newresid = -1;
		int expect_ret = 0;
		int expect_calls = 0;
		ssize_t expect_resid = resid;

		if (resid == 0) {
			expect_calls = 0;
		} else {
			unsigned pick = rng.below(4);
			if (pick == 0) {
				cdbg = (int)(rng.next() & 0xff);
				if (cdbg == 0)
					cdbg = 1;
				expect_ret = cdbg;
				expect_calls = 0;
			} else if (pick == 1) {
				rwret = EINVAL;
				expect_ret = EINVAL;
				expect_calls = 1;
			} else {
				newresid = (ssize_t)rng.below((unsigned)resid + 1);
				expect_calls = 1;
				expect_resid = newresid;
			}
		}
		run_mem_case(resid, cdbg, rwret, newresid, expect_ret,
		    expect_resid, expect_calls);
	}
}

int
main()
{
	stub_reset_both();

	test_note_hand();
	test_note_sweep();
	test_osrel_hand();
	test_osrel_sweep();
	test_mem_hand();
	test_mem_sweep();

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-22s %12llu %12llu\n", st_note.name, st_note.cases,
	    st_note.fails);
	std::printf("%-22s %12llu %12llu\n", st_osrel.name, st_osrel.cases,
	    st_osrel.fails);
	std::printf("%-22s %12llu %12llu\n", st_mem.name, st_mem.cases,
	    st_mem.fails);

	unsigned long long total_fails =
	    st_note.fails + st_osrel.fails + st_mem.fails;

	return (total_fails == 0) ? 0 : 1;
}
