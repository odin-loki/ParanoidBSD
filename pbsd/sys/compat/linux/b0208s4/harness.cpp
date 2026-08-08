// Differential test for PBSD batch b0208s4 (linux_getcwd).

import pbsd.sys.compat.linux.b0208s4;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::sys_compat_linux::b0208s4;

#define GUARD		0x7f
#define LINUX_PATH_MAX	4096
#define ENOMEM		12
#define ERANGE		34
#define EFAULT		14
#define EIO		5
#define EINVAL		22
#define SWEEP		200000L
#define MAX_PRINT	12
#define USER_CAP	(LINUX_PATH_MAX + 64u)

struct stat_row {
	const char *name;
	long cases;
	long failures;
	long printed;
};

static stat_row rows[] = {
	{ "linux_getcwd", 0, 0, 0 },
};

enum { R_LINUX_GETCWD = 0 };

static uint64_t rng_state = 0x00b020804faceULL;

static inline uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline uint32_t
rnd32(void)
{
	return (uint32_t)(rnd64() >> 32);
}

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-28s %-20s %s\n", r.name, label, detail);
	}
}

static void
case_row(int row)
{
	rows[row].cases++;
}

/* ------------------------------------------------------------------ */
/* controllable kernel mocks					      */
/* ------------------------------------------------------------------ */

struct mock_state {
	int vn_error;
	int copyout_error;
	size_t vn_retbuf_offset;
	size_t vn_out_buflen;
	unsigned char fill_byte;
	bool vn_shrink_buflen;
};

static mock_state mock_cfg;

extern "C" {

typedef unsigned long	l_ulong;
typedef long		register_t;

struct thread {
	register_t	td_retval[2];
};

struct linux_getcwd_args {
	char		*buf;
	l_ulong		bufsize;
};

int ref_linux_getcwd(struct thread *td, struct linux_getcwd_args *uap);

void
mock_counters_reset(void)
{
}

void *
hbsd_kmalloc(size_t size, int type, int flags)
{
	void *p;

	(void)type;
	(void)flags;
	p = std::malloc(size);
	if (p != nullptr)
		std::memset(p, GUARD, size);
	return (p);
}

void
hbsd_kfree(void *addr, int type)
{
	(void)type;
	std::free(addr);
}

int
vn_getcwd(char *buf, char **retbuf, size_t *buflen)
{
	size_t cap, outlen, off;

	if (mock_cfg.vn_error != 0)
		return (mock_cfg.vn_error);

	cap = *buflen;
	if (mock_cfg.vn_shrink_buflen)
		outlen = mock_cfg.vn_out_buflen;
	else
		outlen = cap;
	if (outlen > cap)
		outlen = cap;
	*buflen = outlen;

	if (outlen > 0) {
		std::memset(buf, mock_cfg.fill_byte, outlen);
		buf[0] = '/';
		if (outlen > 1)
			buf[outlen - 1] = '\0';
	}

	off = mock_cfg.vn_retbuf_offset;
	if (off >= outlen && outlen > 0)
		off = outlen - 1;
	*retbuf = buf + off;
	return (0);
}

int
copyout(const void *kaddr, void *uaddr, size_t len)
{
	if (mock_cfg.copyout_error != 0)
		return (mock_cfg.copyout_error);
	if (len > 0)
		std::memcpy(uaddr, kaddr, len);
	return (0);
}

} /* extern "C" */

static void
fill_user_buf(char *buf, size_t len)
{
	std::memset(buf, GUARD, len);
}

static bool
bufs_equal(const char *a, const char *b, size_t len)
{
	return (std::memcmp(a, b, len) == 0);
}

static void
init_thread_ref(thread *td)
{
	std::memset(td, 0x55, sizeof(*td));
	td->td_retval[0] = -1;
	td->td_retval[1] = -1;
}

static void
init_thread_port(port::thread *td)
{
	std::memset(td, 0x55, sizeof(*td));
	td->td_retval[0] = -1;
	td->td_retval[1] = -1;
}

