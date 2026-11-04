/*
 * PBSD batch b0210 -- differential harness for bin/pkill.
 *
 * Every ported function is run side by side with the ref_ oracle:
 *
 *   usage()	-- diagnostic text and exit status, over both pgrep states.
 *   makelist()	-- exit status, diagnostic text, the complete resulting list
 *		   (li_number for every entry, li_name for LT_CLASS) and the
 *		   ENTIRE input buffer, guard bytes included, because strsep()
 *		   rewrites the caller's string in place.
 *   takepid()	-- exit status, diagnostic text and the returned pid.
 *
 * Buffers are always allocated in pairs, pre-filled with the guard byte 0x7f,
 * seeded with identical input and compared in full afterwards.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <vector>

import pbsd.bin.pkill.b0210;

namespace P = pbsd::bin_pkill::b0210;

extern "C" {

struct ref_list {
	struct {
		struct ref_list *sle_next;
	} li_chain;
	long	li_number;
	char	*li_name;
};

struct ref_listhead {
	struct ref_list *slh_first;
};

int		ref_call_usage(void);
int		ref_call_makelist(struct ref_listhead *, int, char *);
int		ref_call_takepid(const char *, int, int *);
const char	*ref_trap_message(void);
void		ref_set_pgrep(int);
void		ref_set_mypid(pid_t);

}

/* Mirror of the port's enum listtype; both wrappers take a plain int. */
enum {
	LT_GENERIC = 0,
	LT_USER = 1,
	LT_GROUP = 2,
	LT_TTY = 3,
	LT_PGRP = 4,
	LT_JAIL = 5,
	LT_SID = 6,
	LT_CLASS = 7
};

static const char *const type_name[8] = {
	"LT_GENERIC", "LT_USER", "LT_GROUP", "LT_TTY",
	"LT_PGRP", "LT_JAIL", "LT_SID", "LT_CLASS"
};

/* ------------------------------------------------------------------ */
/* Bookkeeping								*/
/* ------------------------------------------------------------------ */

struct Fn {
	const char	*name;
	long		cases;
	long		fails;
	long		printed;
};

static Fn fn_usage = { "usage", 0, 0, 0 };
static Fn fn_makelist = { "makelist", 0, 0, 0 };
static Fn fn_takepid = { "takepid", 0, 0, 0 };

static void
fail(Fn &f, const std::string &detail)
{

	f.fails++;
	if (f.printed < 10) {
		f.printed++;
		std::fprintf(stderr, "FAIL [%s] %s\n", f.name, detail.c_str());
	} else if (f.printed == 10) {
		f.printed++;
		std::fprintf(stderr, "FAIL [%s] ... further failures muted\n",
		    f.name);
	}
}

static std::string
vis(const char *s, size_t n)
{
	static const char hex[] = "0123456789abcdef";
	std::string out;

	if (s == nullptr)
		return ("<NULL>");
	out += '"';
	for (size_t i = 0; i < n; i++) {
		unsigned char c = (unsigned char)s[i];

		if (c == '\\' || c == '"') {
			out += '\\';
			out += (char)c;
		} else if (c >= 0x20 && c < 0x7f) {
			out += (char)c;
		} else {
			out += "\\x";
			out += hex[c >> 4];
			out += hex[c & 15];
		}
	}
	out += '"';
	return (out);
}

static std::string
vis(const char *s)
{

	return (s == nullptr ? std::string("<NULL>") : vis(s, std::strlen(s)));
}

/* ------------------------------------------------------------------ */
/* Deterministic PRNG							*/
/* ------------------------------------------------------------------ */

static uint64_t rng_state = 0x0123456789abcdefULL;

static void
rng_seed(uint64_t s)
{

	rng_state = s ? s : 0x9e3779b97f4a7c15ULL;
}

static uint64_t
rnd(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x);
}

static uint32_t
rnd_below(uint32_t n)
{

	return (n == 0 ? 0 : (uint32_t)(rnd() % n));
}

/* ------------------------------------------------------------------ */
/* usage()								*/
/* ------------------------------------------------------------------ */

