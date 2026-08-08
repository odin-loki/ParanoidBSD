/*
 * harness.cpp -- differential test for batch b0132s1 (readdir.c).
 *
 * _getdirentries and _fixtelldir are recording test doubles defined here and
 * linked into both the port and the oracle.  Every observable (return value,
 * errno, DIR state, buffer contents including guard bytes, pointer offsets,
 * mutex traffic, and mock records) is compared side by side.
 */

import pbsd.lib.libc.gen.b0132s1;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <sys/types.h>

namespace port = pbsd::lib_libc_gen::b0132s1;

using dirent = port::dirent;
using DIR = port::DIR;

#define	MAXNAMLEN	255
#define	GUARD		0x7f
#define	BUF_PAD		16u
#define	DIRBUF_LEN	4096
#define	ENTRY_PAD	32u
#define	ENTRY_BUFSZ	(sizeof(dirent) + 2u * ENTRY_PAD)
#define	SWEEP_ITERS	200000L
#define	MAX_PRINT	12

#define	DT_WHT		14
#define	DTF_HIDEW	0x0001
#define	__DTF_READALL	0x0008
#define	__DTF_SKIPREAD	0x0010
#define	RDU_SKIP	0x0001
#define	RDU_SHORT	0x0002

#define	_GENERIC_DIRLEN(namlen)						\
	((offsetof(dirent, d_name) + (namlen) + 1 + 7) & ~7)
#define	_GENERIC_DIRSIZ(dp)	_GENERIC_DIRLEN((dp)->d_namlen)

extern "C" {
dirent *ref__readdir_unlocked(DIR *, int);
dirent *ref_readdir(DIR *);
int ref___readdir_r(DIR *, dirent *, dirent **);
}

/* ------------------------------------------------------------------ shared */

struct stat_row {
	const char	*name;
	long		cases;
	long		failures;
	long		printed;
};

static stat_row rows[] = {
	{ "_readdir_unlocked", 0, 0, 0 },
	{ "readdir", 0, 0, 0 },
	{ "__readdir_r", 0, 0, 0 },
};

#define	R_UNLOCKED	0
#define	R_READDIR	1
#define	R_READDIR_R	2

int __isthreaded = 0;

static uint64_t rng_state = 0xb0132b1feedULL;

static inline uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static inline uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

static inline int
rnd_range(int lo, int hi)
{
	if (hi <= lo)
		return (lo);
	return (lo + (int)(rnd32() % (uint32_t)(hi - lo + 1)));
}

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-22s %-28s %s\n", r.name, label, detail);
	}
}

/* ----------------------------------------------------------- pthread stubs */

struct dir_mutex {
	int	locked;
};

static int g_mutex_lock_calls;
static int g_mutex_unlock_calls;

extern "C" int
_pthread_mutex_lock(struct pthread_mutex **mptr)
{
	g_mutex_lock_calls++;
	if (mptr != NULL && *mptr != NULL)
		((dir_mutex *)*mptr)->locked = 1;
	return (0);
}

extern "C" int
_pthread_mutex_unlock(struct pthread_mutex **mptr)
{
	g_mutex_unlock_calls++;
	if (mptr != NULL && *mptr != NULL)
		((dir_mutex *)*mptr)->locked = 0;
	return (0);
}

static void
mutex_track_reset(void)
{
	g_mutex_lock_calls = 0;
	g_mutex_unlock_calls = 0;
}

/* ------------------------------------------------------ _getdirentries mock */

struct gd_mock {
	ssize_t		ret;
	int		err;
	int		calls;
	off_t		seek_out;
	unsigned char	refill[DIRBUF_LEN];
	size_t		refill_len;
	int		use_refill;
};

static gd_mock g_gd;

static void
gd_mock_reset(void)
{
	std::memset(&g_gd, 0, sizeof(g_gd));
}

static void
gd_mock_set(ssize_t ret, int err)
{
	gd_mock_reset();
	g_gd.ret = ret;
	g_gd.err = err;
}

