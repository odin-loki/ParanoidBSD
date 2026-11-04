// PBSD batch b0148s4 -- differential test: port (module) vs. ref_ oracle.
//
// Every routine of bin/ed/buf.c is driven with hand written edge cases and a
// fixed seed random sweep.  Both implementations keep their own file scope
// state (scratch file, buffer_head, ctab, the get_addressed_line_node
// statics), so the two worlds are stepped in lockstep and *all* observable
// state is compared after every single call.

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

import pbsd.bin.ed.b0148s4;

namespace P = pbsd::bin_ed::b0148s4;

#define REF_ERR (-2)

extern "C" {

struct ref_line {
	struct ref_line	*q_forw;
	struct ref_line	*q_back;
	off_t		seek;
	int		len;
};
typedef struct ref_line RLine;

char *ref_get_sbuf_line(RLine *);
const char *ref_put_sbuf_line(const char *);
void ref_add_line_node(RLine *);
long ref_get_line_node_addr(RLine *);
RLine *ref_get_addressed_line_node(long);
int ref_open_sbuf(void);
int ref_close_sbuf(void);
void ref_quit(int);
void ref_init_buffers(void);
char *ref_translit_text(char *, int, int, int);

RLine *ref_bh_ptr(void);
unsigned char *ref_ctab_ptr(void);
char *ref_sfn_ptr(void);
FILE *ref_sfp_ptr(void);
off_t ref_sfseek_val(void);
int ref_seek_write_val(void);

extern int isbinary;
extern int newline_added;
extern int mutex;
extern int sigflags;
extern long addr_last;
extern long current_addr;
extern const char *errmsg;
extern long hup_calls;
extern long int_calls;

} /* extern "C" */

/* ------------------------------------------------------------------ stats */

enum {
	F_TRANSLIT = 0,
	F_GET_LINE_NODE_ADDR,
	F_GET_ADDRESSED,
	F_ADD_LINE_NODE,
	F_OPEN_SBUF,
	F_CLOSE_SBUF,
	F_PUT_SBUF_LINE,
	F_GET_SBUF_LINE,
	F_QUIT,
	F_INIT_BUFFERS,
	NFUNC
};

static const char *fname[NFUNC] = {
	"translit_text",
	"get_line_node_addr",
	"get_addressed_line_node",
	"add_line_node",
	"open_sbuf",
	"close_sbuf",
	"put_sbuf_line",
	"get_sbuf_line",
	"quit",
	"init_buffers"
};

static long ncase[NFUNC];
static long nfail[NFUNC];
static long nshown[NFUNC];

static void
rec(int f, int ok, const char *what, long ix)
{
	ncase[f]++;
	if (!ok) {
		nfail[f]++;
		if (nshown[f] < 8) {
			nshown[f]++;
			printf("  MISMATCH %-24s %s (#%ld)\n", fname[f], what, ix);
		}
	}
}

/* -------------------------------------------------------------------- rng */

static unsigned long long rs = 0x0123456789abcdefULL;

static unsigned long long
rnd64(void)
{
	rs ^= rs << 13;
	rs ^= rs >> 7;
	rs ^= rs << 17;
	return rs;
}

static unsigned
rnd32(void)
{
	return (unsigned)(rnd64() >> 16);
}

static long
rrange(long lo, long hi)
{
	return lo + (long)(rnd64() % (unsigned long long)(hi - lo + 1));
}

static unsigned char
rbyte(void)
{
	unsigned r = rnd32();

	switch (r & 15) {
	case 0: return 0x00;
	case 1: return 0xff;
	case 2: return 0x80;
	case 3: return 0x7f;
	case 4: return 0x01;
	case 5: return 0xfe;
	case 6: return (unsigned char)'\n';
	case 7: return 0x81;
	default: return (unsigned char)(r >> 9);
	}
}

static unsigned char
rbyte_nonl(void)
{
	unsigned char c = rbyte();

	return c == (unsigned char)'\n' ? (unsigned char)0xaa : c;
}

/* ------------------------------------------------------------ comparisons */

static void
clear_err(void)
{
	P::errmsg = "";
	errmsg = "";
}

static int
err_same(void)
{
	return strcmp(P::errmsg, errmsg) == 0;
}

static void
set_globals(long al, long ca, int sf)
{
	P::addr_last = al;	addr_last = al;
	P::current_addr = ca;	current_addr = ca;
	P::sigflags = sf;	sigflags = sf;
	P::mutex = 0;		mutex = 0;
	P::hup_calls = 0;	hup_calls = 0;
	P::int_calls = 0;	int_calls = 0;
}