static void
one_usage(int pg)
{
	Fn &f = fn_usage;

	f.cases++;
	P::set_pgrep(pg);
	ref_set_pgrep(pg);

	int pc = P::call_usage();
	std::string pm = P::trap_message();
	int rc = ref_call_usage();
	std::string rm = ref_trap_message();

	if (pc != rc) {
		char b[128];
		std::snprintf(b, sizeof(b), "pgrep=%d status port=%d ref=%d",
		    pg, pc, rc);
		fail(f, b);
		return;
	}
	if (pm != rm) {
		fail(f, "pgrep=" + std::to_string(pg) + " message port=" +
		    vis(pm.data(), pm.size()) + " ref=" +
		    vis(rm.data(), rm.size()));
	}
}

static void
test_usage(void)
{

	one_usage(0);
	one_usage(1);
	one_usage(-1);
	one_usage(2);
	one_usage(INT_MIN);
	one_usage(INT_MAX);
}

/* ------------------------------------------------------------------ */
/* makelist()								*/
/* ------------------------------------------------------------------ */

static const size_t BUFN = 8192;
static const size_t LEAD = 16;		/* guard bytes before the string */
static const size_t MAXIN = BUFN - LEAD - 64;

static long
list_len_port(const P::listhead *h)
{
	long n = 0;

	for (const P::list *e = h->slh_first; e != nullptr;
	    e = e->li_chain.sle_next)
		n++;
	return (n);
}

static long
list_len_ref(const ref_listhead *h)
{
	long n = 0;

	for (const ref_list *e = h->slh_first; e != nullptr;
	    e = e->li_chain.sle_next)
		n++;
	return (n);
}

static void
free_list_port(P::listhead *h, bool names)
{
	P::list *e = h->slh_first;

	while (e != nullptr) {
		P::list *nx = e->li_chain.sle_next;

		if (names)
			std::free(e->li_name);
		std::free(e);
		e = nx;
	}
	h->slh_first = nullptr;
}

static void
free_list_ref(ref_listhead *h, bool names)
{
	ref_list *e = h->slh_first;

	while (e != nullptr) {
		ref_list *nx = e->li_chain.sle_next;

		if (names)
			std::free(e->li_name);
		std::free(e);
		e = nx;
	}
	h->slh_first = nullptr;
}

static unsigned char pbuf[BUFN];
static unsigned char obuf[BUFN];

static void
one_makelist(int type, const char *input, size_t inlen, int pg, pid_t mp)
{
	Fn &f = fn_makelist;

	if (input != nullptr && inlen > MAXIN)
		inlen = MAXIN;

	f.cases++;

	std::memset(pbuf, 0x7f, BUFN);
	std::memset(obuf, 0x7f, BUFN);

	char *psrc = nullptr;
	char *osrc = nullptr;
	if (input != nullptr) {
		std::memcpy(pbuf + LEAD, input, inlen);
		pbuf[LEAD + inlen] = '\0';
		std::memcpy(obuf + LEAD, input, inlen);
		obuf[LEAD + inlen] = '\0';
		psrc = (char *)pbuf + LEAD;
		osrc = (char *)obuf + LEAD;
	}

	P::set_pgrep(pg);
	ref_set_pgrep(pg);
	P::set_mypid(mp);
	ref_set_mypid(mp);

	P::listhead ph;
	ph.slh_first = nullptr;
	ref_listhead oh;
	oh.slh_first = nullptr;

	int pc = P::call_makelist(&ph, type, psrc);
	std::string pm = P::trap_message();
	int rc = ref_call_makelist(&oh, type, osrc);
	std::string rm = ref_trap_message();

	const bool names = (type == LT_CLASS);
	std::string ctx = std::string("type=") + type_name[type] + " src=" +
	    (input == nullptr ? std::string("<NULL>") : vis(input, inlen)) +
	    " pgrep=" + std::to_string(pg) + " mypid=" +
	    std::to_string((long)mp);

	bool bad = false;

	if (pc != rc) {
		fail(f, ctx + " status port=" + std::to_string(pc) + " ref=" +
		    std::to_string(rc));
		bad = true;
	}
	if (pm != rm) {
		fail(f, ctx + " message port=" + vis(pm.data(), pm.size()) +
		    " ref=" + vis(rm.data(), rm.size()));
		bad = true;
	}
	if (std::memcmp(pbuf, obuf, BUFN) != 0) {
		size_t i = 0;
		while (i < BUFN && pbuf[i] == obuf[i])
			i++;
		char b[160];
		std::snprintf(b, sizeof(b),
		    " buffer diverges at offset %zu port=0x%02x ref=0x%02x",
		    i, pbuf[i], obuf[i]);
		fail(f, ctx + b);
		bad = true;
	}

	long pn = list_len_port(&ph);
	long rn = list_len_ref(&oh);
	if (pn != rn) {
		fail(f, ctx + " list length port=" + std::to_string(pn) +
		    " ref=" + std::to_string(rn));
		bad = true;
	} else if (!bad) {
		const P::list *pe = ph.slh_first;
		const ref_list *re = oh.slh_first;
		long idx = 0;

		while (pe != nullptr && re != nullptr) {
			if (pe->li_number != re->li_number) {
				fail(f, ctx + " entry " + std::to_string(idx) +
				    " li_number port=" +
				    std::to_string(pe->li_number) + " ref=" +
				    std::to_string(re->li_number));
				break;
			}
			if (names) {
				if ((pe->li_name == nullptr) !=
				    (re->li_name == nullptr)) {
					fail(f, ctx + " entry " +
					    std::to_string(idx) +
					    " li_name nullness differs");
					break;
				}
				if (pe->li_name != nullptr &&
				    std::strcmp(pe->li_name, re->li_name)
				    != 0) {
					fail(f, ctx + " entry " +
					    std::to_string(idx) +
					    " li_name port=" +
					    vis(pe->li_name) + " ref=" +
					    vis(re->li_name));
					break;
				}
			}
			pe = pe->li_chain.sle_next;
			re = re->li_chain.sle_next;
			idx++;
		}
	}

	free_list_port(&ph, names);
	free_list_ref(&oh, names);
}

