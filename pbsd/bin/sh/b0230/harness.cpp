// Differential harness for PBSD batch b0230 (mknodes.c).

import pbsd.bin.sh.b0230;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

namespace P = pbsd::bin_sh::b0230;

static const unsigned char GUARD = 0x7f;
static const long SWEEP = 200000L;

extern "C" {
void oracle_mknodes_reset(void);
void oracle_mknodes_set_line(const char *s);
int oracle_mknodes_get_linno(void);

int ref_mknodes_main(int argc, char **argv);
void ref_parsenode(void);
void ref_parsefield(void);
void ref_mknodes_output(char *);
void ref_outsizes(FILE *);
void ref_outfunc(FILE *, int);
void ref_indent(int, FILE *);
int ref_nextfield(char *);
void ref_skipbl(void);
int ref_readline(FILE *);
char *ref_mknodes_savestr(const char *);
}

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[] = {
	{ "mknodes_savestr", 0, 0 },
	{ "nextfield", 0, 0 },
	{ "skipbl", 0, 0 },
	{ "readline", 0, 0 },
	{ "indent", 0, 0 },
	{ "parsenode", 0, 0 },
	{ "parsefield", 0, 0 },
	{ "outsizes", 0, 0 },
	{ "outfunc", 0, 0 },
	{ "mknodes_output", 0, 0 },
	{ "mknodes_main", 0, 0 },
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

static Rng rng(0x00b0230ull);

static void
reset_both()
{
	oracle_mknodes_reset();
	P::port_mknodes_reset();
}

static int
cmp_bufs(Stat &st, const char *a, size_t alen, const char *b, size_t blen,
    const char *tag)
{
	if (alen != blen) {
		char msg[128];
		std::snprintf(msg, sizeof(msg), "%s len %zu vs %zu", tag, alen, blen);
		fail(st, msg);
		return 0;
	}
	if (alen && std::memcmp(a, b, alen) != 0) {
		char msg[128];
		std::snprintf(msg, sizeof(msg), "%s content", tag);
		fail(st, msg);
		return 0;
	}
	return 1;
}

static void
fill_guard(unsigned char *b, size_t n)
{
	for (size_t i = 0; i < n; i++)
		b[i] = GUARD;
}

static void
test_savestr_hand()
{
	Stat &st = S("mknodes_savestr");
	const char *inputs[] = {
		"", "a", "NCMD", "\x80\xff\xfe", "0123456789",
		"nodeptr string int other temp", nullptr,
	};
	for (int i = 0; inputs[i]; i++) {
		reset_both();
		char *r = ref_mknodes_savestr(inputs[i]);
		char *p = P::mknodes_savestr(inputs[i]);
		if (std::strcmp(r, p) != 0)
			fail(st, inputs[i]);
		else
			ok(st);
		std::free(r);
		std::free(p);
	}
}

static void
test_savestr_sweep()
{
	Stat &st = S("mknodes_savestr");
	char in[64];
	for (long i = 0; i < SWEEP; i++) {
		size_t n = (size_t)(rng.u32() % 40);
		for (size_t j = 0; j < n; j++)
			in[j] = (char)(rng.u32() & 0xff);
		in[n] = '\0';
		reset_both();
		char *r = ref_mknodes_savestr(in);
		char *p = P::mknodes_savestr(in);
		if (std::strcmp(r, p) != 0)
			fail(st, "sweep");
		else
			ok(st);
		std::free(r);
		std::free(p);
	}
}

static void
test_nextfield_pair(const char *line, int nf)
{
	Stat &st = S("nextfield");
	char rbuf[256], pbuf[256];
	fill_guard((unsigned char *)rbuf, sizeof(rbuf));
	fill_guard((unsigned char *)pbuf, sizeof(pbuf));
	reset_both();
	oracle_mknodes_set_line(line);
	P::port_mknodes_set_line(line);
	int rret = 0, pret = 0;
	for (int i = 0; i < nf; i++) {
		int rr = ref_nextfield(rbuf + 1);
		int pr = P::nextfield(pbuf + 1);
		rret = rr;
		pret = pr;
		if (rr != pr || std::strcmp(rbuf + 1, pbuf + 1) != 0) {
			fail(st, line);
			return;
		}
	}
	if (rret != pret)
		fail(st, "ret");
	else if (rbuf[0] != GUARD || pbuf[0] != GUARD || rbuf[sizeof(rbuf) - 1] != GUARD ||
	    pbuf[sizeof(pbuf) - 1] != GUARD)
		fail(st, "guard");
	else
		ok(st);
}

static void
test_nextfield_hand()
{
	test_nextfield_pair("", 0);
	test_nextfield_pair("   ", 0);
	test_nextfield_pair("a", 1);
	test_nextfield_pair("a b", 2);
	test_nextfield_pair("\ta\tb\tc", 3);
	test_nextfield_pair("NCMD\tcmdnode", 2);
	test_nextfield_pair("  foo  bar\x80\xff", 2);
	test_nextfield_pair("x", 1);
	test_nextfield_pair("x ", 1);
	test_nextfield_pair(" x", 1);
}

static void
test_nextfield_sweep()
{
	Stat &st = S("nextfield");
	char line[120];
	for (long i = 0; i < SWEEP; i++) {
		size_t n = (size_t)(rng.u32() % (sizeof(line) - 1));
		for (size_t j = 0; j < n; j++) {
			unsigned v = rng.u32() % 5;
			line[j] = (char)(v == 0 ? ' ' : v == 1 ? '\t' : (rng.u32() % 26) + 'a');
		}
		line[n] = '\0';
		char rbuf[256], pbuf[256];
		reset_both();
		oracle_mknodes_set_line(line);
		P::port_mknodes_set_line(line);
		int rr = ref_nextfield(rbuf);
		int pr = P::nextfield(pbuf);
		if (rr != pr || std::strcmp(rbuf, pbuf) != 0)
			fail(st, "sweep");
		else
			ok(st);
	}
}

static void
test_skipbl_hand()
{
	Stat &st = S("skipbl");
	const char *lines[] = { "", " ", "\t", " \t \tx", "x", "\x80", nullptr };
	for (int i = 0; lines[i]; i++) {
		reset_both();
		oracle_mknodes_set_line(lines[i]);
		P::port_mknodes_set_line(lines[i]);
		ref_skipbl();
		P::skipbl();
		char rb[256], pb[256];
		int rr = ref_nextfield(rb);
		int pr = P::nextfield(pb);
		if (rr != pr || std::strcmp(rb, pb) != 0)
			fail(st, lines[i]);
		else
			ok(st);
	}
}

static void
test_skipbl_sweep()
{
	Stat &st = S("skipbl");
	char line[80];
	for (long i = 0; i < SWEEP; i++) {
		size_t lead = (size_t)(rng.u32() % 20);
		size_t n = lead + (size_t)(rng.u32() % 30);
		if (n >= sizeof(line))
			n = sizeof(line) - 1;
		for (size_t j = 0; j < n; j++)
			line[j] = (j < lead) ? (char)((rng.u32() & 1) ? ' ' : '\t') :
					       (char)(rng.u32() & 0xff);
		line[n] = '\0';
		reset_both();
		oracle_mknodes_set_line(line);
		P::port_mknodes_set_line(line);
		ref_skipbl();
		P::skipbl();
		char rb[128], pb[128];
		if (ref_nextfield(rb) != P::nextfield(pb) || std::strcmp(rb, pb) != 0)
			fail(st, "sweep");
		else
			ok(st);
	}
}

static void
test_readline_hand()
{
	Stat &st = S("readline");
	const char *inputs[] = {
		"",
		"NCMD cmdnode\n",
		" NCMDargv nodeptr\n",
		"line with # comment\n",
		"trail spaces   \n",
		"\x80\xff\n",
		"# only comment\n",
		nullptr,
	};
	for (int i = 0; inputs[i]; i++) {
		reset_both();
		FILE *rf = fmemopen((void *)inputs[i], std::strlen(inputs[i]), "r");
		FILE *pf = fmemopen((void *)inputs[i], std::strlen(inputs[i]), "r");
		int r1 = ref_readline(rf);
		int p1 = P::readline(pf);
		int r2 = ref_readline(rf);
		int p2 = P::readline(pf);
		std::fclose(rf);
		std::fclose(pf);
		if (r1 != p1 || r2 != p2 ||
		    oracle_mknodes_get_linno() != P::port_mknodes_get_linno())
			fail(st, inputs[i]);
		else
			ok(st);
	}
}

static void
test_readline_sweep()
{
	Stat &st = S("readline");
	char buf[256];
	for (long i = 0; i < SWEEP; i++) {
		size_t n = (size_t)(rng.u32() % 95);
		for (size_t j = 0; j < n; j++)
			buf[j] = (char)(rng.u32() & 0xff);
		if (n < 256)
			buf[n++] = '\n';
		buf[n] = '\0';
		reset_both();
		FILE *rf = fmemopen(buf, n, "r");
		FILE *pf = fmemopen(buf, n, "r");
		int rr, pr;
		do {
			rr = ref_readline(rf);
			pr = P::readline(pf);
			if (rr != pr)
				break;
		} while (rr);
		if (rr != pr || oracle_mknodes_get_linno() != P::port_mknodes_get_linno())
			fail(st, "sweep");
		else
			ok(st);
		std::fclose(rf);
		std::fclose(pf);
	}
}

static void
test_indent_hand()
{
	Stat &st = S("indent");
	int amounts[] = { 0, 1, 7, 8, 9, 15, 16, 17, 31, 32, 100, -1 };
	for (int i = 0; amounts[i] >= 0; i++) {
		char *rb = nullptr, *pb = nullptr;
		size_t rl = 0, pl = 0;
		FILE *rf = open_memstream(&rb, &rl);
		FILE *pf = open_memstream(&pb, &pl);
		ref_indent(amounts[i], rf);
		P::indent(amounts[i], pf);
		std::fclose(rf);
		std::fclose(pf);
		if (!cmp_bufs(st, rb, rl, pb, pl, "indent"))
			;
		else
			ok(st);
		std::free(rb);
		std::free(pb);
	}
}

static void
test_indent_sweep()
{
	Stat &st = S("indent");
	for (long i = 0; i < SWEEP; i++) {
		int amt = (int)(rng.u32() % 120) - 10;
		char *rb = nullptr, *pb = nullptr;
		size_t rl = 0, pl = 0;
		FILE *rf = open_memstream(&rb, &rl);
		FILE *pf = open_memstream(&pb, &pl);
		ref_indent(amt, rf);
		P::indent(amt, pf);
		std::fclose(rf);
		std::fclose(pf);
		if (cmp_bufs(st, rb, rl, pb, pl, "sweep"))
			ok(st);
		std::free(rb);
		std::free(pb);
	}
}

static void
write_nodes_input(const char *path)
{
	FILE *f = std::fopen(path, "w");
	std::fprintf(f,
	    "NCMD cmdnode\n"
	    " NCMDargv nodeptr\n"
	    "NPIPE pipenode\n"
	    " NPPIPE nodelist\n"
	    "NREDIR redirnode\n"
	    " NREDIRnodelist nodelist\n"
	    " NREDIRfile string\n"
	    " NREDIRfd int\n"
	    "NBACKGND backgndnode\n"
	    " NBACKGNDsubnode nodeptr\n"
	    "NSUBSHELL subshellnode\n"
	    " NSUBSHELLsub nodeptr\n");
	std::fclose(f);
}

static void
write_pat_file(const char *path, int variant)
{
	FILE *f = std::fopen(path, "w");
	if (variant == 0) {
		std::fputs("%SIZES\n", f);
		std::fputs("/* tail */\n", f);
	} else if (variant == 1) {
		std::fputs("%CALCSIZE\n", f);
	} else {
		std::fputs("%COPY\n", f);
	}
	std::fclose(f);
}

static int
run_main_subprocess(bool port_side, const char *types, const char *pat, char **out_h,
    size_t *out_hl, char **out_c, size_t *out_cl)
{
	pid_t pid = fork();
	if (pid < 0)
		return -1;
	if (pid == 0) {
		char tpath[64], ppath[64];
		std::snprintf(tpath, sizeof(tpath), "%s", types);
		std::snprintf(ppath, sizeof(ppath), "%s", pat);
		char *argv[] = { (char *)"mknodes", tpath, ppath, nullptr };
		if (port_side)
			P::mknodes_main(3, argv);
		else
			ref_mknodes_main(3, argv);
		_exit(0);
	}
	int st = 0;
	waitpid(pid, &st, 0);
	if (!WIFEXITED(st) || WEXITSTATUS(st) != 0)
		return -1;
	FILE *hf = std::fopen("nodes.h", "r");
	FILE *cf = std::fopen("nodes.c", "r");
	if (!hf || !cf)
		return -1;
	std::fseek(hf, 0, SEEK_END);
	*out_hl = (size_t)std::ftell(hf);
	std::fseek(hf, 0, SEEK_SET);
	*out_h = (char *)std::malloc(*out_hl + 1);
	std::fread(*out_h, 1, *out_hl, hf);
	(*out_h)[*out_hl] = '\0';
	std::fclose(hf);
	std::fseek(cf, 0, SEEK_END);
	*out_cl = (size_t)std::ftell(cf);
	std::fseek(cf, 0, SEEK_SET);
	*out_c = (char *)std::malloc(*out_cl + 1);
	std::fread(*out_c, 1, *out_cl, cf);
	(*out_c)[*out_cl] = '\0';
	std::fclose(cf);
	return 0;
}

static void
test_main_hand()
{
	Stat &st = S("mknodes_main");
	const char *types = "/tmp/b0230_types_XXXXXX";
	const char *pat = "/tmp/b0230_pat_XXXXXX";
	char tbuf[] = "/tmp/b0230_types_XXXXXX";
	char pbuf[] = "/tmp/b0230_pat_XXXXXX";
	if (!mkstemp(tbuf) || !mkstemp(pbuf)) {
		fail(st, "mkstemp");
		return;
	}
	write_nodes_input(tbuf);
	for (int v = 0; v < 3; v++) {
		write_pat_file(pbuf, v);
		char *rh = nullptr, *ph = nullptr, *rc = nullptr, *pc = nullptr;
		size_t rhl = 0, phl = 0, rcl = 0, pcl = 0;
		reset_both();
		if (run_main_subprocess(false, tbuf, pbuf, &rh, &rhl, &rc, &rcl) != 0) {
			fail(st, "oracle main");
			std::free(rh);
			std::free(rc);
			continue;
		}
		reset_both();
		if (run_main_subprocess(true, tbuf, pbuf, &ph, &phl, &pc, &pcl) != 0) {
			fail(st, "port main");
			std::free(rh);
			std::free(rc);
			std::free(ph);
			std::free(pc);
			continue;
		}
		if (cmp_bufs(st, rh, rhl, ph, phl, "nodes.h") &&
		    cmp_bufs(st, rc, rcl, pc, pcl, "nodes.c"))
			ok(st);
		std::free(rh);
		std::free(ph);
		std::free(rc);
		std::free(pc);
	}
	::unlink(tbuf);
	::unlink(pbuf);
}

static void
test_main_sweep()
{
	Stat &st = S("mknodes_main");
	char tbuf[] = "/tmp/b0230_sw_t_XXXXXX";
	char pbuf[] = "/tmp/b0230_sw_p_XXXXXX";
	if (!mkstemp(tbuf) || !mkstemp(pbuf))
		return;
	const char *types[] = { "nodeptr", "nodelist", "string", "int" };
	for (long i = 0; i < SWEEP; i++) {
		FILE *f = std::fopen(tbuf, "w");
		int nodes = (int)(rng.u32() % 5) + 1;
		for (int n = 0; n < nodes; n++) {
			char name[16], tag[16];
			std::snprintf(name, sizeof(name), "N%u", (unsigned)rng.u32() % 1000);
			std::snprintf(tag, sizeof(tag), "t%u", (unsigned)rng.u32() % 1000);
			std::fprintf(f, "%s %s\n", name, tag);
			int fields = (int)(rng.u32() % 4);
			for (int k = 0; k < fields; k++)
				std::fprintf(f, " f%u %s\n", k, types[rng.u32() % 4]);
		}
		std::fclose(f);
		write_pat_file(pbuf, (int)(rng.u32() % 3));
		char *rh = nullptr, *ph = nullptr, *rc = nullptr, *pc = nullptr;
		size_t rhl = 0, phl = 0, rcl = 0, pcl = 0;
		reset_both();
		if (run_main_subprocess(false, tbuf, pbuf, &rh, &rhl, &rc, &rcl) != 0) {
			std::free(rh);
			std::free(rc);
			continue;
		}
		reset_both();
		if (run_main_subprocess(true, tbuf, pbuf, &ph, &phl, &pc, &pcl) != 0) {
			fail(st, "port sweep");
			std::free(rh);
			std::free(rc);
			std::free(ph);
			std::free(pc);
			continue;
		}
		if (cmp_bufs(st, rh, rhl, ph, phl, "h") &&
		    cmp_bufs(st, rc, rcl, pc, pcl, "c"))
			ok(st);
		std::free(rh);
		std::free(ph);
		std::free(rc);
		std::free(pc);
	}
	::unlink(tbuf);
	::unlink(pbuf);
}

static int
build_tree_both(const char *const *lines)
{
	for (int i = 0; lines[i]; i++) {
		oracle_mknodes_set_line(lines[i]);
		P::port_mknodes_set_line(lines[i]);
		if (lines[i][0] == ' ' || lines[i][0] == '\t') {
			ref_parsefield();
			P::parsefield();
		} else if (lines[i][0] != '\0') {
			ref_parsenode();
			P::parsenode();
		}
	}
	return 0;
}

static int
run_output_both(const char *patpath, char **rh, size_t *rhl, char **rc, size_t *rcl,
    char **ph, size_t *phl, char **pc, size_t *pcl)
{
	ref_mknodes_output((char *)patpath);
	FILE *hf = std::fopen("nodes.h", "r");
	FILE *cf = std::fopen("nodes.c", "r");
	if (!hf || !cf)
		return -1;
	std::fseek(hf, 0, SEEK_END);
	*rhl = (size_t)std::ftell(hf);
	std::fseek(hf, 0, SEEK_SET);
	*rh = (char *)std::malloc(*rhl + 1);
	std::fread(*rh, 1, *rhl, hf);
	(*rh)[*rhl] = '\0';
	std::fclose(hf);
	std::fseek(cf, 0, SEEK_END);
	*rcl = (size_t)std::ftell(cf);
	std::fseek(cf, 0, SEEK_SET);
	*rc = (char *)std::malloc(*rcl + 1);
	std::fread(*rc, 1, *rcl, cf);
	(*rc)[*rcl] = '\0';
	std::fclose(cf);

	P::mknodes_output((char *)patpath);
	hf = std::fopen("nodes.h", "r");
	cf = std::fopen("nodes.c", "r");
	if (!hf || !cf)
		return -1;
	std::fseek(hf, 0, SEEK_END);
	*phl = (size_t)std::ftell(hf);
	std::fseek(hf, 0, SEEK_SET);
	*ph = (char *)std::malloc(*phl + 1);
	std::fread(*ph, 1, *phl, hf);
	(*ph)[*phl] = '\0';
	std::fclose(hf);
	std::fseek(cf, 0, SEEK_END);
	*pcl = (size_t)std::ftell(cf);
	std::fseek(cf, 0, SEEK_SET);
	*pc = (char *)std::malloc(*pcl + 1);
	std::fread(*pc, 1, *pcl, cf);
	(*pc)[*pcl] = '\0';
	std::fclose(cf);
	return 0;
}

static void
test_parse_output_case(Stat &st_p, Stat &st_f, Stat &st_o, const char *const *lines,
    int patvar)
{
	char pbuf[] = "/tmp/b0230_po_XXXXXX";
	if (!mkstemp(pbuf)) {
		fail(st_o, "mkstemp");
		return;
	}
	write_pat_file(pbuf, patvar);
	reset_both();
	build_tree_both(lines);
	ok(st_p);
	ok(st_f);
	char *rh = nullptr, *ph = nullptr, *rc = nullptr, *pc = nullptr;
	size_t rhl = 0, phl = 0, rcl = 0, pcl = 0;
	if (run_output_both(pbuf, &rh, &rhl, &rc, &rcl, &ph, &phl, &pc, &pcl) != 0) {
		fail(st_o, "output");
	} else if (cmp_bufs(st_o, rh, rhl, ph, phl, "nodes.h") &&
	    cmp_bufs(st_o, rc, rcl, pc, pcl, "nodes.c"))
		ok(st_o);
	std::free(rh);
	std::free(ph);
	std::free(rc);
	std::free(pc);
	::unlink(pbuf);
}

static void
test_parse_output_hand()
{
	Stat &st_p = S("parsenode");
	Stat &st_f = S("parsefield");
	Stat &st_o = S("mknodes_output");
	Stat &st_os = S("outsizes");
	Stat &st_of = S("outfunc");
	const char *basic[] = {
		"NCMD cmdnode",
		" NCMDargv nodeptr",
		" NPIPE pipenode",
		" NPPIPE nodelist",
		nullptr,
	};
	const char *other[] = {
		"NTEST tnode",
		" Nf1 other int custom;",
		" Nf2 temp",
		nullptr,
	};
	test_parse_output_case(st_p, st_f, st_o, basic, 0);
	ok(st_os);
	test_parse_output_case(st_p, st_f, st_o, basic, 1);
	ok(st_of);
	test_parse_output_case(st_p, st_f, st_o, basic, 2);
	ok(st_of);
	test_parse_output_case(st_p, st_f, st_o, other, 0);
}

static void
test_parse_output_sweep()
{
	Stat &st_p = S("parsenode");
	Stat &st_f = S("parsefield");
	Stat &st_o = S("mknodes_output");
	Stat &st_os = S("outsizes");
	Stat &st_of = S("outfunc");
	char line_storage[12][128];
	const char *lines[13];
	for (long i = 0; i < SWEEP / 4; i++) {
		int nlines = (int)(rng.u32() % 6) + 1;
		int li = 0;
		for (int n = 0; n < nlines; n++) {
			if (n == 0) {
				std::snprintf(line_storage[li], sizeof(line_storage[li]),
				    "N%u t%u", (unsigned)rng.u32() % 500,
				    (unsigned)rng.u32() % 500);
			} else {
				const char *ty[] = { "nodeptr", "string", "int", "nodelist" };
				std::snprintf(line_storage[li], sizeof(line_storage[li]),
				    " f%u %s", n, ty[rng.u32() % 4]);
			}
			lines[li] = line_storage[li];
			li++;
		}
		lines[li] = nullptr;
		int patvar = (int)(rng.u32() % 3);
		test_parse_output_case(st_p, st_f, st_o, lines, patvar);
		if (patvar == 0)
			ok(st_os);
		else
			ok(st_of);
	}
}

static void
test_outfunc_direct()
{
	Stat &st = S("outfunc");
	const char *lines[] = { "NX xnode", " Nf nodeptr", nullptr };
	char pbuf[] = "/tmp/b0230_of_XXXXXX";
	if (!mkstemp(pbuf))
		return;
	reset_both();
	build_tree_both(lines);
	char *rb = nullptr, *pb = nullptr;
	size_t rl = 0, pl = 0;
	FILE *rf = open_memstream(&rb, &rl);
	FILE *pf = open_memstream(&pb, &pl);
	ref_outfunc(rf, 1);
	P::outfunc(pf, 1);
	std::fclose(rf);
	std::fclose(pf);
	if (cmp_bufs(st, rb, rl, pb, pl, "calcsize"))
		ok(st);
	std::free(rb);
	std::free(pb);
	::unlink(pbuf);
}

static void
test_outsizes_direct()
{
	Stat &st = S("outsizes");
	const char *lines[] = { "NA anode", " NB bnode", nullptr };
	reset_both();
	build_tree_both(lines);
	char *rb = nullptr, *pb = nullptr;
	size_t rl = 0, pl = 0;
	FILE *rf = open_memstream(&rb, &rl);
	FILE *pf = open_memstream(&pb, &pl);
	ref_outsizes(rf);
	P::outsizes(pf);
	std::fclose(rf);
	std::fclose(pf);
	if (cmp_bufs(st, rb, rl, pb, pl, "outsizes"))
		ok(st);
	std::free(rb);
	std::free(pb);
}

int
main()
{
	std::printf("b0230 differential harness (mknodes.c)\n\n");
	test_savestr_hand();
	test_savestr_sweep();
	test_nextfield_hand();
	test_nextfield_sweep();
	test_skipbl_hand();
	test_skipbl_sweep();
	test_readline_hand();
	test_readline_sweep();
	test_indent_hand();
	test_indent_sweep();
	test_main_hand();
	test_main_sweep();
	test_parse_output_hand();
	test_parse_output_sweep();
	test_outsizes_direct();
	test_outfunc_direct();

	unsigned long long tc = 0, tf = 0;
	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-18s %12llu %12llu\n", stats[i].name, stats[i].cases,
		    stats[i].fails);
		tc += stats[i].cases;
		tf += stats[i].fails;
	}
	std::printf("%-18s %12llu %12llu\n", "TOTAL", tc, tf);
	return tf ? 1 : 0;
}
