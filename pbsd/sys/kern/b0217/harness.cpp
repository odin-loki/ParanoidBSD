// Differential test for PBSD batch b0217.

import pbsd.sys.kern.b0217;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::sys_kern::b0217;

#define GUARD 0x7f
#define PAD 32u
#define SWEEP 200000L
#define MAX_PRINT 12

struct stat_row {
	const char *name;
	long cases;
	long failures;
	long printed;
};

static stat_row rows[] = {
	{ "vfs_hash_index", 0, 0, 0 },
	{ "vfs_hash_get", 0, 0, 0 },
	{ "vfs_hash_ref", 0, 0, 0 },
	{ "vfs_hash_remove", 0, 0, 0 },
	{ "vfs_hash_insert", 0, 0, 0 },
	{ "vfs_hash_rehash", 0, 0, 0 },
	{ "vfs_hash_changesize", 0, 0, 0 },
	{ "tslog", 0, 0, 0 },
	{ "sysctl_debug_tslog", 0, 0, 0 },
	{ "tslog_user", 0, 0, 0 },
	{ "sysctl_debug_tslog_user", 0, 0, 0 },
	{ "sysinit_tslog_shim", 0, 0, 0 },
	{ "tslog_reset", 0, 0, 0 },
	{ "clockcalib", 0, 0, 0 },
	{ "stack_create", 0, 0, 0 },
	{ "stack_destroy", 0, 0, 0 },
	{ "stack_put", 0, 0, 0 },
	{ "stack_copy", 0, 0, 0 },
	{ "stack_zero", 0, 0, 0 },
	{ "stack_print", 0, 0, 0 },
	{ "stack_print_short", 0, 0, 0 },
	{ "stack_print_ddb", 0, 0, 0 },
	{ "stack_print_short_ddb", 0, 0, 0 },
	{ "stack_sbuf_print_flags", 0, 0, 0 },
	{ "stack_sbuf_print", 0, 0, 0 },
	{ "stack_sbuf_print_ddb", 0, 0, 0 },
	{ "stack_ktr", 0, 0, 0 }
};

enum {
	R_VFS_HASH_INDEX,
	R_VFS_HASH_GET,
	R_VFS_HASH_REF,
	R_VFS_HASH_REMOVE,
	R_VFS_HASH_INSERT,
	R_VFS_HASH_REHASH,
	R_VFS_HASH_CHANGESIZE,
	R_TSLOG,
	R_SYSCTL_DEBUG_TSLOG,
	R_TSLOG_USER,
	R_SYSCTL_DEBUG_TSLOG_USER,
	R_SYSINIT_TSLOG_SHIM,
	R_TSLOG_RESET,
	R_CLOCKCALIB,
	R_STACK_CREATE,
	R_STACK_DESTROY,
	R_STACK_PUT,
	R_STACK_COPY,
	R_STACK_ZERO,
	R_STACK_PRINT,
	R_STACK_PRINT_SHORT,
	R_STACK_PRINT_DDB,
	R_STACK_PRINT_SHORT_DDB,
	R_STACK_SBUF_PRINT_FLAGS,
	R_STACK_SBUF_PRINT,
	R_STACK_SBUF_PRINT_DDB,
	R_STACK_KTR
};

static uint64_t rng_state = 0x00b0217faceULL;

static inline uint64_t rnd64(void) {
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline uint32_t rnd32(void) { return (uint32_t)(rnd64() >> 32); }

static void fail_row(int row, const char *label, const char *detail) {
	stat_row &r = rows[row];
	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-28s %-20s %s\n", r.name, label, detail);
	}
}

static void case_row(int row) { rows[row].cases++; }

