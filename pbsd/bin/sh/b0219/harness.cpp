/*
 * harness.cpp -- differential test for PBSD batch b0219.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

import pbsd.bin.sh.b0219;

namespace port = pbsd::bin_sh::b0219;

/* Oracle-side layouts (must match oracle.c). */
struct oracle_output {
	char *nextc;
	char *bufend;
	char *buf;
	int bufsize;
	short fd;
	short flags;
};

struct oracle_stackmark {
	struct oracle_stack_block *stackp;
	char *stacknxt;
	int stacknleft;
};

struct oracle_stack_block {
	struct oracle_stack_block *prev;
};

struct oracle_alias {
	struct oracle_alias *next;
	char *name;
	char *val;
	int flag;
};

#define MEM_OUT (-2)
#define OUTPUT_ERR 01
static const unsigned char GUARD = 0x7f;

extern "C" void oracle_reset_state(void);
extern "C" struct oracle_output *oracle_get_memout(void);
extern "C" void oracle_set_out1_memout(void);
extern "C" void oracle_restore_out1(void);
extern "C" char **oracle_argptr;
extern "C" char *oracle_nextopt_optptr;

extern "C" void *ref_ckmalloc(std::size_t);
extern "C" void *ref_ckrealloc(void *, int);
extern "C" void ref_ckfree(void *);
extern "C" char *ref_savestr(const char *);
extern "C" void *ref_stalloc(int);
extern "C" void ref_stunalloc(void *);
extern "C" char *ref_stsavestr(const char *);
extern "C" void ref_setstackmark(struct oracle_stackmark *);
extern "C" void ref_popstackmark(struct oracle_stackmark *);
extern "C" char *ref_growstackstr(void);
extern "C" char *ref_makestrspace(int, char *);
extern "C" char *ref_stputbin(const char *, std::size_t, char *);
extern "C" char *ref_stputs(const char *, char *);

extern "C" void ref_outcslow(int, struct oracle_output *);
extern "C" void ref_out1str(const char *);
extern "C" void ref_out1qstr(const char *);
extern "C" void ref_out2str(const char *);
extern "C" void ref_out2qstr(const char *);
extern "C" void ref_outstr(const char *, struct oracle_output *);
extern "C" void ref_outqstr(const char *, struct oracle_output *);
extern "C" void ref_outbin(const void *, std::size_t, struct oracle_output *);
extern "C" void ref_emptyoutbuf(struct oracle_output *);
extern "C" void ref_flushall(void);
extern "C" void ref_flushout(struct oracle_output *);
extern "C" void ref_freestdout(void);
extern "C" int ref_outiserror(struct oracle_output *);
extern "C" void ref_outclearerror(struct oracle_output *);
extern "C" void ref_outfmt(struct oracle_output *, const char *, ...);
extern "C" void ref_out1fmt(const char *, ...);
extern "C" void ref_out2fmt_flush(const char *, ...);
extern "C" void ref_fmtstr(char *, int, const char *, ...);
extern "C" int ref_xwrite(int, const char *, int);

extern "C" struct oracle_alias *ref_lookupalias(const char *, int);
extern "C" int ref_aliascmd(int, char **);
extern "C" int ref_unaliascmd(int, char **);
extern "C" const struct oracle_alias *ref_iteralias(const struct oracle_alias *);

static const int MAX_REPORT = 8;
static const long RAND_ITERS = 200000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat stats[64];
static int stat_count = 0;

static std::uint64_t rng_state = 0x123456789abcdefULL;