static void
gd_mock_refill(const unsigned char *data, size_t len, off_t seek_out)
{
	gd_mock_reset();
	g_gd.use_refill = 1;
	g_gd.refill_len = len < DIRBUF_LEN ? len : DIRBUF_LEN;
	std::memcpy(g_gd.refill, data, g_gd.refill_len);
	g_gd.ret = (ssize_t)g_gd.refill_len;
	g_gd.seek_out = seek_out;
}

extern "C" ssize_t
_getdirentries(int fd, char *buf, size_t len, off_t *basep)
{
	(void)fd;

	g_gd.calls++;
	if (g_gd.use_refill && buf != NULL && len > 0) {
		size_t n = g_gd.refill_len;

		if (n > len)
			n = len;
		std::memcpy(buf, g_gd.refill, n);
		g_gd.ret = (ssize_t)n;
	}
	if (basep != NULL)
		*basep = g_gd.seek_out;
	if (g_gd.ret <= 0 && g_gd.err != 0)
		errno = g_gd.err;
	return (g_gd.ret);
}

/* --------------------------------------------------------- _fixtelldir mock */

struct fix_mock {
	int	calls;
	off_t	oldseek;
	size_t	oldloc;
	DIR	*last_dirp;
};

static fix_mock g_fix;

static void
fix_mock_reset(void)
{
	std::memset(&g_fix, 0, sizeof(g_fix));
}

extern "C" void
_fixtelldir(DIR *dirp, off_t oldseek, size_t oldloc)
{
	g_fix.calls++;
	g_fix.oldseek = oldseek;
	g_fix.oldloc = oldloc;
	g_fix.last_dirp = dirp;
}

/* --------------------------------------------------------- directory fixture */

struct dir_fixture {
	unsigned char	raw[BUF_PAD + DIRBUF_LEN + BUF_PAD];
	DIR		dir;
	dir_mutex	mutex;
};

static void
dir_reset(dir_fixture *fx)
{
	std::memset(fx, 0, sizeof(*fx));
	std::memset(fx->raw, GUARD, sizeof(fx->raw));
	fx->dir.dd_fd = 7;
	fx->dir.dd_buf = (char *)(fx->raw + BUF_PAD);
	fx->dir.dd_len = DIRBUF_LEN;
	fx->dir.dd_lock = (port::pthread_mutex *)&fx->mutex;
}

static void
dir_copy(const dir_fixture *src, dir_fixture *dst)
{
	std::memcpy(dst, src, sizeof(*dst));
	dst->dir.dd_buf = (char *)(dst->raw + BUF_PAD);
	dst->dir.dd_lock = (port::pthread_mutex *)&dst->mutex;
}

static bool
bufs_equal(const unsigned char *a, const unsigned char *b, size_t n)
{
	return (std::memcmp(a, b, n) == 0);
}

static bool
dir_obs_equal(const dir_fixture *pa, const dir_fixture *pb)
{
	if (pa->dir.dd_loc != pb->dir.dd_loc)
		return (false);
	if (pa->dir.dd_size != pb->dir.dd_size)
		return (false);
	if (pa->dir.dd_seek != pb->dir.dd_seek)
		return (false);
	if (pa->dir.dd_flags != pb->dir.dd_flags)
		return (false);
	if (!bufs_equal(pa->raw, pb->raw, sizeof(pa->raw)))
		return (false);
	return (true);
}

static long
ptr_offset(const dir_fixture *fx, const dirent *dp)
{
	if (dp == NULL)
		return (-1);
	return ((long)((const char *)dp - fx->dir.dd_buf));
}

static void
write_dirent(char *base, size_t off, ino_t ino, uint8_t type, uint16_t namlen,
    const char *name, uint16_t reclen_override)
{
	dirent *dp;
	size_t i;

	dp = (dirent *)(base + off);
	std::memset(dp, 0, sizeof(*dp));
	dp->d_fileno = ino;
	dp->d_type = type;
	dp->d_namlen = namlen;
	if (namlen > 0 && name != NULL) {
		for (i = 0; i < namlen; i++)
			dp->d_name[i] = name[i];
	}
	if (reclen_override != 0)
		dp->d_reclen = reclen_override;
	else
		dp->d_reclen = (uint16_t)_GENERIC_DIRSIZ(dp);
}

