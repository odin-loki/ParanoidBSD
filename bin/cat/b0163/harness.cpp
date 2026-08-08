/*
 * harness.cpp -- differential test for PBSD batch b0163
 *                (hbsd/src/bin/cat/cat.c -> pbsd.bin.cat.b0163).
 *
 * Every function in the batch is driven twice per case: once through the C
 * oracle (ref_*) and once through the C++23 port, from identical starting
 * state, and everything observable is compared:
 *
 *   - the return value (in_kernel_copy) and the exit status (usage);
 *   - the exact bytes the call put on stdout, and the length of them;
 *   - the exact bytes the call put on stderr (warn()/err() output);
 *   - the rval global;
 *   - errno after the call;
 *   - the input stream's position, EOF flag and error flag;
 *   - stdin's EOF/error flags (cook_cat() clears them for fp == stdin);
 *   - all 20 bytes of the stream padding that cook_cat()'s resync memset()
 *     writes into, pre-filled with the guard byte 0x7f on both sides so a
 *     wrong fill value or a wrong length shows up.
 *
 * The stdout/stderr capture buffers are likewise pre-filled with the 0x7f
 * guard byte and compared 32 bytes past the longer of the two write windows,
 * so a port that writes further than the oracle is caught even when the two
 * agree about how much they wrote.
 *
 * None of the four functions returns a pointer or holds iterator state, so the
 * pointer-offset and strsep-style drive-to-exhaustion rules do not apply here;
 * the stream position/flag comparison after every call is the equivalent for
 * cook_cat(), which consumes its FILE * incrementally.
 *
 * usage() ends the process, so it is driven two ways: in-process, capturing
 * the status handed to exit(3) with on_exit(3) and returning through longjmp(3)
 * (cheap enough for the full sweep), and in a forked child that really exits,
 * which cross-checks the in-process technique against a real process exit.
 */

import pbsd.bin.cat.b0163;

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wchar.h>

namespace P = pbsd::bin_cat::b0163;

extern "C" {
void ref_usage(void) __attribute__((__noreturn__));
void ref_cook_cat(FILE *);
ssize_t ref_in_kernel_copy(int);
void ref_raw_cat(int);
extern int ref_bflag, ref_eflag, ref_lflag, ref_nflag, ref_sflag, ref_tflag,
    ref_vflag;
extern int ref_rval;
extern const char *ref_filename;
}

/* ------------------------------------------------------------------ state */

#define	CAP	(1u << 17)	/* stdout capture capacity */
#define	ECAP	4096u		/* stderr capture capacity */
#define	MBSN	20		/* sizeof(((FILE *)0)->_unused2) */
#define	GUARD	0x7f
#define	FNAME	"b0163-input"
#define	MAXIN	65536u

struct Snap {
	long ret;
	long rval;
	long err;
	long olen;
	long elen;
	long fpos;
	int feofFlag;
	int ferrFlag;
	int stdinEof;
	int stdinErr;
	unsigned char mbs[MBSN];
	unsigned char ebuf[ECAP];
	unsigned char obuf[CAP];
};

static Snap A;	/* oracle */
static Snap B;	/* port */

enum { FN_USAGE, FN_COOK, FN_IKC, FN_RAW, FN_N };
static const char *fnName[FN_N] = { "usage", "cook_cat", "in_kernel_copy",
    "raw_cat" };
static long fnCases[FN_N];
static long fnFails[FN_N];
static int fnShown[FN_N];

static FILE *rep;		/* the real stdout, for the report */
static int inChild;		/* set in forked children */
static int earlyExit = 1;	/* cleared once main() reaches its end */

static char dirPath[128];
static char inPath[192], outPath[192], errPath[192], wrPath[192];
static char forkErr[2][192];
static int inFd = -1;		/* O_RDWR on inPath */
static int outRd = -1;		/* O_RDONLY on outPath */
static int errRd = -1;		/* O_RDONLY on errPath */
static unsigned char inBuf[MAXIN];

static void
fatal(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(rep ? rep : stderr, "harness: fatal: ");
	vfprintf(rep ? rep : stderr, fmt, ap);
	fprintf(rep ? rep : stderr, " (errno=%d %s)\n", errno, strerror(errno));
	va_end(ap);
	if (rep)
		fflush(rep);
	_exit(2);
}

/* ------------------------------------------------------------------- rng */

static uint64_t rngState;

static void
rngSeed(uint64_t s)
{
	rngState = s;
}