static Stat *
get_stat(const char *name)
{
	for (int i = 0; i < stat_count; i++)
		if (std::strcmp(stats[i].name, name) == 0)
			return &stats[i];
	stats[stat_count].name = name;
	stats[stat_count].cases = 0;
	stats[stat_count].fails = 0;
	stats[stat_count].reported = 0;
	return &stats[stat_count++];
}

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::size_t
rnd_mod(std::size_t m)
{
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static void
fill_guard(void *p, std::size_t n)
{
	std::memset(p, GUARD, n);
}

static int
cmp_guard(const void *p, std::size_t n)
{
	const unsigned char *b = (const unsigned char *)p;
	for (std::size_t i = 0; i < n; i++)
		if (b[i] != GUARD)
			return (int)i + 1;
	return 0;
}

static void
reset_oracle_aliases(void)
{
	char *av[] = {(char *)"-a", nullptr};
	oracle_argptr = av;
	oracle_nextopt_optptr = nullptr;
	ref_unaliascmd(0, nullptr);
}

static void
reset_port_aliases(void)
{
	char *av[] = {(char *)"-a", nullptr};
	port::port_argptr = av;
	port::port_nextopt_optptr = nullptr;
	port::unaliascmd(0, nullptr);
}

static void
reset_oracle(void)
{
	oracle_reset_state();
	reset_oracle_aliases();
}

static void
reset_port(void)
{
	port::port_reset_state();
	reset_port_aliases();
}

static void
init_memout_oracle(struct oracle_output *o)
{
	std::memset(o, 0, sizeof(*o));
	o->bufsize = 64;
	o->fd = MEM_OUT;
}

static void
init_memout_port(port::output *o)
{
	std::memset(o, 0, sizeof(*o));
	o->bufsize = 64;
	o->fd = MEM_OUT;
}

static std::size_t
memout_len_oracle(struct oracle_output *o)
{
	if (o->buf == nullptr || o->nextc == nullptr || o->nextc < o->buf)
		return 0;
	return (std::size_t)(o->nextc - o->buf);
}

static std::size_t
memout_len_port(port::output *o)
{
	if (o->buf == nullptr || o->nextc == nullptr || o->nextc < o->buf)
		return 0;
	return (std::size_t)(o->nextc - o->buf);
}

static int
fail_stat(Stat *st, const char *tag, const char *detail)
{
	st->fails++;
	if (st->reported < MAX_REPORT) {
		st->reported++;
		std::printf("  FAIL %s [%s] %s\n", st->name, tag, detail);
	}
	return 1;
}

#define ALIASINUSE 1

static void test_alias_set(const char *name, const char *val);
static void test_outqstr(const char *tag, const char *s);

static void
test_alias_inuse(void)
{
	Stat *st = get_stat("unalias/rmaliases/lookupalias");
	int bad = 0;

	reset_oracle();
	reset_port();
	test_alias_set("inuse", "v1");
	struct oracle_alias *oa =
	    (struct oracle_alias *)ref_lookupalias("inuse", 0);
	port::alias *pb = const_cast<port::alias *>(
	    port::lookupalias("inuse", 0));
	if (oa == nullptr || pb == nullptr)
		bad = 1;
	else {
		oa->flag |= ALIASINUSE;
		pb->flag |= ALIASINUSE;
		if (ref_lookupalias("inuse", 1) != nullptr)
			bad |= 2;
		if (port::lookupalias("inuse", 1) != nullptr)
			bad |= 4;
		char *ua[] = {(char *)"inuse", nullptr};
		oracle_argptr = ua;
		oracle_nextopt_optptr = nullptr;
		int ra = ref_unaliascmd(0, nullptr);
		port::port_argptr = ua;
		port::port_nextopt_optptr = nullptr;
		int rp = port::unaliascmd(0, nullptr);
		if (ra != rp)
			bad |= 8;
		if (oa->name[0] != '\0' || pb->name[0] != '\0')
			bad |= 16;
	}
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "alias inuse");
}

static void
test_stack_grow(void)
{
	Stat *st = get_stat("growstackstr/makestrspace/stputbin/stputs");
	char *pa, *pb;
	int bad = 0;

	reset_oracle();
	reset_port();
	struct oracle_stackmark ma;
	port::stackmark mb;
	ref_setstackmark(&ma);
	port::setstackmark(&mb);
	pa = ref_growstackstr();
	pb = port::growstackstr();
	if (pa == nullptr || pb == nullptr)
		bad = 1;
	for (int i = 0; i < 16; i++) {
		unsigned char c = (unsigned char)(0x80 + i);
		pa = ref_stputbin((const char *)&c, 1, pa);
		pb = port::stputbin((const char *)&c, 1, pb);
	}
	ref_popstackmark(&ma);
	port::popstackmark(&mb);
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "stack grow");
}

