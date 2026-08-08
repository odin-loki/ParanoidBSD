/*
 * harness.cpp -- differential test for PBSD batch b0108.
 */

#define _GNU_SOURCE
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef UNMLEN
#define UNMLEN 32
#endif
#ifndef GNMLEN
#define GNMLEN 32
#endif

import pbsd.bin.pax.b0108;

namespace port = pbsd::bin_pax::b0108;

extern "C" {
int ref_tty_init(void);
void ref_tty_prnt(const char *, ...);
int ref_tty_read(char *, int);
void ref_paxwarn(int, const char *, ...);
void ref_syswarn(int, int, const char *, ...);
void ref_ls_list(port::ARCHD *, time_t, FILE *);
void ref_ls_tty(port::ARCHD *);
int ref_l_strncpy(char *, const char *, int);
unsigned long ref_asc_ul(char *, int, int);
int ref_ul_asc(unsigned long, char *, int, int);
unsigned long long ref_asc_uqd(char *, int, int);
int ref_uqd_asc(unsigned long long, char *, int, int);
int ref_uidtb_start(void);
int ref_gidtb_start(void);
int ref_usrtb_start(void);
int ref_grptb_start(void);
const char *ref_name_uid(uid_t, int);
const char *ref_name_gid(gid_t, int);
int ref_uid_name(char *, uid_t *);
int ref_gid_name(char *, gid_t *);

extern int iflag;
extern int vflag;
extern int vfpart;
extern int exit_val;
extern const char *argv0;
extern FILE *listf;
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr int MAX_REPORT = 8;
constexpr unsigned RAND_ITERS = 200000u;
constexpr int HEX = 16;
constexpr int OCT = 8;
constexpr std::size_t BUF_PRE = 16;
constexpr std::size_t BUF_CAP = 512;
constexpr std::size_t BUF_POST = 16;
constexpr std::size_t BUF_TOTAL = BUF_PRE + BUF_CAP + BUF_POST;

constexpr int PAX_CHR = 2;
constexpr int PAX_BLK = 3;
constexpr int PAX_REG = 4;
constexpr int PAX_SLK = 5;
constexpr int PAX_HLK = 8;
constexpr int PAX_HRG = 9;

enum StatId {
	S_TTY_INIT,
	S_TTY_PRNT,
	S_TTY_READ,
	S_PAXWARN,
	S_SYSWARN,
	S_LS_LIST,
	S_LS_TTY,
	S_L_STRNCPY,
	S_ASC_UL,
	S_UL_ASC,
	S_ASC_UQD,
	S_UQD_ASC,
	S_UIDTB_START,
	S_GIDTB_START,
	S_USRTB_START,
	S_GRPTB_START,
	S_NAME_UID,
	S_NAME_GID,
	S_UID_NAME,
	S_GID_NAME,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int reported;
};

Stats g_stat[NSTAT] = {
	{ "tty_init",      0, 0, 0 },
	{ "tty_prnt",      0, 0, 0 },
	{ "tty_read",      0, 0, 0 },
	{ "paxwarn",       0, 0, 0 },
	{ "syswarn",       0, 0, 0 },
	{ "ls_list",       0, 0, 0 },
	{ "ls_tty",        0, 0, 0 },
	{ "l_strncpy",     0, 0, 0 },
	{ "asc_ul",        0, 0, 0 },
	{ "ul_asc",        0, 0, 0 },
	{ "asc_uqd",       0, 0, 0 },
	{ "uqd_asc",       0, 0, 0 },
	{ "uidtb_start",   0, 0, 0 },
	{ "gidtb_start",   0, 0, 0 },
	{ "usrtb_start",   0, 0, 0 },
	{ "grptb_start",   0, 0, 0 },
	{ "name_uid",      0, 0, 0 },
	{ "name_gid",      0, 0, 0 },
	{ "uid_name",      0, 0, 0 },
	{ "gid_name",      0, 0, 0 },
};

std::uint64_t rng_state = 0xb0108facefeedULL;

std::uint64_t rnd_u64(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

unsigned rnd_u32(void) { return (unsigned)(rnd_u64() & 0xffffffffu); }
int rnd_i32(void) { return (int)rnd_u32(); }
unsigned char rnd_byte(void) { return (unsigned char)(rnd_u32() & 0xffu); }

void fail_msg(StatId which, const char *label, const char *detail)
{
	g_stat[which].fails++;
	if (g_stat[which].reported++ < MAX_REPORT)
		std::printf("  FAIL %-14s %-24s %s\n", g_stat[which].name,
		    label, detail);
}

void case_inc(StatId which) { g_stat[which].cases++; }

struct GuardedBuf { unsigned char data[BUF_TOTAL]; };

void guard_fill(GuardedBuf *g) { std::memset(g->data, GUARD, sizeof(g->data)); }
unsigned char *buf_user(GuardedBuf *g) { return g->data + BUF_PRE; }
bool guard_eq(const GuardedBuf *a, const GuardedBuf *b)
{
	return std::memcmp(a->data, b->data, sizeof(a->data)) == 0;
}

void fill_random_string(unsigned char *dst, int len, bool allow_nul)
{
	for (int i = 0; i < len; i++) {
		unsigned char c = rnd_byte();
		if (!allow_nul && c == '\0')
			c = (unsigned char)('a' + (c % 26));
		dst[i] = c;
	}
}

void test_tty_init_edge(void)
{
	int save = iflag;

	case_inc(S_TTY_INIT);
	iflag = 0;
	port::iflag = 0;
	if (ref_tty_init() != port::tty_init())
		fail_msg(S_TTY_INIT, "edge no-iflag", "return mismatch");

	case_inc(S_TTY_INIT);
	iflag = 1;
	port::iflag = 1;
	if (ref_tty_init() != port::tty_init())
		fail_msg(S_TTY_INIT, "edge iflag", "return mismatch");
	iflag = save;
	port::iflag = save;
}

void test_tty_init_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		int save = iflag;
		case_inc(S_TTY_INIT);
		iflag = (int)(rnd_u32() & 1u);
		port::iflag = iflag;
		if (ref_tty_init() != port::tty_init())
			fail_msg(S_TTY_INIT, "random", "return mismatch");
		iflag = save;
		port::iflag = save;
	}
}

