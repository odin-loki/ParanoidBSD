// Differential test for PBSD batch b0217.

import pbsd.sys.kern.b0217;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

namespace port = pbsd::sys_kern::b0217;

#define GUARD 0x7f
#define PAD 32u
#define SWEEP 30000L
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
	{ "stack_ktr", 0, 0, 0 },
	{ "stack_symbol", 0, 0, 0 },
	{ "stack_symbol_ddb", 0, 0, 0 }
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
	R_STACK_KTR,
	R_STACK_SYMBOL,
	R_STACK_SYMBOL_DDB
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

extern "C" int __wrap_printf(const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = std::vprintf(fmt, ap);
	va_end(ap);
	return (n);
}

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
int ref_stack_symbol(uintptr_t, char *, unsigned int, long *, int);
int ref_stack_symbol_ddb(uintptr_t, const char **, long *);

void oracle_reset(void);
void oracle_malloc_fail_at(int);
void oracle_set_vget_enoent(int);
void oracle_set_vget_enoent_once(int);
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
static int g_vget_enoent, g_vget_error, g_linker_fail, g_linker_block, g_sbuf_fail;

static void setup_tc(void) {
	g_clk = 0; g_tc_val = 0;
	g_tc.tc_get_timecount = [](struct timecounter *tc) -> unsigned int {
		(void)tc; g_tc_val += g_tc_step;
		return (unsigned int)(g_tc_val & tc->tc_counter_mask);
	};
	g_tc.tc_counter_mask = 0xffffffffu;
	g_tc.tc_frequency = 1000000000ULL;
	timecounter = &g_tc;
	port::set_timecounter(reinterpret_cast<port::timecounter *>(&g_tc));
	bootverbose = 0;
}

static void apply_test_flags(void)
{
	port::set_vget_enoent(g_vget_enoent);
	port::set_vget_error(g_vget_error);
	port::set_linker_fail(g_linker_fail);
	port::set_linker_block(g_linker_block);
	port::set_sbuf_fail(g_sbuf_fail);
	port::set_bootverbose(bootverbose);
	oracle_set_vget_enoent(g_vget_enoent);
	oracle_set_vget_enoent_once(0);
	oracle_set_vget_error(g_vget_error);
	oracle_set_linker_fail(g_linker_fail);
	oracle_set_linker_block(g_linker_block);
	oracle_set_sbuf_fail(g_sbuf_fail);
	oracle_set_bootverbose(bootverbose);
}

static void reset_port(void)
{
	model_reset();
	port::reset_all();
	setup_tc();
	apply_test_flags();
}