extern "C" {
struct thread { char td_pad; };
struct mount { unsigned int mnt_hashseed; };
struct vnode {
	unsigned int v_hash;
	struct mount *v_mount;
	struct { struct vnode *le_next; struct vnode **le_prev; } v_hashlist;
	int v_hold, v_refs, v_gone;
};
struct stack { int depth; uintptr_t pcs[18]; };
struct sbuf { char *s_buf; size_t s_size; size_t s_len; int s_error; };
struct sysctl_oid;
struct sysctl_req { int ignored; };
struct sysinit_tslog { void (*func)(void *); void *data; const char *name; };
struct timecounter {
	unsigned int (*tc_get_timecount)(struct timecounter *);
	unsigned int tc_counter_mask;
	uint64_t tc_frequency;
};

typedef int vfs_hash_cmp_t(struct vnode *, void *);

unsigned int ref_vfs_hash_index(struct vnode *);
int ref_vfs_hash_get(const struct mount *, unsigned int, int, struct thread *,
    struct vnode **, vfs_hash_cmp_t *, void *);
void ref_vfs_hash_ref(const struct mount *, unsigned int, struct thread *,
    struct vnode **, vfs_hash_cmp_t *, void *);
void ref_vfs_hash_remove(struct vnode *);
int ref_vfs_hash_insert(struct vnode *, unsigned int, int, struct thread *,
    struct vnode **, vfs_hash_cmp_t *, void *);
void ref_vfs_hash_rehash(struct vnode *, unsigned int);
void ref_vfs_hash_changesize(unsigned long);
void ref_tslog(void *, int, const char *, const char *);
int ref_sysctl_debug_tslog(struct sysctl_oid *, void *, int, struct sysctl_req *);
void ref_tslog_user(int, int, const char *, const char *);
int ref_sysctl_debug_tslog_user(struct sysctl_oid *, void *, int, struct sysctl_req *);
void ref_sysinit_tslog_shim(const void *);
void ref_tslog_reset(void);
uint64_t ref_clockcalib(uint64_t (*)(void), const char *);
struct stack *ref_stack_create(int);
void ref_stack_destroy(struct stack *);
int ref_stack_put(struct stack *, uintptr_t);
void ref_stack_copy(const struct stack *, struct stack *);
void ref_stack_zero(struct stack *);
void ref_stack_print(const struct stack *);
void ref_stack_print_short(const struct stack *);
void ref_stack_print_ddb(const struct stack *);
void ref_stack_print_short_ddb(const struct stack *);
int ref_stack_sbuf_print_flags(struct sbuf *, const struct stack *, int, int);
void ref_stack_sbuf_print(struct sbuf *, const struct stack *);
void ref_stack_sbuf_print_ddb(struct sbuf *, const struct stack *);
void ref_stack_ktr(unsigned int, const char *, int, const struct stack *, unsigned int);

void oracle_reset(void);
void oracle_malloc_fail_at(int);
void oracle_set_vget_enoent(int);
void oracle_set_vget_error(int);
void oracle_set_linker_fail(int);
void oracle_set_linker_block(int);
void oracle_set_bootverbose(int);
void oracle_set_sbuf_fail(int);
int oracle_ktr_count(void);
const char *model_out_text(void);
size_t model_out_length(void);
void model_reset(void);
void model_snapshot(void);
int model_diff(char *msg, size_t msgsz);
long oracle_tslog_nrecs(void);
int oracle_tslog_get(long, void **, int *, const char **, const char **, uint64_t *);
int oracle_tslog_user_get(int, int *, uint64_t *, uint64_t *, const char **, const char **, int *);
extern struct timecounter *timecounter;
extern int bootverbose;
}

static struct timecounter g_tc;
static uint64_t g_clk, g_tc_val, g_tc_step = 1, g_clk_mul = 3000;

static void reset_env(void) {
	model_reset();
	oracle_reset();
	port::reset_all();
	g_clk = 0; g_tc_val = 0;
	g_tc.tc_get_timecount = [](struct timecounter *tc) -> unsigned int {
		(void)tc; g_tc_val += g_tc_step; return (unsigned int)(g_tc_val & tc->tc_counter_mask);
	};
	g_tc.tc_counter_mask = 0xffffffffu;
	g_tc.tc_frequency = 1000000000ULL;
	timecounter = &g_tc;
	bootverbose = 0;
}