void test_tty_prnt_edge(void)
{
	case_inc(S_TTY_PRNT);
	ref_tty_prnt("plain %d\n", 42);
	port::tty_prnt("plain %d\n", 42);
	case_inc(S_TTY_PRNT);
	ref_tty_prnt("%s", "");
	port::tty_prnt("%s", "");
	case_inc(S_TTY_PRNT);
	ref_tty_prnt("%c%c%c", (char)0x80, (char)0xff, '\n');
	port::tty_prnt("%c%c%c", (char)0x80, (char)0xff, '\n');
}

void test_tty_prnt_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		case_inc(S_TTY_PRNT);
		ref_tty_prnt("v%%d n%%u\n", rnd_i32(), rnd_u32());
		port::tty_prnt("v%%d n%%u\n", rnd_i32(), rnd_u32());
	}
}

void test_tty_read_edge(void)
{
	GuardedBuf gref, gport;
	int rref, rport;

	guard_fill(&gref);
	guard_fill(&gport);
	case_inc(S_TTY_READ);
	rref = ref_tty_read((char *)buf_user(&gref), 0);
	rport = port::tty_read((char *)buf_user(&gport), 0);
	if (rref != rport || !guard_eq(&gref, &gport))
		fail_msg(S_TTY_READ, "len0", "mismatch");

	guard_fill(&gref);
	guard_fill(&gport);
	case_inc(S_TTY_READ);
	rref = ref_tty_read((char *)buf_user(&gref), 1);
	rport = port::tty_read((char *)buf_user(&gport), 1);
	if (rref != rport || !guard_eq(&gref, &gport))
		fail_msg(S_TTY_READ, "len1", "mismatch");

	guard_fill(&gref);
	guard_fill(&gport);
	case_inc(S_TTY_READ);
	rref = ref_tty_read((char *)buf_user(&gref), -5);
	rport = port::tty_read((char *)buf_user(&gport), -5);
	if (rref != rport || !guard_eq(&gref, &gport))
		fail_msg(S_TTY_READ, "len-neg", "mismatch");
}

void test_tty_read_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		GuardedBuf gref, gport;
		int len = (int)(rnd_u32() % 64u) - 8;
		case_inc(S_TTY_READ);
		guard_fill(&gref);
		guard_fill(&gport);
		if (ref_tty_read((char *)buf_user(&gref), len) !=
		    port::tty_read((char *)buf_user(&gport), len) ||
		    !guard_eq(&gref, &gport))
			fail_msg(S_TTY_READ, "random", "mismatch");
	}
}