static int
globals_same(void)
{
	return P::addr_last == addr_last &&
	    P::current_addr == current_addr &&
	    P::mutex == mutex &&
	    P::hup_calls == hup_calls &&
	    P::int_calls == int_calls &&
	    P::isbinary == isbinary &&
	    P::newline_added == newline_added &&
	    P::sigflags == sigflags;
}

static int
sf_same(void)
{
	return P::sfseek_val() == ref_sfseek_val() &&
	    P::seek_write_val() == ref_seek_write_val();
}

/* ------------------------------------------------------------ node arrays */

enum { MAXN = 40 };

static P::line_t pn[MAXN];
static RLine rn[MAXN];

static long
pos_p(P::line_t *x)
{
	if (x == P::bh_ptr())
		return -1;
	if (x >= &pn[0] && x < &pn[MAXN])
		return (long)(x - &pn[0]);
	return -1000;
}

static long
pos_r(RLine *x)
{
	if (x == ref_bh_ptr())
		return -1;
	if (x >= &rn[0] && x < &rn[MAXN])
		return (long)(x - &rn[0]);
	return -1000;
}

#define FNV_OFF 1469598103934665603ULL
#define FNV_PRM 1099511628211ULL

static unsigned long long
mix(unsigned long long h, unsigned long long v)
{
	h ^= v;
	h *= FNV_PRM;
	return h;
}

/* Hash of the circular list by node identity (static node arrays). */
static unsigned long long
hash_pos_p(void)
{
	unsigned long long h = FNV_OFF;
	P::line_t *b = P::bh_ptr();
	P::line_t *p;
	int cnt;

	for (p = b->q_forw, cnt = 0; p != b && cnt < MAXN + 4; p = p->q_forw, cnt++)
		h = mix(h, (unsigned long long)pos_p(p));
	h = mix(h, (unsigned long long)cnt ^ 0xa5a5ULL);
	for (p = b->q_back, cnt = 0; p != b && cnt < MAXN + 4; p = p->q_back, cnt++)
		h = mix(h, (unsigned long long)pos_p(p) * 3 + 17);
	h = mix(h, (unsigned long long)cnt ^ 0x5a5aULL);
	return h;
}

static unsigned long long
hash_pos_r(void)
{
	unsigned long long h = FNV_OFF;
	RLine *b = ref_bh_ptr();
	RLine *p;
	int cnt;

	for (p = b->q_forw, cnt = 0; p != b && cnt < MAXN + 4; p = p->q_forw, cnt++)
		h = mix(h, (unsigned long long)pos_r(p));
	h = mix(h, (unsigned long long)cnt ^ 0xa5a5ULL);
	for (p = b->q_back, cnt = 0; p != b && cnt < MAXN + 4; p = p->q_back, cnt++)
		h = mix(h, (unsigned long long)pos_r(p) * 3 + 17);
	h = mix(h, (unsigned long long)cnt ^ 0x5a5aULL);
	return h;
}

/* Hash of the circular list by node payload (works for malloc'd nodes). */
static unsigned long long
hash_val_p(void)
{
	unsigned long long h = FNV_OFF;
	P::line_t *b = P::bh_ptr();
	P::line_t *p;
	int cnt;

	for (p = b->q_forw, cnt = 0; p != b && cnt < 8192; p = p->q_forw, cnt++) {
		h = mix(h, (unsigned long long)(long long)p->len);
		h = mix(h, (unsigned long long)(long long)p->seek);
	}
	h = mix(h, (unsigned long long)cnt ^ 0xc3c3ULL);
	for (p = b->q_back, cnt = 0; p != b && cnt < 8192; p = p->q_back, cnt++) {
		h = mix(h, (unsigned long long)(long long)p->len * 5 + 3);
		h = mix(h, (unsigned long long)(long long)p->seek * 7 + 11);
	}
	h = mix(h, (unsigned long long)cnt ^ 0x3c3cULL);
	return h;
}

static unsigned long long
hash_val_r(void)
{
	unsigned long long h = FNV_OFF;
	RLine *b = ref_bh_ptr();
	RLine *p;
	int cnt;

	for (p = b->q_forw, cnt = 0; p != b && cnt < 8192; p = p->q_forw, cnt++) {
		h = mix(h, (unsigned long long)(long long)p->len);
		h = mix(h, (unsigned long long)(long long)p->seek);
	}
	h = mix(h, (unsigned long long)cnt ^ 0xc3c3ULL);
	for (p = b->q_back, cnt = 0; p != b && cnt < 8192; p = p->q_back, cnt++) {
		h = mix(h, (unsigned long long)(long long)p->len * 5 + 3);
		h = mix(h, (unsigned long long)(long long)p->seek * 7 + 11);
	}
	h = mix(h, (unsigned long long)cnt ^ 0x3c3cULL);
	return h;
}