static void reset_ref(void)
{
	model_reset();
	oracle_reset();
	setup_tc();
	apply_test_flags();
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

static void compare_tslog_records(int row)
{
	long n = port::tslog_record_count();
	if (n != oracle_tslog_nrecs()) {
		fail_row(row, "nrecs", "mismatch");
		return;
	}
	for (long i = 0; i < n; i++) {
		void *ptd, *rtd;
		int ptype, rtype;
		const char *pf, *rf, *ps, *rs;
		uint64_t ptsc, rtsc;
		if (!port::tslog_get_record(i, &ptd, &ptype, &pf, &ps, &ptsc) ||
		    !oracle_tslog_get(i, &rtd, &rtype, &rf, &rs, &rtsc)) {
			fail_row(row, "get", "bounds");
			return;
		}
		if (ptype != rtype || pf != rf || ps != rs || ptsc != rtsc)
			fail_row(row, "record", "field mismatch");
	}
}

static void compare_tslog_user_pid(int row, int pid)
{
	pid_t pppid, rppid;
	uint64_t ptf, rtf, pte, rte;
	const char *pex, *rex, *pnm, *rnm;
	int pre, rre;
	if (!port::tslog_user_get_record(pid, &pppid, &ptf, &pte, &pex, &pnm, &pre) ||
	    !oracle_tslog_user_get(pid, &rppid, &rtf, &rte, &rex, &rnm, &rre)) {
		fail_row(row, "user_get", "bounds");
		return;
	}
	if (pppid != rppid || ptf != rtf || pte != rte || pre != rre)
		fail_row(row, "user", "scalar mismatch");
	if ((pex == nullptr) != (rex == nullptr) ||
	    (pex != nullptr && std::strcmp(pex, rex) != 0))
		fail_row(row, "execname", "mismatch");
	if ((pnm == nullptr) != (rnm == nullptr) ||
	    (pnm != nullptr && std::strcmp(pnm, rnm) != 0))
		fail_row(row, "namei", "mismatch");
}

static void setup_vnode(struct vnode *vp, unsigned int hash, struct mount *mp) {
	std::memset(vp, 0, sizeof(*vp));
	vp->v_hash = hash;
	vp->v_mount = mp;
}

static void test_vfs_hash_index(void) {
	port::mount mp{}; port::vnode vp{};
	mp.mnt_hashseed = 17; vp.v_hash = 42; vp.v_mount = &mp;
	case_row(R_VFS_HASH_INDEX);
	reset_port();
	unsigned int p = port::vfs_hash_index(&vp);
	reset_ref();
	unsigned int r = ref_vfs_hash_index(reinterpret_cast<struct vnode *>(&vp));
	if (p != r) fail_row(R_VFS_HASH_INDEX, "value", "mismatch");
}

static void test_vfs_hash_insert_get_one(unsigned int hash, int flags) {
	port::mount mp{}; mp.mnt_hashseed = (unsigned int)(rnd32() % 1000);
	port::vnode vp{}; port::vnode *pout = (port::vnode *)0x42;
	setup_vnode(reinterpret_cast<struct vnode *>(&vp), hash,
	    reinterpret_cast<struct mount *>(&mp));
	case_row(R_VFS_HASH_INSERT);
	reset_port();
	int pi = port::vfs_hash_insert(&vp, hash, flags, nullptr, &pout, nullptr, nullptr);
	reset_ref();
	struct vnode *rout = (struct vnode *)0x42;
	int ri = ref_vfs_hash_insert(reinterpret_cast<struct vnode *>(&vp), hash, flags,
	    nullptr, &rout, nullptr, nullptr);
	if (pi != ri) fail_row(R_VFS_HASH_INSERT, "ret", "mismatch");
	case_row(R_VFS_HASH_GET);
	int get_flags = flags;
	if (g_vget_enoent)
		get_flags |= 0x0001; /* LK_NOWAIT: ENOENT must not retry forever */
	reset_port();
	port::vnode *pg = nullptr;
	int pgd = port::vfs_hash_get(&mp, hash, get_flags, nullptr, &pg, nullptr, nullptr);
	reset_ref();
	struct vnode *rg = nullptr;
	int rgd = ref_vfs_hash_get(reinterpret_cast<struct mount *>(&mp), hash, get_flags,
	    nullptr, &rg, nullptr, nullptr);
	if (pgd != rgd) fail_row(R_VFS_HASH_GET, "ret", "mismatch");
}

static void test_vfs_hash_ref_remove(unsigned int hash) {
	port::mount mp{}; mp.mnt_hashseed = 3;
	port::vnode vp{};
	setup_vnode(reinterpret_cast<struct vnode *>(&vp), hash,
	    reinterpret_cast<struct mount *>(&mp));
	port::vnode *ins = &vp;
	reset_port();
	port::vfs_hash_insert(&vp, hash, 0, nullptr, &ins, nullptr, nullptr);
	port::vnode *pr = nullptr;
	case_row(R_VFS_HASH_REF);
	port::vfs_hash_ref(&mp, hash, nullptr, &pr, nullptr, nullptr);
	case_row(R_VFS_HASH_REMOVE);
	port::vfs_hash_remove(&vp);
	reset_ref();
	struct vnode *rr = nullptr;
	struct vnode *rins = reinterpret_cast<struct vnode *>(&vp);
	ref_vfs_hash_insert(reinterpret_cast<struct vnode *>(&vp), hash, 0, nullptr,
	    &rins, nullptr, nullptr);
	ref_vfs_hash_ref(reinterpret_cast<struct mount *>(&mp), hash, nullptr, &rr,
	    nullptr, nullptr);
	ref_vfs_hash_remove(reinterpret_cast<struct vnode *>(&vp));
}

static void test_vfs_hash_rehash(unsigned int newhash) {
	port::mount mp{}; mp.mnt_hashseed = 5;
	port::vnode vp{};
	setup_vnode(reinterpret_cast<struct vnode *>(&vp), 11,
	    reinterpret_cast<struct mount *>(&mp));
	port::vnode *ins = &vp;
	reset_port();
	port::vfs_hash_insert(&vp, 11, 0, nullptr, &ins, nullptr, nullptr);
	case_row(R_VFS_HASH_REHASH);
	port::vfs_hash_rehash(&vp, newhash);
	reset_ref();
	struct vnode *rins = reinterpret_cast<struct vnode *>(&vp);
	ref_vfs_hash_insert(reinterpret_cast<struct vnode *>(&vp), 11, 0, nullptr,
	    &rins, nullptr, nullptr);
	ref_vfs_hash_rehash(reinterpret_cast<struct vnode *>(&vp), newhash);
	if (vp.v_hash != newhash) fail_row(R_VFS_HASH_REHASH, "hash", "mismatch");
}

static void test_vfs_hash_changesize(unsigned long n) {
	case_row(R_VFS_HASH_CHANGESIZE);
	reset_port();
	port::vfs_hash_changesize(n);
	reset_ref();
	ref_vfs_hash_changesize(n);
}

static void test_tslog_one(void *td, int type, const char *f, const char *s) {
	case_row(R_TSLOG);
	reset_port();
	port::tslog(td, type, f, s);
	long pn = port::tslog_record_count();
	reset_ref();
	ref_tslog(td, type, f, s);
	long rn = oracle_tslog_nrecs();
	if (pn != rn) fail_row(R_TSLOG, "nrecs", "mismatch");
	compare_tslog_records(R_TSLOG);
}

static void test_tslog_user(pid_t pid, pid_t ppid, const char *ex, const char *nm) {
	case_row(R_TSLOG_USER);
	reset_port();
	port::tslog_user(pid, ppid, ex, nm);
	reset_ref();
	ref_tslog_user(pid, ppid, ex, nm);
	compare_tslog_user_pid(R_TSLOG_USER, pid);
}

static void test_sysctl_tslog(void) {
	port::sysctl_req req{};
	case_row(R_SYSCTL_DEBUG_TSLOG);
	reset_port();
	int pe = port::sysctl_debug_tslog(nullptr, nullptr, 0, &req);
	reset_ref();
	struct sysctl_req rreq{};
	int re = ref_sysctl_debug_tslog(nullptr, nullptr, 0, &rreq);
	if (pe != re) fail_row(R_SYSCTL_DEBUG_TSLOG, "ret", "mismatch");
}

static void test_sysctl_tslog_user(void) {
	port::sysctl_req req{};
	case_row(R_SYSCTL_DEBUG_TSLOG_USER);
	reset_port();
	int pe = port::sysctl_debug_tslog_user(nullptr, nullptr, 0, &req);
	reset_ref();
	struct sysctl_req rreq{};
	int re = ref_sysctl_debug_tslog_user(nullptr, nullptr, 0, &rreq);
	if (pe != re) fail_row(R_SYSCTL_DEBUG_TSLOG_USER, "ret", "mismatch");
}

static void test_sysinit_shim(void) {
	static int called;
	called = 0;
	auto fn = +[](void *) { called++; };
	port::sysinit_tslog entry{ fn, nullptr, "init" };
	case_row(R_SYSINIT_TSLOG_SHIM);
	reset_port();
	port::sysinit_tslog_shim(&entry);
	reset_ref();
	struct sysinit_tslog rent{ (void (*)(void*))fn, nullptr, "init" };
	ref_sysinit_tslog_shim(&rent);
	if (called != 2) fail_row(R_SYSINIT_TSLOG_SHIM, "called", "mismatch");
}

static void test_tslog_reset(void) {
	reset_port();
	port::tslog(nullptr, 0, "f", "s");
	case_row(R_TSLOG_RESET);
	port::tslog_reset();
	reset_ref();
	ref_tslog(nullptr, 0, "f", "s");
	ref_tslog_reset();
}

static void test_clockcalib(void) {
	case_row(R_CLOCKCALIB);
	reset_port();
	uint64_t pf = port::clockcalib(test_clk, "tsc");
	size_t plen = port::out_length();
	const char *pout = port::out_text();
	reset_ref();
	uint64_t rf = ref_clockcalib(test_clk, "tsc");
	size_t rlen = model_out_length();
	const char *rout = model_out_text();
	if (pf != rf) fail_row(R_CLOCKCALIB, "freq", "mismatch");
	if (plen != rlen || (plen > 0 && std::memcmp(pout, rout, plen) != 0))
		fail_row(R_CLOCKCALIB, "printf", "mismatch");
}

static bool buf_ok(const unsigned char *b, size_t n) {
	for (size_t i = 0; i < n; i++)
		if (b[i] != GUARD) return false;
	return true;
}

static void test_stack_lifecycle(void) {
	case_row(R_STACK_CREATE);
	reset_port();
	port::stack *ps = port::stack_create(2);
	reset_ref();
	struct stack *rs = ref_stack_create(2);
	if ((ps == nullptr) != (rs == nullptr)) fail_row(R_STACK_CREATE, "null", "mismatch");
	if (!ps || !rs) return;
	case_row(R_STACK_PUT);
	for (int i = 0; i < 20; i++) {
		int pp = port::stack_put(ps, (uintptr_t)(0x1000 + i));
		int rp = ref_stack_put(rs, (uintptr_t)(0x1000 + i));
		if (pp != rp) { fail_row(R_STACK_PUT, "ret", "mismatch"); break; }
	}
	case_row(R_STACK_COPY);
	reset_port();
	port::stack pd{}; port::stack_copy(ps, &pd);
	reset_ref();
	struct stack rd{}; ref_stack_copy(rs, &rd);
	case_row(R_STACK_ZERO);
	reset_port();
	port::stack_zero(&pd);
	reset_ref();
	ref_stack_zero(&rd);
	case_row(R_STACK_DESTROY);
	reset_port();
	port::stack_destroy(ps);
	reset_ref();
	ref_stack_destroy(rs);
}

static void test_stack_prints(void) {
	port::stack st{}; st.depth = 3;
	st.pcs[0] = 0x4000; st.pcs[1] = 0x4100; st.pcs[2] = 0x4200;
	case_row(R_STACK_PRINT);
	reset_port();
	port::stack_print(&st);
	reset_ref();
	struct stack rst{}; std::memcpy(&rst, &st, sizeof(rst));
	ref_stack_print(&rst);
	case_row(R_STACK_PRINT_SHORT);
	reset_port();
	port::stack_print_short(&st);
	reset_ref();
	ref_stack_print_short(&rst);
	case_row(R_STACK_PRINT_DDB);
	reset_port();
	port::stack_print_ddb(&st);
	reset_ref();
	ref_stack_print_ddb(&rst);
	case_row(R_STACK_PRINT_SHORT_DDB);
	reset_port();
	port::stack_print_short_ddb(&st);
	reset_ref();
	ref_stack_print_short_ddb(&rst);
}

static void test_stack_sbuf(void) {
	unsigned char blob[PAD + sizeof(port::sbuf) + 512 + PAD];
	std::memset(blob, GUARD, sizeof(blob));
	port::sbuf *sb = (port::sbuf *)(blob + PAD);
	sb->s_buf = (char *)(blob + PAD + sizeof(port::sbuf));
	sb->s_size = 256; sb->s_len = 0; sb->s_error = 0;
	std::memset(sb->s_buf, GUARD, sb->s_size);
	port::stack st{}; st.depth = 2; st.pcs[0] = 0x5000; st.pcs[1] = 0x5100;
	unsigned char rblob[PAD + sizeof(struct sbuf) + 512 + PAD];
	std::memset(rblob, GUARD, sizeof(rblob));
	struct sbuf *rsb = (struct sbuf *)(rblob + PAD);
	rsb->s_buf = (char *)(rblob + PAD + sizeof(struct sbuf));
	rsb->s_size = 256; rsb->s_len = 0; rsb->s_error = 0;
	std::memset(rsb->s_buf, GUARD, rsb->s_size);
	struct stack rst{}; std::memcpy(&rst, &st, sizeof(rst));
	case_row(R_STACK_SBUF_PRINT_FLAGS);
	reset_port();
	int pe = port::stack_sbuf_print_flags(sb, &st, 2, static_cast<port::stack_sbuf_fmt>(1));
	reset_ref();
	int re = ref_stack_sbuf_print_flags(rsb, &rst, 2, 1);
	if (pe != re) fail_row(R_STACK_SBUF_PRINT_FLAGS, "ret", "mismatch");
	if (sb->s_len != rsb->s_len ||
	    std::memcmp(sb->s_buf, rsb->s_buf, sb->s_size) != 0)
		fail_row(R_STACK_SBUF_PRINT_FLAGS, "sbuf", "mismatch");
	if (!buf_ok(blob, PAD) || !buf_ok(blob + PAD + sizeof(port::sbuf) + sb->s_size, PAD))
		fail_row(R_STACK_SBUF_PRINT_FLAGS, "guard", "port");
	case_row(R_STACK_SBUF_PRINT);
	reset_port();
	port::stack_sbuf_print(sb, &st);
	reset_ref();
	ref_stack_sbuf_print(rsb, &rst);
	case_row(R_STACK_SBUF_PRINT_DDB);
	reset_port();
	port::stack_sbuf_print_ddb(sb, &st);
	reset_ref();
	ref_stack_sbuf_print_ddb(rsb, &rst);
}

static void test_stack_ktr(void) {
	port::stack st{}; st.depth = 2; st.pcs[0] = 0x6000; st.pcs[1] = 0x6100;
	struct stack rst{}; std::memcpy(&rst, &st, sizeof(rst));
	case_row(R_STACK_KTR);
	reset_port();
	port::stack_ktr(1, "f.c", 9, &st, 0);
	int pk = port::ktr_count();
	reset_ref();
	ref_stack_ktr(1, "f.c", 9, &rst, 0);
	if (pk != oracle_ktr_count())
		fail_row(R_STACK_KTR, "ktr", "count mismatch");
}

static void test_stack_symbol(uintptr_t pc, unsigned int buflen, int flags) {
	unsigned char pbuf[PAD + 128 + PAD];
	unsigned char rbuf[PAD + 128 + PAD];
	std::memset(pbuf, GUARD, sizeof(pbuf));
	std::memset(rbuf, GUARD, sizeof(rbuf));
	char *pname = (char *)(pbuf + PAD);
	char *rname = (char *)(rbuf + PAD);
	long poff = -1, roff = -1;

	case_row(R_STACK_SYMBOL);
	reset_port();
	int pe = port::stack_symbol(pc, pname, buflen, &poff, flags);
	reset_ref();
	int re = ref_stack_symbol(pc, rname, buflen, &roff, flags);
	if (pe != re || poff != roff)
		fail_row(R_STACK_SYMBOL, "ret", "mismatch");
	if (std::strcmp(pname, rname) != 0)
		fail_row(R_STACK_SYMBOL, "name", "mismatch");
	if (!buf_ok(pbuf, PAD) || !buf_ok(pbuf + PAD + 128, PAD))
		fail_row(R_STACK_SYMBOL, "guard", "port");
	if (!buf_ok(rbuf, PAD) || !buf_ok(rbuf + PAD + 128, PAD))
		fail_row(R_STACK_SYMBOL, "guard", "ref");
}

static void test_stack_symbol_ddb(uintptr_t pc) {
	const char *pname = nullptr;
	const char *rname = nullptr;
	long poff = -1, roff = -1;

	case_row(R_STACK_SYMBOL_DDB);
	reset_port();
	int pe = port::stack_symbol_ddb(pc, &pname, &poff);
	reset_ref();
	int re = ref_stack_symbol_ddb(pc, &rname, &roff);
	if (pe != re || poff != roff)
		fail_row(R_STACK_SYMBOL_DDB, "ret", "mismatch");
	if (std::strcmp(pname, rname) != 0)
		fail_row(R_STACK_SYMBOL_DDB, "name", "mismatch");
}

static void test_vfs_hash_get_enoent_retry(unsigned int hash) {
	port::mount mp{}; mp.mnt_hashseed = 11;
	port::vnode vp{};
	setup_vnode(reinterpret_cast<struct vnode *>(&vp), hash,
	    reinterpret_cast<struct mount *>(&mp));
	port::vnode *ins = &vp;
	reset_port();
	port::set_vget_enoent_once(1);
	port::vfs_hash_insert(&vp, hash, 0, nullptr, &ins, nullptr, nullptr);
	port::vnode *pg = nullptr;
	case_row(R_VFS_HASH_GET);
	int pgd = port::vfs_hash_get(&mp, hash, 0, nullptr, &pg, nullptr, nullptr);
	reset_ref();
	oracle_set_vget_enoent_once(1);
	struct vnode *rins = reinterpret_cast<struct vnode *>(&vp);
	ref_vfs_hash_insert(reinterpret_cast<struct vnode *>(&vp), hash, 0, nullptr,
	    &rins, nullptr, nullptr);
	struct vnode *rg = nullptr;
	int rgd = ref_vfs_hash_get(reinterpret_cast<struct mount *>(&mp), hash, 0,
	    nullptr, &rg, nullptr, nullptr);
	if (pgd != rgd) fail_row(R_VFS_HASH_GET, "retry", "ret mismatch");
}

static void test_vfs_hand(void) {
	g_vget_enoent = 0;
	g_vget_error = 0;
	test_vfs_hash_index();
	test_vfs_hash_insert_get_one(1, 0);
	test_vfs_hash_insert_get_one(99, 0);
	test_vfs_hash_ref_remove(7);
	test_vfs_hash_rehash(123);
	test_vfs_hash_changesize(128);
	test_vfs_hash_changesize(64);
	g_vget_enoent = 1;
	test_vfs_hash_insert_get_one(77, 0);
	g_vget_enoent = 0;
	test_vfs_hash_get_enoent_retry(55);
	g_vget_error = 13;
	test_vfs_hash_insert_get_one(88, 0);
	g_vget_error = 0;
}

static void test_tslog_hand(void) {
	test_tslog_one(nullptr, 0, "fn", "arg");
	test_tslog_one(nullptr, 1, "x", nullptr);
	test_tslog_one(nullptr, 2, nullptr, "s");
	test_tslog_user(1, 2, nullptr, nullptr);
	test_tslog_user(1, -1, "exec", nullptr);
	test_tslog_user(1, -1, "exec2", nullptr);
	test_tslog_user(1, -1, nullptr, "/path");
	test_tslog_user(1, -1, nullptr, nullptr);
	test_tslog_user(1, 3, nullptr, nullptr);
	test_tslog_user(1, 4, nullptr, nullptr);
	test_sysctl_tslog();
	g_sbuf_fail = 1;
	test_sysctl_tslog();
	g_sbuf_fail = 0;
	test_sysctl_tslog_user();
	test_sysinit_shim();
	test_tslog_reset();
}

static void test_stack_hand(void) {
	g_linker_fail = 0;
	g_linker_block = 0;
	test_stack_lifecycle();
	test_stack_prints();
	test_stack_sbuf();
	test_stack_ktr();
	test_stack_symbol(0xdead, 64, 2);
	test_stack_symbol(0xbeef, 9, 2);
	test_stack_symbol(0xcafe, 3, 2);
	g_linker_fail = 1;
	test_stack_symbol(0x1000, 64, 2);
	g_linker_fail = 0;
	g_linker_block = 1;
	test_stack_symbol(0x2000, 64, 1);
	g_linker_block = 0;
	test_stack_symbol_ddb(0x3000);
}

static void test_clockcalib_hand(void) {
	g_clk_mul = 3000;
	g_tc_step = 1;
	g_tc.tc_frequency = 1000000000ULL;
	bootverbose = 0;
	test_clockcalib();
	bootverbose = 1;
	test_clockcalib();
	bootverbose = 0;
	g_tc.tc_frequency = 50;
	g_clk_mul = 100000;
	test_clockcalib();
	g_tc.tc_frequency = 1000000000ULL;
}

static void sweep_vfs(void) {
	for (long i = 0; i < SWEEP; i++) {
		g_vget_enoent = (int)(rnd32() % 20 == 0);
		g_vget_error = g_vget_enoent ? 0 : (int)((rnd32() % 50 == 0) ? 13 : 0);
		test_vfs_hash_index();
		test_vfs_hash_insert_get_one((unsigned int)(rnd32() % 500), (int)(rnd32() & 1));
		if ((rnd32() % 50) == 0)
			test_vfs_hash_changesize((unsigned long)(rnd32() % 512 + 1));
	}
	g_vget_enoent = 0;
	g_vget_error = 0;
}

static void sweep_tslog(void) {
	for (long i = 0; i < SWEEP; i++) {
		test_tslog_one(nullptr, (int)(rnd32() % 4), "f", (rnd32() & 1) ? "s" : nullptr);
		if ((rnd32() % 10) == 0)
			test_tslog_user((int)(rnd32() % 100),
			    (int)(rnd32() % 2 ? -1 : (int)(rnd32() % 50)),
			    (rnd32() & 1) ? "e" : nullptr, (rnd32() & 2) ? "/n" : nullptr);
	}
}

static void sweep_clockcalib(void) {
	for (long i = 0; i < SWEEP; i++) {
		g_clk_mul = (uint64_t)(rnd32() % 10000 + 1);
		g_tc_step = (uint64_t)(rnd32() % 7 + 1);
		if ((rnd32() % 100) == 0)
			g_tc.tc_frequency = (uint64_t)(rnd32() % 1000 + 10);
		else
			g_tc.tc_frequency = 1000000000ULL;
		bootverbose = (int)(rnd32() & 1);
		test_clockcalib();
	}
	bootverbose = 0;
	g_tc.tc_frequency = 1000000000ULL;
}

static void sweep_stack(void) {
	for (long i = 0; i < SWEEP; i++) {
		test_stack_lifecycle();
		if ((rnd32() % 20) == 0)
			test_stack_prints();
	}
}

int main(void) {
	test_vfs_hand();
	test_tslog_hand();
	test_stack_hand();
	test_clockcalib_hand();
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
