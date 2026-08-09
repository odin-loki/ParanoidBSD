/*
 * Differential test for PBSD batch b0085 (reg_defs_t4vf.c).
 *
 * The source defines register-map tables only (no functions).  Each table is
 * compared element-for-element against the ref_ oracle.
 */

import pbsd.usr.sbin.cxgbetool.b0085;

#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" {
struct reg_info {
	const char *name;
	std::uint32_t addr;
	std::uint32_t len;
};

extern struct reg_info ref_t4vf_sge_regs[];
extern struct reg_info ref_t5vf_sge_regs[];
extern struct reg_info ref_t4vf_mps_regs[];
extern struct reg_info ref_t4vf_pl_regs[];
extern struct reg_info ref_t5vf_pl_regs[];
extern struct reg_info ref_t6vf_pl_regs[];
extern struct reg_info ref_t4vf_cim_regs[];
extern struct reg_info ref_t4vf_mbdata_regs[];
}

namespace P = pbsd::usr_sbin_cxgbetool::b0085;

static const unsigned long long RANDOM_ITERATIONS = 200000ULL;
static const int MAX_REPORT = 8;

struct TablePair {
	const char *name;
	const P::reg_info *port;
	struct reg_info *ref;
};

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned reported;
};

static std::uint64_t rng_state = 0xB0085ULL;

static std::uint64_t
rnd64(void)
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static std::size_t
count_entries(const P::reg_info *arr)
{
	std::size_t n = 0;

	while (arr[n].name != nullptr)
		++n;
	return n + 1;
}

static std::size_t
count_entries_c(struct reg_info *arr)
{
	std::size_t n = 0;

	while (arr[n].name != nullptr)
		++n;
	return n + 1;
}

static bool
entry_same(const P::reg_info &port, const struct reg_info &ref)
{
	if ((port.name == nullptr) != (ref.name == nullptr))
		return false;
	if (port.name != nullptr && std::strcmp(port.name, ref.name) != 0)
		return false;
	return port.addr == ref.addr && port.len == ref.len;
}

static void
report_fail(Stat &st, std::size_t idx, const P::reg_info &port,
    const struct reg_info &ref, const char *tag)
{
	++st.failures;
	if (st.reported >= MAX_REPORT)
		return;
	++st.reported;
	std::fprintf(stderr,
	    "  [%s] idx %zu port{name=%s addr=0x%x len=%u} "
	    "ref{name=%s addr=0x%x len=%u}\n",
	    tag, idx,
	    port.name ? port.name : "(null)",
	    port.addr, port.len,
	    ref.name ? ref.name : "(null)",
	    ref.addr, ref.len);
}

static bool
check_index(Stat &st, const TablePair &tp, std::size_t idx, const char *tag)
{
	const P::reg_info &port = tp.port[idx];
	const struct reg_info &ref = tp.ref[idx];
	const bool ok = entry_same(port, ref);

	++st.cases;
	if (!ok)
		report_fail(st, idx, port, ref, tag);
	return ok;
}

static bool
check_full_scan(Stat &st, const TablePair &tp)
{
	const std::size_t port_n = count_entries(tp.port);
	const std::size_t ref_n = count_entries_c(tp.ref);
	bool ok = true;

	++st.cases;
	if (port_n != ref_n) {
		++st.failures;
		if (st.reported < MAX_REPORT) {
			++st.reported;
			std::fprintf(stderr,
			    "  [full_scan] length mismatch port=%zu ref=%zu\n",
			    port_n, ref_n);
		}
		ok = false;
	}

	const std::size_t n = port_n < ref_n ? port_n : ref_n;
	for (std::size_t i = 0; i < n; ++i)
		ok = check_index(st, tp, i, "full_scan") && ok;
	return ok;
}

static bool
check_edge_indices(Stat &st, const TablePair &tp)
{
	const std::size_t n = count_entries(tp.port);
	bool ok = true;

	ok = check_index(st, tp, 0, "edge_first") && ok;
	ok = check_index(st, tp, n - 1, "edge_terminator") && ok;
	if (n > 2)
		ok = check_index(st, tp, n - 2, "edge_before_term") && ok;
	return ok;
}

static bool
check_field_and_register_entries(Stat &st, const TablePair &tp)
{
	const std::size_t n = count_entries(tp.port);
	bool ok = true;
	bool saw_field = false;
	bool saw_register = false;

	for (std::size_t i = 0; i + 1 < n; ++i) {
		if (tp.port[i].len == 0) {
			ok = check_index(st, tp, i, "edge_register") && ok;
			saw_register = true;
		} else {
			ok = check_index(st, tp, i, "edge_field") && ok;
			saw_field = true;
		}
	}

	++st.cases;
	if (!saw_register || !saw_field) {
		++st.failures;
		if (st.reported < MAX_REPORT) {
			++st.reported;
			std::fprintf(stderr,
			    "  [edge_kind] missing register=%d field=%d\n",
			    saw_register ? 1 : 0, saw_field ? 1 : 0);
		}
		ok = false;
	}
	return ok;
}

