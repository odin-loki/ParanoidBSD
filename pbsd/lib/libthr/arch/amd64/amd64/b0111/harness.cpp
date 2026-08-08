/*
 * harness.cpp -- differential test for PBSD batch b0111 (__thr_setup_tsd).
 */

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef P_OSREL_TLSBASE
#define	P_OSREL_TLSBASE			1500044
#endif

#ifndef THR_C_RUNTIME
#define	THR_C_RUNTIME			0x0004
#endif

#ifndef AMD64_GET_TLSBASE
#define	AMD64_GET_TLSBASE		135
#endif

extern "C" {

struct tcb {
	unsigned char	opaque[64];
};

struct pthread {
	struct tcb	*tcb;
};

void ref___thr_setup_tsd(struct pthread *thread);

int __thr_new_flags;

}

import pbsd.lib.libthr.arch.amd64.amd64.b0111;

namespace port = pbsd::lib_libthr_arch_amd64_amd64::b0111;

struct MockConfig {
	int	osreldate;
	int	sysarch_ret;
	int	sysarch_errno;
	void	*sysarch_base_out;
};

struct MockRecord {
	int	sysarch_calls;
	int	last_sysarch_op;
	void	*last_sysarch_arg;
	void	*sysarch_base_written;
	int	tlsbase_calls;
	int	fsbase_calls;
	void	*last_tlsbase_tcb;
	void	*last_fsbase_tcb;
};

static MockConfig	mock_cfg;
static MockRecord	mock_rec;

static uint32_t
prng_next(uint32_t *state)
{
	*state = *state * 1664525u + 1013904223u;
	return (*state);
}

extern "C" int
__getosreldate(void)
{
	return (mock_cfg.osreldate);
}

extern "C" int
sysarch(int op, void *arg)
{
	mock_rec.sysarch_calls++;
	mock_rec.last_sysarch_op = op;
	mock_rec.last_sysarch_arg = arg;
	if (op == AMD64_GET_TLSBASE && arg != nullptr) {
		*(void **)arg = mock_cfg.sysarch_base_out;
		mock_rec.sysarch_base_written = mock_cfg.sysarch_base_out;
	}
	if (mock_cfg.sysarch_ret != 0)
		errno = mock_cfg.sysarch_errno;
	return (mock_cfg.sysarch_ret);
}

extern "C" void
amd64_set_tlsbase(void *tcb)
{
	mock_rec.tlsbase_calls++;
	mock_rec.last_tlsbase_tcb = tcb;
}

extern "C" void
amd64_set_fsbase(void *tcb)
{
	mock_rec.fsbase_calls++;
	mock_rec.last_fsbase_tcb = tcb;
}

struct Outcome {
	MockRecord	rec;
	int		flags_after;
};

static void
mock_rec_reset(void)
{
	memset(&mock_rec, 0, sizeof(mock_rec));
}

static Outcome
run_ref(struct pthread *thread, int initial_flags)
{
	Outcome out;

	mock_rec_reset();
	__thr_new_flags = initial_flags;
	ref___thr_setup_tsd(thread);
	out.rec = mock_rec;
	out.flags_after = __thr_new_flags;
	return (out);
}

static Outcome
run_port(struct pthread *thread, int initial_flags)
{
	Outcome out;

	mock_rec_reset();
	__thr_new_flags = initial_flags;
	port::__thr_setup_tsd(reinterpret_cast<port::pthread *>(thread));
	out.rec = mock_rec;
	out.flags_after = __thr_new_flags;
	return (out);
}

static int
outcomes_equal(const Outcome *a, const Outcome *b)
{
	if (a->rec.sysarch_calls != b->rec.sysarch_calls)
		return (0);
	if (a->rec.last_sysarch_op != b->rec.last_sysarch_op)
		return (0);
	if (a->rec.last_sysarch_arg != b->rec.last_sysarch_arg)
		return (0);
	if (a->rec.sysarch_base_written != b->rec.sysarch_base_written)
		return (0);
	if (a->rec.tlsbase_calls != b->rec.tlsbase_calls)
		return (0);
	if (a->rec.fsbase_calls != b->rec.fsbase_calls)
		return (0);
	if (a->rec.last_tlsbase_tcb != b->rec.last_tlsbase_tcb)
		return (0);
	if (a->rec.last_fsbase_tcb != b->rec.last_fsbase_tcb)
		return (0);
	if (a->flags_after != b->flags_after)
		return (0);
	return (1);
}

