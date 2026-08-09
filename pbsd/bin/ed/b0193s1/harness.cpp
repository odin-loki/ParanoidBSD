/* harness.cpp: differential test for PBSD batch b0193s1 (bin/ed/sub.c).
 *
 * Every ported function is driven with hand-written edge cases and a fixed-seed
 * randomised sweep (>= 200000 iterations per function).  The C++23 port and the
 * ref_ oracle are compared on return values, every global they touch, pointer
 * results as offsets from buffer bases, and the ENTIRE contents of every buffer
 * they can write into (guard-filled with 0x7f beforehand).
 */

#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <regex.h>
#include <string>
#include <unistd.h>
#include <vector>

import pbsd.bin.ed.b0193s1;

namespace P = pbsd::bin_ed::b0193s1;

/* ------------------------------------------------------------------ */
/* Oracle interface                                                     */
/* ------------------------------------------------------------------ */

extern "C" {
int ref_extract_subst_tail(int *flagp, long *np);
char *ref_extract_subst_template(void);
int ref_substitute_matching_text(regex_t *pat, P::line_t *lp, int gflag, int kth);
int ref_apply_subst_template(const char *boln, regmatch_t *rm, int off, int re_nsub);

void ref_env_reset(void);
void ref_env_set_ibuf(const char *s, int n);
void ref_env_set_isglobal(int v);
void ref_env_set_isbinary(int v);
void ref_env_set_rhbuf(const char *s, int n, int cap);
void ref_env_set_rbuf(int cap);
void ref_env_set_line(const char *t, int n, int fail);
void ref_env_script_add(const char *t, int neg);

int ref_env_get_rhbufi(void);
int ref_env_get_rhbufsz(void);
const char *ref_env_get_rhbuf(void);
int ref_env_get_rbufsz(void);
const char *ref_env_get_rbuf(void);
const char *ref_env_get_ibuf_base(void);
int ref_env_get_ibufsz(void);
long ref_env_get_ibufp_off(void);
const char *ref_env_get_errmsg(void);
int ref_env_get_mutex(void);
int ref_env_get_isbinary(void);
int ref_env_get_script_pos(void);
const char *ref_env_get_sfbuf(void);
}

#define ERR (-2)
#define GPR 002
#define GSG 020
#define GLB 001
#define IBUF_TOTAL 264
#define SFBUF_SZ 640
#define GUARD_BYTE 0x7f
#define SE_MAX 30

static const char SENT[] = "<no-errmsg-set>";

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                          */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	long cases;
	long fails;
	long shown;
};

static Stat st_tail = { "extract_subst_tail", 0, 0, 0 };
static Stat st_tmpl = { "extract_subst_template", 0, 0, 0 };
static Stat st_sub = { "substitute_matching_text", 0, 0, 0 };
static Stat st_apply = { "apply_subst_template", 0, 0, 0 };