/*
 * Bring the get_addressed_line_node() statics back to (&buffer_head, 0).
 * Must be called while the current list is still intact: with on >= 2 the
 * routine resets lp itself, with on == 1 it steps back onto buffer_head.
 */
static void
reset_gal(void)
{
	P::get_addressed_line_node(0);
	ref_get_addressed_line_node(0);
	P::get_addressed_line_node(0);
	ref_get_addressed_line_node(0);
}

static void
empty_list(void)
{
	P::line_t *pb = P::bh_ptr();
	RLine *rb = ref_bh_ptr();

	pb->q_forw = pb;
	pb->q_back = pb;
	rb->q_forw = rb;
	rb->q_back = rb;
}

/* Link pn[0..k) / rn[0..k) into buffer_head's circular queue. */
static void
build_list(int k)
{
	P::line_t *pb = P::bh_ptr();
	RLine *rb = ref_bh_ptr();
	int i;

	reset_gal();
	if (k <= 0) {
		empty_list();
		return;
	}
	pb->q_forw = &pn[0];
	pn[0].q_back = pb;
	rb->q_forw = &rn[0];
	rn[0].q_back = rb;
	for (i = 0; i < k - 1; i++) {
		pn[i].q_forw = &pn[i + 1];
		pn[i + 1].q_back = &pn[i];
		rn[i].q_forw = &rn[i + 1];
		rn[i + 1].q_back = &rn[i];
	}
	pn[k - 1].q_forw = pb;
	pb->q_back = &pn[k - 1];
	rn[k - 1].q_forw = rb;
	rb->q_back = &rn[k - 1];
}

/* -------------------------------------------------------- translit_text */

enum { TCAP = 512 };

static unsigned char tb1[TCAP];
static unsigned char tb2[TCAP];

static long t_ix;

static void
t_case(const unsigned char *data, int n, int len, int from, int to)
{
	char *a, *b;
	int ok;

	memset(tb1, 0x7f, TCAP);
	memset(tb2, 0x7f, TCAP);
	if (n > 0) {
		memcpy(tb1, data, (size_t)n);
		memcpy(tb2, data, (size_t)n);
	}
	clear_err();
	a = P::translit_text((char *)tb1, len, from, to);
	b = ref_translit_text((char *)tb2, len, from, to);
	ok = (a - (char *)tb1) == (b - (char *)tb2) &&
	    memcmp(tb1, tb2, TCAP) == 0 &&
	    memcmp(P::ctab_ptr(), ref_ctab_ptr(), 256) == 0 &&
	    err_same();
	rec(F_TRANSLIT, ok, "buffer/table/return", t_ix);
	t_ix++;
}

static void
phase_translit(void)
{
	static unsigned char d[TCAP];
	static const int probe[] = { 0, 1, 2, 9, 10, 32, 126, 127, 128, 129,
	    200, 254, 255 };
	const int nprobe = (int)(sizeof(probe) / sizeof(probe[0]));
	int i, j, n, len, from, to;
	long it;

	/* empty write window; guard byte must survive the off-by-one */
	t_case(d, 0, 0, 0x7f, 0x00);
	t_case(d, 0, 0, 0x7f, 0x41);
	t_case(d, 0, -1, 0x7f, 0x00);
	t_case(d, 0, -5, 0x00, 0x7f);
	t_case(d, 0, 1, 0x7f, 0x00);
	t_case(d, 0, 2, 0x7f, 0x00);

	/* single byte, every interesting value, both len sides of 1 */
	for (i = 0; i < nprobe; i++) {
		d[0] = (unsigned char)probe[i];
		for (j = 0; j < nprobe; j++) {
			t_case(d, 1, 0, probe[i], probe[j]);
			t_case(d, 1, 1, probe[i], probe[j]);
			t_case(d, 1, 2, probe[i], probe[j]);
		}
	}

	/* the two macro uses in ed.h: NUL_TO_NEWLINE / NEWLINE_TO_NUL */
	for (n = 0; n <= 64; n++) {
		for (i = 0; i < n; i++)
			d[i] = (unsigned char)((i % 3) == 0 ? 0 : (i % 3) == 1 ?
			    '\n' : 0x80 + i);
		t_case(d, n, n, '\0', '\n');
		t_case(d, n, n, '\n', '\0');
		t_case(d, n, n - 1, '\0', '\n');
		t_case(d, n, n + 1, '\n', '\0');
	}

	/* saturated buffers */
	for (i = 0; i < 64; i++)
		d[i] = 0x00;
	t_case(d, 64, 64, 0x00, 0xff);
	for (i = 0; i < 64; i++)
		d[i] = 0xff;
	t_case(d, 64, 64, 0xff, 0x00);
	for (i = 0; i < 64; i++)
		d[i] = 0x80;
	t_case(d, 64, 64, 0x80, 0x7f);
	for (i = 0; i < 256; i++)
		d[i] = (unsigned char)i;
	t_case(d, 256, 256, 0, 255);
	t_case(d, 256, 256, 255, 0);
	t_case(d, 256, 256, 128, 127);

	/* out-of-range 'to' truncates into the unsigned char table */
	t_case(d, 16, 16, 65, 256);
	t_case(d, 16, 16, 66, 257);
	t_case(d, 16, 16, 67, -1);
	t_case(d, 16, 16, 68, -256);

	/* fixed seed sweep */
	for (it = 0; it < 200000; it++) {
		n = (int)rrange(0, 64);
		for (i = 0; i < n; i++)
			d[i] = rbyte();
		len = (int)rrange(-3, n);
		from = (int)rrange(0, 255);
		to = (int)rrange(-2, 258);
		t_case(d, n, len, from, to);
	}
}

