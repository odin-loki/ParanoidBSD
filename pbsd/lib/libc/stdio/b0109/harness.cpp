/*
 * PBSD batch b0109 differential test.
 *
 * Every ported function is exercised side by side with the ref_ oracle in
 * oracle.c: hand-written edge cases plus a fixed-seed randomised sweep of at
 * least 200000 iterations per function.  Each case runs the port and the
 * oracle on two separate but identically prepared streams and compares the
 * return value, errno, stream position, EOF/error indicators and the ENTIRE
 * backing buffer -- guard bytes past the nominal write window included.
 * Exit status is 0 only if every case matched.
 *
 * Stream flavours used, and why:
 *	fmemopen()	byte reads out of a guard-filled buffer (fgetc)
 *	tmpfile()	wide writes; glibc rejects wide I/O on memory
 *			streams, so putwc() needs a real file.  The bytes
 *			are read back with pread() into a guard-filled
 *			buffer, because byte reads are illegal once the
 *			stream is wide-oriented
 *	pipe()		non-seekable streams
 *	fopencookie()	a stream whose seek callback succeeds *and* leaves
 *			errno dirty, which is what makes rewind()'s
 *			`errno = serrno' restore observable
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

import pbsd.lib.libc.stdio.b0109;

namespace P = pbsd::lib_libc_stdio::b0109;

extern "C" {
int ref_fgetc(FILE *);
wint_t ref_putwc_l(wchar_t, FILE *, locale_t);
wint_t ref_putwc(wchar_t, FILE *);
int ref_swprintf(wchar_t *__restrict, size_t, const wchar_t *__restrict, ...);
int ref_swprintf_l(wchar_t *__restrict, size_t, locale_t,
    const wchar_t *__restrict, ...);
void ref_rewind(FILE *);
int ref_sdidinit_state(void);
}

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

enum {
	F_FGETC,
	F_PUTWC,
	F_PUTWC_L,
	F_SWPRINTF,
	F_SWPRINTF_L,
	F_REWIND,
	F_NFUNC
};

static const char *fname[F_NFUNC] = {
	"fgetc", "putwc", "putwc_l", "swprintf", "swprintf_l", "rewind"
};

static unsigned long long ncase[F_NFUNC];
static unsigned long long nfail[F_NFUNC];
static unsigned long long nprint[F_NFUNC];

static void
record(int f, int ok, const char *fmt, ...)
{
	ncase[f]++;
	if (ok)
		return;
	nfail[f]++;
	if (nprint[f]++ < 8) {
		va_list ap;

		fprintf(stderr, "FAIL %s: ", fname[f]);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fputc('\n', stderr);
	}
}

/* fixed-seed xorshift64 */
static uint64_t rstate = 0x0123456789abcdefULL;

static inline uint64_t
rnext(void)
{
	rstate ^= rstate << 13;
	rstate ^= rstate >> 7;
	rstate ^= rstate << 17;
	return (rstate);
}

static inline uint32_t
r32(void)
{
	return ((uint32_t)(rnext() >> 32));
}

static inline uint32_t
rlim(uint32_t n)
{
	return (n == 0 ? 0 : r32() % n);
}

static locale_t loc_c;
static locale_t loc_utf8;

/* ------------------------------------------------------------------ */
/* fgetc							      */
/* ------------------------------------------------------------------ */

#define	RGUARD	16
#define	RMAX	72
#define	RTOTAL	(RGUARD + RMAX + RGUARD)

struct rbuf {
	unsigned char raw[RTOTAL];
};

static void
rbuf_init(struct rbuf *b, const unsigned char *data, size_t len)
{
	memset(b->raw, 0x7f, sizeof(b->raw));
	if (len != 0)
		memcpy(b->raw + RGUARD, data, len);
}

/*
 * Drive both streams with nreads calls, comparing everything observable
 * after each individual call, then compare the two backing buffers whole.
 */
static void
fgetc_drive(FILE *a, FILE *b, struct rbuf *ba, struct rbuf *bb, int nreads,
    const char *what)
{
	int i;

	for (i = 0; i < nreads; i++) {
		int ra, rb, ea, eb;
		long pa, pb;
		int fa, fb, ga, gb;

		errno = 0;
		ra = P::fgetc(a);
		ea = errno;
		errno = 0;
		rb = ref_fgetc(b);
		eb = errno;
		pa = ftell(a);
		pb = ftell(b);
		fa = feof(a) != 0;
		fb = feof(b) != 0;
		ga = ferror(a) != 0;
		gb = ferror(b) != 0;
		record(F_FGETC, ra == rb && ea == eb && pa == pb &&
		    fa == fb && ga == gb &&
		    (ba == NULL || memcmp(ba->raw, bb->raw, RTOTAL) == 0),
		    "%s read %d: ret %d/%d errno %d/%d pos %ld/%ld "
		    "eof %d/%d err %d/%d", what, i, ra, rb, ea, eb, pa, pb,
		    fa, fb, ga, gb);
	}
}

static void
fgetc_mem_case(const unsigned char *data, size_t len, int nreads,
    const char *what)
{
	struct rbuf ba, bb;
	FILE *a, *b;

	rbuf_init(&ba, data, len);
	rbuf_init(&bb, data, len);
	a = fmemopen(ba.raw + RGUARD, len, "r");
	b = fmemopen(bb.raw + RGUARD, len, "r");
	if (a == NULL || b == NULL) {
		record(F_FGETC, a == NULL && b == NULL,
		    "%s fmemopen mismatch", what);
		if (a != NULL)
			fclose(a);
		if (b != NULL)
			fclose(b);
		return;
	}
	fgetc_drive(a, b, &ba, &bb, nreads, what);
	fclose(a);
	fclose(b);
	record(F_FGETC, memcmp(ba.raw, bb.raw, RTOTAL) == 0,
	    "%s backing buffers differ", what);
}

static FILE *
tmpfile_with(const unsigned char *data, size_t len)
{
	FILE *fp = tmpfile();

	if (fp == NULL)
		return (NULL);
	if (len != 0)
		fwrite(data, 1, len, fp);
	fflush(fp);
	fseek(fp, 0L, SEEK_SET);
	return (fp);
}

static void
fgetc_file_case(const unsigned char *data, size_t len, int nreads,
    const char *what)
{
	FILE *a = tmpfile_with(data, len);
	FILE *b = tmpfile_with(data, len);

	if (a == NULL || b == NULL) {
		record(F_FGETC, 0, "%s tmpfile failed", what);
		if (a != NULL)
			fclose(a);
		if (b != NULL)
			fclose(b);
		return;
	}
	fgetc_drive(a, b, NULL, NULL, nreads, what);
	fclose(a);
	fclose(b);
}

