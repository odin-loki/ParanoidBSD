/*
 * PBSD batch b0207 - differential test harness.
 *
 * Every ported function is driven with hand written edge cases and with a
 * fixed seed randomised sweep, and every observable of the port is compared
 * against the ref_ oracle: return values, the complete content of guarded
 * input buffers, pointer offsets relative to a buffer base (never raw
 * addresses), the full state of the stateful opt_next() iterator driven to
 * exhaustion, and the diagnostics recorded by the paxwarn double.
 */

#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

import pbsd.bin.pax.b0207;

namespace P = pbsd::bin_pax::b0207;

extern "C" {
struct roplist {
	char		*name;
	char		*value;
	struct roplist	*fow;
};
int ref_no_op(void);
struct roplist *ref_opt_next(void);
int ref_opt_add(const char *str);
off_t ref_str_offt(char *val);
u_int ref_st_hash(char *name, int len, int tabsz);
void ref_warn_reset(void);
int ref_warn_count(void);
const char *ref_warn_text(void);
}

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

enum { F_NOOP, F_HASH, F_OFFT, F_OPT, F_NFN };

static struct {
	const char	*name;
	long		 cases;
	long		 fails;
	long		 shown;
} fns[F_NFN] = {
	{ "no_op",		0, 0, 0 },
	{ "st_hash",		0, 0, 0 },
	{ "str_offt",		0, 0, 0 },
	{ "opt_add/opt_next",	0, 0, 0 },
};

static void
report(int f, const char *fmt, ...)
{
	va_list ap;

	fns[f].fails++;
	if (fns[f].shown >= 12)
		return;
	fns[f].shown++;
	fprintf(stderr, "FAIL %s: ", fns[f].name);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

/* printable rendering of an arbitrary byte string */
static const char *
show(const void *p, size_t n)
{
	static char out[4][1200];
	static int which;
	const unsigned char *s = (const unsigned char *)p;
	char *o;
	size_t i;

	which = (which + 1) & 3;
	o = out[which];
	if (p == NULL) {
		strcpy(o, "(null)");
		return (o);
	}
	if (n > 280)
		n = 280;
	for (i = 0; i < n; i++) {
		if (s[i] >= 0x20 && s[i] < 0x7f && s[i] != '\\')
			*o++ = (char)s[i];
		else
			o += sprintf(o, "\\x%02x", s[i]);
	}
	*o = '\0';
	return (out[which]);
}

/* ------------------------------------------------------------------ */
/* deterministic prng						      */
/* ------------------------------------------------------------------ */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s ? s : 0x9e3779b97f4a7c15ULL;
}

static uint64_t
rng_next(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x);
}

static uint32_t
rnd(uint32_t n)
{
	return ((uint32_t)(rng_next() % (uint64_t)n));
}

/* ------------------------------------------------------------------ */
/* no_op()							      */
/* ------------------------------------------------------------------ */

static void
noop_case(void)
{
	int a = P::no_op();
	int b = ref_no_op();

	fns[F_NOOP].cases++;
	if (a != b)
		report(F_NOOP, "port=%d oracle=%d", a, b);
}

static void
test_no_op(void)
{
	long i;

	noop_case();
	noop_case();
	rng_seed(0x1234abcd5678ef90ULL);
	for (i = 0; i < 200000; i++) {
		(void)rng_next();
		noop_case();
	}
}

/* ------------------------------------------------------------------ */
/* st_hash()							      */
/* ------------------------------------------------------------------ */

/*
 * The two buffers are 1024 bytes of 0x7f guard with the input copied in at
 * DATA_OFF.  DATA_OFF and the length cap are chosen so that a port whose
 * pointer arithmetic has been mutated (name[len+MAXKEYLEN], name[len-MAXKEYLEN]
 * with a small len, one byte over-reads ...) still lands inside the guarded
 * buffer: it then reads guard bytes instead of the data and the hash differs,
 * rather than the test dying on a segfault before it can report.
 */
#define HBUFSZ		1024
#define DATA_OFF	192
#define MAXDLEN		400