static void
fail(Stat &s, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static void
fail(Stat &s, const char *fmt, ...)
{
	va_list ap;

	s.fails++;
	if (s.shown < 12) {
		s.shown++;
		fprintf(stderr, "  [%s] MISMATCH: ", s.name);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fputc('\n', stderr);
	}
}

/* ------------------------------------------------------------------ */
/* Fixed-seed PRNG (xorshift64*)                                        */
/* ------------------------------------------------------------------ */

static uint64_t rng_state = 0x9e3779b97f4a7c15ULL;

static void
rng_seed(uint64_t s)
{
	rng_state = s ? s : 0x123456789abcdefULL;
}

static inline uint64_t
rnd(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static inline long
rnd_range(long lo, long hi)
{
	if (hi <= lo)
		return lo;
	return lo + (long)(rnd() % (uint64_t)(hi - lo + 1));
}

static inline int
rnd_byte(void)
{
	return (int)(rnd() & 0xffu);
}

/* ------------------------------------------------------------------ */
/* Environment helpers                                                  */
/* ------------------------------------------------------------------ */

static void
reset_both(void)
{
	ref_env_reset();
	P::env_reset();
}

static void
set_isglobal_both(int v)
{
	ref_env_set_isglobal(v);
	P::env_set_isglobal(v);
}

static void
set_isbinary_both(int v)
{
	ref_env_set_isbinary(v);
	P::env_set_isbinary(v);
}

static void
set_ibuf_both(const char *s, int n)
{
	ref_env_set_ibuf(s, n);
	P::env_set_ibuf(s, n);
}

static void
script_both(const char *t, int neg)
{
	ref_env_script_add(t, neg);
	P::env_script_add(t, neg);
}

static int
strcmp_null(const char *a, const char *b)
{
	if (a == nullptr && b == nullptr)
		return 0;
	if (a == nullptr || b == nullptr)
		return 1;
	return strcmp(a, b);
}

static long
ptr_off_rhbuf(const char *p, const char *base)
{
	if (p == nullptr)
		return -1;
	if (base == nullptr)
		return -2;
	return (long)(p - base);
}

static bool
buf_eq(const char *a, const char *b, size_t n)
{
	if (a == nullptr && b == nullptr)
		return true;
	if (a == nullptr || b == nullptr)
		return false;
	return memcmp(a, b, n) == 0;
}

static bool
env_bufs_equal(void)
{
	const char *ib1 = ref_env_get_ibuf_base();
	const char *ib2 = P::env_get_ibuf_base();
	const char *rh1 = ref_env_get_rhbuf();
	const char *rh2 = P::env_get_rhbuf();
	const char *rb1 = ref_env_get_rbuf();
	const char *rb2 = P::env_get_rbuf();
	int rhz1 = ref_env_get_rhbufsz();
	int rhz2 = P::env_get_rhbufsz();
	int rbz1 = ref_env_get_rbufsz();
	int rbz2 = P::env_get_rbufsz();

	if (!buf_eq(ib1, ib2, IBUF_TOTAL))
		return false;
	if (rhz1 != rhz2)
		return false;
	if (rhz1 > 0 && !buf_eq(rh1, rh2, (size_t)rhz1))
		return false;
	if (rbz1 != rbz2)
		return false;
	if (rbz1 > 0 && !buf_eq(rb1, rb2, (size_t)rbz1))
		return false;
	if (!buf_eq(ref_env_get_sfbuf(), P::env_get_sfbuf(), SFBUF_SZ))
		return false;
	return true;
}

static bool
env_meta_equal(void)
{
	if (ref_env_get_mutex() != P::env_get_mutex())
		return false;
	if (ref_env_get_rhbufi() != P::env_get_rhbufi())
		return false;
	if (ref_env_get_ibufp_off() != P::env_get_ibufp_off())
		return false;
	if (ref_env_get_isbinary() != P::env_get_isbinary())
		return false;
	if (ref_env_get_script_pos() != P::env_get_script_pos())
		return false;
	if (strcmp_null(ref_env_get_errmsg(), P::env_get_errmsg()) != 0)
		return false;
	return true;
}

static bool
env_equal(void)
{
	return env_bufs_equal() && env_meta_equal();
}

struct EnvSnap {
	int mutex;
	int rhbufi;
	long ibufp_off;
	int isbinary;
	int script_pos;
	const char *errmsg;
	unsigned char ibuf[IBUF_TOTAL];
	unsigned char rhbuf[2048];
	unsigned char rbuf[2048];
	unsigned char sfbuf[SFBUF_SZ];
	int rhbufsz;
	int rbufsz;
	int rhbuf_len;	/* bytes to compare in rhbuf/rbuf */
};

static void
snap_ref(EnvSnap &s)
{
	const char *rh = ref_env_get_rhbuf();
	const char *rb = ref_env_get_rbuf();
	int rhz = ref_env_get_rhbufsz();
	int rbz = ref_env_get_rbufsz();

	s.mutex = ref_env_get_mutex();
	s.rhbufi = ref_env_get_rhbufi();
	s.ibufp_off = ref_env_get_ibufp_off();
	s.isbinary = ref_env_get_isbinary();
	s.script_pos = ref_env_get_script_pos();
	s.errmsg = ref_env_get_errmsg();
	memcpy(s.ibuf, ref_env_get_ibuf_base(), IBUF_TOTAL);
	s.rhbufsz = rhz;
	s.rbufsz = rbz;
	s.rhbuf_len = rhz > 0 ? rhz : 0;
	memset(s.rhbuf, GUARD_BYTE, sizeof(s.rhbuf));
	memset(s.rbuf, GUARD_BYTE, sizeof(s.rbuf));
	memset(s.sfbuf, GUARD_BYTE, sizeof(s.sfbuf));
	if (rhz > 0 && rh != nullptr)
		memcpy(s.rhbuf, rh, (size_t)rhz);
	if (rbz > 0 && rb != nullptr)
		memcpy(s.rbuf, rb, (size_t)rbz);
	memcpy(s.sfbuf, ref_env_get_sfbuf(), SFBUF_SZ);
}

static void
snap_port(EnvSnap &s)
{
	const char *rh = P::env_get_rhbuf();
	const char *rb = P::env_get_rbuf();
	int rhz = P::env_get_rhbufsz();
	int rbz = P::env_get_rbufsz();

	s.mutex = P::env_get_mutex();
	s.rhbufi = P::env_get_rhbufi();
	s.ibufp_off = P::env_get_ibufp_off();
	s.isbinary = P::env_get_isbinary();
	s.script_pos = P::env_get_script_pos();
	s.errmsg = P::env_get_errmsg();
	memcpy(s.ibuf, P::env_get_ibuf_base(), IBUF_TOTAL);
	s.rhbufsz = rhz;
	s.rbufsz = rbz;
	s.rhbuf_len = rhz > 0 ? rhz : 0;
	memset(s.rhbuf, GUARD_BYTE, sizeof(s.rhbuf));
	memset(s.rbuf, GUARD_BYTE, sizeof(s.rbuf));
	memset(s.sfbuf, GUARD_BYTE, sizeof(s.sfbuf));
	if (rhz > 0 && rh != nullptr)
		memcpy(s.rhbuf, rh, (size_t)rhz);
	if (rbz > 0 && rb != nullptr)
		memcpy(s.rbuf, rb, (size_t)rbz);
	memcpy(s.sfbuf, P::env_get_sfbuf(), SFBUF_SZ);
}

static bool
snap_equal(const EnvSnap &a, const EnvSnap &b)
{
	if (a.mutex != b.mutex || a.rhbufi != b.rhbufi ||
	    a.ibufp_off != b.ibufp_off || a.isbinary != b.isbinary ||
	    a.script_pos != b.script_pos || a.rhbufsz != b.rhbufsz ||
	    a.rbufsz != b.rbufsz)
		return false;
	if (strcmp_null(a.errmsg, b.errmsg) != 0)
		return false;
	if (memcmp(a.ibuf, b.ibuf, IBUF_TOTAL) != 0)
		return false;
	if (a.rhbufsz > 0 && memcmp(a.rhbuf, b.rhbuf, (size_t)a.rhbufsz) != 0)
		return false;
	if (a.rbufsz > 0 && memcmp(a.rbuf, b.rbuf, (size_t)a.rbufsz) != 0)
		return false;
	if (memcmp(a.sfbuf, b.sfbuf, SFBUF_SZ) != 0)
		return false;
	return true;
}

static const char *
snap_why(const EnvSnap &a, const EnvSnap &b)
{
	if (a.mutex != b.mutex)
		return "mutex";
	if (a.rhbufi != b.rhbufi)
		return "rhbufi";
	if (a.ibufp_off != b.ibufp_off)
		return "ibufp_off";
	if (a.isbinary != b.isbinary)
		return "isbinary";
	if (a.script_pos != b.script_pos)
		return "script_pos";
	if (a.rhbufsz != b.rhbufsz)
		return "rhbufsz";
	if (a.rbufsz != b.rbufsz)
		return "rbufsz";
	if (strcmp_null(a.errmsg, b.errmsg) != 0)
		return "errmsg";
	if (memcmp(a.ibuf, b.ibuf, IBUF_TOTAL) != 0)
		return "ibuf";
	if (a.rhbufsz > 0 && memcmp(a.rhbuf, b.rhbuf, (size_t)a.rhbufsz) != 0)
		return "rhbuf";
	if (a.rbufsz > 0 && memcmp(a.rbuf, b.rbuf, (size_t)a.rbufsz) != 0)
		return "rbuf";
	if (memcmp(a.sfbuf, b.sfbuf, SFBUF_SZ) != 0)
		return "sfbuf";
	return nullptr;
}

static void
setup_tail(const char *cmd, int n, int isg)
{
	reset_both();
	set_isglobal_both(isg);
	set_ibuf_both(cmd, n);
}

static void
setup_tmpl(const char *cmd, int cn, int isg, const char *scripts[],
    const int script_neg[], int nscripts)
{
	reset_both();
	set_isglobal_both(isg);
	for (int i = 0; i < nscripts; i++)
		script_both(scripts[i], script_neg[i]);
	set_ibuf_both(cmd, cn);
}

static void
setup_sub(const char *line, int llen, const char *tmpl, int tlen, int isbin,
    int line_fail)
{
	reset_both();
	set_isbinary_both(isbin);
	ref_env_set_line(line, llen, line_fail);
	P::env_set_line(line, llen, line_fail);
	if (!line_fail) {
		set_ibuf_both(tmpl, tlen);
		ref_extract_subst_template();
		P::extract_subst_template();
	}
}

static void
setup_apply(const char *tmpl, int tn)
{
	reset_both();
	ref_env_set_rbuf(0);
	P::env_set_rbuf(0);
	set_ibuf_both(tmpl, tn);
	ref_extract_subst_template();
	P::extract_subst_template();
}

static std::string
rand_body(int maxlen, char delim)
{
	std::string s;
	int n = (int)rnd_range(0, maxlen);

	for (int i = 0; i < n; i++) {
		int b = rnd_byte();
		if (b == '\n' || b == (int)delim)
			b = 'q';
		s.push_back((char)b);
	}
	return s;
}

static std::string
rand_line(int maxlen, bool with_nl)
{
	std::string s = rand_body(maxlen, '\0');
	if (with_nl)
		s.push_back('\n');
	return s;
}

/* ------------------------------------------------------------------ */
/* extract_subst_tail                                                   */
/* ------------------------------------------------------------------ */

static void
tail_case(const char *cmd, int n, int isg)
{
	int rf = 0, pf = 0;
	long rn = 0, pn = 0;
	int rr, pr;
	EnvSnap sr, sp;

	st_tail.cases++;

	setup_tail(cmd, n, isg);
	rr = ref_extract_subst_tail(&rf, &rn);
	snap_ref(sr);

	setup_tail(cmd, n, isg);
	pr = P::extract_subst_tail(&pf, &pn);
	snap_port(sp);

	if (rr != pr || rf != pf || rn != pn || !snap_equal(sr, sp))
		fail(st_tail,
		    "cmd=%.*s isg=%d why=%s -> ref(r=%d f=%d n=%ld) port(r=%d f=%d n=%ld)",
		    n, cmd, isg, snap_why(sr, sp) ? snap_why(sr, sp) : "?",
		    rr, rf, rn, pr, pf, pn);
}

static void
test_extract_subst_tail(void)
{
	static const char *cmds[] = {
		"\n",
		"/\n",
		"/a/\n",
		"/a/g\n",
		"/a/1\n",
		"/a/9\n",
		"/x/\n",
		"/%/\n",
		"/\x80\xff/\n",
		"/\x80\xff/g\n",
		"/\x80\xff/5\n",
		"/boundaryboundaryboundary/\n",
		"/boundaryboundaryboundary/g\n",
		"/a/999999999999999999999/\n",
		"#foo#\n",
		"|bar|2\n",
		"@z@\n",
		"^qq^\n",
		"/a/b/\n",
		"/a/b/g\n",
		"/\\\n",
		"/\\g\n",
		"/\\1\n",
		"/\\9\n",
		"/a/z\n",
		"/a/0\n",
		"/a/\x80\n",
	};
	const int nc = (int)(sizeof(cmds) / sizeof(cmds[0]));

	for (int i = 0; i < nc; i++)
		for (int g = 0; g <= 1; g++)
			tail_case(cmds[i], (int)strlen(cmds[i]), g);

	reset_both();
	set_ibuf_both("/prev/\n", 7);
	ref_extract_subst_template();
	P::extract_subst_template();
	tail_case("/%/g\n", 5, 0);

	rng_seed(0xB0193A01ULL);
	static const char dels[] = "/#|@^";
	for (long it = 0; it < 200000; it++) {
		char d = dels[rnd() % 5];
		std::string rhs = rand_body(28, d);
		std::string cmd;
		cmd.push_back(d);
		cmd += rhs;
		cmd.push_back(d);
		if ((rnd() & 3) == 0)
			cmd.push_back('g');
		else if ((rnd() & 3) == 1)
			cmd.push_back((char)('1' + (rnd() % 9)));
		else if ((rnd() & 7) == 0)
			cmd += "999999999999999999999";
		cmd.push_back('\n');
		tail_case(cmd.c_str(), (int)cmd.size(), (int)(rnd() & 1));
	}
}

/* ------------------------------------------------------------------ */
/* extract_subst_template                                               */
/* ------------------------------------------------------------------ */

static void
tmpl_case(const char *cmd, int cn, int isg, const char *scripts[],
    const int script_neg[], int nscripts)
{
	char *rr, *pr;
	long roff, poff;
	const char *rh1, *rh2;
	EnvSnap sr, sp;

	st_tmpl.cases++;

	setup_tmpl(cmd, cn, isg, scripts, script_neg, nscripts);
	rr = ref_extract_subst_template();
	roff = ptr_off_rhbuf(rr, ref_env_get_rhbuf());
	rh1 = ref_env_get_rhbuf();
	snap_ref(sr);

	setup_tmpl(cmd, cn, isg, scripts, script_neg, nscripts);
	pr = P::extract_subst_template();
	poff = ptr_off_rhbuf(pr, P::env_get_rhbuf());
	rh2 = P::env_get_rhbuf();
	snap_port(sp);

	if ((rr == nullptr) != (pr == nullptr) || roff != poff ||
	    !snap_equal(sr, sp)) {
		fail(st_tmpl,
		    "cmd=%.*s isg=%d -> ref(p=%ld) port(p=%ld) null=%d/%d",
		    cn, cmd, isg, roff, poff, rr == nullptr, pr == nullptr);
		return;
	}
	if (rr != nullptr && rh1 != nullptr && rh2 != nullptr &&
	    memcmp(rr, pr, (size_t)ref_env_get_rhbufi()) != 0)
		fail(st_tmpl, "cmd=%.*s template bytes differ", cn, cmd);
}

static void
test_extract_subst_template(void)
{
	static const char *none = nullptr;
	static const int none_neg = 0;
	static const char *sc_cd[] = { "cd/\n" };
	static const int sc_cd_neg[] = { 0 };
	static const char *sc_empty[] = { "" };
	static const int sc_empty_neg[] = { 0 };
	static const char *sc_x[] = { "x" };
	static const int sc_x_neg[] = { -1 };
	static const char *sc_z[] = { "z/\n" };
	static const int sc_z_neg[] = { 0 };

	tmpl_case("/\n", 2, 0, &none, &none_neg, 0);
	tmpl_case("/x/\n", 4, 0, &none, &none_neg, 0);
	tmpl_case("/\x80\xff/\n", 6, 0, &none, &none_neg, 0);
	tmpl_case("/ab\\\n", 5, 0, sc_cd, sc_cd_neg, 1);
	tmpl_case("/ab\\\n", 5, 1, &none, &none_neg, 0);
	tmpl_case("/\\\n", 3, 0, &none, &none_neg, 0);
	tmpl_case("/\\\n", 3, 0, sc_empty, sc_empty_neg, 1);
	tmpl_case("/\\\n", 3, 0, sc_x, sc_x_neg, 1);
	tmpl_case("/%/\n", 4, 0, &none, &none_neg, 0);
	tmpl_case("/%x/\n", 5, 0, &none, &none_neg, 0);
	tmpl_case("/boundaryboundaryboundaryboundary/\n", 35, 1,
	    &none, &none_neg, 0);
	tmpl_case("/\\&\\1\\9\\\n", 10, 0, sc_z, sc_z_neg, 1);
	tmpl_case("/\x00/\n", 4, 0, &none, &none_neg, 0);
	tmpl_case("/\xff/\n", 4, 0, &none, &none_neg, 0);

	reset_both();
	set_ibuf_both("/prev/\n", 7);
	ref_extract_subst_template();
	P::extract_subst_template();
	tmpl_case("/%/\n", 4, 0, &none, &none_neg, 0);

	reset_both();
	set_ibuf_both("/first/\n", 8);
	ref_extract_subst_template();
	P::extract_subst_template();
	tmpl_case("/%/\n", 4, 0, &none, &none_neg, 0);

	rng_seed(0xB0193A02ULL);
	static const char dels[] = "/#|@^";
	for (long it = 0; it < 200000; it++) {
		char d = dels[rnd() % 5];
		std::string body = rand_body(32, d);
		std::string cmd;
		cmd.push_back(d);
		cmd += body;
		cmd.push_back(d);
		cmd.push_back('\n');

		const char *scripts[4];
		int script_neg[4];
		int ns = 0;
		if ((rnd() & 15) == 0) {
			scripts[ns] = "cont/\n";
			script_neg[ns] = 0;
			ns++;
			if (body.find('\\') != std::string::npos &&
			    (rnd() & 1)) {
				scripts[ns] = "";
				script_neg[ns] = (int)(rnd() & 1) ? -1 : 0;
				ns++;
			}
		}
		tmpl_case(cmd.c_str(), (int)cmd.size(), (int)(rnd() & 1),
		    scripts, script_neg, ns);
	}
}

/* ------------------------------------------------------------------ */
/* substitute_matching_text                                             */
/* ------------------------------------------------------------------ */

static regex_t *
mk_re(const char *pat)
{
	regex_t *re = (regex_t *)malloc(sizeof(regex_t));
	if (re == nullptr)
		return nullptr;
	if (regcomp(re, pat, 0) != 0) {
		free(re);
		return nullptr;
	}
	return re;
}

static void
free_re(regex_t *re)
{
	if (re != nullptr) {
		regfree(re);
		free(re);
	}
}

static void
sub_case(const char *line, int llen, const char *tmpl, int tlen, int gflag,
    int kth, int isbin, int line_fail)
{
	P::line_t rlp, plp;
	regex_t *rre, *pre;
	int rr, pr;
	const char *pat = "a";
	EnvSnap sr, sp;

	st_sub.cases++;

	memset(&rlp, 0, sizeof(rlp));
	memset(&plp, 0, sizeof(plp));
	rlp.len = llen;
	plp.len = llen;

	setup_sub(line, llen, tmpl, tlen, isbin, line_fail);
	rre = mk_re(pat);
	if (rre == nullptr) {
		fail(st_sub, "regcomp failed");
		return;
	}
	rr = ref_substitute_matching_text(rre, &rlp, gflag, kth);
	snap_ref(sr);
	free_re(rre);

	setup_sub(line, llen, tmpl, tlen, isbin, line_fail);
	pre = mk_re(pat);
	if (pre == nullptr) {
		fail(st_sub, "regcomp failed");
		return;
	}
	pr = P::substitute_matching_text(pre, &plp, gflag, kth);
	snap_port(sp);
	free_re(pre);

	if (rr != pr || !snap_equal(sr, sp))
		fail(st_sub,
		    "line_len=%d g=%d k=%d bin=%d fail=%d why=%s -> ref=%d port=%d",
		    llen, gflag, kth, isbin, line_fail,
		    snap_why(sr, sp) ? snap_why(sr, sp) : "?",
		    rr, pr);
}

static void
test_substitute_matching_text(void)
{
	sub_case("a\n", 2, "/Z/\n", 4, 0, 0, 0, 0);
	sub_case("bbb\n", 4, "/Z/\n", 4, 0, 0, 0, 0);
	sub_case("xyz\n", 4, "/Q/\n", 4, 0, 0, 0, 0);
	sub_case("ba\n", 3, "/&/\n", 4, GSG, 0, 0, 0);
	sub_case("aaaa\n", 5, "/&X/\n", 5, GSG, 0, 0, 0);
	sub_case("onea\n", 6, "/R/\n", 4, 0, 2, 0, 0);
	sub_case("a\n", 2, "/Z/\n", 4, 0, 0, 0, 1);
	{
		const char ln[] = { 'a', '\0', 'a', '\n' };
		sub_case(ln, 4, "/Q/\n", 4, GSG, 0, 0, 0);
	}
	{
		const char ln[] = { '\x80', 'a', '\xff', '\n' };
		sub_case(ln, 4, "/R/\n", 4, GSG, 2, 1, 0);
	}
	sub_case("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n", 34, "/&/\n", 4, GSG, 5, 0, 0);
	sub_case("no match\n", 9, "/Z/\n", 4, 0, 0, 0, 0);
	sub_case("aaa\n", 4, "/Z/\n", 4, GSG, 0, 0, 0);
	sub_case("aaa\n", 4, "//\n", 3, GSG, 0, 0, 0);

	rng_seed(0xB0193A03ULL);
	static const char *tmpls[] = { "/X/\n", "/&/\n", "/&Q/\n", "/\\1/\n" };
	const int nt = (int)(sizeof(tmpls) / sizeof(tmpls[0]));
	for (long it = 0; it < 200000; it++) {
		std::string line = rand_line((int)rnd_range(1, 40), true);
		const char *tm = tmpls[rnd() % nt];
		int gflag = 0;
		if (rnd() & 1)
			gflag |= GSG;
		if (rnd() & 1)
			gflag |= GLB;
		sub_case(line.c_str(), (int)line.size(), tm, (int)strlen(tm),
		    gflag, (int)rnd_range(0, 5), (int)(rnd() & 1),
		    (rnd() & 127) == 0 ? 1 : 0);
	}
}

/* ------------------------------------------------------------------ */
/* apply_subst_template                                                 */
/* ------------------------------------------------------------------ */

static void
apply_case(const char *tmpl, int tn, const char *boln, int so, int eo,
    int off, int nsub)
{
	regmatch_t rrm[SE_MAX], prm[SE_MAX];
	int rr, pr;
	EnvSnap sr, sp;

	st_apply.cases++;

	setup_apply(tmpl, tn);

	memset(rrm, 0, sizeof(rrm));
	rrm[0].rm_so = so;
	rrm[0].rm_eo = eo;
	if (nsub >= 1) {
		int e1 = eo > 0 ? eo - 1 : 0;
		rrm[1].rm_so = 0;
		rrm[1].rm_eo = e1;
	}
	if (nsub >= 2) {
		rrm[2].rm_so = so;
		rrm[2].rm_eo = eo;
	}
	rr = ref_apply_subst_template(boln, rrm, off, nsub);
	snap_ref(sr);

	setup_apply(tmpl, tn);

	memset(prm, 0, sizeof(prm));
	prm[0].rm_so = so;
	prm[0].rm_eo = eo;
	if (nsub >= 1) {
		int e1 = eo > 0 ? eo - 1 : 0;
		prm[1].rm_so = 0;
		prm[1].rm_eo = e1;
	}
	if (nsub >= 2) {
		prm[2].rm_so = so;
		prm[2].rm_eo = eo;
	}
	pr = P::apply_subst_template(boln, prm, off, nsub);
	snap_port(sp);

	if (rr != pr || !snap_equal(sr, sp)) {
		size_t di = 0;

		if (snap_why(sr, sp) != nullptr && strcmp(snap_why(sr, sp), "rhbuf") == 0) {
			for (di = 0; di < (size_t)sr.rhbufsz; di++)
				if (sr.rhbuf[di] != sp.rhbuf[di]) {
					fprintf(stderr,
					    "  rhbuf[%zu]=ref:%02x port:%02x "
					    "rhbufi ref:%d port:%d\n",
					    di, sr.rhbuf[di], sp.rhbuf[di],
					    sr.rhbufi, sp.rhbufi);
					break;
				}
		}
		fail(st_apply,
		    "tmpl=%.*s so=%d eo=%d off=%d nsub=%d why=%s -> ref=%d port=%d",
		    tn, tmpl, so, eo, off, nsub,
		    snap_why(sr, sp) ? snap_why(sr, sp) : "?",
		    rr, pr);
	}
}

static void
test_apply_subst_template(void)
{
	apply_case("/&/\n", 4, "abc", 0, 2, 0, 0);
	apply_case("/\\1/\n", 5, "abc", 0, 3, 1, 1);
	apply_case("/x&y/\n", 6, "a", 0, 1, 0, 0);
	apply_case("/\x80&\xff/\n", 8, "xyz", 1, 3, 2, 0);
	apply_case("/boundaryboundary/\n", 19,
	    "aaaaaaaaaaaaaaaaaaaaaaaaaaaa", 3, 12, 4, 0);
	apply_case("/\\9/\n", 5, "abc", 0, 3, 0, 9);
	apply_case("/\\0/\n", 5, "abc", 0, 3, 0, 1);
	apply_case("/\\\\/\n", 5, "abc", 0, 3, 0, 0);
	apply_case("/&/\n", 4, "", 0, 0, 0, 0);
	apply_case("/\\2/\n", 5, "abcd", 0, 4, 0, 2);
	apply_case("/\\2/\n", 5, "abcd", 0, 4, 0, 1);
	apply_case("/a/\n", 4, "xyz", 0, 0, 5, 0);

	rng_seed(0xB0193A04ULL);
	for (long it = 0; it < 200000; it++) {
		std::string boln = rand_line((int)rnd_range(0, 28), false);
		int len = (int)boln.size();
		int so = len ? (int)rnd_range(0, len - 1) : 0;
		int eo = len ? (int)rnd_range(so, len) : 0;
		int off = (int)rnd_range(0, 16);
		int nsub = (int)rnd_range(0, 3);
		static const char *tm[] = {
			"/&/\n", "/X/\n", "/X\\1Y/\n", "/\\1\\2/\n",
			"/\\9/\n", "/\\\\/\n"
		};
		const char *t = tm[rnd() % 6];
		apply_case(t, (int)strlen(t), boln.c_str(), so, eo, off, nsub);
	}
}

/* ------------------------------------------------------------------ */

static void
row(const Stat &s)
{
	printf("  %-28s %10ld %10ld  %s\n", s.name, s.cases, s.fails,
	    s.fails ? "FAIL" : "ok");
}

int
main(void)
{
	long total_fail;

	alarm(300);

	test_extract_subst_tail();
	test_extract_subst_template();
	test_substitute_matching_text();
	test_apply_subst_template();

	printf("\nPBSD b0193s1 -- bin/ed/sub.c differential test\n");
	printf("  %-28s %10s %10s  %s\n", "function", "cases", "failures",
	    "status");
	printf("  ----------------------------------------------------------------\n");
	row(st_tail);
	row(st_tmpl);
	row(st_sub);
	row(st_apply);
	printf("  ----------------------------------------------------------------\n");

	total_fail = st_tail.fails + st_tmpl.fails + st_sub.fails +
	    st_apply.fails;
	printf("  %-28s %10ld %10ld  %s\n", "TOTAL",
	    st_tail.cases + st_tmpl.cases + st_sub.cases + st_apply.cases,
	    total_fail, total_fail ? "FAIL" : "ok");

	return total_fail ? 1 : 0;
}
