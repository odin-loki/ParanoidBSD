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
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.bin.pax.b0200;

namespace port = pbsd::bin_pax::b0200;

extern "C" {
int ref_ftree_start(void);
int ref_ftree_add(char *, int);
void ref_ftree_sel(port::ARCHD *);
void ref_ftree_notsel(void);
void ref_ftree_chk(void);
int ref_next_file(port::ARCHD *);
int ref_sel_chk(port::ARCHD *);
int ref_usr_add(char *);
int ref_grp_add(char *);
int ref_trng_add(char *);
int ref_main(int, char **);
void ref_sig_cleanup(int);
int ref_file_creat(port::ARCHD *);
void ref_file_close(port::ARCHD *, int);
int ref_lnk_creat(port::ARCHD *);
int ref_cross_lnk(port::ARCHD *);
int ref_chk_same(port::ARCHD *);
int ref_node_creat(port::ARCHD *);
int ref_unlnk_exist(char *, int);
int ref_chk_path(char *, uid_t, gid_t);
void ref_set_ftime(char *, time_t, time_t, int);
int ref_set_ids(char *, uid_t, gid_t);
void ref_set_pmode(char *, mode_t);
int ref_file_write(int, char *, int, int *, int *, int, char *);
void ref_file_flush(int, char *, int);
void ref_rdfile_close(port::ARCHD *, int *);
int ref_set_crc(port::ARCHD *, int);

extern int act;
extern int cwdfd;
extern int dflag;
extern int nflag;
extern int tflag;
extern int kflag;
extern int nodirs;
extern int pids;
extern int pmode;
extern int patime;
extern int pmtime;
extern int Lflag;
extern int Hflag;
extern int Xflag;
extern int exit_val;
extern const char *argv0;
extern FILE *listf;
extern char *gzip_program;
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
	S_FTREE_START, S_FTREE_ADD, S_FTREE_SEL, S_FTREE_NOTSEL, S_FTREE_CHK,
	S_NEXT_FILE, S_SEL_CHK, S_USR_ADD, S_GRP_ADD, S_TRNG_ADD,
	S_MAIN, S_SIG_CLEANUP,
	S_FILE_CREAT, S_FILE_CLOSE, S_LNK_CREAT, S_CROSS_LNK, S_CHK_SAME,
	S_NODE_CREAT, S_UNLNK_EXIST, S_CHK_PATH, S_SET_FTIME, S_SET_IDS,
	S_SET_PMODE, S_FILE_WRITE, S_FILE_FLUSH, S_RDFILE_CLOSE, S_SET_CRC,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int reported;
};

Stats g_stat[NSTAT] = {
	{ "ftree_start",   0, 0, 0 },
	{ "ftree_add",     0, 0, 0 },
	{ "ftree_sel",     0, 0, 0 },
	{ "ftree_notsel",  0, 0, 0 },
	{ "ftree_chk",     0, 0, 0 },
	{ "next_file",     0, 0, 0 },
	{ "sel_chk",       0, 0, 0 },
	{ "usr_add",       0, 0, 0 },
	{ "grp_add",       0, 0, 0 },
	{ "trng_add",      0, 0, 0 },
	{ "main",          0, 0, 0 },
	{ "sig_cleanup",   0, 0, 0 },
	{ "file_creat",    0, 0, 0 },
	{ "file_close",    0, 0, 0 },
	{ "lnk_creat",     0, 0, 0 },
	{ "cross_lnk",     0, 0, 0 },
	{ "chk_same",      0, 0, 0 },
	{ "node_creat",    0, 0, 0 },
	{ "unlnk_exist",   0, 0, 0 },
	{ "chk_path",      0, 0, 0 },
	{ "set_ftime",     0, 0, 0 },
	{ "set_ids",       0, 0, 0 },
	{ "set_pmode",     0, 0, 0 },
	{ "file_write",    0, 0, 0 },
	{ "file_flush",    0, 0, 0 },
	{ "rdfile_close",  0, 0, 0 },
	{ "set_crc",       0, 0, 0 },
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
	a->sb.st_blksize = 4096;
}