static void
fgetc_edge(void)
{
	static const unsigned char nul_heavy[] =
	    { 0x00, 0x00, 0x00, 'a', 0x00, 0xff, 0x00 };
	static const unsigned char highbit[] = { 0x80, 0x81, 0xfe, 0xff };
	static const unsigned char one_nul[] = { 0x00 };
	static const unsigned char one_7f[] = { 0x7f };
	static const unsigned char one_80[] = { 0x80 };
	static const unsigned char one_ff[] = { 0xff };
	unsigned char all[64];
	int base, i;

	/* empty stream: EOF straight away, repeatedly */
	fgetc_file_case(NULL, 0, 4, "empty file");
	fgetc_mem_case(one_nul, 0, 4, "empty fmemopen");

	/* single bytes, both sides of the sign-extension boundary */
	fgetc_mem_case(one_nul, 1, 3, "single 0x00");
	fgetc_mem_case(one_7f, 1, 3, "single 0x7f");
	fgetc_mem_case(one_80, 1, 3, "single 0x80");
	fgetc_mem_case(one_ff, 1, 3, "single 0xff");
	fgetc_file_case(one_80, 1, 3, "file single 0x80");
	fgetc_file_case(one_ff, 1, 3, "file single 0xff");

	fgetc_mem_case(nul_heavy, sizeof(nul_heavy),
	    (int)sizeof(nul_heavy) + 3, "NUL heavy");
	fgetc_mem_case(highbit, sizeof(highbit),
	    (int)sizeof(highbit) + 3, "high bit");

	/* every byte value 0x00..0xff */
	for (base = 0; base < 256; base += 64) {
		char lbl[32];

		for (i = 0; i < 64; i++)
			all[i] = (unsigned char)(base + i);
		snprintf(lbl, sizeof(lbl), "all bytes %d..%d", base,
		    base + 63);
		fgetc_mem_case(all, sizeof(all), (int)sizeof(all) + 2, lbl);
		fgetc_file_case(all, sizeof(all), (int)sizeof(all) + 2, lbl);
	}

	/* boundary lengths around the guard window */
	for (i = 0; i <= 8; i++) {
		char lbl[32];

		memset(all, 0xa5, sizeof(all));
		snprintf(lbl, sizeof(lbl), "len %d", i);
		if (i > 0)
			fgetc_mem_case(all, (size_t)i, i + 2, lbl);
		fgetc_file_case(all, (size_t)i, i + 2, lbl);
	}

	/* pushed-back high-bit byte must come back as 0x80..0xff */
	{
		struct rbuf ba, bb;
		FILE *a, *b;

		rbuf_init(&ba, highbit, sizeof(highbit));
		rbuf_init(&bb, highbit, sizeof(highbit));
		a = fmemopen(ba.raw + RGUARD, sizeof(highbit), "r");
		b = fmemopen(bb.raw + RGUARD, sizeof(highbit), "r");
		if (a != NULL && b != NULL) {
			ungetc(0x80, a);
			ungetc(0x80, b);
			fgetc_drive(a, b, &ba, &bb, 2, "ungetc 0x80");
			ungetc(0xff, a);
			ungetc(0xff, b);
			fgetc_drive(a, b, &ba, &bb, 2, "ungetc 0xff");
			ungetc(EOF, a);
			ungetc(EOF, b);
			fgetc_drive(a, b, &ba, &bb, 2, "ungetc EOF");
			fgetc_drive(a, b, &ba, &bb, 8, "ungetc drain");
			fclose(a);
			fclose(b);
		}
	}

	/* stream carrying an error indicator */
	{
		struct rbuf ba, bb;
		FILE *a, *b;

		rbuf_init(&ba, highbit, sizeof(highbit));
		rbuf_init(&bb, highbit, sizeof(highbit));
		a = fmemopen(ba.raw + RGUARD, sizeof(highbit), "r");
		b = fmemopen(bb.raw + RGUARD, sizeof(highbit), "r");
		if (a != NULL && b != NULL) {
			fputc('x', a);
			fputc('x', b);
			fgetc_drive(a, b, &ba, &bb, 6, "error state");
			fclose(a);
			fclose(b);
		}
	}

	/* pipe: non-seekable */
	{
		int fda[2], fdb[2];

		if (pipe(fda) == 0 && pipe(fdb) == 0) {
			FILE *a, *b;

			ssize_t w1 = write(fda[1], highbit, sizeof(highbit));
			ssize_t w2 = write(fdb[1], highbit, sizeof(highbit));

			(void)w1;
			(void)w2;
			close(fda[1]);
			close(fdb[1]);
			a = fdopen(fda[0], "r");
			b = fdopen(fdb[0], "r");
			if (a != NULL && b != NULL) {
				fgetc_drive(a, b, NULL, NULL,
				    (int)sizeof(highbit) + 3, "pipe");
				fclose(a);
				fclose(b);
			} else {
				close(fda[0]);
				close(fdb[0]);
			}
		}
	}
}

static void
fgetc_sweep(unsigned long iters)
{
	unsigned long done = 0;

	while (done < iters) {
		struct rbuf ba, bb;
		unsigned char data[RMAX];
		size_t len = (size_t)(1 + rlim(RMAX));
		int nreads = (int)len + 1 + (int)rlim(3);
		FILE *a, *b;
		size_t i;

		for (i = 0; i < len; i++)
			data[i] = (unsigned char)r32();
		rbuf_init(&ba, data, len);
		rbuf_init(&bb, data, len);
		a = fmemopen(ba.raw + RGUARD, len, "r");
		b = fmemopen(bb.raw + RGUARD, len, "r");
		if (a == NULL || b == NULL) {
			record(F_FGETC, 0, "sweep fmemopen failed");
			if (a != NULL)
				fclose(a);
			if (b != NULL)
				fclose(b);
			return;
		}
		if ((r32() & 3) == 0) {
			int pb = (int)(unsigned char)r32();

			ungetc(pb, a);
			ungetc(pb, b);
			nreads++;
		}
		fgetc_drive(a, b, &ba, &bb, nreads, "sweep");
		fclose(a);
		fclose(b);
		record(F_FGETC, memcmp(ba.raw, bb.raw, RTOTAL) == 0,
		    "sweep backing buffers differ");
		done += (unsigned long)nreads + 1;
	}
}

