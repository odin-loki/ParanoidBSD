/*
 * PBSD b0180 differential harness: bin/kenv/kenv.c.
 *
 * Every function of the batch (usage, kenv_main, kdumpenv, kgetenv, ksetenv,
 * kunsetenv) is driven through both the C++23 module port and the ref_ C
 * oracle, and the two sides are compared on:
 *
 *   - the return value, whether the call terminated through exit(3) and with
 *     which status (usage() exits, and main() reaches it on several paths);
 *   - the bytes written to stdout and to stderr, captured at file-descriptor
 *     level so printf(3), warn(3) and getopt(3) diagnostics are all seen;
 *   - the whole log of kenv(2) calls: action, name string, value NULL-ness,
 *     length argument, return value, errno and every byte the call left in
 *     (or was handed in) the caller's buffer;
 *   - the caller-owned buffers, allocated in pairs, pre-filled with the guard
 *     byte 0x7f and compared in their entirety, well past the nominal write
 *     window (main() writes a NUL into an argv string);
 *   - argv itself, compared as offsets from the buffer base and never as raw
 *     addresses, since getopt(3) permutes it;
 *   - the file-scope flags and optind after the call.
 *
 * kenv(2) does not exist on this host, so the harness supplies a scripted,
 * fully deterministic stand-in that both sides link against.
 */

#include <sys/types.h>

#include <cerrno>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>

import pbsd.bin.kenv.b0180;

namespace P = pbsd::bin_kenv::b0180;

#define	KENV_GET	0
#define	KENV_SET	1
#define	KENV_UNSET	2
#define	KENV_DUMP	3
#define	KENV_DUMP_LOADER	4
#define	KENV_DUMP_STATIC	5

static_assert(P::kenv_get == KENV_GET);
static_assert(P::kenv_set == KENV_SET);
static_assert(P::kenv_unset == KENV_UNSET);
static_assert(P::kenv_dump == KENV_DUMP);
static_assert(P::kenv_dump_loader == KENV_DUMP_LOADER);
static_assert(P::kenv_dump_static == KENV_DUMP_STATIC);

extern "C" {
extern int hflag, lflag, Nflag, qflag, sflag, uflag, vflag;
void	ref_usage(void);
int	ref_kenv_main(int argc, char **argv);
int	ref_kdumpenv(int dump_type);
int	ref_kgetenv(const char *env);
int	ref_ksetenv(const char *env, char *val);
int	ref_kunsetenv(const char *env);
}

/* ------------------------------------------------------------------ */
/* exit(3) interception						      */
/* ------------------------------------------------------------------ */

static jmp_buf g_exit_jmp;
static volatile int g_exit_armed;
static volatile int g_exit_status;

extern "C" void
exit(int status) noexcept
{

	if (g_exit_armed) {
		g_exit_status = status;
		g_exit_armed = 0;
		longjmp(g_exit_jmp, 1);
	}
	fflush(NULL);
	_exit(status);
}

/* ------------------------------------------------------------------ */
/* stdout/stderr capture					      */
/* ------------------------------------------------------------------ */

static FILE *rep;		/* the harness's own report stream */

static void
cap_init(void)
{
	FILE *fo, *fe;
	int r, o, e;

	r = dup(1);
	rep = fdopen(r, "w");
	fo = tmpfile();
	fe = tmpfile();
	if (rep == NULL || fo == NULL || fe == NULL) {
		fprintf(stderr, "b0180: capture setup failed\n");
		_exit(2);
	}
	o = fileno(fo);
	e = fileno(fe);
	fflush(stdout);
	fflush(stderr);
	dup2(o, 1);
	dup2(e, 2);
	/* Warm both streams so the first case pays no set-up cost. */
	fprintf(stdout, "warm");
	fflush(stdout);
	fprintf(stderr, "warm");
	fflush(stderr);
}

static std::string
read_range(int fd, off_t off, off_t len)
{
	std::string s;
	ssize_t n;

	if (len <= 0)
		return (s);
	s.resize((size_t)len);
	n = pread(fd, &s[0], (size_t)len, off);
	if (n < 0)
		n = 0;
	s.resize((size_t)n);
	return (s);
}

static void
cap_maybe_rewind(void)
{

	if (lseek(1, 0, SEEK_CUR) > (off_t)(4 << 20)) {
		fflush(stdout);
		if (ftruncate(1, 0) == 0)
			fseek(stdout, 0, SEEK_SET);
	}
	if (lseek(2, 0, SEEK_CUR) > (off_t)(4 << 20)) {
		fflush(stderr);
		if (ftruncate(2, 0) == 0)
			fseek(stderr, 0, SEEK_SET);
	}
}

/* ------------------------------------------------------------------ */
/* scripted kenv(2)						      */
/* ------------------------------------------------------------------ */

enum { MAXCALLS = 8, MAXDATA = 1200, MAXNAME = 160, MAXPAYLOAD = 512 };

struct KenvCall {
	int action;
	int name_null;
	int name_len;
	int value_null;
	int len;
	int ret;
	int err;
	int ndata;
	char name[MAXNAME];
	unsigned char data[MAXDATA];
};

struct KenvLog {
	int ncalls;
	int overflow;
	KenvCall c[MAXCALLS];
};

struct KenvScript {
	int nret;
	int ret[MAXCALLS];
	int err[MAXCALLS];
	int payload_len;
	int getval_len;
	unsigned char payload[MAXPAYLOAD];
	unsigned char getval[MAXDATA];
};

static KenvScript g_script;
static KenvLog g_log_a, g_log_b;
static KenvLog *g_log = &g_log_a;

