/*
 * Differential harness for batch b0098s3 (lib/libc/stdio/setbuf.c).
 *
 * setbuf() has no return value; every effect it has is observable only
 * through the FILE it is applied to.  The harness therefore observes, for
 * each case, the complete behaviour of a stream that has just been handed
 * to setbuf():
 *
 *   - whether bytes reach the file descriptor before fflush()  (this
 *     distinguishes _IONBF from _IOFBF/_IOLBF, i.e. the ?: branches),
 *   - at exactly which byte count the first flush happens      (this
 *     distinguishes the size argument, BUFSIZ, from any other value),
 *   - the ENTIRE caller supplied buffer, BUFSIZ bytes plus a 64 byte guard
 *     region past the nominal window, both before the flush and after the
 *     stream is closed (this distinguishes "uses the caller's buffer" from
 *     "ignores it", and catches writes past the nominal window),
 *   - the write call return values, ftell(), ferror(), feof(), the fflush()
 *     and fclose() return values and the final file contents.
 *
 * Both the ported setbuf and ref_setbuf are driven through byte-identical
 * sequences on two private temporary files with two private buffers.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

import pbsd.lib.libc.stdio.b0098s3;

extern "C" void ref_setbuf(FILE *__restrict fp, char *__restrict buf);

namespace port = pbsd::lib_libc_stdio::b0098s3;

/* ------------------------------------------------------------------ */

enum Method { M_FWRITE = 0, M_PUTC = 1, M_MIX = 2, M_PUTS = 3 };

static const char *method_name(int m)
{
	switch (m) {
	case M_FWRITE: return "fwrite";
	case M_PUTC:   return "putc";
	case M_MIX:    return "mix";
	default:       return "fputs";
	}
}

static const size_t GUARD = 64;
static const size_t UBUFSZ = (size_t)BUFSIZ + GUARD;
static const size_t POOLSZ = 32768;
static const size_t FILEMAX = 65536;
static const int NVARIANT = 6;

struct Obs {
	long long wret;
	long pos;
	long long size_before;
	int err;
	int eofv;
	int flushret;
	long long size_after;
	int closeret;
	long long filelen;
	unsigned char snap[UBUFSZ];	/* user buffer just before fflush */
	unsigned char post[UBUFSZ];	/* user buffer after fclose */
	unsigned char file[FILEMAX];
};

static Obs g_obs_port;
static Obs g_obs_ref;
static unsigned char g_ubuf_port[UBUFSZ];
static unsigned char g_ubuf_ref[UBUFSZ];
static unsigned char g_pool[NVARIANT][POOLSZ];
static char g_sbuf[POOLSZ + 1];

static char g_path_port[256];
static char g_path_ref[256];

/* ------------------------------------------------------------------ */

static uint64_t g_state;

static void rng_seed(uint64_t s)
{
	g_state = s;
}