/* ------------------------------------------------------------------ */
/* putwc / putwc_l						      */
/* ------------------------------------------------------------------ */

#define	PREAD	256		/* bytes read back from the file	*/
#define	PTOTAL	(PREAD + 32)	/* ... into this much guarded space	*/
#define	PSTEPS	32		/* at most 4 bytes per step		*/

struct pbuf {
	unsigned char raw[PTOTAL];
	ssize_t nread;
};

/*
 * Flush the stream and slurp the file it is backed by into a buffer
 * pre-filled with the guard byte.  Anything the implementation wrote past
 * the bytes it should have written shows up as a difference.
 */
static void
putwc_readback(FILE *fp, struct pbuf *b)
{
	memset(b->raw, 0x7f, sizeof(b->raw));
	fflush(fp);
	b->nread = pread(fileno(fp), b->raw, PREAD, 0);
}

static void
putwc_step(FILE *a, FILE *b, wchar_t wc, int use_l, locale_t loc,
    const char *what, int step)
{
	struct pbuf pa_, pb_;
	wint_t ra, rb;
	int ea, eb, fa, fb, ga, gb, f;
	long pa, pb;

	f = use_l ? F_PUTWC_L : F_PUTWC;
	errno = 0;
	if (use_l)
		ra = P::putwc_l(wc, a, loc);
	else
		ra = P::putwc(wc, a);
	ea = errno;
	errno = 0;
	if (use_l)
		rb = ref_putwc_l(wc, b, loc);
	else
		rb = ref_putwc(wc, b);
	eb = errno;
	pa = ftell(a);
	pb = ftell(b);
	fa = feof(a) != 0;
	fb = feof(b) != 0;
	ga = ferror(a) != 0;
	gb = ferror(b) != 0;
	putwc_readback(a, &pa_);
	putwc_readback(b, &pb_);
	record(f, ra == rb && ea == eb && pa == pb && fa == fb && ga == gb &&
	    pa_.nread == pb_.nread &&
	    memcmp(pa_.raw, pb_.raw, PTOTAL) == 0,
	    "%s step %d wc=0x%lx: ret 0x%lx/0x%lx errno %d/%d pos %ld/%ld "
	    "eof %d/%d err %d/%d read %zd/%zd bufdiff %d", what, step,
	    (unsigned long)wc, (unsigned long)ra, (unsigned long)rb, ea, eb,
	    pa, pb, fa, fb, ga, gb, pa_.nread, pb_.nread,
	    memcmp(pa_.raw, pb_.raw, PTOTAL));
}

static void
putwc_case(const wchar_t *wcs, size_t nwc, int use_l, locale_t loc,
    locale_t thread_loc, const char *what)
{
	struct pbuf pa_, pb_;
	FILE *a, *b;
	locale_t saved;
	size_t i;
	int f = use_l ? F_PUTWC_L : F_PUTWC;

	a = tmpfile();
	b = tmpfile();
	if (a == NULL || b == NULL) {
		record(f, 0, "%s tmpfile failed", what);
		if (a != NULL)
			fclose(a);
		if (b != NULL)
			fclose(b);
		return;
	}
	saved = uselocale(thread_loc);
	for (i = 0; i < nwc && i < PSTEPS; i++)
		putwc_step(a, b, wcs[i], use_l, loc, what, (int)i);
	putwc_readback(a, &pa_);
	putwc_readback(b, &pb_);
	uselocale(saved);
	record(f, pa_.nread == pb_.nread &&
	    memcmp(pa_.raw, pb_.raw, PTOTAL) == 0,
	    "%s final contents: %zd/%zd bytes bufdiff %d", what, pa_.nread,
	    pb_.nread, memcmp(pa_.raw, pb_.raw, PTOTAL));
	fclose(a);
	fclose(b);
}

static void
putwc_edge(void)
{
	static const wchar_t singles[] = {
		L'\0', L'a', (wchar_t)0x7f, (wchar_t)0x80, (wchar_t)0xff,
		(wchar_t)0x100, (wchar_t)0x7ff, (wchar_t)0x800,
		(wchar_t)0xd800, (wchar_t)0xdfff, (wchar_t)0xfffd,
		(wchar_t)0xffff, (wchar_t)0x10000, (wchar_t)0x10ffff,
		(wchar_t)0x110000, (wchar_t)0x7fffffff, (wchar_t)-1,
		(wchar_t)-2
	};
	static const wchar_t mixed[] = {
		L'a', L'\0', (wchar_t)0xe9, L'z', (wchar_t)0x20ac, L'\0',
		(wchar_t)0xff, L'Q', (wchar_t)0x10348, L'\n', (wchar_t)0x80,
		(wchar_t)0x7f
	};
	static const wchar_t bad_then_good[] = {
		(wchar_t)0xd800, L'a', (wchar_t)0x110000, L'b'
	};
	struct combo {
		locale_t *loc;
		locale_t *tloc;
		const char *tag;
	};
	static locale_t nullloc = NULL;
	struct combo combos[5] = {
		{ &loc_c, &loc_c, "C/C" },
		{ &loc_utf8, &loc_c, "U/C" },
		{ &loc_c, &loc_utf8, "C/U" },
		{ &nullloc, &loc_utf8, "NULL/U" },
		{ &nullloc, &loc_c, "NULL/C" }
	};
	size_t i, ci, v;

	for (v = 0; v < 2; v++) {
		int use_l = (int)v;

		for (ci = 0; ci < 5; ci++) {
			for (i = 0; i < sizeof(singles) / sizeof(singles[0]);
			    i++) {
				char lbl[80];

				snprintf(lbl, sizeof(lbl),
				    "single%s wc=0x%lx %s",
				    use_l ? "_l" : "",
				    (unsigned long)singles[i],
				    combos[ci].tag);
				putwc_case(&singles[i], 1, use_l,
				    *combos[ci].loc, *combos[ci].tloc, lbl);
			}
			{
				char lbl[80];

				snprintf(lbl, sizeof(lbl), "mixed%s %s",
				    use_l ? "_l" : "", combos[ci].tag);
				putwc_case(mixed,
				    sizeof(mixed) / sizeof(mixed[0]), use_l,
				    *combos[ci].loc, *combos[ci].tloc, lbl);

				snprintf(lbl, sizeof(lbl),
				    "bad-then-good%s %s", use_l ? "_l" : "",
				    combos[ci].tag);
				putwc_case(bad_then_good,
				    sizeof(bad_then_good) /
				    sizeof(bad_then_good[0]), use_l,
				    *combos[ci].loc, *combos[ci].tloc, lbl);
			}
		}
	}

	/* a long ASCII run */
	{
		wchar_t run[PSTEPS];
		size_t i2;

		for (i2 = 0; i2 < sizeof(run) / sizeof(run[0]); i2++)
			run[i2] = (wchar_t)('A' + (int)(i2 % 26));
		putwc_case(run, sizeof(run) / sizeof(run[0]), 0, loc_c,
		    loc_c, "ascii run C/C");
		putwc_case(run, sizeof(run) / sizeof(run[0]), 1, loc_utf8,
		    loc_c, "ascii run_l U/C");
	}

	/* non-seekable target: ftell fails, pread fails, both alike */
	{
		int fda[2], fdb[2];

		if (pipe(fda) == 0 && pipe(fdb) == 0) {
			FILE *a = fdopen(fda[1], "w");
			FILE *b = fdopen(fdb[1], "w");

			if (a != NULL && b != NULL) {
				locale_t saved = uselocale(loc_utf8);
				int k;

				for (k = 0; k < 4; k++)
					putwc_step(a, b,
					    (wchar_t)(0x20ac + k), k & 1,
					    loc_utf8, "pipe", k);
				uselocale(saved);
				fclose(a);
				fclose(b);
			} else {
				close(fda[1]);
				close(fdb[1]);
			}
			close(fda[0]);
			close(fdb[0]);
		}
	}
}