void test_paxwarn_edge(void)
{
	int save_ev = exit_val, save_vf = vfpart;
	FILE *save_lf = listf;

	exit_val = port::exit_val = 0;
	vflag = port::vflag = 0;
	vfpart = port::vfpart = 0;
	case_inc(S_PAXWARN);
	ref_paxwarn(0, "warn %s", "edge");
	port::paxwarn(0, "warn %s", "edge");
	case_inc(S_PAXWARN);
	ref_paxwarn(1, "set %d", 7);
	port::paxwarn(1, "set %d", 7);
	if (exit_val != port::exit_val)
		fail_msg(S_PAXWARN, "exit_val", "mismatch");

	vflag = port::vflag = 1;
	vfpart = port::vfpart = 1;
	listf = port::listf = stdout;
	case_inc(S_PAXWARN);
	ref_paxwarn(0, "vflag");
	port::paxwarn(0, "vflag");

	exit_val = port::exit_val = save_ev;
	vfpart = port::vfpart = save_vf;
	vflag = port::vflag = 0;
	listf = port::listf = save_lf;
}

void test_paxwarn_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		int set = (int)(rnd_u32() & 1u);
		int val = rnd_i32();
		case_inc(S_PAXWARN);
		exit_val = port::exit_val = 0;
		vflag = port::vflag = (int)(rnd_u32() & 1u);
		vfpart = port::vfpart = (int)(rnd_u32() & 1u);
		listf = port::listf = stdout;
		ref_paxwarn(set, "m %d", val);
		port::paxwarn(set, "m %d", val);
		if (exit_val != port::exit_val)
			fail_msg(S_PAXWARN, "random exit", "mismatch");
	}
}

void test_syswarn_edge(void)
{
	exit_val = port::exit_val = 0;
	vflag = port::vflag = 0;
	case_inc(S_SYSWARN);
	ref_syswarn(0, 0, "plain");
	port::syswarn(0, 0, "plain");
	case_inc(S_SYSWARN);
	ref_syswarn(0, EINVAL, "errno");
	port::syswarn(0, EINVAL, "errno");
	case_inc(S_SYSWARN);
	ref_syswarn(1, ENOENT, "set");
	port::syswarn(1, ENOENT, "set");
	if (exit_val != port::exit_val)
		fail_msg(S_SYSWARN, "exit_val", "mismatch");
}

void test_syswarn_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		int set = (int)(rnd_u32() & 1u);
		int errnum = (int)(rnd_u32() % 64u) - 5;
		int val = rnd_i32();
		case_inc(S_SYSWARN);
		exit_val = port::exit_val = 0;
		vflag = port::vflag = (int)(rnd_u32() & 1u);
		vfpart = port::vfpart = (int)(rnd_u32() & 1u);
		ref_syswarn(set, errnum, "e %d", val);
		port::syswarn(set, errnum, "e %d", val);
		if (exit_val != port::exit_val)
			fail_msg(S_SYSWARN, "random exit", "mismatch");
	}
}

void init_archd(port::ARCHD *a, const char *name, int type, time_t mtime)
{
	std::memset(a, 0, sizeof(*a));
	std::strncpy(a->name, name, sizeof(a->name) - 1);
	std::strncpy(a->ln_name, "linktarget", sizeof(a->ln_name) - 1);
	a->type = type;
	a->sb.st_mode = S_IFREG | 0644;
	a->sb.st_nlink = 1;
	a->sb.st_uid = getuid();
	a->sb.st_gid = getgid();
	a->sb.st_size = 12345;
	a->sb.st_mtime = mtime;
	a->sb.st_rdev = makedev(8, 1);
}

std::string capture_ls_list(bool use_ref, port::ARCHD *arcn, time_t now, int v)
{
	char *buf = nullptr;
	std::size_t cap = 0;
	FILE *fp = open_memstream(&buf, &cap);
	std::string out;

	if (fp == nullptr)
		return out;
	if (use_ref) {
		vflag = v;
		ref_ls_list(arcn, now, fp);
	} else {
		port::vflag = v;
		port::ls_list(arcn, now, fp);
	}
	std::fflush(fp);
	fclose(fp);
	if (buf != nullptr) {
		out = buf;
		free(buf);
	}
	return out;
}