static void
hash_case(const unsigned char *data, int dlen, int off, int len, int tabsz)
{
	unsigned char a[HBUFSZ], b[HBUFSZ];
	u_int ra, rb;
	int i;

	memset(a, 0x7f, sizeof(a));
	memset(b, 0x7f, sizeof(b));
	if (dlen > 0) {
		memcpy(a + off, data, (size_t)dlen);
		memcpy(b + off, data, (size_t)dlen);
	}

	fns[F_HASH].cases++;
	ra = P::st_hash((char *)a + off, len, tabsz);
	rb = ref_st_hash((char *)b + off, len, tabsz);
	if (ra != rb) {
		report(F_HASH, "len=%d tabsz=%d off=%d dlen=%d data=%s "
		    "port=%u oracle=%u", len, tabsz, off, dlen,
		    show(data, (size_t)(dlen > 0 ? dlen : 0)), ra, rb);
		return;
	}
	if (memcmp(a, b, sizeof(a)) != 0) {
		for (i = 0; i < HBUFSZ; i++)
			if (a[i] != b[i])
				break;
		report(F_HASH, "buffer differs at %d (port=%02x oracle=%02x) "
		    "len=%d tabsz=%d", i, a[i], b[i], len, tabsz);
	}
}

static void
test_st_hash(void)
{
	static const int lens[] = {
		-8, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		15, 16, 17, 31, 32, 33, 63, 64, 65,
		124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
		191, 192, 255, 256, 257, 383, 384, 399, 400
	};
	static const int tabs[] = {
		1, 2, 3, 4, 5, 7, 8, 13, 16, 17, 127, 251, 317, 1021,
		4093, 65521, 1000003, 2147483647, -1
	};
	unsigned char data[MAXDLEN];
	int p, l, t, i, dlen, len, off;

	/*
	 * hand written patterns: empty, single char, NUL heavy, all high bit,
	 * alternating, and the full 0x00-0xff range.
	 */
	for (p = 0; p < 8; p++) {
		for (i = 0; i < MAXDLEN; i++) {
			switch (p) {
			case 0: data[i] = 0x00; break;
			case 1: data[i] = 'a'; break;
			case 2: data[i] = (unsigned char)(i & 0xff); break;
			case 3: data[i] = 0xff; break;
			case 4: data[i] = (unsigned char)(0x80 + (i & 0x7f));
				break;
			case 5: data[i] = (unsigned char)((i % 3) ? 0 : 0x80);
				break;
			case 6: data[i] = (unsigned char)((i & 1) ? 0xff : 0);
				break;
			default: data[i] = 0x7f; break;
			}
		}
		for (l = 0; l < (int)(sizeof(lens) / sizeof(lens[0])); l++) {
			len = lens[l];
			dlen = len > 0 ? len : 0;
			if (dlen > MAXDLEN)
				dlen = MAXDLEN;
			for (t = 0; t < (int)(sizeof(tabs) / sizeof(tabs[0]));
			    t++)
				hash_case(data, dlen, DATA_OFF, len, tabs[t]);
		}
	}

	/* single byte inputs, every possible byte value */
	for (i = 0; i < 256; i++) {
		data[0] = (unsigned char)i;
		hash_case(data, 1, DATA_OFF, 1, 317);
		hash_case(data, 1, DATA_OFF, 1, 1);
		hash_case(data, 1, DATA_OFF + 1, 1, 65521);
	}

	/* data shorter than the requested length, and unaligned starts */
	for (off = DATA_OFF; off < DATA_OFF + 4; off++) {
		for (i = 0; i < 32; i++)
			data[i] = (unsigned char)(0x81 + i);
		for (l = 0; l < (int)(sizeof(lens) / sizeof(lens[0])); l++)
			hash_case(data, 32, off, lens[l], 317);
	}

	/* fixed seed randomised sweep */
	rng_seed(0xc0ffee1234567890ULL);
	for (long n = 0; n < 200000; n++) {
		dlen = (int)rnd(MAXDLEN + 1);
		for (i = 0; i < dlen; i++)
			data[i] = (unsigned char)rnd(256);
		switch (rnd(8)) {
		case 0:
			len = dlen ? (int)rnd((uint32_t)dlen) : 0;
			break;
		case 1:
			len = dlen + (int)rnd(9) - 4;
			if (len > MAXDLEN)
				len = MAXDLEN;
			break;
		case 2:
			len = -(int)rnd(9);
			break;
		case 3:
			len = 124 + (int)rnd(11);
			break;
		default:
			len = dlen;
			break;
		}
		off = DATA_OFF + (int)rnd(4);
		if (rnd(2))
			t = tabs[rnd(sizeof(tabs) / sizeof(tabs[0]))];
		else
			t = 1 + (int)rnd(100000);
		hash_case(data, dlen, off, len, t);
	}
}