bool archd_eq(const port::ARCHD *a, const port::ARCHD *b)
{
	if (a->nlen != b->nlen || a->ln_nlen != b->ln_nlen ||
	    a->type != b->type || a->skip != b->skip || a->pad != b->pad ||
	    a->crc != b->crc)
		return false;
	if (std::memcmp(a->name, b->name, sizeof(a->name)) != 0)
		return false;
	if (std::memcmp(a->ln_name, b->ln_name, sizeof(a->ln_name)) != 0)
		return false;
	if (std::memcmp(&a->sb, &b->sb, sizeof(a->sb)) != 0)
		return false;
	return true;
}

void setup_ftree_flags(void)
{
	Lflag = port::Lflag = 0;
	Hflag = port::Hflag = 0;
	Xflag = port::Xflag = 0;
	tflag = port::tflag = 0;
	nflag = port::nflag = 0;
	dflag = port::dflag = 0;
	cwdfd = port::cwdfd = open(".", O_RDONLY | O_CLOEXEC);
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
	char trail[64];
	std::snprintf(trail, sizeof(trail), "%s/foo/", mk_temp_dir().c_str());

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

	if (!trail[0])
		return;
	char tref[256], tport[256];
	std::strcpy(tref, trail);
	std::strcpy(tport, trail);
	test_ftree_add_one(S_FTREE_ADD, tref, tport, 0);
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

void test_ftree_sel_edge(void)
{
	port::ARCHD aref{}, aport{};
	init_archd(&aref, "f", port::PAX_REG);
	init_archd(&aport, "f", port::PAX_REG);

	nflag = port::nflag = 0;
	dflag = port::dflag = 0;
	case_inc(S_FTREE_SEL);
	ref_ftree_sel(&aref);
	port::ftree_sel(&aport);

	nflag = port::nflag = 1;
	case_inc(S_FTREE_SEL);
	ref_ftree_sel(&aref);
	port::ftree_sel(&aport);

	dflag = port::dflag = 1;
	aref.type = port::PAX_DIR;
	aport.type = port::PAX_DIR;
	case_inc(S_FTREE_SEL);
	ref_ftree_sel(&aref);
	port::ftree_sel(&aport);
}

void test_ftree_sel_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		port::ARCHD aref{}, aport{};
		init_archd(&aref, "f", port::PAX_REG);
		init_archd(&aport, "f", port::PAX_REG);
		nflag = port::nflag = (int)(rnd_u32() & 1u);
		dflag = port::dflag = (int)(rnd_u32() & 1u);
		if (rnd_u32() & 1u) {
			aref.type = port::PAX_DIR;
			aport.type = port::PAX_DIR;
		}
		case_inc(S_FTREE_SEL);
		ref_ftree_sel(&aref);
		port::ftree_sel(&aport);
	}
}

void test_ftree_notsel_edge(void)
{
	case_inc(S_FTREE_NOTSEL);
	ref_ftree_notsel();
	port::ftree_notsel();
}

void test_ftree_notsel_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		case_inc(S_FTREE_NOTSEL);
		ref_ftree_notsel();
		port::ftree_notsel();
	}
}

void test_ftree_chk_edge(void)
{
	tflag = port::tflag = 0;
	case_inc(S_FTREE_CHK);
	ref_ftree_chk();
	port::ftree_chk();
}

void test_ftree_chk_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		tflag = port::tflag = (int)(rnd_u32() & 1u);
		case_inc(S_FTREE_CHK);
		ref_ftree_chk();
		port::ftree_chk();
	}
}

void test_ftree_walk_once(const char *label)
{
	std::string td = mk_temp_dir();
	if (td.empty())
		return;
	char fpath[512];
	std::snprintf(fpath, sizeof(fpath), "%s/file.txt", td.c_str());
	int fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd >= 0) {
		const char msg[] = "data";
		(void)write(fd, msg, sizeof(msg) - 1);
		close(fd);
	}

	char sref[512], sport[512];
	std::strcpy(sref, td.c_str());
	std::strcpy(sport, td.c_str());

	setup_ftree_flags();
	case_inc(S_FTREE_ADD);
	if (ref_ftree_add(sref, 0) != port::ftree_add(sport, 0))
		fail_msg(S_FTREE_ADD, label, "add mismatch");

	case_inc(S_FTREE_START);
	if (ref_ftree_start() != port::ftree_start())
		fail_msg(S_FTREE_START, label, "start mismatch");

	for (;;) {
		port::ARCHD aref{}, aport{};
		case_inc(S_NEXT_FILE);
		int rref = ref_next_file(&aref);
		int rport = port::next_file(&aport);
		if (rref != rport || !archd_eq(&aref, &aport))
			fail_msg(S_NEXT_FILE, label, "next_file mismatch");
		if (rref < 0)
			break;
	}
}

