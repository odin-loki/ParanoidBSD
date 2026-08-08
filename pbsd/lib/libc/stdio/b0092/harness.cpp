/*
 * b0092 differential harness.
 *
 * Every ported function is exercised side by side with the ref_ oracle from
 * oracle.c.  Nothing about the two sides differs except which function is
 * called: the same stream contents, the same guard-filled buffers and the same
 * argument values are used, and every externally visible effect is captured
 * and compared.
 *
 * setbuffer()/setlinebuf() are observed through:
 *	- the buffering mode actually installed, which is visible as the number
 *	  of bytes that have reached the file descriptor *before* fflush()
 *	  (fully buffered: none; line buffered: through the last newline;
 *	  unbuffered: all of them),
 *	- the caller-supplied buffer itself: two 4096-byte arenas are filled
 *	  with the guard byte 0x7f and the nominal buffer is placed 64 bytes in,
 *	  so both writes past the end of the nominal window and writes in front
 *	  of it are caught; the ENTIRE arena is compared, three times per case
 *	  (after writing, after reading the file back through the same buffer,
 *	  and after fclose()),
 *	- ftell(), fwrite()/fflush() return values, the error/EOF indicators and
 *	  the final file contents.
 * getwc() is driven to exhaustion (and past it) over each input, comparing the
 * returned wint_t, errno, the stream offset and the error/EOF indicators after
 * EVERY call.
 * wscanf() is driven with 24 format strings whose entire output struct
 * (including every byte of the guard-filled destination buffers, written or
 * not) is compared, along with the return value, errno and the resulting
 * stdin offset.
 */

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wchar.h>

#include <string>
#include <vector>

import pbsd.lib.libc.stdio.b0092;

namespace port = pbsd::lib_libc_stdio::b0092;

extern "C" {
void ref_setbuffer(FILE *fp, char *buf, int size);
int ref_setlinebuf(FILE *fp);
int ref_wscanf(const wchar_t * __restrict fmt, ...);
wint_t ref_getwc(FILE *fp);
}

/* ------------------------------------------------------------------ */
/* bookkeeping							      */

enum { F_SETBUFFER, F_SETLINEBUF, F_WSCANF, F_GETWC, NFUNCS };

static const char *const fname[NFUNCS] = {
	"setbuffer", "setlinebuf", "wscanf", "getwc"
};

static long ncases[NFUNCS];
static long nfails[NFUNCS];
static long nshown[NFUNCS];

static void
fail(int f, const char *what, const std::string &detail)
{
	nfails[f]++;
	if (nshown[f] < 12) {
		nshown[f]++;
		printf("  FAIL %-10s %s: %s\n", fname[f], what, detail.c_str());
	} else if (nshown[f] == 12) {
		nshown[f]++;
		printf("  FAIL %-10s ... further failures suppressed\n",
		    fname[f]);
	}
}

/* ------------------------------------------------------------------ */
/* deterministic RNG (fixed seed, splitmix64)			      */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s;
}