static wchar_t
rand_wchar(void)
{
	switch (rlim(6)) {
	case 0:
		return ((wchar_t)rlim(0x80));
	case 1:
		return ((wchar_t)(0x80 + rlim(0x780)));
	case 2:
		return ((wchar_t)(0x800 + rlim(0xf800)));
	case 3:
		return ((wchar_t)(0x10000 + rlim(0x100000)));
	case 4:
		return ((wchar_t)r32());
	default:
		return ((wchar_t)(rlim(2) ? 0 : 0x110000 + rlim(0x1000)));
	}
}

static void
putwc_sweep(unsigned long iters, int use_l)
{
	unsigned long done = 0;
	int f = use_l ? F_PUTWC_L : F_PUTWC;

	while (done < iters) {
		struct pbuf pa_, pb_;
		FILE *a, *b;
		locale_t loc, tloc, saved;
		int steps = 1 + (int)rlim(PSTEPS);
		int i;

		switch (rlim(3)) {
		case 0:
			loc = loc_c;
			break;
		case 1:
			loc = loc_utf8;
			break;
		default:
			loc = NULL;
			break;
		}
		tloc = (rlim(2) == 0) ? loc_c : loc_utf8;
		a = tmpfile();
		b = tmpfile();
		if (a == NULL || b == NULL) {
			record(f, 0, "sweep tmpfile failed");
			if (a != NULL)
				fclose(a);
			if (b != NULL)
				fclose(b);
			return;
		}
		saved = uselocale(tloc);
		for (i = 0; i < steps; i++) {
			putwc_step(a, b, rand_wchar(), use_l, loc, "sweep",
			    i);
			if (rlim(4) == 0) {
				clearerr(a);
				clearerr(b);
			}
		}
		putwc_readback(a, &pa_);
		putwc_readback(b, &pb_);
		uselocale(saved);
		record(f, pa_.nread == pb_.nread &&
		    memcmp(pa_.raw, pb_.raw, PTOTAL) == 0,
		    "sweep final contents: %zd/%zd bytes", pa_.nread,
		    pb_.nread);
		fclose(a);
		fclose(b);
		done += (unsigned long)steps + 1;
	}
}

/* ------------------------------------------------------------------ */
/* swprintf / swprintf_l					      */
/* ------------------------------------------------------------------ */

#define	SBUFW	64	/* wchar_t slots in each buffer		*/
#define	SMAXN	48	/* largest n handed to swprintf		*/

#define	SWP(n, ...)							\
do {									\
	wchar_t A_[SBUFW], B_[SBUFW];					\
	int ra_, rb_, ea_, eb_, bd_;					\
									\
	memset(A_, 0x7f, sizeof(A_));					\
	memset(B_, 0x7f, sizeof(B_));					\
	errno = 0;							\
	ra_ = P::swprintf(A_, (n), __VA_ARGS__);			\
	ea_ = errno;							\
	errno = 0;							\
	rb_ = ref_swprintf(B_, (n), __VA_ARGS__);			\
	eb_ = errno;							\
	bd_ = memcmp(A_, B_, sizeof(A_));				\
	record(F_SWPRINTF, ra_ == rb_ && ea_ == eb_ && bd_ == 0,	\
	    "n=%zu ret %d/%d errno %d/%d bufdiff %d", (size_t)(n),	\
	    ra_, rb_, ea_, eb_, bd_);					\
} while (0)

#define	SWPL(n, loc, ...)						\
do {									\
	wchar_t A_[SBUFW], B_[SBUFW];					\
	int ra_, rb_, ea_, eb_, bd_;					\
									\
	memset(A_, 0x7f, sizeof(A_));					\
	memset(B_, 0x7f, sizeof(B_));					\
	errno = 0;							\
	ra_ = P::swprintf_l(A_, (n), (loc), __VA_ARGS__);		\
	ea_ = errno;							\
	errno = 0;							\
	rb_ = ref_swprintf_l(B_, (n), (loc), __VA_ARGS__);		\
	eb_ = errno;							\
	bd_ = memcmp(A_, B_, sizeof(A_));				\
	record(F_SWPRINTF_L, ra_ == rb_ && ea_ == eb_ && bd_ == 0,	\
	    "n=%zu ret %d/%d errno %d/%d bufdiff %d", (size_t)(n),	\
	    ra_, rb_, ea_, eb_, bd_);					\
} while (0)

/*
 * %s converts a multibyte string with the *active* locale, so these strings
 * make swprintf_l() locale-sensitive: valid UTF-8 succeeds under C.utf8 and
 * fails with EILSEQ under C, and high-bit bytes that are not UTF-8 do the
 * opposite.
 */
static const char *const nstr[] = {
	"", "a", "ab", "abcdefghij",
	"0123456789012345678901234567890123456789",
	"caf\xc3\xa9", "\xe2\x82\xac", "\xc3\xa9\xc3\xa8\xc3\xaa",
	"a\xc3\xa9z", "\x80\xff", "\xff", "\xc3", "x\x80y"
};
#define	NNSTR	(sizeof(nstr) / sizeof(nstr[0]))