static bool
run_linux_getcwd_case(l_ulong bufsize, const char *label)
{
	char ubuf_ref[USER_CAP];
	char ubuf_port[USER_CAP];
	thread td_ref;
	port::thread td_port;
	linux_getcwd_args uap_ref;
	port::linux_getcwd_args uap_port;
	mock_state saved = mock_cfg;
	int err_ref, err_port;
	bool ok = true;

	case_row(R_LINUX_GETCWD);

	fill_user_buf(ubuf_ref, sizeof(ubuf_ref));
	fill_user_buf(ubuf_port, sizeof(ubuf_port));

	init_thread_ref(&td_ref);
	init_thread_port(&td_port);

	uap_ref.buf = ubuf_ref;
	uap_ref.bufsize = bufsize;
	uap_port.buf = ubuf_port;
	uap_port.bufsize = bufsize;

	mock_cfg = saved;
	err_ref = ref_linux_getcwd(&td_ref, &uap_ref);

	mock_cfg = saved;
	err_port = port::linux_getcwd(&td_port, &uap_port);

	if (err_ref != err_port) {
		fail_row(R_LINUX_GETCWD, label, "return value mismatch");
		ok = false;
	}

	if (td_ref.td_retval[0] != td_port.td_retval[0]) {
		fail_row(R_LINUX_GETCWD, label, "td_retval[0] mismatch");
		ok = false;
	}

	if (td_ref.td_retval[1] != td_port.td_retval[1]) {
		fail_row(R_LINUX_GETCWD, label, "td_retval[1] mismatch");
		ok = false;
	}

	if (!bufs_equal(ubuf_ref, ubuf_port, sizeof(ubuf_ref))) {
		fail_row(R_LINUX_GETCWD, label, "user buffer mismatch");
		ok = false;
	}

	return (ok);
}

static void
set_mock_defaults(void)
{
	mock_cfg = {};
	mock_cfg.vn_error = 0;
	mock_cfg.copyout_error = 0;
	mock_cfg.vn_retbuf_offset = 0;
	mock_cfg.vn_out_buflen = 16;
	mock_cfg.fill_byte = (unsigned char)'a';
	mock_cfg.vn_shrink_buflen = false;
}

static void
test_linux_getcwd_hand(void)
{
	set_mock_defaults();

	/* buflen < 2 early return */
	run_linux_getcwd_case(0, "bufsize=0");
	run_linux_getcwd_case(1, "bufsize=1");

	/* minimum valid bufsize */
	mock_cfg.vn_out_buflen = 2;
	run_linux_getcwd_case(2, "bufsize=2");

	/* LINUX_PATH_MAX boundary */
	mock_cfg.vn_out_buflen = LINUX_PATH_MAX;
	run_linux_getcwd_case(LINUX_PATH_MAX, "bufsize=PATH_MAX");
	run_linux_getcwd_case((l_ulong)LINUX_PATH_MAX + 1, "bufsize=PATH_MAX+1");
	run_linux_getcwd_case((l_ulong)LINUX_PATH_MAX + 127,
	    "bufsize=PATH_MAX+127");

	/* ENOMEM remapped to ERANGE */
	mock_cfg.vn_error = ENOMEM;
	run_linux_getcwd_case(64, "vn_getcwd ENOMEM");
	mock_cfg.vn_error = 0;

	/* other vn_getcwd errors pass through */
	mock_cfg.vn_error = EIO;
	run_linux_getcwd_case(64, "vn_getcwd EIO");
	mock_cfg.vn_error = EINVAL;
	run_linux_getcwd_case(64, "vn_getcwd EINVAL");
	mock_cfg.vn_error = 0;

	/* copyout failure */
	mock_cfg.copyout_error = EFAULT;
	run_linux_getcwd_case(64, "copyout EFAULT");
	mock_cfg.copyout_error = 0;

	/* success path with varied output length */
	mock_cfg.vn_shrink_buflen = true;
	mock_cfg.vn_out_buflen = 3;
	run_linux_getcwd_case(64, "shrunk buflen=3");
	mock_cfg.vn_out_buflen = LINUX_PATH_MAX;
	run_linux_getcwd_case((l_ulong)LINUX_PATH_MAX + 50, "shrunk to PATH_MAX");

	/* retbuf offset from kmalloc base */
	mock_cfg.vn_out_buflen = 32;
	mock_cfg.vn_retbuf_offset = 4;
	run_linux_getcwd_case(128, "retbuf offset 4");
	mock_cfg.vn_retbuf_offset = 31;
	run_linux_getcwd_case(128, "retbuf offset 31");

	/* high-bit path bytes */
	mock_cfg.vn_retbuf_offset = 0;
	mock_cfg.fill_byte = 0x80;
	run_linux_getcwd_case(64, "fill 0x80");
	mock_cfg.fill_byte = 0xff;
	run_linux_getcwd_case(64, "fill 0xff");
	mock_cfg.fill_byte = 0x00;
	run_linux_getcwd_case(64, "fill 0x00");

	/* NUL-heavy kernel buffer */
	mock_cfg.fill_byte = '\0';
	mock_cfg.vn_out_buflen = 20;
	run_linux_getcwd_case(64, "NUL-heavy path");

	/* single-char effective path */
	mock_cfg.vn_shrink_buflen = true;
	mock_cfg.vn_out_buflen = 1;
	mock_cfg.fill_byte = 'x';
	run_linux_getcwd_case(8, "outlen=1");

	/* very large declared bufsize */
	mock_cfg.vn_shrink_buflen = false;
	mock_cfg.vn_out_buflen = 0;
	mock_cfg.fill_byte = 'z';
	run_linux_getcwd_case(0xffffffffUL, "bufsize UINT32_MAX");
	run_linux_getcwd_case(0xffffffffffffffffUL, "bufsize UINT64_MAX");
}

