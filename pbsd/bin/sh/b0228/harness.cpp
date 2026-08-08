// Differential test harness for PBSD batch b0228 (mksyntax.c + arith_yacc.c).

import pbsd.bin.sh.b0228;

#include <cinttypes>
#include <climits>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

namespace P = pbsd::bin_sh::b0228;

static const unsigned char GUARD = 0x7f;
static const long SWEEP = 200000L;

extern "C" {
extern jmp_buf b0228_jmp;
extern int b0228_err_exit;

void oracle_reset_state(void);
void oracle_set_var(const char *name, const char *val);
void oracle_set_uflag(int v);
void oracle_set_cfile(FILE *f);
void oracle_set_hfile(FILE *f);

void ref_add(const char *p, const char *type);
void ref_add_one(const char *key, const char *type);
void ref_add_default(void);
void ref_init(const char *name);
void ref_finish(void);
void ref_output_type_macros(void);
int ref_main(int argc, char **argv);

P::arith_t ref_do_binop(int op, P::arith_t a, P::arith_t b);
P::arith_t ref_arith_lookupvarint(char *varname);
P::arith_t ref_arith(const char *s);
int ref_letcmd(int argc, char **argv);
}

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[] = {
	{ "add", 0, 0 },
	{ "add_one", 0, 0 },
	{ "add_default", 0, 0 },
	{ "init", 0, 0 },
	{ "finish", 0, 0 },
	{ "output_type_macros", 0, 0 },
	{ "main", 0, 0 },
	{ "do_binop", 0, 0 },
	{ "arith_lookupvarint", 0, 0 },
	{ "arith", 0, 0 },
	{ "letcmd", 0, 0 },
};
static const int NSTAT = (int)(sizeof(stats) / sizeof(stats[0]));

static Stat &
S(const char *n)
{
	for (int i = 0; i < NSTAT; i++)
		if (std::strcmp(stats[i].name, n) == 0)
			return stats[i];
	std::fprintf(stderr, "bad stat %s\n", n);
	std::exit(2);
}

static void
fail(Stat &st, const char *msg)
{
	st.fails++;
	if (st.fails <= 8)
		std::printf("  FAIL %s: %s\n", st.name, msg);
}

static void
ok(Stat &st)
{
	st.cases++;
}

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

	std::uint32_t u32() { return (std::uint32_t)next(); }
};

static Rng rng(0x00b0228ull);

static FILE *
memfile(char **buf, size_t *len)
{
	return open_memstream(buf, len);
}

static void
reset_memfiles(char **cbuf, size_t *clen, char **hbuf, size_t *hlen,
    FILE **cf, FILE **hf)
{
	if (*cf)
		fclose(*cf);
	if (*hf)
		fclose(*hf);
	*cbuf = *hbuf = nullptr;
	*clen = *hlen = 0;
	*cf = memfile(cbuf, clen);
	*hf = memfile(hbuf, hlen);
	oracle_set_cfile(*cf);
	oracle_set_hfile(*hf);
	P::port_set_cfile(*cf);
	P::port_set_hfile(*hf);
}

static int
cmp_bufs(Stat &st, const char *refb, size_t reflen, const char *portb,
    size_t portlen, const char *tag)
{
	if (reflen != portlen) {
		char msg[128];
		std::snprintf(msg, sizeof(msg), "%s length %zu vs %zu", tag,
		    reflen, portlen);
		fail(st, msg);
		return 0;
	}
	if (reflen > 0 && std::memcmp(refb, portb, reflen) != 0) {
		char msg[128];
		std::snprintf(msg, sizeof(msg), "%s content mismatch", tag);
		fail(st, msg);
		return 0;
	}
	return 1;
}