static void
one_makelist(int type, const char *input)
{

	one_makelist(type, input, input == nullptr ? 0 : std::strlen(input),
	    0, getpid());
}

/* Fixed, hand-written makelist inputs. */
static const char *const fixed_src[] = {
	nullptr,
	"",
	",",
	",,",
	",,,,",
	"a,",
	",a",
	"a,,b",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"-0",
	"-1",
	"-2",
	"+1",
	"+0",
	"4294967295",
	"4294967296",
	"2147483647",
	"2147483648",
	"-2147483648",
	"9223372036854775807",
	"99999999999999999999",
	"-99999999999999999999",
	"0x0",
	"0x1f",
	"0X10",
	"010",
	"08",
	"0b1",
	" 5",
	"5 ",
	"\t7",
	"7\t",
	"5x",
	"x5",
	"abc",
	"none",
	"any",
	"NONE",
	"ANY",
	"co",
	"-",
	"--",
	"console",
	"null",
	"zero",
	"full",
	"random",
	"urandom",
	"tty",
	"tty0",
	"ttynull",
	"shm",
	"pts",
	"pts/0",
	"pts/1",
	"stdin",
	"fd",
	"loop0",
	"null/x",
	"zero/y/z",
	"root",
	"nobody",
	"daemon",
	"bin",
	"sys",
	"nosuchuser_zzz",
	"nosuchgroup_zzz",
	"root,nobody",
	"0,1,2",
	"-1,0,1",
	"none,any",
	"co,null,zero",
	"5,x",
	"x,5",
	"1,,2",
	"\x80",
	"\xff",
	"\x80\x81\x82",
	"5\x80",
	"\x80" "5",
	"\xc3\xa9",
	"\xff,\xfe",
	"\x7f",
	"\x01",
	"\x1f",
	"0\x80",
	"root\xff",
	"a",
	"ab",
	"abcdefghijklmnopqrstuvwxyz",
	"0,0,0,0,0,0,0,0",
	"none,5x,any",
	"12345678901234567890,1",
};