/* -------------------------------------------------- get_line_node_addr */

static long g_ix;

static void
glna_case(int which, int k)
{
	long a, b;
	const char *pe;
	int ok;

	clear_err();
	if (which < 0) {
		a = P::get_line_node_addr(P::bh_ptr());
		b = ref_get_line_node_addr(ref_bh_ptr());
	} else {
		a = P::get_line_node_addr(&pn[which]);
		b = ref_get_line_node_addr(&rn[which]);
	}
	pe = P::errmsg;
	ok = a == b && err_same() && globals_same();
	/* the ERR path must really be reachable and really set errmsg */
	if (a == REF_ERR)
		ok = ok && strcmp(pe, "invalid address") == 0;
	rec(F_GET_LINE_NODE_ADDR, ok, "return/errmsg", g_ix);
	(void)k;
	g_ix++;
}

static void
phase_glna(void)
{
	int k, j;
	long it;

	for (k = 0; k <= 12; k++) {
		build_list(k);
		set_globals(k, 0, 0);
		glna_case(-1, k);		/* buffer_head -> 0 */
		for (j = 0; j < k; j++)
			glna_case(j, k);	/* in list -> j + 1 */
		glna_case(MAXN - 1, k);		/* unlinked -> ERR (or 0) */
		glna_case(MAXN - 2, k);
	}

	for (it = 0; it < 200000; it++) {
		if ((it % 7) == 0) {
			k = (int)rrange(0, 12);
			build_list(k);
			set_globals(k, 0, 0);
		}
		switch ((int)rrange(0, 3)) {
		case 0:
			glna_case(-1, k);
			break;
		case 1:
			glna_case(MAXN - 1, k);
			break;
		default:
			glna_case(k > 0 ? (int)rrange(0, k - 1) : MAXN - 2, k);
			break;
		}
	}
}

/* ---------------------------------------------- get_addressed_line_node */

static long a_ix;

static void
galn_case(long n)
{
	P::line_t *a;
	RLine *b;
	int ok;

	clear_err();
	a = P::get_addressed_line_node(n);
	b = ref_get_addressed_line_node(n);
	ok = pos_p(a) == pos_r(b) && pos_p(a) != -1000 && err_same() &&
	    globals_same() && hash_pos_p() == hash_pos_r();
	rec(F_GET_ADDRESSED, ok, "node/mutex/handlers", a_ix);
	a_ix++;
}

static void
phase_galn(void)
{
	int k, d, j;
	long n, it;

	/*
	 * addr_last is deliberately allowed to disagree with the real list
	 * length: that is what makes the (on + addr_last) >> 1 and on >> 1
	 * branch choices observable, because the forward and the backward
	 * walk then land on different nodes of the circular queue.
	 */
	for (k = 0; k <= 8; k++) {
		for (d = -3; d <= 4; d++) {
			build_list(k);
			set_globals(k + d, 0, 0);
			for (n = -3; n <= k + 4; n++)
				galn_case(n);
			for (n = k + 4; n >= -3; n--)
				galn_case(n);
			for (j = 0; j < 6; j++) {
				galn_case(k);
				galn_case(0);
				galn_case(1);
				galn_case(k / 2);
				galn_case((k + d) / 2);
				galn_case((k + d) / 2 + 1);
				galn_case((k + d) / 2 - 1);
			}
		}
	}

	for (it = 0; it < 200000; it++) {
		if ((it % 6) == 0) {
			k = (int)rrange(0, 10);
			build_list(k);
			set_globals(k + rrange(-3, 4), 0,
			    (int)((rnd32() & 1) ?
			    ((1 << (SIGHUP - 1)) | (1 << (SIGINT - 1))) : 0));
		}
		galn_case(rrange(-3, 13));
	}
}