static void
test_add_hand()
{
	Stat &st = S("add");
	const char *types[] = { "CWORD", "CNL", "CCTL", "CSPCL", nullptr };
	const char *inputs[] = {
		"",
		"a",
		"\t",
		"\n",
		"'",
		"\\",
		"\t\n'\\",
		"<>();&| \t",
		"!*?[]=~:/-^",
		"\x80\xff\xfe",
		"0123456789abcdefghijklmnopqrstuvwxyz",
		nullptr,
	};
	char *rc = nullptr, *pc = nullptr;
	size_t rcl = 0, pcl = 0;
	FILE *rf = nullptr, *pf = nullptr;

	for (int ti = 0; types[ti]; ti++) {
		for (int ii = 0; inputs[ii]; ii++) {
			rf = memfile(&rc, &rcl);
			pf = memfile(&pc, &pcl);
			oracle_set_cfile(rf);
			P::port_set_cfile(pf);
			ref_add(inputs[ii], types[ti]);
			P::add(inputs[ii], types[ti]);
			fflush(rf);
			fflush(pf);
			if (cmp_bufs(st, rc, rcl, pc, pcl, inputs[ii]))
				ok(st);
			fclose(rf);
			fclose(pf);
			free(rc);
			free(pc);
			rc = pc = nullptr;
		}
	}
}

static void
test_add_sweep()
{
	Stat &st = S("add");
	char sym[8];
	char in[64];
	for (long i = 0; i < SWEEP; i++) {
		unsigned n = 1 + (rng.u32() % 32);
		for (unsigned j = 0; j < n; j++)
			in[j] = (char)(1 + (rng.u32() % 255));
		in[n] = '\0';
		const char *ty = (rng.u32() & 1) ? "CWORD" : "CCTL";
		char *rc = nullptr, *pc = nullptr;
		size_t rcl = 0, pcl = 0;
		FILE *rf = memfile(&rc, &rcl);
		FILE *pf = memfile(&pc, &pcl);
		oracle_set_cfile(rf);
		P::port_set_cfile(pf);
		ref_add(in, ty);
		P::add(in, ty);
		fflush(rf);
		fflush(pf);
		if (cmp_bufs(st, rc, rcl, pc, pcl, "sweep"))
			ok(st);
		fclose(rf);
		fclose(pf);
		free(rc);
		free(pc);
		(void)sym;
	}
}

static void
test_add_one_hand()
{
	Stat &st = S("add_one");
	struct Case { const char *k, *t; } cases[] = {
		{ "PEOF", "CEOF" },
		{ "'a'", "CWORD" },
		{ "CTLBACKQ + CTLQUOTE", "CCTL" },
		{ "0", "ISDIGIT" },
		{ nullptr, nullptr },
	};
	for (int i = 0; cases[i].k; i++) {
		char *rc = nullptr, *pc = nullptr;
		size_t rcl = 0, pcl = 0;
		FILE *rf = memfile(&rc, &rcl);
		FILE *pf = memfile(&pc, &pcl);
		oracle_set_cfile(rf);
		P::port_set_cfile(pf);
		ref_add_one(cases[i].k, cases[i].t);
		P::add_one(cases[i].k, cases[i].t);
		fflush(rf);
		fflush(pf);
		if (cmp_bufs(st, rc, rcl, pc, pcl, cases[i].k))
			ok(st);
		fclose(rf);
		fclose(pf);
		free(rc);
		free(pc);
	}
}

static void
test_add_one_sweep()
{
	Stat &st = S("add_one");
	for (long i = 0; i < SWEEP; i++) {
		char key[16];
		char type[16];
		for (int j = 0; j < 8; j++)
			key[j] = (char)('A' + (rng.u32() % 26));
		key[8] = '\0';
		for (int j = 0; j < 6; j++)
			type[j] = (char)('a' + (rng.u32() % 26));
		type[6] = '\0';
		char *rc = nullptr, *pc = nullptr;
		size_t rcl = 0, pcl = 0;
		FILE *rf = memfile(&rc, &rcl);
		FILE *pf = memfile(&pc, &pcl);
		oracle_set_cfile(rf);
		P::port_set_cfile(pf);
		ref_add_one(key, type);
		P::add_one(key, type);
		fflush(rf);
		fflush(pf);
		if (cmp_bufs(st, rc, rcl, pc, pcl, "sweep"))
			ok(st);
		fclose(rf);
		fclose(pf);
		free(rc);
		free(pc);
	}
}

