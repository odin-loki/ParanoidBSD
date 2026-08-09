/*
 * Differential harness for PBSD batch b0329.
 */

#include <sys/queue.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

import pbsd.lib.libc.iconv.b0329;

namespace P = pbsd::lib_libc_iconv::b0329;

extern "C" {

typedef struct {
	char			mock_modname[64];
	int			(*mock_stdenc_getops)(P::_citrus_stdenc_ops *, size_t);
	int			iconv_open_ret;
	P::_citrus_iconv	*iconv_open_handle;
	int			iconv_convert_ret;
	size_t			iconv_convert_nresults;
	char			**esdb_list;
	size_t			esdb_list_sz;
	bool			esdb_sorted;
	int			esdb_get_list_ret;
	const char		*canonicalize_ret;
	char			**fgetln_lines;
	size_t			*fgetln_lens;
	size_t			fgetln_count;
	size_t			fgetln_idx;
} B0329MockState;

void b0329_mock_reset(void);
void b0329_mock_snap(B0329MockState *);
void b0329_mock_set_stdenc_module(const char *, int (*)(P::_citrus_stdenc_ops *, size_t));

int ref__citrus_NONE_stdenc_init(P::_citrus_stdenc *, const void *, size_t,
    P::_citrus_stdenc_traits *);
void ref__citrus_NONE_stdenc_uninit(P::_citrus_stdenc *);
int ref__citrus_NONE_stdenc_init_state(P::_citrus_stdenc *, void *);
int ref__citrus_NONE_stdenc_mbtocs(P::_citrus_stdenc *, P::_citrus_csid_t *,
    P::_citrus_index_t *, char **, size_t, void *, size_t *, struct iconv_hooks *);
int ref__citrus_NONE_stdenc_cstomb(P::_citrus_stdenc *, char *, size_t,
    P::_citrus_csid_t, P::_citrus_index_t, void *, size_t *, struct iconv_hooks *);
int ref__citrus_NONE_stdenc_mbtowc(P::_citrus_stdenc *, P::_citrus_wc_t *,
    char **, size_t, void *, size_t *, struct iconv_hooks *);
int ref__citrus_NONE_stdenc_wctomb(P::_citrus_stdenc *, char *, size_t,
    P::_citrus_wc_t, void *, size_t *, struct iconv_hooks *);
int ref__citrus_NONE_stdenc_put_state_reset(P::_citrus_stdenc *, char *, size_t,
    void *, size_t *);
int ref__citrus_NONE_stdenc_get_state_desc(P::_citrus_stdenc *, void *, int,
    P::_citrus_stdenc_state_desc *);

int ref__citrus_stdenc_open(P::_citrus_stdenc **, const char *, const void *, size_t);
void ref__citrus_stdenc_close(P::_citrus_stdenc *);

struct src_entry {
	char				*se_name;
	P::_citrus_db_factory		*se_df;
	STAILQ_ENTRY(src_entry)		 se_entry;
};
STAILQ_HEAD(src_head, src_entry);

int ref_find_src(struct src_head *, struct src_entry **, const char *);
void ref_free_src(struct src_head *);
int ref_convert_line(struct src_head *, const char *, size_t);
int ref_dump_db(struct src_head *, P::_citrus_region *);
int ref__citrus_pivot_factory_convert(FILE *, FILE *);

iconv_t ref___bsd___iconv_open(const char *, const char *, P::_citrus_iconv *);
iconv_t ref__bsd_iconv_open(const char *, const char *);
int ref__bsd_iconv_open_into(const char *, const char *, iconv_allocation_t *);
int ref__bsd_iconv_close(iconv_t);
size_t ref__bsd_iconv(iconv_t, char **, size_t *, char **, size_t *);
size_t ref__bsd___iconv(iconv_t, char **, size_t *, char **, size_t *, uint32_t,
    size_t *);
int ref__bsd___iconv_get_list(char ***, size_t *, bool);
void ref__bsd___iconv_free_list(char **, size_t);
int ref_qsort_helper(const void *, const void *);
void ref__bsd_iconvlist(int (*)(unsigned int, const char *const *, void *), void *);
const char *ref__bsd_iconv_canonicalize(const char *);
int ref__bsd_iconvctl(iconv_t, int, void *);
void ref__bsd_iconv_set_relocation_prefix(const char *, const char *);

extern struct _citrus_stdenc_ops _citrus_NONE_stdenc_ops;
extern struct _citrus_stdenc_traits _citrus_NONE_stdenc_traits;
extern P::_citrus_stdenc _citrus_stdenc_default;
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t BIGBUF = 16384;
constexpr unsigned long long RANDOM_ITERS = 30000;

enum FuncId {
	F_NONE_INIT, F_NONE_UNINIT, F_NONE_INIT_STATE, F_NONE_MBTOCS, F_NONE_CSTOMB,
	F_NONE_MBTOWC, F_NONE_WCTOMB, F_NONE_PUT_RESET, F_NONE_GET_DESC,
	F_STDENC_OPEN, F_STDENC_CLOSE,
	F_FIND_SRC, F_FREE_SRC, F_CONVERT_LINE, F_DUMP_DB, F_PIVOT_CONVERT,
	F_ICONV_OPEN_INNER, F_ICONV_OPEN, F_ICONV_OPEN_INTO, F_ICONV_CLOSE,
	F_ICONV, F_ICONV_X, F_ICONV_GET_LIST, F_ICONV_FREE_LIST, F_QSORT_HELPER,
	F_ICONVLIST, F_ICONV_CANON, F_ICONVCTL, F_ICONV_RELOC,
	F_COUNT
};

const char *const FNAME[F_COUNT] = {
	"NONE_stdenc_init", "NONE_stdenc_uninit", "NONE_stdenc_init_state",
	"NONE_stdenc_mbtocs", "NONE_stdenc_cstomb", "NONE_stdenc_mbtowc",
	"NONE_stdenc_wctomb", "NONE_stdenc_put_state_reset",
	"NONE_stdenc_get_state_desc",
	"stdenc_open", "stdenc_close",
	"find_src", "free_src", "convert_line", "dump_db", "pivot_factory_convert",
	"__bsd___iconv_open", "__bsd_iconv_open", "__bsd_iconv_open_into",
	"__bsd_iconv_close", "__bsd_iconv", "__bsd___iconv",
	"__bsd___iconv_get_list", "__bsd___iconv_free_list", "qsort_helper",
	"__bsd_iconvlist", "__bsd_iconv_canonicalize", "__bsd_iconvctl",
	"__bsd_iconv_set_relocation_prefix",
};

unsigned long long NCASE[F_COUNT];
unsigned long long NFAIL[F_COUNT];
unsigned long long NPRINT[F_COUNT];

uint64_t rng = 0xb0329b0329b0329ULL;

uint64_t rng_next(void)
{
	uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

unsigned char rb(void) { return (unsigned char)(rng_next() & 0xffu); }

void bump(FuncId f) { NCASE[f]++; }

void fail(FuncId f, const char *why)
{
	NFAIL[f]++;
	if (NPRINT[f]++ < 8)
		std::fprintf(stderr, "  FAIL %-34s : %s\n", FNAME[f], why);
}

static P::_citrus_stdenc_ops g_stdenc_ops;
static int g_stdenc_init_calls;

static int mock_stdenc_getops(P::_citrus_stdenc_ops *ops, size_t sz)
{
	if (sz < sizeof(*ops))
		return (EINVAL);
	*ops = g_stdenc_ops;
	return (0);
}

static void setup_stdenc_ops(void)
{
	g_stdenc_ops.eo_init = ref__citrus_NONE_stdenc_init;
	g_stdenc_ops.eo_uninit = ref__citrus_NONE_stdenc_uninit;
	g_stdenc_ops.eo_init_state = ref__citrus_NONE_stdenc_init_state;
	g_stdenc_ops.eo_mbtocs = ref__citrus_NONE_stdenc_mbtocs;
	g_stdenc_ops.eo_cstomb = ref__citrus_NONE_stdenc_cstomb;
	g_stdenc_ops.eo_mbtowc = ref__citrus_NONE_stdenc_mbtowc;
	g_stdenc_ops.eo_wctomb = ref__citrus_NONE_stdenc_wctomb;
	g_stdenc_ops.eo_put_state_reset = ref__citrus_NONE_stdenc_put_state_reset;
	g_stdenc_ops.eo_get_state_desc = ref__citrus_NONE_stdenc_get_state_desc;
}

static int mock_stdenc_init(P::_citrus_stdenc *ce, const void *, size_t,
    P::_citrus_stdenc_traits *tr)
{
	g_stdenc_init_calls++;
	tr->et_state_size = 0;
	tr->et_mb_cur_max = 1;
	ce->ce_closure = nullptr;
	return (0);
}

static P::_citrus_iconv *make_mock_iconv(const char *conv)
{
	auto *sh = (P::_citrus_iconv_shared *)std::calloc(1, sizeof(*sh));
	auto *cv = (P::_citrus_iconv *)std::calloc(1, sizeof(*cv));
	auto *ops = (P::_citrus_iconv_ops *)std::calloc(1, sizeof(*ops));
	char *cn = (char *)std::malloc(std::strlen(conv) + 1);
	std::strcpy(cn, conv);
	sh->ci_ops = ops;
	sh->ci_convname = cn;
	cv->cv_shared = sh;
	return (cv);
}

static void free_mock_iconv(P::_citrus_iconv *cv)
{
	if (!cv)
		return;
	std::free(cv->cv_shared->ci_convname);
	std::free(cv->cv_shared->ci_ops);
	std::free(cv->cv_shared);
	std::free(cv);
}

static void setup_fgetln(const std::vector<std::string> &lines)
{
	static std::vector<std::string> storage;
	static std::vector<char *> ptrs;
	static std::vector<size_t> lens;
	storage = lines;
	ptrs.clear();
	lens.clear();
	for (auto &s : storage) {
		ptrs.push_back(s.data());
		lens.push_back(s.size());
	}
	b0329_mock_reset();
	B0329MockState st{};
	st.fgetln_lines = ptrs.data();
	st.fgetln_lens = lens.data();
	st.fgetln_count = ptrs.size();
	st.fgetln_idx = 0;
	b0329_mock_snap(&st);
	// restore via direct assignment through snap copy back - use mock state setter
	extern B0329MockState b0329_mock_state;
	b0329_mock_state = st;
}

void test_none_stdenc_ops(void)
{
	P::_citrus_stdenc ce{};
	P::_citrus_stdenc_traits tr{};
	char buf[8];
	char *p;
	size_t nres;
	P::_citrus_csid_t csid;
	P::_citrus_index_t idx;
	P::_citrus_wc_t wc;
	P::_citrus_stdenc_state_desc sd{};
	unsigned hook_uc = 0, hook_wc = 0;
	struct iconv_hooks hooks{};

	hooks.uc_hook = [](unsigned int c, void *d) {
		*(unsigned *)d = c;
	};
	hooks.wc_hook = [](wchar_t c, void *d) {
		*(unsigned *)d = (unsigned)c;
	};
	hooks.data = &hook_uc;

	bump(F_NONE_INIT);
	int rp = P::_citrus_NONE_stdenc_init(&ce, nullptr, 0, &tr);
	int rr = ref__citrus_NONE_stdenc_init(&ce, nullptr, 0, &tr);
	if (rp != rr || tr.et_state_size != 0 || tr.et_mb_cur_max != 1)
		fail(F_NONE_INIT, "init");

	bump(F_NONE_UNINIT);
	P::_citrus_NONE_stdenc_uninit(&ce);
	ref__citrus_NONE_stdenc_uninit(&ce);

	bump(F_NONE_INIT_STATE);
	rp = P::_citrus_NONE_stdenc_init_state(&ce, nullptr);
	rr = ref__citrus_NONE_stdenc_init_state(&ce, nullptr);
	if (rp != rr)
		fail(F_NONE_INIT_STATE, "init_state");

	p = buf;
	buf[0] = 'A';
	bump(F_NONE_MBTOCS);
	rp = P::_citrus_NONE_stdenc_mbtocs(&ce, &csid, &idx, &p, 1, nullptr, &nres, &hooks);
	rr = ref__citrus_NONE_stdenc_mbtocs(&ce, &csid, &idx, &p, 1, nullptr, &nres, &hooks);
	if (rp != rr || csid != 0 || idx != 'A' || nres != 1 || hook_uc != 'A')
		fail(F_NONE_MBTOCS, "mbtocs");

	bump(F_NONE_CSTOMB);
	nres = 0;
	rp = P::_citrus_NONE_stdenc_cstomb(&ce, buf, 4, 0, 0x42, nullptr, &nres, nullptr);
	rr = ref__citrus_NONE_stdenc_cstomb(&ce, buf, 4, 0, 0x42, nullptr, &nres, nullptr);
	if (rp != rr || buf[0] != 'B' || nres != 1)
		fail(F_NONE_CSTOMB, "cstomb1");
	rp = P::_citrus_NONE_stdenc_cstomb(&ce, buf, 4, 0, 0x12345, nullptr, &nres, nullptr);
	rr = ref__citrus_NONE_stdenc_cstomb(&ce, buf, 4, 0, 0x12345, nullptr, &nres, nullptr);
	if (rp != rr || nres != 4)
		fail(F_NONE_CSTOMB, "cstomb4");

	p = buf;
	buf[0] = 'Z';
	hook_wc = 0;
	hooks.wc_hook = [](wchar_t c, void *d) { *(unsigned *)d = (unsigned)c; };
	hooks.data = &hook_wc;
	bump(F_NONE_MBTOWC);
	rp = P::_citrus_NONE_stdenc_mbtowc(&ce, &wc, &p, 1, nullptr, &nres, &hooks);
	rr = ref__citrus_NONE_stdenc_mbtowc(&ce, &wc, &p, 1, nullptr, &nres, &hooks);
	if (rp != rr || wc != 'Z' || hook_wc != 'Z')
		fail(F_NONE_MBTOWC, "mbtowc");

	bump(F_NONE_WCTOMB);
	rp = P::_citrus_NONE_stdenc_wctomb(&ce, buf, 4, 'x', nullptr, &nres, nullptr);
	rr = ref__citrus_NONE_stdenc_wctomb(&ce, buf, 4, 'x', nullptr, &nres, nullptr);
	if (rp != rr || buf[0] != 'x')
		fail(F_NONE_WCTOMB, "wctomb");
	rp = P::_citrus_NONE_stdenc_wctomb(&ce, buf, 4, 0x100, nullptr, &nres, nullptr);
	rr = ref__citrus_NONE_stdenc_wctomb(&ce, buf, 4, 0x100, nullptr, &nres, nullptr);
	if (rp != rr || rp != EILSEQ)
		fail(F_NONE_WCTOMB, "wctomb_ilseq");

	bump(F_NONE_PUT_RESET);
	rp = P::_citrus_NONE_stdenc_put_state_reset(&ce, buf, 4, nullptr, &nres);
	rr = ref__citrus_NONE_stdenc_put_state_reset(&ce, buf, 4, nullptr, &nres);
	if (rp != rr || nres != 0)
		fail(F_NONE_PUT_RESET, "put_reset");

	bump(F_NONE_GET_DESC);
	rp = P::_citrus_NONE_stdenc_get_state_desc(&ce, nullptr, 0, &sd);
	rr = ref__citrus_NONE_stdenc_get_state_desc(&ce, nullptr, 0, &sd);
	if (rp != rr || sd.u.generic.state != 1)
		fail(F_NONE_GET_DESC, "get_desc");
	rp = P::_citrus_NONE_stdenc_get_state_desc(&ce, nullptr, 99, &sd);
	rr = ref__citrus_NONE_stdenc_get_state_desc(&ce, nullptr, 99, &sd);
	if (rp != rr || rp != EOPNOTSUPP)
		fail(F_NONE_GET_DESC, "get_desc_bad");
}

void test_stdenc_open_close(void)
{
	P::_citrus_stdenc *pa = nullptr, *pb = nullptr;
	int ra, rbv;

	bump(F_STDENC_OPEN);
	ra = P::_citrus_stdenc_open(&pa, "NONE", nullptr, 0);
	rbv = ref__citrus_stdenc_open(&pb, "NONE", nullptr, 0);
	if (ra != rbv || pa != &_citrus_stdenc_default || pb != &_citrus_stdenc_default)
		fail(F_STDENC_OPEN, "NONE default");

	b0329_mock_reset();
	b0329_mock_set_stdenc_module("TESTENC", mock_stdenc_getops);
	setup_stdenc_ops();
	g_stdenc_ops.eo_init = mock_stdenc_init;
	g_stdenc_init_calls = 0;
	ra = P::_citrus_stdenc_open(&pa, "TESTENC", nullptr, 0);
	int port_calls = g_stdenc_init_calls;
	g_stdenc_init_calls = 0;
	rbv = ref__citrus_stdenc_open(&pb, "TESTENC", nullptr, 0);
	if (ra != rbv || port_calls != 1 || g_stdenc_init_calls != 1)
		fail(F_STDENC_OPEN, "module open");

	bump(F_STDENC_CLOSE);
	P::_citrus_stdenc_close(pa);
	ref__citrus_stdenc_close(pb);
}

void test_pivot_statics(void)
{
	src_head sha{}, shb{};
	struct src_entry *ea = nullptr, *eb = nullptr;
	int ra, rbv;

	STAILQ_INIT(&sha);
	STAILQ_INIT(&shb);

	bump(F_FIND_SRC);
	ra = P::find_src(&sha, &ea, "UTF8");
	rbv = ref_find_src(&shb, &eb, "UTF8");
	if (ra != rbv || !ea || !eb)
		fail(F_FIND_SRC, "find_src1");
	ra = P::find_src(&sha, &ea, "utf8");
	rbv = ref_find_src(&shb, &eb, "utf8");
	if (ra != rbv || ea != STAILQ_FIRST(&sha))
		fail(F_FIND_SRC, "find_src_case");

	bump(F_CONVERT_LINE);
	ra = P::convert_line(&sha, "UTF8\tA\t0x41\n", 13);
	rbv = ref_convert_line(&shb, "UTF8\tA\t0x41\n", 13);
	if (ra != rbv)
		fail(F_CONVERT_LINE, "convert");
	ra = P::convert_line(&sha, "  \n", 3);
	rbv = ref_convert_line(&shb, "  \n", 3);
	if (ra != rbv)
		fail(F_CONVERT_LINE, "blank");
	ra = P::convert_line(&sha, "UTF8\tB\tbad\n", 12);
	rbv = ref_convert_line(&shb, "UTF8\tB\tbad\n", 12);
	if (ra != rbv || ra != EFTYPE)
		fail(F_CONVERT_LINE, "badval");

	P::_citrus_region ra_r{}, rb_r{};
	unsigned char ba[BIGBUF], bb[BIGBUF];
	std::memset(ba, GUARD, sizeof(ba));
	std::memset(bb, GUARD, sizeof(bb));
	P::_citrus_region_init(&ra_r, ba + 64, sizeof(ba) - 128);
	P::_citrus_region_init(&rb_r, bb + 64, sizeof(bb) - 128);

	bump(F_DUMP_DB);
	ra = P::dump_db(&sha, &ra_r);
	rbv = ref_dump_db(&shb, &rb_r);
	if (ra != rbv || P::_citrus_region_size(&ra_r) != P::_citrus_region_size(&rb_r) ||
	    std::memcmp(P::_citrus_region_head(&ra_r), P::_citrus_region_head(&rb_r),
	    P::_citrus_region_size(&ra_r)) != 0)
		fail(F_DUMP_DB, "dump_db");

	bump(F_FREE_SRC);
	P::free_src(&sha);
	ref_free_src(&shb);
}

void test_pivot_convert(void)
{
	setup_fgetln({
	    "UTF8\tA\t65\n",
	    "# comment\n",
	    "LATIN1\tB\t0x42\n",
	});
	unsigned char outa[4096], outb[4096];
	std::memset(outa, GUARD, sizeof(outa));
	std::memset(outb, GUARD, sizeof(outb));
	FILE *fa = fmemopen(outa, sizeof(outa), "w");
	FILE *fb = fmemopen(outb, sizeof(outb), "w");
	FILE *ia = tmpfile();
	FILE *ib = tmpfile();
	if (!fa || !fb || !ia || !ib) {
		fail(F_PIVOT_CONVERT, "tmpfile");
		return;
	}
	bump(F_PIVOT_CONVERT);
	int ra = P::_citrus_pivot_factory_convert(fa, ia);
	setup_fgetln({
	    "UTF8\tA\t65\n",
	    "# comment\n",
	    "LATIN1\tB\t0x42\n",
	});
	int rbv = ref__citrus_pivot_factory_convert(fb, ib);
	if (ra != rbv)
		fail(F_PIVOT_CONVERT, "ret");
	std::fflush(fa);
	std::fflush(fb);
	long sa = std::ftell(fa), sb = std::ftell(fb);
	if (sa != sb || sa <= 0 ||
	    std::memcmp(outa, outb, (size_t)sa) != 0)
		fail(F_PIVOT_CONVERT, "bytes");
	std::fclose(fa);
	std::fclose(fb);
	std::fclose(ia);
	std::fclose(ib);
}

void test_bsd_iconv(void)
{
	b0329_mock_reset();
	P::_citrus_iconv *cv = make_mock_iconv("utf8/utf8");
	B0329MockState st{};
	st.iconv_open_handle = cv;
	st.iconv_convert_ret = 0;
	st.iconv_convert_nresults = 7;
	b0329_mock_snap(&st);
	extern B0329MockState b0329_mock_state;
	b0329_mock_state = st;

	bump(F_ICONV_OPEN);
	iconv_t pa = P::__bsd_iconv_open("utf8", "utf8");
	iconv_t pb = ref__bsd_iconv_open("utf8", "utf8");
	if ((pa == (iconv_t)-1) != (pb == (iconv_t)-1))
		fail(F_ICONV_OPEN, "open");

	bump(F_ICONV_OPEN_INNER);
	P::_citrus_iconv handle{};
	pa = P::__bsd___iconv_open("utf8", "ascii", &handle);
	pb = ref___bsd___iconv_open("utf8", "ascii", &handle);
	if ((pa == (iconv_t)-1) != (pb == (iconv_t)-1))
		fail(F_ICONV_OPEN_INNER, "inner");

	iconv_allocation_t alloc{};
	bump(F_ICONV_OPEN_INTO);
	int ra = P::__bsd_iconv_open_into("utf8", "ascii", &alloc);
	int rbv = ref__bsd_iconv_open_into("utf8", "ascii", &alloc);
	if (ra != rbv)
		fail(F_ICONV_OPEN_INTO, "open_into");

	char inbuf[] = "hi", outbuf[8];
	char *pin = inbuf, *pout = outbuf;
	size_t sin = 2, sout = 8;

	bump(F_ICONV);
	b0329_mock_state.iconv_convert_nresults = 2;
	size_t ca = P::__bsd_iconv((iconv_t)cv, &pin, &sin, &pout, &sout);
	b0329_mock_state.iconv_convert_nresults = 2;
	size_t cb = ref__bsd_iconv((iconv_t)cv, &pin, &sin, &pout, &sout);
	if (ca != cb)
		fail(F_ICONV, "iconv");

	bump(F_ICONV_X);
	size_t inv = 99;
	ca = P::__bsd___iconv((iconv_t)cv, &pin, &sin, &pout, &sout, 0, &inv);
	inv = 99;
	cb = ref__bsd___iconv((iconv_t)cv, &pin, &sin, &pout, &sout, 0, &inv);
	if (ca != cb)
		fail(F_ICONV_X, "__iconv");

	bump(F_ICONV_CLOSE);
	ra = P::__bsd_iconv_close((iconv_t)cv);
	rbv = ref__bsd_iconv_close((iconv_t)cv);
	if (ra != rbv)
		fail(F_ICONV_CLOSE, "close");
	ra = P::__bsd_iconv_close((iconv_t)-1);
	rbv = ref__bsd_iconv_close((iconv_t)-1);
	if (ra != rbv || ra != -1)
		fail(F_ICONV_CLOSE, "badfd");

	static char *elist[] = {
		(char *)"utf8/utf8",
		(char *)"utf8/ascii",
		(char *)"ascii/utf8",
	};
	b0329_mock_reset();
	st = {};
	st.esdb_list = elist;
	st.esdb_list_sz = 3;
	b0329_mock_state = st;

	bump(F_ICONV_GET_LIST);
	char **la = nullptr, **lb = nullptr;
	size_t sa = 0, sb = 0;
	ra = P::__bsd___iconv_get_list(&la, &sa, true);
	rbv = ref__bsd___iconv_get_list(&lb, &sb, true);
	if (ra != rbv || sa != sb)
		fail(F_ICONV_GET_LIST, "get_list");

	bump(F_ICONV_FREE_LIST);
	P::__bsd___iconv_free_list(la, sa);
	ref__bsd___iconv_free_list(lb, sb);

	bump(F_QSORT_HELPER);
	const char *a = "beta", *b = "alpha";
	ra = P::qsort_helper(&a, &b);
	rbv = ref_qsort_helper(&a, &b);
	if (ra != rbv)
		fail(F_QSORT_HELPER, "qsort_helper");

	b0329_mock_state = st;
	bump(F_ICONVLIST);
	int cb_count = 0;
	auto cb = [](unsigned int n, const char *const *names, void *) -> int {
		(void)n;
		(void)names;
		return (0);
	};
	P::__bsd_iconvlist(+[](unsigned int n, const char *const *names, void *d) -> int {
		*(int *)d = (int)n;
		return (0);
	}, &cb_count);
	int cb_count2 = -1;
	ref__bsd_iconvlist(+[](unsigned int n, const char *const *names, void *d) -> int {
		*(int *)d = (int)n;
		return (0);
	}, &cb_count2);
	if (cb_count != cb_count2)
		fail(F_ICONVLIST, "iconvlist");

	b0329_mock_reset();
	st = {};
	st.canonicalize_ret = "CANON";
	b0329_mock_state = st;
	bump(F_ICONV_CANON);
	const char *cpa = P::__bsd_iconv_canonicalize("utf8");
	const char *cpb = ref__bsd_iconv_canonicalize("utf8");
	if (cpa != cpb)
		fail(F_ICONV_CANON, "canonicalize");

	cv = make_mock_iconv("ab/ab");
	b0329_mock_state.iconv_open_handle = cv;
	int val = 0;
	bump(F_ICONVCTL);
	ra = P::__bsd_iconvctl((iconv_t)cv, ICONV_TRIVIALP, &val);
	rbv = ref__bsd_iconvctl((iconv_t)cv, ICONV_TRIVIALP, &val);
	if (ra != rbv || val != 1)
		fail(F_ICONVCTL, "trivialp");
	ra = P::__bsd_iconvctl((iconv_t)-1, ICONV_TRIVIALP, &val);
	rbv = ref__bsd_iconvctl((iconv_t)-1, ICONV_TRIVIALP, &val);
	if (ra != rbv || ra != -1)
		fail(F_ICONVCTL, "badfd");

	bump(F_ICONV_RELOC);
	P::__bsd_iconv_set_relocation_prefix("a", "b");
	ref__bsd_iconv_set_relocation_prefix("a", "b");

	free_mock_iconv(cv);
}

void random_none_mbtocs(unsigned n)
{
	P::_citrus_stdenc ce{};
	char buf[16];
	for (unsigned i = 0; i < n; i++) {
		unsigned len = (unsigned)(rng_next() % 8u) + 1;
		for (unsigned j = 0; j < len; j++)
			buf[j] = (char)rb();
		char *pp = buf, *pr = buf;
		size_t np = 0, nr = 0;
		P::_citrus_csid_t csp = 0, csr = 0;
		P::_citrus_index_t ip = 0, ir = 0;
		size_t avail = (unsigned)(rng_next() % 9u);
		bump(F_NONE_MBTOCS);
		int rp = P::_citrus_NONE_stdenc_mbtocs(&ce, &csp, &ip, &pp, avail, nullptr, &np, nullptr);
		int rr = ref__citrus_NONE_stdenc_mbtocs(&ce, &csr, &ir, &pr, avail, nullptr, &nr, nullptr);
		if (rp != rr || csp != csr || ip != ir || np != nr)
			fail(F_NONE_MBTOCS, "rand");
	}
}

void random_convert_line(unsigned n)
{
	for (unsigned i = 0; i < n; i++) {
		src_head sp{}, sr{};
		STAILQ_INIT(&sp);
		STAILQ_INIT(&sr);
		char line[128];
		unsigned k1 = (unsigned)(rng_next() % 8u) + 1;
		unsigned k2 = (unsigned)(rng_next() % 8u) + 1;
		unsigned pos = 0;
		for (unsigned j = 0; j < k1; j++)
			line[pos++] = (char)('a' + rb() % 26);
		line[pos++] = '\t';
		for (unsigned j = 0; j < k2; j++)
			line[pos++] = (char)('A' + rb() % 26);
		line[pos++] = '\t';
		line[pos++] = '0';
		line[pos++] = 'x';
		unsigned val = (unsigned)(rng_next() & 0xffu);
		std::snprintf(line + pos, sizeof(line) - pos, "%x\n", val);
		pos = std::strlen(line);
		bump(F_CONVERT_LINE);
		int rp = P::convert_line(&sp, line, pos);
		int rr = ref_convert_line(&sr, line, pos);
		if (rp != rr) {
			fail(F_CONVERT_LINE, "rand");
		} else if (rp == 0) {
			struct src_entry *ep, *er;
			P::find_src(&sp, &ep, line);
			ref_find_src(&sr, &er, line);
		}
		P::free_src(&sp);
		ref_free_src(&sr);
	}
}

void random_qsort_helper(unsigned n)
{
	const char *strings[8];
	char pool[8][16];
	for (unsigned i = 0; i < n; i++) {
		unsigned cnt = (unsigned)(rng_next() % 8u) + 1;
		for (unsigned j = 0; j < cnt; j++) {
			unsigned l = (unsigned)(rng_next() % 8u) + 1;
			for (unsigned k = 0; k < l; k++)
				pool[j][k] = (char)('a' + rb() % 26);
			pool[j][l] = '\0';
			strings[j] = pool[j];
		}
		const void *ap = strings, *bp = strings;
		if (cnt > 1) {
			ap = &strings[0];
			bp = &strings[1];
		}
		bump(F_QSORT_HELPER);
		int rp = P::qsort_helper(ap, bp);
		int rr = ref_qsort_helper(ap, bp);
		if (rp != rr)
			fail(F_QSORT_HELPER, "rand");
	}
}

} // namespace

int main(void)
{
	test_none_stdenc_ops();
	test_stdenc_open_close();
	test_pivot_statics();
	test_pivot_convert();
	test_bsd_iconv();

	random_none_mbtocs(RANDOM_ITERS);
	random_convert_line(RANDOM_ITERS);
	random_qsort_helper(RANDOM_ITERS);

	std::printf("\n%-34s %12s %12s\n", "function", "cases", "failures");
	unsigned long long tc = 0, tf = 0;
	for (int i = 0; i < F_COUNT; i++) {
		if (NCASE[i] || NFAIL[i])
			std::printf("%-34s %12llu %12llu\n", FNAME[i],
			    (unsigned long long)NCASE[i],
			    (unsigned long long)NFAIL[i]);
		tc += NCASE[i];
		tf += NFAIL[i];
	}
	std::printf("%-34s %12llu %12llu\n", "TOTAL", tc, tf);
	return (tf ? 1 : 0);
}