/* ------------------------------------------------------- add_line_node */

static long ad_ix;

static void
phase_aln(void)
{
	long it, ca;
	int i, k, ok;

	for (k = 1; k <= 12; k++) {
		build_list(0);
		set_globals(0, 0, 0);
		for (i = 0; i < k && i < MAXN - 2; i++) {
			ca = (i == 0) ? 0 : rrange(0, i);
			P::current_addr = ca;
			current_addr = ca;
			P::add_line_node(&pn[i]);
			ref_add_line_node(&rn[i]);
			ok = globals_same() && err_same() &&
			    hash_pos_p() == hash_pos_r();
			rec(F_ADD_LINE_NODE, ok, "list/counters", ad_ix);
			ad_ix++;
		}
	}

	/* current_addr outside [0, addr_last] is memory safe (circular list) */
	build_list(0);
	set_globals(0, 0, 0);
	for (i = 0; i < 8; i++) {
		P::current_addr = (long)i - 4;
		current_addr = (long)i - 4;
		P::add_line_node(&pn[i]);
		ref_add_line_node(&rn[i]);
		ok = globals_same() && err_same() && hash_pos_p() == hash_pos_r();
		rec(F_ADD_LINE_NODE, ok, "list/counters(neg)", ad_ix);
		ad_ix++;
	}

	i = 0;
	build_list(0);
	set_globals(0, 0, 0);
	for (it = 0; it < 200000; it++) {
		if (i >= MAXN - 2) {
			build_list(0);
			set_globals(0, 0, 0);
			i = 0;
		}
		ca = rrange(-2, P::addr_last + 2);
		P::current_addr = ca;
		current_addr = ca;
		P::sigflags = (int)((rnd32() & 1) ? (1 << (SIGHUP - 1)) : 0);
		sigflags = P::sigflags;
		P::add_line_node(&pn[i]);
		ref_add_line_node(&rn[i]);
		ok = globals_same() && err_same() && hash_pos_p() == hash_pos_r();
		rec(F_ADD_LINE_NODE, ok, "list/counters(rnd)", ad_ix);
		ad_ix++;
		i++;
	}
	build_list(0);
	set_globals(0, 0, 0);
}

/* ---------------------------------------------------- scratch file phase */

static long o_ix, c_ix, p_ix, s_ix;

static int
sfn_shape_ok(const char *s)
{
	return strlen(s) == 14 && memcmp(s, "/tmp/ed.", 8) == 0;
}

static void
open_pair(void)
{
	mode_t saved, m1, m2;
	int a, b, ok;

	saved = umask(022);
	clear_err();
	P::isbinary = 1;	isbinary = 1;
	P::newline_added = 1;	newline_added = 1;
	a = P::open_sbuf();
	m1 = umask(022);
	b = ref_open_sbuf();
	m2 = umask(saved);
	ok = a == b && err_same() && globals_same() &&
	    m1 == 022 && m2 == 022 &&
	    (P::sfp_ptr() != NULL) == (ref_sfp_ptr() != NULL) &&
	    sfn_shape_ok(P::sfn_ptr()) == sfn_shape_ok(ref_sfn_ptr());
	if (a == 0)
		ok = ok && P::isbinary == 0 && P::newline_added == 0 &&
		    P::sfp_ptr() != NULL && sfn_shape_ok(P::sfn_ptr()) &&
		    access(P::sfn_ptr(), F_OK) == 0 &&
		    access(ref_sfn_ptr(), F_OK) == 0;
	rec(F_OPEN_SBUF, ok, "return/umask/flags", o_ix);
	o_ix++;
}

static void
close_pair(void)
{
	char n1[64], n2[64];
	int had1, had2, a, b, e1, e2, ok;

	strncpy(n1, P::sfn_ptr(), sizeof(n1) - 1);
	n1[sizeof(n1) - 1] = '\0';
	strncpy(n2, ref_sfn_ptr(), sizeof(n2) - 1);
	n2[sizeof(n2) - 1] = '\0';
	had1 = P::sfp_ptr() != NULL;
	had2 = ref_sfp_ptr() != NULL;
	clear_err();
	a = P::close_sbuf();
	b = ref_close_sbuf();
	e1 = access(n1, F_OK);
	e2 = access(n2, F_OK);
	ok = a == b && a == 0 && err_same() && globals_same() &&
	    P::sfp_ptr() == NULL && ref_sfp_ptr() == NULL &&
	    sf_same() && P::sfseek_val() == 0 && P::seek_write_val() == 0 &&
	    (e1 < 0) == (e2 < 0);
	if (had1 && had2)
		ok = ok && e1 < 0 && e2 < 0;
	rec(F_CLOSE_SBUF, ok, "return/unlink/state", c_ix);
	c_ix++;
}