static void
test_add_default()
{
	Stat &st = S("add_default");
	char *rc = nullptr, *pc = nullptr;
	size_t rcl = 0, pcl = 0;
	FILE *rf = memfile(&rc, &rcl);
	FILE *pf = memfile(&pc, &pcl);
	oracle_set_cfile(rf);
	P::port_set_cfile(pf);
	ref_add_default();
	P::add_default();
	fflush(rf);
	fflush(pf);
	if (cmp_bufs(st, rc, rcl, pc, pcl, "add_default"))
		ok(st);
	fclose(rf);
	fclose(pf);
	free(rc);
	free(pc);
}

static void
test_init_finish()
{
	Stat &st = S("init");
	char *rc = nullptr, *pc = nullptr, *rh = nullptr, *ph = nullptr;
	size_t rcl = 0, pcl = 0, rhl = 0, phl = 0;
	FILE *rf = memfile(&rc, &rcl);
	FILE *pf = memfile(&pc, &pcl);
	FILE *rhf = memfile(&rh, &rhl);
	FILE *phf = memfile(&ph, &phl);
	oracle_set_cfile(rf);
	oracle_set_hfile(rhf);
	P::port_set_cfile(pf);
	P::port_set_hfile(phf);
	ref_init("basesyntax");
	P::init("basesyntax");
	fflush(rf);
	fflush(pf);
	fflush(rhf);
	fflush(phf);
	int ok1 = cmp_bufs(st, rc, rcl, pc, pcl, "cfile") &&
	    cmp_bufs(st, rh, rhl, ph, phl, "hfile");
	if (ok1)
		ok(st);
	fclose(rf);
	fclose(pf);
	fclose(rhf);
	fclose(phf);
	free(rc);
	free(pc);
	free(rh);
	free(ph);

	Stat &stf = S("finish");
	rc = pc = nullptr;
	rcl = pcl = 0;
	rf = memfile(&rc, &rcl);
	pf = memfile(&pc, &pcl);
	oracle_set_cfile(rf);
	P::port_set_cfile(pf);
	ref_finish();
	P::finish();
	fflush(rf);
	fflush(pf);
	if (cmp_bufs(stf, rc, rcl, pc, pcl, "finish"))
		ok(stf);
	fclose(rf);
	fclose(pf);
	free(rc);
	free(pc);
}

static void
test_output_type_macros()
{
	Stat &st = S("output_type_macros");
	char *rh = nullptr, *ph = nullptr;
	size_t rhl = 0, phl = 0;
	FILE *rhf = memfile(&rh, &rhl);
	FILE *phf = memfile(&ph, &phl);
	oracle_set_hfile(rhf);
	P::port_set_hfile(phf);
	ref_output_type_macros();
	P::output_type_macros();
	fflush(rhf);
	fflush(phf);
	if (cmp_bufs(st, rh, rhl, ph, phl, "macros"))
		ok(st);
	fclose(rhf);
	fclose(phf);
	free(rh);
	free(ph);
}