void test_ls_list_edge(void)
{
	port::ARCHD arcn{};
	time_t now = std::time(nullptr);

	init_archd(&arcn, "short.txt", PAX_REG, now);
	case_inc(S_LS_LIST);
	if (capture_ls_list(true, &arcn, now, 0) != capture_ls_list(false, &arcn, now, 0))
		fail_msg(S_LS_LIST, "short nv", "output mismatch");

	case_inc(S_LS_LIST);
	if (capture_ls_list(true, &arcn, now, 1) != capture_ls_list(false, &arcn, now, 1))
		fail_msg(S_LS_LIST, "short v", "output mismatch");

	init_archd(&arcn, "devnode", PAX_CHR, now - 200 * 86400);
	case_inc(S_LS_LIST);
	if (capture_ls_list(true, &arcn, now, 1) != capture_ls_list(false, &arcn, now, 1))
		fail_msg(S_LS_LIST, "chr old", "output mismatch");

	init_archd(&arcn, "symlink", PAX_SLK, now);
	case_inc(S_LS_LIST);
	if (capture_ls_list(true, &arcn, now, 1) != capture_ls_list(false, &arcn, now, 1))
		fail_msg(S_LS_LIST, "slk", "output mismatch");
}

void test_ls_list_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		port::ARCHD arcn{};
		time_t now = std::time(nullptr);
		int types[] = { PAX_REG, PAX_CHR, PAX_BLK, PAX_SLK, PAX_HLK, PAX_HRG };
		char name[32];

		std::snprintf(name, sizeof(name), "f%u", rnd_u32());
		init_archd(&arcn, name, types[rnd_u32() % 6],
		    now - (time_t)(rnd_u32() % (400u * 86400u)));
		int v = (int)(rnd_u32() & 1u);
		case_inc(S_LS_LIST);
		if (capture_ls_list(true, &arcn, now, v) !=
		    capture_ls_list(false, &arcn, now, v))
			fail_msg(S_LS_LIST, "random", "output mismatch");
	}
}

void test_ls_tty_edge(void)
{
	port::ARCHD arcn{};
	time_t now = std::time(nullptr);

	init_archd(&arcn, "ttyfile", PAX_REG, now);
	case_inc(S_LS_TTY);
	ref_ls_tty(&arcn);
	port::ls_tty(&arcn);

	init_archd(&arcn, "oldfile", PAX_REG, now - 400 * 86400);
	case_inc(S_LS_TTY);
	ref_ls_tty(&arcn);
	port::ls_tty(&arcn);
}

void test_ls_tty_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		port::ARCHD arcn{};
		char name[32];
		std::snprintf(name, sizeof(name), "t%u", rnd_u32());
		init_archd(&arcn, name, PAX_REG,
		    std::time(nullptr) - (time_t)(rnd_u32() % (500u * 86400u)));
		case_inc(S_LS_TTY);
		ref_ls_tty(&arcn);
		port::ls_tty(&arcn);
	}
}

void test_l_strncpy_one(const char *label, const char *src, int len)
{
	GuardedBuf gref, gport;
	int rref, rport;

	guard_fill(&gref);
	guard_fill(&gport);
	if (src != nullptr) {
		std::strncpy((char *)buf_user(&gref), src, BUF_CAP - 1);
		std::strncpy((char *)buf_user(&gport), src, BUF_CAP - 1);
	}
	case_inc(S_L_STRNCPY);
	rref = ref_l_strncpy((char *)buf_user(&gref), src, len);
	rport = port::l_strncpy((char *)buf_user(&gport), src, len);
	if (rref != rport || !guard_eq(&gref, &gport))
		fail_msg(S_L_STRNCPY, label, "mismatch");
}

void test_l_strncpy_edge(void)
{
	test_l_strncpy_one("empty0", "", 0);
	test_l_strncpy_one("empty1", "", 1);
	test_l_strncpy_one("single", "a", 8);
	test_l_strncpy_one("nulheavy", "a\0b\0c", 10);
	test_l_strncpy_one("exact", "hello", 5);
	test_l_strncpy_one("pad", "hi", 8);
	test_l_strncpy_one("highbit", "\x80\xff\xfe", 12);
}