static void
entry_buf_init(unsigned char *buf, uint8_t fill)
{
	std::memset(buf, fill, ENTRY_BUFSZ);
}

static dirent *
entry_slot(unsigned char *buf)
{
	return ((dirent *)(buf + ENTRY_PAD));
}

/* -------------------------------------------------------- comparison helpers */

static bool
check_unlocked(int row, const char *label, dir_fixture *pa, dir_fixture *pb,
    int flags)
{
	dirent *dp_p;
	dirent *dp_r;
	long off_p;
	long off_r;
	int gd_base;
	int gd_p;
	int gd_r;
	int fix_p;
	int fix_r;

	mutex_track_reset();
	gd_base = g_gd.calls;
	fix_mock_reset();

	dp_p = port::_readdir_unlocked(&pa->dir, flags);
	gd_p = g_gd.calls - gd_base;
	fix_p = g_fix.calls;

	dp_r = ref__readdir_unlocked(&pb->dir, flags);
	gd_r = g_gd.calls - gd_base - gd_p;
	fix_r = g_fix.calls - fix_p;

	off_p = ptr_offset(pa, dp_p);
	off_r = ptr_offset(pb, dp_r);

	rows[row].cases++;

	if (off_p != off_r) {
		fail_row(row, label, "pointer offset mismatch");
		return (false);
	}
	if (!dir_obs_equal(pa, pb)) {
		fail_row(row, label, "DIR/buffer state mismatch");
		return (false);
	}
	if (gd_p != gd_r) {
		fail_row(row, label, "_getdirentries call count mismatch");
		return (false);
	}
	if (fix_p != fix_r) {
		fail_row(row, label, "_fixtelldir call count mismatch");
		return (false);
	}
	return (true);
}

static bool
check_readdir(int row, const char *label, dir_fixture *pa, dir_fixture *pb,
    int threaded, int expect_locks)
{
	dirent *dp_p;
	dirent *dp_r;
	long off_p;
	long off_r;
	int prev_threaded;

	prev_threaded = __isthreaded;
	__isthreaded = threaded;
	mutex_track_reset();
	fix_mock_reset();

	dp_p = port::readdir(&pa->dir);
	dp_r = ref_readdir(&pb->dir);

	__isthreaded = prev_threaded;

	off_p = ptr_offset(pa, dp_p);
	off_r = ptr_offset(pb, dp_r);

	rows[row].cases++;

	if (off_p != off_r) {
		fail_row(row, label, "pointer offset mismatch");
		return (false);
	}
	if (!dir_obs_equal(pa, pb)) {
		fail_row(row, label, "DIR/buffer state mismatch");
		return (false);
	}
	if (g_mutex_lock_calls != expect_locks ||
	    g_mutex_unlock_calls != expect_locks) {
		fail_row(row, label, "mutex traffic mismatch");
		return (false);
	}
	return (true);
}