static void
test_linux_getcwd_sweep(void)
{
	static const int vn_errors[] = {
		0, ENOMEM, ERANGE, EIO, EINVAL, EFAULT,
	};
	static const int copyout_errors[] = { 0, EFAULT, EIO, EINVAL };
	long i;

	for (i = 0; i < SWEEP; i++) {
		l_ulong bufsize;
		unsigned pick = rnd32() % 100;

		set_mock_defaults();

		if (pick < 3)
			bufsize = (l_ulong)(rnd32() % 2);
		else if (pick < 8)
			bufsize = 2;
		else if (pick < 12)
			bufsize = (l_ulong)LINUX_PATH_MAX;
		else if (pick < 16)
			bufsize = (l_ulong)LINUX_PATH_MAX + 1 +
			    (rnd32() % 256);
		else if (pick < 20)
			bufsize = (l_ulong)LINUX_PATH_MAX - (rnd32() % 4);
		else
			bufsize = 2 + (rnd32() % (LINUX_PATH_MAX + 512));

		mock_cfg.vn_error = vn_errors[rnd32() %
		    (sizeof(vn_errors) / sizeof(vn_errors[0]))];
		mock_cfg.copyout_error = copyout_errors[rnd32() %
		    (sizeof(copyout_errors) / sizeof(copyout_errors[0]))];
		mock_cfg.vn_shrink_buflen = (rnd32() & 1) != 0;
		mock_cfg.vn_out_buflen = 1 + (rnd32() % (LINUX_PATH_MAX + 8));
		mock_cfg.vn_retbuf_offset = rnd32() % 64;
		mock_cfg.fill_byte = (unsigned char)(rnd32() & 0xff);

		run_linux_getcwd_case(bufsize, "sweep");
	}
}

int
main(void)
{
	long total_cases = 0;
	long total_failures = 0;
	size_t nrows = sizeof(rows) / sizeof(rows[0]);

	test_linux_getcwd_hand();
	test_linux_getcwd_sweep();

	std::printf("=== b0208s4 differential test results ===\n");
	std::printf("%-16s %-10s %-10s\n", "function", "cases", "failures");
	for (size_t i = 0; i < nrows; i++) {
		std::printf("%-16s %-10ld %-10ld\n",
		    rows[i].name, rows[i].cases, rows[i].failures);
		total_cases += rows[i].cases;
		total_failures += rows[i].failures;
	}
	std::printf("--------------------------------------\n");
	std::printf("%-16s %-10ld %-10ld\n", "TOTAL", total_cases,
	    total_failures);

	return (total_failures == 0 ? 0 : 1);
}