extern "C" int
kenv(int action, const char *name, char *value, int len)
{
	KenvCall *c;
	size_t n;
	int idx, ret, e, nd;

	idx = g_log->ncalls;
	if (idx >= MAXCALLS) {
		g_log->overflow++;
		errno = EINVAL;
		return (-1);
	}
	g_log->ncalls++;
	c = &g_log->c[idx];
	memset(c->name, 0, sizeof(c->name));
	c->action = action;
	c->len = len;
	c->name_null = (name == NULL);
	c->value_null = (value == NULL);
	if (name != NULL) {
		n = strlen(name);
		c->name_len = (int)n;
		if (n > MAXNAME - 1)
			n = MAXNAME - 1;
		memcpy(c->name, name, n);
	} else
		c->name_len = -1;

	/*
	 * Calls beyond the script fail.  That keeps a mutated port which
	 * loops more often than the original from looping forever.
	 */
	ret = (idx < g_script.nret) ? g_script.ret[idx] : -1;
	e = (idx < g_script.nret) ? g_script.err[idx] : EINVAL;

	nd = 0;
	if (value != NULL && len > 0) {
		if (action == KENV_GET) {
			int k = g_script.getval_len;
			if (k > len - 1)
				k = len - 1;
			if (k < 0)
				k = 0;
			memcpy(value, g_script.getval, (size_t)k);
			value[k] = '\0';
			nd = k + 1;
		} else if (action == KENV_DUMP || action == KENV_DUMP_LOADER ||
		    action == KENV_DUMP_STATIC) {
			int k = g_script.payload_len;
			if (k > len)
				k = len;
			memcpy(value, g_script.payload, (size_t)k);
			nd = k;
		} else {
			/* KENV_SET and friends: record what we were handed. */
			nd = len;
		}
		if (nd > MAXDATA)
			nd = MAXDATA;
		if (nd > 0)
			memcpy(c->data, value, (size_t)nd);
	}
	c->ndata = nd;
	c->ret = ret;
	/*
	 * Only the errno this call itself produces is recorded; the ambient
	 * value is left out because heap growth may perturb it.
	 */
	if (ret < 0) {
		errno = e;
		c->err = e;
	} else
		c->err = 0;
	return (ret);
}

static bool
log_equal(const KenvLog &a, const KenvLog &b)
{
	int i, n;

	if (a.ncalls != b.ncalls || a.overflow != b.overflow)
		return (false);
	n = a.ncalls < MAXCALLS ? a.ncalls : MAXCALLS;
	for (i = 0; i < n; i++) {
		const KenvCall &x = a.c[i], &y = b.c[i];
		if (x.action != y.action || x.name_null != y.name_null ||
		    x.name_len != y.name_len || x.value_null != y.value_null ||
		    x.len != y.len || x.ret != y.ret || x.err != y.err ||
		    x.ndata != y.ndata)
			return (false);
		if (memcmp(x.name, y.name, sizeof(x.name)) != 0)
			return (false);
		if (x.ndata > 0 && memcmp(x.data, y.data, (size_t)x.ndata) != 0)
			return (false);
	}
	return (true);
}

/* ------------------------------------------------------------------ */
/* running one side of a case					      */
/* ------------------------------------------------------------------ */

struct RunResult {
	int ret;
	int exited;
	int status;
	std::string out;
	std::string err;
};

typedef int (*thunk_t)(void *);

static RunResult
run_side(thunk_t fn, void *ctx, KenvLog *lg)
{
	RunResult r;
	off_t o0, e0, o1, e1;
	volatile int vret = 0, vexited = 0;

	lg->ncalls = 0;
	lg->overflow = 0;
	g_log = lg;
	fflush(stdout);
	fflush(stderr);
	o0 = lseek(1, 0, SEEK_CUR);
	e0 = lseek(2, 0, SEEK_CUR);
	errno = 0;
	g_exit_status = -0x7fff;
	g_exit_armed = 1;
	if (setjmp(g_exit_jmp) == 0) {
		vret = fn(ctx);
		g_exit_armed = 0;
	} else
		vexited = 1;
	fflush(stdout);
	fflush(stderr);
	o1 = lseek(1, 0, SEEK_CUR);
	e1 = lseek(2, 0, SEEK_CUR);
	r.ret = vret;
	r.exited = vexited;
	r.status = g_exit_status;
	r.out = read_range(1, o0, o1 - o0);
	r.err = read_range(2, e0, e1 - e0);
	return (r);
}

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

enum { S_USAGE, S_KUNSET, S_KSET, S_KGET, S_KDUMP, S_MAIN, S_NSLOT };

static const char *const slot_name[S_NSLOT] = {
	"usage", "kunsetenv", "ksetenv", "kgetenv", "kdumpenv", "kenv_main"
};

static long stat_cases[S_NSLOT];
static long stat_fails[S_NSLOT];
static int stat_shown[S_NSLOT];

static std::string
esc(const std::string &s)
{
	std::string o;
	char t[8];

	for (size_t i = 0; i < s.size(); i++) {
		unsigned char ch = (unsigned char)s[i];
		if (ch == '\n')
			o += "\\n";
		else if (ch >= 0x20 && ch < 0x7f)
			o += (char)ch;
		else {
			snprintf(t, sizeof(t), "\\x%02x", ch);
			o += t;
		}
	}
	return (o);
}

struct Extra {
	int n;
	long v[24];
	Extra() : n(0) {}
	void add(long x) { if (n < 24) v[n++] = x; }
};