static uint64_t
rng_next(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static uint32_t
rng_below(uint32_t n)
{
	return ((uint32_t)(rng_next() % n));
}

/* ------------------------------------------------------------------ */
/* temporary files						      */

static std::vector<std::string> tmp_paths;
static std::string tmp_dir;

static void
tmp_init(void)
{
	static char tmpl[] = "/tmp/pbsd_b0092_XXXXXX";
	char *d = mkdtemp(tmpl);
	if (d == NULL) {
		perror("mkdtemp");
		exit(2);
	}
	tmp_dir = d;
}

static std::string
tmp_path(const char *tag)
{
	std::string p = tmp_dir + "/" + tag;
	tmp_paths.push_back(p);
	return (p);
}

static void
tmp_cleanup(void)
{
	for (size_t i = 0; i < tmp_paths.size(); i++)
		unlink(tmp_paths[i].c_str());
	if (!tmp_dir.empty())
		rmdir(tmp_dir.c_str());
}

static void
write_file(const std::string &path, const unsigned char *d, size_t n)
{
	FILE *f = fopen(path.c_str(), "wb");
	if (f == NULL) {
		perror(path.c_str());
		exit(2);
	}
	if (n != 0 && fwrite(d, 1, n, f) != n) {
		perror("fwrite");
		exit(2);
	}
	if (fclose(f) != 0) {
		perror("fclose");
		exit(2);
	}
}

static std::vector<unsigned char>
slurp(const std::string &path)
{
	std::vector<unsigned char> out;
	FILE *f = fopen(path.c_str(), "rb");
	if (f == NULL) {
		perror(path.c_str());
		exit(2);
	}
	unsigned char b[512];
	size_t got;
	while ((got = fread(b, 1, sizeof b, f)) > 0)
		out.insert(out.end(), b, b + got);
	fclose(f);
	return (out);
}

/* ------------------------------------------------------------------ */
/* guard arenas							      */

static const size_t ARENA = 4096;	/* whole region that is compared */
static const size_t BUFOFF = 64;	/* nominal buffer starts here	 */
static const unsigned char GUARD = 0x7f;

static unsigned char arena[2][ARENA];

static std::string
arena_diff(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	char msg[160];
	if (a.size() != b.size()) {
		snprintf(msg, sizeof msg, "arena size %zu vs %zu", a.size(),
		    b.size());
		return (std::string(msg));
	}
	for (size_t i = 0; i < a.size(); i++)
		if (a[i] != b[i]) {
			snprintf(msg, sizeof msg,
			    "arena[%zu] (buf%+d) %02x vs %02x", i,
			    (int)i - (int)BUFOFF, a[i], b[i]);
			return (std::string(msg));
		}
	return (std::string());
}

static std::string
bytes_diff(const char *tag, const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	char msg[160];
	if (a.size() != b.size()) {
		snprintf(msg, sizeof msg, "%s length %zu vs %zu", tag,
		    a.size(), b.size());
		return (std::string(msg));
	}
	for (size_t i = 0; i < a.size(); i++)
		if (a[i] != b[i]) {
			snprintf(msg, sizeof msg, "%s[%zu] %02x vs %02x", tag,
			    i, a[i], b[i]);
			return (std::string(msg));
		}
	return (std::string());
}

/* ------------------------------------------------------------------ */
/* write-side observations for setbuffer()/setlinebuf()		      */

enum Op { OP_SETBUFFER, OP_SETLINEBUF, OP_BOTH };

struct WObs {
	int lb_ret = -12345;		/* setlinebuf() return		*/
	int lb_ret2 = -12345;
	long long size_before = -1;	/* fd size before the writes	*/
	long long size_mid = -1;	/* fd size after write, pre-flush */
	long long size_mid2 = -1;
	long long size_end = -1;	/* fd size after fflush		*/
	size_t wret = (size_t)-1;
	size_t wret2 = (size_t)-1;
	long tell_mid = -1;
	long tell_mid2 = -1;
	long tell_end = -1;
	long tell_read = -1;
	int fflush_ret = -12345;
	int errflag = -1, eofflag = -1;
	int errflag2 = -1, eofflag2 = -1;
	int read_errno = -1;
	std::vector<unsigned char> arena_mid;	/* after the writes	*/
	std::vector<unsigned char> arena_read;	/* after reading back	*/
	std::vector<unsigned char> arena_close;	/* after fclose()	*/
	std::vector<unsigned char> file;	/* file contents	*/
};

#define CMP(field)							\
	do {								\
		if (a.field != b.field) {				\
			char m[160];					\
			snprintf(m, sizeof m, #field " %lld vs %lld",	\
			    (long long)a.field, (long long)b.field);	\
			d = m;						\
			return (false);					\
		}							\
	} while (0)

static bool
wobs_eq(const WObs &a, const WObs &b, std::string &d)
{
	CMP(lb_ret);
	CMP(lb_ret2);
	CMP(size_before);
	CMP(size_mid);
	CMP(size_mid2);
	CMP(size_end);
	CMP(wret);
	CMP(wret2);
	CMP(tell_mid);
	CMP(tell_mid2);
	CMP(tell_end);
	CMP(tell_read);
	CMP(fflush_ret);
	CMP(errflag);
	CMP(eofflag);
	CMP(errflag2);
	CMP(eofflag2);
	CMP(read_errno);
	d = arena_diff(a.arena_mid, b.arena_mid);
	if (!d.empty()) {
		d = "mid " + d;
		return (false);
	}
	d = arena_diff(a.arena_read, b.arena_read);
	if (!d.empty()) {
		d = "read " + d;
		return (false);
	}
	d = arena_diff(a.arena_close, b.arena_close);
	if (!d.empty()) {
		d = "close " + d;
		return (false);
	}
	d = bytes_diff("file", a.file, b.file);
	if (!d.empty())
		return (false);
	return (true);
}

static long long
fd_size(FILE *fp)
{
	struct stat st;
	if (fstat(fileno(fp), &st) != 0) {
		perror("fstat");
		exit(2);
	}
	return ((long long)st.st_size);
}

static void
call_setbuffer(int side, FILE *fp, char *buf, int size)
{
	if (side == 0)
		port::setbuffer(fp, buf, size);
	else
		ref_setbuffer(fp, buf, size);
}

static int
call_setlinebuf(int side, FILE *fp)
{
	return (side == 0 ? port::setlinebuf(fp) : ref_setlinebuf(fp));
}

/*
 * Install buffering on an already-open stream, write `n' bytes, then flush,
 * recording everything observable.  For OP_BOTH the stream is switched to line
 * buffering half way through the data, which exercises the interaction of the
 * two functions.
 */
static void
do_write_phase(FILE *fp, unsigned char *ar, int side, Op op, bool use_null,
    int size, const unsigned char *data, size_t n, WObs &o)
{
	char *buf = use_null ? (char *)NULL : (char *)(ar + BUFOFF);

	memset(ar, GUARD, ARENA);
	o.size_before = fd_size(fp);

	if (op == OP_SETLINEBUF) {
		o.lb_ret = call_setlinebuf(side, fp);
		o.wret = fwrite(data, 1, n, fp);
		o.size_mid = fd_size(fp);
		o.tell_mid = ftell(fp);
	} else if (op == OP_SETBUFFER) {
		call_setbuffer(side, fp, buf, size);
		o.wret = fwrite(data, 1, n, fp);
		o.size_mid = fd_size(fp);
		o.tell_mid = ftell(fp);
	} else {
		size_t half = n / 2;
		call_setbuffer(side, fp, buf, size);
		o.wret = fwrite(data, 1, half, fp);
		o.size_mid = fd_size(fp);
		o.tell_mid = ftell(fp);
		o.lb_ret = call_setlinebuf(side, fp);
		o.wret2 = fwrite(data + half, 1, n - half, fp);
		o.size_mid2 = fd_size(fp);
		o.tell_mid2 = ftell(fp);
		o.lb_ret2 = call_setlinebuf(side, fp);
	}

	o.fflush_ret = fflush(fp);
	o.size_end = fd_size(fp);
	o.tell_end = ftell(fp);
	o.errflag = ferror(fp) ? 1 : 0;
	o.eofflag = feof(fp) ? 1 : 0;
	o.arena_mid.assign(ar, ar + ARENA);
}

/* A whole case on a private stream, including reading the file back. */
static void
run_fresh_write(int side, Op op, bool use_null, int size,
    const unsigned char *data, size_t n, WObs &o)
{
	unsigned char *ar = arena[side];
	FILE *fp = tmpfile();

	if (fp == NULL) {
		perror("tmpfile");
		exit(2);
	}
	do_write_phase(fp, ar, side, op, use_null, size, data, n, o);

	/* Read the file back through the very same buffer. */
	rewind(fp);
	errno = 0;
	unsigned char rb[64];
	size_t got;
	o.file.clear();
	while ((got = fread(rb, 1, sizeof rb, fp)) > 0)
		o.file.insert(o.file.end(), rb, rb + got);
	o.read_errno = errno;
	o.tell_read = ftell(fp);
	o.errflag2 = ferror(fp) ? 1 : 0;
	o.eofflag2 = feof(fp) ? 1 : 0;
	o.arena_read.assign(ar, ar + ARENA);

	fclose(fp);
	o.arena_close.assign(ar, ar + ARENA);
}

/* ------------------------------------------------------------------ */
/* read-side observations for setbuffer()			      */

struct RObs {
	size_t r1 = (size_t)-1;
	long tell1 = -1, tell2 = -1, tell3 = -1;
	int eof1 = -1, err1 = -1, eof2 = -1, err2 = -1;
	int gc[3] = { -12345, -12345, -12345 };
	std::vector<unsigned char> got1, rest;
	std::vector<unsigned char> arena1, arena2, arena3;
};

static bool
robs_eq(const RObs &a, const RObs &b, std::string &d)
{
	CMP(r1);
	CMP(tell1);
	CMP(tell2);
	CMP(tell3);
	CMP(eof1);
	CMP(err1);
	CMP(eof2);
	CMP(err2);
	CMP(gc[0]);
	CMP(gc[1]);
	CMP(gc[2]);
	d = bytes_diff("got1", a.got1, b.got1);
	if (!d.empty())
		return (false);
	d = bytes_diff("rest", a.rest, b.rest);
	if (!d.empty())
		return (false);
	d = arena_diff(a.arena1, b.arena1);
	if (!d.empty()) {
		d = "r1 " + d;
		return (false);
	}
	d = arena_diff(a.arena2, b.arena2);
	if (!d.empty()) {
		d = "r2 " + d;
		return (false);
	}
	d = arena_diff(a.arena3, b.arena3);
	if (!d.empty()) {
		d = "r3 " + d;
		return (false);
	}
	return (true);
}

static void
run_read_case(int side, bool use_null, int size, const std::string &path,
    size_t k1, RObs &o)
{
	unsigned char *ar = arena[side];
	unsigned char tmp[512];

	memset(ar, GUARD, ARENA);
	FILE *fp = fopen(path.c_str(), "rb");
	if (fp == NULL) {
		perror(path.c_str());
		exit(2);
	}
	call_setbuffer(side, fp, use_null ? (char *)NULL :
	    (char *)(ar + BUFOFF), size);

	o.r1 = fread(tmp, 1, k1, fp);
	o.got1.assign(tmp, tmp + o.r1);
	o.tell1 = ftell(fp);
	o.eof1 = feof(fp) ? 1 : 0;
	o.err1 = ferror(fp) ? 1 : 0;
	o.arena1.assign(ar, ar + ARENA);

	for (int i = 0; i < 3; i++)
		o.gc[i] = fgetc(fp);
	o.tell2 = ftell(fp);

	size_t got;
	o.rest.clear();
	while ((got = fread(tmp, 1, sizeof tmp, fp)) > 0)
		o.rest.insert(o.rest.end(), tmp, tmp + got);
	o.tell3 = ftell(fp);
	o.eof2 = feof(fp) ? 1 : 0;
	o.err2 = ferror(fp) ? 1 : 0;
	o.arena2.assign(ar, ar + ARENA);

	fclose(fp);
	o.arena3.assign(ar, ar + ARENA);
}

/* ------------------------------------------------------------------ */
/* data patterns						      */

enum { NPAT = 9 };

static void
fill_pattern(unsigned char *d, size_t n, int pat)
{
	for (size_t i = 0; i < n; i++) {
		switch (pat) {
		case 0:
			d[i] = 0x00;			/* NUL-heavy	*/
			break;
		case 1:
			d[i] = 0xff;			/* high bits	*/
			break;
		case 2:
			d[i] = (unsigned char)(0x80 + (i & 0x7f));
			break;
		case 3:
			d[i] = (unsigned char)(i & 0xff);
			break;
		case 4:
			d[i] = (unsigned char)((i & 1) ? 0x00 : 0x80);
			break;
		case 5:
			d[i] = (unsigned char)"abcdefghij\n"[i % 11];
			break;
		case 6:
			d[i] = (unsigned char)((i % 7 == 0) ? '\n' : 'x');
			break;
		case 7:	/* newline only in the very last position */
			d[i] = (unsigned char)(i + 1 == n ? '\n' :
			    ((i & 1) ? 0x00 : 0xa5));
			break;
		default:
			d[i] = (unsigned char)'\n';	/* newlines only */
			break;
		}
	}
}

/* ------------------------------------------------------------------ */
/* phase A: setbuffer() on write streams			      */

static const int sizes_hand[] = {
	-100000, -1000, -64, -3, -2, -1, 0, 1, 2, 3, 4, 7, 8, 15, 16, 17,
	31, 32, 63, 64, 65, 127, 128, 255, 256, 300, 301, 512, 1024, 2048
};
static const size_t lens_hand[] = {
	0, 1, 2, 3, 7, 8, 15, 16, 17, 63, 64, 65, 127, 128, 255, 256, 300
};

static void
phase_setbuffer_write(void)
{
	unsigned char data[512];
	char tag[160];

	for (int nullbuf = 0; nullbuf < 2; nullbuf++)
		for (size_t si = 0; si < sizeof sizes_hand / sizeof *sizes_hand;
		    si++)
			for (size_t li = 0;
			    li < sizeof lens_hand / sizeof *lens_hand; li++)
				for (int pat = 0; pat < NPAT; pat++) {
					size_t n = lens_hand[li];
					int size = sizes_hand[si];
					fill_pattern(data, n, pat);

					WObs a, b;
					run_fresh_write(0, OP_SETBUFFER,
					    nullbuf != 0, size, data, n, a);
					run_fresh_write(1, OP_SETBUFFER,
					    nullbuf != 0, size, data, n, b);
					ncases[F_SETBUFFER]++;
					std::string d;
					if (!wobs_eq(a, b, d)) {
						snprintf(tag, sizeof tag,
						    "write null=%d size=%d n=%zu pat=%d",
						    nullbuf, size, n, pat);
						fail(F_SETBUFFER, tag, d);
					}
				}
}

/* ------------------------------------------------------------------ */
/* phase B: setbuffer() on read streams				      */

static void
phase_setbuffer_read(void)
{
	static const size_t clens[] = { 0, 1, 2, 3, 16, 17, 64, 100, 255,
	    256, 300 };
	static const int rsizes[] = { 0, 1, 2, 3, 4, 7, 8, 16, 17, 63, 64,
	    100, 256, 300, 512 };
	static const size_t ks[] = { 0, 1, 2, 3, 7, 16, 100, 300 };
	unsigned char data[512];
	char tag[160];

	for (size_t ci = 0; ci < sizeof clens / sizeof *clens; ci++) {
		size_t clen = clens[ci];
		std::string path = tmp_path("rdata");
		for (int pat = 0; pat < NPAT; pat += 4) {
			fill_pattern(data, clen, pat);
			write_file(path, data, clen);
			for (size_t zi = 0;
			    zi < sizeof rsizes / sizeof *rsizes; zi++)
				for (size_t ki = 0;
				    ki < sizeof ks / sizeof *ks; ki++)
					for (int nullbuf = 0; nullbuf < 2;
					    nullbuf++) {
						RObs a, b;
						run_read_case(0,
						    nullbuf != 0, rsizes[zi],
						    path, ks[ki], a);
						run_read_case(1,
						    nullbuf != 0, rsizes[zi],
						    path, ks[ki], b);
						ncases[F_SETBUFFER]++;
						std::string d;
						if (!robs_eq(a, b, d)) {
							snprintf(tag,
							    sizeof tag,
							    "read clen=%zu pat=%d null=%d size=%d k=%zu",
							    clen, pat, nullbuf,
							    rsizes[zi],
							    ks[ki]);
							fail(F_SETBUFFER, tag,
							    d);
						}
					}
		}
	}
}

/* ------------------------------------------------------------------ */
/* phase C: setlinebuf() and the setbuffer()/setlinebuf() combination  */

static void
phase_setlinebuf(void)
{
	unsigned char data[512];
	char tag[160];

	for (size_t li = 0; li < sizeof lens_hand / sizeof *lens_hand; li++)
		for (int pat = 0; pat < NPAT; pat++) {
			size_t n = lens_hand[li];
			fill_pattern(data, n, pat);

			WObs a, b;
			run_fresh_write(0, OP_SETLINEBUF, true, 0, data, n, a);
			run_fresh_write(1, OP_SETLINEBUF, true, 0, data, n, b);
			ncases[F_SETLINEBUF]++;
			std::string d;
			if (!wobs_eq(a, b, d)) {
				snprintf(tag, sizeof tag,
				    "plain n=%zu pat=%d", n, pat);
				fail(F_SETLINEBUF, tag, d);
			}
		}

	/* setbuffer() first, then switch to line buffering mid-stream. */
	for (size_t si = 0; si < sizeof sizes_hand / sizeof *sizes_hand; si++)
		for (size_t li = 0; li < sizeof lens_hand / sizeof *lens_hand;
		    li++)
			for (int pat = 0; pat < NPAT; pat++) {
				size_t n = lens_hand[li];
				int size = sizes_hand[si];
				fill_pattern(data, n, pat);

				WObs a, b;
				run_fresh_write(0, OP_BOTH, false, size, data,
				    n, a);
				run_fresh_write(1, OP_BOTH, false, size, data,
				    n, b);
				ncases[F_SETLINEBUF]++;
				std::string d;
				if (!wobs_eq(a, b, d)) {
					snprintf(tag, sizeof tag,
					    "combo size=%d n=%zu pat=%d",
					    size, n, pat);
					fail(F_SETLINEBUF, tag, d);
				}
			}
}

/* ------------------------------------------------------------------ */
/* phase D: randomised sweep for setbuffer()/setlinebuf()	      */

static void
phase_buffering_sweep(long iters)
{
	FILE *fp[2];
	std::string paths[2];
	unsigned char data[288];
	char tag[160];

	for (int s = 0; s < 2; s++) {
		paths[s] = tmp_path(s == 0 ? "sweep_p" : "sweep_r");
		fp[s] = fopen(paths[s].c_str(), "w+b");
		if (fp[s] == NULL) {
			perror(paths[s].c_str());
			exit(2);
		}
	}

	for (long it = 0; it < iters; it++) {
		uint32_t opsel = rng_below(10);
		Op op = opsel < 6 ? OP_SETBUFFER :
		    (opsel < 8 ? OP_SETLINEBUF : OP_BOTH);
		bool use_null = (rng_below(4) == 0);
		int size;
		switch (rng_below(8)) {
		case 0:
			size = (int)rng_below(4);		/* 0..3    */
			break;
		case 1:
			size = -(int)rng_below(8) - 1;		/* -8..-1  */
			break;
		case 2:
			size = (int)rng_below(600);
			break;
		case 3:
			size = (int)rng_below(2048) + 1;
			break;
		default:
			size = (int)rng_below(64);
			break;
		}
		size_t n = (size_t)rng_below(257);
		for (size_t i = 0; i < n; i++) {
			switch (rng_below(6)) {
			case 0:
				data[i] = 0x00;
				break;
			case 1:
				data[i] = '\n';
				break;
			case 2:
				data[i] = (unsigned char)(0x80 +
				    rng_below(128));
				break;
			default:
				data[i] = (unsigned char)rng_below(256);
				break;
			}
		}

		WObs a, b;
		do_write_phase(fp[0], arena[0], 0, op, use_null, size, data,
		    n, a);
		do_write_phase(fp[1], arena[1], 1, op, use_null, size, data,
		    n, b);
		int f = (op == OP_SETLINEBUF) ? F_SETLINEBUF : F_SETBUFFER;
		ncases[f]++;
		std::string d;
		if (!wobs_eq(a, b, d)) {
			snprintf(tag, sizeof tag,
			    "sweep it=%ld op=%d null=%d size=%d n=%zu",
			    it, (int)op, (int)use_null, size, n);
			fail(f, tag, d);
		}
	}

	/*
	 * Detach the arenas from the streams before they stop being written
	 * to, then compare the two files in full.
	 */
	for (int s = 0; s < 2; s++) {
		fflush(fp[s]);
		setvbuf(fp[s], (char *)NULL, _IONBF, 0);
		fclose(fp[s]);
	}
	std::vector<unsigned char> fa = slurp(paths[0]);
	std::vector<unsigned char> fb = slurp(paths[1]);
	ncases[F_SETBUFFER]++;
	std::string d = bytes_diff("sweepfile", fa, fb);
	if (!d.empty())
		fail(F_SETBUFFER, "sweep final file", d);
}

/* ------------------------------------------------------------------ */
/* getwc()							      */

struct GObs {
	unsigned int ret = 0;
	int e = -1;
	long tell = -2;
	int eofflag = -1, errflag = -1;
};

static bool
gobs_eq(const GObs &a, const GObs &b, std::string &d)
{
	CMP(ret);
	CMP(e);
	CMP(tell);
	CMP(eofflag);
	CMP(errflag);
	return (true);
}

static void
one_getwc(int side, FILE *fp, GObs &o)
{
	errno = 0;
	wint_t c = (side == 0) ? port::getwc(fp) : ref_getwc(fp);
	o.e = errno;
	o.ret = (unsigned int)c;
	o.tell = ftell(fp);
	o.eofflag = feof(fp) ? 1 : 0;
	o.errflag = ferror(fp) ? 1 : 0;
}

/* Drive one input to exhaustion, comparing after every single call. */
static void
getwc_drain(const std::string &path, long start, long steps, const char *what)
{
	FILE *f0 = fopen(path.c_str(), "rb");
	FILE *f1 = fopen(path.c_str(), "rb");
	char tag[192];

	if (f0 == NULL || f1 == NULL) {
		perror(path.c_str());
		exit(2);
	}
	if (start != 0) {
		fseek(f0, start, SEEK_SET);
		fseek(f1, start, SEEK_SET);
	}
	for (long step = 0; step < steps; step++) {
		GObs a, b;
		one_getwc(0, f0, a);
		one_getwc(1, f1, b);
		ncases[F_GETWC]++;
		std::string d;
		if (!gobs_eq(a, b, d)) {
			snprintf(tag, sizeof tag, "%s start=%ld step=%ld",
			    what, start, step);
			fail(F_GETWC, tag, d);
			break;
		}
		if (a.errflag) {
			/* Identical recovery on both sides. */
			clearerr(f0);
			clearerr(f1);
			fseek(f0, a.tell + 1, SEEK_SET);
			fseek(f1, b.tell + 1, SEEK_SET);
		}
	}
	fclose(f0);
	fclose(f1);
}

static void
phase_getwc_hand(void)
{
	struct {
		const char *name;
		const char *bytes;
		size_t len;
	} cases[] = {
		{ "empty", "", 0 },
		{ "one-ascii", "A", 1 },
		{ "one-nul", "\x00", 1 },
		{ "one-0x7f", "\x7f", 1 },
		{ "one-0x80", "\x80", 1 },
		{ "one-0xff", "\xff", 1 },
		{ "nul-heavy", "\x00\x00\x00\x00", 4 },
		{ "ascii-nul-mix", "A\x00" "B\x00" "C", 5 },
		{ "highbits", "\x80\x81\xfe\xff", 4 },
		{ "ascii-then-high", "abc\x80\xff" "d", 6 },
		{ "utf8-2byte", "\xc3\xa9", 2 },
		{ "utf8-3byte", "\xe2\x82\xac", 3 },
		{ "utf8-4byte", "\xf0\x9f\x92\xa9", 4 },
		{ "utf8-trunc2", "\xc3", 1 },
		{ "utf8-trunc3", "\xe2\x82", 2 },
		{ "utf8-trunc4", "\xf0\x9f\x92", 3 },
		{ "utf8-overlong", "\xc0\x80", 2 },
		{ "utf8-surrogate", "\xed\xa0\x80", 3 },
		{ "utf8-mixed", "a\xc3\xa9" "b\xe2\x82\xac" "c", 9 },
		{ "newlines", "\n\n\n", 3 },
		{ "high-then-ascii", "\xff" "Z", 2 },
		{ "cont-byte-only", "\x80\x80\x80", 3 },
	};
	std::string path = tmp_path("getwc_hand");

	for (size_t i = 0; i < sizeof cases / sizeof *cases; i++) {
		write_file(path, (const unsigned char *)cases[i].bytes,
		    cases[i].len);
		for (long start = 0; start <= (long)cases[i].len + 1; start++)
			getwc_drain(path, start, (long)cases[i].len + 6,
			    cases[i].name);
	}

	/* Boundary lengths around the stdio buffer size. */
	std::vector<unsigned char> big;
	static const size_t biglens[] = { 63, 64, 65, 127, 128, 129, 255,
	    256, 257, 4095, 4096, 4097 };
	std::string bpath = tmp_path("getwc_big");
	for (size_t i = 0; i < sizeof biglens / sizeof *biglens; i++) {
		size_t n = biglens[i];
		big.assign(n, 0);
		for (size_t j = 0; j < n; j++)
			big[j] = (unsigned char)('0' + (j % 10));
		if (n >= 3) {
			big[n - 2] = 0x80;	/* illegal byte at the end */
			big[n / 2] = 0x00;
		}
		write_file(bpath, big.data(), n);
		getwc_drain(bpath, 0, (long)n + 6, "big");
		getwc_drain(bpath, (long)n - 1, 8, "big-tail");
		getwc_drain(bpath, (long)n, 8, "big-eof");
	}
}

static void
phase_getwc_sweep(long iters)
{
	std::vector<unsigned char> blob;
	std::string path = tmp_path("getwc_sweep");
	char tag[160];

	blob.reserve(8192);
	while (blob.size() < 8192) {
		switch (rng_below(8)) {
		case 0:
			blob.push_back(0x00);
			break;
		case 1:
			blob.push_back((unsigned char)(0x80 + rng_below(128)));
			break;
		case 2:
			blob.push_back(0xc3);
			blob.push_back(0xa9);
			break;
		case 3:
			blob.push_back(0xe2);
			blob.push_back(0x82);
			blob.push_back(0xac);
			break;
		case 4:
			blob.push_back(0xf0);
			blob.push_back(0x9f);
			blob.push_back(0x92);
			blob.push_back(0xa9);
			break;
		case 5:
			blob.push_back((unsigned char)('0' + rng_below(10)));
			break;
		case 6:
			blob.push_back((unsigned char)rng_below(256));
			break;
		default:
			blob.push_back((unsigned char)(0x20 + rng_below(95)));
			break;
		}
	}
	write_file(path, blob.data(), blob.size());

	FILE *f0 = fopen(path.c_str(), "rb");
	FILE *f1 = fopen(path.c_str(), "rb");
	if (f0 == NULL || f1 == NULL) {
		perror(path.c_str());
		exit(2);
	}

	long limit = (long)blob.size();
	for (long it = 0; it < iters; it++) {
		if ((it % 10000) == 0) {
			/*
			 * Alternate between the C and the UTF-8 locale so that
			 * both the single-byte and the multibyte decoding
			 * paths are covered.  Both sides always run under the
			 * same locale.
			 */
			const char *want = ((it / 10000) & 1) ? "C.UTF-8" : "C";
			if (setlocale(LC_CTYPE, want) == NULL)
				setlocale(LC_CTYPE, "C");
		}
		if (rng_below(3) != 0) {
			long pos;
			switch (rng_below(4)) {
			case 0:
				pos = limit - (long)rng_below(6);
				break;
			case 1:
				pos = limit + (long)rng_below(4);
				break;
			default:
				pos = (long)rng_below((uint32_t)limit);
				break;
			}
			if (pos < 0)
				pos = 0;
			clearerr(f0);
			clearerr(f1);
			if (fseek(f0, pos, SEEK_SET) != 0 ||
			    fseek(f1, pos, SEEK_SET) != 0) {
				perror("fseek");
				exit(2);
			}
		}

		GObs a, b;
		one_getwc(0, f0, a);
		one_getwc(1, f1, b);
		ncases[F_GETWC]++;
		std::string d;
		if (!gobs_eq(a, b, d)) {
			snprintf(tag, sizeof tag, "sweep it=%ld", it);
			fail(F_GETWC, tag, d);
		}
		if (a.errflag || b.errflag) {
			clearerr(f0);
			clearerr(f1);
			fseek(f0, a.tell + 1, SEEK_SET);
			fseek(f1, b.tell + 1, SEEK_SET);
		}
	}
	fclose(f0);
	fclose(f1);
	setlocale(LC_CTYPE, "C");
}

/* ------------------------------------------------------------------ */
/* wscanf()							      */

typedef int (*ScanFn)(const wchar_t * __restrict fmt, ...);

struct ScanObs {
	/* Filled after the call; everything else stays at the guard byte. */
	int ret;
	int e;
	int eofflag;
	int errflag;
	int seekret;
	long tell;
	/* Conversion destinations. */
	int iv[4];
	unsigned int uv[2];
	long lv;
	long long llv;
	double dv;
	float fv;
	int nv;
	wchar_t wbuf[48];
	char cbuf[96];
	wchar_t wcv[4];
	char ccv[16];
};

enum { NFMT = 24 };

static const wchar_t *const fmt_name[NFMT] = {
	L"%d", L"%d %d", L"%d%d", L"%20ls", L"%15s", L"%lc", L"%c", L"%d%n",
	L"%lf", L"%f", L"%x %o", L"%5d%5d", L"%*d%d", L"%10l[0-9]",
	L"%10[^0-9]", L"abc%d", L"%%%d", L"%lld", L"%ld %lf", L" %d ",
	L"%d,%d", L"%2lc%2lc", L"%u", L"%8ls%d"
};

static void
run_scan(ScanFn fn, int fi, long pos, ScanObs &o)
{
	memset(&o, GUARD, sizeof o);

	clearerr(stdin);
	int sr = fseek(stdin, pos, SEEK_SET);
	clearerr(stdin);
	errno = 0;

	int r;
	switch (fi) {
	case 0:
		r = fn(L"%d", &o.iv[0]);
		break;
	case 1:
		r = fn(L"%d %d", &o.iv[0], &o.iv[1]);
		break;
	case 2:
		r = fn(L"%d%d", &o.iv[0], &o.iv[1]);
		break;
	case 3:
		r = fn(L"%20ls", o.wbuf);
		break;
	case 4:
		r = fn(L"%15s", o.cbuf);
		break;
	case 5:
		r = fn(L"%lc", &o.wcv[0]);
		break;
	case 6:
		r = fn(L"%c", o.ccv);
		break;
	case 7:
		r = fn(L"%d%n", &o.iv[0], &o.nv);
		break;
	case 8:
		r = fn(L"%lf", &o.dv);
		break;
	case 9:
		r = fn(L"%f", &o.fv);
		break;
	case 10:
		r = fn(L"%x %o", &o.uv[0], &o.uv[1]);
		break;
	case 11:
		r = fn(L"%5d%5d", &o.iv[0], &o.iv[1]);
		break;
	case 12:
		r = fn(L"%*d%d", &o.iv[0]);
		break;
	case 13:
		r = fn(L"%10l[0-9]", o.wbuf);
		break;
	case 14:
		r = fn(L"%10[^0-9]", o.cbuf);
		break;
	case 15:
		r = fn(L"abc%d", &o.iv[0]);
		break;
	case 16:
		r = fn(L"%%%d", &o.iv[0]);
		break;
	case 17:
		r = fn(L"%lld", &o.llv);
		break;
	case 18:
		r = fn(L"%ld %lf", &o.lv, &o.dv);
		break;
	case 19:
		r = fn(L" %d ", &o.iv[0]);
		break;
	case 20:
		r = fn(L"%d,%d", &o.iv[0], &o.iv[1]);
		break;
	case 21:
		r = fn(L"%2lc%2lc", &o.wcv[0], &o.wcv[2]);
		break;
	case 22:
		r = fn(L"%u", &o.uv[0]);
		break;
	default:
		r = fn(L"%8ls%d", o.wbuf, &o.iv[0]);
		break;
	}

	int e = errno;
	o.ret = r;
	o.e = e;
	o.seekret = sr;
	o.tell = ftell(stdin);
	o.eofflag = feof(stdin) ? 1 : 0;
	o.errflag = ferror(stdin) ? 1 : 0;
}

static std::string
scanobs_diff(const ScanObs &a, const ScanObs &b)
{
	char m[192];
	if (a.ret != b.ret || a.e != b.e || a.tell != b.tell ||
	    a.eofflag != b.eofflag || a.errflag != b.errflag ||
	    a.seekret != b.seekret) {
		snprintf(m, sizeof m,
		    "ret %d/%d errno %d/%d tell %ld/%ld eof %d/%d err %d/%d seek %d/%d",
		    a.ret, b.ret, a.e, b.e, a.tell, b.tell, a.eofflag,
		    b.eofflag, a.errflag, b.errflag, a.seekret, b.seekret);
		return (std::string(m));
	}
	const unsigned char *pa = (const unsigned char *)&a;
	const unsigned char *pb = (const unsigned char *)&b;
	for (size_t i = 0; i < sizeof(ScanObs); i++)
		if (pa[i] != pb[i]) {
			snprintf(m, sizeof m, "output byte %zu: %02x vs %02x",
			    i, pa[i], pb[i]);
			return (std::string(m));
		}
	return (std::string());
}

static void
scan_compare(int fi, long pos, const char *what)
{
	ScanObs a, b;
	char tag[192];

	run_scan(&port::wscanf, fi, pos, a);
	run_scan(&ref_wscanf, fi, pos, b);
	ncases[F_WSCANF]++;
	std::string d = scanobs_diff(a, b);
	if (!d.empty()) {
		snprintf(tag, sizeof tag, "%s fmt=%ls pos=%ld", what,
		    fmt_name[fi], pos);
		fail(F_WSCANF, tag, d);
	}
}

static void
phase_wscanf_hand(void)
{
	struct {
		const char *bytes;
		size_t len;
	} inputs[] = {
		{ "", 0 },
		{ "0", 1 },
		{ "1", 1 },
		{ "9", 1 },
		{ " ", 1 },
		{ "\n", 1 },
		{ "\x00", 1 },
		{ "-", 1 },
		{ "+", 1 },
		{ "42", 2 },
		{ "-42", 3 },
		{ "  42  ", 6 },
		{ "42 17", 5 },
		{ "42\n17\n", 6 },
		{ "12,34", 5 },
		{ "0x1f 0777", 9 },
		{ "3.14159", 7 },
		{ "-2.5e-3 8", 9 },
		{ "abc", 3 },
		{ "abc42", 5 },
		{ "%42", 3 },
		{ "99999999999999999999999999", 26 },
		{ "-99999999999999999999999999", 27 },
		{ "2147483647 2147483648", 21 },
		{ "-2147483648 -2147483649", 23 },
		{ "\x80", 1 },
		{ "\xff", 1 },
		{ "1\x80" "2", 3 },
		{ "\x80" "12", 3 },
		{ "A\x00" "B", 3 },
		{ "\x00\x00\x00", 3 },
		{ "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 36 },
		{ "111111111111111111111111111111111111", 36 },
		{ "\xc3\xa9\xc3\xa9 5", 6 },
		{ "\xe2\x82\xac 7", 5 },
		{ "\t\t 8", 4 },
		{ "12345678901234567890 x", 22 },
		{ "  \n\n  -0 ", 9 },
	};
	std::string path = tmp_path("wscanf_hand");

	for (size_t i = 0; i < sizeof inputs / sizeof *inputs; i++) {
		write_file(path, (const unsigned char *)inputs[i].bytes,
		    inputs[i].len);
		if (freopen(path.c_str(), "rb", stdin) == NULL) {
			perror("freopen stdin");
			exit(2);
		}
		for (int locsel = 0; locsel < 2; locsel++) {
			const char *want = locsel ? "C.UTF-8" : "C";
			if (setlocale(LC_CTYPE, want) == NULL)
				setlocale(LC_CTYPE, "C");
			for (int fi = 0; fi < NFMT; fi++)
				for (long pos = 0;
				    pos <= (long)inputs[i].len + 1; pos++)
					scan_compare(fi, pos, "hand");
		}
		setlocale(LC_CTYPE, "C");
	}
}

static void
phase_wscanf_sweep(long iters)
{
	static const struct {
		const char *s;
		size_t n;
	} toks[] = {
		{ "0 ", 2 }, { "1 ", 2 }, { "-1 ", 3 }, { "+12 ", 4 },
		{ "42\n", 3 }, { "  7", 3 }, { "999999999999999999999 ", 22 },
		{ "0x1f ", 5 }, { "007 ", 4 }, { "3.14 ", 5 },
		{ "-2.5e-3 ", 8 }, { "abc ", 4 }, { "ABC", 3 }, { ",", 1 },
		{ "\t", 1 }, { "\n", 1 }, { "  ", 2 }, { "1,2 ", 4 },
		{ "zz9 ", 4 }, { "\x80", 1 }, { "\xff", 1 },
		{ "\xc3\xa9 ", 3 }, { "\xe2\x82\xac ", 4 }, { "\x00", 1 },
		{ "-", 1 }, { "+", 1 }, { ".5 ", 3 }, { "1e ", 3 },
		{ "inf ", 4 }, { "nan ", 4 }, { "%", 1 }, { "5d", 2 },
		{ "2147483648 ", 11 }, { "-0 ", 3 }, { "18446744073709551616 ", 21 }
	};
	std::vector<unsigned char> blob;
	std::string path = tmp_path("wscanf_sweep");

	blob.reserve(8192);
	while (blob.size() < 8000) {
		if (rng_below(16) == 0) {
			blob.push_back((unsigned char)rng_below(256));
			continue;
		}
		uint32_t t = rng_below(sizeof toks / sizeof *toks);
		blob.insert(blob.end(), (const unsigned char *)toks[t].s,
		    (const unsigned char *)toks[t].s + toks[t].n);
	}
	write_file(path, blob.data(), blob.size());
	if (freopen(path.c_str(), "rb", stdin) == NULL) {
		perror("freopen stdin");
		exit(2);
	}

	long limit = (long)blob.size();
	for (long it = 0; it < iters; it++) {
		if ((it % 10000) == 0) {
			const char *want = ((it / 10000) & 1) ? "C.UTF-8" : "C";
			if (setlocale(LC_CTYPE, want) == NULL)
				setlocale(LC_CTYPE, "C");
		}
		int fi = (int)rng_below(NFMT);
		long pos;
		switch (rng_below(8)) {
		case 0:
			pos = limit - (long)rng_below(4);
			break;
		case 1:
			pos = limit + (long)rng_below(3);
			break;
		default:
			pos = (long)rng_below((uint32_t)limit);
			break;
		}
		if (pos < 0)
			pos = 0;
		scan_compare(fi, pos, "sweep");
	}
	setlocale(LC_CTYPE, "C");
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	setvbuf(stdout, (char *)NULL, _IOLBF, 0);
	setlocale(LC_ALL, "C");
	tmp_init();
	rng_seed(0x5eed0092b0092ULL);

	printf("b0092 differential test: setbuffer, setlinebuf, wscanf, getwc\n");

	phase_setbuffer_write();
	phase_setbuffer_read();
	phase_setlinebuf();
	phase_buffering_sweep(200000);

	phase_getwc_hand();
	phase_getwc_sweep(200000);

	phase_wscanf_hand();
	phase_wscanf_sweep(200000);

	tmp_cleanup();

	long tc = 0, tf = 0;
	printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	printf("%-14s %12s %12s\n", "--------------", "------------",
	    "------------");
	for (int i = 0; i < NFUNCS; i++) {
		printf("%-14s %12ld %12ld\n", fname[i], ncases[i], nfails[i]);
		tc += ncases[i];
		tf += nfails[i];
	}
	printf("%-14s %12s %12s\n", "--------------", "------------",
	    "------------");
	printf("%-14s %12ld %12ld\n", "TOTAL", tc, tf);
	printf("\n%s\n", tf == 0 ? "PASS: all cases matched the oracle" :
	    "FAIL: divergence from the oracle");
	return (tf == 0 ? 0 : 1);
}