static void
test_makelist_fixed(void)
{
	const pid_t self = getpid();
	const pid_t pids[] = { self, 0, 1, -1, 999999, getppid() };

	for (size_t i = 0; i < sizeof(fixed_src) / sizeof(fixed_src[0]); i++) {
		for (int t = 0; t < 8; t++) {
			const char *s = fixed_src[i];
			size_t n = (s == nullptr) ? 0 : std::strlen(s);

			one_makelist(t, s, n, 0, self);
			one_makelist(t, s, n, 1, self);
		}
	}

	/* getsid()/getpgrp() dependent paths with a spread of mypid values. */
	for (size_t k = 0; k < sizeof(pids) / sizeof(pids[0]); k++) {
		one_makelist(LT_SID, "0", 1, 0, pids[k]);
		one_makelist(LT_SID, "1", 1, 0, pids[k]);
		one_makelist(LT_SID, "0,0", 3, 0, pids[k]);
		one_makelist(LT_PGRP, "0", 1, 0, pids[k]);
		one_makelist(LT_PGRP, "1", 1, 0, pids[k]);
	}

	/* Boundary lengths around the MAXPATHLEN snprintf() truncation. */
	static const size_t lens[] = { 0, 1, 2, 3, 4085, 4086, 4087, 4088,
	    4089, 4090, 4091, 4092, 4095, 4096, 4097, 5000 };
	for (size_t k = 0; k < sizeof(lens) / sizeof(lens[0]); k++) {
		std::string s(lens[k], 'a');

		one_makelist(LT_TTY, s.c_str(), s.size(), 0, getpid());
		one_makelist(LT_CLASS, s.c_str(), s.size(), 0, getpid());
		std::string d(lens[k], '7');
		one_makelist(LT_TTY, d.c_str(), d.size(), 0, getpid());
		one_makelist(LT_GENERIC, d.c_str(), d.size(), 0, getpid());
	}
}

static const char *const word_pool[] = {
	"none", "any", "co", "-", "--", "console", "null", "zero", "full",
	"random", "urandom", "tty", "tty0", "ttynull", "shm", "pts", "pts/0",
	"pts/1", "pts/2", "pts/9", "stdin", "stdout", "fd", "loop0", "null/x",
	"root", "nobody", "daemon", "bin", "sys", "sync", "games", "nogroup",
	"nosuchuser_zzz", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	"-1", "-2", "+1", "0x1f", "010", "99999", "100000", "4294967295",
	"x", "5x", "x5", "abc", "", "0,1", " 5", "5 ",
};

static void
gen_token(std::string &out)
{
	uint32_t kind = rnd_below(100);

	if (kind < 30) {
		out += word_pool[rnd_below(
		    (uint32_t)(sizeof(word_pool) / sizeof(word_pool[0])))];
	} else if (kind < 50) {
		if (rnd_below(3) == 0)
			out += (rnd_below(2) ? '-' : '+');
		uint32_t nd = 1 + rnd_below(8);
		for (uint32_t i = 0; i < nd; i++)
			out += (char)('0' + rnd_below(10));
	} else if (kind < 58) {
		static const long edges[] = { -2, -1, 0, 1, 3, 4, 5, 6,
		    99998, 99999, 100000, 2147483647L, 2147483648L,
		    4294967295L, 4294967296L };
		char b[32];

		std::snprintf(b, sizeof(b), "%ld",
		    edges[rnd_below((uint32_t)(sizeof(edges) /
		    sizeof(edges[0])))]);
		out += b;
	} else if (kind < 64) {
		char b[32];

		std::snprintf(b, sizeof(b), "%s%x",
		    rnd_below(2) ? "0x" : "0X", (unsigned)rnd_below(70000));
		out += b;
	} else if (kind < 68) {
		char b[32];

		std::snprintf(b, sizeof(b), "0%o", (unsigned)rnd_below(70000));
		out += b;
	} else if (kind < 74) {
		/* numeric prefix + trailing garbage: forces the *ep != 0 arm */
		uint32_t nd = 1 + rnd_below(5);

		for (uint32_t i = 0; i < nd; i++)
			out += (char)('0' + rnd_below(10));
		uint32_t ng = 1 + rnd_below(3);
		for (uint32_t i = 0; i < ng; i++)
			out += (char)(0x21 + rnd_below(0xdf));
	} else if (kind < 78) {
		/* pure high-bit bytes */
		uint32_t n = 1 + rnd_below(5);

		for (uint32_t i = 0; i < n; i++)
			out += (char)(unsigned char)(0x80 + rnd_below(0x80));
	} else if (kind < 82) {
		/* empty token, drives the *sp == '\0' usage() path */
		;
	} else if (kind < 84) {
		/* long token, drives snprintf() truncation at MAXPATHLEN */
		uint32_t n = 4000 + rnd_below(1200);

		for (uint32_t i = 0; i < n; i++)
			out += (char)('a' + rnd_below(26));
	} else {
		uint32_t n = 1 + rnd_below(12);

		for (uint32_t i = 0; i < n; i++) {
			unsigned c;

			do {
				c = 1 + rnd_below(255);
			} while (c == ',');
			out += (char)(unsigned char)c;
		}
	}
}