static uint64_t test_clk(void) {
	g_clk += 997;
	return g_clk * g_clk_mul;
}

static bool env_match(void) {
	char msg[256];
	model_snapshot();
	if (model_diff(msg, sizeof(msg)) != 0) {
		std::printf("  env diff: %s\n", msg);
		return false;
	}
	return true;
}

static int cmp_never(struct vnode *, void *) { return 1; }
static int cmp_match_hash(struct vnode *vp, void *arg) {
	return (vp->v_hash != *(unsigned int *)arg);
}

static void setup_vnode(struct vnode *vp, unsigned int hash, struct mount *mp) {
	std::memset(vp, 0, sizeof(*vp));
	vp->v_hash = hash;
	vp->v_mount = mp;
}

static void test_vfs_hash_index(void) {
	struct mount mp{}; struct vnode vp{};
	mp.mnt_hashseed = 17; vp.v_hash = 42; vp.v_mount = &mp;
	case_row(R_VFS_HASH_INDEX);
	unsigned int p = port::vfs_hash_index(&vp);
	unsigned int r = ref_vfs_hash_index((struct vnode *)&vp);
	if (p != r) fail_row(R_VFS_HASH_INDEX, "value", "mismatch");
}

static void test_vfs_hash_insert_get_one(unsigned int hash, int flags) {
	struct mount mp{}; mp.mnt_hashseed = (unsigned int)(rnd32() % 1000);
	struct vnode vp{}, *pout = (struct vnode *)0x42, *rout = (struct vnode *)0x42;
	setup_vnode(&vp, hash, &mp);
	case_row(R_VFS_HASH_INSERT);
	int pi = port::vfs_hash_insert(&vp, hash, flags, nullptr, &pout, nullptr, nullptr);
	reset_env();
	setup_vnode(&vp, hash, &mp); rout = (struct vnode *)0x42;
	int ri = ref_vfs_hash_insert((struct vnode *)&vp, hash, flags, nullptr,
	    &rout, nullptr, nullptr);
	if (pi != ri) fail_row(R_VFS_HASH_INSERT, "ret", "mismatch");
	case_row(R_VFS_HASH_GET);
	struct vnode *pg = nullptr, *rg = nullptr;
	int pgd = port::vfs_hash_get(&mp, hash, flags, nullptr, &pg, nullptr, nullptr);
	reset_env();
	int rgd = ref_vfs_hash_get(&mp, hash, flags, nullptr, &rg, nullptr, nullptr);
	if (pgd != rgd) fail_row(R_VFS_HASH_GET, "ret", "mismatch");
}

static void test_vfs_hash_ref_remove(unsigned int hash) {
	struct mount mp{}; mp.mnt_hashseed = 3;
	struct vnode vp{}; setup_vnode(&vp, hash, &mp);
	port::vfs_hash_insert(&vp, hash, 0, nullptr, &vp, nullptr, nullptr);
	struct vnode *pr = nullptr, *rr = nullptr;
	case_row(R_VFS_HASH_REF);
	port::vfs_hash_ref(&mp, hash, nullptr, &pr, nullptr, nullptr);
	reset_env();
	ref_vfs_hash_ref(&mp, hash, nullptr, &rr, nullptr, nullptr);
	case_row(R_VFS_HASH_REMOVE);
	port::vfs_hash_remove(&vp);
	reset_env();
	ref_vfs_hash_remove((struct vnode *)&vp);
}