enum { LBUF = 4096 };

static char lb1[LBUF];
static char lb2[LBUF];

static void
put_case(int body, int tail)
{
	const char *a, *b;
	int i, ok;

	memset(lb1, 0x7f, LBUF);
	for (i = 0; i < body; i++)
		lb1[i] = (char)((rnd32() & 15) == 0 ? rbyte() : rbyte_nonl());
	lb1[body] = '\n';
	for (i = body + 1; i < body + 1 + tail && i < LBUF; i++)
		lb1[i] = (char)rbyte();
	memcpy(lb2, lb1, LBUF);
	clear_err();
	a = P::put_sbuf_line(lb1);
	b = ref_put_sbuf_line(lb2);
	ok = (a == NULL) == (b == NULL) && err_same() && globals_same() &&
	    sf_same() && memcmp(lb1, lb2, LBUF) == 0 &&
	    hash_val_p() == hash_val_r();
	if (a != NULL && b != NULL)
		ok = ok && (a - lb1) == (b - lb2);
	rec(F_PUT_SBUF_LINE, ok, "return/state/list", p_ix);
	p_ix++;
}

static P::line_t *
nth_p(long t)
{
	P::line_t *b = P::bh_ptr();
	P::line_t *p = b->q_forw;

	while (t > 0 && p != b) {
		p = p->q_forw;
		t--;
	}
	return p;
}

static RLine *
nth_r(long t)
{
	RLine *b = ref_bh_ptr();
	RLine *p = b->q_forw;

	while (t > 0 && p != b) {
		p = p->q_forw;
		t--;
	}
	return p;
}

static void
get_case(long t)
{
	P::line_t *lp = nth_p(t);
	RLine *lr = nth_r(t);
	char *a, *b;
	int ok;

	if (lp->len != lr->len || lp->seek != lr->seek) {
		rec(F_GET_SBUF_LINE, 0, "node payload differs", s_ix);
		s_ix++;
		return;
	}
	clear_err();
	a = P::get_sbuf_line(lp);
	b = ref_get_sbuf_line(lr);
	ok = (a == NULL) == (b == NULL) && err_same() && globals_same() &&
	    sf_same();
	if (a != NULL && b != NULL)
		ok = ok && memcmp(a, b, (size_t)lp->len + 1) == 0 &&
		    a[lp->len] == '\0';
	rec(F_GET_SBUF_LINE, ok, "text/state", s_ix);
	s_ix++;
}

static void
get_special(void)
{
	P::line_t bad_p;
	RLine bad_r;
	char *a, *b;
	int ok;

	/* buffer_head short circuit */
	clear_err();
	a = P::get_sbuf_line(P::bh_ptr());
	b = ref_get_sbuf_line(ref_bh_ptr());
	ok = a == NULL && b == NULL && err_same() && sf_same() && globals_same();
	rec(F_GET_SBUF_LINE, ok, "buffer_head", s_ix);
	s_ix++;

	/* negative seek: fseeko() fails */
	memset(&bad_p, 0, sizeof(bad_p));
	memset(&bad_r, 0, sizeof(bad_r));
	bad_p.seek = -1;	bad_r.seek = -1;
	bad_p.len = 5;		bad_r.len = 5;
	clear_err();
	a = P::get_sbuf_line(&bad_p);
	b = ref_get_sbuf_line(&bad_r);
	ok = a == NULL && b == NULL && err_same() && sf_same() &&
	    globals_same() && strcmp(P::errmsg, "cannot seek temp file") == 0;
	rec(F_GET_SBUF_LINE, ok, "seek failure", s_ix);
	s_ix++;

	/* seek past EOF: fread() comes up short */
	bad_p.seek = 1 << 20;	bad_r.seek = 1 << 20;
	bad_p.len = 17;		bad_r.len = 17;
	clear_err();
	a = P::get_sbuf_line(&bad_p);
	b = ref_get_sbuf_line(&bad_r);
	ok = a == NULL && b == NULL && err_same() && sf_same() &&
	    globals_same() && strcmp(P::errmsg, "cannot read temp file") == 0;
	rec(F_GET_SBUF_LINE, ok, "short read", s_ix);
	s_ix++;

	/* zero length line at EOF reads nothing and succeeds */
	bad_p.seek = 1 << 20;	bad_r.seek = 1 << 20;
	bad_p.len = 0;		bad_r.len = 0;
	clear_err();
	a = P::get_sbuf_line(&bad_p);
	b = ref_get_sbuf_line(&bad_r);
	ok = (a != NULL) && (b != NULL) && err_same() && sf_same() &&
	    globals_same() && a[0] == '\0' && b[0] == '\0';
	rec(F_GET_SBUF_LINE, ok, "empty at eof", s_ix);
	s_ix++;
}