void test_l_strncpy_random(void)
{
	unsigned char src[128];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		GuardedBuf gref, gport;
		int len = (int)(rnd_u32() % 96u);
		int slen = (int)(rnd_u32() % (sizeof(src) + 1u));
		case_inc(S_L_STRNCPY);
		fill_random_string(src, slen, (rnd_u32() & 3u) == 0);
		guard_fill(&gref);
		guard_fill(&gport);
		if (ref_l_strncpy((char *)buf_user(&gref), (char *)src, len) !=
		    port::l_strncpy((char *)buf_user(&gport), (char *)src, len) ||
		    !guard_eq(&gref, &gport))
			fail_msg(S_L_STRNCPY, "random", "mismatch");
	}
}

void test_asc_ul_edge(void)
{
	struct { const char *s; int len; int base; } cases[] = {
		{ "", 0, HEX }, { "0", 1, HEX }, { "00", 2, HEX },
		{ "  00ff", 6, HEX }, { "deadBEEF", 8, HEX }, { "g1", 2, HEX },
		{ "777", 3, OCT }, { "  0777", 6, OCT }, { "89", 2, OCT },
		{ "\x80\xff", 2, HEX },
	};
	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		case_inc(S_ASC_UL);
		if (ref_asc_ul((char *)cases[i].s, cases[i].len, cases[i].base) !=
		    port::asc_ul((char *)cases[i].s, cases[i].len, cases[i].base))
			fail_msg(S_ASC_UL, "edge", "mismatch");
	}
}

void test_asc_ul_random(void)
{
	unsigned char s[64];
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		int len = (int)(rnd_u32() % sizeof(s));
		int base = (rnd_u32() & 1u) ? HEX : OCT;
		fill_random_string(s, len, true);
		case_inc(S_ASC_UL);
		if (ref_asc_ul((char *)s, len, base) != port::asc_ul((char *)s, len, base))
			fail_msg(S_ASC_UL, "random", "mismatch");
	}
}

void test_ul_asc_edge(void)
{
	struct { unsigned long val; int len; int base; } cases[] = {
		{ 0, 1, HEX }, { 0, 4, HEX }, { 0xdeadbeefUL, 8, HEX },
		{ 0xffffffffUL, 8, HEX }, { 0777UL, 4, OCT }, { 1UL << 20, 3, HEX },
	};
	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		GuardedBuf gref, gport;
		guard_fill(&gref);
		guard_fill(&gport);
		case_inc(S_UL_ASC);
		if (ref_ul_asc(cases[i].val, (char *)buf_user(&gref), cases[i].len,
			cases[i].base) != port::ul_asc(cases[i].val,
			(char *)buf_user(&gport), cases[i].len, cases[i].base) ||
		    !guard_eq(&gref, &gport))
			fail_msg(S_UL_ASC, "edge", "mismatch");
	}
}

void test_ul_asc_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		GuardedBuf gref, gport;
		unsigned long val = (unsigned long)rnd_u64();
		int len = 1 + (int)(rnd_u32() % 16u);
		int base = (rnd_u32() & 1u) ? HEX : OCT;
		guard_fill(&gref);
		guard_fill(&gport);
		case_inc(S_UL_ASC);
		if (ref_ul_asc(val, (char *)buf_user(&gref), len, base) !=
		    port::ul_asc(val, (char *)buf_user(&gport), len, base) ||
		    !guard_eq(&gref, &gport))
			fail_msg(S_UL_ASC, "random", "mismatch");
	}
}

void test_asc_uqd_edge(void)
{
	struct { const char *s; int len; int base; } cases[] = {
		{ "0", 1, HEX }, { "ffffffffffffffff", 16, HEX },
		{ "777", 3, OCT }, { "Z", 1, HEX },
	};
	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		case_inc(S_ASC_UQD);
		if (ref_asc_uqd((char *)cases[i].s, cases[i].len, cases[i].base) !=
		    port::asc_uqd((char *)cases[i].s, cases[i].len, cases[i].base))
			fail_msg(S_ASC_UQD, "edge", "mismatch");
	}
}