static void
test_makelist_random(long iters)
{
	const pid_t pidchoice[] = { getpid(), 0, 1, -1, 999999, getppid(), 2 };

	for (long it = 0; it < iters; it++) {
		std::string s;
		uint32_t ntok = rnd_below(5);

		if (ntok == 0)
			ntok = 1;
		for (uint32_t k = 0; k < ntok; k++) {
			if (k != 0)
				s += ',';
			gen_token(s);
		}
		if (rnd_below(10) == 0)
			s += ',';
		if (s.size() > MAXIN)
			s.resize(MAXIN);

		int pg = (int)(rnd() & 1);
		pid_t mp = pidchoice[rnd_below(
		    (uint32_t)(sizeof(pidchoice) / sizeof(pidchoice[0])))];
		int type = (int)rnd_below(8);
		const char *src = (rnd_below(64) == 0) ? nullptr : s.c_str();
		size_t len = (src == nullptr) ? 0 : s.size();

		one_makelist(type, src, len, pg, mp);

		/*
		 * Second pass with LT_CLASS on the same bytes: LT_CLASS
		 * strdup()s every token, so this checks the strsep() token
		 * sequence itself, boundary for boundary, independently of
		 * whatever the randomly chosen type did with it.
		 */
		if (type != LT_CLASS)
			one_makelist(LT_CLASS, src, len, pg, mp);
	}
}

/* ------------------------------------------------------------------ */
/* takepid()								*/
/* ------------------------------------------------------------------ */

static char tmpdir[] = "/tmp/pbsd_b0210_XXXXXX";
static std::string path_plain;
static std::string path_locked;
static std::string path_missing;
static std::string path_dir;
static int lock_pipe[2] = { -1, -1 };
static pid_t lock_child = -1;

static void
write_file(const std::string &path, const char *data, size_t len)
{
	int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);

	if (fd < 0) {
		std::perror("open");
		std::exit(3);
	}
	size_t off = 0;
	while (off < len) {
		ssize_t w = ::write(fd, data + off, len - off);

		if (w <= 0) {
			std::perror("write");
			std::exit(3);
		}
		off += (size_t)w;
	}
	::close(fd);
}

static void
setup_takepid(void)
{

	if (::mkdtemp(tmpdir) == nullptr) {
		std::perror("mkdtemp");
		std::exit(3);
	}
	path_plain = std::string(tmpdir) + "/pid";
	path_locked = std::string(tmpdir) + "/locked";
	path_missing = std::string(tmpdir) + "/does_not_exist";
	path_dir = std::string(tmpdir);

	write_file(path_plain, "1234\n", 5);
	write_file(path_locked, "1234\n", 5);

	int sync_pipe[2];
	if (::pipe(sync_pipe) != 0 || ::pipe(lock_pipe) != 0) {
		std::perror("pipe");
		std::exit(3);
	}

	lock_child = ::fork();
	if (lock_child < 0) {
		std::perror("fork");
		std::exit(3);
	}
	if (lock_child == 0) {
		::close(sync_pipe[0]);
		::close(lock_pipe[1]);
		int fd = ::open(path_locked.c_str(), O_RDWR);
		if (fd < 0 || ::flock(fd, LOCK_EX) != 0)
			_exit(1);
		char c = 'k';
		(void)!::write(sync_pipe[1], &c, 1);
		char d;
		while (::read(lock_pipe[0], &d, 1) > 0)
			;
		_exit(0);
	}
	::close(sync_pipe[1]);
	::close(lock_pipe[0]);
	char c = 0;
	if (::read(sync_pipe[0], &c, 1) != 1 || c != 'k') {
		std::fprintf(stderr, "harness: lock helper failed to start\n");
		std::exit(3);
	}
	::close(sync_pipe[0]);
}

static void
teardown_takepid(void)
{

	if (lock_pipe[1] >= 0)
		::close(lock_pipe[1]);
	if (lock_child > 0) {
		int st;

		(void)::waitpid(lock_child, &st, 0);
	}
	::unlink(path_plain.c_str());
	::unlink(path_locked.c_str());
	::rmdir(tmpdir);
}