static bool
check(int slot, const char *label, const RunResult &a, const RunResult &b,
    const KenvLog &la, const KenvLog &lb, const unsigned char *ba,
    const unsigned char *bb, int bn, const Extra *ea, const Extra *eb)
{
	const char *why = NULL;

	stat_cases[slot]++;
	if (a.ret != b.ret)
		why = "return value";
	else if (a.exited != b.exited)
		why = "exit() taken";
	else if (a.exited && a.status != b.status)
		why = "exit status";
	else if (a.out != b.out)
		why = "stdout";
	else if (a.err != b.err)
		why = "stderr";
	else if (!log_equal(la, lb))
		why = "kenv(2) call log";
	else if (bn > 0 && memcmp(ba, bb, (size_t)bn) != 0)
		why = "caller buffer";
	else if (ea != NULL && eb != NULL) {
		if (ea->n != eb->n)
			why = "state vector size";
		else
			for (int i = 0; i < ea->n; i++)
				if (ea->v[i] != eb->v[i]) {
					why = "state vector";
					break;
				}
	}
	if (why == NULL)
		return (true);

	stat_fails[slot]++;
	if (stat_shown[slot]++ < 6) {
		fprintf(rep, "FAIL %s [%s]: %s differs\n", slot_name[slot],
		    label, why);
		fprintf(rep, "  ret     %d / %d\n", a.ret, b.ret);
		fprintf(rep, "  exit    %d:%d / %d:%d\n", a.exited, a.status,
		    b.exited, b.status);
		fprintf(rep, "  stdout  \"%s\" / \"%s\"\n", esc(a.out).c_str(),
		    esc(b.out).c_str());
		fprintf(rep, "  stderr  \"%s\" / \"%s\"\n", esc(a.err).c_str(),
		    esc(b.err).c_str());
		fprintf(rep, "  ncalls  %d / %d\n", la.ncalls, lb.ncalls);
		for (int i = 0; i < la.ncalls && i < lb.ncalls && i < MAXCALLS;
		    i++)
			fprintf(rep, "   [%d] act %d/%d len %d/%d ret %d/%d "
			    "nd %d/%d\n", i, la.c[i].action, lb.c[i].action,
			    la.c[i].len, lb.c[i].len, la.c[i].ret, lb.c[i].ret,
			    la.c[i].ndata, lb.c[i].ndata);
		if (ea != NULL && eb != NULL && ea->n == eb->n) {
			fprintf(rep, "  state  ");
			for (int i = 0; i < ea->n; i++)
				fprintf(rep, " %ld/%ld", ea->v[i], eb->v[i]);
			fprintf(rep, "\n");
		}
		if (bn > 0) {
			for (int i = 0; i < bn; i++)
				if (ba[i] != bb[i]) {
					fprintf(rep, "  buffer diff at %d: "
					    "%02x / %02x\n", i, ba[i], bb[i]);
					break;
				}
		}
		fflush(rep);
	}
	return (false);
}

/* ------------------------------------------------------------------ */
/* deterministic randomness					      */
/* ------------------------------------------------------------------ */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{

	rng_state = s;
}

static uint32_t
rnd(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return ((uint32_t)(x >> 32));
}

static int
rndn(int n)
{

	if (n <= 1)
		return (0);
	return ((int)(rnd() % (uint32_t)n));
}

/* ------------------------------------------------------------------ */
/* guarded buffers						      */
/* ------------------------------------------------------------------ */

enum { GBUF = 2048 };

struct GBuf {
	unsigned char b[GBUF];
	void fill(void) { memset(b, 0x7f, sizeof(b)); }
	char *at(int off) { return ((char *)b + off); }
};

static GBuf bufA, bufB;

/*
 * Byte alphabet for generated names and values: NUL excluded, high-bit bytes
 * and the characters kenv(1) cares about well represented.
 */
static const unsigned char alpha[] = {
	'a', 'b', 'c', 'h', 'i', 'n', 't', '.', '=', '_', '-', '0', '1', '9',
	' ', '"', '\\', '\n', '\t', 0x01, 0x1f, 0x80, 0x81, 0xa0, 0xc3,
	0xfe, 0xff
};

static int
gen_str(unsigned char *dst, int maxlen)
{
	int len, i;

	len = rndn(maxlen + 1);
	for (i = 0; i < len; i++)
		dst[i] = alpha[rndn((int)sizeof(alpha))];
	dst[len] = '\0';
	return (len);
}

/* ------------------------------------------------------------------ */
/* flags							      */
/* ------------------------------------------------------------------ */

struct Flags {
	int h, l, N, q, s, u, v;
};

static void
flags_set(const Flags &f)
{

	hflag = f.h; lflag = f.l; Nflag = f.N; qflag = f.q;
	sflag = f.s; uflag = f.u; vflag = f.v;
	P::hflag = f.h; P::lflag = f.l; P::Nflag = f.N; P::qflag = f.q;
	P::sflag = f.s; P::uflag = f.u; P::vflag = f.v;
}

static void
extra_flags_ref(Extra &e)
{

	e.add(hflag); e.add(lflag); e.add(Nflag); e.add(qflag);
	e.add(sflag); e.add(uflag); e.add(vflag);
}

static void
extra_flags_port(Extra &e)
{

	e.add(P::hflag); e.add(P::lflag); e.add(P::Nflag); e.add(P::qflag);
	e.add(P::sflag); e.add(P::uflag); e.add(P::vflag);
}

/*
 * -1 is deliberately in the flag range: it is the initial value for which
 * `flag++' and `flag--' differ in truthiness (0 versus -2).
 */
static int
rnd_flag(void)
{
	int r = rndn(6);

	if (r < 3)
		return (0);
	if (r < 5)
		return (1);
	return (-1);
}

static Flags
rnd_flags(void)
{
	Flags f;

	f.h = rnd_flag(); f.l = rnd_flag(); f.N = rnd_flag(); f.q = rnd_flag();
	f.s = rnd_flag(); f.u = rnd_flag(); f.v = rnd_flag();
	return (f);
}

/* ------------------------------------------------------------------ */
/* kenv(2) script construction					      */
/* ------------------------------------------------------------------ */

/*
 * Replay kdumpenv()'s buffer-sizing loop over a scripted return sequence and
 * report the buflen of the iteration that breaks out, or -1 if the loop never
 * gets there.
 */