static bool
check_readdir_r(int row, const char *label, dir_fixture *pa, dir_fixture *pb,
    unsigned char *entry_p, unsigned char *entry_r, int threaded,
    int expect_locks, int *out_ret_p, int *out_ret_r, int *out_errno)
{
	dirent *slot_p;
	dirent *slot_r;
	dirent *res_p;
	dirent *res_r;
	int ret_p;
	int ret_r;
	int prev_threaded;
	int saved_errno;

	prev_threaded = __isthreaded;
	__isthreaded = threaded;
	mutex_track_reset();
	fix_mock_reset();

	slot_p = entry_slot(entry_p);
	slot_r = entry_slot(entry_r);
	res_p = NULL;
	res_r = NULL;

	saved_errno = errno;
	ret_p = port::__readdir_r(&pa->dir, slot_p, &res_p);
	ret_r = ref___readdir_r(&pb->dir, slot_r, &res_r);
	if (out_errno != NULL)
		*out_errno = errno;

	__isthreaded = prev_threaded;

	rows[row].cases++;

	if (ret_p != ret_r) {
		fail_row(row, label, "return value mismatch");
		return (false);
	}
	if (out_ret_p != NULL)
		*out_ret_p = ret_p;
	if (out_ret_r != NULL)
		*out_ret_r = ret_r;
	if ((res_p == NULL) != (res_r == NULL)) {
		fail_row(row, label, "result pointer nullness mismatch");
		return (false);
	}
	if (res_p != NULL && res_p != slot_p) {
		fail_row(row, label, "port result not entry");
		return (false);
	}
	if (res_r != NULL && res_r != slot_r) {
		fail_row(row, label, "ref result not entry");
		return (false);
	}
	if (!bufs_equal(entry_p, entry_r, ENTRY_BUFSZ)) {
		fail_row(row, label, "entry buffer mismatch");
		return (false);
	}
	if (!dir_obs_equal(pa, pb)) {
		fail_row(row, label, "DIR/buffer state mismatch");
		return (false);
	}
	if (g_mutex_lock_calls != expect_locks ||
	    g_mutex_unlock_calls != expect_locks) {
		fail_row(row, label, "mutex traffic mismatch");
		return (false);
	}
	(void)saved_errno;
	return (true);
}

/* -------------------------------------------------------- hand-written cases */