static void
test_memout_grow(void)
{
	Stat *st = get_stat("outbin/emptyoutbuf");
	struct oracle_output oa;
	port::output ob;
	unsigned char data[256];
	int bad = 0;

	for (int i = 0; i < 256; i++)
		data[i] = (unsigned char)(0x80 + i);
	reset_oracle();
	reset_port();
	init_memout_oracle(&oa);
	init_memout_port(&ob);
	ref_outbin(data, 200, &oa);
	port::outbin(data, 200, &ob);
	std::size_t la = memout_len_oracle(&oa);
	std::size_t lb = memout_len_port(&ob);
	if (la != lb || la != 200)
		bad = 1;
	if (la > 0 && std::memcmp(oa.buf, ob.buf, la) != 0)
		bad |= 2;
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "memout grow");
	if (oa.buf)
		std::free(oa.buf);
	if (ob.buf)
		std::free(ob.buf);
}

static void
test_outqstr_exhaustive(void)
{
	const char *more[] = {
		" \t", "foo\nbar", "foo\rbar", "foo\tbar", "noquote",
		"needs'quote", "needs$dollar", "semi;", "amp&", "lt<",
		"gt>", "paren()", "back`tick", "star*", "q?mark",
		"tilde~", "hash#", "weird\x01\x02", "utf8\xc2\xa9",
		"onlyhigh\xff\xfe", "mix a|b", "ends=", "begins#hash"
	};
	for (std::size_t i = 0; i < sizeof(more) / sizeof(more[0]); i++)
		test_outqstr("edge2", more[i]);
}

static void
test_ckmalloc_ckfree(void)
{
	Stat *st = get_stat("ckmalloc/ckfree/savestr");

	for (int i = 0; i < 32; i++) {
		reset_oracle();
		reset_port();
		std::size_t n = (std::size_t)(i * 17 + 1);
		void *pa = ref_ckmalloc(n);
		void *pb = port::ckmalloc(n);
		int bad = 0;
		if (pa == nullptr || pb == nullptr)
			bad = 1;
		std::memset(pa, (unsigned char)(0x80 + i), n);
		std::memset(pb, (unsigned char)(0x80 + i), n);
		if (std::memcmp(pa, pb, n) != 0)
			bad = 2;
		const char *sa = ref_savestr("hello\x80\xff");
		const char *sb = port::savestr("hello\x80\xff");
		if (std::strcmp(sa, sb) != 0)
			bad |= 4;
		ref_ckfree(pa);
		port::ckfree(pb);
		ref_ckfree((void *)sa);
		port::ckfree((void *)sb);
		void *pr = ref_ckrealloc(ref_ckmalloc(8), 32);
		void *pp = port::ckrealloc(port::ckmalloc(8), 32);
		if (pr == nullptr || pp == nullptr)
			bad |= 8;
		ref_ckfree(pr);
		port::ckfree(pp);
		st->cases++;
		if (bad)
			fail_stat(st, "edge", "ckmalloc roundtrip");
	}
}

static void
test_stack(void)
{
	Stat *st = get_stat("stalloc/stunalloc/stackmark");
	struct oracle_stackmark ma;
	port::stackmark mb;

	reset_oracle();
	reset_port();
	ref_setstackmark(&ma);
	port::setstackmark(&mb);
	void *pa = ref_stalloc(64);
	void *pb = port::stalloc(64);
	std::memset(pa, 0xab, 64);
	std::memset(pb, 0xab, 64);
	char *sa = ref_stsavestr("stack\x80test");
	char *sb = port::stsavestr("stack\x80test");
	int bad = 0;
	if (std::strcmp(sa, sb) != 0)
		bad = 1;
	ref_stunalloc(pa);
	port::stunalloc(pb);
	ref_popstackmark(&ma);
	port::popstackmark(&mb);
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "stack mark");
}