void test_ftree_walk_edge(void)
{
	test_ftree_walk_once("walk");
}

void test_ftree_walk_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++)
		test_ftree_walk_once("random");
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
	init_archd(&aref, "f", port::PAX_REG);
	init_archd(&aport, "f", port::PAX_REG);
	test_sel_chk_one(&aref, &aport);

	char s[32];
	std::snprintf(s, sizeof(s), "#%u", (unsigned)getuid());
	char s2[32];
	std::strcpy(s2, s);
	ref_usr_add(s);
	port::usr_add(s2);
	aref.sb.st_uid = getuid() + 1;
	aport.sb.st_uid = getuid() + 1;
	test_sel_chk_one(&aref, &aport);
	aref.sb.st_uid = getuid();
	aport.sb.st_uid = getuid();
	test_sel_chk_one(&aref, &aport);
}

void test_sel_chk_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char sref[32], sport[32];
		port::ARCHD aref{}, aport{};
		init_archd(&aref, "f", port::PAX_REG);
		init_archd(&aport, "f", port::PAX_REG);
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
		if (rnd_u32() & 3u) == 0) {
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

	char s3ref[32], s3port[32];
	std::strcpy(s3ref, "bad!");
	std::strcpy(s3port, "bad!");
	test_trng_add_one(s3ref, s3port);
}

void test_trng_add_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char sref[128], sport[128];
		int hr = rnd_u32() % 24u;
		int mn = rnd_u32() % 60u;
		std::snprintf(sref, sizeof(sref), "%02d%02d", hr, mn);
		std::snprintf(sport, sizeof(sport), "%02d%02d", hr, mn);
		if (rnd_u32() & 1u) {
			char up[64];
			std::snprintf(up, sizeof(up), ",%02d%02d", hr, (mn + 1) % 60);
			std::strcat(sref, up);
			std::strcat(sport, up);
		}
		if (rnd_u32() & 3u) == 0) {
			std::strcat(sref, "/m");
			std::strcat(sport, "/m");
		}
		test_trng_add_one(sref, sport);
	}
}

void test_main_edge(void)
{
	char av0[] = "pax";
	char *av[] = { av0, nullptr };
	case_inc(S_MAIN);
	exit_val = port::exit_val = 0;
	listf = port::listf = stderr;
	gzip_program = port::gzip_program = nullptr;
	if (ref_main(1, av) != port::main(1, av))
		fail_msg(S_MAIN, "edge", "return mismatch");
}

void test_main_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char av0[] = "pax";
		char *av[] = { av0, nullptr };
		case_inc(S_MAIN);
		exit_val = port::exit_val = 0;
		listf = port::listf = stderr;
		gzip_program = port::gzip_program = nullptr;
		if (ref_main(1, av) != port::main(1, av))
			fail_msg(S_MAIN, "random", "return mismatch");
	}
}

void test_sig_cleanup_one(int sig)
{
	pid_t pref = fork();
	if (pref == 0) {
		ref_sig_cleanup(sig);
		_exit(99);
	}
	pid_t pport = fork();
	if (pport == 0) {
		port::sig_cleanup(sig);
		_exit(99);
	}
	int stref = 0, stport = 0;
	waitpid(pref, &stref, 0);
	waitpid(pport, &stport, 0);
	case_inc(S_SIG_CLEANUP);
	if (WEXITSTATUS(stref) != WEXITSTATUS(stport))
		fail_msg(S_SIG_CLEANUP, "exit", "child exit mismatch");
}

void test_sig_cleanup_edge(void)
{
	test_sig_cleanup_one(SIGINT);
	test_sig_cleanup_one(SIGXCPU);
}

void test_sig_cleanup_random(void)
{
	int sigs[] = { SIGINT, SIGTERM, SIGHUP, SIGQUIT, SIGXCPU };
	for (unsigned i = 0; i < RAND_ITERS; i++)
		test_sig_cleanup_one(sigs[rnd_u32() % 5u]);
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
	    n1 != n2 || std::memcmp(b1, b2, (std::size_t)n1) != 0 ||
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
	for (int i = 0; i < 512; i++)
		mixed[i] = (unsigned char)(i & 0xff);

	test_file_write_one(0, 4096, nullptr);
	test_file_write_one(1, 4096, mixed);
	test_file_write_one(512, 4096, zeros);
	test_file_write_one(512, 4096, mixed);
	test_file_write_one(4096, 4096, zeros);
	test_file_write_one(8192, 4096, mixed);
}