/* ------------------------------------------------------------------ */
/* str_offt()							      */
/* ------------------------------------------------------------------ */

#define OBUFSZ		1024
#define OSTR_OFF	16

static void
offt_case(const char *s, size_t slen)
{
	char a[OBUFSZ], b[OBUFSZ];
	off_t ra, rb;
	int i;

	if (slen > OBUFSZ - OSTR_OFF - 2)
		slen = OBUFSZ - OSTR_OFF - 2;
	memset(a, 0x7f, sizeof(a));
	memset(b, 0x7f, sizeof(b));
	memcpy(a + OSTR_OFF, s, slen);
	memcpy(b + OSTR_OFF, s, slen);
	a[OSTR_OFF + slen] = '\0';
	b[OSTR_OFF + slen] = '\0';

	fns[F_OFFT].cases++;
	ra = P::str_offt(a + OSTR_OFF);
	rb = ref_str_offt(b + OSTR_OFF);
	if (ra != rb) {
		report(F_OFFT, "input=\"%s\" port=%lld oracle=%lld",
		    show(s, slen), (long long)ra, (long long)rb);
		return;
	}
	if (memcmp(a, b, sizeof(a)) != 0) {
		for (i = 0; i < OBUFSZ; i++)
			if (a[i] != b[i])
				break;
		report(F_OFFT, "buffer differs at %d (port=%02x oracle=%02x) "
		    "input=\"%s\"", i, (unsigned char)a[i],
		    (unsigned char)b[i], show(s, slen));
	}
}

static void
offt_str(const char *s)
{
	offt_case(s, strlen(s));
}

static void
test_str_offt(void)
{
	static const char *cases[] = {
		"", " ", "  ", "\t", "\n", "+", "-", "x", "b", "*",
		"0", "1", "2", "7", "9", "10", "511", "512", "513",
		"1023", "1024", "1025", "-0", "-1", "-5", "-1024",
		"+0", "+1", "+512", " 5", "  5", "\t5", "\n5", " -5",
		"5 ", "5\t", "5\n", "abc", "zzz", "5abc", "a5",
		"5b", "1b", "0b", "-1b", "2b", "5k", "1k", "0k", "2k",
		"5m", "1m", "2m", "5w", "1w", "0w", "2w", "5B", "5K",
		"5M", "5W", "5bb", "5bk", "5kb", "5kk", "5wb", "5mb",
		"5x", "x5", "5*", "*5", "5x3", "3x5", "5*3", "5x3x2",
		"5x3*2", "2x2x2x2", "1x1", "0x0", "5x0", "0x5", "5x-3",
		"-3x5", "5xb", "5bx3", "3xk", "2kx4", "4x2k", "2bx2b",
		"010", "0100", "08", "09", "0x10", "0X10", "0xff",
		"0xFF", "0x", "0X", "0xg", "0x1p", "00", "000",
		"9223372036854775807", "9223372036854775806",
		"9223372036854775808", "99999999999999999999",
		"-9223372036854775808", "-9223372036854775809",
		"4611686018427387904b", "18014398509481984k",
		"9007199254740992m", "2305843009213693952w",
		"1000000000000x1000000000000", "65536x65536x65536",
		"9223372036854775807x2", "9223372036854775807b",
		"2147483647", "2147483648", "4294967295", "4294967296",
		"1x", "1xx", "1x1x", ",", ".", "5.5", "5,5", "5;",
		"1 x 1", "1w1", "1b1", "5m5",
	};
	static const char digits[] = "0123456789";
	static const char sufs[] = "bkmw";
	char buf[160];
	size_t i, n;
	int p, parts;

	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
		offt_str(cases[i]);

	/* every single byte value on its own, and prefixed by a digit */
	for (i = 0; i < 256; i++) {
		buf[0] = (char)i;
		buf[1] = '\0';
		offt_case(buf, 1);
		buf[0] = '7';
		buf[1] = (char)i;
		buf[2] = '\0';
		offt_case(buf, 2);
		buf[0] = '7';
		buf[1] = 'b';
		buf[2] = (char)i;
		buf[3] = '\0';
		offt_case(buf, 3);
		buf[0] = '7';
		buf[1] = 'x';
		buf[2] = (char)i;
		buf[3] = '\0';
		offt_case(buf, 3);
	}

	/* a string with an embedded NUL: only the head is seen */
	offt_case("5b\0009", 5);
	offt_case("\0005", 2);

	/* fixed seed randomised sweep */
	rng_seed(0xfeedfacecafebeefULL);
	for (long it = 0; it < 200000; it++) {
		n = 0;
		if (rnd(4) == 0) {
			/* free form junk over an interesting alphabet */
			static const char alpha[] =
			    "0123456789abkmwxX*+- \t\nz.,0x";
			int l = (int)rnd(20);
			for (p = 0; p < l; p++) {
				if (rnd(16) == 0)
					buf[n++] = (char)(0x80 + rnd(128));
				else
					buf[n++] =
					    alpha[rnd(sizeof(alpha) - 1)];
			}
		} else {
			parts = 1 + (int)rnd(3);
			for (p = 0; p < parts; p++) {
				if (p && n + 2 < sizeof(buf))
					buf[n++] = rnd(2) ? 'x' : '*';
				if (rnd(8) == 0)
					buf[n++] = ' ';
				if (rnd(8) == 0)
					buf[n++] = rnd(2) ? '+' : '-';
				if (rnd(8) == 0) {
					buf[n++] = '0';
					buf[n++] = rnd(2) ? 'x' : 'X';
					int l = 1 + (int)rnd(6);
					while (l--)
						buf[n++] =
						    "0123456789abcdefABCDEF"
						    [rnd(22)];
				} else {
					int l = 1 + (int)rnd(20);
					while (l--)
						buf[n++] = digits[rnd(10)];
				}
				if (rnd(3) == 0)
					buf[n++] = sufs[rnd(4)];
				if (rnd(24) == 0)
					buf[n++] = (char)(0x21 + rnd(94));
			}
		}
		if (n >= sizeof(buf))
			n = sizeof(buf) - 1;
		offt_case(buf, n);
	}
}