static void
test_growstackstr(void)
{
	Stat *st = get_stat("growstackstr/makestrspace/stputbin/stputs");
	char *pa, *pb;
	int bad = 0;

	reset_oracle();
	reset_port();
	struct oracle_stackmark ma;
	port::stackmark mb;
	ref_setstackmark(&ma);
	port::setstackmark(&mb);
	pa = (char *)ref_stalloc(1);
	pb = (char *)port::stalloc(1);
	if (pa == nullptr || pb == nullptr)
		bad = 1;
	{
		char *pa0 = pa;
		char *pb0 = pb;
		pa = ref_stputbin("abc\x80", 4, pa);
		pb = port::stputbin("abc\x80", 4, pb);
		if ((pa - pa0) != (pb - pb0))
			bad |= 2;
		pa = ref_stputs("def", pa);
		pb = port::stputs("def", pb);
		if ((pa - pa0) != (pb - pb0))
			bad |= 4;
		{
			const unsigned char expect[7] = {
				'a', 'b', 'c', 0x80, 'd', 'e', 'f'
			};
			if (pa - pa0 == 7 && std::memcmp(pa0, expect, 7) != 0)
				bad |= 8;
			if (pb - pb0 == 7 && std::memcmp(pb0, expect, 7) != 0)
				bad |= 8;
		}
		if ((pa - pa0) != 7 || (pb - pb0) != 7)
			bad |= 16;
	}
	ref_popstackmark(&ma);
	port::popstackmark(&mb);
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "growstackstr");
}

static void
test_outbin_guard(const char *tag, const void *data, std::size_t len)
{
	Stat *st = get_stat("outbin/emptyoutbuf");
	struct oracle_output oa;
	port::output ob;
	int bad = 0;

	reset_oracle();
	reset_port();
	init_memout_oracle(&oa);
	init_memout_port(&ob);
	ref_outbin(data, len, &oa);
	port::outbin(data, len, &ob);
	std::size_t la = memout_len_oracle(&oa);
	std::size_t lb = memout_len_port(&ob);
	if (la != lb || la != len)
		bad = 1;
	if (la > 0 && std::memcmp(oa.buf, ob.buf, la) != 0)
		bad |= 2;
	st->cases++;
	if (bad)
		fail_stat(st, tag, "outbin buffer");
	if (oa.buf)
		std::free(oa.buf);
	if (ob.buf)
		std::free(ob.buf);
}

static void
test_outqstr(const char *tag, const char *s)
{
	Stat *st = get_stat("outqstr/outdqstr/byteseq");
	struct oracle_output oa;
	port::output ob;
	int bad = 0;

	reset_oracle();
	reset_port();
	init_memout_oracle(&oa);
	init_memout_port(&ob);
	ref_outqstr(s, &oa);
	port::outqstr(s, &ob);
	std::size_t la = memout_len_oracle(&oa);
	std::size_t lb = memout_len_port(&ob);
	if (la != lb)
		bad = 1;
	if (la > 0 && std::memcmp(oa.buf, ob.buf, la) != 0)
		bad |= 2;
	st->cases++;
	if (bad)
		fail_stat(st, tag, s);
	if (oa.buf)
		std::free(oa.buf);
	if (ob.buf)
		std::free(ob.buf);
}

static void
test_output_edge(void)
{
	const char *qstrs[] = {
		"", "a", "'", "''", "abc", "a b", "a=b", "|", "&", "\n",
		"\x80", "\xff", "\x01", "hello\x80world", "$'", "test'",
		"[", "]", "()", "$", "`", "\\", "abc|def", "x\x7f\x80",
		"\r", "\t", "normal", "z9_", "~#="
	};
	for (std::size_t i = 0; i < sizeof(qstrs) / sizeof(qstrs[0]); i++)
		test_outqstr("edge", qstrs[i]);

	unsigned char bin[16];
	for (int i = 0; i < 16; i++)
		bin[i] = (unsigned char)(0x80 + i);
	test_outbin_guard("edge", bin, 16);
	test_outbin_guard("edge", "", 0);
}