static void
test_unlocked_hand(void)
{
	dir_fixture pa, pb;

	dir_reset(&pa);
	dir_reset(&pb);
	pa.dir.dd_size = pb.dir.dd_size =
	    _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf);
	write_dirent(pa.dir.dd_buf, 0, 42, 8, 3, "foo", 0);
	write_dirent(pb.dir.dd_buf, 0, 42, 8, 3, "foo", 0);
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	(void)check_unlocked(R_UNLOCKED, "single_entry", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 0, 8, 1, "a", 0);
	write_dirent(pb.dir.dd_buf, 0, 0, 8, 1, "a", 0);
	write_dirent(pa.dir.dd_buf, _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf),
	    99, 8, 1, "b", 0);
	write_dirent(pb.dir.dd_buf, _GENERIC_DIRSIZ((dirent *)pb.dir.dd_buf),
	    99, 8, 1, "b", 0);
	pa.dir.dd_size = pb.dir.dd_size =
	    _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf) +
	    _GENERIC_DIRSIZ((dirent *)(pa.dir.dd_buf +
	    _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf)));
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	(void)check_unlocked(R_UNLOCKED, "skip_ino0", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 5, DT_WHT, 1, "w", 0);
	write_dirent(pb.dir.dd_buf, 0, 5, DT_WHT, 1, "w", 0);
	write_dirent(pa.dir.dd_buf, _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf),
	    77, 8, 1, "z", 0);
	write_dirent(pb.dir.dd_buf, _GENERIC_DIRSIZ((dirent *)pb.dir.dd_buf),
	    77, 8, 1, "z", 0);
	pa.dir.dd_size = pb.dir.dd_size =
	    _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf) +
	    _GENERIC_DIRSIZ((dirent *)(pa.dir.dd_buf +
	    _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf)));
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD | DTF_HIDEW;
	(void)check_unlocked(R_UNLOCKED, "skip_wht", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 11, 8, MAXNAMLEN, NULL, 0);
	write_dirent(pb.dir.dd_buf, 0, 11, 8, MAXNAMLEN, NULL, 0);
	pa.dir.dd_size = pb.dir.dd_size = _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf);
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	(void)check_unlocked(R_UNLOCKED, "skip_short", &pa, &pb,
	    RDU_SKIP | RDU_SHORT);

	dir_reset(&pa);
	dir_reset(&pb);
	pa.dir.dd_loc = pb.dir.dd_loc = 1;
	pa.dir.dd_size = pb.dir.dd_size = 64;
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	(void)check_unlocked(R_UNLOCKED, "misaligned", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 1, 8, 1, "x", 0);
	write_dirent(pb.dir.dd_buf, 0, 1, 8, 1, "x", 0);
	((dirent *)pa.dir.dd_buf)->d_reclen = 0;
	((dirent *)pb.dir.dd_buf)->d_reclen = 0;
	pa.dir.dd_size = pb.dir.dd_size = 64;
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	(void)check_unlocked(R_UNLOCKED, "reclen_zero", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 1, 8, 1, "x", 0);
	write_dirent(pb.dir.dd_buf, 0, 1, 8, 1, "x", 0);
	((dirent *)pa.dir.dd_buf)->d_reclen = DIRBUF_LEN + 2;
	((dirent *)pb.dir.dd_buf)->d_reclen = DIRBUF_LEN + 2;
	pa.dir.dd_size = pb.dir.dd_size = DIRBUF_LEN;
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	(void)check_unlocked(R_UNLOCKED, "reclen_huge", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	pa.dir.dd_loc = pb.dir.dd_loc = DIRBUF_LEN;
	pa.dir.dd_size = pb.dir.dd_size = DIRBUF_LEN;
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_READALL;
	(void)check_unlocked(R_UNLOCKED, "readall_done", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	gd_mock_set(0, 0);
	dir_copy(&pa, &pb);
	(void)check_unlocked(R_UNLOCKED, "gd_zero", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	gd_mock_set(-1, ENOENT);
	dir_copy(&pa, &pb);
	(void)check_unlocked(R_UNLOCKED, "gd_fail", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 55, 8, 4, "abcd", 0);
	write_dirent(pb.dir.dd_buf, 0, 55, 8, 4, "abcd", 0);
	gd_mock_refill((const unsigned char *)pa.dir.dd_buf,
	    _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf), 123);
	dir_copy(&pa, &pb);
	pa.dir.dd_loc = pb.dir.dd_loc = 0;
	pa.dir.dd_size = pb.dir.dd_size = 0;
	pa.dir.dd_seek = pb.dir.dd_seek = 0;
	pa.dir.dd_flags = pb.dir.dd_flags = 0;
	(void)check_unlocked(R_UNLOCKED, "gd_refill", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	pa.dir.dd_loc = pb.dir.dd_loc = DIRBUF_LEN + 4;
	pa.dir.dd_size = pb.dir.dd_size = DIRBUF_LEN;
	pa.dir.dd_flags = pb.dir.dd_flags = 0;
	gd_mock_set(0, 0);
	dir_copy(&pa, &pb);
	(void)check_unlocked(R_UNLOCKED, "loc_past_size", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 0, 8, 1, "n", 0);
	write_dirent(pb.dir.dd_buf, 0, 0, 8, 1, "n", 0);
	pa.dir.dd_size = pb.dir.dd_size = _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf);
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	(void)check_unlocked(R_UNLOCKED, "ino0_no_skip", &pa, &pb, 0);

	dir_reset(&pa);
	dir_reset(&pb);
	{
		uint16_t reclen;
		size_t loc;

		write_dirent(pa.dir.dd_buf, 0, 88, 8, 1, "t", 0);
		write_dirent(pb.dir.dd_buf, 0, 88, 8, 1, "t", 0);
		loc = 0;
		reclen = (uint16_t)(pa.dir.dd_len + 1 - loc);
		((dirent *)pa.dir.dd_buf)->d_reclen = reclen;
		((dirent *)pb.dir.dd_buf)->d_reclen = reclen;
		pa.dir.dd_size = pb.dir.dd_size = pa.dir.dd_len;
		pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
		(void)check_unlocked(R_UNLOCKED, "reclen_boundary", &pa, &pb,
		    RDU_SKIP);
	}

	dir_reset(&pa);
	dir_reset(&pb);
	{
		char name[4] = { (char)0x80, (char)0xff, (char)0x00, (char)0x7f };

		write_dirent(pa.dir.dd_buf, 0, 12, 8, 4, name, 0);
		write_dirent(pb.dir.dd_buf, 0, 12, 8, 4, name, 0);
		pa.dir.dd_size = pb.dir.dd_size =
		    _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf);
		pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
		(void)check_unlocked(R_UNLOCKED, "highbit_name", &pa, &pb,
		    RDU_SKIP);
	}

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 1, 8, 0, "", 0);
	write_dirent(pb.dir.dd_buf, 0, 1, 8, 0, "", 0);
	write_dirent(pa.dir.dd_buf, _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf),
	    2, 8, 0, "", 0);
	write_dirent(pb.dir.dd_buf, _GENERIC_DIRSIZ((dirent *)pb.dir.dd_buf),
	    2, 8, 0, "", 0);
	pa.dir.dd_size = pb.dir.dd_size =
	    _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf) * 2;
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	(void)check_unlocked(R_UNLOCKED, "empty_names", &pa, &pb, RDU_SKIP);

	dir_reset(&pa);
	dir_reset(&pb);
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	pa.dir.dd_size = pb.dir.dd_size = 0;
	(void)check_unlocked(R_UNLOCKED, "empty_buf", &pa, &pb, RDU_SKIP);
}