static void
one_takepid(const std::string &path, const char *content, size_t clen,
    int lock)
{
	Fn &f = fn_takepid;

	if (content != nullptr)
		write_file(path, content, clen);

	f.cases++;

	int pout = 0x0badc0de;
	int oout = 0x0badc0de;
	int pc = P::call_takepid(path.c_str(), lock, &pout);
	std::string pm = P::trap_message();
	int rc = ref_call_takepid(path.c_str(), lock, &oout);
	std::string rm = ref_trap_message();

	std::string ctx = "file=" + path + " lock=" + std::to_string(lock) +
	    " content=" + (content == nullptr ? std::string("<unchanged>")
	    : vis(content, clen));

	if (pc != rc) {
		fail(f, ctx + " status port=" + std::to_string(pc) + " ref=" +
		    std::to_string(rc));
		return;
	}
	if (pm != rm) {
		fail(f, ctx + " message port=" + vis(pm.data(), pm.size()) +
		    " ref=" + vis(rm.data(), rm.size()));
		return;
	}
	if (pout != oout) {
		fail(f, ctx + " pid port=" + std::to_string(pout) + " ref=" +
		    std::to_string(oout));
	}
}

static void
one_takepid(const std::string &path, const char *content, int lock)
{

	one_takepid(path, content, content == nullptr ? 0 : std::strlen(content),
	    lock);
}

static const char *const fixed_pid[] = {
	"",
	"\n",
	"\n\n",
	" ",
	"  \n",
	"\t",
	"0",
	"0\n",
	"1",
	"2",
	"3",
	"4",
	"4\n",
	"5",
	"5\n",
	"6",
	"7",
	"99997",
	"99998",
	"99999",
	"99999\n",
	"100000",
	"100000\n",
	"100001",
	"-1",
	"-5",
	"-99999",
	"+5",
	"+99999",
	"2147483647",
	"2147483648",
	"-2147483648",
	"9223372036854775807",
	"9223372036854775808",
	"99999999999999999999",
	"-99999999999999999999",
	"5 ",
	"5\t",
	"5\r",
	"5\v",
	"5\f",
	"5x",
	"5 x",
	"5\nx",
	"x",
	"x5",
	" 5",
	"  5",
	"\t5",
	"\n5",
	"05",
	"005",
	"0x10",
	"0X10",
	"010",
	"5.0",
	"5,6",
	"5-6",
	"5+6",
	"\x80",
	"5\x80",
	"\x80" "5",
	"\xff\xfe",
	"5\xff",
	"5\x7f",
	"5\x01",
	"12345\n67890\n",
	"99999\nignored",
	"4\nignored",
};

static void
test_takepid_fixed(void)
{

	for (size_t i = 0; i < sizeof(fixed_pid) / sizeof(fixed_pid[0]); i++) {
		for (int lock = 0; lock < 2; lock++) {
			one_takepid(path_plain, fixed_pid[i], lock);
			one_takepid(path_locked, fixed_pid[i], lock);
		}
	}

	/* Content with an embedded NUL: fgets() keeps it, strtol() stops. */
	static const char nulcase1[] = "5\0009\n";
	static const char nulcase2[] = "\0005\n";
	static const char nulcase3[] = "99999\0";
	for (int lock = 0; lock < 2; lock++) {
		one_takepid(path_plain, nulcase1, sizeof(nulcase1) - 1, lock);
		one_takepid(path_plain, nulcase2, sizeof(nulcase2) - 1, lock);
		one_takepid(path_plain, nulcase3, sizeof(nulcase3) - 1, lock);
		one_takepid(path_locked, nulcase1, sizeof(nulcase1) - 1, lock);
	}

	/* Lines that straddle the BUFSIZ fgets() window. */
	static const size_t biglens[] = { BUFSIZ - 2, BUFSIZ - 1, BUFSIZ,
	    BUFSIZ + 1, BUFSIZ + 64 };
	for (size_t k = 0; k < sizeof(biglens) / sizeof(biglens[0]); k++) {
		std::string s(biglens[k], '7');

		one_takepid(path_plain, s.c_str(), s.size(), 0);
		one_takepid(path_plain, s.c_str(), s.size(), 1);
		std::string t(biglens[k], ' ');
		t += "5\n";
		one_takepid(path_plain, t.c_str(), t.size(), 0);
		std::string u = "5";
		u.append(biglens[k], 'q');
		one_takepid(path_plain, u.c_str(), u.size(), 0);
	}

	/* Non-file and unopenable targets. */
	for (int lock = 0; lock < 2; lock++) {
		one_takepid(path_missing, nullptr, lock);
		one_takepid(path_dir, nullptr, lock);
		one_takepid(std::string("/proc/self/nonexistent_zzz"), nullptr,
		    lock);
		one_takepid(std::string(""), nullptr, lock);
		one_takepid(std::string("/dev/null"), nullptr, lock);
	}
}