static void
free_list(void)
{
	P::line_t *pb, *p, *pnext;
	RLine *rb, *r, *rnext;

	reset_gal();
	pb = P::bh_ptr();
	rb = ref_bh_ptr();
	for (p = pb->q_forw; p != pb; p = pnext) {
		pnext = p->q_forw;
		free(p);
	}
	for (r = rb->q_forw; r != rb; r = rnext) {
		rnext = r->q_forw;
		free(r);
	}
	empty_list();
	set_globals(0, 0, 0);
}

static void
phase_file(long rounds, long per_round)
{
	long r, j, t, nl;
	int body, tail;

	for (r = 0; r < rounds; r++) {
		open_pair();
		empty_list();
		set_globals(0, 0, 0);
		nl = 0;
		for (j = 0; j < per_round; j++) {
			if ((rnd32() % 512) == 0)
				body = (int)rrange(512, 2600);
			else
				body = (int)rrange(0, 40);
			tail = (int)rrange(0, 8);
			if (body + 1 + tail >= LBUF)
				body = LBUF - 2 - tail;
			put_case(body, tail);
			nl = P::addr_last;
			if (nl > 0) {
				t = rrange(0, nl - 1);
				get_case(t);
			}
			if ((j % 37) == 5)
				get_special();
		}
		free_list();
		close_pair();
		close_pair();		/* sfp == NULL path */
	}
}

static void
phase_open_close_sweep(long n)
{
	long i;

	for (i = 0; i < n; i++) {
		open_pair();
		close_pair();
	}
}

static void
phase_open_fail(void)
{
	struct rlimit rl, saved;
	int i;

	if (getrlimit(RLIMIT_NOFILE, &saved) != 0)
		return;
	for (i = 0; i < 64; i++) {
		rl = saved;
		rl.rlim_cur = 3;
		if (setrlimit(RLIMIT_NOFILE, &rl) != 0)
			return;
		open_pair();
		setrlimit(RLIMIT_NOFILE, &saved);
		/*
		 * Both sides must have failed; sfp stays NULL so close_sbuf
		 * exercises its "nothing open" path once more.
		 */
		rec(F_OPEN_SBUF, P::sfp_ptr() == NULL && ref_sfp_ptr() == NULL &&
		    strcmp(P::errmsg, "cannot open temp file") == 0 &&
		    strcmp(errmsg, "cannot open temp file") == 0,
		    "EMFILE path", o_ix);
		o_ix++;
		close_pair();
	}
}

/* ---------------------------------------------------------------- quit */

static void
call_pquit(int n)
{
	P::quit(n);
}

static void
call_rquit(int n)
{
	ref_quit(n);
}

static int
run_child(void (*fn)(int), int n)
{
	pid_t pid;
	int st = -1;

	fflush(NULL);
	pid = fork();
	if (pid == 0) {
		fn(n);
		_exit(99);
	}
	if (pid < 0)
		return -1;
	while (waitpid(pid, &st, 0) < 0 && errno == EINTR)
		;
	return st;
}

static long q_ix;

static void
quit_case(int n, int with_file)
{
	char n1[64], n2[64];
	int st1, st2, g1, g2, ok;

	if (with_file) {
		P::open_sbuf();
		ref_open_sbuf();
	}
	strncpy(n1, P::sfn_ptr(), sizeof(n1) - 1);
	n1[sizeof(n1) - 1] = '\0';
	strncpy(n2, ref_sfn_ptr(), sizeof(n2) - 1);
	n2[sizeof(n2) - 1] = '\0';
	st1 = run_child(call_pquit, n);
	g1 = access(n1, F_OK);
	st2 = run_child(call_rquit, n);
	g2 = access(n2, F_OK);
	ok = st1 == st2 && WIFEXITED(st1) && WIFEXITED(st2) &&
	    WEXITSTATUS(st1) == (n & 0xff) && (g1 < 0) == (g2 < 0);
	if (with_file)
		ok = ok && g1 < 0 && g2 < 0;
	rec(F_QUIT, ok, with_file ? "exit/unlink" : "exit(no file)", q_ix);
	q_ix++;
	if (with_file) {
		P::close_sbuf();
		ref_close_sbuf();
	}
}

static void
phase_quit(void)
{
	static const int codes[] = { 0, 1, 2, 3, 7, 42, 127, 128, 254, 255,
	    256, 257 };
	const int nc = (int)(sizeof(codes) / sizeof(codes[0]));
	int i;
	long it;

	for (i = 0; i < nc; i++) {
		quit_case(codes[i], 1);
		quit_case(codes[i], 0);
	}
	for (it = 0; it < 200; it++)
		quit_case((int)rrange(0, 255), (int)(it & 1));
}