static void
test_outfmt(void)
{
	Stat *st = get_stat("outfmt/doformat/out1fmt");
	struct oracle_output oa;
	port::output ob;
	char fa[64], fb[64];
	int bad = 0;

	reset_oracle();
	reset_port();
	init_memout_oracle(&oa);
	init_memout_port(&ob);
	ref_outfmt(&oa, "n=%d s=%s c=%c", 42, "hi\x80", 0x80);
	port::outfmt(&ob, "n=%d s=%s c=%c", 42, "hi\x80", 0x80);
	std::size_t la = memout_len_oracle(&oa);
	std::size_t lb = memout_len_port(&ob);
	if (la != lb || (la > 0 && std::memcmp(oa.buf, ob.buf, la) != 0))
		bad = 1;
	oracle_set_out1_memout();
	port::port_set_out1_memout();
	ref_out1fmt("x=%d", -1);
	port::out1fmt("x=%d", -1);
	ref_fmtstr(fa, 64, "%d\x80", 255);
	port::fmtstr(fb, 64, "%d\x80", 255);
	if (std::strcmp(fa, fb) != 0)
		bad |= 2;
	oracle_restore_out1();
	port::port_restore_out1();
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "outfmt");
	if (oa.buf)
		std::free(oa.buf);
	if (ob.buf)
		std::free(ob.buf);
}

static void
test_xwrite_pair(const char *wbuf, int n, int *bad)
{
	int fds[2];
	char ra_buf[512], rb_buf[512];

	if (pipe(fds) != 0)
		return;
	int na = ref_xwrite(fds[1], wbuf, n);
	ssize_t r1 = read(fds[0], ra_buf, n);
	close(fds[0]);
	close(fds[1]);
	if (pipe(fds) != 0)
		return;
	int nb = port::xwrite(fds[1], wbuf, n);
	ssize_t r2 = read(fds[0], rb_buf, n);
	close(fds[0]);
	close(fds[1]);
	if (na != nb)
		*bad |= 1;
	if (r1 != r2 || (r1 > 0 && std::memcmp(ra_buf, rb_buf, (size_t)r1) != 0))
		*bad |= 2;
}

static void
test_xwrite(void)
{
	Stat *st = get_stat("xwrite");
	char wbuf[256];
	int bad = 0;

	reset_oracle();
	reset_port();
	for (int i = 0; i < 256; i++)
		wbuf[i] = (char)(0x80 + (i & 0x7f));
	test_xwrite_pair(wbuf, 200, &bad);
	test_xwrite_pair(wbuf, 1, &bad);
	test_xwrite_pair(wbuf, 0, &bad);
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "xwrite");
}

static void
test_out_flags(void)
{
	Stat *st = get_stat("outiserror/outclearerror/flushout");
	struct oracle_output oa;
	port::output ob;
	int bad = 0;

	reset_oracle();
	reset_port();
	init_memout_oracle(&oa);
	init_memout_port(&ob);
	oa.flags = OUTPUT_ERR;
	ob.flags = OUTPUT_ERR;
	if (ref_outiserror(&oa) != port::outiserror(&ob))
		bad = 1;
	ref_outclearerror(&oa);
	port::outclearerror(&ob);
	if (oa.flags != ob.flags)
		bad |= 2;
	ref_flushout(&oa);
	port::flushout(&ob);
	ref_flushall();
	port::flushall();
	ref_freestdout();
	port::freestdout();
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "flags");
	if (oa.buf)
		std::free(oa.buf);
	if (ob.buf)
		std::free(ob.buf);
}

static void
test_alias_lookup(const char *tag, const char *name, int check)
{
	Stat *st = get_stat("lookupalias/hashalias");
	struct oracle_alias *pa;
	const port::alias *pb;
	int bad = 0;

	pa = ref_lookupalias(name, check);
	pb = port::lookupalias(name, check);
	if ((pa == nullptr) != (pb == nullptr))
		bad = 1;
	else if (pa != nullptr) {
		if (std::strcmp(pa->name, pb->name) != 0 ||
		    std::strcmp(pa->val, pb->val) != 0)
			bad |= 2;
	}
	st->cases++;
	if (bad)
		fail_stat(st, tag, name);
}