static uint64_t
rnd64(void)
{
	uint64_t z = (rngState += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static unsigned
rnd(unsigned n)
{
	return ((unsigned)(rnd64() % n));
}

/* ------------------------------------------------------------- plumbing */

static long
errOff(void)
{
	long off;

	fflush(stderr);
	off = (long)lseek(2, 0, SEEK_CUR);
	if (off < 0)
		fatal("lseek on stderr");
	return (off);
}

static void
writeInput(const unsigned char *data, size_t len)
{
	if (ftruncate(inFd, 0) != 0)
		fatal("ftruncate input");
	if (len > 0 && pwrite(inFd, data, len, 0) != (ssize_t)len)
		fatal("pwrite input");
}

static void
freshStdout(void)
{
	if (freopen(outPath, "w", stdout) == NULL)
		fatal("freopen stdout");
}

static void
resetStdoutRaw(void)
{
	if (fflush(stdout) != 0)
		fatal("fflush stdout");
	if (ftruncate(1, 0) != 0)
		fatal("ftruncate stdout");
	if (lseek(1, 0, SEEK_SET) != 0)
		fatal("lseek stdout");
}

static void
grabOut(Snap &s, int viaStdio)
{
	long n;

	if (viaStdio)
		fflush(stdout);
	n = (long)lseek(1, 0, SEEK_CUR);
	if (n < 0)
		fatal("lseek stdout for capture");
	if ((unsigned long)n + 32 > CAP)
		fatal("stdout capture overflow (%ld bytes)", n);
	s.olen = n;
	memset(s.obuf, GUARD, (size_t)n + 32);
	if (n > 0 && pread(outRd, s.obuf, (size_t)n, 0) != n)
		fatal("pread stdout capture");
}

static void
grabErr(Snap &s, long e0)
{
	long n = errOff() - e0;

	if (n < 0)
		fatal("stderr went backwards");
	if ((unsigned long)n + 32 > ECAP)
		fatal("stderr capture overflow (%ld bytes)", n);
	s.elen = n;
	memset(s.ebuf, GUARD, (size_t)n + 32);
	if (n > 0 && pread(errRd, s.ebuf, (size_t)n, e0) != n)
		fatal("pread stderr capture");
}

static void
snapInit(Snap &s)
{
	s.ret = s.rval = s.err = s.olen = s.elen = s.fpos = 0;
	s.feofFlag = s.ferrFlag = s.stdinEof = s.stdinErr = 0;
	memset(s.mbs, GUARD, MBSN);
	memset(s.obuf, GUARD, 32);
	memset(s.ebuf, GUARD, 32);
}

/* ------------------------------------------------------------ comparison */

static void
dumpBytes(const char *tag, const unsigned char *b, long n, long limit)
{
	long i;

	fprintf(rep, "      %s[%ld]:", tag, n);
	for (i = 0; i < n && i < limit; i++)
		fprintf(rep, " %02x", b[i]);
	if (n > limit)
		fprintf(rep, " ...");
	fprintf(rep, "\n");
}

static int
cmpSnaps(int fn, const char *desc)
{
	long cmpo, cmpe;
	int bad = 0;

	fnCases[fn]++;

	cmpo = (A.olen > B.olen ? A.olen : B.olen) + 32;
	cmpe = (A.elen > B.elen ? A.elen : B.elen) + 32;
	if ((unsigned long)cmpo > CAP)
		cmpo = CAP;
	if ((unsigned long)cmpe > ECAP)
		cmpe = ECAP;

	if (A.ret != B.ret || A.rval != B.rval || A.err != B.err ||
	    A.olen != B.olen || A.elen != B.elen || A.fpos != B.fpos ||
	    A.feofFlag != B.feofFlag || A.ferrFlag != B.ferrFlag ||
	    A.stdinEof != B.stdinEof || A.stdinErr != B.stdinErr ||
	    memcmp(A.mbs, B.mbs, MBSN) != 0 ||
	    memcmp(A.obuf, B.obuf, (size_t)cmpo) != 0 ||
	    memcmp(A.ebuf, B.ebuf, (size_t)cmpe) != 0)
		bad = 1;

	if (!bad)
		return (1);

	fnFails[fn]++;
	if (fnShown[fn]++ < 8) {
		fprintf(rep, "  FAIL %s: %s\n", fnName[fn], desc);
		if (A.ret != B.ret)
			fprintf(rep, "      ret:   ref=%ld port=%ld\n", A.ret,
			    B.ret);
		if (A.rval != B.rval)
			fprintf(rep, "      rval:  ref=%ld port=%ld\n", A.rval,
			    B.rval);
		if (A.err != B.err)
			fprintf(rep, "      errno: ref=%ld port=%ld\n", A.err,
			    B.err);
		if (A.fpos != B.fpos)
			fprintf(rep, "      ftell: ref=%ld port=%ld\n", A.fpos,
			    B.fpos);
		if (A.feofFlag != B.feofFlag || A.ferrFlag != B.ferrFlag)
			fprintf(rep, "      feof/ferror: ref=%d/%d port=%d/%d\n",
			    A.feofFlag, A.ferrFlag, B.feofFlag, B.ferrFlag);
		if (A.stdinEof != B.stdinEof || A.stdinErr != B.stdinErr)
			fprintf(rep, "      stdin eof/err: ref=%d/%d port=%d/%d\n",
			    A.stdinEof, A.stdinErr, B.stdinEof, B.stdinErr);
		if (memcmp(A.mbs, B.mbs, MBSN) != 0) {
			dumpBytes("ref  mbstate", A.mbs, MBSN, MBSN);
			dumpBytes("port mbstate", B.mbs, MBSN, MBSN);
		}
		if (A.olen != B.olen ||
		    memcmp(A.obuf, B.obuf, (size_t)cmpo) != 0) {
			dumpBytes("ref  stdout", A.obuf, A.olen + 8, 48);
			dumpBytes("port stdout", B.obuf, B.olen + 8, 48);
		}
		if (A.elen != B.elen ||
		    memcmp(A.ebuf, B.ebuf, (size_t)cmpe) != 0) {
			dumpBytes("ref  stderr", A.ebuf, A.elen, 64);
			dumpBytes("port stderr", B.ebuf, B.elen, 64);
		}
		fflush(rep);
	}
	return (0);
}

/* ------------------------------------------------------------ input data */

static const unsigned char specialBytes[] = {
	'\n', '\t', '\0', 'a', ' ', 0x7f, 0x80, 0xff, 0x01, 0x1f, '$', 'M',
	0x0d, 0xc3, 0xa9, 0x40
};

static size_t
randomBytes(unsigned char *dst, size_t len)
{
	size_t i = 0;

	while (i < len) {
		unsigned k = rnd(100);

		if (k < 8 && i + 4 <= len) {
			/* a valid multibyte sequence, for the UTF-8 pass */
			switch (rnd(3)) {
			case 0:
				dst[i++] = 0xc3;
				dst[i++] = 0xa9;
				break;
			case 1:
				dst[i++] = 0xe2;
				dst[i++] = 0x82;
				dst[i++] = 0xac;
				break;
			default:
				dst[i++] = 0xf0;
				dst[i++] = 0x9f;
				dst[i++] = 0x98;
				dst[i++] = 0x80;
				break;
			}
		} else if (k < 38)
			dst[i++] = specialBytes[rnd(sizeof(specialBytes))];
		else if (k < 58)
			dst[i++] = (unsigned char)rnd(256);
		else if (k < 73)
			dst[i++] = (unsigned char)(0x80 + rnd(128));
		else if (k < 86)
			dst[i++] = (unsigned char)rnd(0x20);
		else
			dst[i++] = (unsigned char)(0x20 + rnd(0x5f));
	}
	return (len);
}

/* ------------------------------------------------------------ flag setup */

static void
setFlags(int side, unsigned f)
{
	if (side) {
		P::bflag = (f >> 0) & 1;
		P::eflag = (f >> 1) & 1;
		P::nflag = (f >> 2) & 1;
		P::sflag = (f >> 3) & 1;
		P::tflag = (f >> 4) & 1;
		P::vflag = (f >> 5) & 1;
		P::rval = 0;
		P::filename = FNAME;
	} else {
		ref_bflag = (f >> 0) & 1;
		ref_eflag = (f >> 1) & 1;
		ref_nflag = (f >> 2) & 1;
		ref_sflag = (f >> 3) & 1;
		ref_tflag = (f >> 4) & 1;
		ref_vflag = (f >> 5) & 1;
		ref_rval = 0;
		ref_filename = FNAME;
	}
}

/* ------------------------------------------------------------ usage tests */

static jmp_buf jb;
static volatile int caughtStatus;

static void
exitHook(int status, void *arg)
{
	(void)arg;
	caughtStatus = status;
	longjmp(jb, 1);
}

/*
 * Everything runUsage() still needs after the longjmp lives at file scope, so
 * that no local has to survive it.
 */
static Snap *usageSnap;
static int usageSide;
static long usageE0, usageO0;

static void
runUsage(int side, Snap &s)
{
	snapInit(s);
	usageSnap = &s;
	usageSide = side;
	fflush(stdout);
	usageO0 = (long)lseek(1, 0, SEEK_CUR);
	usageE0 = errOff();
	caughtStatus = -999;
	if (setjmp(jb) == 0) {
		if (on_exit(exitHook, NULL) != 0)
			fatal("on_exit");
		if (side)
			P::usage();
		else
			ref_usage();
	}
	usageSnap->ret = caughtStatus;
	grabErr(*usageSnap, usageE0);
	fflush(stdout);
	usageSnap->olen = (long)lseek(1, 0, SEEK_CUR) - usageO0;
	usageSnap->rval = usageSide ? P::rval : ref_rval;
}

static void
runUsageForked(int side, Snap &s)
{
	pid_t pid;
	int st = 0;
	long n;
	int fd;

	snapInit(s);
	fflush(NULL);
	pid = fork();
	if (pid < 0)
		fatal("fork");
	if (pid == 0) {
		inChild = 1;
		if (freopen(forkErr[side], "w", stderr) == NULL)
			_exit(71);
		if (side)
			P::usage();
		else
			ref_usage();
		_exit(70);	/* NOTREACHED */
	}
	if (waitpid(pid, &st, 0) != pid)
		fatal("waitpid");
	s.ret = WIFEXITED(st) ? WEXITSTATUS(st) : -128 - WTERMSIG(st);
	fd = open(forkErr[side], O_RDONLY);
	if (fd < 0)
		fatal("open fork stderr");
	n = (long)lseek(fd, 0, SEEK_END);
	if (n < 0 || (unsigned long)n + 32 > ECAP)
		fatal("fork stderr size %ld", n);
	s.elen = n;
	memset(s.ebuf, GUARD, (size_t)n + 32);
	if (n > 0 && pread(fd, s.ebuf, (size_t)n, 0) != n)
		fatal("pread fork stderr");
	close(fd);
}

static void
usageTests(void)
{
	char desc[64];
	int i;

	/* hand-written: both orders, all three stderr buffering modes */
	for (i = 0; i < 6; i++) {
		int mode = i % 3;

		fflush(stderr);
		setvbuf(stderr, NULL, mode == 0 ? _IONBF :
		    (mode == 1 ? _IOLBF : _IOFBF), mode == 2 ? 512 : 0);
		snprintf(desc, sizeof(desc), "directed mode=%d order=%d", mode,
		    i / 3);
		if (i / 3 == 0) {
			runUsage(0, A);
			runUsage(1, B);
		} else {
			runUsage(1, B);
			runUsage(0, A);
		}
		cmpSnaps(FN_USAGE, desc);
	}
	fflush(stderr);
	setvbuf(stderr, NULL, _IONBF, 0);

	/* real process exits, to validate the in-process technique */
	for (i = 0; i < 24; i++) {
		snprintf(desc, sizeof(desc), "forked #%d", i);
		if (i & 1) {
			runUsageForked(1, B);
			runUsageForked(0, A);
		} else {
			runUsageForked(0, A);
			runUsageForked(1, B);
		}
		cmpSnaps(FN_USAGE, desc);
	}

	/*
	 * Sweep.  usage() takes no arguments, so what varies is the state it
	 * runs against: which side goes first, how stderr is buffered and how
	 * much unrelated output precedes it.
	 */
	rngSeed(0x0163000000000001ULL);
	for (i = 0; i < 200000; i++) {
		unsigned k = rnd(3);
		unsigned pre = rnd(4);
		unsigned j;

		fflush(stderr);
		setvbuf(stderr, NULL, k == 0 ? _IONBF :
		    (k == 1 ? _IOLBF : _IOFBF), 0);
		for (j = 0; j < pre; j++)
			fputc((int)('0' + rnd(10)), stderr);
		if (rnd(2)) {
			runUsage(0, A);
			runUsage(1, B);
		} else {
			runUsage(1, B);
			runUsage(0, A);
		}
		snprintf(desc, sizeof(desc), "sweep #%d buf=%u pre=%u", i, k,
		    pre);
		cmpSnaps(FN_USAGE, desc);
	}
	fflush(stderr);
	setvbuf(stderr, NULL, _IONBF, 0);
}

/* --------------------------------------------------------- cook_cat tests */

enum { IK_FILE, IK_STDIN, IK_STDIN_EOF, IK_FILE_STDIN_EOF, IK_DIR, IK_N };
static const char *ikName[IK_N] = { "file", "stdin", "stdin-at-eof",
    "file+stdin-at-eof", "directory" };

static void
drainStdin(void)
{
	if (freopen(inPath, "r", stdin) == NULL)
		fatal("freopen stdin");
	while (getc(stdin) != EOF)
		;
}

static void
runCook(int side, Snap &s, int kind, unsigned flags)
{
	FILE *fp;
	long e0;

	snapInit(s);
	freshStdout();
	e0 = errOff();

	switch (kind) {
	case IK_STDIN:
		if (freopen(inPath, "r", stdin) == NULL)
			fatal("freopen stdin");
		fp = stdin;
		break;
	case IK_STDIN_EOF:
		drainStdin();
		fp = stdin;
		break;
	case IK_FILE_STDIN_EOF:
		drainStdin();
		fp = fopen(inPath, "r");
		break;
	case IK_DIR:
		if (freopen("/dev/null", "r", stdin) == NULL)
			fatal("freopen stdin");
		fp = fopen(dirPath, "r");
		break;
	default:
		if (freopen("/dev/null", "r", stdin) == NULL)
			fatal("freopen stdin");
		fp = fopen(inPath, "r");
		break;
	}
	if (fp == NULL)
		fatal("open cook_cat input (kind %d)", kind);
	memset(&fp->_unused2, GUARD, MBSN);

	setFlags(side, flags);
	errno = 0;
	if (side)
		P::cook_cat(fp);
	else
		ref_cook_cat(fp);

	s.err = errno;
	s.rval = side ? P::rval : ref_rval;
	s.fpos = ftell(fp);
	s.feofFlag = feof(fp) ? 1 : 0;
	s.ferrFlag = ferror(fp) ? 1 : 0;
	s.stdinEof = feof(stdin) ? 1 : 0;
	s.stdinErr = ferror(stdin) ? 1 : 0;
	memcpy(s.mbs, &fp->_unused2, MBSN);
	grabOut(s, 1);
	grabErr(s, e0);
	if (fp != stdin)
		fclose(fp);
}

static void
cookCase(int kind, unsigned flags, const unsigned char *data, size_t len,
    const char *what)
{
	char desc[160];

	writeInput(data, len);
	if (rnd64() & 1) {
		runCook(0, A, kind, flags);
		runCook(1, B, kind, flags);
	} else {
		runCook(1, B, kind, flags);
		runCook(0, A, kind, flags);
	}
	snprintf(desc, sizeof(desc), "%s len=%zu flags=%02x(b%d e%d n%d s%d "
	    "t%d v%d) in=%s", what, len, flags, (flags >> 0) & 1,
	    (flags >> 1) & 1, (flags >> 2) & 1, (flags >> 3) & 1,
	    (flags >> 4) & 1, (flags >> 5) & 1, ikName[kind]);
	cmpSnaps(FN_COOK, desc);
}

struct Pat {
	const char *p;
	size_t n;
};
#define	PAT(s)	{ s, sizeof(s) - 1 }

static const Pat cookPats[] = {
	PAT(""),
	PAT("\n"),
	PAT("\n\n"),
	PAT("\n\n\n"),
	PAT("\n\n\n\n"),
	PAT("a"),
	PAT("ab"),
	PAT("a\n"),
	PAT("\na"),
	PAT("a\nb"),
	PAT("a\nb\n"),
	PAT("\n\na\n\n\nb\n"),
	PAT("\t"),
	PAT("\t\t"),
	PAT("a\tb"),
	PAT("\ta\t"),
	PAT("\n\t\n"),
	PAT("\0"),
	PAT("a\0b"),
	PAT("\0\0\0\0\0\0\0\0"),
	PAT("\x7f"),
	PAT("\x80"),
	PAT("\xff"),
	PAT("\x80\x81\x82"),
	PAT("\xff\xff"),
	PAT("\x01"),
	PAT("\x1f"),
	PAT("\x20"),
	PAT("\x01\x02\x03\x1f\x7f"),
	PAT("\xc3\xa9"),
	PAT("\xe2\x82\xac"),
	PAT("\xf0\x9f\x98\x80"),
	PAT("\xc3"),
	PAT("\xc0\x80"),
	PAT("\xed\xa0\x80"),
	PAT("a\xc3\xa9\n"),
	PAT("\n\xff\n"),
	PAT("\t\xff\t"),
	PAT("\x7f\x7f\x7f"),
	PAT("M-"),
	PAT("\xff\n\xff"),
	PAT("\x80\n"),
	PAT("\xe2\x82"),
	PAT("\xf0\x9f"),
	PAT("\r\n"),
	PAT("^A"),
	PAT("\x40\x41\x60"),
	PAT("\xbf\xc1\xf5\xfe"),
	PAT("\n\x7f\n\x80\n\t\n"),
	PAT("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\n")
};
#define	NCOOKPATS	(sizeof(cookPats) / sizeof(cookPats[0]))

static void
cookDirected(void)
{
	unsigned char buf[4096];
	unsigned flags;
	size_t i;

	for (i = 0; i < NCOOKPATS; i++)
		for (flags = 0; flags < 64; flags++)
			cookCase(IK_FILE, flags,
			    (const unsigned char *)cookPats[i].p,
			    cookPats[i].n, "pattern");

	/* every byte value, in order */
	for (i = 0; i < 256; i++)
		buf[i] = (unsigned char)i;
	for (flags = 0; flags < 64; flags++)
		cookCase(IK_FILE, flags, buf, 256, "all-256-bytes");

	/* boundary lengths around stdio's buffer size */
	memset(buf, 'a', sizeof(buf));
	for (flags = 0; flags < 64; flags += 7) {
		cookCase(IK_FILE, flags, buf, 4095, "4095-a");
		cookCase(IK_FILE, flags, buf, 4096, "4096-a");
	}
	memset(buf, '\n', sizeof(buf));
	for (flags = 0; flags < 64; flags += 7)
		cookCase(IK_FILE, flags, buf, 1000, "1000-newlines");
	memset(buf, 0xff, sizeof(buf));
	for (flags = 0; flags < 64; flags += 7)
		cookCase(IK_FILE, flags, buf, 600, "600-0xff");
	memset(buf, 0x00, sizeof(buf));
	for (flags = 0; flags < 64; flags += 7)
		cookCase(IK_FILE, flags, buf, 600, "600-nul");

	/* the stdin-flavoured paths and the error path */
	for (i = 0; i < 21; i++)
		for (flags = 0; flags < 64; flags++) {
			cookCase(IK_STDIN, flags,
			    (const unsigned char *)cookPats[i].p,
			    cookPats[i].n, "pattern");
			cookCase(IK_STDIN_EOF, flags,
			    (const unsigned char *)cookPats[i].p,
			    cookPats[i].n, "pattern");
			cookCase(IK_FILE_STDIN_EOF, flags,
			    (const unsigned char *)cookPats[i].p,
			    cookPats[i].n, "pattern");
		}
	for (flags = 0; flags < 64; flags++)
		cookCase(IK_DIR, flags, (const unsigned char *)"", 0,
		    "unreadable-stream");
}

static void
cookSweep(int n, uint64_t seed)
{
	int i;

	rngSeed(seed);
	for (i = 0; i < n; i++) {
		unsigned k = rnd(100);
		size_t len;
		unsigned flags = rnd(64);
		int kind;

		if (k < 40)
			len = rnd(9);
		else if (k < 80)
			len = rnd(65);
		else if (k < 95)
			len = rnd(513);
		else
			len = rnd(4097);
		randomBytes(inBuf, len);

		k = rnd(1000);
		if (k < 900)
			kind = IK_FILE;
		else if (k < 935)
			kind = IK_STDIN;
		else if (k < 968)
			kind = IK_STDIN_EOF;
		else if (k < 998)
			kind = IK_FILE_STDIN_EOF;
		else
			kind = IK_DIR;
		cookCase(kind, flags, inBuf, len, "sweep");
	}
}

/* ------------------------------------------------ source-descriptor kinds */

enum { FK_FILE, FK_SEQ, FK_PIPE, FK_DEVNULL, FK_DIR, FK_CLOSED, FK_WRONLY,
    FK_N };
static const char *fkName[FK_N] = { "regular-file", "seqpacket", "pipe",
    "/dev/null", "directory", "closed-fd", "write-only-fd" };

struct Src {
	int fd;
	int aux;
};

static Src
openSrc(int kind, const unsigned char *data, size_t len, unsigned nsplit)
{
	Src s;
	int p[2];
	size_t done, i;

	s.fd = -1;
	s.aux = -1;
	switch (kind) {
	case FK_FILE:
		if (lseek(inFd, 0, SEEK_SET) != 0)
			fatal("lseek input");
		s.fd = inFd;
		break;
	case FK_SEQ:
		if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, p) != 0)
			fatal("socketpair");
		if (nsplit < 1)
			nsplit = 1;
		done = 0;
		for (i = 0; i < nsplit; i++) {
			size_t chunk = (len - done) / (nsplit - i);

			if (i == nsplit - 1)
				chunk = len - done;
			if (send(p[1], data + done, chunk, 0) !=
			    (ssize_t)chunk)
				fatal("send seqpacket %zu", chunk);
			done += chunk;
		}
		close(p[1]);
		s.fd = p[0];
		break;
	case FK_PIPE:
		if (pipe(p) != 0)
			fatal("pipe");
		if (len > 0 && write(p[1], data, len) != (ssize_t)len)
			fatal("write pipe");
		close(p[1]);
		s.fd = p[0];
		break;
	case FK_DEVNULL:
		s.fd = open("/dev/null", O_RDONLY);
		break;
	case FK_DIR:
		s.fd = open(dirPath, O_RDONLY | O_DIRECTORY);
		break;
	case FK_CLOSED:
		s.fd = 9999;
		break;
	case FK_WRONLY:
		s.fd = open(wrPath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
		break;
	}
	if (s.fd < 0)
		fatal("openSrc kind %d", kind);
	return (s);
}

static void
closeSrc(int kind, Src s)
{
	if (kind == FK_FILE || kind == FK_CLOSED)
		return;
	close(s.fd);
	if (s.aux >= 0)
		close(s.aux);
}

/* --------------------------------------------------- in_kernel_copy tests */

static void
runIkc(int side, Snap &s, int kind, const unsigned char *data, size_t len,
    unsigned nsplit)
{
	Src src;
	long e0;
	ssize_t r;

	snapInit(s);
	resetStdoutRaw();
	e0 = errOff();
	src = openSrc(kind, data, len, nsplit);
	setFlags(side, 0);
	errno = 0;
	r = side ? P::in_kernel_copy(src.fd) : ref_in_kernel_copy(src.fd);
	s.err = errno;
	s.ret = (long)r;
	s.rval = side ? P::rval : ref_rval;
	grabOut(s, 0);
	grabErr(s, e0);
	closeSrc(kind, src);
}

static void
ikcCase(int kind, const unsigned char *data, size_t len, unsigned nsplit,
    const char *what)
{
	char desc[128];

	if (kind == FK_FILE)
		writeInput(data, len);
	if (rnd64() & 1) {
		runIkc(0, A, kind, data, len, nsplit);
		runIkc(1, B, kind, data, len, nsplit);
	} else {
		runIkc(1, B, kind, data, len, nsplit);
		runIkc(0, A, kind, data, len, nsplit);
	}
	snprintf(desc, sizeof(desc), "%s len=%zu split=%u src=%s", what, len,
	    nsplit, fkName[kind]);
	cmpSnaps(FN_IKC, desc);
}

static const size_t edgeLens[] = { 0, 1, 2, 3, 4, 7, 8, 63, 64, 65, 511, 512,
    513, 1023, 1024, 4095, 4096, 4097, 8191, 8192, 16384, 32768, 65535, 65536 };
#define	NEDGELENS	(sizeof(edgeLens) / sizeof(edgeLens[0]))

static void
fillPattern(unsigned char *b, size_t len, int which)
{
	size_t i;

	switch (which) {
	case 0:
		memset(b, 0, len);
		break;
	case 1:
		memset(b, 0xff, len);
		break;
	case 2:
		for (i = 0; i < len; i++)
			b[i] = (unsigned char)i;
		break;
	default:
		randomBytes(b, len);
		break;
	}
}

static void
ikcTests(void)
{
	size_t i;
	int w;

	rngSeed(0x0163000000000002ULL);
	for (i = 0; i < NEDGELENS; i++)
		for (w = 0; w < 4; w++) {
			size_t len = edgeLens[i];

			fillPattern(inBuf, len, w);
			ikcCase(FK_FILE, inBuf, len, 1, "edge");
			if (len <= 32768) {
				ikcCase(FK_PIPE, inBuf, len, 1, "edge");
				ikcCase(FK_SEQ, inBuf, len, 1 + (unsigned)(len ?
				    (len > 3 ? 3 : 1) : 1), "edge");
			}
		}
	for (w = 0; w < 4; w++) {
		ikcCase(FK_DEVNULL, inBuf, 0, 1, "special");
		ikcCase(FK_DIR, inBuf, 0, 1, "special");
		ikcCase(FK_CLOSED, inBuf, 0, 1, "special");
		ikcCase(FK_WRONLY, inBuf, 0, 1, "special");
	}

	rngSeed(0x0163000000000003ULL);
	for (i = 0; i < 200000; i++) {
		unsigned k = rnd(100);
		size_t len;
		int kind;

		if (k < 50)
			len = rnd(65);
		else if (k < 85)
			len = rnd(1025);
		else if (k < 98)
			len = rnd(8193);
		else
			len = rnd(MAXIN + 1);
		randomBytes(inBuf, len);

		k = rnd(1000);
		if (k < 920)
			kind = FK_FILE;
		else if (k < 960 && len <= 32768)
			kind = FK_PIPE;
		else if (k < 995 && len <= 32768)
			kind = FK_SEQ;
		else
			kind = FK_DEVNULL + (int)rnd(4);
		ikcCase(kind, inBuf, len, 1 + rnd(4), "sweep");
	}
}

/* ---------------------------------------------------------- raw_cat tests */

static void
runRaw(int side, Snap &s, int kind, const unsigned char *data, size_t len,
    unsigned nsplit)
{
	Src src;
	long e0;

	snapInit(s);
	resetStdoutRaw();
	e0 = errOff();
	src = openSrc(kind, data, len, nsplit);
	setFlags(side, 0);
	errno = 0;
	if (side)
		P::raw_cat(src.fd);
	else
		ref_raw_cat(src.fd);
	s.err = errno;
	s.rval = side ? P::rval : ref_rval;
	grabOut(s, 0);
	grabErr(s, e0);
	closeSrc(kind, src);
}

static void
rawCase(int kind, const unsigned char *data, size_t len, unsigned nsplit,
    const char *what)
{
	char desc[128];

	if (kind == FK_FILE)
		writeInput(data, len);
	if (rnd64() & 1) {
		runRaw(0, A, kind, data, len, nsplit);
		runRaw(1, B, kind, data, len, nsplit);
	} else {
		runRaw(1, B, kind, data, len, nsplit);
		runRaw(0, A, kind, data, len, nsplit);
	}
	snprintf(desc, sizeof(desc), "%s len=%zu split=%u src=%s", what, len,
	    nsplit, fkName[kind]);
	cmpSnaps(FN_RAW, desc);
}

static void
rawTests(void)
{
	size_t i;
	int w;
	unsigned sp;

	rngSeed(0x0163000000000004ULL);
	for (i = 0; i < NEDGELENS; i++)
		for (w = 0; w < 4; w++) {
			size_t len = edgeLens[i];

			fillPattern(inBuf, len, w);
			rawCase(FK_FILE, inBuf, len, 1, "edge");
			if (len <= 32768) {
				rawCase(FK_PIPE, inBuf, len, 1, "edge");
				for (sp = 1; sp <= 5; sp++)
					if (sp <= len || sp == 1)
						rawCase(FK_SEQ, inBuf, len, sp,
						    "edge");
			}
		}
	for (w = 0; w < 4; w++) {
		rawCase(FK_DEVNULL, inBuf, 0, 1, "special");
		rawCase(FK_DIR, inBuf, 0, 1, "special");
		rawCase(FK_CLOSED, inBuf, 0, 1, "special");
		rawCase(FK_WRONLY, inBuf, 0, 1, "special");
	}

	rngSeed(0x0163000000000005ULL);
	for (i = 0; i < 200000; i++) {
		unsigned k = rnd(100);
		size_t len;
		int kind;

		if (k < 50)
			len = rnd(65);
		else if (k < 85)
			len = rnd(1025);
		else if (k < 98)
			len = rnd(8193);
		else
			len = rnd(MAXIN + 1);
		randomBytes(inBuf, len);

		k = rnd(1000);
		if (k < 830)
			kind = FK_FILE;
		else if (k < 900 && len <= 32768)
			kind = FK_PIPE;
		else if (k < 995 && len <= 32768)
			kind = FK_SEQ;
		else
			kind = FK_DEVNULL + (int)rnd(4);
		rawCase(kind, inBuf, len, 1 + rnd(6), "sweep");
	}
}

/* ------------------------------------------------------------ setup/report */

static void
report(void)
{
	long tc = 0, tf = 0;
	int i;

	fprintf(rep, "\n");
	fprintf(rep, "  %-16s %10s %10s  %s\n", "function", "cases",
	    "failures", "result");
	fprintf(rep, "  %-16s %10s %10s  %s\n", "----------------",
	    "----------", "----------", "------");
	for (i = 0; i < FN_N; i++) {
		fprintf(rep, "  %-16s %10ld %10ld  %s\n", fnName[i],
		    fnCases[i], fnFails[i], fnFails[i] ? "FAIL" : "ok");
		tc += fnCases[i];
		tf += fnFails[i];
	}
	fprintf(rep, "  %-16s %10s %10s  %s\n", "----------------",
	    "----------", "----------", "------");
	fprintf(rep, "  %-16s %10ld %10ld  %s\n", "TOTAL", tc, tf,
	    tf ? "FAIL" : "ok");
	fprintf(rep, "\n%s\n", tf ? "RESULT: FAIL" : "RESULT: PASS");
	fflush(rep);
}

static void
atExitReport(void)
{
	if (inChild || !earlyExit || rep == NULL)
		return;
	fprintf(rep, "\nharness: terminated before finishing "
	    "(a call to err(3) inside the port or the oracle?)\n");
	report();
}

static void
setup(void)
{
	const char *tmp = getenv("TMPDIR");
	int fd;

	if (tmp == NULL || *tmp == '\0')
		tmp = "/tmp";
	snprintf(dirPath, sizeof(dirPath), "%s/pbsd-b0163-XXXXXX", tmp);
	if (mkdtemp(dirPath) == NULL) {
		perror("mkdtemp");
		exit(2);
	}
	snprintf(inPath, sizeof(inPath), "%s/in", dirPath);
	snprintf(outPath, sizeof(outPath), "%s/out", dirPath);
	snprintf(errPath, sizeof(errPath), "%s/err", dirPath);
	snprintf(wrPath, sizeof(wrPath), "%s/wronly", dirPath);
	snprintf(forkErr[0], sizeof(forkErr[0]), "%s/fork-ref", dirPath);
	snprintf(forkErr[1], sizeof(forkErr[1]), "%s/fork-port", dirPath);

	setvbuf(stdout, NULL, _IOLBF, 0);
	fd = dup(1);
	if (fd < 0 || (rep = fdopen(fd, "w")) == NULL) {
		perror("dup stdout");
		exit(2);
	}
	setvbuf(rep, NULL, _IOLBF, 0);

	inFd = open(inPath, O_RDWR | O_CREAT | O_TRUNC, 0600);
	fd = open(outPath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (inFd < 0 || fd < 0)
		fatal("create scratch files in %s", dirPath);
	close(fd);
	outRd = open(outPath, O_RDONLY);
	if (outRd < 0)
		fatal("open %s for reading", outPath);
	if (freopen(errPath, "w", stderr) == NULL)
		fatal("freopen stderr");
	setvbuf(stderr, NULL, _IONBF, 0);
	errRd = open(errPath, O_RDONLY);
	if (errRd < 0)
		fatal("open %s for reading", errPath);
	if (freopen(outPath, "w", stdout) == NULL)
		fatal("freopen stdout");
	if (freopen("/dev/null", "r", stdin) == NULL)
		fatal("freopen stdin");
	if (atexit(atExitReport) != 0)
		fatal("atexit");
}

static void
cleanup(void)
{
	if (inFd >= 0)
		close(inFd);
	if (outRd >= 0)
		close(outRd);
	if (errRd >= 0)
		close(errRd);
	unlink(inPath);
	unlink(outPath);
	unlink(errPath);
	unlink(wrPath);
	unlink(forkErr[0]);
	unlink(forkErr[1]);
	rmdir(dirPath);
}

int
main(void)
{
	const char *utf8 = NULL;
	static const char *cands[] = { "C.UTF-8", "en_US.UTF-8", "C.utf8" };
	size_t i;

	setup();
	setlocale(LC_CTYPE, "C");

	fprintf(rep, "pbsd b0163: differential test of cat.c "
	    "(usage, cook_cat, in_kernel_copy, raw_cat)\n");
	fprintf(rep, "  scratch dir: %s\n", dirPath);

	fprintf(rep, "  usage ...\n");
	usageTests();

	fprintf(rep, "  cook_cat (LC_CTYPE=C) ...\n");
	cookDirected();
	cookSweep(200000, 0x0163000000000010ULL);

	for (i = 0; i < sizeof(cands) / sizeof(cands[0]) && utf8 == NULL; i++)
		if (setlocale(LC_CTYPE, cands[i]) != NULL)
			utf8 = cands[i];
	if (utf8 != NULL) {
		fprintf(rep, "  cook_cat (LC_CTYPE=%s) ...\n", utf8);
		cookDirected();
		cookSweep(60000, 0x0163000000000011ULL);
		setlocale(LC_CTYPE, "C");
	} else
		fprintf(rep, "  cook_cat: no UTF-8 locale available, "
		    "C locale only\n");

	fprintf(rep, "  in_kernel_copy ...\n");
	ikcTests();

	fprintf(rep, "  raw_cat ...\n");
	rawTests();

	earlyExit = 0;
	report();
	cleanup();

	for (i = 0; i < FN_N; i++)
		if (fnFails[i] != 0)
			return (1);
	return (0);
}