static const wchar_t *const wstr[] = {
	L"", L"a", L"ab", L"abcdefghij",
	L"0123456789012345678901234567890123456789",
	L"caf\u00e9", L"\u20ac\u00ff", L"\U00010348"
};
#define	NWSTR	(sizeof(wstr) / sizeof(wstr[0]))

static void
swprintf_edge_body(const char *tag)
{
	static const wchar_t hi[] = { (wchar_t)0xe9, (wchar_t)0x20ac,
	    (wchar_t)0xff, 0 };
	locale_t locs[3];
	size_t n, i, li;

	(void)tag;
	locs[0] = loc_c;
	locs[1] = loc_utf8;
	locs[2] = NULL;

	/* boundary n: 0, 1 and either side of the exact fit */
	for (n = 0; n <= 8; n++) {
		SWP(n, L"");
		SWP(n, L"a");
		SWP(n, L"ab");
		SWP(n, L"abc");
		SWP(n, L"abcdefg");
		SWP(n, L"%d", 0);
		SWP(n, L"%d", -1);
		SWP(n, L"%d", INT_MIN);
		SWP(n, L"%d", INT_MAX);
		SWP(n, L"%u", (unsigned)UINT_MAX);
		SWP(n, L"%lc", (wint_t)0);
		SWP(n, L"%lc", (wint_t)L'Z');
		SWP(n, L"%c", 'q');
		SWP(n, L"%%");
		SWP(n, L"%s%s", "", "");
		SWP(n, L"%ls|%d", L"xy", 42);
		SWP(n, L"%s", "caf\xc3\xa9");
		SWP(n, L"%s", "\x80\xff");
		for (li = 0; li < 3; li++) {
			SWPL(n, locs[li], L"");
			SWPL(n, locs[li], L"abc");
			SWPL(n, locs[li], L"%d", -12345);
			SWPL(n, locs[li], L"%ls", hi);
			SWPL(n, locs[li], L"%lc", (wint_t)0x20ac);
			SWPL(n, locs[li], L"%s", "caf\xc3\xa9");
			SWPL(n, locs[li], L"%s", "\x80\xff");
			SWPL(n, locs[li], L"%s", "\xe2\x82\xac");
		}
	}

	/* exact-fit and off-by-one around every string length */
	for (i = 0; i < NNSTR; i++) {
		size_t len = strlen(nstr[i]);

		if (len + 2 > SMAXN)
			continue;
		SWP(len, L"%s", nstr[i]);
		SWP(len + 1, L"%s", nstr[i]);
		SWP(len + 2, L"%s", nstr[i]);
		SWPL(len, loc_utf8, L"%s", nstr[i]);
		SWPL(len + 1, loc_utf8, L"%s", nstr[i]);
		SWPL(len + 1, loc_c, L"%s", nstr[i]);
		SWPL(len + 1, NULL, L"%s", nstr[i]);
	}
	for (i = 0; i < NWSTR; i++) {
		size_t len = wcslen(wstr[i]);

		if (len + 2 > SMAXN)
			continue;
		SWP(len, L"%ls", wstr[i]);
		SWP(len + 1, L"%ls", wstr[i]);
		SWP(len + 2, L"%ls", wstr[i]);
		SWPL(len + 1, NULL, L"%ls", wstr[i]);
		SWPL(len + 1, loc_c, L"%ls", wstr[i]);
	}

	/* NUL-heavy output */
	SWP((size_t)8, L"%lc%lc%lc", (wint_t)0, (wint_t)0, (wint_t)0);
	SWP((size_t)3, L"%lc%lc%lc", (wint_t)0, (wint_t)0, (wint_t)0);
	SWP((size_t)4, L"a%lcb", (wint_t)0);
	SWPL((size_t)4, loc_utf8, L"a%lcb", (wint_t)0);

	/* high-bit / multibyte material */
	SWP((size_t)SMAXN, L"%ls", hi);
	SWP((size_t)SMAXN, L"%s", "caf\xc3\xa9");
	SWPL((size_t)SMAXN, loc_utf8, L"%ls", hi);
	SWPL((size_t)SMAXN, loc_utf8, L"%s", "caf\xc3\xa9");
	SWPL((size_t)SMAXN, loc_c, L"%ls", hi);
	SWPL((size_t)SMAXN, loc_c, L"%s", "caf\xc3\xa9");
	SWPL((size_t)SMAXN, loc_utf8, L"%s", "\x80\xff");
	SWPL((size_t)SMAXN, loc_c, L"%s", "\x80\xff");
	SWPL((size_t)SMAXN, NULL, L"%s", "caf\xc3\xa9");

	/* numeric conversions, widths, precisions */
	SWP((size_t)SMAXN, L"%x %X %o", 0xdeadbeefu, 0xdeadbeefu, 0777u);
	SWP((size_t)SMAXN, L"%.3f|%e|%g", 3.14159, 3.14159, 3.14159);
	SWP((size_t)SMAXN, L"%.3f", -0.0);
	SWP((size_t)SMAXN, L"%*d", 12, 7);
	SWP((size_t)SMAXN, L"%-*d|", 12, 7);
	SWP((size_t)SMAXN, L"%.*s", 3, "abcdefgh");
	SWP((size_t)SMAXN, L"%lld %llu", (long long)LLONG_MIN,
	    (unsigned long long)ULLONG_MAX);
	SWP((size_t)SMAXN, L"%zu %ld", (size_t)0, (long)-1);
	SWP((size_t)5, L"%.3f|%e|%g", 3.14159, 3.14159, 3.14159);
	SWP((size_t)1, L"%x", 0xffffffffu);
	SWP((size_t)2, L"%x", 0xffffffffu);
	SWPL((size_t)SMAXN, loc_utf8, L"%.3f", 3.14159);
	SWPL((size_t)SMAXN, loc_c, L"%.3f", 3.14159);
	SWPL((size_t)6, loc_c, L"%lld", (long long)LLONG_MIN);

	/* output far longer than the buffer */
	SWP((size_t)SMAXN, L"%s%s%s%s", nstr[4], nstr[4], nstr[4], nstr[4]);
	SWPL((size_t)SMAXN, loc_utf8, L"%ls%ls%ls", wstr[4], wstr[4],
	    wstr[4]);
}

static void
swprintf_edge(void)
{
	locale_t saved = uselocale(loc_c);

	swprintf_edge_body("thread=C");
	uselocale(loc_utf8);
	swprintf_edge_body("thread=UTF-8");
	uselocale(saved);
}

