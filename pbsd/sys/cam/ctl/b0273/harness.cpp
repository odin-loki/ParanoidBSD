/*
 * Differential test harness for PBSD batch b0273.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>

import pbsd.sys.cam.ctl.b0273;

namespace P = pbsd::sys_cam_ctl::b0273;

extern "C" {

struct ref_sbuf {
	char *s_buf;
	void *s_drain_func;
	void *s_drain_arg;
	int s_error;
	long s_size;
	long s_len;
	int s_flags;
	long s_sect_len;
	long s_rec_off;
};

struct ref_nvme_command {
	uint8_t opc, fuse;
	uint16_t cid;
	uint32_t nsid;
	uint32_t rsvd2, rsvd3;
	uint64_t mptr;
	uint64_t prp1, prp2;
	uint32_t cdw10, cdw11, cdw12, cdw13, cdw14, cdw15;
} __attribute__((aligned(8)));

struct ref_nvme_completion {
	uint32_t cdw0, rsvd1;
	uint16_t sqhd, sqid;
	uint16_t cid, status;
} __attribute__((aligned(8)));

enum ref_ctl_io_type {
	REF_CTL_IO_NONE,
	REF_CTL_IO_SCSI,
	REF_CTL_IO_TASK,
	REF_CTL_IO_NVME,
	REF_CTL_IO_NVME_ADMIN,
};

struct ref_ctl_nexus { uint32_t a, b, c, d; };
struct ref_bintime { long sec; uint64_t frac; };
struct ref_ctl_io_hdr {
	uint32_t version;
	ref_ctl_io_type io_type;
	int msg_type;
	ref_ctl_nexus nexus;
	uint32_t iid_indx, flags, status, port_status, timeout, retries;
	long start_time;
	ref_bintime start_bt, dma_start_bt, dma_bt;
	uint32_t num_dmas;
	void *remote_io, *blocker, *pool;
	uint8_t ctl_private[96];
	uint8_t blocked_queue[16];
	uint8_t links[8];
	uint8_t ooa_links[16];
	uint8_t blocked_links[16];
};

struct ref_ctl_nvmeio {
	uint8_t bytes[472];
};

static constexpr std::size_t REF_IO_TYPE_OFF = 4;
static constexpr std::size_t REF_CMD_OPC_OFF = 352;
static constexpr std::size_t REF_CPL_STATUS_OFF = 430;

static void ref_set_io_type(ref_ctl_nvmeio &io, ref_ctl_io_type t)
{
	*reinterpret_cast<ref_ctl_io_type *>(io.bytes + REF_IO_TYPE_OFF) = t;
}

static void ref_set_opc(ref_ctl_nvmeio &io, uint8_t opc)
{
	io.bytes[REF_CMD_OPC_OFF] = opc;
}

static void ref_set_status(ref_ctl_nvmeio &io, uint16_t status)
{
	*reinterpret_cast<uint16_t *>(io.bytes + REF_CPL_STATUS_OFF) = status;
}

struct ref_ctl_backend_driver {
	char name[32];
	int flags;
	int (*init)();
	int (*shutdown)();
	void *data_submit;
	void *config_read;
	void *config_write;
	void *ioctl;
	void *lun_info;
	void *lun_attr;
	struct { struct ref_ctl_backend_driver *stqe_next; } links;
};

struct ref_ctl_nvme_cmd_entry {
	int (*execute)(struct ref_ctl_nvmeio *);
	int flags;
};

extern const uint8_t ctl_serialize_table[14][14];
extern const struct ref_ctl_nvme_cmd_entry nvme_admin_cmd_table[256];
extern const struct ref_ctl_nvme_cmd_entry nvme_nvm_cmd_table[256];

void ref_ctl_nvme_command_string(struct ref_ctl_nvmeio *ctnio, struct ref_sbuf *sb);
void ref_ctl_nvme_status_string(struct ref_ctl_nvmeio *ctnio, struct ref_sbuf *sb);
int ref_ctl_backend_register(struct ref_ctl_backend_driver *be);
int ref_ctl_backend_deregister(struct ref_ctl_backend_driver *be);
struct ref_ctl_backend_driver *ref_ctl_backend_find(char *backend_name);
void ref_reset_backend_state(void);
uint32_t ref_num_backends(void);

int ctl_nvme_identify(struct ref_ctl_nvmeio *ctnio);
int ctl_nvme_flush(struct ref_ctl_nvmeio *ctnio);
int ctl_nvme_read_write(struct ref_ctl_nvmeio *ctnio);
int ctl_nvme_write_uncorrectable(struct ref_ctl_nvmeio *ctnio);
int ctl_nvme_compare(struct ref_ctl_nvmeio *ctnio);
int ctl_nvme_write_zeroes(struct ref_ctl_nvmeio *ctnio);
int ctl_nvme_dataset_management(struct ref_ctl_nvmeio *ctnio);
int ctl_nvme_verify(struct ref_ctl_nvmeio *ctnio);

} // extern "C"

static const char *kNames[] = {
	"ctl_serialize_table",
	"nvme_admin_cmd_table",
	"nvme_nvm_cmd_table",
	"ctl_nvme_command_string",
	"ctl_nvme_status_string",
	"ctl_backend_register",
	"ctl_backend_deregister",
	"ctl_backend_find",
};

enum FnIdx {
	FN_SER = 0,
	FN_ADMIN_TBL,
	FN_NVM_TBL,
	FN_CMD_STR,
	FN_STS_STR,
	FN_REG,
	FN_DEREG,
	FN_FIND,
	FN_COUNT
};

static unsigned long g_cases[FN_COUNT];
static unsigned long g_fails[FN_COUNT];

static void fail_row(int fn, const char *why)
{
	++g_fails[fn];
	if (g_fails[fn] <= 3)
		std::fprintf(stderr, "FAIL %s: %s\n", kNames[fn], why);
}

static int stub_init_ok() { return 0; }
static int stub_init_fail() { return 42; }
static int stub_shutdown_ok() { return 0; }
static int stub_shutdown_fail() { return 17; }

static int fn_index(void *p)
{
	if (p == (void *)P::ctl_nvme_identify) return 1;
	if (p == (void *)P::ctl_nvme_flush) return 2;
	if (p == (void *)P::ctl_nvme_read_write) return 3;
	if (p == (void *)P::ctl_nvme_write_uncorrectable) return 4;
	if (p == (void *)P::ctl_nvme_compare) return 5;
	if (p == (void *)P::ctl_nvme_write_zeroes) return 6;
	if (p == (void *)P::ctl_nvme_dataset_management) return 7;
	if (p == (void *)P::ctl_nvme_verify) return 8;
	return 0;
}

static int ref_fn_index(void *p)
{
	if (p == (void *)ctl_nvme_identify) return 1;
	if (p == (void *)ctl_nvme_flush) return 2;
	if (p == (void *)ctl_nvme_read_write) return 3;
	if (p == (void *)ctl_nvme_write_uncorrectable) return 4;
	if (p == (void *)ctl_nvme_compare) return 5;
	if (p == (void *)ctl_nvme_write_zeroes) return 6;
	if (p == (void *)ctl_nvme_dataset_management) return 7;
	if (p == (void *)ctl_nvme_verify) return 8;
	return 0;
}

static void test_serialize_table()
{
	for (int r = 0; r < static_cast<int>(P::CTL_SERIDX_COUNT); ++r) {
		for (int c = 0; c < static_cast<int>(P::CTL_SERIDX_COUNT); ++c) {
			++g_cases[FN_SER];
			if (P::ctl_serialize_table_ref()[r][c] != ctl_serialize_table[r][c])
				fail_row(FN_SER, "byte mismatch");
		}
	}
}

static void test_cmd_table(int fn, P::ctl_nvme_cmd_entry *ptbl,
    const ref_ctl_nvme_cmd_entry *rtbl)
{
	for (int opc = 0; opc < 256; ++opc) {
		++g_cases[fn];
		if (ptbl[opc].flags != rtbl[opc].flags)
			fail_row(fn, "flags mismatch");
		const int pi = fn_index((void *)ptbl[opc].execute);
		const int ri = ref_fn_index((void *)rtbl[opc].execute);
		if (pi != ri)
			fail_row(fn, "execute mismatch");
	}
}

static void fill_guard(char *buf, std::size_t n, unsigned char v = 0x7f)
{
	std::memset(buf, v, n);
}

static void compare_sbufs(const char *pbuf, long plen, const char *rbuf, long rlen,
    int fn)
{
	if (plen != rlen || std::memcmp(pbuf, rbuf, (std::size_t)plen) != 0)
		fail_row(fn, "sbuf mismatch");
}

static void run_nvme_string_case(int fn, bool admin, uint8_t opc, uint16_t status)
{
	const std::size_t cap = 256;
	char pscratch[cap + 64], rscratch[cap + 64];
	fill_guard(pscratch, sizeof(pscratch));
	fill_guard(rscratch, sizeof(rscratch));

	P::ctl_nvmeio pio{};
	ref_ctl_nvmeio rio{};
	P::sbuf psb{};
	ref_sbuf rsb{};
	psb.s_buf = pscratch + 32;
	psb.s_size = cap;
	psb.s_len = 0;
	rsb.s_buf = rscratch + 32;
	rsb.s_size = cap;
	rsb.s_len = 0;

	pio.io_hdr.io_type = admin ? P::CTL_IO_NVME_ADMIN : P::CTL_IO_NVME;
	ref_set_io_type(rio, admin ? REF_CTL_IO_NVME_ADMIN : REF_CTL_IO_NVME);
	pio.cmd.opc = opc;
	ref_set_opc(rio, opc);
	pio.cpl.status = status;
	ref_set_status(rio, status);

	++g_cases[fn];
	if (fn == FN_CMD_STR) {
		P::ctl_nvme_command_string(&pio, &psb);
		ref_ctl_nvme_command_string(&rio, &rsb);
	} else {
		P::ctl_nvme_status_string(&pio, &psb);
		ref_ctl_nvme_status_string(&rio, &rsb);
	}
	compare_sbufs(psb.s_buf, psb.s_len, rsb.s_buf, rsb.s_len, fn);
	if (std::memcmp(pscratch, rscratch, sizeof(pscratch)) != 0)
		fail_row(fn, "guard mismatch");
}

static void test_nvme_strings()
{
	const uint8_t opcs[] = {0, 1, 2, 5, 6, 0x06, 0x7f, 0x80, 0xff};
	const uint16_t statuses[] = {
		0x0000,
		0x0001,
		(0 << 9) | (1 << 1),
		(1 << 9) | (2 << 1),
		(2 << 9) | (0x80 << 1),
		(3 << 9) | (0x60 << 1),
		(7 << 9) | (3 << 1),
		(4 << 9) | (1 << 1),
		(0 << 9) | (0 << 1) | (1 << 14),
		(0 << 9) | (0 << 1) | (1 << 15),
		(0 << 9) | (0 << 1) | (1 << 14) | (1 << 15),
		0xffff,
	};
	for (bool admin : std::initializer_list<bool>{false, true}) {
		for (uint8_t opc : opcs)
			run_nvme_string_case(FN_CMD_STR, admin, opc, 0);
	}
	for (uint16_t st : statuses)
		run_nvme_string_case(FN_STS_STR, false, 0, st);
}

struct BePool {
	static constexpr int N = 8;
	P::ctl_backend_driver port[N];
	ref_ctl_backend_driver refb[N];
};

static void reset_backends()
{
	P::reset_backend_state();
	ref_reset_backend_state();
}

static void test_backend_register(const char *name, int (*init)(), int expect)
{
	BePool pool{};
	reset_backends();
	std::snprintf(pool.port[0].name, sizeof(pool.port[0].name), "%s", name);
	std::snprintf(pool.refb[0].name, sizeof(pool.refb[0].name), "%s", name);
	pool.port[0].init = init;
	pool.refb[0].init = init;
	++g_cases[FN_REG];
	const int pr = P::ctl_backend_register(&pool.port[0]);
	const int rr = ref_ctl_backend_register(&pool.refb[0]);
	if (pr != rr)
		fail_row(FN_REG, "return mismatch");
	if (pr != expect)
		fail_row(FN_REG, "unexpected return");
	if (pr == 0 && ref_num_backends() != 1)
		fail_row(FN_REG, "count mismatch");
}

static void test_backend_deregister(const char *name, int (*shutdown)(), int expect)
{
	BePool pool{};
	reset_backends();
	std::snprintf(pool.port[0].name, sizeof(pool.port[0].name), "%s", name);
	std::snprintf(pool.refb[0].name, sizeof(pool.refb[0].name), "%s", name);
	pool.port[0].shutdown = shutdown;
	pool.refb[0].shutdown = shutdown;
	P::ctl_backend_register(&pool.port[0]);
	ref_ctl_backend_register(&pool.refb[0]);
	++g_cases[FN_DEREG];
	const int pr = P::ctl_backend_deregister(&pool.port[0]);
	const int rr = ref_ctl_backend_deregister(&pool.refb[0]);
	if (pr != rr)
		fail_row(FN_DEREG, "return mismatch");
	if (pr != expect)
		fail_row(FN_DEREG, "unexpected return");
}

static void test_backend_find(const char *reg, const char *query, bool expect_hit)
{
	BePool pool{};
	reset_backends();
	std::snprintf(pool.port[0].name, sizeof(pool.port[0].name), "%s", reg);
	std::snprintf(pool.refb[0].name, sizeof(pool.refb[0].name), "%s", reg);
	P::ctl_backend_register(&pool.port[0]);
	ref_ctl_backend_register(&pool.refb[0]);
	char qbuf[32];
	std::snprintf(qbuf, sizeof(qbuf), "%s", query);
	++g_cases[FN_FIND];
	P::ctl_backend_driver *pf = P::ctl_backend_find(qbuf);
	ref_ctl_backend_driver *rf = ref_ctl_backend_find(qbuf);
	const bool phit = pf != nullptr;
	const bool rhit = rf != nullptr;
	if (phit != rhit)
		fail_row(FN_FIND, "null mismatch");
	if (expect_hit) {
		if (!phit)
			fail_row(FN_FIND, "expected hit");
		else {
			const std::ptrdiff_t po = pf - &pool.port[0];
			const std::ptrdiff_t ro = rf - &pool.refb[0];
			if (po != ro)
				fail_row(FN_FIND, "offset mismatch");
		}
	} else if (phit)
		fail_row(FN_FIND, "unexpected hit");
}

static void test_backends_hand()
{
	test_backend_register("ramdisk", nullptr, 0);
	test_backend_register("ramdisk", stub_init_ok, -1);
	test_backend_register("block", stub_init_ok, 0);
	test_backend_register("failbe", stub_init_fail, 42);
	test_backend_deregister("block", nullptr, 0);
	test_backend_deregister("block", stub_shutdown_fail, 17);
	test_backend_find("alpha", "alpha", true);
	test_backend_find("alpha", "beta", false);
	test_backend_find("alpha", "", false);
}

static unsigned rng_state = 0x12345678u;
static unsigned rng_next()
{
	rng_state = rng_state * 1103515245u + 12345u;
	return rng_state;
}

static void test_backends_random(unsigned long n)
{
	BePool pool{};
	for (unsigned long i = 0; i < n; ++i) {
		reset_backends();
		const int which = (int)(rng_next() % BePool::N);
		char name[32];
		std::snprintf(name, sizeof(name), "be%lu_%u", i, rng_next() & 0xff);
		std::snprintf(pool.port[which].name, sizeof(pool.port[which].name), "%s", name);
		std::snprintf(pool.refb[which].name, sizeof(pool.refb[which].name), "%s", name);
		const int use_init = (int)(rng_next() & 3u);
		if (use_init == 1) {
			pool.port[which].init = stub_init_ok;
			pool.refb[which].init = stub_init_ok;
		} else if (use_init == 2) {
			pool.port[which].init = stub_init_fail;
			pool.refb[which].init = stub_init_fail;
		}
		++g_cases[FN_REG];
		const int pr = P::ctl_backend_register(&pool.port[which]);
		const int rr = ref_ctl_backend_register(&pool.refb[which]);
		if (pr != rr)
			fail_row(FN_REG, "rand return");
		if (pr == 0) {
			++g_cases[FN_FIND];
			P::ctl_backend_driver *pf = P::ctl_backend_find(name);
			ref_ctl_backend_driver *rf = ref_ctl_backend_find(name);
			if ((pf == nullptr) != (rf == nullptr))
				fail_row(FN_FIND, "rand find null");
			if (pf && (pf - &pool.port[which] != rf - &pool.refb[which]))
				fail_row(FN_FIND, "rand find offset");
			++g_cases[FN_DEREG];
			const int pd = P::ctl_backend_deregister(&pool.port[which]);
			const int rd = ref_ctl_backend_deregister(&pool.refb[which]);
			if (pd != rd)
				fail_row(FN_DEREG, "rand dereg");
		}
	}
}

static void test_nvme_strings_random(unsigned long n)
{
	for (unsigned long i = 0; i < n; ++i) {
		const bool admin = (rng_next() & 1u) != 0;
		const uint8_t opc = (uint8_t)(rng_next() & 0xffu);
		run_nvme_string_case(FN_CMD_STR, admin, opc, 0);
		const uint16_t status = (uint16_t)(rng_next() & 0xffffu);
		run_nvme_string_case(FN_STS_STR, false, 0, status);
	}
}

int main()
{
	test_serialize_table();
	test_cmd_table(FN_ADMIN_TBL, P::nvme_admin_cmd_table, nvme_admin_cmd_table);
	test_cmd_table(FN_NVM_TBL, P::nvme_nvm_cmd_table, nvme_nvm_cmd_table);
	test_nvme_strings();
	test_backends_hand();
	test_nvme_strings_random(100000);
	test_backends_random(100000);

	std::printf("%-28s %10s %10s\n", "function", "cases", "failures");
	unsigned long tc = 0, tf = 0;
	for (int i = 0; i < FN_COUNT; ++i) {
		std::printf("%-28s %10lu %10lu\n", kNames[i], g_cases[i], g_fails[i]);
		tc += g_cases[i];
		tf += g_fails[i];
	}
	std::printf("%-28s %10lu %10lu\n", "TOTAL", tc, tf);
	return tf == 0 ? 0 : 1;
}