static int
predict_final_buflen(const int *ret, int nret)
{
	int envlen, buflen, k;

	if (nret < 1)
		return (-1);
	envlen = ret[0];
	if (envlen < 0)
		return (-1);
	for (k = 1;; k++) {
		buflen = envlen * 120 / 100;
		if (k >= nret)
			return (-1);
		envlen = ret[k];
		if (envlen < 0)
			return (-1);
		if (envlen > buflen)
			continue;
		return (buflen);
	}
}

/*
 * kdumpenv() walks the dump with `bp += strlen(bp) + 1', which reads one byte
 * past the allocation when the dump fills the buffer exactly with no
 * terminator.  That is a real defect of the original, but the byte read is
 * indeterminate, so a NUL is planted at the last byte the dump will occupy.
 * Nothing else about the case is altered, and the mutated port still sees the
 * identical input.
 */
static void
script_make_deterministic(void)
{
	int b;

	b = predict_final_buflen(g_script.ret, g_script.nret);
	if (b >= 1 && b <= g_script.payload_len)
		g_script.payload[b - 1] = '\0';
}

static const char *const entpool[] = {
	"hint.acpi.0.disabled=1", "hint.a=1", "hint.=v", "hint.=", "hint4=1",
	"hint=1", "hint", "hint.", "hin.t=1", "hintt.a=1", "abc=def", "noeq",
	"=v", "=", "a=", "a=b=c", "x=\x80\xff", "\x80=y", "z", "",
	"kern.ident=GENERIC", "hint.uart.0.at=isa"
};

static void
gen_payload(void)
{
	int n, i, off, l;

	off = 0;
	if (rndn(8) == 0) {
		/* Raw bytes, embedded NULs and all. */
		n = rndn(48);
		for (i = 0; i < n; i++)
			g_script.payload[i] = (rndn(6) == 0) ? 0 :
			    alpha[rndn((int)sizeof(alpha))];
		off = n;
	} else {
		n = rndn(6);
		for (i = 0; i < n; i++) {
			const char *e = entpool[rndn((int)(sizeof(entpool) /
			    sizeof(entpool[0])))];
			l = (int)strlen(e);
			if (off + l + 1 > MAXPAYLOAD - 8)
				break;
			memcpy(g_script.payload + off, e, (size_t)l + 1);
			off += l + 1;
		}
		if (rndn(4) == 0 && off < MAXPAYLOAD - 8) {
			/* An unterminated tail entry. */
			l = rndn(6);
			for (i = 0; i < l; i++)
				g_script.payload[off + i] =
				    alpha[rndn((int)sizeof(alpha))];
			off += l;
		}
	}
	g_script.payload_len = off;
}

static int
gen_report(void)
{
	int r = rndn(10);

	if (r == 0)
		return (0);
	if (r == 1)
		return (1);
	return (rndn(90));
}

/*
 * Build a scripted kenv(2) return sequence.  The dump loop is steered
 * explicitly through one to three iterations and the breaking iteration is
 * chosen so that both sides of `envlen > buflen' are exercised, equality
 * included.
 */
static void
gen_script(void)
{
	int iters, i, prev, buflen;

	gen_payload();
	g_script.getval_len = gen_str(g_script.getval, rndn(20) == 0 ? 1199 : 40);

	if (rndn(9) == 0) {
		g_script.ret[0] = -(1 + rndn(3));
		g_script.err[0] = (rndn(2) == 0) ? ENOENT : EPERM;
		g_script.nret = 1;
		script_make_deterministic();
		return;
	}
	g_script.ret[0] = gen_report();
	g_script.err[0] = 0;
	prev = g_script.ret[0];
	iters = 1 + rndn(3);
	for (i = 1; i <= iters; i++) {
		buflen = prev * 120 / 100;
		g_script.err[i] = 0;
		if (i < iters) {
			g_script.ret[i] = buflen + 1 + rndn(15);
			prev = g_script.ret[i];
			continue;
		}
		if (rndn(10) == 0) {
			g_script.ret[i] = -(1 + rndn(3));
			g_script.err[i] = (rndn(2) == 0) ? ENOENT : EPERM;
			break;
		}
		switch (rndn(4)) {
		case 0:
			g_script.ret[i] = buflen;	/* boundary: breaks */
			break;
		case 1:
			g_script.ret[i] = 0;
			break;
		default:
			g_script.ret[i] = rndn(buflen + 1);
			break;
		}
	}
	g_script.nret = iters + 1;
	script_make_deterministic();
}

static void
script_simple(int r0, int e0)
{

	memset(&g_script, 0, sizeof(g_script));
	g_script.nret = 1;
	g_script.ret[0] = r0;
	g_script.err[0] = e0;
}

static void
set_payload(const char *const *ents, int n)
{
	int off = 0, i, l;

	for (i = 0; i < n; i++) {
		l = (int)strlen(ents[i]);
		memcpy(g_script.payload + off, ents[i], (size_t)l + 1);
		off += l + 1;
	}
	g_script.payload_len = off;
}

/* ------------------------------------------------------------------ */
/* usage()							      */
/* ------------------------------------------------------------------ */

static int thunk_ref_usage(void *) { ref_usage(); return (0); }
static int thunk_port_usage(void *) { P::usage(); return (0); }

static void
test_usage(void)
{
	long i;

	for (i = 0; i < 200000; i++) {
		Flags f = (i < 4) ? Flags{(int)i, 0, 0, 0, 0, 0, 0} :
		    rnd_flags();

		flags_set(f);
		RunResult a = run_side(thunk_ref_usage, NULL, &g_log_a);
		Extra ea;
		extra_flags_ref(ea);

		flags_set(f);
		RunResult b = run_side(thunk_port_usage, NULL, &g_log_b);
		Extra eb;
		extra_flags_port(eb);

		check(S_USAGE, "usage", a, b, g_log_a, g_log_b, NULL, NULL, 0,
		    &ea, &eb);
		if ((i & 0xfff) == 0)
			cap_maybe_rewind();
	}
}

