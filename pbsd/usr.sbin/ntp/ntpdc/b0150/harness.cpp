/*
 * Differential harness for batch b0150 (ntpdc nl.c layout printer).
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

import pbsd.usr.sbin.ntp.ntpdc.b0150;

namespace P = pbsd::usr_sbin_ntp_ntpdc::b0150;

extern "C" int ref_nl(void);

static constexpr long long SWEEP = 200000;
static constexpr std::size_t OUT_CAP = 65536;

struct stat_row {
	const char *name;
	long long cases;
	long long failures;
	long long printed;
};

static stat_row row = { "nl", 0, 0, 0 };

static int saved_stdout = -1;

static void
save_stdout_once(void)
{
	if (saved_stdout < 0)
		saved_stdout = dup(STDOUT_FILENO);
}

static void
restore_stdout(void)
{
	if (saved_stdout < 0)
		return;
	std::fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	(void)std::freopen(nullptr, "w", stdout);
}

static int
capture_nl_output(char *buf, std::size_t cap, int (*fn)(void))
{
	char path[] = "/tmp/pbsd_b0150_XXXXXX";
	int fd;
	FILE *fp;
	std::size_t n;

	save_stdout_once();
	restore_stdout();

	fd = mkstemp(path);
	if (fd < 0)
		return -1;
	close(fd);

	fp = std::fopen(path, "w+");
	if (fp == nullptr) {
		std::remove(path);
		return -1;
	}

	std::fflush(stdout);
	if (dup2(fileno(fp), STDOUT_FILENO) < 0) {
		std::fclose(fp);
		std::remove(path);
		return -1;
	}
	(void)std::freopen(nullptr, "w", stdout);

	(void)fn();

	std::fflush(stdout);
	restore_stdout();

	rewind(fp);
	n = std::fread(buf, 1, cap - 1, fp);
	buf[n] = '\0';
	std::fclose(fp);
	std::remove(path);
	return (int)n;
}

static void
fail_case(const char *tag, const char *detail)
{
	row.failures++;
	if (row.printed < 12) {
		row.printed++;
		std::printf("  FAIL %-8s %-20s %s\n", row.name, tag, detail);
	}
}

static void
check_nl_pair(const char *tag)
{
	char ref_out[OUT_CAP];
	char port_out[OUT_CAP];
	int ref_len;
	int port_len;
	int ref_ret;
	int port_ret;

	row.cases++;

	ref_ret = capture_nl_output(ref_out, sizeof(ref_out), ref_nl);
	if (ref_ret < 0) {
		fail_case(tag, "ref capture failed");
		return;
	}

	port_ret = capture_nl_output(port_out, sizeof(port_out), []() {
		return P::nl();
	});
	if (port_ret < 0) {
		fail_case(tag, "port capture failed");
		return;
	}

	if (ref_ret != port_ret) {
		fail_case(tag, "output length mismatch");
		return;
	}
	if (std::memcmp(ref_out, port_out, (std::size_t)ref_ret) != 0) {
		std::size_t i;

		for (i = 0; i < (std::size_t)ref_ret; i++) {
			if (ref_out[i] != port_out[i])
				break;
		}
		fail_case(tag, "output byte mismatch");
		if (row.printed <= 12) {
			std::printf("    first diff at byte %zu: ref=0x%02x port=0x%02x\n",
			    i, (unsigned char)ref_out[i],
			    (unsigned char)port_out[i]);
		}
		return;
	}
}

static void
run_hand_cases(void)
{
	check_nl_pair("baseline");
	check_nl_pair("repeat");
	check_nl_pair("third_call");

	/* stdout state edge: ensure buffered then flushed between calls */
	for (int i = 0; i < 8; i++) {
		char tag[32];

		std::snprintf(tag, sizeof(tag), "burst_%d", i);
		check_nl_pair(tag);
	}
}

static uint64_t rng = 0xB0150FACEULL;

static uint64_t
rnd(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (rng);
}

static void
run_sweep(void)
{
	for (long long i = 0; i < SWEEP; i++) {
		char tag[48];

		(void)rnd();
		std::snprintf(tag, sizeof(tag), "sweep_%lld", i);
		check_nl_pair(tag);
	}
}

int
main(void)
{
	run_hand_cases();
	run_sweep();

	std::printf("\n%-24s %8lld %8lld\n", row.name, row.cases, row.failures);
	std::printf("%-24s %8s %8s\n", "function", "cases", "failures");
	std::printf("%-24s %8lld %8lld\n", row.name, row.cases, row.failures);

	return (row.failures == 0 ? 0 : 1);
}
