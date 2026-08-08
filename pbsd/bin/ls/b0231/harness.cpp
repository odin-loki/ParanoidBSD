/*
 * harness.cpp -- differential test for PBSD batch b0231 (ls cmp/util/print/mastercmp).
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include <fts.h>

#ifndef st_birthtim
#define st_birthtim st_mtim
#endif

#ifndef S_IFWHT
#define S_IFWHT 0
#endif

import pbsd.bin.ls.b0231;

namespace P = pbsd::bin_ls::b0231;

extern "C" {
int ref_namecmp(const FTSENT *, const FTSENT *);
int ref_revnamecmp(const FTSENT *, const FTSENT *);
int ref_verscmp(const FTSENT *, const FTSENT *);
int ref_revverscmp(const FTSENT *, const FTSENT *);
int ref_modcmp(const FTSENT *, const FTSENT *);
int ref_revmodcmp(const FTSENT *, const FTSENT *);
int ref_acccmp(const FTSENT *, const FTSENT *);
int ref_revacccmp(const FTSENT *, const FTSENT *);
int ref_birthcmp(const FTSENT *, const FTSENT *);
int ref_revbirthcmp(const FTSENT *, const FTSENT *);
int ref_statcmp(const FTSENT *, const FTSENT *);
int ref_revstatcmp(const FTSENT *, const FTSENT *);
int ref_sizecmp(const FTSENT *, const FTSENT *);
int ref_revsizecmp(const FTSENT *, const FTSENT *);
int ref_prn_normal(const char *);
int ref_prn_printable(const char *);
size_t ref_len_octal(const char *, int);
int ref_prn_octal(const char *);
int ref_printname(const char *);
int ref_printtype(unsigned int);
int ref_mastercmp(const FTSENT *const *, const FTSENT *const *);
extern int f_samesort;
extern int f_octal;
extern int f_octal_escape;
extern int f_nonprint;
extern int f_slash;
extern int f_type;
extern int f_listdir;
extern int f_groupdir;
extern int (*ref_sortfcn)(const FTSENT *, const FTSENT *);
}

namespace {

constexpr long SWEEP = 200000L;
constexpr int MAX_SHOW = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

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

	int bits(int lo, int hi)
	{
		if (hi <= lo)
			return lo;
		return lo + (int)(next() % (std::uint64_t)(hi - lo + 1));
	}

	bool coin()
	{
		return (next() & 1u) != 0;
	}

	unsigned char byte()
	{
		return (unsigned char)(next() & 0xffu);
	}
};

Rng rng(0xb0231faceULL);

Stat st_namecmp = { "namecmp", 0, 0, 0 };
Stat st_revnamecmp = { "revnamecmp", 0, 0, 0 };
Stat st_verscmp = { "verscmp", 0, 0, 0 };
Stat st_revverscmp = { "revverscmp", 0, 0, 0 };
Stat st_modcmp = { "modcmp", 0, 0, 0 };
Stat st_revmodcmp = { "revmodcmp", 0, 0, 0 };
Stat st_acccmp = { "acccmp", 0, 0, 0 };
Stat st_revacccmp = { "revacccmp", 0, 0, 0 };
Stat st_birthcmp = { "birthcmp", 0, 0, 0 };
Stat st_revbirthcmp = { "revbirthcmp", 0, 0, 0 };
Stat st_statcmp = { "statcmp", 0, 0, 0 };
Stat st_revstatcmp = { "revstatcmp", 0, 0, 0 };
Stat st_sizecmp = { "sizecmp", 0, 0, 0 };
Stat st_revsizecmp = { "revsizecmp", 0, 0, 0 };
Stat st_prn_normal = { "prn_normal", 0, 0, 0 };
Stat st_prn_printable = { "prn_printable", 0, 0, 0 };
Stat st_len_octal = { "len_octal", 0, 0, 0 };
Stat st_prn_octal = { "prn_octal", 0, 0, 0 };
Stat st_printname = { "printname", 0, 0, 0 };
Stat st_printtype = { "printtype", 0, 0, 0 };
Stat st_mastercmp = { "mastercmp", 0, 0, 0 };

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

void
sync_cmp_flags(int samesort)
{
	::f_samesort = samesort;
	P::f_samesort = samesort;
}

void
sync_print_flags(int octal, int octal_escape, int nonprint)
{
	::f_octal = octal;
	::f_octal_escape = octal_escape;
	::f_nonprint = nonprint;
	P::f_octal = octal;
	P::f_octal_escape = octal_escape;
	P::f_nonprint = nonprint;
}

void
sync_type_flags(int slash, int type)
{
	::f_slash = slash;
	::f_type = type;
	P::f_slash = slash;
	P::f_type = type;
}

void
sync_master_flags(int listdir, int groupdir)
{
	::f_listdir = listdir;
	::f_groupdir = groupdir;
	P::f_listdir = listdir;
	P::f_groupdir = groupdir;
}

struct FtsNode {
	std::vector<char> blob;
	FTSENT *ent;
	struct stat st {};

	void set_name(const char *n)
	{
		std::string name = n ? n : "";
		size_t nlen = name.size();
		blob.resize(sizeof(FTSENT) + nlen + 1);
		ent = reinterpret_cast<FTSENT *>(blob.data());
		std::memset(ent, 0, sizeof(FTSENT));
		ent->fts_statp = &st;
		std::memcpy(ent->fts_name, name.c_str(), nlen + 1);
		ent->fts_namelen = (unsigned short)nlen;
	}

	explicit FtsNode(const char *n)
	{
		std::memset(&st, 0, sizeof(st));
		set_name(n);
	}

	void set_mtim(long sec, long nsec)
	{
		st.st_mtim.tv_sec = sec;
		st.st_mtim.tv_nsec = nsec;
	}

	void set_atim(long sec, long nsec)
	{
		st.st_atim.tv_sec = sec;
		st.st_atim.tv_nsec = nsec;
	}

	void set_ctim(long sec, long nsec)
	{
		st.st_ctim.tv_sec = sec;
		st.st_ctim.tv_nsec = nsec;
	}

	void set_birthtim(long sec, long nsec)
	{
		st.st_birthtim.tv_sec = sec;
		st.st_birthtim.tv_nsec = nsec;
	}

	void set_size(off_t sz)
	{
		st.st_size = sz;
	}

	void set_info(int info)
	{
		ent->fts_info = (short)info;
	}

	void set_level(int level)
	{
		ent->fts_level = (short)level;
	}
};

struct IoCapture {
	int saved = -1;
	int readfd = -1;

	bool begin()
	{
		int fds[2];
		saved = dup(STDOUT_FILENO);
		if (saved < 0)
			return false;
		if (pipe(fds) != 0) {
			close(saved);
			saved = -1;
			return false;
		}
		if (dup2(fds[1], STDOUT_FILENO) < 0) {
			close(fds[0]);
			close(fds[1]);
			close(saved);
			saved = -1;
			return false;
		}
		close(fds[1]);
		readfd = fds[0];
		return true;
	}

	std::vector<unsigned char> end()
	{
		std::vector<unsigned char> out;
		unsigned char buf[4096];
		ssize_t nr;

		std::fflush(stdout);
		while ((nr = read(readfd, buf, sizeof(buf))) > 0)
			out.insert(out.end(), buf, buf + nr);
		close(readfd);
		readfd = -1;
		dup2(saved, STDOUT_FILENO);
		close(saved);
		saved = -1;
		return out;
	}
};

template <typename RefFn, typename PortFn>
bool
test_cmp_pair(Stat &st, RefFn ref_fn, PortFn port_fn,
    FtsNode &a, FtsNode &b, int samesort, const char *tag)
{
	st.cases++;
	sync_cmp_flags(samesort);
	int r1 = ref_fn(a.ent, b.ent);
	int r2 = port_fn(a.ent, b.ent);
	if (r1 != r2) {
		fail(st, tag);
		return false;
	}
	return true;
}

using CmpFn = int (*)(const FTSENT *, const FTSENT *);

void
edge_cmps(Stat &st, CmpFn ref_fn, CmpFn port_fn,
    void (FtsNode::*set_ts)(long, long))
{
	FtsNode a("a"), b("b");
	(a.*set_ts)(0, 0);
	(b.*set_ts)(0, 0);
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "equal zero");
	test_cmp_pair(st, ref_fn, port_fn, a, b, 1, "equal zero samesort");

	(b.*set_ts)(1, 0);
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "sec gt");
	test_cmp_pair(st, ref_fn, port_fn, a, b, 1, "sec gt samesort");

	(b.*set_ts)(-1, 0);
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "sec lt");

	(b.*set_ts)(0, 0);
	(b.*set_ts)(0, 1);
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "nsec gt");

	(b.*set_ts)(0, 0);
	(b.*set_ts)(0, -1);
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "nsec lt");

	(b.*set_ts)(5, 100);
	(a.*set_ts)(5, 100);
	a.set_name("apple");
	b.set_name("banana");
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "tie name");
	test_cmp_pair(st, ref_fn, port_fn, a, b, 1, "tie name samesort");

	a.set_name("banana");
	b.set_name("apple");
	test_cmp_pair(st, ref_fn, port_fn, a, b, 1, "tie name rev samesort");
}

void
sweep_cmps(Stat &st, CmpFn ref_fn, CmpFn port_fn,
    void (FtsNode::*set_ts)(long, long), long n)
{
	static const char *names[] = {
		"", "a", "b", "file", "file2", "z", "A", "a1", "a2",
		"ver1", "ver2", "ver10", "\x80\xff", "."
	};
	for (long i = 0; i < n; i++) {
		FtsNode a(names[rng.bits(0, 12)]);
		FtsNode b(names[rng.bits(0, 12)]);
		long sa = rng.bits(-3, 3);
		long sb = rng.bits(-3, 3);
		long na = (long)(rng.next() % 1000000000u);
		long nb = (long)(rng.next() % 1000000000u);
		(a.*set_ts)(sa, na);
		(b.*set_ts)(sb, nb);
		int ss = rng.coin() ? 1 : 0;
		test_cmp_pair(st, ref_fn, port_fn, a, b, ss, "random");
	}
}

void
test_name_like(Stat &st, CmpFn ref_fn, CmpFn port_fn, long sweep_n)
{
	FtsNode a(""), b("");
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "empty");

	a.set_name("a");
	b.set_name("b");
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "ab");

	a.set_name("file1");
	b.set_name("file10");
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "vers");

	a.set_name("\xff");
	b.set_name("\xfe");
	test_cmp_pair(st, ref_fn, port_fn, a, b, 0, "highbit");

	for (long i = 0; i < sweep_n; i++) {
		char na[16], nb[16];
		int la = rng.bits(0, 8);
		int lb = rng.bits(0, 8);
		for (int j = 0; j < la; j++)
			na[j] = (char)rng.byte();
		na[la] = '\0';
		for (int j = 0; j < lb; j++)
			nb[j] = (char)rng.byte();
		nb[lb] = '\0';
		FtsNode fa(na), fb(nb);
		test_cmp_pair(st, ref_fn, port_fn, fa, fb, rng.coin() ? 1 : 0,
		    "random");
	}
}

void
test_sizecmp(long sweep_n)
{
	FtsNode a("a"), b("b");
	a.set_size(0);
	b.set_size(1);
	test_cmp_pair(st_sizecmp, ref_sizecmp, P::sizecmp, a, b, 0, "size gt");
	b.set_size(-1);
	test_cmp_pair(st_sizecmp, ref_sizecmp, P::sizecmp, a, b, 0, "size lt");
	a.set_size(5);
	b.set_size(5);
	b.set_name("z");
	test_cmp_pair(st_sizecmp, ref_sizecmp, P::sizecmp, a, b, 0, "tie");

	for (long i = 0; i < sweep_n; i++) {
		char na[12], nb[12];
		snprintf(na, sizeof(na), "n%ld", i & 7);
		snprintf(nb, sizeof(nb), "m%ld", (i >> 3) & 7);
		FtsNode fa(na), fb(nb);
		fa.set_size((off_t)rng.bits(-1000, 1000));
		fb.set_size((off_t)rng.bits(-1000, 1000));
		test_cmp_pair(st_sizecmp, ref_sizecmp, P::sizecmp, fa, fb, 0,
		    "random");
	}
}

bool
test_prn(Stat &st, int (*ref_fn)(const char *), int (*port_fn)(const char *),
    const char *s, int octal, int oct_esc, int nonprint, const char *tag)
{
	st.cases++;
	sync_print_flags(octal, oct_esc, nonprint);
	IoCapture cap;
	if (!cap.begin()) {
		fail(st, "capture");
		return false;
	}
	int r1 = ref_fn(s);
	auto out1 = cap.end();

	if (!cap.begin()) {
		fail(st, "capture2");
		return false;
	}
	int r2 = port_fn(s);
	auto out2 = cap.end();

	if (r1 != r2 || out1 != out2) {
		fail(st, tag);
		return false;
	}
	return true;
}

void
edge_prn(Stat &st, int (*ref_fn)(const char *), int (*port_fn)(const char *),
    int octal, int oct_esc, int nonprint)
{
	test_prn(st, ref_fn, port_fn, "", octal, oct_esc, nonprint, "empty");
	test_prn(st, ref_fn, port_fn, "a", octal, oct_esc, nonprint, "single");
	test_prn(st, ref_fn, port_fn, "hello", octal, oct_esc, nonprint,
	    "ascii");
	test_prn(st, ref_fn, port_fn, "\x7f", octal, oct_esc, nonprint,
	    "del");
	test_prn(st, ref_fn, port_fn, "\x80\xff", octal, oct_esc, nonprint,
	    "high");
	test_prn(st, ref_fn, port_fn, "\xff", octal, oct_esc, nonprint,
	    "lone ff");
	test_prn(st, ref_fn, port_fn, "\xc0\x80", octal, oct_esc, nonprint,
	    "invalid");
	test_prn(st, ref_fn, port_fn, "caf\xe9", octal, oct_esc, nonprint,
	    "utf8");
	test_prn(st, ref_fn, port_fn, "\n\t\\\"", octal, oct_esc, nonprint,
	    "escapes");
	test_prn(st, ref_fn, port_fn, "\xc2", octal, oct_esc, nonprint,
	    "incomplete");
	test_prn(st, ref_fn, port_fn, "\xc2Z", octal, oct_esc, nonprint,
	    "incomplete2");
}

void
sweep_prn(Stat &st, int (*ref_fn)(const char *), int (*port_fn)(const char *),
    long n, int octal, int oct_esc, int nonprint)
{
	std::vector<char> buf;
	for (long i = 0; i < n; i++) {
		int len = rng.bits(0, 32);
		buf.resize((size_t)len + 1);
		for (int j = 0; j < len; j++)
			buf[(size_t)j] = (char)rng.byte();
		buf[(size_t)len] = '\0';
		test_prn(st, ref_fn, port_fn, buf.data(), octal, oct_esc,
		    nonprint, "random");
	}
}

void
test_len_octal_case(const char *s, int len, const char *tag)
{
	st_len_octal.cases++;
	size_t r1 = ref_len_octal(s, len);
	size_t r2 = P::len_octal(s, len);
	if (r1 != r2)
		fail(st_len_octal, tag);
}

void
test_len_octal_all(long sweep_n)
{
	test_len_octal_case("", 0, "empty");
	test_len_octal_case("a", 0, "len0");
	test_len_octal_case("a", 1, "len1");
	test_len_octal_case("ab", 1, "trunc");
	test_len_octal_case("\xff", 1, "high");
	test_len_octal_case("\xc0\x80", 2, "invalid");
	test_len_octal_case("caf\xe9", 4, "utf8");
	test_len_octal_case("\xc2", 1, "incomplete");
	test_len_octal_case("\xc2Z", 2, "incomplete2");

	std::vector<char> buf;
	for (long i = 0; i < sweep_n; i++) {
		int len = rng.bits(0, 40);
		int slen = rng.bits(0, 40);
		buf.resize((size_t)slen + 1);
		for (int j = 0; j < slen; j++)
			buf[(size_t)j] = (char)rng.byte();
		buf[(size_t)slen] = '\0';
		test_len_octal_case(buf.data(), len, "random");
	}
}

void
test_printname_all(long sweep_n)
{
	edge_prn(st_printname, ref_printname, P::printname, 0, 0, 0);
	edge_prn(st_printname, ref_printname, P::printname, 0, 0, 1);
	edge_prn(st_printname, ref_printname, P::printname, 1, 0, 0);
	edge_prn(st_printname, ref_printname, P::printname, 0, 1, 0);
	sweep_prn(st_printname, ref_printname, P::printname, sweep_n / 4, 0, 0,
	    0);
	sweep_prn(st_printname, ref_printname, P::printname, sweep_n / 4, 0, 0,
	    1);
	sweep_prn(st_printname, ref_printname, P::printname, sweep_n / 4, 1, 0,
	    0);
	sweep_prn(st_printname, ref_printname, P::printname, sweep_n / 4, 0, 1,
	    0);
}

bool
test_printtype_case(unsigned mode, int slash, const char *tag)
{
	st_printtype.cases++;
	sync_type_flags(slash, 0);
	IoCapture cap;
	if (!cap.begin()) {
		fail(st_printtype, "cap");
		return false;
	}
	int r1 = ref_printtype(mode);
	auto o1 = cap.end();
	if (!cap.begin()) {
		fail(st_printtype, "cap2");
		return false;
	}
	int r2 = P::printtype(mode);
	auto o2 = cap.end();
	if (r1 != r2 || o1 != o2) {
		fail(st_printtype, tag);
		return false;
	}
	return true;
}

void
test_printtype_all(long sweep_n)
{
	test_printtype_case(S_IFREG, 0, "reg");
	test_printtype_case(S_IFDIR, 0, "dir");
	test_printtype_case(S_IFIFO, 0, "fifo");
	test_printtype_case(S_IFLNK, 0, "lnk");
	test_printtype_case(S_IFSOCK, 0, "sock");
	test_printtype_case(S_IFWHT, 0, "wht");
	test_printtype_case(S_IFREG | S_IXUSR, 0, "exec");
	test_printtype_case(S_IFDIR, 1, "slash dir");
	test_printtype_case(S_IFREG, 1, "slash reg");
	test_printtype_case(S_IFREG | S_IXOTH, 1, "slash exec");

	for (long i = 0; i < sweep_n; i++) {
		unsigned mode = (unsigned)rng.bits(0, 0xffff);
		int slash = rng.coin() ? 1 : 0;
		test_printtype_case(mode, slash, "random");
	}
}

bool
test_mastercmp_case(FtsNode &a, FtsNode &b, int listdir, int groupdir,
    const char *tag)
{
	st_mastercmp.cases++;
	sync_master_flags(listdir, groupdir);
	::ref_sortfcn = ref_namecmp;
	P::sortfcn = P::namecmp;
	const FTSENT *pa = &a.ent;
	const FTSENT *pb = &b.ent;
	int r1 = ref_mastercmp(&pa, &pb);
	int r2 = P::mastercmp(&pa, &pb);
	if (r1 != r2) {
		fail(st_mastercmp, tag);
		return false;
	}
	return true;
}

void
test_mastercmp_all(long sweep_n)
{
	FtsNode a("a"), b("b");
	a.set_info(FTS_F);
	b.set_info(FTS_F);
	a.set_level(FTS_ROOTLEVEL);
	b.set_level(FTS_ROOTLEVEL);
	test_mastercmp_case(a, b, 0, P::GRP_NONE, "root files");

	a.set_info(FTS_D);
	b.set_info(FTS_F);
	test_mastercmp_case(a, b, 0, P::GRP_NONE, "dir first root");

	a.set_info(FTS_F);
	b.set_info(FTS_D);
	test_mastercmp_case(a, b, 0, P::GRP_NONE, "dir second root");

	a.set_info(FTS_NS);
	b.set_info(FTS_F);
	test_mastercmp_case(a, b, 0, P::GRP_NONE, "ns");

	a.set_info(FTS_ERR);
	b.set_info(FTS_F);
	test_mastercmp_case(a, b, 0, P::GRP_NONE, "err");

	a.set_info(FTS_D);
	b.set_info(FTS_F);
	a.set_level(1);
	b.set_level(1);
	test_mastercmp_case(a, b, 0, P::GRP_DIR_FIRST, "group first");

	a.set_info(FTS_D);
	b.set_info(FTS_F);
	test_mastercmp_case(a, b, 0, P::GRP_DIR_LAST, "group last");

	a.set_info(FTS_D);
	b.set_info(FTS_F);
	a.set_level(FTS_ROOTLEVEL);
	b.set_level(FTS_ROOTLEVEL);
	test_mastercmp_case(a, b, 1, P::GRP_NONE, "listdir");

	for (long i = 0; i < sweep_n; i++) {
		char na[8], nb[8];
		snprintf(na, sizeof(na), "a%ld", i & 3);
		snprintf(nb, sizeof(nb), "b%ld", (i >> 2) & 3);
		FtsNode fa(na), fb(nb);
		static const int infos[] = { FTS_F, FTS_D, FTS_NS, FTS_ERR,
			FTS_DEFAULT };
		fa.set_info(infos[rng.bits(0, 4)]);
		fb.set_info(infos[rng.bits(0, 4)]);
		fa.set_level((short)rng.bits(0, 2));
		fb.set_level((short)rng.bits(0, 2));
		int gd = rng.bits(0, 2);
		if (gd == 2)
			gd = P::GRP_DIR_LAST;
		else if (gd == 1)
			gd = P::GRP_DIR_FIRST;
		else
			gd = P::GRP_NONE;
		test_mastercmp_case(fa, fb, rng.coin() ? 1 : 0, gd, "random");
	}
}

long
per_fn_sweep()
{
	return SWEEP / 21;
}

} // namespace

int
main()
{
	const long sweep = per_fn_sweep();

	test_name_like(st_namecmp, ref_namecmp, P::namecmp, sweep);
	test_name_like(st_revnamecmp, ref_revnamecmp, P::revnamecmp, sweep);
	test_name_like(st_verscmp, ref_verscmp, P::verscmp, sweep);
	test_name_like(st_revverscmp, ref_revverscmp, P::revverscmp, sweep);

	edge_cmps(st_modcmp, ref_modcmp, P::modcmp, &FtsNode::set_mtim);
	sweep_cmps(st_modcmp, ref_modcmp, P::modcmp, &FtsNode::set_mtim, sweep);
	edge_cmps(st_revmodcmp, ref_revmodcmp, P::revmodcmp,
	    &FtsNode::set_mtim);
	sweep_cmps(st_revmodcmp, ref_revmodcmp, P::revmodcmp,
	    &FtsNode::set_mtim, sweep);

	edge_cmps(st_acccmp, ref_acccmp, P::acccmp, &FtsNode::set_atim);
	sweep_cmps(st_acccmp, ref_acccmp, P::acccmp, &FtsNode::set_atim, sweep);
	edge_cmps(st_revacccmp, ref_revacccmp, P::revacccmp,
	    &FtsNode::set_atim);
	sweep_cmps(st_revacccmp, ref_revacccmp, P::revacccmp,
	    &FtsNode::set_atim, sweep);

	edge_cmps(st_birthcmp, ref_birthcmp, P::birthcmp,
	    &FtsNode::set_birthtim);
	sweep_cmps(st_birthcmp, ref_birthcmp, P::birthcmp,
	    &FtsNode::set_birthtim, sweep);
	edge_cmps(st_revbirthcmp, ref_revbirthcmp, P::revbirthcmp,
	    &FtsNode::set_birthtim);
	sweep_cmps(st_revbirthcmp, ref_revbirthcmp, P::revbirthcmp,
	    &FtsNode::set_birthtim, sweep);

	edge_cmps(st_statcmp, ref_statcmp, P::statcmp, &FtsNode::set_ctim);
	sweep_cmps(st_statcmp, ref_statcmp, P::statcmp, &FtsNode::set_ctim,
	    sweep);
	edge_cmps(st_revstatcmp, ref_revstatcmp, P::revstatcmp,
	    &FtsNode::set_ctim);
	sweep_cmps(st_revstatcmp, ref_revstatcmp, P::revstatcmp,
	    &FtsNode::set_ctim, sweep);

	test_sizecmp(sweep);

	edge_prn(st_prn_normal, ref_prn_normal, P::prn_normal, 0, 0, 0);
	sweep_prn(st_prn_normal, ref_prn_normal, P::prn_normal, sweep, 0, 0,
	    0);

	edge_prn(st_prn_printable, ref_prn_printable, P::prn_printable, 0, 0,
	    1);
	sweep_prn(st_prn_printable, ref_prn_printable, P::prn_printable, sweep,
	    0, 0, 1);

	edge_prn(st_prn_octal, ref_prn_octal, P::prn_octal, 1, 0, 0);
	sweep_prn(st_prn_octal, ref_prn_octal, P::prn_octal, sweep / 2, 1, 0,
	    0);
	edge_prn(st_prn_octal, ref_prn_octal, P::prn_octal, 0, 1, 0);
	sweep_prn(st_prn_octal, ref_prn_octal, P::prn_octal, sweep / 2, 0, 1,
	    0);

	test_len_octal_all(sweep);
	test_printname_all(sweep);
	test_printtype_all(sweep);
	test_mastercmp_all(sweep);

	Stat *all[] = {
		&st_namecmp, &st_revnamecmp, &st_verscmp, &st_revverscmp,
		&st_modcmp, &st_revmodcmp, &st_acccmp, &st_revacccmp,
		&st_birthcmp, &st_revbirthcmp, &st_statcmp, &st_revstatcmp,
		&st_sizecmp, &st_revsizecmp, &st_prn_normal, &st_prn_printable,
		&st_len_octal, &st_prn_octal, &st_printname, &st_printtype,
		&st_mastercmp
	};

	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-16s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-16s %10s %10s\n", "--------", "-----", "--------");
	for (Stat *st : all) {
		std::printf("%-16s %10ld %10ld\n", st->name, st->cases,
		    st->fails);
		total_cases += st->cases;
		total_fails += st->fails;
	}
	std::printf("%-16s %10ld %10ld\n", "TOTAL", total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