/* ------------------------------------------------------------------ */
/* kunsetenv()							      */
/* ------------------------------------------------------------------ */

struct StrCtx {
	char *s;
};

static int thunk_ref_kunset(void *p)
{ return (ref_kunsetenv(((StrCtx *)p)->s)); }
static int thunk_port_kunset(void *p)
{ return (P::kunsetenv(((StrCtx *)p)->s)); }

static void
one_kunsetenv(const unsigned char *name, int off, const char *label)
{
	StrCtx ca, cb;
	size_t l;

	l = strlen((const char *)name) + 1;
	bufA.fill();
	bufB.fill();
	memcpy(bufA.b + off, name, l);
	memcpy(bufB.b + off, name, l);
	ca.s = bufA.at(off);
	cb.s = bufB.at(off);
	RunResult a = run_side(thunk_ref_kunset, &ca, &g_log_a);
	RunResult b = run_side(thunk_port_kunset, &cb, &g_log_b);
	check(S_KUNSET, label, a, b, g_log_a, g_log_b, bufA.b, bufB.b, GBUF,
	    NULL, NULL);
}

static void
test_kunsetenv(void)
{
	static const char *const hand[] = {
		"", "a", "ab", "hint.x", "\x80", "\xff\xfe\x80", "=", "a=b",
		"\x01\x7f\x80\xff", "averyveryverylongnamethatgoesonandonand"
	};
	static const int rets[] = { 0, -1, 1, -2, 5, -5 };
	unsigned char tmp[256];
	long i;
	size_t j, k;

	for (j = 0; j < sizeof(hand) / sizeof(hand[0]); j++)
		for (k = 0; k < sizeof(rets) / sizeof(rets[0]); k++) {
			script_simple(rets[k], ENOENT);
			flags_set(Flags{0, 0, 0, 0, 0, 0, 0});
			one_kunsetenv((const unsigned char *)hand[j],
			    (int)(k % 3), "hand");
		}

	for (i = 0; i < 200000; i++) {
		script_simple(rets[rndn(6)], (rndn(2) == 0) ? ENOENT : EPERM);
		flags_set(rnd_flags());
		gen_str(tmp, rndn(3) == 0 ? 120 : 12);
		one_kunsetenv(tmp, rndn(4), "rnd");
		if ((i & 0xfff) == 0)
			cap_maybe_rewind();
	}
}

/* ------------------------------------------------------------------ */
/* ksetenv()							      */
/* ------------------------------------------------------------------ */

struct SetCtx {
	char *env;
	char *val;
};

static int thunk_ref_kset(void *p)
{ return (ref_ksetenv(((SetCtx *)p)->env, ((SetCtx *)p)->val)); }
static int thunk_port_kset(void *p)
{ return (P::ksetenv(((SetCtx *)p)->env, ((SetCtx *)p)->val)); }

static void
one_ksetenv(const unsigned char *name, const unsigned char *val, int noff,
    int voff, const char *label)
{
	SetCtx ca, cb;
	size_t ln, lv;

	ln = strlen((const char *)name) + 1;
	lv = strlen((const char *)val) + 1;
	bufA.fill();
	bufB.fill();
	memcpy(bufA.b + noff, name, ln);
	memcpy(bufB.b + noff, name, ln);
	memcpy(bufA.b + voff, val, lv);
	memcpy(bufB.b + voff, val, lv);
	ca.env = bufA.at(noff);
	ca.val = bufA.at(voff);
	cb.env = bufB.at(noff);
	cb.val = bufB.at(voff);
	RunResult a = run_side(thunk_ref_kset, &ca, &g_log_a);
	RunResult b = run_side(thunk_port_kset, &cb, &g_log_b);
	check(S_KSET, label, a, b, g_log_a, g_log_b, bufA.b, bufB.b, GBUF,
	    NULL, NULL);
}

static void
test_ksetenv(void)
{
	static const char *const hand[] = {
		"", "a", "hint.a", "\x80\xff", "x=y", "\x01", "name"
	};
	static const char *const hval[] = {
		"", "v", "value", "\x80", "\xff\xff\xff", "a\"b", "=", "1",
		"averylongvaluethatkeepsgoingforquiteawhileindeed"
	};
	static const int rets[] = { 0, -1, 1, -2, 2 };
	unsigned char n[256], v[256];
	size_t i, j, k;
	long t;

	for (i = 0; i < sizeof(hand) / sizeof(hand[0]); i++)
		for (j = 0; j < sizeof(hval) / sizeof(hval[0]); j++)
			for (k = 0; k < sizeof(rets) / sizeof(rets[0]); k++) {
				script_simple(rets[k], ENOENT);
				flags_set(Flags{0, 0, 0, 0, 0, 0, 0});
				one_ksetenv((const unsigned char *)hand[i],
				    (const unsigned char *)hval[j], 3, 600,
				    "hand");
			}

	for (t = 0; t < 200000; t++) {
		script_simple(rets[rndn(5)], (rndn(2) == 0) ? ENOENT : EPERM);
		flags_set(rnd_flags());
		gen_str(n, rndn(3) == 0 ? 100 : 10);
		gen_str(v, rndn(3) == 0 ? 200 : 10);
		one_ksetenv(n, v, rndn(4), 600 + rndn(4), "rnd");
		if ((t & 0xfff) == 0)
			cap_maybe_rewind();
	}
}

/* ------------------------------------------------------------------ */
/* kgetenv()							      */
/* ------------------------------------------------------------------ */

static int thunk_ref_kget(void *p)
{ return (ref_kgetenv(((StrCtx *)p)->s)); }
static int thunk_port_kget(void *p)
{ return (P::kgetenv(((StrCtx *)p)->s)); }