/* ------------------------------------------------------------------ */
/* opt_add() / opt_next()					      */
/* ------------------------------------------------------------------ */

#define SBUFSZ		512
#define SSTR_OFF	8
#define DRAIN_CAP	4096

/* empty both lists without comparing anything (recovery between cases) */
static void
opt_flush(void)
{
	int i;

	for (i = 0; i < DRAIN_CAP; i++)
		if (P::opt_next() == NULL)
			break;
	for (i = 0; i < DRAIN_CAP; i++)
		if (ref_opt_next() == NULL)
			break;
}

/*
 * Drive the iterator to exhaustion, comparing after every single step: the
 * NULL-ness of the returned node, the name and value text, the offset of the
 * name and of the value from the base of that node's own string buffer (never
 * raw addresses), and whether the node still chains onwards.
 */
static int
opt_drain(const char *what, int offsets_valid)
{
	struct roplist *pb;
	P::OPLIST *pa;
	char *basea = NULL, *baseb = NULL;
	int n = 0;
	int bad = 0;

	for (;;) {
		pa = P::opt_next();
		pb = ref_opt_next();
		if ((pa == NULL) != (pb == NULL)) {
			report(F_OPT, "%s: step %d port=%s oracle=%s", what, n,
			    pa ? "node" : "NULL", pb ? "node" : "NULL");
			bad = 1;
			break;
		}
		if (pa == NULL)
			break;
		if (n == 0) {
			basea = pa->name;
			baseb = pb->name;
		}
		if ((pa->name == NULL) != (pb->name == NULL) ||
		    (pa->value == NULL) != (pb->value == NULL)) {
			report(F_OPT, "%s: step %d null field mismatch", what,
			    n);
			bad = 1;
			break;
		}
		if (pa->name != NULL && strcmp(pa->name, pb->name) != 0) {
			report(F_OPT, "%s: step %d name port=\"%s\" "
			    "oracle=\"%s\"", what, n,
			    show(pa->name, strlen(pa->name)),
			    show(pb->name, strlen(pb->name)));
			bad = 1;
		}
		if (pa->value != NULL && strcmp(pa->value, pb->value) != 0) {
			report(F_OPT, "%s: step %d value port=\"%s\" "
			    "oracle=\"%s\"", what, n,
			    show(pa->value, strlen(pa->value)),
			    show(pb->value, strlen(pb->value)));
			bad = 1;
		}
		if (pa->name != NULL && pa->value != NULL &&
		    (pa->value - pa->name) != (pb->value - pb->name)) {
			report(F_OPT, "%s: step %d value offset port=%ld "
			    "oracle=%ld", what, n, (long)(pa->value - pa->name),
			    (long)(pb->value - pb->name));
			bad = 1;
		}
		if (offsets_valid && pa->name != NULL &&
		    (pa->name - basea) != (pb->name - baseb)) {
			report(F_OPT, "%s: step %d name offset port=%ld "
			    "oracle=%ld", what, n, (long)(pa->name - basea),
			    (long)(pb->name - baseb));
			bad = 1;
		}
		if ((pa->fow == NULL) != (pb->fow == NULL)) {
			report(F_OPT, "%s: step %d fow port=%s oracle=%s", what,
			    n, pa->fow ? "node" : "NULL",
			    pb->fow ? "node" : "NULL");
			bad = 1;
		}
		if (++n >= DRAIN_CAP) {
			report(F_OPT, "%s: iterator did not terminate", what);
			bad = 1;
			break;
		}
	}
	if (bad)
		opt_flush();
	return (bad);
}