static uint64_t rng_next(void)
{
	uint64_t z = (g_state += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static size_t rng_below(size_t n)
{
	return (size_t)(rng_next() % (uint64_t)n);
}

static void build_pools(void)
{
	rng_seed(0x5eed0098s3ULL & 0);	/* placeholder, replaced below */
}

/* ------------------------------------------------------------------ */

static void make_temp(char *dst, size_t dstsz, const char *tag)
{
	static const char *dirs[] = { "/dev/shm", "/tmp", "." };

	for (size_t i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
		snprintf(dst, dstsz, "%s/pbsd_b0098s3_%s_XXXXXX", dirs[i], tag);
		int fd = mkstemp(dst);
		if (fd >= 0) {
			close(fd);
			return;
		}
	}
	fprintf(stderr, "harness: cannot create temporary file\n");
	exit(2);
}

/* ------------------------------------------------------------------ */

/*
 * side 0 => the ported setbuf, side 1 => ref_setbuf.  Everything else about
 * the two runs is identical.
 */
static void run_one(int side, const char *path, unsigned char *ubuf, Obs &o,
    int with_buf, int twice, int method, const unsigned char *data, size_t n)
{
	struct stat st;

	memset(ubuf, 0x7f, UBUFSZ);
	memset(&o, 0, sizeof(o));
	o.wret = -12345;
	o.pos = -12345;
	o.size_before = -1;
	o.size_after = -1;
	o.filelen = -1;

	FILE *fp = fopen(path, "wb");
	if (fp == NULL) {
		fprintf(stderr, "harness: fopen(%s) failed: %s\n", path,
		    strerror(errno));
		exit(2);
	}

	char *b = with_buf ? (char *)ubuf : (char *)NULL;

	if (twice) {
		if (side == 0)
			port::setbuf(fp, (char *)NULL);
		else
			ref_setbuf(fp, (char *)NULL);
	}
	if (side == 0)
		port::setbuf(fp, b);
	else
		ref_setbuf(fp, b);

	switch (method) {
	case M_FWRITE:
		o.wret = (long long)fwrite(data, 1, n, fp);
		break;
	case M_PUTC: {
		long long c = 0;
		for (size_t i = 0; i < n; i++)
			if (fputc((int)data[i], fp) != EOF)
				c++;
		o.wret = c;
		break;
	}
	case M_MIX: {
		size_t h = n / 2;
		long long c = 0;
		for (size_t i = 0; i < h; i++)
			if (fputc((int)data[i], fp) != EOF)
				c++;
		c += (long long)fwrite(data + h, 1, n - h, fp);
		o.wret = c;
		break;
	}
	default: {
		for (size_t i = 0; i < n; i++)
			g_sbuf[i] = (char)(data[i] == 0 ? 'Z' : data[i]);
		g_sbuf[n] = '\0';
		o.wret = (long long)fputs(g_sbuf, fp);
		break;
	}
	}

	o.pos = ftell(fp);
	o.err = ferror(fp);
	o.eofv = feof(fp);
	if (fstat(fileno(fp), &st) == 0)
		o.size_before = (long long)st.st_size;
	memcpy(o.snap, ubuf, UBUFSZ);

	o.flushret = fflush(fp);
	if (fstat(fileno(fp), &st) == 0)
		o.size_after = (long long)st.st_size;

	o.closeret = fclose(fp);
	memcpy(o.post, ubuf, UBUFSZ);

	int rfd = open(path, O_RDONLY);
	if (rfd >= 0) {
		ssize_t total = 0;
		for (;;) {
			ssize_t r = read(rfd, o.file + total,
			    FILEMAX - (size_t)total);
			if (r <= 0)
				break;
			total += r;
			if ((size_t)total >= FILEMAX)
				break;
		}
		close(rfd);
		o.filelen = (long long)total;
	}
}

/* ------------------------------------------------------------------ */

static long long g_cases;
static long long g_fails;
static int g_reported;

static void report(const char *what, const char *desc)
{
	if (g_reported < 15) {
		printf("  FAIL [%s] %s\n", what, desc);
		g_reported++;
	} else if (g_reported == 15) {
		printf("  ... further failures suppressed\n");
		g_reported++;
	}
}

static size_t first_diff(const unsigned char *a, const unsigned char *b,
    size_t n)
{
	for (size_t i = 0; i < n; i++)
		if (a[i] != b[i])
			return i;
	return n;
}

static void check_case(int with_buf, int twice, int method, int variant,
    size_t off, size_t n)
{
	char desc[256];

	snprintf(desc, sizeof(desc),
	    "buf=%s twice=%d method=%s variant=%d off=%zu n=%zu",
	    with_buf ? "user" : "NULL", twice, method_name(method), variant,
	    off, n);

	const unsigned char *data = g_pool[variant] + off;

	run_one(0, g_path_port, g_ubuf_port, g_obs_port, with_buf, twice,
	    method, data, n);
	run_one(1, g_path_ref, g_ubuf_ref, g_obs_ref, with_buf, twice,
	    method, data, n);

	g_cases++;

	int bad = 0;
	char det[512];

	const Obs &p = g_obs_port;
	const Obs &r = g_obs_ref;

	if (p.wret != r.wret) {
		snprintf(det, sizeof(det), "%s: write ret port=%lld ref=%lld",
		    desc, p.wret, r.wret);
		report("setbuf", det);
		bad = 1;
	}
	if (p.pos != r.pos) {
		snprintf(det, sizeof(det), "%s: ftell port=%ld ref=%ld",
		    desc, p.pos, r.pos);
		report("setbuf", det);
		bad = 1;
	}
	if (p.size_before != r.size_before) {
		snprintf(det, sizeof(det),
		    "%s: bytes on fd before fflush port=%lld ref=%lld",
		    desc, p.size_before, r.size_before);
		report("setbuf", det);
		bad = 1;
	}
	if (p.size_after != r.size_after) {
		snprintf(det, sizeof(det),
		    "%s: bytes on fd after fflush port=%lld ref=%lld",
		    desc, p.size_after, r.size_after);
		report("setbuf", det);
		bad = 1;
	}
	if (p.err != r.err || p.eofv != r.eofv) {
		snprintf(det, sizeof(det),
		    "%s: err/eof port=%d/%d ref=%d/%d",
		    desc, p.err, p.eofv, r.err, r.eofv);
		report("setbuf", det);
		bad = 1;
	}
	if (p.flushret != r.flushret || p.closeret != r.closeret) {
		snprintf(det, sizeof(det),
		    "%s: fflush/fclose port=%d/%d ref=%d/%d",
		    desc, p.flushret, p.closeret, r.flushret, r.closeret);
		report("setbuf", det);
		bad = 1;
	}
	if (memcmp(p.snap, r.snap, UBUFSZ) != 0) {
		size_t i = first_diff(p.snap, r.snap, UBUFSZ);
		snprintf(det, sizeof(det),
		    "%s: user buffer (pre-flush) differs at %zu port=0x%02x "
		    "ref=0x%02x", desc, i, p.snap[i], r.snap[i]);
		report("setbuf", det);
		bad = 1;
	}
	if (memcmp(p.post, r.post, UBUFSZ) != 0) {
		size_t i = first_diff(p.post, r.post, UBUFSZ);
		snprintf(det, sizeof(det),
		    "%s: user buffer (post-close) differs at %zu port=0x%02x "
		    "ref=0x%02x", desc, i, p.post[i], r.post[i]);
		report("setbuf", det);
		bad = 1;
	}
	if (p.filelen != r.filelen) {
		snprintf(det, sizeof(det), "%s: file length port=%lld ref=%lld",
		    desc, p.filelen, r.filelen);
		report("setbuf", det);
		bad = 1;
	} else if (p.filelen > 0 &&
	    memcmp(p.file, r.file, (size_t)p.filelen) != 0) {
		size_t i = first_diff(p.file, r.file, (size_t)p.filelen);
		snprintf(det, sizeof(det),
		    "%s: file contents differ at %zu port=0x%02x ref=0x%02x",
		    desc, i, p.file[i], r.file[i]);
		report("setbuf", det);
		bad = 1;
	}

	if (bad)
		g_fails++;
}

/* ------------------------------------------------------------------ */

static void edge_cases(void)
{
	static const size_t small_n[] = {
		0, 1, 2, 3, 4, 5, 7, 8, 15, 16, 31, 32, 63, 64, 65,
		127, 128, 129, 255, 256, 511, 512, 1023, 1024
	};
	static const size_t big_n[] = {
		2047, 2048, 4095, 4096,
		(size_t)BUFSIZ - 2, (size_t)BUFSIZ - 1, (size_t)BUFSIZ,
		(size_t)BUFSIZ + 1, (size_t)BUFSIZ + 2,
		2 * (size_t)BUFSIZ - 1, 2 * (size_t)BUFSIZ,
		2 * (size_t)BUFSIZ + 1
	};

	for (int with_buf = 0; with_buf < 2; with_buf++)
		for (int twice = 0; twice < 2; twice++)
			for (int method = 0; method < 4; method++)
				for (int v = 0; v < NVARIANT; v++)
					for (size_t i = 0;
					    i < sizeof(small_n) / sizeof(small_n[0]);
					    i++)
						check_case(with_buf, twice,
						    method, v,
						    (i * 37) % 64,
						    small_n[i]);

	/*
	 * The large sizes straddle the BUFSIZ boundary, which is where a
	 * mutated size argument shows up: with a user buffer of exactly
	 * BUFSIZ, byte-at-a-time writing flushes on byte BUFSIZ and not
	 * before.  Restricted to two data variants and twice=0 to keep the
	 * unbuffered byte-at-a-time cases affordable.
	 */
	for (int with_buf = 0; with_buf < 2; with_buf++)
		for (int method = 0; method < 4; method++)
			for (int v = 0; v < 2; v++)
				for (size_t i = 0;
				    i < sizeof(big_n) / sizeof(big_n[0]); i++)
					check_case(with_buf, 0, method,
					    v == 0 ? 0 : 3, 0, big_n[i]);
}

static void random_sweep(long iters)
{
	for (long it = 0; it < iters; it++) {
		uint64_t bits = rng_next();
		int with_buf = (int)(bits & 1);
		int method = (int)((bits >> 1) & 3);
		int twice = ((bits >> 3) % 16) == 0;
		int variant = (int)rng_below(NVARIANT);

		size_t n;
		if (method == M_PUTC)
			n = rng_below(97);
		else if (method == M_MIX)
			n = rng_below(161);
		else
			n = rng_below(2049);

		size_t off = rng_below(POOLSZ - n);

		check_case(with_buf, twice, method, variant, off, n);
	}
}

/* ------------------------------------------------------------------ */

int main(void)
{
	rng_seed(0x0098533141592653ULL);

	for (size_t i = 0; i < POOLSZ; i++)
		g_pool[0][i] = (unsigned char)(rng_next() & 0xff);
	memset(g_pool[1], 0x00, POOLSZ);
	memset(g_pool[2], 0xff, POOLSZ);
	for (size_t i = 0; i < POOLSZ; i++)
		g_pool[3][i] = (unsigned char)(0x80 | (rng_next() & 0x7f));
	for (size_t i = 0; i < POOLSZ; i++)
		g_pool[4][i] = (unsigned char)((i & 1) ? '\n'
		    : (0x20 + (rng_next() % 0x40)));
	for (size_t i = 0; i < POOLSZ; i++)
		g_pool[5][i] = (unsigned char)(i & 0xff);

	make_temp(g_path_port, sizeof(g_path_port), "port");
	make_temp(g_path_ref, sizeof(g_path_ref), "ref");

	rng_seed(0x0098531618033988ULL);

	edge_cases();
	long long edge_cases_run = g_cases;
	long long edge_fails = g_fails;

	random_sweep(200000);

	unlink(g_path_port);
	unlink(g_path_ref);

	printf("\n");
	printf("%-12s %12s %12s %12s\n", "function", "cases", "failures",
	    "result");
	printf("%-12s %12lld %12lld %12s\n", "setbuf", g_cases, g_fails,
	    g_fails == 0 ? "PASS" : "FAIL");
	printf("\n");
	printf("  edge cases      : %lld (failures %lld)\n", edge_cases_run,
	    edge_fails);
	printf("  randomised sweep: %lld (failures %lld)\n",
	    g_cases - edge_cases_run, g_fails - edge_fails);
	printf("\n");
	printf("TOTAL: %lld cases, %lld failures -> %s\n", g_cases, g_fails,
	    g_fails == 0 ? "PASS" : "FAIL");

	return g_fails == 0 ? 0 : 1;
}
