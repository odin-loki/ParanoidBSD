/*
 * harness.cpp -- differential test for PBSD batch b0200.
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
#include <sys/stat.h>
#include <unistd.h>

import pbsd.bin.pax.b0200;

namespace port = pbsd::bin_pax::b0200;

constexpr int PAX_REG = 4;

extern "C" {
int ref_ftree_add(char *, int);
int ref_sel_chk(port::ARCHD *);
int ref_usr_add(char *);
int ref_grp_add(char *);
int ref_trng_add(char *);
int ref_str_sec(const char *, time_t *);
int ref_usr_match(port::ARCHD *);
int ref_grp_match(port::ARCHD *);
int ref_trng_match(port::ARCHD *);
int ref_file_write(int, char *, int, int *, int *, int, char *);
void ref_file_flush(int, char *, int);
int ref_set_crc(port::ARCHD *, int);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr int MAX_REPORT = 8;
constexpr unsigned RAND_ITERS = 200000u;
constexpr std::size_t BUF_PRE = 16;
constexpr std::size_t BUF_CAP = 8192;
constexpr std::size_t BUF_POST = 16;
constexpr std::size_t BUF_TOTAL = BUF_PRE + BUF_CAP + BUF_POST;

enum StatId {
	S_FTREE_ADD,
	S_SEL_CHK,
	S_USR_ADD,
	S_GRP_ADD,
	S_TRNG_ADD,
	S_STR_SEC,
	S_USR_MATCH,
	S_GRP_MATCH,
	S_TRNG_MATCH,
	S_FILE_WRITE,
	S_FILE_FLUSH,
	S_SET_CRC,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int reported;
};

Stats g_stat[NSTAT] = {
	{ "ftree_add",    0, 0, 0 },
	{ "sel_chk",      0, 0, 0 },
	{ "usr_add",      0, 0, 0 },
	{ "grp_add",      0, 0, 0 },
	{ "trng_add",     0, 0, 0 },
	{ "str_sec",      0, 0, 0 },
	{ "usr_match",    0, 0, 0 },
	{ "grp_match",    0, 0, 0 },
	{ "trng_match",   0, 0, 0 },
	{ "file_write",   0, 0, 0 },
	{ "file_flush",   0, 0, 0 },
	{ "set_crc",      0, 0, 0 },
};

std::uint64_t rng_state = 0xb0200facefeedULL;
char g_tmpbase[256];

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

std::string mk_temp_dir(void)
{
	char tpl[512];
	std::snprintf(tpl, sizeof(tpl), "%s/td_XXXXXX", g_tmpbase);
	if (mkdtemp(tpl) == nullptr)
		return "";
	return tpl;
}

void init_archd(port::ARCHD *a, const char *name, int type)
{
	std::memset(a, 0, sizeof(*a));
	std::strncpy(a->name, name, sizeof(a->name) - 1);
	std::strncpy(a->ln_name, "linktgt", sizeof(a->ln_name) - 1);
	a->org_name = a->name;
	a->type = type;
	a->sb.st_mode = S_IFREG | 0644;
	a->sb.st_uid = getuid();
	a->sb.st_gid = getgid();
	a->sb.st_size = 100;
	a->sb.st_mtime = std::time(nullptr);
	a->sb.st_atime = a->sb.st_mtime;
	a->sb.st_ctime = a->sb.st_mtime;
	a->sb.st_blksize = 4096;
}

void test_ftree_add_one(StatId id, char *sref, char *sport, int chflg)
{
	case_inc(id);
	int rref = ref_ftree_add(sref, chflg);
	int rport = port::ftree_add(sport, chflg);
	if (rref != rport)
		fail_msg(id, "return", "mismatch");
}

void test_ftree_add_edge(void)
{
	char empty[] = "";
	char slash[] = "/";

	case_inc(S_FTREE_ADD);
	if (ref_ftree_add(nullptr, 0) != port::ftree_add(nullptr, 0))
		fail_msg(S_FTREE_ADD, "null", "return mismatch");

	test_ftree_add_one(S_FTREE_ADD, empty, empty, 0);

	char sref[64], sport[64];
	std::snprintf(sref, sizeof(sref), "%s/x", g_tmpbase);
	std::snprintf(sport, sizeof(sport), "%s/x", g_tmpbase);
	test_ftree_add_one(S_FTREE_ADD, sref, sport, 0);

	char sref2[64], sport2[64];
	std::strcpy(sref2, slash);
	std::strcpy(sport2, slash);
	test_ftree_add_one(S_FTREE_ADD, sref2, sport2, 0);

	std::string td = mk_temp_dir();
	if (!td.empty()) {
		char tref[256], tport[256];
		std::snprintf(tref, sizeof(tref), "%s/foo/", td.c_str());
		std::snprintf(tport, sizeof(tport), "%s/foo/", td.c_str());
		test_ftree_add_one(S_FTREE_ADD, tref, tport, 0);
	}
}

void test_ftree_add_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char sref[128], sport[128];
		int n = (int)(rnd_u32() % 80u) + 1;
		fill_random_string((unsigned char *)sref, n, false);
		fill_random_string((unsigned char *)sport, n, false);
		sref[n] = '\0';
		sport[n] = '\0';
		if (n > 1 && (rnd_u32() & 3u) == 0) {
			sref[n - 1] = '/';
			sport[n - 1] = '/';
		}
		test_ftree_add_one(S_FTREE_ADD, sref, sport, (int)(rnd_u32() & 1u));
	}
}

void test_sel_chk_one(const port::ARCHD *aref, const port::ARCHD *aport)
{
	case_inc(S_SEL_CHK);
	if (ref_sel_chk((port::ARCHD *)aref) != port::sel_chk((port::ARCHD *)aport))
		fail_msg(S_SEL_CHK, "cmp", "return mismatch");
}

void test_sel_chk_edge(void)
{
	port::ARCHD aref{}, aport{};
	init_archd(&aref, "f", PAX_REG);
	init_archd(&aport, "f", PAX_REG);
	test_sel_chk_one(&aref, &aport);

	char s[32], s2[32];
	std::snprintf(s, sizeof(s), "#%u", (unsigned)getuid());
	std::strcpy(s2, s);
	ref_usr_add(s);
	port::usr_add(s2);
	aref.sb.st_uid = getuid() + 1;
	aport.sb.st_uid = getuid() + 1;
	test_sel_chk_one(&aref, &aport);
	aref.sb.st_uid = getuid();
	aport.sb.st_uid = getuid();
	test_sel_chk_one(&aref, &aport);

	char gs[32], gs2[32];
	std::snprintf(gs, sizeof(gs), "#%u", (unsigned)getgid());
	std::strcpy(gs2, gs);
	ref_grp_add(gs);
	port::grp_add(gs2);
	aref.sb.st_gid = getgid() + 1;
	aport.sb.st_gid = getgid() + 1;
	test_sel_chk_one(&aref, &aport);

	char trref[64], trport[64];
	std::strcpy(trref, "01011230,01011245/m");
	std::strcpy(trport, "01011230,01011245/m");
	ref_trng_add(trref);
	port::trng_add(trport);
	aref.sb.st_mtime = 0;
	aport.sb.st_mtime = 0;
	test_sel_chk_one(&aref, &aport);
}

void test_sel_chk_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char sref[32], sport[32];
		port::ARCHD aref{}, aport{};
		init_archd(&aref, "f", PAX_REG);
		init_archd(&aport, "f", PAX_REG);
		aref.sb.st_uid = (uid_t)rnd_u32();
		aport.sb.st_uid = aref.sb.st_uid;
		aref.sb.st_gid = (gid_t)rnd_u32();
		aport.sb.st_gid = aref.sb.st_gid;
		aref.sb.st_mtime = (time_t)rnd_i32();
		aport.sb.st_mtime = aref.sb.st_mtime;
		aref.sb.st_ctime = (time_t)rnd_i32();
		aport.sb.st_ctime = aref.sb.st_ctime;
		if (rnd_u32() & 1u) {
			std::snprintf(sref, sizeof(sref), "#%u", (unsigned)getuid());
			std::snprintf(sport, sizeof(sport), "#%u", (unsigned)getuid());
			ref_usr_add(sref);
			port::usr_add(sport);
		}
		if (rnd_u32() & 1u) {
			std::snprintf(sref, sizeof(sref), "#%u", (unsigned)getgid());
			std::snprintf(sport, sizeof(sport), "#%u", (unsigned)getgid());
			ref_grp_add(sref);
			port::grp_add(sport);
		}
		if ((rnd_u32() & 3u) == 0) {
			char trref[64], trport[64];
			std::snprintf(trref, sizeof(trref), "%02d%02d", rnd_u32() % 24u,
			    rnd_u32() % 60u);
			std::strcpy(trport, trref);
			ref_trng_add(trref);
			port::trng_add(trport);
		}
		test_sel_chk_one(&aref, &aport);
	}
}

void test_usr_add_one(char *sref, char *sport)
{
	case_inc(S_USR_ADD);
	if (ref_usr_add(sref) != port::usr_add(sport))
		fail_msg(S_USR_ADD, "add", "return mismatch");
}

void test_usr_add_edge(void)
{
	char empty[] = "";
	case_inc(S_USR_ADD);
	if (ref_usr_add(nullptr) != port::usr_add(nullptr))
		fail_msg(S_USR_ADD, "null", "mismatch");
	test_usr_add_one(empty, empty);

	char sref[32], sport[32];
	std::snprintf(sref, sizeof(sref), "#%u", (unsigned)getuid());
	std::snprintf(sport, sizeof(sport), "#%u", (unsigned)getuid());
	test_usr_add_one(sref, sport);

	struct passwd *pw = getpwuid(getuid());
	if (pw != nullptr) {
		char nref[64], nport[64];
		std::strcpy(nref, pw->pw_name);
		std::strcpy(nport, pw->pw_name);
		test_usr_add_one(nref, nport);
	}

	char esc[64], esc2[64];
	std::snprintf(esc, sizeof(esc), "\\#%u", (unsigned)getuid());
	std::strcpy(esc2, esc);
	test_usr_add_one(esc, esc2);
}

void test_usr_add_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char sref[64], sport[64];
		if (rnd_u32() & 1u) {
			std::snprintf(sref, sizeof(sref), "#%u", rnd_u32());
			std::snprintf(sport, sizeof(sport), "#%u", rnd_u32());
		} else {
			int n = (int)(rnd_u32() % 16u) + 1;
			fill_random_string((unsigned char *)sref, n, false);
			fill_random_string((unsigned char *)sport, n, false);
			sref[n] = '\0';
			sport[n] = '\0';
		}
		test_usr_add_one(sref, sport);
	}
}

void test_grp_add_edge(void)
{
	char empty[] = "";
	case_inc(S_GRP_ADD);
	if (ref_grp_add(nullptr) != port::grp_add(nullptr))
		fail_msg(S_GRP_ADD, "null", "mismatch");
	case_inc(S_GRP_ADD);
	if (ref_grp_add(empty) != port::grp_add(empty))
		fail_msg(S_GRP_ADD, "empty", "mismatch");

	char sref[32], sport[32];
	std::snprintf(sref, sizeof(sref), "#%u", (unsigned)getgid());
	std::snprintf(sport, sizeof(sport), "#%u", (unsigned)getgid());
	case_inc(S_GRP_ADD);
	if (ref_grp_add(sref) != port::grp_add(sport))
		fail_msg(S_GRP_ADD, "gid", "mismatch");
}

void test_grp_add_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char sref[64], sport[64];
		std::snprintf(sref, sizeof(sref), "#%u", rnd_u32());
		std::snprintf(sport, sizeof(sport), "#%u", rnd_u32());
		case_inc(S_GRP_ADD);
		if (ref_grp_add(sref) != port::grp_add(sport))
			fail_msg(S_GRP_ADD, "random", "mismatch");
	}
}

void test_trng_add_one(char *sref, char *sport)
{
	case_inc(S_TRNG_ADD);
	if (ref_trng_add(sref) != port::trng_add(sport))
		fail_msg(S_TRNG_ADD, "add", "return mismatch");
}

void test_trng_add_edge(void)
{
	char empty[] = "";
	case_inc(S_TRNG_ADD);
	if (ref_trng_add(nullptr) != port::trng_add(nullptr))
		fail_msg(S_TRNG_ADD, "null", "mismatch");
	test_trng_add_one(empty, empty);

	char s1ref[32], s1port[32];
	std::strcpy(s1ref, "1234");
	std::strcpy(s1port, "1234");
	test_trng_add_one(s1ref, s1port);

	char s2ref[64], s2port[64];
	std::strcpy(s2ref, "01011230,01011245/m");
	std::strcpy(s2port, "01011230,01011245/m");
	test_trng_add_one(s2ref, s2port);

	char s3ref[64], s3port[64];
	std::strcpy(s3ref, "01011230,01011245/c");
	std::strcpy(s3port, "01011230,01011245/c");
	test_trng_add_one(s3ref, s3port);

	char s4ref[64], s4port[64];
	std::strcpy(s4ref, "01011230,01011245/cm");
	std::strcpy(s4port, "01011230,01011245/cm");
	test_trng_add_one(s4ref, s4port);

	char s5ref[32], s5port[32];
	std::strcpy(s5ref, "bad!");
	std::strcpy(s5port, "bad!");
	test_trng_add_one(s5ref, s5port);

	char s6ref[32], s6port[32];
	std::strcpy(s6ref, "1234.56");
	std::strcpy(s6port, "1234.56");
	test_trng_add_one(s6ref, s6port);

	char s7ref[64], s7port[64];
	std::strcpy(s7ref, "9912312359,0001010000");
	std::strcpy(s7port, "9912312359,0001010000");
	test_trng_add_one(s7ref, s7port);
}

void test_trng_add_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char sref[128], sport[128];
		int hr = (int)(rnd_u32() % 24u);
		int mn = (int)(rnd_u32() % 60u);
		std::snprintf(sref, sizeof(sref), "%02d%02d", hr, mn);
		std::snprintf(sport, sizeof(sport), "%02d%02d", hr, mn);
		if (rnd_u32() & 1u) {
			char up[64];
			std::snprintf(up, sizeof(up), ",%02d%02d", hr, (mn + 1) % 60);
			std::strcat(sref, up);
			std::strcat(sport, up);
		}
		if ((rnd_u32() & 3u) == 0) {
			std::strcat(sref, "/m");
			std::strcat(sport, "/m");
		} else if ((rnd_u32() & 3u) == 1) {
			std::strcat(sref, "/c");
			std::strcat(sport, "/c");
		}
		test_trng_add_one(sref, sport);
	}
}

void test_str_sec_one(const char *s, time_t tref_seed, time_t tport_seed)
{
	time_t tref = tref_seed;
	time_t tport = tport_seed;
	case_inc(S_STR_SEC);
	int rref = ref_str_sec(s, &tref);
	int rport = port::str_sec(s, &tport);
	if (rref != rport || (rref == 0 && tref != tport))
		fail_msg(S_STR_SEC, s, "mismatch");
}

void test_str_sec_edge(void)
{
	time_t now = std::time(nullptr);

	test_str_sec_one("", now, now);
	test_str_sec_one("12", now, now);
	test_str_sec_one("1234", now, now);
	test_str_sec_one("01011230", now, now);
	test_str_sec_one("25011230", now, now);
	test_str_sec_one("01010000.00", now, now);
	test_str_sec_one("01010000.62", now, now);
	test_str_sec_one("01010000.0", now, now);
	test_str_sec_one("0101", now, now);
	test_str_sec_one("x", now, now);
	test_str_sec_one("9912312359", now, now);
	test_str_sec_one("0001010000", now, now);
	test_str_sec_one("6912312359", now, now);
	test_str_sec_one("2012312359", now, now);
	test_str_sec_one("200001010000", now, now);
}

void test_str_sec_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char s[32];
		time_t now = std::time(nullptr);
		unsigned pick = rnd_u32() % 10u;
		if (pick == 0)
			std::snprintf(s, sizeof(s), "%02d", rnd_u32() % 60u);
		else if (pick == 1)
			std::snprintf(s, sizeof(s), "%02d%02d", rnd_u32() % 24u,
			    rnd_u32() % 60u);
		else if (pick == 2)
			std::snprintf(s, sizeof(s), "%02d%02d%02d",
			    (rnd_u32() % 28u) + 1, rnd_u32() % 24u,
			    rnd_u32() % 60u);
		else if (pick == 3)
			std::snprintf(s, sizeof(s), "%02d%02d%02d%02d",
			    (rnd_u32() % 12u) + 1, (rnd_u32() % 28u) + 1,
			    rnd_u32() % 24u, rnd_u32() % 60u);
		else if (pick == 4)
			std::snprintf(s, sizeof(s), "%02d%02d%02d%02d.%02d",
			    (rnd_u32() % 12u) + 1, (rnd_u32() % 28u) + 1,
			    rnd_u32() % 24u, rnd_u32() % 60u, rnd_u32() % 62u);
		else if (pick == 5)
			s[0] = (char)('a' + (rnd_u32() % 26u)), s[1] = '\0';
		else
			std::snprintf(s, sizeof(s), "%010d", rnd_u32() % 1000000000u);
		test_str_sec_one(s, now, now);
	}
}

void test_usr_match_edge(void)
{
	char s[32], s2[32];
	std::snprintf(s, sizeof(s), "#%u", (unsigned)getuid());
	std::strcpy(s2, s);
	ref_usr_add(s);
	port::usr_add(s2);

	port::ARCHD aref{}, aport{};
	init_archd(&aref, "f", PAX_REG);
	init_archd(&aport, "f", PAX_REG);
	aref.sb.st_uid = getuid();
	aport.sb.st_uid = getuid();
	case_inc(S_USR_MATCH);
	if (ref_usr_match(&aref) != port::usr_match(&aport))
		fail_msg(S_USR_MATCH, "match", "mismatch");

	aref.sb.st_uid = getuid() + 1;
	aport.sb.st_uid = getuid() + 1;
	case_inc(S_USR_MATCH);
	if (ref_usr_match(&aref) != port::usr_match(&aport))
		fail_msg(S_USR_MATCH, "nomatch", "mismatch");
}

void test_usr_match_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char sref[32], sport[32];
		std::snprintf(sref, sizeof(sref), "#%u", rnd_u32() % 65536u);
		std::snprintf(sport, sizeof(sport), "#%u", rnd_u32() % 65536u);
		ref_usr_add(sref);
		port::usr_add(sport);
		port::ARCHD aref{}, aport{};
		init_archd(&aref, "f", PAX_REG);
		init_archd(&aport, "f", PAX_REG);
		aref.sb.st_uid = (uid_t)rnd_u32();
		aport.sb.st_uid = aref.sb.st_uid;
		case_inc(S_USR_MATCH);
		if (ref_usr_match(&aref) != port::usr_match(&aport))
			fail_msg(S_USR_MATCH, "random", "mismatch");
	}
}

void test_grp_match_edge(void)
{
	char s[32], s2[32];
	std::snprintf(s, sizeof(s), "#%u", (unsigned)getgid());
	std::strcpy(s2, s);
	ref_grp_add(s);
	port::grp_add(s2);

	port::ARCHD aref{}, aport{};
	init_archd(&aref, "f", PAX_REG);
	init_archd(&aport, "f", PAX_REG);
	aref.sb.st_gid = getgid();
	aport.sb.st_gid = getgid();
	case_inc(S_GRP_MATCH);
	if (ref_grp_match(&aref) != port::grp_match(&aport))
		fail_msg(S_GRP_MATCH, "match", "mismatch");

	aref.sb.st_gid = getgid() + 1;
	aport.sb.st_gid = getgid() + 1;
	case_inc(S_GRP_MATCH);
	if (ref_grp_match(&aref) != port::grp_match(&aport))
		fail_msg(S_GRP_MATCH, "nomatch", "mismatch");
}

void test_grp_match_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char sref[32], sport[32];
		std::snprintf(sref, sizeof(sref), "#%u", rnd_u32() % 65536u);
		std::snprintf(sport, sizeof(sport), "#%u", rnd_u32() % 65536u);
		ref_grp_add(sref);
		port::grp_add(sport);
		port::ARCHD aref{}, aport{};
		init_archd(&aref, "f", PAX_REG);
		init_archd(&aport, "f", PAX_REG);
		aref.sb.st_gid = (gid_t)rnd_u32();
		aport.sb.st_gid = aref.sb.st_gid;
		case_inc(S_GRP_MATCH);
		if (ref_grp_match(&aref) != port::grp_match(&aport))
			fail_msg(S_GRP_MATCH, "random", "mismatch");
	}
}

void test_trng_match_edge(void)
{
	port::ARCHD aref{}, aport{};
	init_archd(&aref, "f", PAX_REG);
	init_archd(&aport, "f", PAX_REG);
	case_inc(S_TRNG_MATCH);
	if (ref_trng_match(&aref) != port::trng_match(&aport))
		fail_msg(S_TRNG_MATCH, "empty", "mismatch");

	char trref[64], trport[64];
	std::strcpy(trref, "01011230,01011245/m");
	std::strcpy(trport, "01011230,01011245/m");
	ref_trng_add(trref);
	port::trng_add(trport);
	aref.sb.st_mtime = std::time(nullptr);
	aport.sb.st_mtime = aref.sb.st_mtime;
	case_inc(S_TRNG_MATCH);
	if (ref_trng_match(&aref) != port::trng_match(&aport))
		fail_msg(S_TRNG_MATCH, "mtime", "mismatch");

	char tr2ref[64], tr2port[64];
	std::strcpy(tr2ref, "01011230,01011245/c");
	std::strcpy(tr2port, "01011230,01011245/c");
	ref_trng_add(tr2ref);
	port::trng_add(tr2port);
	aref.sb.st_ctime = std::time(nullptr);
	aport.sb.st_ctime = aref.sb.st_ctime;
	case_inc(S_TRNG_MATCH);
	if (ref_trng_match(&aref) != port::trng_match(&aport))
		fail_msg(S_TRNG_MATCH, "ctime", "mismatch");

	char tr3ref[64], tr3port[64];
	std::strcpy(tr3ref, "01011230,01011245/cm");
	std::strcpy(tr3port, "01011230,01011245/cm");
	ref_trng_add(tr3ref);
	port::trng_add(tr3port);
	aref.sb.st_mtime = 0;
	aport.sb.st_mtime = 0;
	aref.sb.st_ctime = std::time(nullptr);
	aport.sb.st_ctime = aref.sb.st_ctime;
	case_inc(S_TRNG_MATCH);
	if (ref_trng_match(&aref) != port::trng_match(&aport))
		fail_msg(S_TRNG_MATCH, "both", "mismatch");
}

void test_trng_match_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char trref[64], trport[64];
		std::snprintf(trref, sizeof(trref), "%02d%02d", rnd_u32() % 24u,
		    rnd_u32() % 60u);
		std::strcpy(trport, trref);
		const char *fl = "/m";
		if ((rnd_u32() & 3u) == 1)
			fl = "/c";
		else if ((rnd_u32() & 3u) == 2)
			fl = "/cm";
		std::strcat(trref, fl);
		std::strcat(trport, fl);
		ref_trng_add(trref);
		port::trng_add(trport);
		port::ARCHD aref{}, aport{};
		init_archd(&aref, "f", PAX_REG);
		init_archd(&aport, "f", PAX_REG);
		aref.sb.st_mtime = (time_t)rnd_i32();
		aport.sb.st_mtime = aref.sb.st_mtime;
		aref.sb.st_ctime = (time_t)rnd_i32();
		aport.sb.st_ctime = aref.sb.st_ctime;
		case_inc(S_TRNG_MATCH);
		if (ref_trng_match(&aref) != port::trng_match(&aport))
			fail_msg(S_TRNG_MATCH, "random", "mismatch");
	}
}

int read_file_bytes(const char *path, unsigned char *buf, std::size_t cap)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	ssize_t n = read(fd, buf, cap);
	close(fd);
	return (int)n;
}

void test_file_write_one(int cnt, int sz, const unsigned char *data)
{
	char p1[256], p2[256];
	std::snprintf(p1, sizeof(p1), "%s/fw1_%u", g_tmpbase, rnd_u32());
	std::snprintf(p2, sizeof(p2), "%s/fw2_%u", g_tmpbase, rnd_u32());
	int fd1 = open(p1, O_RDWR | O_CREAT | O_TRUNC, 0644);
	int fd2 = open(p2, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (fd1 < 0 || fd2 < 0) {
		if (fd1 >= 0) close(fd1);
		if (fd2 >= 0) close(fd2);
		return;
	}

	GuardedBuf gref, gport;
	guard_fill(&gref);
	guard_fill(&gport);
	if (cnt > 0 && data != nullptr) {
		std::memcpy(buf_user(&gref), data, (std::size_t)cnt);
		std::memcpy(buf_user(&gport), data, (std::size_t)cnt);
	}

	int rem_ref = 0, rem_port = 0;
	int isempt_ref = 1, isempt_port = 1;
	case_inc(S_FILE_WRITE);
	int wref = ref_file_write(fd1, (char *)buf_user(&gref), cnt,
	    &rem_ref, &isempt_ref, sz, p1);
	int wport = port::file_write(fd2, (char *)buf_user(&gport), cnt,
	    &rem_port, &isempt_port, sz, p2);

	unsigned char b1[BUF_CAP], b2[BUF_CAP];
	int n1 = read_file_bytes(p1, b1, sizeof(b1));
	int n2 = read_file_bytes(p2, b2, sizeof(b2));

	if (wref != wport || rem_ref != rem_port || isempt_ref != isempt_port ||
	    n1 != n2 || std::memcmp(b1, b2, (std::size_t)(n1 > 0 ? n1 : 0)) != 0 ||
	    !guard_eq(&gref, &gport))
		fail_msg(S_FILE_WRITE, "write", "mismatch");

	close(fd1);
	close(fd2);
	unlink(p1);
	unlink(p2);
}

void test_file_write_edge(void)
{
	unsigned char zeros[4096] = {};
	unsigned char mixed[512];
	unsigned char highbit[256];
	for (int i = 0; i < 512; i++)
		mixed[i] = (unsigned char)(i & 0xff);
	for (int i = 0; i < 256; i++)
		highbit[i] = (unsigned char)(0x80 | (i & 0x7f));

	test_file_write_one(0, 4096, nullptr);
	test_file_write_one(1, 4096, mixed);
	test_file_write_one(1, 1, mixed);
	test_file_write_one(512, 4096, zeros);
	test_file_write_one(512, 4096, mixed);
	test_file_write_one(256, 256, highbit);
	test_file_write_one(4096, 4096, zeros);
	test_file_write_one(8192, 4096, mixed);
	test_file_write_one(4096, 512, zeros);
}

void test_file_write_random(void)
{
	unsigned char data[BUF_CAP];
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		int cnt = (int)(rnd_u32() % BUF_CAP);
		int sz = 1 + (int)(rnd_u32() % 4096u);
		fill_random_string(data, cnt, true);
		test_file_write_one(cnt, sz, data);
	}
}

void test_file_flush_one(int isempt)
{
	char p1[256], p2[256];
	std::snprintf(p1, sizeof(p1), "%s/ff1_%u", g_tmpbase, rnd_u32());
	std::snprintf(p2, sizeof(p2), "%s/ff2_%u", g_tmpbase, rnd_u32());
	int fd1 = open(p1, O_RDWR | O_CREAT | O_TRUNC, 0644);
	int fd2 = open(p2, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (fd1 < 0 || fd2 < 0) {
		if (fd1 >= 0) close(fd1);
		if (fd2 >= 0) close(fd2);
		return;
	}
	const char z[4096] = {};
	(void)write(fd1, z, 4096);
	(void)write(fd2, z, 4096);
	lseek(fd1, 0, SEEK_SET);
	lseek(fd2, 0, SEEK_SET);

	case_inc(S_FILE_FLUSH);
	ref_file_flush(fd1, p1, isempt);
	port::file_flush(fd2, p2, isempt);

	unsigned char b1[4096], b2[4096];
	int n1 = read_file_bytes(p1, b1, sizeof(b1));
	int n2 = read_file_bytes(p2, b2, sizeof(b2));
	if (n1 != n2 || std::memcmp(b1, b2, (std::size_t)(n1 > 0 ? n1 : 0)) != 0)
		fail_msg(S_FILE_FLUSH, "flush", "file mismatch");

	close(fd1);
	close(fd2);
	unlink(p1);
	unlink(p2);
}

void test_file_flush_edge(void)
{
	test_file_flush_one(0);
	test_file_flush_one(1);
}

void test_file_flush_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++)
		test_file_flush_one((int)(rnd_u32() & 1u));
}

void test_set_crc_one(const char *fname, int fdval, off_t size,
    const unsigned char *payload, int plen)
{
	char fref[512], fport[512];
	std::snprintf(fref, sizeof(fref), "%s/%s", g_tmpbase, fname);
	std::snprintf(fport, sizeof(fport), "%s/%s", g_tmpbase, fname);

	if (payload != nullptr && plen > 0) {
		int wfd = open(fref, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (wfd >= 0) {
			(void)write(wfd, payload, (std::size_t)plen);
			close(wfd);
		}
		wfd = open(fport, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (wfd >= 0) {
			(void)write(wfd, payload, (std::size_t)plen);
			close(wfd);
		}
	}

	port::ARCHD aref{}, aport{};
	init_archd(&aref, fref, PAX_REG);
	init_archd(&aport, fport, PAX_REG);
	aref.sb.st_size = size;
	aport.sb.st_size = size;
	aref.org_name = fref;
	aport.org_name = fport;

	int fdr, fdp;
	if (fdval < 0) {
		fdr = fdp = fdval;
	} else {
		fdr = open(fref, O_RDONLY);
		fdp = open(fport, O_RDONLY);
	}
	case_inc(S_SET_CRC);
	int rref = ref_set_crc(&aref, fdr);
	int rport = port::set_crc(&aport, fdp);
	if (rref != rport || aref.crc != aport.crc)
		fail_msg(S_SET_CRC, fname, "mismatch");
	if (fdr >= 0) close(fdr);
	if (fdp >= 0) close(fdp);
	unlink(fref);
	unlink(fport);
}

void test_set_crc_edge(void)
{
	test_set_crc_one("crc_neg", -1, 0, nullptr, 0);
	const unsigned char data[] = "crc test payload";
	test_set_crc_one("crc_edge", 1, (off_t)(sizeof(data) - 1),
	    data, (int)(sizeof(data) - 1));
}

void test_set_crc_random(void)
{
	unsigned char payload[256];
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		if ((rnd_u32() & 7u) == 0) {
			test_set_crc_one("crc_neg", -1, 0, nullptr, 0);
			continue;
		}
		int n = (int)(rnd_u32() % 200u) + 1;
		fill_random_string(payload, n, true);
		char fname[64];
		std::snprintf(fname, sizeof(fname), "crc_%u.dat", rnd_u32());
		test_set_crc_one(fname, 1, n, payload, n);
	}
}

} // namespace

int main(void)
{
	std::snprintf(g_tmpbase, sizeof(g_tmpbase), "/tmp/pbsd_b0200_%d", getpid());
	mkdir(g_tmpbase, 0700);

	test_ftree_add_edge();
	test_sel_chk_edge();
	test_usr_add_edge();
	test_grp_add_edge();
	test_trng_add_edge();
	test_str_sec_edge();
	test_usr_match_edge();
	test_grp_match_edge();
	test_trng_match_edge();
	test_file_write_edge();
	test_file_flush_edge();
	test_set_crc_edge();

	test_ftree_add_random();
	test_sel_chk_random();
	test_usr_add_random();
	test_grp_add_random();
	test_trng_add_random();
	test_str_sec_random();
	test_usr_match_random();
	test_grp_match_random();
	test_trng_match_random();
	test_file_write_random();
	test_file_flush_random();
	test_set_crc_random();

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