static void
opt_warncmp(const char *what)
{
	if (P::warn_count() != ref_warn_count() ||
	    strcmp(P::warn_text(), ref_warn_text()) != 0)
		report(F_OPT, "%s: warnings port=%d\"%s\" oracle=%d\"%s\"",
		    what, P::warn_count(), P::warn_text(), ref_warn_count(),
		    ref_warn_text());
}

/* one opt_add() on a fresh list, then the iterator to exhaustion */
static void
opt_case(const char *s, size_t slen, int isnull)
{
	char a[SBUFSZ], b[SBUFSZ];
	int ra, rb, i;
	char label[320];

	opt_flush();
	P::warn_reset();
	ref_warn_reset();

	memset(a, 0x7f, sizeof(a));
	memset(b, 0x7f, sizeof(b));
	if (!isnull) {
		if (slen > SBUFSZ - SSTR_OFF - 2)
			slen = SBUFSZ - SSTR_OFF - 2;
		memcpy(a + SSTR_OFF, s, slen);
		memcpy(b + SSTR_OFF, s, slen);
		a[SSTR_OFF + slen] = '\0';
		b[SSTR_OFF + slen] = '\0';
	}
	snprintf(label, sizeof(label), "opt_add(%s)",
	    isnull ? "NULL" : show(s, slen));

	fns[F_OPT].cases++;
	ra = P::opt_add(isnull ? NULL : a + SSTR_OFF);
	rb = ref_opt_add(isnull ? NULL : b + SSTR_OFF);
	if (ra != rb) {
		report(F_OPT, "%s: port=%d oracle=%d", label, ra, rb);
		opt_flush();
		return;
	}
	if (!isnull && memcmp(a, b, sizeof(a)) != 0) {
		for (i = 0; i < SBUFSZ; i++)
			if (a[i] != b[i])
				break;
		report(F_OPT, "%s: caller buffer differs at %d "
		    "(port=%02x oracle=%02x)", label, i, (unsigned char)a[i],
		    (unsigned char)b[i]);
	}
	opt_warncmp(label);
	(void)opt_drain(label, 1);
}

static void
opt_str(const char *s)
{
	opt_case(s, strlen(s), 0);
}

/* several opt_add() calls stacked up before the list is drained */
static void
opt_seq(const char *const *strs, int n)
{
	char buf[SBUFSZ];
	int i, ra, rb;

	opt_flush();
	P::warn_reset();
	ref_warn_reset();
	fns[F_OPT].cases++;
	for (i = 0; i < n; i++) {
		size_t l = strlen(strs[i]);
		char b2[SBUFSZ];

		memset(buf, 0x7f, sizeof(buf));
		memset(b2, 0x7f, sizeof(b2));
		memcpy(buf, strs[i], l + 1);
		memcpy(b2, strs[i], l + 1);
		ra = P::opt_add(buf);
		rb = ref_opt_add(b2);
		if (ra != rb) {
			report(F_OPT, "seq[%d] \"%s\": port=%d oracle=%d", i,
			    strs[i], ra, rb);
			opt_flush();
			return;
		}
		if (memcmp(buf, b2, sizeof(buf)) != 0) {
			report(F_OPT, "seq[%d] \"%s\": caller buffer differs",
			    i, strs[i]);
		}
	}
	opt_warncmp("seq");
	(void)opt_drain("seq", 0);
}

