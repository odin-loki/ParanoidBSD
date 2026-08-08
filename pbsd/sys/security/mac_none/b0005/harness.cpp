/*
 * harness.cpp -- differential test for PBSD batch b0005.
 *
 * mac_none.c defines no callable functions; it registers an empty MAC policy
 * via MAC_POLICY_SET().  The harness compares the resulting static data --
 * none_ops, mac_none_mac_policy_conf and mac_none_mod -- between the oracle
 * and the port.  Pointer fields are compared as offsets from their owning
 * object bases, never as raw addresses across translation units.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.sys.security.mac.none.b0005;

namespace port = pbsd::sys_security_mac_none::b0005;

extern "C" {
struct mac_policy_ops;
struct mac_policy_conf;
typedef struct moduledata {
	const char *name;
	int (*modevent)(void *, int, void *);
	void *priv;
} moduledata_t;

extern const struct mac_policy_ops *ref_none_ops(void);
extern const struct mac_policy_conf *ref_mac_none_mac_policy_conf(void);
extern const moduledata_t *ref_mac_none_mod(void);
}

namespace {

constexpr int MAX_REPORT = 10;
constexpr long RANDOM_ITERATIONS = 200000;
constexpr int OPS_SLOTS = 255;

constexpr int MPC_LOADTIME_FLAG_NOTLATE = 0x00000001;
constexpr int MPC_LOADTIME_FLAG_UNLOADOK = 0x00000002;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_none_ops{"none_ops", 0, 0, 0};
Stat st_mac_none_conf{"mac_none_mac_policy_conf", 0, 0, 0};
Stat st_mac_none_mod{"mac_none_mod", 0, 0, 0};

void report_fail(Stat &st, const char *detail)
{
	++st.fails;
	if (st.reported < MAX_REPORT) {
		std::printf("  FAIL %s: %s\n", st.name, detail);
		++st.reported;
	}
}

/* splitmix64 */
std::uint64_t prng_state;

void prng_seed(std::uint64_t seed) { prng_state = seed; }

