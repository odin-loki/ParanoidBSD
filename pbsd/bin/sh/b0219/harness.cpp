/*
 * PBSD batch b0219 -- differential harness.
 *
 * Every ported routine is exercised against the ref_ oracle in oracle.c with
 * hand written edge cases and with a fixed seed randomised sweep.  Nothing is
 * compared by address: buffers are compared byte for byte (guard bytes
 * included), returned pointers are compared as offsets from their own base,
 * and the file scope state of both sides (alias table, stack allocator,
 * struct output) is dumped and compared after every step.
 */

#include <algorithm>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csetjmp>
#include <clocale>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.bin.sh.b0219;

namespace P = pbsd::bin_sh::b0219;

/* ------------------------------------------------------------------ */
/* oracle declarations                                                */
/* ------------------------------------------------------------------ */

struct r_output {
	char *nextc;
	char *bufend;
	char *buf;
	int bufsize;
	short fd;
	short flags;
};

struct r_alias {
	struct r_alias *next;
	char *name;
	char *val;
	int flag;
};

struct r_stackmark {
	void *stackp;
	char *stacknxt;
	int stacknleft;
};

extern "C" {
/* memalloc.c */
void *ref_ckmalloc(size_t);
void *ref_ckrealloc(void *, int);
void ref_ckfree(void *);
char *ref_savestr(const char *);
void ref_stnewblock(int);
void *ref_stalloc(int);
void ref_stunalloc(void *);
char *ref_stsavestr(const char *);
void ref_setstackmark(r_stackmark *);
void ref_popstackmark(r_stackmark *);
void ref_growstackblock(int);
char *ref_growstrstackblock(int, int);
char *ref_growstackstr(void);
char *ref_makestrspace(int, char *);
char *ref_stputbin(const char *, size_t, char *);
char *ref_stputs(const char *, char *);

/* output.c */
void ref_outcslow(int, r_output *);
void ref_out1str(const char *);
void ref_out1qstr(const char *);
void ref_out2str(const char *);
void ref_out2qstr(const char *);
void ref_outstr(const char *, r_output *);
void ref_byteseq(int, r_output *);
void ref_outdqstr(const char *, r_output *);
void ref_outqstr(const char *, r_output *);
void ref_outbin(const void *, size_t, r_output *);
void ref_freestdout(void);
int ref_outiserror(r_output *);
void ref_outclearerror(r_output *);
void ref_fmtstr(char *, int, const char *, ...);

/* alias.c */
void ref_setalias(const char *, const char *);
void ref_freealias(r_alias *);
int ref_unalias(const char *);
void ref_rmaliases(void);
r_alias *ref_lookupalias(const char *, int);
int ref_comparealiases(const void *, const void *);
size_t ref_hashalias(const char *);
const r_alias *ref_iteralias(const r_alias *);

/* probes */
void ref_set_suppressint(int);
int ref_get_suppressint(void);
void ref_err_arm(int);
jmp_buf *ref_err_jmp(void);
void ref_stack_probe(int *, long *, long *, int *);
char *ref_get_stacknxt(void);
char *ref_get_stackp(void);
char *ref_get_sstrend(void);
int ref_get_stacknleft(void);
size_t ref_sizeof_stackmark(void);
size_t ref_sizeof_alias(void);
int ref_alias_count(void);
r_alias *ref_atab_get(int);
r_output *ref_var_output(void);
r_output *ref_var_errout(void);
r_output *ref_var_memout(void);
r_output **ref_var_out1(void);
r_output **ref_var_out2(void);
}

/* ------------------------------------------------------------------ */
/* adapters                                                           */
/* ------------------------------------------------------------------ */

struct Ref {
	using Out = r_output;
	using Alias = r_alias;
	using Mark = r_stackmark;

	static void *ckmalloc(size_t n) { return ref_ckmalloc(n); }
	static void *ckrealloc(void *p, int n) { return ref_ckrealloc(p, n); }
	static void ckfree(void *p) { ref_ckfree(p); }
	static char *savestr(const char *s) { return ref_savestr(s); }
	static void stnewblock(int n) { ref_stnewblock(n); }
	static void *stalloc(int n) { return ref_stalloc(n); }
	static void stunalloc(void *p) { ref_stunalloc(p); }
	static char *stsavestr(const char *s) { return ref_stsavestr(s); }
	static void setstackmark(Mark *m) { ref_setstackmark(m); }
	static void popstackmark(Mark *m) { ref_popstackmark(m); }
	static void growstackblock(int m) { ref_growstackblock(m); }
	static char *growstrstackblock(int n, int m)
	    { return ref_growstrstackblock(n, m); }
	static char *growstackstr(void) { return ref_growstackstr(); }
	static char *makestrspace(int m, char *p)
	    { return ref_makestrspace(m, p); }
	static char *stputbin(const char *d, size_t l, char *p)
	    { return ref_stputbin(d, l, p); }
	static char *stputs(const char *d, char *p)
	    { return ref_stputs(d, p); }

	static void outcslow(int c, Out *f) { ref_outcslow(c, f); }
	static void out1str(const char *p) { ref_out1str(p); }
	static void out1qstr(const char *p) { ref_out1qstr(p); }
	static void out2str(const char *p) { ref_out2str(p); }
	static void out2qstr(const char *p) { ref_out2qstr(p); }
	static void outstr(const char *p, Out *f) { ref_outstr(p, f); }
	static void byteseq(int c, Out *f) { ref_byteseq(c, f); }
	static void outdqstr(const char *p, Out *f) { ref_outdqstr(p, f); }
	static void outqstr(const char *p, Out *f) { ref_outqstr(p, f); }
	static void outbin(const void *d, size_t l, Out *f)
	    { ref_outbin(d, l, f); }
	static void freestdout(void) { ref_freestdout(); }
	static int outiserror(Out *f) { return ref_outiserror(f); }
	static void outclearerror(Out *f) { ref_outclearerror(f); }
	static void fmtstr_sd(char *b, int l, const char *s, int d)
	    { ref_fmtstr(b, l, "[%s#%d]", s, d); }
	static void fmtstr_s(char *b, int l, const char *s)
	    { ref_fmtstr(b, l, "%s", s); }

	static void setalias(const char *n, const char *v)
	    { ref_setalias(n, v); }
	static void freealias(Alias *a) { ref_freealias(a); }
	static int unalias(const char *n) { return ref_unalias(n); }
	static void rmaliases(void) { ref_rmaliases(); }
	static Alias *lookupalias(const char *n, int c)
	    { return ref_lookupalias(n, c); }
	static int comparealiases(const void *a, const void *b)
	    { return ref_comparealiases(a, b); }
	static size_t hashalias(const char *p) { return ref_hashalias(p); }
	static const Alias *iteralias(const Alias *i)
	    { return ref_iteralias(i); }

	static void set_suppressint(int v) { ref_set_suppressint(v); }
	static void err_arm(int v) { ref_err_arm(v); }
	static jmp_buf *err_jmp(void) { return ref_err_jmp(); }
	static void stack_probe(int *a, long *b, long *c, int *d)
	    { ref_stack_probe(a, b, c, d); }
	static char *get_stacknxt(void) { return ref_get_stacknxt(); }
	static char *get_stackp(void) { return ref_get_stackp(); }
	static char *get_sstrend(void) { return ref_get_sstrend(); }
	static int get_stacknleft(void) { return ref_get_stacknleft(); }
	static size_t sizeof_stackmark(void) { return ref_sizeof_stackmark(); }
	static size_t sizeof_alias(void) { return ref_sizeof_alias(); }
	static int alias_count(void) { return ref_alias_count(); }
	static Alias *atab_get(int i) { return ref_atab_get(i); }
	static Out *var_output(void) { return ref_var_output(); }
	static Out **var_out1(void) { return ref_var_out1(); }
	static Out **var_out2(void) { return ref_var_out2(); }
};

struct Port {
	using Out = std::remove_pointer_t<decltype(P::out1)>;
	using Alias = P::alias;
	using Mark = P::stackmark;

	static void *ckmalloc(size_t n) { return P::ckmalloc(n); }
	static void *ckrealloc(void *p, int n) { return P::ckrealloc(p, n); }
	static void ckfree(void *p) { P::ckfree(p); }
	static char *savestr(const char *s) { return P::savestr(s); }
	static void stnewblock(int n) { P::stnewblock(n); }
	static void *stalloc(int n) { return P::stalloc(n); }
	static void stunalloc(void *p) { P::stunalloc(p); }
	static char *stsavestr(const char *s) { return P::stsavestr(s); }
	static void setstackmark(Mark *m) { P::setstackmark(m); }
	static void popstackmark(Mark *m) { P::popstackmark(m); }
	static void growstackblock(int m) { P::growstackblock(m); }
	static char *growstrstackblock(int n, int m)
	    { return P::growstrstackblock(n, m); }
	static char *growstackstr(void) { return P::growstackstr(); }
	static char *makestrspace(int m, char *p)
	    { return P::makestrspace(m, p); }
	static char *stputbin(const char *d, size_t l, char *p)
	    { return P::stputbin(d, l, p); }
	static char *stputs(const char *d, char *p)
	    { return P::stputs(d, p); }