static void
alias_arg_sanitize(char *name)
{
	if (name[0] == '\0' || name[0] == '-')
		name[0] = 'a' + (unsigned char)name[1] % 26;
	if (name[0] == '=')
		name[0] = 'z';
}

static void
test_alias_set(const char *name, const char *val)
{
	char obuf[128], pbuf[128];
	char oname[64], pname[64];
	char oval[64], pval[64];

	std::strncpy(oname, name, sizeof(oname) - 1);
	oname[sizeof(oname) - 1] = '\0';
	std::strncpy(pname, name, sizeof(pname) - 1);
	pname[sizeof(pname) - 1] = '\0';
	std::strncpy(oval, val, sizeof(oval) - 1);
	oval[sizeof(oval) - 1] = '\0';
	std::strncpy(pval, val, sizeof(pval) - 1);
	pval[sizeof(pval) - 1] = '\0';
	alias_arg_sanitize(oname);
	alias_arg_sanitize(pname);

	std::snprintf(obuf, sizeof(obuf), "%s=%s", oname, oval);
	std::snprintf(pbuf, sizeof(pbuf), "%s=%s", pname, pval);
	char *oav[] = {obuf, nullptr};
	char *pav[] = {pbuf, nullptr};
	oracle_set_out1_memout();
	port::port_set_out1_memout();
	oracle_argptr = oav;
	oracle_nextopt_optptr = nullptr;
	ref_aliascmd(0, nullptr);
	port::port_argptr = pav;
	port::port_nextopt_optptr = nullptr;
	port::aliascmd(0, nullptr);
	oracle_restore_out1();
	port::port_restore_out1();
}

static void
test_alias_edge(void)
{
	reset_oracle();
	reset_port();
	test_alias_set("a", "1");
	test_alias_lookup("edge", "a", 0);
	test_alias_lookup("edge", "missing", 0);
	test_alias_set("hi", "val\x80\xff");
	test_alias_lookup("edge", "hi", 0);
	test_alias_set("longname", "longval\x80");

	Stat *st = get_stat("iteralias");
	const struct oracle_alias *ia = nullptr;
	const port::alias *ib = nullptr;
	int count = 0;
	int bad = 0;
	do {
		ia = ref_iteralias(ia);
		ib = port::iteralias(ib);
		if ((ia == nullptr) != (ib == nullptr))
			bad = 1;
		else if (ia != nullptr &&
		    std::strcmp(ia->name, ib->name) != 0)
			bad |= 2;
		count++;
	} while (ia != nullptr && count < 100);
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "iteralias");

	char *ua[] = {(char *)"a", nullptr};
	oracle_argptr = ua;
	ref_unaliascmd(0, nullptr);
	port::port_argptr = ua;
	port::unaliascmd(0, nullptr);
	test_alias_lookup("edge", "a", 0);
}

static void
test_aliascmd(void)
{
	Stat *st = get_stat("aliascmd/unaliascmd");
	int ra, rp;
	char ob1[] = "z=9", ob2[] = "z", pb1[] = "z=9", pb2[] = "z";
	char *oav1[] = {ob1, nullptr};
	char *oav2[] = {ob2, nullptr};
	char *pav1[] = {pb1, nullptr};
	char *pav2[] = {pb2, nullptr};
	char *oua[] = {(char *)"-a", nullptr};
	char *pua[] = {(char *)"-a", nullptr};
	int bad = 0;

	reset_oracle();
	reset_port();
	oracle_set_out1_memout();
	port::port_set_out1_memout();
	oracle_argptr = oav1;
	oracle_nextopt_optptr = nullptr;
	ra = ref_aliascmd(0, nullptr);
	port::port_argptr = pav1;
	port::port_nextopt_optptr = nullptr;
	rp = port::aliascmd(0, nullptr);
	if (ra != rp)
		bad = 1;
	oracle_argptr = oav2;
	oracle_nextopt_optptr = nullptr;
	ra = ref_aliascmd(0, nullptr);
	port::port_argptr = pav2;
	port::port_nextopt_optptr = nullptr;
	rp = port::aliascmd(0, nullptr);
	if (ra != rp)
		bad |= 2;
	oracle_restore_out1();
	port::port_restore_out1();
	oracle_argptr = oua;
	oracle_nextopt_optptr = nullptr;
	ra = ref_unaliascmd(0, nullptr);
	port::port_argptr = pua;
	port::port_nextopt_optptr = nullptr;
	rp = port::unaliascmd(0, nullptr);
	if (ra != rp)
		bad |= 4;
	st->cases++;
	if (bad)
		fail_stat(st, "edge", "aliascmd");
}