static const TablePair tp_t4vf_sge_regs = {
	"t4vf_sge_regs", P::t4vf_sge_regs, ref_t4vf_sge_regs
};
static const TablePair tp_t5vf_sge_regs = {
	"t5vf_sge_regs", P::t5vf_sge_regs, ref_t5vf_sge_regs
};
static const TablePair tp_t4vf_mps_regs = {
	"t4vf_mps_regs", P::t4vf_mps_regs, ref_t4vf_mps_regs
};
static const TablePair tp_t4vf_pl_regs = {
	"t4vf_pl_regs", P::t4vf_pl_regs, ref_t4vf_pl_regs
};
static const TablePair tp_t5vf_pl_regs = {
	"t5vf_pl_regs", P::t5vf_pl_regs, ref_t5vf_pl_regs
};
static const TablePair tp_t6vf_pl_regs = {
	"t6vf_pl_regs", P::t6vf_pl_regs, ref_t6vf_pl_regs
};
static const TablePair tp_t4vf_cim_regs = {
	"t4vf_cim_regs", P::t4vf_cim_regs, ref_t4vf_cim_regs
};
static const TablePair tp_t4vf_mbdata_regs = {
	"t4vf_mbdata_regs", P::t4vf_mbdata_regs, ref_t4vf_mbdata_regs
};

static bool
check_t5_t6_divergence(Stat &st, Stat &st_t6)
{
	bool ok = true;

	for (std::size_t i = 0; tp_t5vf_pl_regs.port[i].name != nullptr; ++i) {
		const P::reg_info &t5 = tp_t5vf_pl_regs.port[i];
		const P::reg_info &t6 = tp_t6vf_pl_regs.port[i];
		const char *n = t5.name;

		if (n != nullptr && std::strcmp(n, "SourcePF") == 0) {
			++st.cases;
			++st_t6.cases;
			if (t5.addr != 8 || t5.len != 3) {
				++st.failures;
				ok = false;
			}
			if (t6.addr != 9 || t6.len != 3) {
				++st_t6.failures;
				ok = false;
			}
		} else if (n != nullptr && std::strcmp(n, "IsVF") == 0) {
			++st.cases;
			++st_t6.cases;
			if (t5.addr != 7 || t5.len != 1) {
				++st.failures;
				ok = false;
			}
			if (t6.addr != 8 || t6.len != 1) {
				++st_t6.failures;
				ok = false;
			}
		} else if (n != nullptr && std::strcmp(n, "VFID") == 0) {
			++st.cases;
			++st_t6.cases;
			if (t5.addr != 0 || t5.len != 7) {
				++st.failures;
				ok = false;
			}
			if (t6.addr != 0 || t6.len != 8) {
				++st_t6.failures;
				ok = false;
			}
		}
	}
	return ok;
}

static const TablePair all_tables[] = {
	tp_t4vf_sge_regs,
	tp_t5vf_sge_regs,
	tp_t4vf_mps_regs,
	tp_t4vf_pl_regs,
	tp_t5vf_pl_regs,
	tp_t6vf_pl_regs,
	tp_t4vf_cim_regs,
	tp_t4vf_mbdata_regs,
};

static Stat stats[8];

static bool
run_random_sweep(void)
{
	const std::size_t ntables = sizeof(all_tables) / sizeof(all_tables[0]);
	bool ok = true;

	for (unsigned long long i = 0; i < RANDOM_ITERATIONS; ++i) {
		const std::size_t tidx = (std::size_t)(rnd64() % ntables);
		const TablePair &tp = all_tables[tidx];
		const std::size_t len = count_entries(tp.port);
		const std::size_t idx = (std::size_t)(rnd64() % len);

		ok = check_index(stats[tidx], tp, idx, "random") && ok;
	}
	return ok;
}

static bool
run_table_tests(void)
{
	bool ok = true;

	for (std::size_t i = 0; i < sizeof(all_tables) / sizeof(all_tables[0]); ++i) {
		const TablePair &tp = all_tables[i];
		Stat &st = stats[i];

		st.name = tp.name;
		ok = check_full_scan(st, tp) && ok;
		ok = check_edge_indices(st, tp) && ok;
		ok = check_field_and_register_entries(st, tp) && ok;
	}

	ok = check_t5_t6_divergence(stats[4], stats[5]) && ok;
	ok = run_random_sweep() && ok;
	return ok;
}

int
main(void)
{
	const bool ok = run_table_tests();

	std::printf("%-22s %12s %12s\n", "table", "cases", "failures");
	for (const Stat &st : stats)
		std::printf("%-22s %12llu %12llu\n",
		    st.name, st.cases, st.failures);

	return ok ? 0 : 1;
}
