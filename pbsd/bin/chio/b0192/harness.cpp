/*
 * harness.cpp -- differential test for PBSD batch b0192 (chio.c helpers).
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <setjmp.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.chio.b0192;

namespace P = pbsd::bin_chio::b0192;

extern "C" {
extern jmp_buf oracle_err_jmp;
extern int oracle_err_called;
extern int oracle_err_status;
void oracle_err_arm(void);
void oracle_err_disarm(void);
u_int16_t ref_parse_element_type(char *cp);
const char *ref_element_type_name(int et);
u_int16_t ref_parse_element_unit(char *cp);
int ref_parse_special(char *cp);
int ref_is_special(char *cp);
const char *ref_bits_to_string(P::ces_status_flags v, const char *cp);
}

#define SWEEP 200000L
#define MAX_SHOW 8

#define	CESTATUS_BITS	\
	"\20\6INENAB\5EXENAB\4ACCESS\3EXCEPT\2IMPEXP\1FULL"

namespace {

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

Stat st_parse_element_type = { "parse_element_type", 0, 0, 0 };
Stat st_element_type_name = { "element_type_name", 0, 0, 0 };
Stat st_parse_element_unit = { "parse_element_unit", 0, 0, 0 };
Stat st_parse_special = { "parse_special", 0, 0, 0 };
Stat st_is_special = { "is_special", 0, 0, 0 };
Stat st_bits_to_string = { "bits_to_string", 0, 0, 0 };

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
};

Rng rng(0x00b0192faceULL);

bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

int
silence_stderr()
{
	int saved = dup(STDERR_FILENO);
	int devnull = open("/dev/null", O_WRONLY);
	if (devnull >= 0) {
		dup2(devnull, STDERR_FILENO);
		close(devnull);
	}
	return saved;
}

void
restore_stderr(int saved)
{
	if (saved >= 0) {
		dup2(saved, STDERR_FILENO);
		close(saved);
	}
}

struct ErrxResult {
	bool threw;
	int status;
	u_int16_t value;
};

ErrxResult
run_ref_parse_element_type(char *cp)
{
	ErrxResult res{};

	int saved = silence_stderr();
	oracle_err_called = 0;
	oracle_err_status = 0;
	oracle_err_arm();
	if (setjmp(oracle_err_jmp) == 0) {
		res.value = ref_parse_element_type(cp);
		oracle_err_disarm();
		res.threw = false;
	} else {
		oracle_err_disarm();
		res.threw = true;
		res.status = oracle_err_status;
	}
	restore_stderr(saved);
	return res;
}

ErrxResult
run_port_parse_element_type(char *cp)
{
	ErrxResult res{};

	int saved = silence_stderr();
	P::port_err_called = 0;
	P::port_err_status = 0;
	P::port_err_arm();
	if (setjmp(P::port_err_jmp) == 0) {
		res.value = P::parse_element_type(cp);
		P::port_err_disarm();
		res.threw = false;
	} else {
		P::port_err_disarm();
		res.threw = true;
		res.status = P::port_err_status;
	}
	restore_stderr(saved);
	return res;
}

bool
run_parse_element_type_case(const char *label, char *cp)
{
	st_parse_element_type.cases++;

	ErrxResult ref = run_ref_parse_element_type(cp);
	ErrxResult port = run_port_parse_element_type(cp);

	if (ref.threw != port.threw) {
		std::printf("    %s: threw %d vs %d arg=%s\n", label, ref.threw,
		    port.threw, cp);
		return fail(st_parse_element_type, label);
	}
	if (ref.threw) {
		if (ref.status != port.status) {
			std::printf("    %s: status %d vs %d\n", label,
			    ref.status, port.status);
			return fail(st_parse_element_type, label);
		}
		return true;
	}
	if (ref.value != port.value) {
		std::printf("    %s: value %u vs %u arg=%s\n", label,
		    (unsigned)ref.value, (unsigned)port.value, cp);
		return fail(st_parse_element_type, label);
	}
	return true;
}

void
test_parse_element_type_hand()
{
	static char drive[] = "drive";
	static char picker[] = "picker";
	static char portal[] = "portal";
	static char slot[] = "slot";
	static char voltag[] = "voltag";
	static char empty[] = "";
	static char bad[] = "bad";
	static char driveX[] = "driveX";
	static char Drive[] = "Drive";
	static char prefix[] = "driveextra";
	static char hi[] = {'\x80', 'd', 'r', 'i', 'v', 'e', '\0'};
	static char nul[] = "d\0rive";
	static char space[] = " drive";
	static char tab[] = "\tdrive";
	static char partial[] = "driv";
	static char abbrev[] = "dri";
	static char vol[] = "volt";
	static char picker0[] = "picker0";
	static char ff[] = "\xff";
	static char sevenf[] = "\x7f";
	static char mixed[] = "slot\x80";

	run_parse_element_type_case("drive", drive);
	run_parse_element_type_case("picker", picker);
	run_parse_element_type_case("portal", portal);
	run_parse_element_type_case("slot", slot);
	run_parse_element_type_case("voltag", voltag);
	run_parse_element_type_case("empty", empty);
	run_parse_element_type_case("bad", bad);
	run_parse_element_type_case("driveX", driveX);
	run_parse_element_type_case("Drive", Drive);
	run_parse_element_type_case("prefix", prefix);
	run_parse_element_type_case("hi", hi);
	run_parse_element_type_case("nul", nul);
	run_parse_element_type_case("space", space);
	run_parse_element_type_case("tab", tab);
	run_parse_element_type_case("partial", partial);
	run_parse_element_type_case("abbrev", abbrev);
	run_parse_element_type_case("vol", vol);
	run_parse_element_type_case("picker0", picker0);
	run_parse_element_type_case("ff", ff);
	run_parse_element_type_case("sevenf", sevenf);
	run_parse_element_type_case("mixed", mixed);
}

std::vector<char>
rnd_cstring(std::size_t max_len)
{
	std::size_t len = static_cast<std::size_t>(rng.next() % (max_len + 1));
	std::vector<char> s(len + 1, '\0');
	for (std::size_t i = 0; i < len; i++) {
		s[i] = static_cast<char>(rng.next() & 0xffu);
	}
	return s;
}

void
test_parse_element_type_sweep()
{
	static const char *known[] = {
		"drive", "picker", "portal", "slot", "voltag"
	};

	for (long i = 0; i < SWEEP; i++) {
		if ((rng.next() & 0x1fu) == 0u) {
			const char *k = known[rng.next() % 5u];
			run_parse_element_type_case("sweep", const_cast<char *>(k));
			continue;
		}
		std::vector<char> s = rnd_cstring(32);
		run_parse_element_type_case("sweep", s.data());
	}
}

bool
run_element_type_name_case(const char *label, int et)
{
	st_element_type_name.cases++;

	const char *ref = ref_element_type_name(et);
	const char *port = P::element_type_name(et);

	if (std::strcmp(ref, port) != 0) {
		std::printf("    %s: et=%d ref=%s port=%s\n", label, et, ref,
		    port);
		return fail(st_element_type_name, label);
	}
	return true;
}

void
test_element_type_name_hand()
{
	run_element_type_name_case("drive", 3);
	run_element_type_name_case("picker", 0);
	run_element_type_name_case("portal", 2);
	run_element_type_name_case("slot", 1);
	run_element_type_name_case("voltag", 10);
	run_element_type_name_case("unknown_neg", -1);
	run_element_type_name_case("unknown_big", 999);
	run_element_type_name_case("unknown_4", 4);
	run_element_type_name_case("unknown_9", 9);
	run_element_type_name_case("zero", 0);
	run_element_type_name_case("hi_7f", 0x7f);
	run_element_type_name_case("hi_80", 0x80);
	run_element_type_name_case("hi_ff", 0xff);
	run_element_type_name_case("max_int", 0x7fffffff);
	run_element_type_name_case("min_int", -0x80000000);
}

void
test_element_type_name_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		int et;
		if ((rng.next() & 7u) == 0u) {
			et = static_cast<int>(rng.next() % 11u);
		} else {
			et = static_cast<int>(rng.next() & 0xffffffffu);
			if ((rng.next() & 1u) != 0u) {
				et = -et;
			}
		}
		run_element_type_name_case("sweep", et);
	}
}

ErrxResult
run_ref_parse_element_unit(char *cp)
{
	ErrxResult res{};

	int saved = silence_stderr();
	oracle_err_called = 0;
	oracle_err_status = 0;
	oracle_err_arm();
	if (setjmp(oracle_err_jmp) == 0) {
		res.value = ref_parse_element_unit(cp);
		oracle_err_disarm();
		res.threw = false;
	} else {
		oracle_err_disarm();
		res.threw = true;
		res.status = oracle_err_status;
	}
	restore_stderr(saved);
	return res;
}

ErrxResult
run_port_parse_element_unit(char *cp)
{
	ErrxResult res{};

	int saved = silence_stderr();
	P::port_err_called = 0;
	P::port_err_status = 0;
	P::port_err_arm();
	if (setjmp(P::port_err_jmp) == 0) {
		res.value = P::parse_element_unit(cp);
		P::port_err_disarm();
		res.threw = false;
	} else {
		P::port_err_disarm();
		res.threw = true;
		res.status = P::port_err_status;
	}
	restore_stderr(saved);
	return res;
}

bool
run_parse_element_unit_case(const char *label, char *cp)
{
	st_parse_element_unit.cases++;

	ErrxResult ref = run_ref_parse_element_unit(cp);
	ErrxResult port = run_port_parse_element_unit(cp);

	if (ref.threw != port.threw) {
		std::printf("    %s: threw %d vs %d arg=%s\n", label, ref.threw,
		    port.threw, cp);
		return fail(st_parse_element_unit, label);
	}
	if (ref.threw) {
		if (ref.status != port.status) {
			std::printf("    %s: status %d vs %d\n", label,
			    ref.status, port.status);
			return fail(st_parse_element_unit, label);
		}
		return true;
	}
	if (ref.value != port.value) {
		std::printf("    %s: value %u vs %u arg=%s\n", label,
		    (unsigned)ref.value, (unsigned)port.value, cp);
		return fail(st_parse_element_unit, label);
	}
	return true;
}

void
test_parse_element_unit_hand()
{
	static char zero[] = "0";
	static char one[] = "1";
	static char max16[] = "65535";
	static char neg[] = "-1";
	static char bad[] = "abc";
	static char trail[] = "12x";
	static char space[] = " 12";
	static char plus[] = "+5";
	static char empty[] = "";
	static char hi[] = {'\x80', '1', '\0'};
	static char nul[] = "1\02";
	static char huge[] = "99999999999999999999";
	static char leading_zero[] = "007";
	static char ff[] = "\xff";
	static char sevenf[] = {'\x7f', '0', '\0'};
	static char zero_neg[] = "-0";
	static char tab1[] = "\t1";
	static char sign_only[] = "-";
	static char dot[] = "1.5";
	static char hex[] = "0x10";
	static char oct[] = "010";
	static char boundary[] = "65536";

	run_parse_element_unit_case("zero", zero);
	run_parse_element_unit_case("one", one);
	run_parse_element_unit_case("max16", max16);
	run_parse_element_unit_case("neg", neg);
	run_parse_element_unit_case("bad", bad);
	run_parse_element_unit_case("trail", trail);
	run_parse_element_unit_case("space", space);
	run_parse_element_unit_case("plus", plus);
	run_parse_element_unit_case("empty", empty);
	run_parse_element_unit_case("hi", hi);
	run_parse_element_unit_case("nul", nul);
	run_parse_element_unit_case("huge", huge);
	run_parse_element_unit_case("leading_zero", leading_zero);
	run_parse_element_unit_case("ff", ff);
	run_parse_element_unit_case("sevenf", sevenf);
	run_parse_element_unit_case("zero_neg", zero_neg);
	run_parse_element_unit_case("tab1", tab1);
	run_parse_element_unit_case("sign_only", sign_only);
	run_parse_element_unit_case("dot", dot);
	run_parse_element_unit_case("hex", hex);
	run_parse_element_unit_case("oct", oct);
	run_parse_element_unit_case("boundary", boundary);
}

void
test_parse_element_unit_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		std::vector<char> s = rnd_cstring(24);
		run_parse_element_unit_case("sweep", s.data());
	}
}

bool
run_is_special_case(const char *label, char *cp)
{
	st_is_special.cases++;

	int ref = ref_is_special(cp);
	int port = P::is_special(cp);

	if (ref != port) {
		std::printf("    %s: %d vs %d arg=%s\n", label, ref, port,
		    cp);
		return fail(st_is_special, label);
	}
	return true;
}

void
test_is_special_hand()
{
	static char inv[] = "inv";
	static char inv1[] = "inv1";
	static char inv2[] = "inv2";
	static char bad[] = "invert";
	static char empty[] = "";
	static char Inv[] = "Inv";
	static char inv3[] = "inv3";
	static char prefix[] = "invx";
	static char hi[] = "\x80inv";
	static char nul[] = "inv\01";
	static char space[] = " inv";
	static char inv_space[] = "inv ";
	static char ff[] = "\xff";
	static char sevenf[] = "\x7f";

	run_is_special_case("inv", inv);
	run_is_special_case("inv1", inv1);
	run_is_special_case("inv2", inv2);
	run_is_special_case("bad", bad);
	run_is_special_case("empty", empty);
	run_is_special_case("Inv", Inv);
	run_is_special_case("inv3", inv3);
	run_is_special_case("prefix", prefix);
	run_is_special_case("hi", hi);
	run_is_special_case("nul", nul);
	run_is_special_case("space", space);
	run_is_special_case("inv_space", inv_space);
	run_is_special_case("ff", ff);
	run_is_special_case("sevenf", sevenf);
}

void
test_is_special_sweep()
{
	static const char *known[] = { "inv", "inv1", "inv2" };

	for (long i = 0; i < SWEEP; i++) {
		if ((rng.next() & 0x1fu) == 0u) {
			const char *k = known[rng.next() % 3u];
			run_is_special_case("sweep", const_cast<char *>(k));
			continue;
		}
		std::vector<char> s = rnd_cstring(16);
		run_is_special_case("sweep", s.data());
	}
}

struct IntResult {
	bool threw;
	int status;
	int value;
};

IntResult
run_ref_parse_special(char *cp)
{
	IntResult res{};

	int saved = silence_stderr();
	oracle_err_called = 0;
	oracle_err_status = 0;
	oracle_err_arm();
	if (setjmp(oracle_err_jmp) == 0) {
		res.value = ref_parse_special(cp);
		oracle_err_disarm();
		res.threw = false;
	} else {
		oracle_err_disarm();
		res.threw = true;
		res.status = oracle_err_status;
	}
	restore_stderr(saved);
	return res;
}

IntResult
run_port_parse_special(char *cp)
{
	IntResult res{};

	int saved = silence_stderr();
	P::port_err_called = 0;
	P::port_err_status = 0;
	P::port_err_arm();
	if (setjmp(P::port_err_jmp) == 0) {
		res.value = P::parse_special(cp);
		P::port_err_disarm();
		res.threw = false;
	} else {
		P::port_err_disarm();
		res.threw = true;
		res.status = P::port_err_status;
	}
	restore_stderr(saved);
	return res;
}

bool
run_parse_special_case(const char *label, char *cp)
{
	st_parse_special.cases++;

	IntResult ref = run_ref_parse_special(cp);
	IntResult port = run_port_parse_special(cp);

	if (ref.threw != port.threw) {
		std::printf("    %s: threw %d vs %d arg=%s\n", label, ref.threw,
		    port.threw, cp);
		return fail(st_parse_special, label);
	}
	if (ref.threw) {
		if (ref.status != port.status) {
			std::printf("    %s: status %d vs %d\n", label,
			    ref.status, port.status);
			return fail(st_parse_special, label);
		}
		return true;
	}
	if (ref.value != port.value) {
		std::printf("    %s: value %d vs %d arg=%s\n", label,
		    ref.value, port.value, cp);
		return fail(st_parse_special, label);
	}
	return true;
}

void
test_parse_special_hand()
{
	static char inv[] = "inv";
	static char inv1[] = "inv1";
	static char inv2[] = "inv2";
	static char bad[] = "invert";
	static char empty[] = "";
	static char hi[] = "\x80inv1";
	static char nul[] = "inv\02";
	static char ff[] = "\xff";

	run_parse_special_case("inv", inv);
	run_parse_special_case("inv1", inv1);
	run_parse_special_case("inv2", inv2);
	run_parse_special_case("bad", bad);
	run_parse_special_case("empty", empty);
	run_parse_special_case("hi", hi);
	run_parse_special_case("nul", nul);
	run_parse_special_case("ff", ff);
}

void
test_parse_special_sweep()
{
	static const char *known[] = { "inv", "inv1", "inv2" };

	for (long i = 0; i < SWEEP; i++) {
		if ((rng.next() & 0x1fu) == 0u) {
			const char *k = known[rng.next() % 3u];
			run_parse_special_case("sweep", const_cast<char *>(k));
			continue;
		}
		std::vector<char> s = rnd_cstring(16);
		run_parse_special_case("sweep", s.data());
	}
}

std::vector<char>
rnd_bitstring(std::size_t max_fields)
{
	std::size_t n = static_cast<std::size_t>(rng.next() % (max_fields + 1));
	std::vector<char> s;
	for (std::size_t i = 0; i < n; i++) {
		char code = static_cast<char>((rng.next() % 31u) + 1u);
		s.push_back(code);
		std::size_t nlen = static_cast<std::size_t>(rng.next() % 8u);
		for (std::size_t j = 0; j < nlen; j++) {
			unsigned char c = static_cast<unsigned char>(
			    rng.next() & 0xffu);
			if (c < ' ')
				c = static_cast<unsigned char>('A' + (c % 26));
			s.push_back(static_cast<char>(c));
		}
		if ((rng.next() & 3u) == 0u) {
			s.push_back(static_cast<char>(rng.next() & 0x1fu));
		}
	}
	s.push_back('\0');
	return s;
}

bool
run_bits_to_string_case(const char *label, P::ces_status_flags flags,
    const char *cp)
{
	st_bits_to_string.cases++;

	const char *ref = ref_bits_to_string(flags, cp);
	const char *port = P::bits_to_string(flags, cp);

	if (std::strcmp(ref, port) != 0) {
		std::printf("    %s: flags=0x%x ref=%s port=%s\n", label,
		    (unsigned)flags, ref, port);
		return fail(st_bits_to_string, label);
	}
	return true;
}

void
test_bits_to_string_hand()
{
	run_bits_to_string_case("none", (P::ces_status_flags)0, CESTATUS_BITS);
	run_bits_to_string_case("full",
	    (P::ces_status_flags)P::CES_STATUS_FULL, CESTATUS_BITS);
	run_bits_to_string_case("impexp",
	    (P::ces_status_flags)P::CES_STATUS_IMPEXP, CESTATUS_BITS);
	run_bits_to_string_case("except",
	    (P::ces_status_flags)P::CES_STATUS_EXCEPT, CESTATUS_BITS);
	run_bits_to_string_case("access",
	    (P::ces_status_flags)P::CES_STATUS_ACCESS, CESTATUS_BITS);
	run_bits_to_string_case("exenab",
	    (P::ces_status_flags)P::CES_STATUS_EXENAB, CESTATUS_BITS);
	run_bits_to_string_case("inenab",
	    (P::ces_status_flags)P::CES_STATUS_INENAB, CESTATUS_BITS);
	run_bits_to_string_case("invert",
	    (P::ces_status_flags)P::CES_INVERT, CESTATUS_BITS);
	run_bits_to_string_case("all_low",
	    (P::ces_status_flags)0x07f, CESTATUS_BITS);
	run_bits_to_string_case("all_flags",
	    (P::ces_status_flags)0x7ff, CESTATUS_BITS);
	run_bits_to_string_case("full_impexp",
	    (P::ces_status_flags)(P::CES_STATUS_FULL |
	    P::CES_STATUS_IMPEXP), CESTATUS_BITS);
	run_bits_to_string_case("access_except",
	    (P::ces_status_flags)(P::CES_STATUS_ACCESS |
	    P::CES_STATUS_EXCEPT), CESTATUS_BITS);

	static char empty_fmt[] = "";
	run_bits_to_string_case("empty_fmt",
	    (P::ces_status_flags)0xff, empty_fmt);

	static char one_bit[] = "\1A";
	run_bits_to_string_case("one_bit", (P::ces_status_flags)0x001,
	    one_bit);
	run_bits_to_string_case("one_bit_clear",
	    (P::ces_status_flags)0, one_bit);

	static char two_bit[] = "\2AB\1C";
	run_bits_to_string_case("two_bit_ab",
	    (P::ces_status_flags)0x002, two_bit);
	run_bits_to_string_case("two_bit_c",
	    (P::ces_status_flags)0x001, two_bit);
	run_bits_to_string_case("two_bit_both",
	    (P::ces_status_flags)0x003, two_bit);

	static char hi_code[] = "\x7fHI";
	run_bits_to_string_case("hi_code",
	    (P::ces_status_flags)0x40000000, hi_code);

	static char space_end[] = "\1ABC ";
	run_bits_to_string_case("space_end",
	    (P::ces_status_flags)0x001, space_end);
}

void
test_bits_to_string_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		P::ces_status_flags flags =
		    (P::ces_status_flags)(rng.next() & 0x7ffu);
		if ((rng.next() & 3u) == 0u) {
			flags = (P::ces_status_flags)(rng.next() & 0xffffffffu);
		}
		if ((rng.next() & 1u) != 0u) {
			run_bits_to_string_case("sweep", flags, CESTATUS_BITS);
		} else {
			std::vector<char> fmt = rnd_bitstring(8);
			run_bits_to_string_case("sweep", flags, fmt.data());
		}
	}
}

long
total_cases()
{
	return st_parse_element_type.cases + st_element_type_name.cases +
	    st_parse_element_unit.cases + st_parse_special.cases +
	    st_is_special.cases + st_bits_to_string.cases;
}

long
total_fails()
{
	return st_parse_element_type.fails + st_element_type_name.fails +
	    st_parse_element_unit.fails + st_parse_special.fails +
	    st_is_special.fails + st_bits_to_string.fails;
}

} // namespace

int
main()
{
	std::printf("PBSD batch b0192 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	test_parse_element_type_hand();
	test_parse_element_type_sweep();
	test_element_type_name_hand();
	test_element_type_name_sweep();
	test_parse_element_unit_hand();
	test_parse_element_unit_sweep();
	test_is_special_hand();
	test_is_special_sweep();
	test_parse_special_hand();
	test_parse_special_sweep();
	test_bits_to_string_hand();
	test_bits_to_string_sweep();

	Stat stats[] = {
		st_parse_element_type,
		st_element_type_name,
		st_parse_element_unit,
		st_parse_special,
		st_is_special,
		st_bits_to_string,
	};

	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");

	for (const Stat &st : stats) {
		std::printf("%-32s %12ld %12ld %10s\n", st.name, st.cases,
		    st.fails, st.fails == 0 ? "PASS" : "FAIL");
	}

	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", total_cases(),
	    total_fails(), total_fails() == 0 ? "PASS" : "FAIL");

	std::printf("\n%ld case(s) run, %ld failure(s).\n", total_cases(),
	    total_fails());

	return total_fails() == 0 ? 0 : 1;
}