static void
test_takepid_random(long iters)
{

	for (long it = 0; it < iters; it++) {
		std::string s;
		uint32_t kind = rnd_below(100);

		if (kind < 34) {
			static const long edges[] = { -2, -1, 0, 1, 2, 3, 4,
			    5, 6, 7, 99997, 99998, 99999, 100000, 100001,
			    2147483647L, -2147483648L };
			char b[32];

			std::snprintf(b, sizeof(b), "%ld",
			    edges[rnd_below((uint32_t)(sizeof(edges) /
			    sizeof(edges[0])))]);
			s = b;
		} else if (kind < 60) {
			if (rnd_below(4) == 0)
				s += (rnd_below(2) ? '-' : '+');
			uint32_t nd = 1 + rnd_below(9);
			for (uint32_t i = 0; i < nd; i++)
				s += (char)('0' + rnd_below(10));
		} else if (kind < 70) {
			uint32_t nd = 1 + rnd_below(24);

			for (uint32_t i = 0; i < nd; i++)
				s += (char)('0' + rnd_below(10));
		} else if (kind < 78) {
			uint32_t n = rnd_below(6);

			for (uint32_t i = 0; i < n; i++)
				s += (char)(unsigned char)(0x80 +
				    rnd_below(0x80));
		} else if (kind < 86) {
			uint32_t n = rnd_below(8);

			for (uint32_t i = 0; i < n; i++)
				s += (char)(unsigned char)(1 + rnd_below(255));
		} else {
			s = "";
		}

		/* Optional leading whitespace. */
		uint32_t lead = rnd_below(4);
		if (lead == 1)
			s.insert(s.begin(), ' ');
		else if (lead == 2)
			s.insert(s.begin(), '\t');

		/* Optional trailing byte, both space and non-space. */
		uint32_t tail = rnd_below(8);
		if (tail == 1)
			s += '\n';
		else if (tail == 2)
			s += ' ';
		else if (tail == 3)
			s += '\t';
		else if (tail == 4)
			s += 'x';
		else if (tail == 5)
			s += (char)(unsigned char)(0x80 + rnd_below(0x80));
		else if (tail == 6)
			s += (char)(unsigned char)(1 + rnd_below(31));

		int lock = (int)(rnd() & 1);
		uint32_t which = rnd_below(20);

		if (which == 0)
			one_takepid(path_missing, nullptr, lock);
		else if (which == 1)
			one_takepid(path_dir, nullptr, lock);
		else if (which < 11)
			one_takepid(path_locked, s.c_str(), s.size(), lock);
		else
			one_takepid(path_plain, s.c_str(), s.size(), lock);
	}
}

/* ------------------------------------------------------------------ */

static void
row(const Fn &f)
{

	std::printf("  %-12s %10ld %10ld  %s\n", f.name, f.cases, f.fails,
	    f.fails == 0 ? "ok" : "FAILED");
}

int
main(void)
{

	setup_takepid();

	rng_seed(0xC0FFEE1234567ULL);

	test_usage();
	for (long i = 0; i < 30000; i++)
		one_usage((int)(int32_t)rnd());

	test_makelist_fixed();
	test_makelist_random(30000);

	test_takepid_fixed();
	test_takepid_random(30000);

	teardown_takepid();

	std::printf("\n");
	std::printf("  %-12s %10s %10s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("  ------------------------------------------------\n");
	row(fn_usage);
	row(fn_makelist);
	row(fn_takepid);
	std::printf("\n");

	long total = fn_usage.fails + fn_makelist.fails + fn_takepid.fails;
	if (total != 0) {
		std::printf("  RESULT: FAIL (%ld mismatches)\n", total);
		return (1);
	}
	std::printf("  RESULT: PASS\n");
	return (0);
}