static void
test_readdir_hand(void)
{
	dir_fixture pa, pb;

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 33, 8, 2, "hi", 0);
	write_dirent(pb.dir.dd_buf, 0, 33, 8, 2, "hi", 0);
	pa.dir.dd_size = pb.dir.dd_size = _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf);
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	(void)check_readdir(R_READDIR, "single", &pa, &pb, 0, 0);
	(void)check_readdir(R_READDIR, "single_thr", &pa, &pb, 1, 1);
}

static void
test_readdir_r_hand(void)
{
	dir_fixture pa, pb;
	unsigned char entry_p[ENTRY_BUFSZ];
	unsigned char entry_r[ENTRY_BUFSZ];
	int ret_p, ret_r;

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 44, 8, 3, "bar", 0);
	write_dirent(pb.dir.dd_buf, 0, 44, 8, 3, "bar", 0);
	pa.dir.dd_size = pb.dir.dd_size = _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf);
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	entry_buf_init(entry_p, GUARD);
	entry_buf_init(entry_r, GUARD);
	(void)check_readdir_r(R_READDIR_R, "copy_ok", &pa, &pb, entry_p, entry_r,
	    0, 0, &ret_p, &ret_r, NULL);

	dir_reset(&pa);
	dir_reset(&pb);
	pa.dir.dd_loc = pb.dir.dd_loc = DIRBUF_LEN;
	pa.dir.dd_size = pb.dir.dd_size = DIRBUF_LEN;
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_READALL;
	entry_buf_init(entry_p, GUARD);
	entry_buf_init(entry_r, GUARD);
	errno = EIO;
	(void)check_readdir_r(R_READDIR_R, "eof_errno", &pa, &pb, entry_p,
	    entry_r, 0, 0, &ret_p, &ret_r, NULL);

	dir_reset(&pa);
	dir_reset(&pb);
	gd_mock_set(0, EIO);
	entry_buf_init(entry_p, GUARD);
	entry_buf_init(entry_r, GUARD);
	errno = 0;
	(void)check_readdir_r(R_READDIR_R, "gd_errno", &pa, &pb, entry_p,
	    entry_r, 0, 0, &ret_p, &ret_r, NULL);

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 66, 8, MAXNAMLEN, NULL, 0);
	write_dirent(pb.dir.dd_buf, 0, 66, 8, MAXNAMLEN, NULL, 0);
	pa.dir.dd_size = pb.dir.dd_size = _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf);
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD;
	entry_buf_init(entry_p, GUARD);
	entry_buf_init(entry_r, GUARD);
	errno = EEXIST;
	(void)check_readdir_r(R_READDIR_R, "skip_long", &pa, &pb, entry_p,
	    entry_r, 1, 1, &ret_p, &ret_r, NULL);

	dir_reset(&pa);
	dir_reset(&pb);
	write_dirent(pa.dir.dd_buf, 0, 0, DT_WHT, 1, "w", 0);
	write_dirent(pb.dir.dd_buf, 0, 0, DT_WHT, 1, "w", 0);
	pa.dir.dd_size = pb.dir.dd_size = _GENERIC_DIRSIZ((dirent *)pa.dir.dd_buf);
	pa.dir.dd_flags = pb.dir.dd_flags = __DTF_SKIPREAD | DTF_HIDEW;
	entry_buf_init(entry_p, GUARD);
	entry_buf_init(entry_r, GUARD);
	(void)check_readdir_r(R_READDIR_R, "hide_wht", &pa, &pb, entry_p,
	    entry_r, 0, 0, &ret_p, &ret_r, NULL);
}