static void
swprintf_sweep(unsigned long iters, int use_l)
{
	unsigned long k;
	locale_t saved = uselocale((locale_t)0);

	for (k = 0; k < iters; k++) {
		size_t n = (size_t)rlim(SMAXN + 1);
		locale_t loc, tloc;
		int shape = (int)rlim(14);
		int iv = (int)r32();
		unsigned uv = r32();
		const char *sv = nstr[rlim(NNSTR)];
		const wchar_t *wv = wstr[rlim(NWSTR)];
		wint_t cv = (wint_t)rand_wchar();
		int wid = (int)rlim(20);
		int prec = (int)rlim(12);
		int chr = (int)(uv & 0x7f);
		double dv;

		switch (rlim(3)) {
		case 0:
			loc = loc_c;
			break;
		case 1:
			loc = loc_utf8;
			break;
		default:
			loc = NULL;
			break;
		}
		tloc = (rlim(2) == 0) ? loc_c : loc_utf8;
		uselocale(tloc);
		dv = (double)(int)r32() / (double)(1 + rlim(1000));

		if (!use_l) {
			switch (shape) {
			case 0:
				SWP(n, L"%d", iv);
				break;
			case 1:
				SWP(n, L"%u", uv);
				break;
			case 2:
				SWP(n, L"%x", uv);
				break;
			case 3:
				SWP(n, L"%s", sv);
				break;
			case 4:
				SWP(n, L"%ls", wv);
				break;
			case 5:
				SWP(n, L"%lc", cv);
				break;
			case 6:
				SWP(n, L"%c", chr);
				break;
			case 7:
				SWP(n, L"%.3f", dv);
				break;
			case 8:
				SWP(n, L"%*d", wid, iv);
				break;
			case 9:
				SWP(n, L"%.*s", prec, sv);
				break;
			case 10:
				SWP(n, L"%%%d%%", iv);
				break;
			case 11:
				SWP(n, L"%ls%d%lc", wv, iv, cv);
				break;
			case 12:
				SWP(n, L"lit");
				break;
			default:
				SWP(n, L"%s|%ls|%d", sv, wv, iv);
				break;
			}
		} else {
			switch (shape) {
			case 0:
				SWPL(n, loc, L"%d", iv);
				break;
			case 1:
				SWPL(n, loc, L"%u", uv);
				break;
			case 2:
				SWPL(n, loc, L"%x", uv);
				break;
			case 3:
				SWPL(n, loc, L"%s", sv);
				break;
			case 4:
				SWPL(n, loc, L"%ls", wv);
				break;
			case 5:
				SWPL(n, loc, L"%lc", cv);
				break;
			case 6:
				SWPL(n, loc, L"%c", chr);
				break;
			case 7:
				SWPL(n, loc, L"%.3f", dv);
				break;
			case 8:
				SWPL(n, loc, L"%*d", wid, iv);
				break;
			case 9:
				SWPL(n, loc, L"%.*s", prec, sv);
				break;
			case 10:
				SWPL(n, loc, L"%%%d%%", iv);
				break;
			case 11:
				SWPL(n, loc, L"%ls%d%lc", wv, iv, cv);
				break;
			case 12:
				SWPL(n, loc, L"lit");
				break;
			default:
				SWPL(n, loc, L"%s|%ls|%d", sv, wv, iv);
				break;
			}
		}
	}
	uselocale(saved);
}

/* ------------------------------------------------------------------ */
/* rewind							      */
/* ------------------------------------------------------------------ */

static const unsigned char rw_data[40] = {
	0x00, 0x01, 0x7f, 0x80, 0xff, 'a', 'b', 0x00,
	0xfe, 0x81, 0x10, 0x20, 0x00, 0x00, 0x55, 0xaa,
	'x', 'y', 'z', 0x00, 0x7f, 0x80, 0x01, 0x02,
	0x03, 0x04, 0xf0, 0x0f, 0x00, 0xff, 'A', 'B',
	'C', 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66
};

/*
 * A cookie stream whose seek callback succeeds but leaves errno set.  This
 * is what makes rewind()'s `errno = serrno' restore observable: on every
 * other stream flavour a successful seek leaves errno alone, so dropping the
 * restore would be invisible.
 */
struct ck {
	size_t pos;
	int seek_errno;
	int seek_fail;
};

extern "C" {
static ssize_t
ck_read(void *c, char *buf, size_t n)
{
	struct ck *k = (struct ck *)c;
	size_t avail;

	if (k->pos >= sizeof(rw_data))
		return (0);
	avail = sizeof(rw_data) - k->pos;
	if (n > avail)
		n = avail;
	memcpy(buf, rw_data + k->pos, n);
	k->pos += n;
	return ((ssize_t)n);
}

static int
ck_seek(void *c, off64_t *off, int whence)
{
	struct ck *k = (struct ck *)c;

	errno = k->seek_errno;
	if (k->seek_fail)
		return (-1);
	if (whence == SEEK_SET)
		k->pos = (size_t)*off;
	else if (whence == SEEK_CUR)
		k->pos = (size_t)((off64_t)k->pos + *off);
	else
		k->pos = (size_t)((off64_t)sizeof(rw_data) + *off);
	*off = (off64_t)k->pos;
	return (0);
}

static int
ck_close(void *c)
{
	(void)c;
	return (0);
}
}

static FILE *
ck_open(struct ck *k, int seek_errno, int seek_fail)
{
	cookie_io_functions_t fns;

	fns.read = ck_read;
	fns.write = NULL;
	fns.seek = ck_seek;
	fns.close = ck_close;
	k->pos = 0;
	k->seek_errno = seek_errno;
	k->seek_fail = seek_fail;
	return (fopencookie(k, "r", fns));
}

/*
 * rewind() both streams and compare everything the caller can see
 * afterwards: errno (restored or not), position, indicators, the first byte
 * readable at the new position, and the private init flag.
 */
static void
rewind_pair(FILE *a, FILE *b, int serrno, int readback, const char *what)
{
	int ea, eb, fa, fb, ga, gb;
	long pa, pb;
	int ca = 0, cb = 0;

	errno = serrno;
	P::rewind(a);
	ea = errno;
	errno = serrno;
	ref_rewind(b);
	eb = errno;
	pa = ftell(a);
	pb = ftell(b);
	fa = feof(a) != 0;
	fb = feof(b) != 0;
	ga = ferror(a) != 0;
	gb = ferror(b) != 0;
	if (readback) {
		ca = getc(a);
		cb = getc(b);
	}
	record(F_REWIND, ea == eb && pa == pb && fa == fb && ga == gb &&
	    ca == cb && P::sdidinit_state() == ref_sdidinit_state(),
	    "%s: errno %d/%d pos %ld/%ld eof %d/%d err %d/%d byte %d/%d "
	    "didinit %d/%d", what, ea, eb, pa, pb, fa, fb, ga, gb, ca, cb,
	    P::sdidinit_state(), ref_sdidinit_state());
}