void test_asc_uqd_random(void)
{
	unsigned char s[64];
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		int len = (int)(rnd_u32() % sizeof(s));
		int base = (rnd_u32() & 1u) ? HEX : OCT;
		fill_random_string(s, len, true);
		case_inc(S_ASC_UQD);
		if (ref_asc_uqd((char *)s, len, base) != port::asc_uqd((char *)s, len, base))
			fail_msg(S_ASC_UQD, "random", "mismatch");
	}
}

void test_uqd_asc_edge(void)
{
	struct { unsigned long long val; int len; int base; } cases[] = {
		{ 0, 1, HEX }, { 0xdeadbeefcafebabeULL, 16, HEX }, { 0777ULL, 4, OCT },
	};
	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		GuardedBuf gref, gport;
		guard_fill(&gref);
		guard_fill(&gport);
		case_inc(S_UQD_ASC);
		if (ref_uqd_asc(cases[i].val, (char *)buf_user(&gref), cases[i].len,
			cases[i].base) != port::uqd_asc(cases[i].val,
			(char *)buf_user(&gport), cases[i].len, cases[i].base) ||
		    !guard_eq(&gref, &gport))
			fail_msg(S_UQD_ASC, "edge", "mismatch");
	}
}

void test_uqd_asc_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		GuardedBuf gref, gport;
		unsigned long long val = rnd_u64();
		int len = 1 + (int)(rnd_u32() % 20u);
		int base = (rnd_u32() & 1u) ? HEX : OCT;
		guard_fill(&gref);
		guard_fill(&gport);
		case_inc(S_UQD_ASC);
		if (ref_uqd_asc(val, (char *)buf_user(&gref), len, base) !=
		    port::uqd_asc(val, (char *)buf_user(&gport), len, base) ||
		    !guard_eq(&gref, &gport))
			fail_msg(S_UQD_ASC, "random", "mismatch");
	}
}

void test_tb_start_random(StatId id, int (*ref_fn)(void), int (*port_fn)(void))
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		case_inc(id);
		if (ref_fn() != port_fn())
			fail_msg(id, "random", "return mismatch");
	}
}

void test_name_uid_edge(void)
{
	case_inc(S_NAME_UID);
	if (std::strcmp(ref_name_uid(0, 1), port::name_uid(0, 1)) != 0)
		fail_msg(S_NAME_UID, "uid0 frc1", "string mismatch");
	case_inc(S_NAME_UID);
	if (std::strcmp(ref_name_uid((uid_t)-1, 0), port::name_uid((uid_t)-1, 0)) != 0)
		fail_msg(S_NAME_UID, "uidmax frc0", "string mismatch");
	case_inc(S_NAME_UID);
	if (std::strcmp(ref_name_uid(getuid(), 1), port::name_uid(getuid(), 1)) != 0)
		fail_msg(S_NAME_UID, "getuid", "string mismatch");
}

void test_name_uid_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		uid_t uid = (uid_t)rnd_u32();
		int frc = (int)(rnd_u32() & 1u);
		case_inc(S_NAME_UID);
		if (std::strcmp(ref_name_uid(uid, frc), port::name_uid(uid, frc)) != 0)
			fail_msg(S_NAME_UID, "random", "string mismatch");
	}
}

void test_name_gid_edge(void)
{
	case_inc(S_NAME_GID);
	if (std::strcmp(ref_name_gid(0, 1), port::name_gid(0, 1)) != 0)
		fail_msg(S_NAME_GID, "gid0", "string mismatch");
	case_inc(S_NAME_GID);
	if (std::strcmp(ref_name_gid(getgid(), 1), port::name_gid(getgid(), 1)) != 0)
		fail_msg(S_NAME_GID, "getgid", "string mismatch");
}

void test_name_gid_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		gid_t gid = (gid_t)rnd_u32();
		int frc = (int)(rnd_u32() & 1u);
		case_inc(S_NAME_GID);
		if (std::strcmp(ref_name_gid(gid, frc), port::name_gid(gid, frc)) != 0)
			fail_msg(S_NAME_GID, "random", "string mismatch");
	}
}