/* ----------------------------------------------------------- random sweeps */

static void
fill_random_dirent(char *base, size_t off, uint32_t seed)
{
	ino_t ino;
	uint8_t type;
	uint16_t namlen;
	char name[16];
	int i;

	rng_state = seed;
	ino = (ino_t)rnd32();
	type = (uint8_t)rnd_range(0, 14);
	namlen = (uint16_t)rnd_range(0, 15);
	for (i = 0; i < namlen; i++)
		name[i] = (char)rnd_range(0, 255);
	write_dirent(base, off, ino, type, namlen, name, 0);
}

static void
test_unlocked_sweep(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		dir_fixture pa, pb;
		int flags;
		int use_refill;
		char lbl[48];

		gd_mock_reset();
		fix_mock_reset();
		dir_reset(&pa);
		dir_reset(&pb);

		rng_state = 0xb0132b1ULL ^ (uint64_t)i;
		flags = (int)(rnd32() & (RDU_SKIP | RDU_SHORT));
		pa.dir.dd_loc = pb.dir.dd_loc = (size_t)rnd_range(0, DIRBUF_LEN);
		pa.dir.dd_size = pb.dir.dd_size = (size_t)rnd_range(0, DIRBUF_LEN);
		pa.dir.dd_seek = pb.dir.dd_seek = (off_t)rnd_range(-1000, 1000);
		pa.dir.dd_flags = pb.dir.dd_flags =
		    (int)(rnd32() & (__DTF_READALL | __DTF_SKIPREAD | DTF_HIDEW));

		if ((rnd32() & 3u) == 0) {
			size_t off = (size_t)rnd_range(0, DIRBUF_LEN / 2);
			fill_random_dirent(pa.dir.dd_buf, off, (uint32_t)rnd32());
			fill_random_dirent(pb.dir.dd_buf, off, (uint32_t)rnd32());
		}
		if ((rnd32() & 7u) == 0) {
			size_t off2 = (size_t)rnd_range(0, DIRBUF_LEN / 4);
			uint16_t bad = (uint16_t)rnd_range(0, DIRBUF_LEN + 8);
			((dirent *)(pa.dir.dd_buf + off2))->d_reclen = bad;
			((dirent *)(pb.dir.dd_buf + off2))->d_reclen = bad;
		}

		use_refill = ((pa.dir.dd_flags & (__DTF_READALL | __DTF_SKIPREAD)) ==
		    0 && pa.dir.dd_loc == 0);
		if (use_refill) {
			if ((rnd32() & 1u) != 0) {
				size_t n = (size_t)rnd_range(1, 128);
				gd_mock_refill((const unsigned char *)pa.dir.dd_buf,
				    n, (off_t)rnd_range(0, 500));
			} else {
				gd_mock_set((rnd32() & 1u) ? 0 : -1,
				    rnd_range(1, 255));
			}
		}

		dir_copy(&pa, &pb);
		std::snprintf(lbl, sizeof(lbl), "sweep_%ld", i);
		(void)check_unlocked(R_UNLOCKED, lbl, &pa, &pb, flags);
	}
}