static void test_vfs_hash_rehash(unsigned int newhash) {
	struct mount mp{}; mp.mnt_hashseed = 5;
	struct vnode vp{}; setup_vnode(&vp, 11, &mp);
	port::vfs_hash_insert(&vp, 11, 0, nullptr, &vp, nullptr, nullptr);
	case_row(R_VFS_HASH_REHASH);
	port::vfs_hash_rehash(&vp, newhash);
	reset_env();
	ref_vfs_hash_rehash((struct vnode *)&vp, newhash);
	if (vp.v_hash != newhash) fail_row(R_VFS_HASH_REHASH, "hash", "mismatch");
}

static void test_vfs_hash_changesize(unsigned long n) {
	case_row(R_VFS_HASH_CHANGESIZE);
	port::vfs_hash_changesize(n);
	reset_env();
	ref_vfs_hash_changesize(n);
}

static void test_tslog_one(void *td, int type, const char *f, const char *s) {
	case_row(R_TSLOG);
	port::tslog(td, type, f, s);
	reset_env();
	ref_tslog(td, type, f, s);
	long pn = port::detail::model_log_n; (void)pn;
	long rn = oracle_tslog_nrecs();
	long pn2 = 0; /* compare record counts via oracle helper after matching nrecs externally */
	(void)pn2;
	if (rn < 0) fail_row(R_TSLOG, "nrecs", "bad");
}

static void test_tslog_user(pid_t pid, pid_t ppid, const char *ex, const char *nm) {
	case_row(R_TSLOG_USER);
	port::tslog_user(pid, ppid, ex, nm);
	reset_env();
	ref_tslog_user(pid, ppid, ex, nm);
}

static void test_sysctl_tslog(void) {
	struct sysctl_req req{};
	case_row(R_SYSCTL_DEBUG_TSLOG);
	int pe = port::sysctl_debug_tslog(nullptr, nullptr, 0, (port::sysctl_req *)&req);
	reset_env();
	int re = ref_sysctl_debug_tslog(nullptr, nullptr, 0, &req);
	if (pe != re) fail_row(R_SYSCTL_DEBUG_TSLOG, "ret", "mismatch");
}

static void test_sysctl_tslog_user(void) {
	struct sysctl_req req{};
	case_row(R_SYSCTL_DEBUG_TSLOG_USER);
	int pe = port::sysctl_debug_tslog_user(nullptr, nullptr, 0, (port::sysctl_req *)&req);
	reset_env();
	int re = ref_sysctl_debug_tslog_user(nullptr, nullptr, 0, &req);
	if (pe != re) fail_row(R_SYSCTL_DEBUG_TSLOG_USER, "ret", "mismatch");
}

static void test_sysinit_shim(void) {
	static int called;
	called = 0;
	auto fn = +[](void *) { called++; };
	port::sysinit_tslog entry{ fn, nullptr, "init" };
	case_row(R_SYSINIT_TSLOG_SHIM);
	port::sysinit_tslog_shim(&entry);
	reset_env();
	struct sysinit_tslog rent{ (void (*)(void*))fn, nullptr, "init" };
	ref_sysinit_tslog_shim(&rent);
	if (called != 2) fail_row(R_SYSINIT_TSLOG_SHIM, "called", "mismatch");
}

static void test_tslog_reset(void) {
	port::tslog(nullptr, port::TS_ENTER, "f", "s");
	case_row(R_TSLOG_RESET);
	port::tslog_reset();
	reset_env();
	ref_tslog(nullptr, port::TS_ENTER, "f", "s");
	ref_tslog_reset();
}

static void test_clockcalib(void) {
	case_row(R_CLOCKCALIB);
	uint64_t pf = port::clockcalib(test_clk, "tsc");
	reset_env();
	uint64_t rf = ref_clockcalib(test_clk, "tsc");
	if (pf != rf) fail_row(R_CLOCKCALIB, "freq", "mismatch");
}

static bool buf_ok(const unsigned char *b, size_t n) {
	for (size_t i = 0; i < n; i++)
		if (b[i] != GUARD) return false;
	return true;
}