static void
rewind_edge(void)
{
	FILE *a, *b;
	int i;

	/* the private init flag: unset before the first call, set after */
	record(F_REWIND, P::sdidinit_state() == ref_sdidinit_state() &&
	    P::sdidinit_state() == 0, "sdidinit before first rewind: %d/%d",
	    P::sdidinit_state(), ref_sdidinit_state());

	/* fresh file, already at offset 0 */
	a = tmpfile_with(rw_data, sizeof(rw_data));
	b = tmpfile_with(rw_data, sizeof(rw_data));
	if (a != NULL && b != NULL) {
		rewind_pair(a, b, 12345, 1, "fresh file");
		record(F_REWIND, P::sdidinit_state() ==
		    ref_sdidinit_state() && P::sdidinit_state() == 1,
		    "sdidinit after first rewind: %d/%d",
		    P::sdidinit_state(), ref_sdidinit_state());

		/* mid-file */
		for (i = 0; i < 10; i++) {
			getc(a);
			getc(b);
		}
		rewind_pair(a, b, 0, 1, "after 10 reads");

		/* at EOF */
		while (getc(a) != EOF)
			;
		while (getc(b) != EOF)
			;
		rewind_pair(a, b, ERANGE, 1, "at EOF");

		/* seeked to the very end, then to offset 1 */
		fseek(a, 0L, SEEK_END);
		fseek(b, 0L, SEEK_END);
		rewind_pair(a, b, EDOM, 1, "after SEEK_END");
		fseek(a, 1L, SEEK_SET);
		fseek(b, 1L, SEEK_SET);
		rewind_pair(a, b, EDOM, 1, "after seek to 1");
		fclose(a);
		fclose(b);
	} else {
		record(F_REWIND, 0, "tmpfile failed");
		if (a != NULL)
			fclose(a);
		if (b != NULL)
			fclose(b);
	}

	/* read-only memory stream carrying an error indicator */
	{
		struct rbuf ba, bb;

		rbuf_init(&ba, rw_data, sizeof(rw_data));
		rbuf_init(&bb, rw_data, sizeof(rw_data));
		a = fmemopen(ba.raw + RGUARD, sizeof(rw_data), "r");
		b = fmemopen(bb.raw + RGUARD, sizeof(rw_data), "r");
		if (a != NULL && b != NULL) {
			fputc('x', a);
			fputc('x', b);
			record(F_REWIND, ferror(a) != 0 && ferror(b) != 0,
			    "error indicator setup: %d/%d", ferror(a),
			    ferror(b));
			rewind_pair(a, b, 4242, 1, "error indicator set");
			record(F_REWIND, memcmp(ba.raw, bb.raw, RTOTAL) == 0,
			    "read-only stream buffers differ");
			fclose(a);
			fclose(b);
		}
	}

	/* write stream: rewind must flush, so the whole file must match */
	{
		struct rbuf ba, bb;
		FILE *fa = tmpfile();
		FILE *fb = tmpfile();

		if (fa != NULL && fb != NULL) {
			fwrite(rw_data, 1, sizeof(rw_data), fa);
			fwrite(rw_data, 1, sizeof(rw_data), fb);
			rewind_pair(fa, fb, 999, 0, "write stream");
			rbuf_init(&ba, NULL, 0);
			rbuf_init(&bb, NULL, 0);
			{
				size_t na = fread(ba.raw + RGUARD, 1, RMAX,
				    fa);
				size_t nb = fread(bb.raw + RGUARD, 1, RMAX,
				    fb);

				record(F_REWIND, na == nb &&
				    memcmp(ba.raw, bb.raw, RTOTAL) == 0,
				    "write stream contents: %zu/%zu bytes",
				    na, nb);
				record(F_REWIND,
				    na == sizeof(rw_data) &&
				    memcmp(ba.raw + RGUARD, rw_data,
				    sizeof(rw_data)) == 0,
				    "write stream flushed from offset 0: "
				    "%zu bytes", na);
			}
			fclose(fa);
			fclose(fb);
		} else {
			record(F_REWIND, 0, "tmpfile (write) failed");
			if (fa != NULL)
				fclose(fa);
			if (fb != NULL)
				fclose(fb);
		}
	}

	/* non-seekable stream: the seek fails, so errno is NOT restored */
	{
		int fda[2], fdb[2];

		if (pipe(fda) == 0 && pipe(fdb) == 0) {
			ssize_t w1 = write(fda[1], rw_data, sizeof(rw_data));
			ssize_t w2 = write(fdb[1], rw_data, sizeof(rw_data));

			(void)w1;
			(void)w2;
			close(fda[1]);
			close(fdb[1]);
			a = fdopen(fda[0], "r");
			b = fdopen(fdb[0], "r");
			if (a != NULL && b != NULL) {
				rewind_pair(a, b, 7777, 1, "pipe fresh");
				getc(a);
				getc(b);
				rewind_pair(a, b, 0, 1, "pipe mid");
				rewind_pair(a, b, ERANGE, 0, "pipe again");
				fclose(a);
				fclose(b);
			} else {
				close(fda[0]);
				close(fdb[0]);
			}
		}
	}

	/* cookie stream: successful seek, dirty errno -> restore is visible */
	{
		static const int errs[] = { ENOTTY, EACCES, EBADF, 0, ERANGE };
		size_t ei;

		for (ei = 0; ei < sizeof(errs) / sizeof(errs[0]); ei++) {
			struct ck ka, kb;
			char lbl[64];

			a = ck_open(&ka, errs[ei], 0);
			b = ck_open(&kb, errs[ei], 0);
			if (a == NULL || b == NULL) {
				record(F_REWIND, 0, "fopencookie failed");
				if (a != NULL)
					fclose(a);
				if (b != NULL)
					fclose(b);
				continue;
			}
			snprintf(lbl, sizeof(lbl),
			    "cookie ok-seek dirty errno %d", errs[ei]);
			rewind_pair(a, b, 12345, 1, lbl);
			getc(a);
			getc(b);
			rewind_pair(a, b, 0, 1, lbl);
			rewind_pair(a, b, EDOM, 1, lbl);
			while (getc(a) != EOF)
				;
			while (getc(b) != EOF)
				;
			rewind_pair(a, b, 4242, 1, lbl);
			fclose(a);
			fclose(b);
		}

		/* and with the seek callback failing */
		for (ei = 0; ei < sizeof(errs) / sizeof(errs[0]); ei++) {
			struct ck ka, kb;
			char lbl[64];

			a = ck_open(&ka, errs[ei], 1);
			b = ck_open(&kb, errs[ei], 1);
			if (a == NULL || b == NULL) {
				record(F_REWIND, 0, "fopencookie failed");
				if (a != NULL)
					fclose(a);
				if (b != NULL)
					fclose(b);
				continue;
			}
			snprintf(lbl, sizeof(lbl),
			    "cookie failing seek errno %d", errs[ei]);
			rewind_pair(a, b, 12345, 1, lbl);
			rewind_pair(a, b, 0, 0, lbl);
			fclose(a);
			fclose(b);
		}
	}
}