static void
one_kgetenv(const unsigned char *name, int off, const char *label)
{
	StrCtx ca, cb;
	size_t l;

	l = strlen((const char *)name) + 1;
	bufA.fill();
	bufB.fill();
	memcpy(bufA.b + off, name, l);
	memcpy(bufB.b + off, name, l);
	ca.s = bufA.at(off);
	cb.s = bufB.at(off);
	RunResult a = run_side(thunk_ref_kget, &ca, &g_log_a);
	RunResult b = run_side(thunk_port_kget, &cb, &g_log_b);
	check(S_KGET, label, a, b, g_log_a, g_log_b, bufA.b, bufB.b, GBUF,
	    NULL, NULL);
}

static void
test_kgetenv(void)
{
	static const char *const hand[] = {
		"", "a", "hint.a", "\x80\xff", "\x01\x02", "kern.ident"
	};
	/* -1 is the only return kgetenv() treats as failure. */
	static const int rets[] = { -1, 0, 1, -2, 2, 1023, 1024, -1024 };
	static const char *const vals[] = {
		"", "v", "\x80\xff\xfe", "a b c", "\"quoted\"", "x"
	};
	unsigned char n[256];
	size_t i, j, k;
	int vf, len;
	long t;

	for (i = 0; i < sizeof(hand) / sizeof(hand[0]); i++)
		for (j = 0; j < sizeof(rets) / sizeof(rets[0]); j++)
			for (k = 0; k < sizeof(vals) / sizeof(vals[0]); k++)
				for (vf = -1; vf <= 1; vf++) {
					script_simple(rets[j], ENOENT);
					g_script.getval_len =
					    (int)strlen(vals[k]);
					memcpy(g_script.getval, vals[k],
					    (size_t)g_script.getval_len + 1);
					flags_set(Flags{0, 0, 0, 0, 0, 0, vf});
					one_kgetenv((const unsigned char *)
					    hand[i], 5, "hand");
				}

	/* Values around the point where the 1024 byte buffer truncates. */
	for (len = 1020; len <= 1030; len++)
		for (vf = 0; vf <= 1; vf++) {
			script_simple(0, 0);
			for (i = 0; i < (size_t)len; i++)
				g_script.getval[i] = alpha[i % sizeof(alpha)];
			g_script.getval[len] = '\0';
			g_script.getval_len = len;
			flags_set(Flags{0, 0, 0, 0, 0, 0, vf});
			one_kgetenv((const unsigned char *)"long", 0, "long");
		}

	for (t = 0; t < 200000; t++) {
		script_simple(rets[rndn(8)], (rndn(2) == 0) ? ENOENT : EPERM);
		g_script.getval_len = gen_str(g_script.getval,
		    rndn(20) == 0 ? 1199 : 24);
		flags_set(rnd_flags());
		gen_str(n, rndn(3) == 0 ? 100 : 10);
		one_kgetenv(n, rndn(4), "rnd");
		if ((t & 0xfff) == 0)
			cap_maybe_rewind();
	}
}

/* ------------------------------------------------------------------ */
/* kdumpenv()							      */
/* ------------------------------------------------------------------ */

struct DumpCtx {
	int type;
};

static int thunk_ref_kdump(void *p)
{ return (ref_kdumpenv(((DumpCtx *)p)->type)); }
static int thunk_port_kdump(void *p)
{ return (P::kdumpenv(((DumpCtx *)p)->type)); }

static void
one_kdumpenv(int type, const char *label)
{
	DumpCtx c;

	c.type = type;
	RunResult a = run_side(thunk_ref_kdump, &c, &g_log_a);
	RunResult b = run_side(thunk_port_kdump, &c, &g_log_b);
	check(S_KDUMP, label, a, b, g_log_a, g_log_b, NULL, NULL, 0, NULL,
	    NULL);
}

static void
hand_dump(const int *rets, int nret, const char *const *ents, int nent,
    const char *label)
{
	static const int types[] = {
		KENV_DUMP, KENV_DUMP_LOADER, KENV_DUMP_STATIC, 99
	};
	int hf, nf, i;

	for (hf = -1; hf <= 1; hf++)
		for (nf = -1; nf <= 1; nf++)
			for (i = 0; i < 4; i++) {
				memset(&g_script, 0, sizeof(g_script));
				g_script.nret = nret;
				memcpy(g_script.ret, rets,
				    sizeof(int) * (size_t)nret);
				g_script.err[nret - 1] = (i & 1) ? ENOENT :
				    EPERM;
				set_payload(ents, nent);
				script_make_deterministic();
				flags_set(Flags{hf, 0, nf, 0, 0, 0, 0});
				one_kdumpenv(types[i], label);
			}
}