void test_file_write_random(void)
{
	unsigned char data[BUF_CAP];
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		int cnt = (int)(rnd_u32() % BUF_CAP);
		int sz = 256 + (int)(rnd_u32() % 4096u);
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
	if (n1 != n2 || std::memcmp(b1, b2, (std::size_t)n1) != 0)
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

void test_file_ops_in_dir(const char *td)
{
	char fref[512], fport[512];
	std::snprintf(fref, sizeof(fref), "%s/f.txt", td);
	std::snprintf(fport, sizeof(fport), "%s/f.txt", td);

	port::ARCHD aref{}, aport{};
	init_archd(&aref, fref, port::PAX_REG);
	init_archd(&aport, fport, port::PAX_REG);
	aref.sb.st_mode = S_IFREG | 0644;
	aport.sb.st_mode = S_IFREG | 0644;

	nodirs = port::nodirs = 0;
	kflag = port::kflag = 0;
	pids = port::pids = 0;
	pmode = port::pmode = 0;
	patime = port::patime = 0;
	pmtime = port::pmtime = 0;
	tflag = port::tflag = 0;
	argv0 = port::argv0 = "pax";

	case_inc(S_FILE_CREAT);
	int fdref = ref_file_creat(&aref);
	int fdport = port::file_creat(&aport);
	if (fdref != fdport)
		fail_msg(S_FILE_CREAT, "creat", "fd mismatch");

	case_inc(S_FILE_CLOSE);
	ref_file_close(&aref, fdref);
	port::file_close(&aport, fdport);

	case_inc(S_UNLNK_EXIST);
	if (ref_unlnk_exist(fref, port::PAX_REG) !=
	    port::unlnk_exist(fport, port::PAX_REG))
		fail_msg(S_UNLNK_EXIST, "unlink", "mismatch");

	char sub[512];
	std::snprintf(sub, sizeof(sub), "%s/sub/miss", td);
	case_inc(S_CHK_PATH);
	if (ref_chk_path(sub, getuid(), getgid()) !=
	    port::chk_path(sub, getuid(), getgid()))
		fail_msg(S_CHK_PATH, "path", "mismatch");

	case_inc(S_SET_FTIME);
	ref_set_ftime(fref, 1000, 2000, 1);
	port::set_ftime(fport, 1000, 2000, 1);

	case_inc(S_SET_IDS);
	if (ref_set_ids(fref, getuid(), getgid()) !=
	    port::set_ids(fport, getuid(), getgid()))
		fail_msg(S_SET_IDS, "ids", "mismatch");

	case_inc(S_SET_PMODE);
	ref_set_pmode(fref, 0644);
	port::set_pmode(fport, 0644);

	int fd_rd = open(fref, O_RDONLY);
	case_inc(S_RDFILE_CLOSE);
	int fdr = fd_rd;
	ref_rdfile_close(&aref, &fdr);
	fdr = fd_rd;
	port::rdfile_close(&aport, &fdr);
}

void test_file_ops_edge(void)
{
	std::string td = mk_temp_dir();
	if (!td.empty())
		test_file_ops_in_dir(td.c_str());
}

void test_file_ops_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::string td = mk_temp_dir();
		if (!td.empty())
			test_file_ops_in_dir(td.c_str());
	}
}

void test_lnk_edge(void)
{
	std::string td = mk_temp_dir();
	if (td.empty())
		return;
	char tgt[512], lnk[512];
	std::snprintf(tgt, sizeof(tgt), "%s/target", td.c_str());
	std::snprintf(lnk, sizeof(lnk), "%s/link", td.c_str());
	int fd = open(tgt, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd >= 0)
		close(fd);

	port::ARCHD aref{}, aport{};
	init_archd(&aref, lnk, port::PAX_REG);
	init_archd(&aport, lnk, port::PAX_REG);
	aref.ln_name[0] = '\0';
	std::strncpy(aref.ln_name, tgt, sizeof(aref.ln_name) - 1);
	std::strncpy(aport.ln_name, tgt, sizeof(aport.ln_name) - 1);
	aref.org_name = tgt;
	aport.org_name = tgt;

	kflag = port::kflag = 0;
	nodirs = port::nodirs = 0;

	case_inc(S_LNK_CREAT);
	if (ref_lnk_creat(&aref) != port::lnk_creat(&aport))
		fail_msg(S_LNK_CREAT, "lnk", "mismatch");

	case_inc(S_CROSS_LNK);
	if (ref_cross_lnk(&aref) != port::cross_lnk(&aport))
		fail_msg(S_CROSS_LNK, "cross", "mismatch");

	case_inc(S_CHK_SAME);
	if (ref_chk_same(&aref) != port::chk_same(&aport))
		fail_msg(S_CHK_SAME, "same", "mismatch");
}