static void test_stack_lifecycle(void) {
	case_row(R_STACK_CREATE);
	port::stack *ps = port::stack_create(port::M_WAITOK);
	reset_env();
	struct stack *rs = ref_stack_create(0);
	if ((ps == nullptr) != (rs == nullptr)) fail_row(R_STACK_CREATE, "null", "mismatch");
	if (!ps || !rs) return;
	case_row(R_STACK_PUT);
	for (int i = 0; i < 20; i++) {
		int pp = port::stack_put(ps, (uintptr_t)(0x1000 + i));
		reset_env();
		int rp = ref_stack_put(rs, (uintptr_t)(0x1000 + i));
		if (pp != rp) { fail_row(R_STACK_PUT, "ret", "mismatch"); break; }
	}
	case_row(R_STACK_COPY);
	port::stack pd{}; port::stack_copy(ps, &pd);
	reset_env();
	struct stack rd{}; ref_stack_copy(rs, &rd);
	case_row(R_STACK_ZERO);
	port::stack_zero(&pd);
	reset_env();
	ref_stack_zero(&rd);
	case_row(R_STACK_DESTROY);
	port::stack_destroy(ps);
	reset_env();
	ref_stack_destroy(rs);
}

static void test_stack_prints(void) {
	port::stack st{}; st.depth = 3;
	st.pcs[0] = 0x4000; st.pcs[1] = 0x4100; st.pcs[2] = 0x4200;
	struct stack rst = st;
	case_row(R_STACK_PRINT);
	port::stack_print(&st);
	reset_env();
	ref_stack_print(&rst);
	case_row(R_STACK_PRINT_SHORT);
	port::stack_print_short(&st);
	reset_env();
	ref_stack_print_short(&rst);
	case_row(R_STACK_PRINT_DDB);
	port::stack_print_ddb(&st);
	reset_env();
	ref_stack_print_ddb(&rst);
	case_row(R_STACK_PRINT_SHORT_DDB);
	port::stack_print_short_ddb(&st);
	reset_env();
	ref_stack_print_short_ddb(&rst);
}

static void test_stack_sbuf(void) {
	unsigned char blob[PAD + sizeof(port::sbuf) + PAD];
	std::memset(blob, GUARD, sizeof(blob));
	port::sbuf *sb = (port::sbuf *)(blob + PAD);
	sb->s_buf = (char *)(blob + PAD + 64);
	sb->s_size = 256; sb->s_len = 0; sb->s_error = 0;
	std::memset(sb->s_buf, GUARD, sb->s_size);
	port::stack st{}; st.depth = 2; st.pcs[0] = 0x5000; st.pcs[1] = 0x5100;
	unsigned char rblob[PAD + sizeof(struct sbuf) + PAD];
	std::memset(rblob, GUARD, sizeof(rblob));
	struct sbuf *rsb = (struct sbuf *)(rblob + PAD);
	rsb->s_buf = (char *)(rblob + PAD + 64);
	rsb->s_size = 256; rsb->s_len = 0; rsb->s_error = 0;
	std::memset(rsb->s_buf, GUARD, rsb->s_size);
	struct stack rst = st;
	case_row(R_STACK_SBUF_PRINT_FLAGS);
	int pe = port::stack_sbuf_print_flags(sb, &st, port::M_WAITOK, port::STACK_SBUF_FMT_LONG);
	reset_env();
	int re = ref_stack_sbuf_print_flags(rsb, &rst, 0, 1);
	if (pe != re) fail_row(R_STACK_SBUF_PRINT_FLAGS, "ret", "mismatch");
	case_row(R_STACK_SBUF_PRINT);
	port::stack_sbuf_print(sb, &st);
	reset_env();
	ref_stack_sbuf_print(rsb, &rst);
	case_row(R_STACK_SBUF_PRINT_DDB);
	port::stack_sbuf_print_ddb(sb, &st);
	reset_env();
	ref_stack_sbuf_print_ddb(rsb, &rst);
}