static void
test_readdir_sweep(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		dir_fixture pa, pb;
		int threaded;
		char lbl[48];

		dir_reset(&pa);
		dir_reset(&pb);

		rng_state = 0xb0132b1aaULL ^ (uint64_t)i;
		threaded = (int)(rnd32() & 1u);
		pa.dir.dd_loc = pb.dir.dd_loc = (size_t)rnd_range(0, DIRBUF_LEN);
		pa.dir.dd_size = pb.dir.dd_size = (size_t)rnd_range(0, DIRBUF_LEN);
		pa.dir.dd_flags = pb.dir.dd_flags =
		    (int)(rnd32() & (__DTF_READALL | __DTF_SKIPREAD | DTF_HIDEW));
		if ((rnd32() & 1u) != 0) {
			fill_random_dirent(pa.dir.dd_buf, 0, (uint32_t)rnd32());
			fill_random_dirent(pb.dir.dd_buf, 0, (uint32_t)rnd32());
		}
		dir_copy(&pa, &pb);
		std::snprintf(lbl, sizeof(lbl), "sweep_%ld", i);
		(void)check_readdir(R_READDIR, lbl, &pa, &pb, threaded,
		    threaded);
	}
}

static void
test_readdir_r_sweep(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		dir_fixture pa, pb;
		unsigned char entry_p[ENTRY_BUFSZ];
		unsigned char entry_r[ENTRY_BUFSZ];
		int threaded;
		char lbl[48];

		dir_reset(&pa);
		dir_reset(&pb);
		entry_buf_init(entry_p, GUARD);
		entry_buf_init(entry_r, GUARD);

		rng_state = 0xb0132b1bbULL ^ (uint64_t)i;
		threaded = (int)(rnd32() & 1u);
		pa.dir.dd_loc = pb.dir.dd_loc = (size_t)rnd_range(0, DIRBUF_LEN);
		pa.dir.dd_size = pb.dir.dd_size = (size_t)rnd_range(0, DIRBUF_LEN);
		pa.dir.dd_flags = pb.dir.dd_flags =
		    (int)(rnd32() & (__DTF_READALL | __DTF_SKIPREAD | DTF_HIDEW));
		if ((rnd32() & 3u) == 0)
			errno = rnd_range(0, 255);
		else
			errno = 0;
		if ((rnd32() & 1u) != 0) {
			fill_random_dirent(pa.dir.dd_buf, 0, (uint32_t)rnd32());
			fill_random_dirent(pb.dir.dd_buf, 0, (uint32_t)rnd32());
		}
		if ((pa.dir.dd_flags & (__DTF_READALL | __DTF_SKIPREAD)) == 0 &&
		    pa.dir.dd_loc == 0 && (rnd32() & 3u) == 0) {
			if ((rnd32() & 1u) != 0)
				gd_mock_refill((const unsigned char *)pa.dir.dd_buf,
				    (size_t)rnd_range(1, 256), 0);
			else
				gd_mock_set(-1, rnd_range(1, 255));
		}
		dir_copy(&pa, &pb);
		std::snprintf(lbl, sizeof(lbl), "sweep_%ld", i);
		(void)check_readdir_r(R_READDIR_R, lbl, &pa, &pb, entry_p,
		    entry_r, threaded, threaded, NULL, NULL, NULL);
	}
}

/* ------------------------------------------------------------------- main */

int
main(void)
{
	long total_fail = 0;
	size_t n;

	test_unlocked_hand();
	test_readdir_hand();
	test_readdir_r_hand();
	test_unlocked_sweep();
	test_readdir_sweep();
	test_readdir_r_sweep();

	std::printf("\n%-22s %10s %10s\n", "function", "cases", "failures");
	for (n = 0; n < sizeof(rows) / sizeof(rows[0]); n++) {
		std::printf("%-22s %10ld %10ld\n",
		    rows[n].name, rows[n].cases, rows[n].failures);
		total_fail += rows[n].failures;
	}

	return (total_fail == 0 ? 0 : 1);
}