static void
test_kdumpenv(void)
{
	static const char *const e_mixed[] = {
		"hint.a=1", "hint4=2", "hint=3", "hint.", "noeq", "abc=def",
		"\x80=\xff"
	};
	static const char *const e_empty[] = { "" };
	static const char *const e_one[] = { "a=b" };
	static const char *const e_noeq[] = { "noeq", "a=b" };
	static const char *const e_hint[] = {
		"hint.x=1", "hintx=2", "hint.y=3"
	};
	static const char *const e_stop[] = { "a=b", "", "c=d" };
	static const int types[] = {
		KENV_DUMP, KENV_DUMP_LOADER, KENV_DUMP_STATIC, 0, 99
	};
	long t;

	/* The first call fails outright. */
	{ const int r[] = { -1 }; hand_dump(r, 1, e_one, 1, "first -1"); }
	{ const int r[] = { -5 }; hand_dump(r, 1, e_one, 1, "first -5"); }
	/* envlen == 0 is not an error: both sides of `envlen < 0'. */
	{ const int r[] = { 0, 0 }; hand_dump(r, 2, e_one, 1, "zero"); }
	{ const int r[] = { 0, 1, 1 }; hand_dump(r, 3, e_one, 1, "grow0"); }
	/* Both sides of `envlen > buflen', equality included. */
	{ const int r[] = { 10, 12 }; hand_dump(r, 2, e_mixed, 7, "eq12"); }
	{ const int r[] = { 10, 13, 15 }; hand_dump(r, 3, e_mixed, 7, "gt12"); }
	{ const int r[] = { 10, 11 }; hand_dump(r, 2, e_mixed, 7, "lt12"); }
	{ const int r[] = { 50, 60 }; hand_dump(r, 2, e_mixed, 7, "eq60"); }
	{ const int r[] = { 50, 61, 73 }; hand_dump(r, 3, e_mixed, 7, "gt60"); }
	{ const int r[] = { 50, 61, 74, 88 };
	  hand_dump(r, 4, e_mixed, 7, "gt3"); }
	/* Failure on the second and the third call. */
	{ const int r[] = { 20, -1 }; hand_dump(r, 2, e_mixed, 7, "second"); }
	{ const int r[] = { 20, 30, -1 }; hand_dump(r, 3, e_mixed, 7, "third"); }
	/* Content shapes. */
	{ const int r[] = { 60, 60 }; hand_dump(r, 2, e_empty, 1, "empty"); }
	{ const int r[] = { 60, 60 }; hand_dump(r, 2, e_one, 1, "one"); }
	{ const int r[] = { 60, 60 }; hand_dump(r, 2, e_noeq, 2, "noeq"); }
	{ const int r[] = { 60, 60 }; hand_dump(r, 2, e_hint, 3, "hint"); }
	{ const int r[] = { 60, 60 }; hand_dump(r, 2, e_stop, 3, "stop"); }
	{ const int r[] = { 60, 60 }; hand_dump(r, 2, e_mixed, 7, "mixed"); }
	/* Truncation: the dump does not fit the buffer it was sized for. */
	{ const int r[] = { 20, 24 }; hand_dump(r, 2, e_mixed, 7, "trunc24"); }
	{ const int r[] = { 14, 16 }; hand_dump(r, 2, e_mixed, 7, "trunc16"); }
	{ const int r[] = { 8, 9 }; hand_dump(r, 2, e_mixed, 7, "trunc9"); }
	{ const int r[] = { 4, 4 }; hand_dump(r, 2, e_mixed, 7, "trunc4"); }
	{ const int r[] = { 1, 1 }; hand_dump(r, 2, e_mixed, 7, "trunc1"); }
	{ const int r[] = { 2, 2 }; hand_dump(r, 2, e_hint, 3, "trunc2"); }
	{ const int r[] = { 7, 8 }; hand_dump(r, 2, e_hint, 3, "trunc8"); }

	for (t = 0; t < 200000; t++) {
		memset(&g_script, 0, sizeof(g_script));
		gen_script();
		flags_set(rnd_flags());
		one_kdumpenv(types[rndn(5)], "rnd");
		if ((t & 0xfff) == 0)
			cap_maybe_rewind();
	}
}

/* ------------------------------------------------------------------ */
/* kenv_main()							      */
/* ------------------------------------------------------------------ */

struct MainCtx {
	int argc;
	char **argv;
};

static int thunk_ref_main(void *p)
{ return (ref_kenv_main(((MainCtx *)p)->argc, ((MainCtx *)p)->argv)); }
static int thunk_port_main(void *p)
{ return (P::kenv_main(((MainCtx *)p)->argc, ((MainCtx *)p)->argv)); }

enum { MAXARG = 8 };

static char *argvA[MAXARG + 1], *argvB[MAXARG + 1];

static void
build_argv(const char *const *toks, int n, GBuf &g, char **av)
{
	int i, off, l;

	g.fill();
	off = 0;
	for (i = 0; i < n; i++) {
		l = (int)strlen(toks[i]);
		memcpy(g.b + off, toks[i], (size_t)l + 1);
		av[i] = g.at(off);
		off += l + 2;		/* a guard byte between arguments */
	}
	av[n] = NULL;
}

static void
argv_offsets(Extra &e, char *const *av, const GBuf &g, int n)
{
	int i;

	for (i = 0; i <= n && i < MAXARG; i++)
		e.add(av[i] == NULL ? -1 : (long)(av[i] - (const char *)g.b));
}

static void
one_main(const char *const *toks, int n, const Flags &f, const char *label)
{
	MainCtx ca, cb;
	Extra ea, eb;

	build_argv(toks, n, bufA, argvA);
	build_argv(toks, n, bufB, argvB);
	ca.argc = n;
	ca.argv = argvA;
	cb.argc = n;
	cb.argv = argvB;

	flags_set(f);
	optind = 0;
	RunResult a = run_side(thunk_ref_main, &ca, &g_log_a);
	extra_flags_ref(ea);
	ea.add(optind);
	argv_offsets(ea, argvA, bufA, n);

	flags_set(f);
	optind = 0;
	RunResult b = run_side(thunk_port_main, &cb, &g_log_b);
	extra_flags_port(eb);
	eb.add(optind);
	argv_offsets(eb, argvB, bufB, n);

	check(S_MAIN, label, a, b, g_log_a, g_log_b, bufA.b, bufB.b, GBUF,
	    &ea, &eb);
}

static const char *const tokpool[] = {
	"-h", "-l", "-N", "-q", "-s", "-u", "-v", "-x", "-hl", "-hq", "-ls",
	"-uv", "-qu", "-qv", "-hN", "-lN", "-hlNqsuv", "--", "-", "-h-",
	"a", "b", "zz", "a=b", "=b", "a=", "=", "", "a=b=c", "name=value",
	"hint.a=1", "hint4=1", "hint=1", "hin=1", "hint.", "hint",
	"\x80\xff", "\x80=\xff", "x\x80=y", "q=", "kern.ident", "--h"
};

static const char *const progpool[] = { "kenv", "k", "\x80prog" };