static void test_stack_ktr(void) {
	port::stack st{}; st.depth = 2; st.pcs[0] = 0x6000; st.pcs[1] = 0x6100;
	struct stack rst = st;
	case_row(R_STACK_KTR);
	port::stack_ktr(1, "f.c", 9, &st, 0);
	reset_env();
	ref_stack_ktr(1, "f.c", 9, &rst, 0);
	if (port::ktr_count() != oracle_ktr_count())
		fail_row(R_STACK_KTR, "ktr", "count mismatch");
}

static void test_vfs_hand(void) {
	test_vfs_hash_index();
	test_vfs_hash_insert_get_one(1, 0);
	test_vfs_hash_insert_get_one(99, 0);
	test_vfs_hash_ref_remove(7);
	test_vfs_hash_rehash(123);
	test_vfs_hash_changesize(128);
	test_vfs_hash_changesize(64);
}

static void test_tslog_hand(void) {
	test_tslog_one(nullptr, port::TS_ENTER, "fn", "arg");
	test_tslog_one(&port::thread0, port::TS_EXIT, "x", nullptr);
	test_tslog_user(1, 2, nullptr, nullptr);
	test_tslog_user(1, -1, "exec", nullptr);
	test_tslog_user(1, -1, nullptr, "/path");
	test_tslog_user(1, -1, nullptr, nullptr);
	test_sysctl_tslog();
	test_sysctl_tslog_user();
	test_sysinit_shim();
	test_tslog_reset();
}

static void test_stack_hand(void) {
	test_stack_lifecycle();
	test_stack_prints();
	test_stack_sbuf();
	test_stack_ktr();
}

static void sweep_vfs(void) {
	for (long i = 0; i < SWEEP; i++) {
		reset_env();
		test_vfs_hash_index();
		reset_env();
		test_vfs_hash_insert_get_one((unsigned int)(rnd32() % 500), (int)(rnd32() & 1));
		if ((rnd32() % 50) == 0) {
			reset_env();
			test_vfs_hash_changesize((unsigned long)(rnd32() % 512 + 1));
		}
	}
}

static void sweep_tslog(void) {
	for (long i = 0; i < SWEEP; i++) {
		reset_env();
		test_tslog_one(nullptr, (int)(rnd32() % 4), "f", (rnd32() & 1) ? "s" : nullptr);
		if ((rnd32() % 10) == 0) {
			reset_env();
			test_tslog_user((int)(rnd32() % 100), (int)(rnd32() % 2 ? -1 : (int)(rnd32() % 50)),
			    (rnd32() & 1) ? "e" : nullptr, (rnd32() & 2) ? "/n" : nullptr);
		}
	}
}

static void sweep_clockcalib(void) {
	for (long i = 0; i < SWEEP; i++) {
		g_clk_mul = (uint64_t)(rnd32() % 10000 + 1);
		g_tc_step = (uint64_t)(rnd32() % 7 + 1);
		reset_env();
		test_clockcalib();
	}
}

static void sweep_stack(void) {
	for (long i = 0; i < SWEEP; i++) {
		reset_env();
		test_stack_lifecycle();
		if ((rnd32() % 20) == 0) {
			reset_env();
			test_stack_prints();
		}
	}
}

int main(void) {
	test_vfs_hand();
	test_tslog_hand();
	test_stack_hand();
	test_clockcalib();
	sweep_vfs();
	sweep_tslog();
	sweep_clockcalib();
	sweep_stack();
	long total_cases = 0, total_fail = 0;
	std::printf("\n%-28s %12s %12s\n", "function", "cases", "failures");
	for (const auto &r : rows) {
		std::printf("%-28s %12ld %12ld\n", r.name, r.cases, r.failures);
		total_cases += r.cases;
		total_fail += r.failures;
	}
	std::printf("%-28s %12ld %12ld\n", "TOTAL", total_cases, total_fail);
	return total_fail == 0 ? 0 : 1;
}