	static void outcslow(int c, Out *f) { P::outcslow(c, f); }
	static void out1str(const char *p) { P::out1str(p); }
	static void out1qstr(const char *p) { P::out1qstr(p); }
	static void out2str(const char *p) { P::out2str(p); }
	static void out2qstr(const char *p) { P::out2qstr(p); }
	static void outstr(const char *p, Out *f) { P::outstr(p, f); }
	static void byteseq(int c, Out *f) { P::byteseq(c, f); }
	static void outdqstr(const char *p, Out *f) { P::outdqstr(p, f); }
	static void outqstr(const char *p, Out *f) { P::outqstr(p, f); }
	static void outbin(const void *d, size_t l, Out *f)
	    { P::outbin(d, l, f); }
	static void freestdout(void) { P::freestdout(); }
	static int outiserror(Out *f) { return P::outiserror(f); }
	static void outclearerror(Out *f) { P::outclearerror(f); }
	static void fmtstr_sd(char *b, int l, const char *s, int d)
	    { P::fmtstr(b, l, "[%s#%d]", s, d); }
	static void fmtstr_s(char *b, int l, const char *s)
	    { P::fmtstr(b, l, "%s", s); }

	static void setalias(const char *n, const char *v)
	    { P::setalias(n, v); }
	static void freealias(Alias *a) { P::freealias(a); }
	static int unalias(const char *n) { return P::unalias(n); }
	static void rmaliases(void) { P::rmaliases(); }
	static Alias *lookupalias(const char *n, int c)
	    { return P::lookupalias(n, c); }
	static int comparealiases(const void *a, const void *b)
	    { return P::comparealiases(a, b); }
	static size_t hashalias(const char *p) { return P::hashalias(p); }
	static const Alias *iteralias(const Alias *i)
	    { return P::iteralias(i); }

	static void set_suppressint(int v) { P::set_suppressint(v); }
	static void err_arm(int v) { P::err_arm(v); }
	static jmp_buf *err_jmp(void) { return P::err_jmp_ptr(); }
	static void stack_probe(int *a, long *b, long *c, int *d)
	    { P::stack_probe(a, b, c, d); }
	static char *get_stacknxt(void) { return P::get_stacknxt(); }
	static char *get_stackp(void) { return P::get_stackp(); }
	static char *get_sstrend(void) { return P::get_sstrend(); }
	static int get_stacknleft(void) { return P::get_stacknleft(); }
	static size_t sizeof_stackmark(void) { return P::sizeof_stackmark(); }
	static size_t sizeof_alias(void) { return P::sizeof_alias(); }
	static int alias_count(void) { return P::alias_count(); }
	static Alias *atab_get(int i) { return P::atab_get(i); }
	static Out *var_output(void) { return P::var_output(); }
	static Out **var_out1(void) { return P::var_out1(); }
	static Out **var_out2(void) { return P::var_out2(); }
};

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

struct Rec {
	std::string name;
	long long cases = 0;
	long long fails = 0;
	std::string first;
};

static std::vector<Rec> recs;

static Rec &
recFor(const char *n)
{
	for (Rec &r : recs)
		if (r.name == n)
			return r;
	recs.push_back(Rec{std::string(n), 0, 0, std::string()});
	return recs.back();
}

#define CHK(fn, ok, detail)						\
	do {								\
		Rec &r_ = recFor(fn);					\
		r_.cases++;						\
		if (!(ok)) {						\
			r_.fails++;					\
			if (r_.first.empty())				\
				r_.first = (detail);			\
		}							\
	} while (0)

static std::string
esc(const char *s, size_t n)
{
	static const char hexd[] = "0123456789abcdef";
	std::string o;
	for (size_t i = 0; i < n; i++) {
		unsigned char c = (unsigned char)s[i];
		if (c >= 0x20 && c < 0x7f && c != '\\') {
			o += (char)c;
		} else {
			o += "\\x";
			o += hexd[c >> 4];
			o += hexd[c & 15];
		}
	}
	return o;
}

static std::string
esc(const std::string &s)
{
	return esc(s.data(), s.size());
}

static uint64_t
fnv(const void *data, size_t n)
{
	const unsigned char *p = (const unsigned char *)data;
	uint64_t h = 1469598103934665603ULL;
	for (size_t i = 0; i < n; i++) {
		h ^= p[i];
		h *= 1099511628211ULL;
	}
	return h;
}

static std::string
u64(uint64_t v)
{
	char b[32];
	snprintf(b, sizeof(b), "%016llx", (unsigned long long)v);
	return std::string(b);
}

static std::string
num(long long v)
{
	char b[32];
	snprintf(b, sizeof(b), "%lld", v);
	return std::string(b);
}

/* fixed seed PRNG */
static uint64_t rngstate = 0x9e3779b97f4a7c15ULL;

static uint64_t
rnd(void)
{
	rngstate ^= rngstate << 13;
	rngstate ^= rngstate >> 7;
	rngstate ^= rngstate << 17;
	return rngstate;
}

static unsigned
rndn(unsigned n)
{
	return (unsigned)(rnd() % n);
}

static const char SPECIALS[] = "|&;<>()$`\\\" \n*?[~#=";

static std::string
randstr(unsigned maxlen, bool allow_nul = false)
{
	unsigned n = rndn(maxlen + 1);
	std::string s;
	while (s.size() < n) {
		unsigned k = rndn(100);
		if (k < 30) {
			s += (char)("abcdefghijklmnopqrstuvwxyz"
			    "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
			    "_-.:/"[rndn(67)]);
		} else if (k < 45) {
			s += SPECIALS[rndn(sizeof(SPECIALS) - 1)];
		} else if (k < 58) {
			s += (char)(1 + rndn(31));	/* 0x01..0x1f */
		} else if (k < 60) {
			s += (char)0x7f;
		} else if (k < 80) {
			s += (char)(0x80 + rndn(128));	/* high bit */
		} else if (k < 90) {
			/* a valid UTF-8 sequence */
			unsigned cp = rndn(2) ? 0xe9 + rndn(16) : 0x4e00 +
			    rndn(64);
			if (cp < 0x800) {
				s += (char)(0xc0 | (cp >> 6));
				s += (char)(0x80 | (cp & 0x3f));
			} else {
				s += (char)(0xe0 | (cp >> 12));
				s += (char)(0x80 | ((cp >> 6) & 0x3f));
				s += (char)(0x80 | (cp & 0x3f));
			}
		} else if (k < 95) {
			/* a truncated UTF-8 lead byte */
			s += (char)(0xe0 | rndn(16));
		} else if (allow_nul) {
			s += '\0';
		} else {
			s += ' ';
		}
	}
	s.resize(n);
	return s;
}

/* ------------------------------------------------------------------ */
/* traces                                                             */
/* ------------------------------------------------------------------ */

using Trace = std::vector<std::pair<std::string, std::string> >;

static void
cmpTrace(const Trace &a, const Trace &b, const char *what)
{
	size_t n = std::max(a.size(), b.size());
	for (size_t i = 0; i < n; i++) {
		const char *fn = i < a.size() ? a[i].first.c_str() :
		    (i < b.size() ? b[i].first.c_str() : "?");
		bool ok = i < a.size() && i < b.size() && a[i] == b[i];
		CHK(fn, ok, std::string(what) + " step " + num((long long)i) +
		    ": ref=[" + (i < a.size() ? a[i].first + " " + a[i].second :
		    std::string("<missing>")) + "] port=[" +
		    (i < b.size() ? b[i].first + " " + b[i].second :
		    std::string("<missing>")) + "]");
	}
}

/* ------------------------------------------------------------------ */
/* output.c                                                           */
/* ------------------------------------------------------------------ */

static const size_t SINKSZ = 1400;

template <class A>
struct Sink {
	std::vector<char> mem;
	typename A::Out out;

	Sink() : mem(SINKSZ, (char)0x7f)
	{
		out.nextc = mem.data();
		out.bufend = mem.data() + SINKSZ;
		out.buf = mem.data();
		out.bufsize = (int)SINKSZ;
		out.fd = -2;
		out.flags = 0;
	}
	long len() const { return (long)(out.nextc - out.buf); }
};

struct Cap {
	long len;
	std::vector<char> buf;

	bool operator==(const Cap &o) const
	{
		return len == o.len && buf == o.buf;
	}
};

static std::string
capdesc(const Cap &c)
{
	std::string s = "len=" + num(c.len) + " out=<" +
	    esc(c.buf.data(), (size_t)(c.len < 0 ? 0 :
	    (c.len > (long)SINKSZ ? (long)SINKSZ : c.len))) + "> full=" +
	    u64(fnv(c.buf.data(), c.buf.size()));
	return s;
}

static void
cmpCap(const char *fn, const Cap &a, const Cap &b, const std::string &in)
{
	CHK(fn, a == b, std::string(fn) + "(" + in + "): ref " + capdesc(a) +
	    " | port " + capdesc(b));
}