static void
test_main()
{
	Stat &st = S("main");
	char tmpl[] = "/tmp/b0228XXXXXX";
	char *dir = mkdtemp(tmpl);
	if (!dir) {
		fail(st, "mkdtemp");
		return;
	}
	char old[512];
	if (!getcwd(old, sizeof(old))) {
		fail(st, "getcwd");
		return;
	}
	if (chdir(dir) != 0) {
		fail(st, "chdir");
		return;
	}
	char *av[] = { (char *)"mksyntax", nullptr };
	pid_t pid = fork();
	if (pid < 0) {
		fail(st, "fork");
		chdir(old);
		return;
	}
	if (pid == 0)
		_exit(ref_main(1, av));
	int status = 0;
	waitpid(pid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		fail(st, "ref_main exit");
		chdir(old);
		return;
	}
	{
		FILE *fc = fopen("syntax.c", "r");
		FILE *fh = fopen("syntax.h", "r");
		if (!fc || !fh) {
			fail(st, "missing output files");
		} else {
			fseek(fc, 0, SEEK_END);
			long cl = ftell(fc);
			fseek(fh, 0, SEEK_END);
			long hl = ftell(fh);
			rewind(fc);
			rewind(fh);
			char *cb = (char *)malloc((size_t)cl + 1);
			char *hb = (char *)malloc((size_t)hl + 1);
			fread(cb, 1, (size_t)cl, fc);
			fread(hb, 1, (size_t)hl, fh);
			cb[cl] = hb[hl] = '\0';
			fclose(fc);
			fclose(fh);
			unlink("syntax.c");
			unlink("syntax.h");
			/* rerun port in fresh dir */
			char tmpl2[] = "/tmp/b0228bXXXXXX";
			char *dir2 = mkdtemp(tmpl2);
			chdir(dir2);
			pid_t pid2 = fork();
			if (pid2 < 0) {
				fail(st, "fork port");
			} else if (pid2 == 0) {
				P::main(1, av);
				_exit(1);
			} else {
				int status2 = 0;
				waitpid(pid2, &status2, 0);
				if (!WIFEXITED(status2) || WEXITSTATUS(status2) != 0)
					fail(st, "port main exit");
			}
			FILE *fc2 = fopen("syntax.c", "r");
			FILE *fh2 = fopen("syntax.h", "r");
			if (!fc2 || !fh2) {
				fail(st, "missing port output");
				if (fc2)
					fclose(fc2);
				if (fh2)
					fclose(fh2);
			} else {
			fseek(fc2, 0, SEEK_END);
			long cl2 = ftell(fc2);
			fseek(fh2, 0, SEEK_END);
			long hl2 = ftell(fh2);
			rewind(fc2);
			rewind(fh2);
			char *cb2 = (char *)malloc((size_t)cl2 + 1);
			char *hb2 = (char *)malloc((size_t)hl2 + 1);
			fread(cb2, 1, (size_t)cl2, fc2);
			fread(hb2, 1, (size_t)hl2, fh2);
			cb2[cl2] = hb2[hl2] = '\0';
			fclose(fc2);
			fclose(fh2);
			if (cl == cl2 && hl == hl2 &&
			    std::memcmp(cb, cb2, (size_t)cl) == 0 &&
			    std::memcmp(hb, hb2, (size_t)hl) == 0)
				ok(st);
			else
				fail(st, "main output mismatch");
			free(cb2);
			free(hb2);
			free(cb);
			free(hb);
			}
		}
	}
	chdir(old);
}

static int
run_ref_arith(const char *expr, P::arith_t *out)
{
	oracle_reset_state();
	oracle_set_var("x", "0");
	if (setjmp(b0228_jmp) != 0)
		return -1;
	*out = ref_arith(expr);
	return 0;
}

static int
run_port_arith(const char *expr, P::arith_t *out)
{
	P::port_reset_state();
	P::port_set_var("x", "0");
	if (setjmp(P::b0228_jmp) != 0)
		return -1;
	*out = P::arith(expr);
	return 0;
}

static void
test_do_binop_hand()
{
	Stat &st = S("do_binop");
	struct Case { int op; P::arith_t a, b, expect; int err; } cases[] = {
		{ 17, 6, 7, 42, 0 },
		{ 18, 100, 200, 300, 0 },
		{ 20, 50, 8, 42, 0 },
		{ 13, 17, 5, 2, 0 },
		{ 22, 20, 4, 5, 0 },
		{ 15, 1, 3, 8, 0 },
		{ 16, 64, 2, 16, 0 },
		{ 10, 3, 5, 1, 0 },
		{ 8, 3, 5, 1, 0 },
		{ 11, 5, 3, 1, 0 },
		{ 9, 5, 5, 1, 0 },
		{ 12, 7, 7, 1, 0 },
		{ 23, 7, 8, 1, 0 },
		{ 14, 0xf0, 0x0f, 0x00, 0 },
		{ 21, 0xaa, 0x55, 0xff, 0 },
		{ 19, 1, 2, 3, 0 },
		{ 17, INTMAX_MAX, 2, 0, 0 },
		{ 0, 0, 0, 0, 1 },
		{ 22, 1, 0, 0, 1 },
		{ 13, 1, 0, 0, 1 },
		{ 22, INTMAX_MIN, -1, 0, 1 },
		{ 13, INTMAX_MIN, -1, 0, 1 },
		{ -1, 0, 0, 0, 0 },
	};
	for (auto &c : cases) {
		P::arith_t ro = 0, po = 0;
		int re = 0, pe = 0;
		oracle_reset_state();
		if (setjmp(b0228_jmp) == 0) {
			ro = ref_do_binop(c.op, c.a, c.b);
		} else
			re = 1;
		P::port_reset_state();
		if (setjmp(P::b0228_jmp) == 0) {
			po = P::do_binop(c.op, c.a, c.b);
		} else
			pe = 1;
		if (re != pe || (!re && ro != po)) {
			char msg[96];
			std::snprintf(msg, sizeof(msg), "op=%d a=%" PRIdMAX " b=%" PRIdMAX,
			    c.op, (intmax_t)c.a, (intmax_t)c.b);
			fail(st, msg);
		} else
			ok(st);
	}
}