static void
test_kenv_main(void)
{
	static const char *const hand[][MAXARG] = {
		{ "kenv", NULL },
		{ "kenv", "-h", NULL },
		{ "kenv", "-l", NULL },
		{ "kenv", "-s", NULL },
		{ "kenv", "-N", NULL },
		{ "kenv", "-q", NULL },
		{ "kenv", "-u", NULL },
		{ "kenv", "-v", NULL },
		{ "kenv", "-x", NULL },
		{ "kenv", "-l", "-s", NULL },
		{ "kenv", "-ls", NULL },
		{ "kenv", "-h", "-N", NULL },
		{ "kenv", "-hN", NULL },
		{ "kenv", "-h", "a", NULL },
		{ "kenv", "-N", "a", NULL },
		{ "kenv", "-h", "a=b", NULL },
		{ "kenv", "a", NULL },
		{ "kenv", "a=b", NULL },
		{ "kenv", "=b", NULL },
		{ "kenv", "a=", NULL },
		{ "kenv", "=", NULL },
		{ "kenv", "", NULL },
		{ "kenv", "a=b=c", NULL },
		{ "kenv", "a", "b", NULL },
		{ "kenv", "a=b", "c", NULL },
		{ "kenv", "-u", "a", NULL },
		{ "kenv", "-u", "a=b", NULL },
		{ "kenv", "-v", "a", NULL },
		{ "kenv", "-v", "a=b", NULL },
		{ "kenv", "-q", "a", NULL },
		{ "kenv", "-q", "-u", "zz", NULL },
		{ "kenv", "-qv", "a=b", NULL },
		{ "kenv", "--", "-h", NULL },
		{ "kenv", "-h", "--", "a", NULL },
		{ "kenv", "-l", "-h", NULL },
		{ "kenv", "-s", "-N", NULL },
		{ "kenv", "-lq", NULL },
		{ "kenv", "-sq", NULL },
		{ "kenv", "\x80\xff=\xfe", NULL },
		{ "kenv", "hint.a=1", NULL },
		{ "kenv", "-uv", "a", NULL },
		{ "kenv", "-", NULL },
		{ "kenv", "-", "a", NULL },
		{ "kenv", "a", "-h", NULL },
		{ "kenv", "a=b", "-l", NULL },
		{ "kenv", "-lqv", "a=b", NULL },
		{ "kenv", "-su", "a", NULL },
	};
	static const Flags fset[] = {
		{ 0, 0, 0, 0, 0, 0, 0 },
		{ -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 1, 0, 0, 0, 0 },
		{ 0, 1, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 1, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 1, 0 },
		{ 0, 0, 0, 0, 0, 0, 1 },
		{ -1, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, -1, 0, 0, 0, 0 },
		{ 0, -1, 0, 0, -1, 0, 0 },
	};
	static const int scripts[][3] = {
		{ -1, 0, 0 }, { 0, 0, 0 }, { 60, 60, 0 }, { 10, 12, 0 },
		{ 10, 13, 15 }, { 20, -1, 0 }, { 1, 1, 0 }
	};
	static const int scriptn[] = { 1, 2, 2, 2, 3, 2, 2 };
	static const char *const ents[] = {
		"hint.a=1", "hint4=2", "abc=def", "noeq", "\x80=\xff"
	};
	const char *toks[MAXARG];
	char rndtok[MAXARG][40];
	size_t hi, fi, si;
	int n, i;
	long t;

	for (hi = 0; hi < sizeof(hand) / sizeof(hand[0]); hi++) {
		for (n = 0; n < MAXARG && hand[hi][n] != NULL; n++)
			toks[n] = hand[hi][n];
		for (fi = 0; fi < sizeof(fset) / sizeof(fset[0]); fi++)
			for (si = 0; si < sizeof(scripts) / sizeof(scripts[0]);
			    si++) {
				memset(&g_script, 0, sizeof(g_script));
				g_script.nret = scriptn[si];
				for (i = 0; i < scriptn[si]; i++)
					g_script.ret[i] = scripts[si][i];
				g_script.err[0] = (si & 1) ? ENOENT : EPERM;
				g_script.err[scriptn[si] - 1] = (si & 1) ?
				    ENOENT : EPERM;
				set_payload(ents, 5);
				g_script.getval_len = 3;
				memcpy(g_script.getval, "val", 4);
				script_make_deterministic();
				one_main(toks, n, fset[fi], "hand");
			}
	}

	for (t = 0; t < 200000; t++) {
		n = 1 + rndn(4);
		for (i = 0; i < n; i++) {
			if (i == 0) {
				toks[0] = progpool[rndn(3)];
				continue;
			}
			if (rndn(12) == 0) {
				gen_str((unsigned char *)rndtok[i], 8);
				toks[i] = rndtok[i];
			} else
				toks[i] = tokpool[rndn((int)(sizeof(tokpool) /
				    sizeof(tokpool[0])))];
		}
		memset(&g_script, 0, sizeof(g_script));
		gen_script();
		one_main(toks, n, rnd_flags(), "rnd");
		if ((t & 0xfff) == 0)
			cap_maybe_rewind();
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long total_fail = 0;
	int i;

	cap_init();
	rng_seed(0x5eed0180cafe1234ULL);

	test_usage();
	test_kunsetenv();
	test_ksetenv();
	test_kgetenv();
	test_kdumpenv();
	test_kenv_main();

	fprintf(rep, "\n%-12s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	fprintf(rep, "%-12s %12s %12s  %s\n", "------------", "------------",
	    "------------", "------");
	for (i = 0; i < S_NSLOT; i++) {
		fprintf(rep, "%-12s %12ld %12ld  %s\n", slot_name[i],
		    stat_cases[i], stat_fails[i],
		    stat_fails[i] == 0 ? "ok" : "FAIL");
		total_fail += stat_fails[i];
	}
	fprintf(rep, "\nb0180 %s (%ld failures)\n",
	    total_fail == 0 ? "PASS" : "FAIL", total_fail);
	fflush(rep);
	return (total_fail == 0 ? 0 : 1);
}