template <class A>
static Cap
c_outbin(const char *d, size_t l)
{
	Sink<A> s;
	A::outbin(d, l, &s.out);
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_outstr(const char *p)
{
	Sink<A> s;
	A::outstr(p, &s.out);
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_outcslow(int c)
{
	Sink<A> s;
	A::outcslow(c, &s.out);
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_byteseq(int c)
{
	Sink<A> s;
	A::byteseq(c, &s.out);
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_outdqstr(const char *p)
{
	Sink<A> s;
	A::outdqstr(p, &s.out);
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_outqstr(const char *p)
{
	Sink<A> s;
	A::outqstr(p, &s.out);
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_out1str(const char *p)
{
	Sink<A> s;
	typename A::Out **slot = A::var_out1();
	typename A::Out *save = *slot;
	*slot = &s.out;
	A::out1str(p);
	*slot = save;
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_out1qstr(const char *p)
{
	Sink<A> s;
	typename A::Out **slot = A::var_out1();
	typename A::Out *save = *slot;
	*slot = &s.out;
	A::out1qstr(p);
	*slot = save;
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_out2str(const char *p)
{
	Sink<A> s;
	typename A::Out **slot = A::var_out2();
	typename A::Out *save = *slot;
	*slot = &s.out;
	A::out2str(p);
	*slot = save;
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_out2qstr(const char *p)
{
	Sink<A> s;
	typename A::Out **slot = A::var_out2();
	typename A::Out *save = *slot;
	*slot = &s.out;
	A::out2qstr(p);
	*slot = save;
	return Cap{s.len(), s.mem};
}

template <class A>
static Cap
c_freestdout(long off)
{
	Sink<A> s;
	typename A::Out *o = A::var_output();
	typename A::Out saved = *o;

	o->buf = s.mem.data();
	o->nextc = s.mem.data() + off;
	o->bufend = s.mem.data() + SINKSZ;
	A::freestdout();
	long r = (long)(o->nextc - s.mem.data());
	*o = saved;
	return Cap{r, s.mem};
}

template <class A>
static Cap
c_outiserror(int flags)
{
	Sink<A> s;
	s.out.flags = (short)flags;
	int r = A::outiserror(&s.out);
	return Cap{(long)((r << 8) | (unsigned short)s.out.flags), s.mem};
}

template <class A>
static Cap
c_outclearerror(int flags)
{
	Sink<A> s;
	s.out.flags = (short)flags;
	A::outclearerror(&s.out);
	return Cap{(long)(unsigned short)s.out.flags, s.mem};
}

template <class A>
static Cap
c_fmtstr_sd(int length, const char *s, int d)
{
	std::vector<char> mem(SINKSZ, (char)0x7f);
	A::fmtstr_sd(mem.data(), length, s, d);
	return Cap{0, mem};
}

template <class A>
static Cap
c_fmtstr_s(int length, const char *s)
{
	std::vector<char> mem(SINKSZ, (char)0x7f);
	A::fmtstr_s(mem.data(), length, s);
	return Cap{0, mem};
}

static void
do_string_family(const std::string &s)
{
	const char *p = s.c_str();
	std::string in = "\"" + esc(s) + "\"";

	cmpCap("outstr", c_outstr<Ref>(p), c_outstr<Port>(p), in);
	cmpCap("outqstr", c_outqstr<Ref>(p), c_outqstr<Port>(p), in);
	cmpCap("outdqstr", c_outdqstr<Ref>(p), c_outdqstr<Port>(p), in);
	cmpCap("out1str", c_out1str<Ref>(p), c_out1str<Port>(p), in);
	cmpCap("out1qstr", c_out1qstr<Ref>(p), c_out1qstr<Port>(p), in);
	cmpCap("out2str", c_out2str<Ref>(p), c_out2str<Port>(p), in);
	cmpCap("out2qstr", c_out2qstr<Ref>(p), c_out2qstr<Port>(p), in);
}

static void
do_outbin(const std::string &s)
{
	std::string in = "\"" + esc(s) + "\",len=" + num((long long)s.size());
	cmpCap("outbin", c_outbin<Ref>(s.data(), s.size()),
	    c_outbin<Port>(s.data(), s.size()), in);
}

static void
test_output_edge(void)
{
	static const char *strs[] = {
		"", "a", "ab", "abc", " ", "\n", "\t", "\r", "'", "\\",
		"''", "'a'", "a'b", "[", "[]", "[a", "a[", "]", "~", "#",
		"=", "a=b", "*", "?", "|", "&", ";", "<", ">", "(", ")",
		"$", "`", "\"", "a b", "a\nb", "a\tb", "\x01", "\x1f",
		"\x20", "\x7f", "\x80", "\xff", "\x80\x80", "\xc3\xa9",
		"\xc3", "\xe2\x82\xac", "\xe2\x82", "\xe2", "\xf0\x9f\x92\xa9",
		"a\xff" "b", "\x01\x02\x03", "\n\n\n", "no_specials_here",
		"has space", "trailing ", " leading", "a\\b", "a\"b",
		"\xc3\xa9\x27", "\xff\x01", "\x1f\x80", "abc\x7f",
		"\x80z", "z\x80", "\xed\xa0\x80", "\xc0\x80", "\xf4\x90\x80\x80",
	};
	for (size_t i = 0; i < sizeof(strs) / sizeof(strs[0]); i++) {
		do_string_family(strs[i]);
		do_outbin(strs[i]);
	}

	/* embedded NUL and explicit length for outbin */
	static const char nulbuf[] = "a\0b\0\0c";
	for (size_t l = 0; l <= sizeof(nulbuf); l++)
		do_outbin(std::string(nulbuf, l));

	/* boundary lengths for outbin */
	for (size_t l = 0; l < 8; l++)
		do_outbin(std::string(l, 'x'));
	do_outbin(std::string(300, '\xa5'));

	/* outcslow / byteseq over the whole byte range and beyond */
	for (int c = -300; c <= 300; c++) {
		cmpCap("outcslow", c_outcslow<Ref>(c), c_outcslow<Port>(c),
		    num(c));
		cmpCap("byteseq", c_byteseq<Ref>(c), c_byteseq<Port>(c),
		    num(c));
	}
	static const int bs[] = {INT32_MIN, -1000000, -513, -512, -511, -65,
	    -64, -63, -9, -8, -7, 0, 7, 8, 9, 63, 64, 65, 511, 512, 513,
	    1000000, INT32_MAX};
	for (size_t i = 0; i < sizeof(bs) / sizeof(bs[0]); i++)
		cmpCap("byteseq", c_byteseq<Ref>(bs[i]), c_byteseq<Port>(bs[i]),
		    num(bs[i]));

	/* freestdout */
	for (long off = 0; off < 8; off++)
		cmpCap("freestdout", c_freestdout<Ref>(off),
		    c_freestdout<Port>(off), num(off));
	cmpCap("freestdout", c_freestdout<Ref>(1399),
	    c_freestdout<Port>(1399), "1399");

	/* outiserror / outclearerror */
	for (int f = -4; f < 8; f++) {
		cmpCap("outiserror", c_outiserror<Ref>(f),
		    c_outiserror<Port>(f), num(f));
		cmpCap("outclearerror", c_outclearerror<Ref>(f),
		    c_outclearerror<Port>(f), num(f));
	}

	/* fmtstr */
	static const char *fs[] = { "", "a", "abcdef", "0123456789abcdef",
	    "\xff\x80", "%s%d" };
	static const int lens[] = { -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	    15, 16, 17, 63, 64 };
	for (size_t i = 0; i < sizeof(fs) / sizeof(fs[0]); i++) {
		for (size_t j = 0; j < sizeof(lens) / sizeof(lens[0]); j++) {
			std::string in = num(lens[j]) + ",\"" +
			    esc(fs[i], strlen(fs[i])) + "\"";
			cmpCap("fmtstr", c_fmtstr_sd<Ref>(lens[j], fs[i], -7),
			    c_fmtstr_sd<Port>(lens[j], fs[i], -7), in);
			cmpCap("fmtstr", c_fmtstr_s<Ref>(lens[j], fs[i]),
			    c_fmtstr_s<Port>(lens[j], fs[i]), in);
		}
	}
}

/* ------------------------------------------------------------------ */
/* alias.c                                                            */
/* ------------------------------------------------------------------ */

static const int ATABSIZE = 39;

template <class A>
static std::string
dumpAliases(void)
{
	std::string s = "n=" + num(A::alias_count());
	for (int i = 0; i < ATABSIZE; i++) {
		for (const typename A::Alias *ap = A::atab_get(i); ap != NULL;
		    ap = ap->next) {
			s += "|" + num(i) + ":" + esc(ap->name,
			    strlen(ap->name)) + "=" + esc(ap->val,
			    strlen(ap->val)) + "/" + num(ap->flag);
		}
	}
	return s;
}

template <class A>
static std::string
nodeDesc(const typename A::Alias *p)
{
	if (p == NULL)
		return "NULL";
	for (int i = 0; i < ATABSIZE; i++) {
		int j = 0;
		for (const typename A::Alias *q = A::atab_get(i); q != NULL;
		    q = q->next, j++) {
			if (q == p)
				return "b" + num(i) + "#" + num(j) + ":" +
				    esc(p->name, strlen(p->name)) + "=" +
				    esc(p->val, strlen(p->val)) + "/" +
				    num(p->flag);
		}
	}
	return "detached:" + esc(p->name, strlen(p->name));
}

enum AliasOp {
	AO_SET = 0, AO_UNALIAS, AO_LOOKUP0, AO_LOOKUP1, AO_MARKUSE,
	AO_CLRUSE, AO_RMALIASES, AO_ITER, AO_HASH, AO_COMPARE, AO_NOP
};

static const char *
aliasOpName(int op)
{
	switch (op) {
	case AO_SET: return "setalias";
	case AO_UNALIAS: return "unalias";
	case AO_LOOKUP0: return "lookupalias";
	case AO_LOOKUP1: return "lookupalias";
	case AO_MARKUSE: return "lookupalias";
	case AO_CLRUSE: return "lookupalias";
	case AO_RMALIASES: return "rmaliases";
	case AO_ITER: return "iteralias";
	case AO_HASH: return "hashalias";
	case AO_COMPARE: return "comparealiases";
	}
	return "?";
}

template <class A>
static std::string
aliasOp(int op, const char *n, const char *v)
{
	switch (op) {
	case AO_SET:
		A::setalias(n, v);
		return "";
	case AO_UNALIAS:
		return num(A::unalias(n));
	case AO_LOOKUP0:
		return nodeDesc<A>(A::lookupalias(n, 0));
	case AO_LOOKUP1:
		return nodeDesc<A>(A::lookupalias(n, 1));
	case AO_MARKUSE: {
		typename A::Alias *ap = A::lookupalias(n, 0);
		if (ap == NULL)
			return "miss";
		ap->flag |= 1;
		return "inuse " + nodeDesc<A>(ap);
	}
	case AO_CLRUSE: {
		typename A::Alias *ap = A::lookupalias(n, 0);
		if (ap == NULL)
			return "miss";
		ap->flag &= ~1;
		return "clear " + nodeDesc<A>(ap);
	}
	case AO_RMALIASES:
		A::rmaliases();
		return "";
	case AO_ITER: {
		std::string s;
		const typename A::Alias *it = NULL;
		int guard = 0;
		while ((it = A::iteralias(it)) != NULL && guard++ < 400)
			s += nodeDesc<A>(it) + ",";
		return s;
	}
	case AO_HASH:
		return num((long long)A::hashalias(n));
	case AO_COMPARE: {
		typename A::Alias a1, a2;
		a1.name = (char *)n;
		a1.val = (char *)v;
		a1.flag = 0;
		a1.next = NULL;
		a2.name = (char *)v;
		a2.val = (char *)n;
		a2.flag = 0;
		a2.next = NULL;
		const typename A::Alias *pa1 = &a1;
		const typename A::Alias *pa2 = &a2;
		int r = A::comparealiases(&pa1, &pa2);
		int r2 = A::comparealiases(&pa1, &pa1);
		return num(r) + "/" + num(r < 0 ? -1 : (r > 0 ? 1 : 0)) + "/" +
		    num(r2);
	}
	}
	return "?";
}

template <class A>
static Trace
tr_alias(const std::vector<std::pair<int, std::pair<std::string, std::string> > > &ops)
{
	Trace t;
	for (size_t i = 0; i < ops.size(); i++) {
		int op = ops[i].first;
		const std::string &n = ops[i].second.first;
		const std::string &v = ops[i].second.second;
		std::string r = aliasOp<A>(op, n.c_str(), v.c_str());
		t.push_back(std::make_pair(std::string(aliasOpName(op)),
		    "in=(" + esc(n) + "," + esc(v) + ") r=" + r + " tab=" +
		    dumpAliases<A>()));
	}
	return t;
}

static void
runAliasOps(const std::vector<std::pair<int, std::pair<std::string,
    std::string> > > &ops, const char *what)
{
	cmpTrace(tr_alias<Ref>(ops), tr_alias<Port>(ops), what);
}

static void
addOp(std::vector<std::pair<int, std::pair<std::string, std::string> > > &v,
    int op, const std::string &n, const std::string &val)
{
	v.push_back(std::make_pair(op, std::make_pair(n, val)));
}

static void
test_alias_edge(void)
{
	std::vector<std::pair<int, std::pair<std::string, std::string> > > ops;

	/* hashalias corner cases (empty, single byte, high bit, long) */
	static const char *hs[] = {
		"", "a", "b", "z", "A", "0", "\x01", "\x7f", "\x80", "\xff",
		"\xff\xff", "\x80\x01", "ab", "ba", "abc", "aaaaaaaa",
		"\xff\x80\x01\x7f", "the quick brown fox",
		"\xc3\xa9\xc3\xa9", "=", "==", "aa", "j", "m",
	};
	for (size_t i = 0; i < sizeof(hs) / sizeof(hs[0]); i++) {
		addOp(ops, AO_HASH, hs[i], "");
		addOp(ops, AO_COMPARE, hs[i], hs[(i + 1) % (sizeof(hs) /
		    sizeof(hs[0]))]);
	}
	runAliasOps(ops, "hash/compare");
	ops.clear();

	/* build a table, exercise collisions, in-use marking and removal */
	addOp(ops, AO_RMALIASES, "", "");
	addOp(ops, AO_ITER, "", "");
	addOp(ops, AO_LOOKUP0, "nothing", "");
	addOp(ops, AO_UNALIAS, "nothing", "");
	addOp(ops, AO_SET, "ls", "ls -F");
	addOp(ops, AO_LOOKUP0, "ls", "");
	addOp(ops, AO_LOOKUP1, "ls", "");
	addOp(ops, AO_ITER, "", "");
	addOp(ops, AO_SET, "ls", "ls -l");	/* replace */
	addOp(ops, AO_ITER, "", "");
	addOp(ops, AO_SET, "", "empty name");
	addOp(ops, AO_LOOKUP0, "", "");
	addOp(ops, AO_ITER, "", "");
	/* "ab" and "ba" collide only if their hashes agree; add many */
	for (int i = 0; i < 40; i++) {
		char nm[16], vl[16];
		snprintf(nm, sizeof(nm), "k%d", i);
		snprintf(vl, sizeof(vl), "v%d", i);
		addOp(ops, AO_SET, nm, vl);
	}
	addOp(ops, AO_ITER, "", "");
	for (int i = 0; i < 40; i += 3) {
		char nm[16];
		snprintf(nm, sizeof(nm), "k%d", i);
		addOp(ops, AO_MARKUSE, nm, "");
	}
	addOp(ops, AO_ITER, "", "");
	for (int i = 0; i < 40; i += 3) {
		char nm[16];
		snprintf(nm, sizeof(nm), "k%d", i);
		addOp(ops, AO_LOOKUP0, nm, "");
		addOp(ops, AO_LOOKUP1, nm, "");
		addOp(ops, AO_UNALIAS, nm, "");	/* in use: name nulled */
		addOp(ops, AO_LOOKUP0, nm, "");
		addOp(ops, AO_ITER, "", "");
	}
	addOp(ops, AO_LOOKUP0, "", "");
	addOp(ops, AO_UNALIAS, "", "");
	addOp(ops, AO_ITER, "", "");
	for (int i = 1; i < 40; i += 2) {
		char nm[16];
		snprintf(nm, sizeof(nm), "k%d", i);
		addOp(ops, AO_UNALIAS, nm, "");
		addOp(ops, AO_UNALIAS, nm, "");	/* second time: miss */
	}
	addOp(ops, AO_ITER, "", "");
	/* high bit names */
	addOp(ops, AO_SET, "\x80", "hi");
	addOp(ops, AO_SET, "\xff\xfe", "hi2");
	addOp(ops, AO_SET, "\x01\x02", "lo");
	addOp(ops, AO_ITER, "", "");
	addOp(ops, AO_LOOKUP0, "\x80", "");
	addOp(ops, AO_UNALIAS, "\x80", "");
	addOp(ops, AO_ITER, "", "");
	addOp(ops, AO_MARKUSE, "\xff\xfe", "");
	addOp(ops, AO_CLRUSE, "\xff\xfe", "");
	addOp(ops, AO_LOOKUP1, "\xff\xfe", "");
	addOp(ops, AO_RMALIASES, "", "");
	addOp(ops, AO_ITER, "", "");
	addOp(ops, AO_LOOKUP0, "k1", "");
	runAliasOps(ops, "alias table");
	ops.clear();

	/* rmaliases with in-use entries left behind */
	addOp(ops, AO_RMALIASES, "", "");
	for (int i = 0; i < 20; i++) {
		char nm[16], vl[16];
		snprintf(nm, sizeof(nm), "r%d", i);
		snprintf(vl, sizeof(vl), "w%d", i);
		addOp(ops, AO_SET, nm, vl);
	}
	for (int i = 0; i < 20; i += 2) {
		char nm[16];
		snprintf(nm, sizeof(nm), "r%d", i);
		addOp(ops, AO_MARKUSE, nm, "");
	}
	addOp(ops, AO_RMALIASES, "", "");
	addOp(ops, AO_ITER, "", "");
	addOp(ops, AO_SET, "after", "rm");
	addOp(ops, AO_ITER, "", "");
	addOp(ops, AO_RMALIASES, "", "");
	addOp(ops, AO_RMALIASES, "", "");
	addOp(ops, AO_ITER, "", "");
	runAliasOps(ops, "rmaliases");
}

/* freealias, exercised on nodes the harness owns */
template <class A>
static std::string
c_freealias(const char *n, const char *v)
{
	typename A::Alias *ap = (typename A::Alias *)A::ckmalloc(
	    sizeof(typename A::Alias));
	ap->name = A::savestr(n);
	ap->val = A::savestr(v);
	ap->flag = 0;
	ap->next = NULL;
	std::string s = esc(ap->name, strlen(ap->name)) + "=" +
	    esc(ap->val, strlen(ap->val)) + "/" + num(ap->flag);
	A::freealias(ap);
	return s;
}

static void
test_freealias(void)
{
	static const char *pairs[][2] = {
		{ "", "" }, { "a", "" }, { "", "b" }, { "name", "value" },
		{ "\xff", "\x80" }, { "\x01", "\x7f" },
	};
	for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++) {
		std::string a = c_freealias<Ref>(pairs[i][0], pairs[i][1]);
		std::string b = c_freealias<Port>(pairs[i][0], pairs[i][1]);
		CHK("freealias", a == b, "freealias: ref=" + a + " port=" + b);
	}
}

/* ------------------------------------------------------------------ */
/* memalloc.c                                                         */
/* ------------------------------------------------------------------ */

template <class A>
static std::string
stackState(void)
{
	int nleft, depth;
	long nxtoff, ssoff;

	A::stack_probe(&nleft, &nxtoff, &ssoff, &depth);
	return "{nleft=" + num(nleft) + " nxtoff=" + num(nxtoff) + " ssoff=" +
	    num(ssoff) + " depth=" + num(depth) + "}";
}

template <class A>
static std::string
ptrOff(const char *p)
{
	if (p == NULL)
		return "null";
	char *base = A::get_stackp();
	if (base == NULL)
		return "nobase";
	return num((long long)(p - base));
}

/*
 * Make sure a stack block exists so that the string routines are never handed
 * a NULL cursor.  Both sides take this branch on the same steps.
 */
template <class A>
static void
ensureBlock(void)
{
	if (A::get_stacknxt() == NULL) {
		char *w = (char *)A::stalloc(1);
		A::stunalloc(w);
	}
}

/* content of the free region, as far as it is deterministic */
template <class A>
static std::string
blockWindow(size_t window)
{
	const char *base = A::get_stacknxt();
	if (base == NULL)
		return "nobase";
	size_t n = window;
	if ((long)n > (long)A::get_stacknleft())
		n = (size_t)(A::get_stacknleft() < 0 ? 0 :
		    A::get_stacknleft());
	std::string s = "w=" + num((long long)n) + ":" + u64(fnv(base, n));
	if (n <= 96)
		s += ":<" + esc(base, n) + ">";
	return s;
}

template <class A>
static Trace
tr_stalloc(void)
{
	Trace t;
	typename A::Mark m;

	t.push_back(std::make_pair("stalloc", "enter " + stackState<A>()));
	A::setstackmark(&m);
	t.push_back(std::make_pair("setstackmark", "m " + stackState<A>()));

	static const int ns[] = { 0, 1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 32,
	    63, 64, 100, 200, 494, 495, 496, 497, 503, 504, 505, 1000, 2000,
	    4096 };
	std::vector<char *> ptrs;
	for (size_t i = 0; i < sizeof(ns) / sizeof(ns[0]); i++) {
		char *p = (char *)A::stalloc(ns[i]);
		if (p != NULL && ns[i] > 0)
			memset(p, 0x5a, (size_t)ns[i]);
		ptrs.push_back(p);
		t.push_back(std::make_pair("stalloc", "n=" + num(ns[i]) +
		    " p=" + ptrOff<A>(p) + " " + stackState<A>()));
	}

	/* exactly the remaining space, then one byte more */
	int left = A::get_stacknleft();
	char *p1 = (char *)A::stalloc(left);
	t.push_back(std::make_pair("stalloc", "n=nleft(" + num(left) +
	    ") p=" + ptrOff<A>(p1) + " " + stackState<A>()));
	char *p2 = (char *)A::stalloc(1);
	t.push_back(std::make_pair("stalloc", "n=1 after-exhaust p=" +
	    ptrOff<A>(p2) + " " + stackState<A>()));
	A::stunalloc(p2);
	t.push_back(std::make_pair("stunalloc", "p2 " + stackState<A>()));

	/* stacknleft == 1 boundary: ALIGN(1) == 8 > 1 */
	left = A::get_stacknleft();
	if (left > 1) {
		char *p3 = (char *)A::stalloc(left - 8);
		t.push_back(std::make_pair("stalloc", "n=left-8 p=" +
		    ptrOff<A>(p3) + " " + stackState<A>()));
		char *p4 = (char *)A::stalloc(8);
		t.push_back(std::make_pair("stalloc", "n=8 p=" +
		    ptrOff<A>(p4) + " " + stackState<A>()));
		char *p5 = (char *)A::stalloc(1);
		t.push_back(std::make_pair("stalloc", "n=1 p=" +
		    ptrOff<A>(p5) + " " + stackState<A>()));
	}

	A::popstackmark(&m);
	t.push_back(std::make_pair("popstackmark", "m " + stackState<A>()));
	return t;
}

template <class A>
static Trace
tr_stnewblock(void)
{
	Trace t;
	typename A::Mark m;

	A::setstackmark(&m);
	t.push_back(std::make_pair("setstackmark", "m " + stackState<A>()));
	static const int ns[] = { -1, 0, 1, 8, 494, 495, 496, 497, 512,
	    1000, 5000 };
	for (size_t i = 0; i < sizeof(ns) / sizeof(ns[0]); i++) {
		A::stnewblock(ns[i]);
		t.push_back(std::make_pair("stnewblock", "n=" + num(ns[i]) +
		    " " + stackState<A>()));
	}
	A::popstackmark(&m);
	t.push_back(std::make_pair("popstackmark", "m " + stackState<A>()));
	return t;
}

template <class A>
static Trace
tr_marks(void)
{
	Trace t;
	typename A::Mark m0, m1, m2;

	A::setstackmark(&m0);
	t.push_back(std::make_pair("setstackmark", "m0 " + stackState<A>()));

	/* fresh block, then rewind so that stacknxt == SPACE(stackp) */
	char *p = (char *)A::stalloc(A::get_stacknleft() + 1);
	t.push_back(std::make_pair("stalloc", "fresh p=" + ptrOff<A>(p) +
	    " " + stackState<A>()));
	A::stunalloc(p);
	t.push_back(std::make_pair("stunalloc", "rewound " + stackState<A>()));
	/* setstackmark must now insert a stalloc(1) */
	A::setstackmark(&m1);
	t.push_back(std::make_pair("setstackmark", "m1-at-space " +
	    stackState<A>()));
	char *q = (char *)A::stsavestr("hello stack");
	t.push_back(std::make_pair("stsavestr", "p=" + ptrOff<A>(q) +
	    " s=<" + esc(q, strlen(q) + 1) + "> " + stackState<A>()));
	char *q2 = (char *)A::stsavestr("");
	t.push_back(std::make_pair("stsavestr", "p=" + ptrOff<A>(q2) +
	    " s=<" + esc(q2, 1) + "> " + stackState<A>()));
	char *q3 = (char *)A::stsavestr("\xff\x80\x01");
	t.push_back(std::make_pair("stsavestr", "p=" + ptrOff<A>(q3) +
	    " s=<" + esc(q3, strlen(q3) + 1) + "> " + stackState<A>()));

	/* drive stacknleft to exactly zero, then mark and pop */
	int left = A::get_stacknleft();
	char *r = (char *)A::stalloc(left);
	t.push_back(std::make_pair("stalloc", "drain p=" + ptrOff<A>(r) +
	    " " + stackState<A>()));
	A::setstackmark(&m2);
	t.push_back(std::make_pair("setstackmark", "m2-empty " +
	    stackState<A>()));
	char *s = (char *)A::stalloc(3);
	t.push_back(std::make_pair("stalloc", "n=3 p=" + ptrOff<A>(s) + " " +
	    stackState<A>()));
	A::popstackmark(&m2);
	t.push_back(std::make_pair("popstackmark", "m2 " + stackState<A>()));
	A::popstackmark(&m1);
	t.push_back(std::make_pair("popstackmark", "m1 " + stackState<A>()));
	A::popstackmark(&m0);
	t.push_back(std::make_pair("popstackmark", "m0 " + stackState<A>()));
	return t;
}

template <class A>
static Trace
tr_growstackstr(int count)
{
	Trace t;
	typename A::Mark m;

	A::setstackmark(&m);
	ensureBlock<A>();
	/* guard fill the whole free region */
	if (A::get_stacknleft() > 0)
		memset(A::get_stacknxt(), 0x7f, (size_t)A::get_stacknleft());
	size_t fill = (size_t)(A::get_stacknleft() > 0 ?
	    A::get_stacknleft() : 0);
	t.push_back(std::make_pair("growstackstr", "enter " + stackState<A>() +
	    " " + blockWindow<A>(fill)));

	char *p = A::get_stacknxt();
	size_t written = 0;
	for (int i = 0; i < count; i++) {
		bool grew = false;
		if (p == A::get_sstrend()) {
			p = A::growstackstr();
			grew = true;
		}
		*p++ = (char)(i * 7 + 1);
		written++;
		size_t win = std::max(fill, written);
		if (grew || i < 16 || (i % 97) == 0 || i == count - 1) {
			t.push_back(std::make_pair("growstackstr", "i=" +
			    num(i) + (grew ? " GREW" : "") + " off=" +
			    num((long long)(p - A::get_stacknxt())) + " " +
			    stackState<A>() + " " + blockWindow<A>(win)));
		}
	}
	A::popstackmark(&m);
	t.push_back(std::make_pair("popstackmark", "m " + stackState<A>()));
	return t;
}

template <class A>
static Trace
tr_makestrspace(void)
{
	Trace t;
	typename A::Mark outer;

	A::setstackmark(&outer);
	ensureBlock<A>();
	static const int mins[] = { 0, 1, 2, 7, 8, 9, 100, 255, 256, 400,
	    495, 496, 497, 511, 512, 513, 1000, 1024, 1025, 4096 };
	for (size_t i = 0; i < sizeof(mins) / sizeof(mins[0]); i++) {
		/* p at three positions inside the block */
		for (int k = 0; k < 3; k++) {
			typename A::Mark m;
			A::setstackmark(&m);
			if (A::get_stacknleft() > 0)
				memset(A::get_stacknxt(), 0x7f,
				    (size_t)A::get_stacknleft());
			size_t fill = (size_t)(A::get_stacknleft() > 0 ?
			    A::get_stacknleft() : 0);
			int left = A::get_stacknleft();
			long off = k == 0 ? 0 : (k == 1 ? left / 2 : left);
			char *p = A::get_stacknxt() + off;
			char *r = A::makestrspace(mins[i], p);
			t.push_back(std::make_pair("makestrspace", "min=" +
			    num(mins[i]) + " off=" + num(off) + " r=" +
			    num((long long)(r - A::get_stacknxt())) + " " +
			    stackState<A>() + " " + blockWindow<A>(fill)));
			A::popstackmark(&m);
			t.push_back(std::make_pair("popstackmark",
			    "makestrspace " + stackState<A>()));
		}
	}

	/* growstrstackblock and growstackblock directly */
	static const int gs[] = { 0, 1, 8, 512, 513, 4096 };
	for (size_t i = 0; i < sizeof(gs) / sizeof(gs[0]); i++) {
		typename A::Mark m;
		A::setstackmark(&m);
		if (A::get_stacknleft() > 0)
			memset(A::get_stacknxt(), 0x7f,
			    (size_t)A::get_stacknleft());
		size_t fill = (size_t)(A::get_stacknleft() > 0 ?
		    A::get_stacknleft() : 0);
		A::growstackblock(gs[i]);
		t.push_back(std::make_pair("growstackblock", "min=" +
		    num(gs[i]) + " " + stackState<A>() + " " +
		    blockWindow<A>(fill)));
		char *r = A::growstrstackblock(gs[i] % 7, gs[i]);
		t.push_back(std::make_pair("growstrstackblock", "n=" +
		    num(gs[i] % 7) + " min=" + num(gs[i]) + " r=" +
		    num((long long)(r - A::get_stacknxt())) + " " +
		    stackState<A>() + " " + blockWindow<A>(fill)));
		A::popstackmark(&m);
		t.push_back(std::make_pair("popstackmark", "grow " +
		    stackState<A>()));
	}
	A::popstackmark(&outer);
	t.push_back(std::make_pair("popstackmark", "outer " +
	    stackState<A>()));
	return t;
}

/*
 * growstackblock() rounds up to a power of two with
 *
 *	newlen = 512; while (newlen < min) newlen <<= 1;
 *
 * so the interesting inputs are the ones where the requested total lands
 * exactly on a power of two.  Aim at 2^k - 1, 2^k and 2^k + 1 for the total,
 * which means asking for 2^k - stacknleft - ALIGN(sizeof(struct stack_block))
 * plus the delta.
 */
template <class A>
static Trace
tr_grow_exact(void)
{
	Trace t;
	typename A::Mark outer;

	A::setstackmark(&outer);
	ensureBlock<A>();
	for (int k = 10; k <= 15; k++) {
		for (int delta = -1; delta <= 1; delta++) {
			typename A::Mark m;
			A::setstackmark(&m);
			int left = A::get_stacknleft();
			long target = 1L << k;
			long arg = target - left - 8 + delta;
			if (arg > left && arg < 1000000L) {
				A::growstackblock((int)arg);
				t.push_back(std::make_pair("growstackblock",
				    "k=" + num(k) + " d=" + num(delta) +
				    " left=" + num(left) + " arg=" +
				    num(arg) + " " + stackState<A>()));
				char *p = A::get_stacknxt();
				char *r = A::makestrspace((int)arg, p);
				t.push_back(std::make_pair("makestrspace",
				    "k=" + num(k) + " d=" + num(delta) +
				    " arg=" + num(arg) + " r=" +
				    num((long long)(r - A::get_stacknxt())) +
				    " " + stackState<A>()));
			} else {
				t.push_back(std::make_pair("growstackblock",
				    "k=" + num(k) + " d=" + num(delta) +
				    " skipped left=" + num(left)));
			}
			A::popstackmark(&m);
			t.push_back(std::make_pair("popstackmark", "exact " +
			    stackState<A>()));
		}
	}
	A::popstackmark(&outer);
	t.push_back(std::make_pair("popstackmark", "exact-outer " +
	    stackState<A>()));
	return t;
}

template <class A>
static Trace
tr_stputbin(void)
{
	Trace t;
	typename A::Mark m;

	A::setstackmark(&m);
	ensureBlock<A>();
	static const char data[] =
	    "0123456789abcdefghijklmnopqrstuvwxyz\xff\x80\x01\x7f";

	/* small lengths, no growth */
	for (size_t l = 0; l <= 40; l++) {
		if (A::get_stacknleft() > 0)
			memset(A::get_stacknxt(), 0x7f,
			    (size_t)A::get_stacknleft());
		size_t fill = (size_t)(A::get_stacknleft() > 0 ?
		    A::get_stacknleft() : 0);
		char *p = A::get_stacknxt();
		char *r = A::stputbin(data, l, p);
		t.push_back(std::make_pair("stputbin", "len=" +
		    num((long long)l) + " r=" +
		    num((long long)(r - A::get_stacknxt())) + " " +
		    stackState<A>() + " " + blockWindow<A>(fill)));
	}

	/* exactly the available space, one under and one over */
	for (int delta = -1; delta <= 1; delta++) {
		if (A::get_stacknleft() > 0)
			memset(A::get_stacknxt(), 0x7f,
			    (size_t)A::get_stacknleft());
		size_t fill = (size_t)(A::get_stacknleft() > 0 ?
		    A::get_stacknleft() : 0);
		char *p = A::get_stacknxt();
		long avail = (long)(A::get_sstrend() - p) + delta;
		if (avail < 0)
			avail = 0;
		std::string big((size_t)avail, 'Q');
		for (size_t i = 0; i < big.size(); i++)
			big[i] = (char)(i * 31 + 5);
		char *r = A::stputbin(big.data(), big.size(), p);
		size_t win = std::max(fill, big.size());
		t.push_back(std::make_pair("stputbin", "avail" +
		    std::string(delta < 0 ? "-1" : (delta ? "+1" : "+0")) +
		    "=" + num(avail) + " r=" +
		    num((long long)(r - A::get_stacknxt())) + " " +
		    stackState<A>() + " " + blockWindow<A>(win)));
	}

	/* stputs */
	static const char *ss[] = { "", "a", "abc", "\xff\x80",
	    "0123456789012345678901234567890123456789" };
	for (size_t i = 0; i < sizeof(ss) / sizeof(ss[0]); i++) {
		if (A::get_stacknleft() > 0)
			memset(A::get_stacknxt(), 0x7f,
			    (size_t)A::get_stacknleft());
		size_t fill = (size_t)(A::get_stacknleft() > 0 ?
		    A::get_stacknleft() : 0);
		char *p = A::get_stacknxt();
		char *r = A::stputs(ss[i], p);
		t.push_back(std::make_pair("stputs", "s=<" +
		    esc(ss[i], strlen(ss[i])) + "> r=" +
		    num((long long)(r - A::get_stacknxt())) + " " +
		    stackState<A>() + " " + blockWindow<A>(fill)));
	}

	/* a chain of appends that has to cross a block boundary */
	if (A::get_stacknleft() > 0)
		memset(A::get_stacknxt(), 0x7f, (size_t)A::get_stacknleft());
	size_t fill = (size_t)(A::get_stacknleft() > 0 ?
	    A::get_stacknleft() : 0);
	char *p = A::get_stacknxt();
	size_t written = 0;
	for (int i = 0; i < 60; i++) {
		char buf[37];
		for (size_t k = 0; k < sizeof(buf); k++)
			buf[k] = (char)(i * 13 + k);
		p = A::stputbin(buf, sizeof(buf), p);
		written += sizeof(buf);
		t.push_back(std::make_pair("stputbin", "chain i=" + num(i) +
		    " off=" + num((long long)(p - A::get_stacknxt())) + " " +
		    stackState<A>() + " " +
		    blockWindow<A>(std::max(fill, written))));
	}

	A::popstackmark(&m);
	t.push_back(std::make_pair("popstackmark", "m " + stackState<A>()));
	return t;
}

template <class A>
static Trace
tr_ckalloc(void)
{
	Trace t;
	static const size_t ns[] = { 0, 1, 2, 7, 8, 15, 16, 100, 4096 };

	for (size_t i = 0; i < sizeof(ns) / sizeof(ns[0]); i++) {
		char *p = (char *)A::ckmalloc(ns[i]);
		t.push_back(std::make_pair("ckmalloc", "n=" +
		    num((long long)ns[i]) + " nonnull=" +
		    num(p != NULL)));
		if (p != NULL && ns[i] > 0)
			memset(p, 0x33, ns[i]);
		p = (char *)A::ckrealloc(p, (int)(ns[i] + 64));
		if (p != NULL) {
			memset(p + ns[i], 0x44, 64);
			t.push_back(std::make_pair("ckrealloc", "n=" +
			    num((long long)ns[i] + 64) + " content=" +
			    u64(fnv(p, ns[i] + 64))));
		} else {
			t.push_back(std::make_pair("ckrealloc", "null"));
		}
		A::ckfree(p);
		t.push_back(std::make_pair("ckfree", "done"));
	}

	static const char *ss[] = { "", "a", "abc", "\xff\x80\x01",
	    "a string with spaces", "\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f" };
	for (size_t i = 0; i < sizeof(ss) / sizeof(ss[0]); i++) {
		char *p = A::savestr(ss[i]);
		t.push_back(std::make_pair("savestr", "s=<" +
		    esc(p, strlen(p) + 1) + "> len=" +
		    num((long long)strlen(p))));
		A::ckfree(p);
	}
	return t;
}

/* the "Out of space" path of growstackblock() */
template <class A>
static std::string
c_growstackblock_oom(int min)
{
	volatile int hit = 0;

	A::err_arm(1);
	if (setjmp(*A::err_jmp()) == 0) {
		char *p = A::get_stacknxt();
		A::makestrspace(min, p);
		hit = 0;
	} else {
		hit = 1;
	}
	A::err_arm(0);
	return "error=" + num(hit) + " " + stackState<A>();
}

static void
test_oom(void)
{
	static const int mins[] = {
		INT32_MAX / 2 - 8,	/* exactly the limit */
		INT32_MAX / 2 - 7,
		INT32_MAX / 2,
		INT32_MAX - 1,
		INT32_MAX,
	};
	for (size_t i = 0; i < sizeof(mins) / sizeof(mins[0]); i++) {
		std::string a = c_growstackblock_oom<Ref>(mins[i]);
		std::string b = c_growstackblock_oom<Port>(mins[i]);
		CHK("growstackblock", a == b, "oom min=" + num(mins[i]) +
		    ": ref=" + a + " port=" + b);
	}
}

/* the badalloc() path, in a child process */
template <class A>
static std::string
c_badalloc(int which)
{
	char tmpl[] = "/tmp/b0219badallocXXXXXX";
	int fd = mkstemp(tmpl);
	if (fd < 0)
		return "mkstemp failed";

	fflush(NULL);
	pid_t pid = fork();
	if (pid == 0) {
		struct rlimit rl;
		rl.rlim_cur = 0;
		rl.rlim_max = 0;
		setrlimit(RLIMIT_CORE, &rl);
		dup2(fd, 2);
		A::set_suppressint(0);
		switch (which) {
		case 0:
			A::ckmalloc(16);
			break;
		case 1:
			A::ckrealloc(NULL, 16);
			break;
		default:
			A::ckfree(NULL);
			break;
		}
		A::set_suppressint(1);
		_exit(99);
	}
	int status = 0;
	waitpid(pid, &status, 0);

	char buf[256];
	lseek(fd, 0, SEEK_SET);
	ssize_t n = read(fd, buf, sizeof(buf));
	if (n < 0)
		n = 0;
	close(fd);
	unlink(tmpl);

	std::string s;
	if (WIFSIGNALED(status))
		s = "signal=" + num(WTERMSIG(status));
	else
		s = "exit=" + num(WEXITSTATUS(status));
	s += " msg=<" + esc(buf, (size_t)n) + ">";
	return s;
}

static void
test_badalloc(void)
{
	static const char *names[] = { "ckmalloc", "ckrealloc", "ckfree" };
	for (int i = 0; i < 3; i++) {
		std::string a = c_badalloc<Ref>(i);
		std::string b = c_badalloc<Port>(i);
		CHK("badalloc", a == b, std::string("badalloc via ") +
		    names[i] + ": ref=" + a + " port=" + b);
		CHK(names[i], a == b, std::string("unsafe ") + names[i] +
		    ": ref=" + a + " port=" + b);
	}
}

/* ------------------------------------------------------------------ */
/* randomised sweeps                                                  */
/* ------------------------------------------------------------------ */

static void
sweep_output(int iters)
{
	for (int i = 0; i < iters; i++) {
		std::string s = randstr(rndn(4) == 0 ? 250 : 24);
		do_string_family(s);
		if ((i & 3) == 0) {
			std::string b = randstr(60, true);
			do_outbin(b);
		}
		if ((i & 7) == 0) {
			int c = (int)(rnd() % 1024) - 512;
			cmpCap("outcslow", c_outcslow<Ref>(c),
			    c_outcslow<Port>(c), num(c));
			cmpCap("byteseq", c_byteseq<Ref>(c),
			    c_byteseq<Port>(c), num(c));
			int len = (int)(rnd() % 40) - 2;
			std::string f = randstr(20);
			cmpCap("fmtstr", c_fmtstr_sd<Ref>(len, f.c_str(), i),
			    c_fmtstr_sd<Port>(len, f.c_str(), i),
			    num(len) + ",\"" + esc(f) + "\"");
		}
	}
}

static void
sweep_alias(int iters)
{
	std::vector<std::string> names;
	for (int i = 0; i < 64; i++)
		names.push_back(randstr(6));
	names.push_back("");
	names.push_back("\x80");
	names.push_back("\xff\xff");

	std::vector<std::pair<int, std::pair<std::string, std::string> > > ops;
	for (int i = 0; i < iters; i++) {
		unsigned k = rndn(100);
		int op;
		if (k < 32)
			op = AO_SET;
		else if (k < 50)
			op = AO_UNALIAS;
		else if (k < 62)
			op = AO_LOOKUP0;
		else if (k < 72)
			op = AO_LOOKUP1;
		else if (k < 78)
			op = AO_MARKUSE;
		else if (k < 82)
			op = AO_CLRUSE;
		else if (k < 86)
			op = AO_ITER;
		else if (k < 94)
			op = AO_HASH;
		else if (k < 99)
			op = AO_COMPARE;
		else
			op = AO_RMALIASES;
		const std::string &n = names[rndn((unsigned)names.size())];
		const std::string &v = names[rndn((unsigned)names.size())];
		addOp(ops, op, n, v);
		if (ops.size() >= 500) {
			runAliasOps(ops, "alias sweep");
			ops.clear();
		}
	}
	if (!ops.empty())
		runAliasOps(ops, "alias sweep");
	std::vector<std::pair<int, std::pair<std::string, std::string> > > fin;
	addOp(fin, AO_RMALIASES, "", "");
	runAliasOps(fin, "alias sweep cleanup");
}

/* a scripted random walk over the stack allocator */
struct StackStep {
	int op;
	int arg;
};

template <class A>
static int
stackDepth(void)
{
	int nleft, depth;
	long nxtoff, ssoff;

	A::stack_probe(&nleft, &nxtoff, &ssoff, &depth);
	return depth;
}

template <class A>
static Trace
tr_stack_walk(const std::vector<StackStep> &steps)
{
	Trace t;
	typename A::Mark base;
	A::setstackmark(&base);
	ensureBlock<A>();

	std::vector<std::pair<char *, int> > live;
	std::vector<typename A::Mark> marks;
	char *strp = A::get_stacknxt();
	size_t fill = 0;

	if (A::get_stacknleft() > 0) {
		memset(A::get_stacknxt(), 0x7f, (size_t)A::get_stacknleft());
		fill = (size_t)A::get_stacknleft();
	}

	for (size_t i = 0; i < steps.size(); i++) {
		int op = steps[i].op;
		int arg = steps[i].arg;
		std::string d;
		const char *fn = "?";
		bool reset = false;
		switch (op) {
		case 0: {
			fn = "stalloc";
			char *p = (char *)A::stalloc(arg);
			if (p != NULL && arg > 0)
				memset(p, (char)arg, (size_t)arg);
			live.push_back(std::make_pair(p, stackDepth<A>()));
			d = "n=" + num(arg) + " p=" + ptrOff<A>(p);
			reset = true;
			break;
		}
		case 1: {
			fn = "stunalloc";
			if (live.empty()) {
				d = "skip-empty";
				break;
			}
			char *p = live.back().first;
			int dep = live.back().second;
			live.pop_back();
			if (p == NULL) {
				d = "skip-null";
				break;
			}
			if (dep != stackDepth<A>()) {
				/* not in the current block any more */
				d = "skip-depth";
				break;
			}
			A::stunalloc(p);
			d = "p=" + ptrOff<A>(p);
			reset = true;
			break;
		}
		case 2: {
			fn = "stsavestr";
			char b[24];
			snprintf(b, sizeof(b), "s%d", arg);
			char *p = A::stsavestr(b);
			live.push_back(std::make_pair(p, stackDepth<A>()));
			d = "s=<" + std::string(b) + "> p=" + ptrOff<A>(p) +
			    " back=<" + esc(p, strlen(p) + 1) + ">";
			reset = true;
			break;
		}
		case 3: {
			fn = "setstackmark";
			typename A::Mark m;
			A::setstackmark(&m);
			marks.push_back(m);
			/*
			 * Allocations made before the mark may no longer be
			 * unwound: stunalloc()ing below a live mark would let
			 * growstackblock() realloc the marked block.
			 */
			live.clear();
			d = "depth=" + num((long long)marks.size());
			reset = true;
			break;
		}
		case 4: {
			fn = "popstackmark";
			if (marks.empty()) {
				d = "skip";
				break;
			}
			typename A::Mark m = marks.back();
			marks.pop_back();
			A::popstackmark(&m);
			live.clear();
			d = "depth=" + num((long long)marks.size());
			reset = true;
			break;
		}
		case 5: {
			fn = "growstackstr";
			char *p = A::growstackstr();
			d = "r=" + num((long long)(p - A::get_stacknxt()));
			reset = true;
			break;
		}
		case 6: {
			fn = "makestrspace";
			char *p = A::makestrspace(arg, strp);
			d = "min=" + num(arg) + " r=" +
			    num((long long)(p - A::get_stacknxt()));
			strp = p;
			break;
		}
		case 7: {
			fn = "stputbin";
			std::string s((size_t)(arg & 63), 'z');
			for (size_t k = 0; k < s.size(); k++)
				s[k] = (char)(arg + k * 7);
			strp = A::stputbin(s.data(), s.size(), strp);
			d = "len=" + num((long long)s.size()) + " off=" +
			    num((long long)(strp - A::get_stacknxt()));
			break;
		}
		default: {
			fn = "stputs";
			char b[32];
			snprintf(b, sizeof(b), "put%d!", arg);
			strp = A::stputs(b, strp);
			d = "s=<" + std::string(b) + "> off=" +
			    num((long long)(strp - A::get_stacknxt()));
			break;
		}
		}

		size_t used = (size_t)(strp - A::get_stacknxt());
		t.push_back(std::make_pair(std::string(fn), d + " " +
		    stackState<A>() + " " +
		    blockWindow<A>(std::max(fill, used))));

		if (reset) {
			/* the free region moved: re-arm the guard bytes */
			strp = A::get_stacknxt();
			fill = 0;
			if (A::get_stacknleft() > 0) {
				memset(A::get_stacknxt(), 0x7f,
				    (size_t)A::get_stacknleft());
				fill = (size_t)A::get_stacknleft();
			}
		}
		/* keep the doubling in growstackblock() bounded */
		if (A::get_stacknleft() > (1 << 18)) {
			while (!marks.empty()) {
				typename A::Mark m = marks.back();
				marks.pop_back();
				A::popstackmark(&m);
			}
			live.clear();
			A::popstackmark(&base);
			A::setstackmark(&base);
			ensureBlock<A>();
			strp = A::get_stacknxt();
			fill = 0;
			if (A::get_stacknleft() > 0) {
				memset(A::get_stacknxt(), 0x7f,
				    (size_t)A::get_stacknleft());
				fill = (size_t)A::get_stacknleft();
			}
			t.push_back(std::make_pair("popstackmark", "rewind " +
			    stackState<A>()));
		}
	}

	while (!marks.empty()) {
		typename A::Mark m = marks.back();
		marks.pop_back();
		A::popstackmark(&m);
	}
	A::popstackmark(&base);
	t.push_back(std::make_pair("popstackmark", "unwind " +
	    stackState<A>()));
	return t;
}

static void
sweep_stack(int iters)
{
	std::vector<StackStep> steps;
	for (int i = 0; i < iters; i++) {
		StackStep s;
		unsigned k = rndn(100);
		if (k < 18)
			s.op = 0;
		else if (k < 28)
			s.op = 1;
		else if (k < 38)
			s.op = 2;
		else if (k < 45)
			s.op = 3;
		else if (k < 52)
			s.op = 4;
		else if (k < 60)
			s.op = 5;
		else if (k < 70)
			s.op = 6;
		else if (k < 88)
			s.op = 7;
		else
			s.op = 8;
		unsigned m = rndn(10);
		if (m < 4)
			s.arg = (int)rndn(32);
		else if (m < 7)
			s.arg = (int)rndn(600);
		else if (m < 9)
			s.arg = (int)rndn(2048);
		else
			s.arg = (int)rndn(8192);
		steps.push_back(s);
		if (steps.size() >= 120) {
			cmpTrace(tr_stack_walk<Ref>(steps),
			    tr_stack_walk<Port>(steps), "stack sweep");
			steps.clear();
		}
	}
	if (!steps.empty())
		cmpTrace(tr_stack_walk<Ref>(steps),
		    tr_stack_walk<Port>(steps), "stack sweep");
}

/* ------------------------------------------------------------------ */

static void
run_all(const char *tag)
{
	(void)tag;

	test_output_edge();
	test_alias_edge();
	test_freealias();

	cmpTrace(tr_ckalloc<Ref>(), tr_ckalloc<Port>(), "ckalloc");
	cmpTrace(tr_stalloc<Ref>(), tr_stalloc<Port>(), "stalloc");
	cmpTrace(tr_stnewblock<Ref>(), tr_stnewblock<Port>(), "stnewblock");
	cmpTrace(tr_marks<Ref>(), tr_marks<Port>(), "marks");
	cmpTrace(tr_growstackstr<Ref>(3000), tr_growstackstr<Port>(3000),
	    "growstackstr");
	cmpTrace(tr_makestrspace<Ref>(), tr_makestrspace<Port>(),
	    "makestrspace");
	cmpTrace(tr_grow_exact<Ref>(), tr_grow_exact<Port>(), "grow exact");
	cmpTrace(tr_stputbin<Ref>(), tr_stputbin<Port>(), "stputbin");
	test_oom();

	sweep_output(16000);
	sweep_alias(16000);
	sweep_stack(10000);
}

int
main(void)
{
	if (sizeof(r_stackmark) != ref_sizeof_stackmark() ||
	    sizeof(r_alias) != ref_sizeof_alias() ||
	    sizeof(Port::Mark) != Port::sizeof_stackmark() ||
	    sizeof(Port::Alias) != Port::sizeof_alias() ||
	    sizeof(r_alias) != sizeof(Port::Alias) ||
	    sizeof(r_output) != sizeof(Port::Out)) {
		printf("b0219: layout mismatch between oracle and port\n");
		return 1;
	}

	test_badalloc();

	setlocale(LC_ALL, "C");
	run_all("C");

	const char *utf8 = NULL;
	if (setlocale(LC_ALL, "C.UTF-8") != NULL)
		utf8 = "C.UTF-8";
	else if (setlocale(LC_ALL, "en_US.UTF-8") != NULL)
		utf8 = "en_US.UTF-8";
	if (utf8 != NULL)
		run_all(utf8);

	long long totc = 0, totf = 0;
	printf("\n%-24s %10s %10s\n", "function", "cases", "failures");
	printf("------------------------------------------------\n");
	for (size_t i = 0; i < recs.size(); i++) {
		printf("%-24s %10lld %10lld\n", recs[i].name.c_str(),
		    recs[i].cases, recs[i].fails);
		totc += recs[i].cases;
		totf += recs[i].fails;
	}
	printf("------------------------------------------------\n");
	printf("%-24s %10lld %10lld\n", "TOTAL", totc, totf);
	printf("locales tested: C%s%s\n", utf8 ? ", " : "", utf8 ? utf8 : "");

	if (totf != 0) {
		printf("\nfirst failure per function:\n");
		for (size_t i = 0; i < recs.size(); i++) {
			if (recs[i].fails != 0)
				printf("  %s: %s\n", recs[i].name.c_str(),
				    recs[i].first.c_str());
		}
		return 1;
	}
	printf("\nALL %lld CASES MATCHED\n", totc);
	return 0;
}