static void
rewind_sweep(unsigned long iters)
{
	static const int serrnos[] = { 0, ERANGE, EDOM, ESPIPE, EINVAL,
	    12345 };
	static const int ckerrnos[] = { ENOTTY, EACCES, 0, EBADF };
	struct rbuf ba, bb;
	struct ck ka, kb;
	FILE *fa, *fb, *ma, *mb, *pa, *pb, *ca, *cb;
	int fda[2], fdb[2];
	unsigned long k;

	fa = tmpfile_with(rw_data, sizeof(rw_data));
	fb = tmpfile_with(rw_data, sizeof(rw_data));
	rbuf_init(&ba, rw_data, sizeof(rw_data));
	rbuf_init(&bb, rw_data, sizeof(rw_data));
	ma = fmemopen(ba.raw + RGUARD, sizeof(rw_data), "r");
	mb = fmemopen(bb.raw + RGUARD, sizeof(rw_data), "r");
	ca = ck_open(&ka, ENOTTY, 0);
	cb = ck_open(&kb, ENOTTY, 0);
	pa = NULL;
	pb = NULL;
	if (pipe(fda) == 0 && pipe(fdb) == 0) {
		close(fda[1]);
		close(fdb[1]);
		pa = fdopen(fda[0], "r");
		pb = fdopen(fdb[0], "r");
	}
	if (fa == NULL || fb == NULL || ma == NULL || mb == NULL ||
	    ca == NULL || cb == NULL) {
		record(F_REWIND, 0, "sweep stream setup failed");
		return;
	}

	for (k = 0; k < iters; k++) {
		FILE *a, *b;
		int which = (int)rlim(12);
		int serrno = serrnos[rlim(6)];
		int j, nops;

		if (which < 4) {
			a = fa;
			b = fb;
		} else if (which < 7) {
			a = ma;
			b = mb;
		} else if (which < 11) {
			int e = ckerrnos[rlim(4)];
			int fail = (int)rlim(4) == 0;

			ka.seek_errno = e;
			kb.seek_errno = e;
			ka.seek_fail = fail;
			kb.seek_fail = fail;
			a = ca;
			b = cb;
		} else if (pa != NULL) {
			a = pa;
			b = pb;
		} else {
			a = fa;
			b = fb;
		}

		/* put both streams into some identical state first */
		nops = (int)rlim(4);
		for (j = 0; j < nops; j++) {
			switch (rlim(4)) {
			case 0:
				getc(a);
				getc(b);
				break;
			case 1: {
				long off = (long)rlim(sizeof(rw_data) + 4);

				fseek(a, off, SEEK_SET);
				fseek(b, off, SEEK_SET);
				break;
			}
			case 2:
				fseek(a, 0L, SEEK_END);
				fseek(b, 0L, SEEK_END);
				break;
			default:
				fputc('Z', a);
				fputc('Z', b);
				break;
			}
		}
		rewind_pair(a, b, serrno, (int)rlim(2), "sweep");
	}

	record(F_REWIND, memcmp(ba.raw, bb.raw, RTOTAL) == 0,
	    "sweep memory stream buffers differ");
	fclose(fa);
	fclose(fb);
	fclose(ma);
	fclose(mb);
	ka.seek_fail = 0;
	kb.seek_fail = 0;
	fclose(ca);
	fclose(cb);
	if (pa != NULL) {
		fclose(pa);
		fclose(pb);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned long long tcase = 0, tfail = 0;
	int i;

	loc_c = newlocale(LC_ALL_MASK, "C", (locale_t)0);
	loc_utf8 = newlocale(LC_ALL_MASK, "C.utf8", (locale_t)0);
	if (loc_utf8 == (locale_t)0)
		loc_utf8 = newlocale(LC_ALL_MASK, "C.UTF-8", (locale_t)0);
	if (loc_utf8 == (locale_t)0)
		loc_utf8 = newlocale(LC_ALL_MASK, "en_US.UTF-8", (locale_t)0);
	if (loc_c == (locale_t)0 || loc_utf8 == (locale_t)0) {
		fprintf(stderr,
		    "cannot build test locales (C=%p, UTF-8=%p)\n",
		    (void *)loc_c, (void *)loc_utf8);
		return (1);
	}

	/* rewind first: its init-flag transition is observed only once */
	rewind_edge();
	rewind_sweep(200000);

	fgetc_edge();
	fgetc_sweep(200000);

	putwc_edge();
	putwc_sweep(200000, 0);
	putwc_sweep(200000, 1);

	swprintf_edge();
	swprintf_sweep(200000, 0);
	swprintf_sweep(200000, 1);

	printf("%-14s %14s %14s\n", "function", "cases", "failures");
	printf("%-14s %14s %14s\n", "--------------", "--------------",
	    "--------------");
	for (i = 0; i < F_NFUNC; i++) {
		printf("%-14s %14llu %14llu\n", fname[i], ncase[i], nfail[i]);
		tcase += ncase[i];
		tfail += nfail[i];
	}
	printf("%-14s %14s %14s\n", "--------------", "--------------",
	    "--------------");
	printf("%-14s %14llu %14llu\n", "TOTAL", tcase, tfail);

	freelocale(loc_c);
	freelocale(loc_utf8);

	if (tcase == 0) {
		fprintf(stderr, "no cases ran\n");
		return (1);
	}
	return (tfail == 0 ? 0 : 1);
}