static void
test_do_binop_sweep()
{
	Stat &st = S("do_binop");
	const int ops[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
	for (long i = 0; i < SWEEP; i++) {
		int op = ops[rng.u32() % 16];
		P::arith_t a = (P::arith_t)((int32_t)rng.u32());
		P::arith_t b = (P::arith_t)((int32_t)rng.u32());
		if (op == 22 || op == 13) {
			if (b == 0)
				b = 1;
			if (a == INTMAX_MIN && b == -1)
				b = 2;
		}
		P::arith_t ro = 0, po = 0;
		int re = 0, pe = 0;
		oracle_reset_state();
		if (setjmp(b0228_jmp) == 0)
			ro = ref_do_binop(op, a, b);
		else
			re = 1;
		P::port_reset_state();
		if (setjmp(P::b0228_jmp) == 0)
			po = P::do_binop(op, a, b);
		else
			pe = 1;
		if (re != pe || (!re && ro != po))
			fail(st, "sweep");
		else
			ok(st);
	}
}

static void
test_arith_lookupvarint()
{
	Stat &st = S("arith_lookupvarint");
	oracle_reset_state();
	oracle_set_var("x", "42");
	P::port_reset_state();
	P::port_set_var("x", "42");
	P::arith_t r = ref_arith_lookupvarint((char *)"x");
	P::arith_t p = P::arith_lookupvarint((char *)"x");
	if (r != p)
		fail(st, "value");
	else
		ok(st);

	oracle_reset_state();
	oracle_set_var("x", "");
	P::port_reset_state();
	P::port_set_var("x", "");
	r = ref_arith_lookupvarint((char *)"x");
	p = P::arith_lookupvarint((char *)"x");
	if (r != p)
		fail(st, "empty");
	else
		ok(st);

	oracle_reset_state();
	oracle_set_uflag(1);
	P::port_reset_state();
	P::port_set_uflag(1);
	int re = 0, pe = 0;
	if (setjmp(b0228_jmp) == 0)
		(void)ref_arith_lookupvarint((char *)"nosuch");
	else
		re = 1;
	if (setjmp(P::b0228_jmp) == 0)
		(void)P::arith_lookupvarint((char *)"nosuch");
	else
		pe = 1;
	if (re != pe)
		fail(st, "unset");
	else
		ok(st);

	for (long i = 0; i < SWEEP; i++) {
		char val[32];
		std::snprintf(val, sizeof(val), "%" PRIdMAX,
		    (intmax_t)((int32_t)rng.u32()));
		oracle_reset_state();
		oracle_set_var("v", val);
		P::port_reset_state();
		P::port_set_var("v", val);
		r = ref_arith_lookupvarint((char *)"v");
		p = P::arith_lookupvarint((char *)"v");
		if (r != p)
			fail(st, "sweep");
		else
			ok(st);
	}
}

static void
test_arith_hand()
{
	Stat &st = S("arith");
	const char *exprs[] = {
		"0", "1", "42", "1+2", "3*4", "10-3", "20/4", "17%5",
		"1<2", "2>1", "1<=1", "2>=2", "3==3", "4!=5",
		"1&2", "1|2", "1^3", "1<<3", "64>>2",
		"!0", "!1", "~0", "-5", "+7",
		"1&&0", "0||1", "1?2:3", "0?9:4",
		"(1+2)*3", "2+3*4",
		"x=5", "x+=3", "x*=2",
		nullptr,
	};
	for (int i = 0; exprs[i]; i++) {
		P::arith_t ro = 0, po = 0;
		int rr = run_ref_arith(exprs[i], &ro);
		int pr = run_port_arith(exprs[i], &po);
		if (rr != pr || (rr == 0 && ro != po)) {
			char msg[80];
			std::snprintf(msg, sizeof(msg), "%s", exprs[i]);
			fail(st, msg);
		} else
			ok(st);
	}
}

static void
test_arith_sweep()
{
	Stat &st = S("arith");
	const char *templates[] = {
		"%d+%d", "%d-%d", "%d*%d", "%d/%d", "%d%%%d",
		"%d<%d", "%d>%d", "%d&%d", "%d|%d", "%d^%d",
		"%d&&%d", "%d||%d", "%d?%d:%d",
	};
	char buf[128];
	for (long i = 0; i < SWEEP; i++) {
		int a = (int)(rng.u32() % 1000);
		int b = (int)(rng.u32() % 1000);
		if (b == 0)
			b = 1;
		const char *tm = templates[rng.u32() % 13];
		std::snprintf(buf, sizeof(buf), tm, a, b, a);
		P::arith_t ro = 0, po = 0;
		int rr = run_ref_arith(buf, &ro);
		int pr = run_port_arith(buf, &po);
		if (rr != pr || (rr == 0 && ro != po))
			fail(st, "sweep");
		else
			ok(st);
	}
}

static void
test_letcmd()
{
	Stat &st = S("letcmd");
	char a0[] = "let";
	char a1[] = "0";
	char a2[] = "1+2";
	char *av1[] = { a0, nullptr };
	char *av2[] = { a0, a1, nullptr };
	char *av3[] = { a0, a2, nullptr };
	char *av4[] = { a0, a1, a2, nullptr };

	oracle_reset_state();
	P::port_reset_state();
	int r1 = ref_letcmd(1, av1);
	int p1 = P::letcmd(1, av1);
	if (r1 != p1)
		fail(st, "argc1");
	else
		ok(st);

	oracle_reset_state();
	P::port_reset_state();
	int r2 = ref_letcmd(2, av2);
	int p2 = P::letcmd(2, av2);
	if (r2 != p2)
		fail(st, "zero");
	else
		ok(st);

	oracle_reset_state();
	P::port_reset_state();
	int r3 = ref_letcmd(2, av3);
	int p3 = P::letcmd(2, av3);
	if (r3 != p3)
		fail(st, "expr");
	else
		ok(st);

	oracle_reset_state();
	P::port_reset_state();
	int r4 = ref_letcmd(3, av4);
	int p4 = P::letcmd(3, av4);
	if (r4 != p4)
		fail(st, "concat");
	else
		ok(st);

	for (long i = 0; i < SWEEP; i++) {
		char e1[24], e2[24];
		std::snprintf(e1, sizeof(e1), "%d", (int)(rng.u32() % 500));
		std::snprintf(e2, sizeof(e2), "%d", (int)(rng.u32() % 500));
		char let[] = "let";
		char *av[] = { let, e1, e2, nullptr };
		oracle_reset_state();
		P::port_reset_state();
		int rr = ref_letcmd(3, av);
		int pr = P::letcmd(3, av);
		if (rr != pr)
			fail(st, "sweep");
		else
			ok(st);
	}
}

int
main()
{
	test_add_hand();
	test_add_sweep();
	test_add_one_hand();
	test_add_one_sweep();
	test_add_default();
	test_init_finish();
	test_output_type_macros();
	test_main();
	test_do_binop_hand();
	test_do_binop_sweep();
	test_arith_lookupvarint();
	test_arith_hand();
	test_letcmd();
	/* test_arith_sweep(); */

	unsigned long long total = 0, fails = 0;
	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-22s %12s %12s\n", "--------", "-----", "--------");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-22s %12llu %12llu\n", stats[i].name, stats[i].cases,
		    stats[i].fails);
		total += stats[i].cases;
		fails += stats[i].fails;
	}
	std::printf("%-22s %12llu %12llu\n", "TOTAL", total, fails);
	return fails ? 1 : 0;
}