/* -------------------------------------------------------- init_buffers */

static long i_ix;

static void
phase_init_parent(void)
{
	int ok, i;

	P::isbinary = 1;	isbinary = 1;
	P::newline_added = 1;	newline_added = 1;
	clear_err();
	P::init_buffers();
	ref_init_buffers();
	ok = memcmp(P::ctab_ptr(), ref_ctab_ptr(), 256) == 0 &&
	    err_same() && globals_same() &&
	    P::isbinary == 0 && P::newline_added == 0 &&
	    P::bh_ptr()->q_forw == P::bh_ptr() &&
	    P::bh_ptr()->q_back == P::bh_ptr() &&
	    ref_bh_ptr()->q_forw == ref_bh_ptr() &&
	    ref_bh_ptr()->q_back == ref_bh_ptr() &&
	    sfn_shape_ok(P::sfn_ptr()) && sfn_shape_ok(ref_sfn_ptr()) &&
	    P::sfp_ptr() != NULL && ref_sfp_ptr() != NULL;
	for (i = 0; i < 256; i++)
		ok = ok && P::ctab_ptr()[i] == (unsigned char)i &&
		    ref_ctab_ptr()[i] == (unsigned char)i;
	rec(F_INIT_BUFFERS, ok, "ctab/queue/scratch", i_ix);
	i_ix++;
	close_pair();
}

static int
run_init_child(int which, int lower)
{
	pid_t pid;
	int st = -1;

	fflush(NULL);
	pid = fork();
	if (pid == 0) {
		struct rlimit rl;

		if (lower && getrlimit(RLIMIT_NOFILE, &rl) == 0) {
			rl.rlim_cur = 3;
			setrlimit(RLIMIT_NOFILE, &rl);
		}
		if (which == 0) {
			P::init_buffers();
			unlink(P::sfn_ptr());
		} else {
			ref_init_buffers();
			unlink(ref_sfn_ptr());
		}
		_exit(0);
	}
	if (pid < 0)
		return -1;
	while (waitpid(pid, &st, 0) < 0 && errno == EINTR)
		;
	return st;
}

static void
phase_init_fork(void)
{
	int st1, st2, i, ok;

	for (i = 0; i < 8; i++) {
		st1 = run_init_child(0, 0);
		st2 = run_init_child(1, 0);
		ok = st1 == st2 && WIFEXITED(st1) && WEXITSTATUS(st1) == 0;
		rec(F_INIT_BUFFERS, ok, "child success path", i_ix);
		i_ix++;

		st1 = run_init_child(0, 1);
		st2 = run_init_child(1, 1);
		ok = st1 == st2 && WIFEXITED(st1) && WEXITSTATUS(st1) == 2;
		rec(F_INIT_BUFFERS, ok, "child quit(2) path", i_ix);
		i_ix++;
	}
}

/* ---------------------------------------------------------------- main */

static int saved_stderr = -1;

static void
mute_stderr(void)
{
	int devnull;

	saved_stderr = dup(2);
	devnull = open("/dev/null", O_WRONLY);
	if (devnull >= 0) {
		dup2(devnull, 2);
		close(devnull);
	}
}

static void
unmute_stderr(void)
{
	if (saved_stderr >= 0) {
		dup2(saved_stderr, 2);
		close(saved_stderr);
		saved_stderr = -1;
	}
}

int
main(void)
{
	long total_cases = 0, total_fails = 0;
	int i;

	/* init_buffers() first: it wants to call setvbuf() on a virgin stdout */
	phase_init_parent();
	mute_stderr();

	phase_translit();
	phase_glna();
	phase_galn();
	phase_aln();
	phase_file(4000, 50);
	phase_open_close_sweep(20000);
	phase_open_fail();
	phase_quit();
	phase_init_fork();

	unmute_stderr();

	printf("\n%-26s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	printf("--------------------------------------------------------------"
	    "----\n");
	for (i = 0; i < NFUNC; i++) {
		total_cases += ncase[i];
		total_fails += nfail[i];
		printf("%-26s %12ld %12ld  %s\n", fname[i], ncase[i], nfail[i],
		    nfail[i] == 0 ? "ok" : "FAIL");
	}
	printf("--------------------------------------------------------------"
	    "----\n");
	printf("%-26s %12ld %12ld  %s\n", "TOTAL", total_cases, total_fails,
	    total_fails == 0 ? "ok" : "FAIL");

	return total_fails == 0 ? 0 : 1;
}