void test_uid_name_edge(void)
{
	uid_t uref = 0, uport = 0;
	char buf[64];

	case_inc(S_UID_NAME);
	if (ref_uid_name((char *)"", &uref) != port::uid_name((char *)"", &uport))
		fail_msg(S_UID_NAME, "empty", "return mismatch");

	struct passwd *pw = getpwuid(getuid());
	if (pw != nullptr) {
		case_inc(S_UID_NAME);
		if (ref_uid_name(pw->pw_name, &uref) != port::uid_name(pw->pw_name, &uport) ||
		    uref != uport)
			fail_msg(S_UID_NAME, "valid", "mismatch");
	}

	case_inc(S_UID_NAME);
	std::snprintf(buf, sizeof(buf), "nouser_%u", rnd_u32());
	if (ref_uid_name(buf, &uref) != port::uid_name(buf, &uport))
		fail_msg(S_UID_NAME, "invalid", "return mismatch");
}

void test_uid_name_random(void)
{
	char name[UNMLEN + 8];
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		uid_t uref = 0, uport = 0;
		int nlen = (int)(rnd_u32() % (sizeof(name) - 1));
		fill_random_string((unsigned char *)name, nlen, (rnd_u32() & 7u) == 0);
		name[nlen] = '\0';
		case_inc(S_UID_NAME);
		int rref = ref_uid_name(name, &uref);
		int rport = port::uid_name(name, &uport);
		if (rref != rport || (rref == 0 && uref != uport))
			fail_msg(S_UID_NAME, "random", "mismatch");
	}
}

void test_gid_name_edge(void)
{
	gid_t gref = 0, gport = 0;
	char buf[64];

	case_inc(S_GID_NAME);
	if (ref_gid_name((char *)"", &gref) != port::gid_name((char *)"", &gport))
		fail_msg(S_GID_NAME, "empty", "return mismatch");

	struct group *gr = getgrgid(getgid());
	if (gr != nullptr) {
		case_inc(S_GID_NAME);
		if (ref_gid_name(gr->gr_name, &gref) != port::gid_name(gr->gr_name, &gport) ||
		    gref != gport)
			fail_msg(S_GID_NAME, "valid", "mismatch");
	}

	case_inc(S_GID_NAME);
	std::snprintf(buf, sizeof(buf), "nogroup_%u", rnd_u32());
	if (ref_gid_name(buf, &gref) != port::gid_name(buf, &gport))
		fail_msg(S_GID_NAME, "invalid", "return mismatch");
}

void test_gid_name_random(void)
{
	char name[GNMLEN + 8];
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		gid_t gref = 0, gport = 0;
		int nlen = (int)(rnd_u32() % (sizeof(name) - 1));
		fill_random_string((unsigned char *)name, nlen, (rnd_u32() & 7u) == 0);
		name[nlen] = '\0';
		case_inc(S_GID_NAME);
		int rref = ref_gid_name(name, &gref);
		int rport = port::gid_name(name, &gport);
		if (rref != rport || (rref == 0 && gref != gport))
			fail_msg(S_GID_NAME, "random", "mismatch");
	}
}

} // namespace

int main(void)
{
	test_tty_init_edge();
	test_tty_prnt_edge();
	test_tty_read_edge();
	test_paxwarn_edge();
	test_syswarn_edge();
	test_ls_list_edge();
	test_ls_tty_edge();
	test_l_strncpy_edge();
	test_asc_ul_edge();
	test_ul_asc_edge();
	test_asc_uqd_edge();
	test_uqd_asc_edge();
	test_tb_start_random(S_UIDTB_START, ref_uidtb_start, port::uidtb_start);
	test_tb_start_random(S_GIDTB_START, ref_gidtb_start, port::gidtb_start);
	test_tb_start_random(S_USRTB_START, ref_usrtb_start, port::usrtb_start);
	test_tb_start_random(S_GRPTB_START, ref_grptb_start, port::grptb_start);
	test_name_uid_edge();
	test_name_gid_edge();
	test_uid_name_edge();
	test_gid_name_edge();

	test_tty_init_random();
	test_tty_prnt_random();
	test_tty_read_random();
	test_paxwarn_random();
	test_syswarn_random();
	test_ls_list_random();
	test_ls_tty_random();
	test_l_strncpy_random();
	test_asc_ul_random();
	test_ul_asc_random();
	test_asc_uqd_random();
	test_uqd_asc_random();
	test_name_uid_random();
	test_name_gid_random();
	test_uid_name_random();
	test_gid_name_random();

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NSTAT; i++)
		std::printf("%-14s %12lld %12lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);

	for (int i = 0; i < NSTAT; i++) {
		if (g_stat[i].fails != 0)
			return 1;
	}
	return 0;
}