static void
test_opt(void)
{
	static const char *cases[] = {
		"", "=", ",", "=,", ",=", ",,", "a", "ab", "a,b", "a=",
		"=a", "a=b", "a=b,", ",a=b", "a=b,c", "a,b=c", "a=b=c",
		"a==b", "a=,b=", "=,=", "a=b,c=d", "a=b,c=d,e=f",
		"a=b,,c=d", "aa=bb,cc=dd", "x=1,y=,=2", "write_opt=nodir",
		"compress=1,blocksize=5120,name=foo",
		"n=", "=v", "n=v", ",,,,", "====", "a=b,c", "c,a=b",
		"listopt=%M %(size)d", "a=b,c=d,e", "e,a=b",
		"\x80=\xff", "a=\x80\x81", "\xff\xfe=\x01",
		" = ", "\t=\n", "a b=c d", "-=-", "==",
	};
	static const char *seq1[] = { "a=1", "b=2" };
	static const char *seq2[] = { "a=1,b=2", "c=3" };
	static const char *seq3[] = { "bad", "a=1" };
	static const char *seq4[] = { "a=1", "bad", "b=2" };
	static const char *seq5[] = { "", "a=1", "" };
	static const char *seq6[] = { "a=1,b", "c=3" };
	static const char alpha[] = "ab=,cx01\xff\x80= ";
	char buf[64];
	size_t i, n;
	int l;

	opt_case(NULL, 0, 1);
	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
		opt_str(cases[i]);

	/* every byte value as a name, as a value and on its own */
	for (i = 1; i < 256; i++) {
		buf[0] = (char)i;
		buf[1] = '\0';
		opt_case(buf, 1, 0);
		buf[0] = (char)i;
		buf[1] = '=';
		buf[2] = 'v';
		buf[3] = '\0';
		opt_case(buf, 3, 0);
		buf[0] = 'n';
		buf[1] = '=';
		buf[2] = (char)i;
		buf[3] = '\0';
		opt_case(buf, 3, 0);
	}

	/* long chain of pairs */
	{
		char longbuf[512];

		n = 0;
		for (i = 0; i < 30; i++)
			n += (size_t)sprintf(longbuf + n, "%c%zu=%zu%s",
			    'a' + (int)i % 26, i, i * 7, i == 29 ? "" : ",");
		opt_case(longbuf, n, 0);
	}

	opt_seq(seq1, 2);
	opt_seq(seq2, 2);
	opt_seq(seq3, 2);
	opt_seq(seq4, 3);
	opt_seq(seq5, 3);
	opt_seq(seq6, 2);

	/* fixed seed randomised sweep */
	rng_seed(0x0badc0de12345678ULL);
	for (long it = 0; it < 200000; it++) {
		if (rnd(100) == 0) {
			opt_case(NULL, 0, 1);
			continue;
		}
		l = (int)rnd(17);
		for (n = 0; n < (size_t)l; n++)
			buf[n] = alpha[rnd(sizeof(alpha) - 1)];
		opt_case(buf, n, 0);
	}
}

/* ------------------------------------------------------------------ */

static void
on_alarm(int sig)
{
	static const char msg[] =
	    "\nHARNESS TIMEOUT: the port failed to terminate\n";

	(void)sig;
	(void)!write(2, msg, sizeof(msg) - 1);
	_exit(1);
}

int
main(void)
{
	long total = 0;
	int i;

	signal(SIGALRM, on_alarm);
	alarm(600);

	test_no_op();
	test_st_hash();
	test_str_offt();
	test_opt();

	printf("\n%-24s %12s %12s %s\n", "function", "cases", "failures",
	    "result");
	printf("%-24s %12s %12s %s\n", "------------------------",
	    "------------", "------------", "------");
	for (i = 0; i < F_NFN; i++) {
		printf("%-24s %12ld %12ld %s\n", fns[i].name, fns[i].cases,
		    fns[i].fails, fns[i].fails ? "FAIL" : "ok");
		total += fns[i].fails;
	}
	printf("%-24s %12s %12ld %s\n", "TOTAL", "", total,
	    total ? "FAIL" : "ok");
	return (total ? 1 : 0);
}