struct CaseStats {
	unsigned long	cases;
	unsigned long	failures;
};

static int
check_case(struct CaseStats *stats, struct pthread *thread, int initial_flags,
    const char *label)
{
	Outcome ref_out, port_out;

	(void)label;

	stats->cases++;
	ref_out = run_ref(thread, initial_flags);
	port_out = run_port(thread, initial_flags);
	if (!outcomes_equal(&ref_out, &port_out)) {
		stats->failures++;
		return (0);
	}
	return (1);
}

static void
configure_mock(int osreldate, int sysarch_ret, int sysarch_errno, void *base_out)
{
	mock_cfg.osreldate = osreldate;
	mock_cfg.sysarch_ret = sysarch_ret;
	mock_cfg.sysarch_errno = sysarch_errno;
	mock_cfg.sysarch_base_out = base_out;
}

static void
run_hand_cases(struct CaseStats *stats, struct tcb *tcbs, struct pthread *threads)
{
	struct pthread *thr;

	configure_mock(P_OSREL_TLSBASE - 1, 0, 0, nullptr);
	thr = &threads[0];
	thr->tcb = &tcbs[0];
	check_case(stats, thr, THR_C_RUNTIME, "old_osrel_tlsbase");

	configure_mock(P_OSREL_TLSBASE - 2, -1, ESRCH, nullptr);
	thr = &threads[1];
	thr->tcb = &tcbs[1];
	check_case(stats, thr, 0xffff, "old_osrel_ignores_sysarch");

	configure_mock(0, 0, 0, nullptr);
	thr = &threads[2];
	thr->tcb = nullptr;
	check_case(stats, thr, THR_C_RUNTIME, "old_osrel_null_tcb");

	configure_mock(P_OSREL_TLSBASE, 0, ESRCH, &tcbs[3]);
	thr = &threads[3];
	thr->tcb = &tcbs[3];
	check_case(stats, thr, THR_C_RUNTIME, "boundary_osrel_sysarch_ok");

	configure_mock(P_OSREL_TLSBASE + 1, 0, ESRCH, &tcbs[4]);
	thr = &threads[4];
	thr->tcb = &tcbs[4];
	check_case(stats, thr, 0, "new_osrel_sysarch_ok");

	configure_mock(P_OSREL_TLSBASE, -1, ESRCH, &tcbs[5]);
	thr = &threads[5];
	thr->tcb = &tcbs[5];
	check_case(stats, thr, THR_C_RUNTIME, "esrch_clears_flag");

	configure_mock(P_OSREL_TLSBASE, 1, ESRCH, &tcbs[6]);
	thr = &threads[6];
	thr->tcb = &tcbs[6];
	check_case(stats, thr, THR_C_RUNTIME | 0x00f0, "esrch_clears_only_runtime");

	configure_mock(P_OSREL_TLSBASE, -1, EINVAL, &tcbs[7]);
	thr = &threads[7];
	thr->tcb = &tcbs[7];
	check_case(stats, thr, THR_C_RUNTIME, "error_not_esrch");

	configure_mock(P_OSREL_TLSBASE, 0, ESRCH, &tcbs[8]);
	thr = &threads[8];
	thr->tcb = &tcbs[8];
	check_case(stats, thr, THR_C_RUNTIME, "success_ignores_errno");

	configure_mock(P_OSREL_TLSBASE, 1, 0, &tcbs[9]);
	thr = &threads[9];
	thr->tcb = &tcbs[9];
	check_case(stats, thr, 0x1234, "error_zero_errno");

	configure_mock(P_OSREL_TLSBASE, -1, ESRCH, nullptr);
	thr = &threads[10];
	thr->tcb = &tcbs[10];
	check_case(stats, thr, 0, "esrch_null_base_out");

	configure_mock(INT_MAX, -1, ESRCH, &tcbs[11]);
	thr = &threads[11];
	thr->tcb = &tcbs[11];
	check_case(stats, thr, THR_C_RUNTIME, "max_osrel_esrch");

	configure_mock(P_OSREL_TLSBASE, -1, ESRCH, (void *)0xdeadbeefUL);
	thr = &threads[12];
	thr->tcb = (struct tcb *)(uintptr_t)0xfeedfaceUL;
	check_case(stats, thr, ~0, "odd_tcb_pointers");

	configure_mock(P_OSREL_TLSBASE - 1, -1, ESRCH, &tcbs[13]);
	thr = &threads[13];
	thr->tcb = &tcbs[13];
	check_case(stats, thr, THR_C_RUNTIME, "old_osrel_high_flags");

	configure_mock(P_OSREL_TLSBASE, 2, ESRCH, &tcbs[14]);
	thr = &threads[14];
	thr->tcb = &tcbs[14];
	check_case(stats, thr, THR_C_RUNTIME, "positive_error_esrch");

	configure_mock(P_OSREL_TLSBASE, -1, ESRCH, &tcbs[15]);
	thr = &threads[15];
	thr->tcb = &tcbs[15];
	check_case(stats, thr, THR_C_RUNTIME, "negative_error_esrch");
}