std::uint64_t prng_next()
{
	std::uint64_t z = (prng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

const void *ops_slot_ptr(const void *ops_base, int slot)
{
	const auto *bytes = static_cast<const unsigned char *>(ops_base);
	return bytes + static_cast<std::size_t>(slot) * sizeof(void *);
}

void check_none_ops()
{
	const auto &port_ops = port::get_none_ops();
	const struct mac_policy_ops *ref_ops = ref_none_ops();

	++st_none_ops.cases;
	if (sizeof(port_ops) != sizeof(*ref_ops)) {
		report_fail(st_none_ops, "sizeof(mac_policy_ops) mismatch");
		return;
	}

	++st_none_ops.cases;
	if (std::memcmp(&port_ops, ref_ops, sizeof(port_ops)) != 0) {
		report_fail(st_none_ops, "memcmp(mac_policy_ops) mismatch");
	}

	for (int slot = 0; slot < OPS_SLOTS; ++slot) {
		++st_none_ops.cases;
		const void *p_port = ops_slot_ptr(&port_ops, slot);
		const void *p_ref = ops_slot_ptr(ref_ops, slot);
		if (std::memcmp(p_port, p_ref, sizeof(void *)) != 0) {
			report_fail(st_none_ops, "operation slot not identically zero");
			break;
		}
	}

	/* boundary: first, last, and interior slots */
	const int edge_slots[] = {0, 1, 2, 126, 127, 128, 252, 253, 254};
	for (int slot : edge_slots) {
		++st_none_ops.cases;
		const void *p_port = ops_slot_ptr(&port_ops, slot);
		const void *p_ref = ops_slot_ptr(ref_ops, slot);
		if (p_port == nullptr || p_ref == nullptr ||
		    std::memcmp(p_port, p_ref, sizeof(void *)) != 0) {
			report_fail(st_none_ops, "edge slot not identically zero");
		}
	}
}

void check_conf_field_strings(const struct mac_policy_conf *ref_conf,
    const port::mac_policy_conf &port_conf)
{
	++st_mac_none_conf.cases;
	if (ref_conf->mpc_name == nullptr ||
	    std::strcmp(ref_conf->mpc_name, "mac_none") != 0) {
		report_fail(st_mac_none_conf, "oracle mpc_name");
	}
	++st_mac_none_conf.cases;
	if (port_conf.mpc_name == nullptr ||
	    std::strcmp(port_conf.mpc_name, "mac_none") != 0) {
		report_fail(st_mac_none_conf, "port mpc_name");
	}

	++st_mac_none_conf.cases;
	if (ref_conf->mpc_fullname == nullptr ||
	    std::strcmp(ref_conf->mpc_fullname, "TrustedBSD MAC/None") != 0) {
		report_fail(st_mac_none_conf, "oracle mpc_fullname");
	}
	++st_mac_none_conf.cases;
	if (port_conf.mpc_fullname == nullptr ||
	    std::strcmp(port_conf.mpc_fullname, "TrustedBSD MAC/None") != 0) {
		report_fail(st_mac_none_conf, "port mpc_fullname");
	}

	const char *edge_names[] = {"", "mac_non", "mac_none", "mac_noneX",
	    "TrustedBSD MAC/None"};
	for (const char *probe : edge_names) {
		++st_mac_none_conf.cases;
		const int ref_cmp = ref_conf->mpc_name == nullptr ?
		    -1 :
		    std::strcmp(ref_conf->mpc_name, probe);
		const int port_cmp = port_conf.mpc_name == nullptr ?
		    -1 :
		    std::strcmp(port_conf.mpc_name, probe);
		if ((ref_cmp == 0) != (port_cmp == 0)) {
			report_fail(st_mac_none_conf, "mpc_name strcmp boundary");
		}
	}
}

void check_conf_field_ints(const struct mac_policy_conf *ref_conf,
    const port::mac_policy_conf &port_conf)
{
	++st_mac_none_conf.cases;
	if (ref_conf->mpc_loadtime_flags != MPC_LOADTIME_FLAG_UNLOADOK) {
		report_fail(st_mac_none_conf, "oracle mpc_loadtime_flags");
	}
	++st_mac_none_conf.cases;
	if (port_conf.mpc_loadtime_flags != MPC_LOADTIME_FLAG_UNLOADOK) {
		report_fail(st_mac_none_conf, "port mpc_loadtime_flags");
	}

	const int flag_probes[] = {0, 1, MPC_LOADTIME_FLAG_NOTLATE,
	    MPC_LOADTIME_FLAG_UNLOADOK, MPC_LOADTIME_FLAG_UNLOADOK - 1,
	    MPC_LOADTIME_FLAG_UNLOADOK + 1, 0x7f, 0x80, 0xff, -1, 0x7fffffff};
	for (int probe : flag_probes) {
		++st_mac_none_conf.cases;
		const int ref_eq = (ref_conf->mpc_loadtime_flags == probe);
		const int port_eq = (port_conf.mpc_loadtime_flags == probe);
		if (ref_eq != port_eq) {
			report_fail(st_mac_none_conf,
			    "mpc_loadtime_flags equality boundary");
		}
	}

	++st_mac_none_conf.cases;
	if (ref_conf->mpc_field_off != nullptr) {
		report_fail(st_mac_none_conf, "oracle mpc_field_off");
	}
	++st_mac_none_conf.cases;
	if (port_conf.mpc_field_off != nullptr) {
		report_fail(st_mac_none_conf, "port mpc_field_off");
	}

	++st_mac_none_conf.cases;
	if (ref_conf->mpc_runtime_flags != 0) {
		report_fail(st_mac_none_conf, "oracle mpc_runtime_flags");
	}
	++st_mac_none_conf.cases;
	if (port_conf.mpc_runtime_flags != 0) {
		report_fail(st_mac_none_conf, "port mpc_runtime_flags");
	}

	++st_mac_none_conf.cases;
	if (ref_conf->_mpc_spare1 != 0 || ref_conf->_mpc_spare2 != 0 ||
	    ref_conf->_mpc_spare3 != 0 || ref_conf->_mpc_spare4 != nullptr) {
		report_fail(st_mac_none_conf, "oracle spare fields");
	}
	++st_mac_none_conf.cases;
	if (port_conf._mpc_spare1 != 0 || port_conf._mpc_spare2 != 0 ||
	    port_conf._mpc_spare3 != 0 || port_conf._mpc_spare4 != nullptr) {
		report_fail(st_mac_none_conf, "port spare fields");
	}

	++st_mac_none_conf.cases;
	if (ref_conf->mpc_list.le_next != nullptr ||
	    ref_conf->mpc_list.le_prev != nullptr) {
		report_fail(st_mac_none_conf, "oracle mpc_list");
	}
	++st_mac_none_conf.cases;
	if (port_conf.mpc_list.le_next != nullptr ||
	    port_conf.mpc_list.le_prev != nullptr) {
		report_fail(st_mac_none_conf, "port mpc_list");
	}
}

void check_conf_ops_pointer(const struct mac_policy_conf *ref_conf,
    const port::mac_policy_conf &port_conf,
    const struct mac_policy_ops *ref_ops,
    const port::mac_policy_ops &port_ops)
{
	++st_mac_none_conf.cases;
	if (ref_conf->mpc_ops != ref_ops) {
		report_fail(st_mac_none_conf, "oracle mpc_ops target");
	}
	++st_mac_none_conf.cases;
	if (port_conf.mpc_ops != &port_ops) {
		report_fail(st_mac_none_conf, "port mpc_ops target");
	}

	const std::ptrdiff_t ref_off = reinterpret_cast<const char *>(ref_conf->mpc_ops) -
	    reinterpret_cast<const char *>(ref_ops);
	const std::ptrdiff_t port_off =
	    reinterpret_cast<const char *>(port_conf.mpc_ops) -
	    reinterpret_cast<const char *>(&port_ops);
	++st_mac_none_conf.cases;
	if (ref_off != 0 || port_off != 0) {
		report_fail(st_mac_none_conf, "mpc_ops offset from none_ops");
	}
}

void check_mac_none_conf()
{
	const struct mac_policy_conf *ref_conf = ref_mac_none_mac_policy_conf();
	const auto &port_conf = port::get_mac_none_mac_policy_conf();
	const struct mac_policy_ops *ref_ops = ref_none_ops();
	const auto &port_ops = port::get_none_ops();

	++st_mac_none_conf.cases;
	if (sizeof(port_conf) != sizeof(*ref_conf)) {
		report_fail(st_mac_none_conf, "sizeof(mac_policy_conf) mismatch");
		return;
	}

	check_conf_field_strings(ref_conf, port_conf);
	check_conf_field_ints(ref_conf, port_conf);
	check_conf_ops_pointer(ref_conf, port_conf, ref_ops, port_ops);
}

void check_mac_none_mod()
{
	const moduledata_t *ref_mod = ref_mac_none_mod();
	const auto &port_mod = port::get_mac_none_mod();
	const struct mac_policy_conf *ref_conf = ref_mac_none_mac_policy_conf();
	const auto &port_conf = port::get_mac_none_mac_policy_conf();

	++st_mac_none_mod.cases;
	if (ref_mod->name == nullptr || std::strcmp(ref_mod->name, "mac_none") != 0) {
		report_fail(st_mac_none_mod, "oracle mod name");
	}
	++st_mac_none_mod.cases;
	if (port_mod.name == nullptr || std::strcmp(port_mod.name, "mac_none") != 0) {
		report_fail(st_mac_none_mod, "port mod name");
	}

	++st_mac_none_mod.cases;
	if (ref_mod->modevent == nullptr) {
		report_fail(st_mac_none_mod, "oracle mod modevent null");
	}
	++st_mac_none_mod.cases;
	if (port_mod.modevent == nullptr) {
		report_fail(st_mac_none_mod, "port mod modevent null");
	}

	const std::ptrdiff_t ref_priv_off =
	    reinterpret_cast<const char *>(ref_mod->priv) -
	    reinterpret_cast<const char *>(ref_conf);
	const std::ptrdiff_t port_priv_off =
	    reinterpret_cast<const char *>(port_mod.priv) -
	    reinterpret_cast<const char *>(&port_conf);
	++st_mac_none_mod.cases;
	if (ref_priv_off != 0 || port_priv_off != 0) {
		report_fail(st_mac_none_mod, "mod.priv offset from conf");
	}

	++st_mac_none_mod.cases;
	if (ref_mod->priv != ref_conf) {
		report_fail(st_mac_none_mod, "oracle mod.priv target");
	}
	++st_mac_none_mod.cases;
	if (port_mod.priv != &port_conf) {
		report_fail(st_mac_none_mod, "port mod.priv target");
	}
}

void run_random_sweep()
{
	const auto &port_ops = port::get_none_ops();
	const struct mac_policy_ops *ref_ops = ref_none_ops();
	const struct mac_policy_conf *ref_conf = ref_mac_none_mac_policy_conf();
	const auto &port_conf = port::get_mac_none_mac_policy_conf();

	prng_seed(0xB0005D1FFULL);

	for (long i = 0; i < RANDOM_ITERATIONS; ++i) {
		const std::uint64_t r = prng_next();
		const int slot = static_cast<int>(r % OPS_SLOTS);

		++st_none_ops.cases;
		const void *p_port = ops_slot_ptr(&port_ops, slot);
		const void *p_ref = ops_slot_ptr(ref_ops, slot);
		if (std::memcmp(p_port, p_ref, sizeof(void *)) != 0) {
			report_fail(st_none_ops, "random slot mismatch");
		}

		++st_mac_none_conf.cases;
		const int probe = static_cast<int>(r >> 32);
		const int ref_eq = (ref_conf->mpc_loadtime_flags == probe);
		const int port_eq = (port_conf.mpc_loadtime_flags == probe);
		if (ref_eq != port_eq) {
			report_fail(st_mac_none_conf, "random flag equality");
		}

		++st_mac_none_conf.cases;
		const std::size_t name_idx =
		    static_cast<std::size_t>((r >> 16) & 0xf);
		const char *name = ref_conf->mpc_name;
		const unsigned char ch =
		    (name != nullptr && name[name_idx] != '\0') ?
		    static_cast<unsigned char>(name[name_idx]) :
		    0;
		const unsigned char port_ch =
		    (port_conf.mpc_name != nullptr &&
			port_conf.mpc_name[name_idx] != '\0') ?
		    static_cast<unsigned char>(port_conf.mpc_name[name_idx]) :
		    0;
		if (ch != port_ch) {
			report_fail(st_mac_none_conf, "random mpc_name byte");
		}

		++st_mac_none_mod.cases;
		const int ref_name_cmp = ref_conf->mpc_name == nullptr ?
		    -1 :
		    std::strcmp(ref_conf->mpc_name, "mac_none");
		const int port_name_cmp = port_conf.mpc_name == nullptr ?
		    -1 :
		    std::strcmp(port_conf.mpc_name, "mac_none");
		if (ref_name_cmp != port_name_cmp) {
			report_fail(st_mac_none_mod, "random mpc_name strcmp");
		}
	}
}

} /* namespace */

int main()
{
	std::printf("PBSD batch b0005 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	check_none_ops();
	check_mac_none_conf();
	check_mac_none_mod();
	run_random_sweep();

	const Stat *all[] = {&st_none_ops, &st_mac_none_conf, &st_mac_none_mod};

	std::printf("\n%-28s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-28s %12s %12s %10s\n", "----------------------------",
	    "------------", "------------", "----------");

	long total_cases = 0;
	long total_fails = 0;
	for (const Stat *s : all) {
		total_cases += s->cases;
		total_fails += s->fails;
		std::printf("%-28s %12ld %12ld %10s\n", s->name, s->cases,
		    s->fails, s->fails == 0 ? "PASS" : "FAIL");
	}
	std::printf("%-28s %12s %12s %10s\n", "----------------------------",
	    "------------", "------------", "----------");
	std::printf("%-28s %12ld %12ld %10s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	return total_fails == 0 ? 0 : 1;
}