void test_lnk_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++)
		test_lnk_edge();
}

void test_node_creat_edge(void)
{
	std::string td = mk_temp_dir();
	if (td.empty())
		return;
	char dref[512], dport[512];
	std::snprintf(dref, sizeof(dref), "%s/ndir", td.c_str());
	std::snprintf(dport, sizeof(dport), "%s/ndir", td.c_str());

	port::ARCHD aref{}, aport{};
	init_archd(&aref, dref, port::PAX_DIR);
	init_archd(&aport, dport, port::PAX_DIR);
	aref.sb.st_mode = S_IFDIR | 0755;
	aport.sb.st_mode = S_IFDIR | 0755;
	argv0 = port::argv0 = "pax";
	nodirs = port::nodirs = 0;
	pids = port::pids = 0;
	pmode = port::pmode = 0;
	patime = port::patime = 0;
	pmtime = port::pmtime = 0;

	case_inc(S_NODE_CREAT);
	if (ref_node_creat(&aref) != port::node_creat(&aport))
		fail_msg(S_NODE_CREAT, "dir", "mismatch");
}

void test_node_creat_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++)
		test_node_creat_edge();
}

void test_set_crc_edge(void)
{
	std::string td = mk_temp_dir();
	if (td.empty())
		return;
	char fref[512], fport[512];
	std::snprintf(fref, sizeof(fref), "%s/crc.dat", td.c_str());
	std::snprintf(fport, sizeof(fport), "%s/crc.dat", td.c_str());
	const char data[] = "crc test payload";
	int fd = open(fref, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		return;
	(void)write(fd, data, sizeof(data) - 1);
	close(fd);

	port::ARCHD aref{}, aport{};
	init_archd(&aref, fref, port::PAX_REG);
	init_archd(&aport, fport, port::PAX_REG);
	aref.sb.st_size = sizeof(data) - 1;
	aport.sb.st_size = sizeof(data) - 1;
	aref.org_name = fref;
	aport.org_name = fport;

	int fdr = open(fref, O_RDONLY);
	int fdp = open(fport, O_RDONLY);
	case_inc(S_SET_CRC);
	int rref = ref_set_crc(&aref, fdr);
	int rport = port::set_crc(&aport, fdp);
	if (rref != rport || aref.crc != aport.crc)
		fail_msg(S_SET_CRC, "edge", "mismatch");
	if (fdr >= 0) close(fdr);
	if (fdp >= 0) close(fdp);
}

void test_set_crc_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++)
		test_set_crc_edge();
}

} // namespace

int main(void)
{
	std::snprintf(g_tmpbase, sizeof(g_tmpbase), "/tmp/pbsd_b0200_%d", getpid());
	mkdir(g_tmpbase, 0700);

	test_ftree_add_edge();
	test_ftree_sel_edge();
	test_ftree_notsel_edge();
	test_ftree_chk_edge();
	test_ftree_walk_edge();
	test_sel_chk_edge();
	test_usr_add_edge();
	test_grp_add_edge();
	test_trng_add_edge();
	test_main_edge();
	test_sig_cleanup_edge();
	test_file_write_edge();
	test_file_flush_edge();
	test_file_ops_edge();
	test_lnk_edge();
	test_node_creat_edge();
	test_set_crc_edge();

	test_ftree_add_random();
	test_ftree_sel_random();
	test_ftree_notsel_random();
	test_ftree_chk_random();
	test_ftree_walk_random();
	test_sel_chk_random();
	test_usr_add_random();
	test_grp_add_random();
	test_trng_add_random();
	test_main_random();
	test_sig_cleanup_random();
	test_file_write_random();
	test_file_flush_random();
	test_file_ops_random();
	test_lnk_random();
	test_node_creat_random();
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