static void
run_random_cases(struct CaseStats *stats, struct tcb *tcbs, struct pthread *threads,
    unsigned long iterations, uint32_t seed)
{
	for (unsigned long i = 0; i < iterations; i++) {
		uint32_t r0 = prng_next(&seed);
		uint32_t r1 = prng_next(&seed);
		uint32_t r2 = prng_next(&seed);
		uint32_t r3 = prng_next(&seed);
		uint32_t r4 = prng_next(&seed);
		int osreldate;
		int sysarch_ret;
		int sysarch_errno;
		int initial_flags;
		unsigned tcb_idx;
		struct pthread *thr;

		if ((r0 & 7u) == 0u)
			osreldate = (int)(P_OSREL_TLSBASE - 1 - (r1 & 0xffu));
		else if ((r0 & 7u) == 1u)
			osreldate = (int)(P_OSREL_TLSBASE + (r1 & 0xffu));
		else
			osreldate = (int)r1;

		if ((r2 & 3u) == 0u) {
			sysarch_ret = 0;
			sysarch_errno = (int)(r3 & 0xffu);
		} else if ((r2 & 3u) == 1u) {
			sysarch_ret = (int)((r3 & 1u) ? 1 : -1);
			sysarch_errno = ESRCH;
		} else {
			sysarch_ret = (int)((r3 & 1u) ? 1 : -1);
			sysarch_errno = (int)(r3 & 0x7fu);
			if (sysarch_errno == ESRCH)
				sysarch_errno = EINVAL;
		}

		initial_flags = (int)r4;
		tcb_idx = (unsigned)(r3 % 256u);
		thr = &threads[tcb_idx];
		thr->tcb = &tcbs[tcb_idx];

		configure_mock(osreldate, sysarch_ret, sysarch_errno,
		    (r4 & 1u) ? &tcbs[tcb_idx] : nullptr);
		check_case(stats, thr, initial_flags, "random");
	}
}

int
main(void)
{
	struct tcb		tcbs[256];
	struct pthread		threads[256];
	struct CaseStats	stats = { 0, 0 };
	int			rc = 0;

	memset(tcbs, 0x7f, sizeof(tcbs));
	memset(threads, 0, sizeof(threads));

	run_hand_cases(&stats, tcbs, threads);
	run_random_cases(&stats, tcbs, threads, 200000u, 0xb0111u);

	printf("Function            Cases    Failures\n");
	printf("------------------  -------  --------\n");
	printf("__thr_setup_tsd     %-7lu  %-8lu\n",
	    stats.cases, stats.failures);

	if (stats.failures != 0)
		rc = 1;
	return (rc);
}