static void
rand_string(char *buf, std::size_t cap, std::size_t len)
{
	if (len >= cap)
		len = cap - 1;
	for (std::size_t i = 0; i < len; i++) {
		unsigned m = rnd_mod(8);
		if (m == 0)
			buf[i] = (char)('a' + rnd_mod(26));
		else if (m == 1)
			buf[i] = (char)(0x80 + rnd_mod(128));
		else if (m == 2)
			buf[i] = '\0';
		else
			buf[i] = (char)(rnd_mod(256));
	}
	buf[len] = '\0';
	if (buf[0] == '\0' || buf[0] == '-')
		buf[0] = 'a' + (unsigned char)buf[1] % 26;
	if (buf[0] == '=')
		buf[0] = 'z';
}

static void
random_sweep(void)
{
	char buf[128];
	unsigned char bin[64];

	for (long i = 0; i < RAND_ITERS; i++) {
		std::size_t len = rnd_mod(32) + 1;
		rand_string(buf, sizeof(buf), len);
		reset_oracle();
		reset_port();
		void *pa = ref_ckmalloc(len);
		void *pb = port::ckmalloc(len);
		Stat *st = get_stat("ckmalloc/ckfree/savestr");
		st->cases++;
		if (pa == nullptr || pb == nullptr)
			st->fails++;
		ref_ckfree(pa);
		port::ckfree(pb);

		reset_oracle();
		reset_port();
		struct oracle_stackmark ma;
		port::stackmark mb;
		ref_setstackmark(&ma);
		port::setstackmark(&mb);
		int n = (int)rnd_mod(128) + 1;
		pa = ref_stalloc(n);
		pb = port::stalloc(n);
		st = get_stat("stalloc/stunalloc/stackmark");
		st->cases++;
		if (pa == nullptr || pb == nullptr)
			st->fails++;
		ref_popstackmark(&ma);
		port::popstackmark(&mb);

		reset_oracle();
		reset_port();
		len = rnd_mod(24);
		rand_string(buf, sizeof(buf), len);
		test_outqstr("rand", buf);

		reset_oracle();
		reset_port();
		len = rnd_mod(32);
		for (std::size_t j = 0; j < len; j++)
			bin[j] = (unsigned char)rnd_mod(256);
		test_outbin_guard("rand", bin, len);

		if ((i & 0xff) == 0) {
			reset_oracle();
			reset_port();
			char name[16], val[32];
			rand_string(name, sizeof(name), rnd_mod(8) + 1);
			rand_string(val, sizeof(val), rnd_mod(16) + 1);
			test_alias_set(name, val);
			test_alias_lookup("rand", name, 0);
		}
	}
}

int
main(void)
{
	rng_state = 0xc0ffee123456789ULL;

	test_ckmalloc_ckfree();
	test_stack();
	test_growstackstr();
	test_output_edge();
	test_outfmt();
	test_xwrite();
	test_out_flags();
	test_alias_edge();
	test_alias_inuse();
	test_aliascmd();
	test_stack_grow();
	test_memout_grow();
	test_outqstr_exhaustive();
	random_sweep();

	long total_fails = 0;
	std::printf("b0219 differential test results:\n");
	std::printf("%-36s %10s %10s\n", "function", "cases", "failures");
	for (int i = 0; i < stat_count; i++) {
		std::printf("%-36s %10ld %10ld\n",
		    stats[i].name, stats[i].cases, stats[i].fails);
		total_fails += stats[i].fails;
	}
	std::printf("----------------------------------------------\n");
	std::printf("TOTAL FAILURES: %ld\n", total_fails);
	return total_fails == 0 ? 0 : 1;
}
