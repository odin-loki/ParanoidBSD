/*
 * oracle.c - reference implementation ("the specification") for PBSD batch
 * b0146.
 *
 * The four HardenedBSD sources of this batch are reproduced below, in order,
 * with their original copyright headers.  Every function has been renamed with
 * a "ref_" prefix and file-static functions have been given external linkage so
 * that the differential harness can reach them.  No function body has been
 * modified in any other way.
 *
 * Because these are kernel sources, the environment they run in (types,
 * constants, and the handful of external routines they call: malloc(9),
 * read_random_uio(9), mtx(9), condvar(9), physmem(9), printf(9), KASSERT and
 * CTR*) is modelled deterministically in the first part of this file.  The
 * C++23 port in port.cppm calls exactly these same model routines, so both
 * sides observe an identical environment and any divergence is a divergence of
 * the ported code itself.
 *
 * Sources:
 *   hbsd/src/sys/kern/sys_getrandom.c
 *   hbsd/src/sys/kern/subr_efi_map.c
 *   hbsd/src/sys/kern/subr_hash.c
 *   hbsd/src/sys/kern/kern_sema.c
 */

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Defines the kernel headers would otherwise supply. */
#ifndef LONG_BIT
#define	LONG_BIT		(8 * (int)sizeof(long))
#endif
#ifndef NBBY
#define	NBBY			8
#endif
#ifndef __unused
#define	__unused		__attribute__((__unused__))
#endif
#ifndef __inline
#define	__inline		inline
#endif

typedef unsigned long	u_long;
typedef long		register_t;

#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#define	CTASSERT(x)	_Static_assert((x), "compile-time assertion failed")

/* ------------------------------------------------------------------------ */
/* sys/queue.h (the list macros subr_hash.c uses)                           */
/* ------------------------------------------------------------------------ */

#define	LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;						\
}
#define	LIST_FIRST(head)	((head)->lh_first)
#define	LIST_EMPTY(head)	(LIST_FIRST((head)) == NULL)
#define	LIST_INIT(head) do {						\
	LIST_FIRST((head)) = NULL;					\
} while (0)

/* ------------------------------------------------------------------------ */
/* sys/malloc.h, sys/systm.h                                                */
/* ------------------------------------------------------------------------ */

struct malloc_type {
	const char	*ks_shortdesc;
};

#define	M_NOWAIT	0x0001
#define	M_WAITOK	0x0002

#define	HASH_NOWAIT	0x00000001
#define	HASH_WAITOK	0x00000002

/* ------------------------------------------------------------------------ */
/* sys/uio.h, sys/proc.h, sys/random.h, sys/limits.h                        */
/* ------------------------------------------------------------------------ */

enum uio_rw { UIO_READ, UIO_WRITE };
enum uio_seg { UIO_USERSPACE, UIO_SYSSPACE, UIO_NOCOPY };

struct iovec {
	void	*iov_base;
	size_t	 iov_len;
};

struct thread {
	register_t	td_retval[2];
	long		td_pad;
};

struct uio {
	struct iovec	*uio_iov;
	int		 uio_iovcnt;
	long long	 uio_offset;	/* off_t */
	long		 uio_resid;	/* ssize_t */
	enum uio_seg	 uio_segflg;
	enum uio_rw	 uio_rw;
	struct thread	*uio_td;
};

#define	IOSIZE_MAX	INT_MAX

#define	GRND_NONBLOCK	0x0001
#define	GRND_RANDOM	0x0002
#define	GRND_INSECURE	0x0004

/* ------------------------------------------------------------------------ */
/* sys/efi.h, sys/efi_map.h, machine/efi.h, sys/physmem.h                   */
/* ------------------------------------------------------------------------ */

struct efi_map_header {
	uint64_t	memory_size;
	uint64_t	descriptor_size;
	uint32_t	descriptor_version;
};

struct efi_md {
	uint32_t	md_type;
	uint32_t	md_pad;
	uint64_t	md_phys;
	uint64_t	md_virt;
	uint64_t	md_pages;
	uint64_t	md_attr;
};

#define	EFI_MD_TYPE_NULL	0
#define	EFI_MD_TYPE_CODE	1
#define	EFI_MD_TYPE_DATA	2
#define	EFI_MD_TYPE_BS_CODE	3
#define	EFI_MD_TYPE_BS_DATA	4
#define	EFI_MD_TYPE_RT_CODE	5
#define	EFI_MD_TYPE_RT_DATA	6
#define	EFI_MD_TYPE_FREE	7
#define	EFI_MD_TYPE_BAD		8
#define	EFI_MD_TYPE_RECLAIM	9
#define	EFI_MD_TYPE_FIRMWARE	10
#define	EFI_MD_TYPE_IOMEM	11
#define	EFI_MD_TYPE_IOPORT	12
#define	EFI_MD_TYPE_PALCODE	13
#define	EFI_MD_TYPE_PERSISTENT	14

#define	EFI_MD_ATTR_UC		0x0000000000000001UL
#define	EFI_MD_ATTR_WC		0x0000000000000002UL
#define	EFI_MD_ATTR_WT		0x0000000000000004UL
#define	EFI_MD_ATTR_WB		0x0000000000000008UL
#define	EFI_MD_ATTR_UCE		0x0000000000000010UL
#define	EFI_MD_ATTR_WP		0x0000000000001000UL
#define	EFI_MD_ATTR_RP		0x0000000000002000UL
#define	EFI_MD_ATTR_XP		0x0000000000004000UL
#define	EFI_MD_ATTR_NV		0x0000000000008000UL
#define	EFI_MD_ATTR_MORE_RELIABLE 0x0000000000010000UL
#define	EFI_MD_ATTR_RO		0x0000000000020000UL
#define	EFI_MD_ATTR_RT		0x8000000000000000UL

#define	EFI_PAGE_SHIFT		12
#define	EFI_PAGE_SIZE		(1 << EFI_PAGE_SHIFT)

#define	EXFLAG_NOALLOC		0x0001

typedef void (*efi_map_entry_cb)(struct efi_md *, void *argp);

static struct efi_md *
efi_next_descriptor(struct efi_md *md, size_t size)
{

	return ((struct efi_md *)(((uint8_t *)md) + size));
}

/* ------------------------------------------------------------------------ */
/* sys/mutex.h, sys/condvar.h, sys/sema.h, sys/ktr.h                        */
/* ------------------------------------------------------------------------ */

#define	MTX_DEF			0x00000000
#define	MTX_SPIN		0x00000001
#define	MTX_NOWITNESS		0x00000008
#define	MTX_QUIET		0x00040000

#define	KTR_LOCK		0x00000004

/*
 * The modelled mutex keeps hashes rather than pointers for its two
 * descriptions: one of them is a string literal that lives in this translation
 * unit for the oracle and in port.cppm for the port, so the addresses
 * necessarily differ while the contents do not, and the harness compares the
 * raw bytes of struct sema.
 */
struct mtx {
	unsigned long long	mtx_name_hash;
	unsigned long long	mtx_type_hash;
	int			mtx_flags;
	int			mtx_locked;
};

struct cv {
	const char	*cv_description;
	int		 cv_waits;
	int		 cv_signals;
};

struct sema {
	struct mtx	sema_mtx;	/* General protection lock. */
	struct cv	sema_cv;	/* Waiters. */
	int		sema_value;	/* Semaphore value. */
	int		sema_waiters;	/* Number of waiters. */
};

/* ======================================================================== */
/* Deterministic model of the kernel environment.                           */
/* ======================================================================== */

enum {
	MOP_ASSERT = 1,
	MOP_BZERO,
	MOP_MALLOC,
	MOP_FREE,
	MOP_RANDOM_UIO,
	MOP_MTX_INIT,
	MOP_MTX_DESTROY,
	MOP_MTX_LOCK,
	MOP_MTX_UNLOCK,
	MOP_CV_INIT,
	MOP_CV_DESTROY,
	MOP_CV_WAIT,
	MOP_CV_TIMEDWAIT,
	MOP_CV_SIGNAL,
	MOP_CV_WMESG,
	MOP_CTR,
	MOP_PHYS_HW,
	MOP_PHYS_EXCL,
	MOP_PRINTF,
	MOP_NOTE
};

#define	MODEL_LOG_MAX		16384
#define	MODEL_OUT_MAX		262144
#define	MODEL_ARENA_SIZE	262144
#define	MODEL_GUARD		0x7f
#define	MODEL_WAIT_RUNAWAY	256

struct model_ev {
	int			mev_op;
	unsigned long long	mev_a, mev_b, mev_c, mev_d;
};

static struct model_ev	model_log[MODEL_LOG_MAX];
static int		model_log_n;
static int		model_log_over;
static struct model_ev	model_slog[MODEL_LOG_MAX];
static int		model_slog_n;
static int		model_slog_over;

static char	model_out[MODEL_OUT_MAX];
static size_t	model_out_n;
static int	model_out_over;
static char	model_sout[MODEL_OUT_MAX];
static size_t	model_sout_n;
static int	model_sout_over;

static unsigned char	model_arena_mem[MODEL_ARENA_SIZE];
static unsigned char	model_sarena[MODEL_ARENA_SIZE];
static int		model_arena_dirty = 1;	/* forces the first guard fill */
static int		model_sarena_valid;
static size_t		model_arena_off;
static size_t		model_sarena_off;

static int	model_runaway;
static int	model_srunaway;
static int	model_total_waits;

/* Policy knobs driven by the harness; not part of the compared state. */
int		model_malloc_fail;
unsigned long	model_malloc_max = MODEL_ARENA_SIZE;
int		model_random_error;
int		model_random_nb_fail;
unsigned long	model_random_cap;
unsigned char	model_random_seed;
int		model_cv_release_after = 1;
int		model_cv_post_amount = 1;
int		model_timedwait_post_after;
int		model_timedwait_fail_after = 1;
struct sema    *model_sema_target;

static void
model_ev_log(int op, unsigned long long a, unsigned long long b,
    unsigned long long c, unsigned long long d)
{

	if (model_log_n >= MODEL_LOG_MAX) {
		model_log_over++;
		return;
	}
	model_log[model_log_n].mev_op = op;
	model_log[model_log_n].mev_a = a;
	model_log[model_log_n].mev_b = b;
	model_log[model_log_n].mev_c = c;
	model_log[model_log_n].mev_d = d;
	model_log_n++;
}

static unsigned long long
model_strhash(const char *s)
{
	unsigned long long h = 1469598103934665603ULL;

	if (s == NULL)
		return (0);
	while (*s != '\0') {
		h ^= (unsigned long long)(unsigned char)*s++;
		h *= 1099511628211ULL;
	}
	return (h);
}

static long long
model_tgt_value(void)
{

	return (model_sema_target != NULL ?
	    (long long)model_sema_target->sema_value : -999999);
}

static long long
model_tgt_waiters(void)
{

	return (model_sema_target != NULL ?
	    (long long)model_sema_target->sema_waiters : -999999);
}

void
model_set_sema_target(void *p)
{

	model_sema_target = (struct sema *)p;
}

unsigned char *
model_arena_base(void)
{

	return (model_arena_mem);
}

size_t
model_arena_size(void)
{

	return (MODEL_ARENA_SIZE);
}

const char *
model_out_text(void)
{

	return (model_out);
}

size_t
model_out_length(void)
{

	return (model_out_n);
}

int
model_log_length(void)
{

	return (model_log_n);
}

void
model_reset(void)
{

	model_log_n = 0;
	model_log_over = 0;
	model_out_n = 0;
	model_out[0] = '\0';
	model_out_over = 0;
	model_runaway = 0;
	model_total_waits = 0;
	model_arena_off = 0;
	if (model_arena_dirty) {
		memset(model_arena_mem, MODEL_GUARD, MODEL_ARENA_SIZE);
		model_arena_dirty = 0;
	}
}

void
model_snapshot(void)
{

	memcpy(model_slog, model_log,
	    sizeof(model_log[0]) * (size_t)model_log_n);
	model_slog_n = model_log_n;
	model_slog_over = model_log_over;
	memcpy(model_sout, model_out, model_out_n + 1);
	model_sout_n = model_out_n;
	model_sout_over = model_out_over;
	model_srunaway = model_runaway;
	model_sarena_off = model_arena_off;
	if (model_arena_dirty) {
		memcpy(model_sarena, model_arena_mem, MODEL_ARENA_SIZE);
		model_sarena_valid = 1;
	} else
		model_sarena_valid = 0;
}

static const char *
model_op_name(int op)
{

	switch (op) {
	case MOP_ASSERT:	return ("KASSERT");
	case MOP_BZERO:		return ("bzero");
	case MOP_MALLOC:	return ("malloc");
	case MOP_FREE:		return ("free");
	case MOP_RANDOM_UIO:	return ("read_random_uio");
	case MOP_MTX_INIT:	return ("mtx_init");
	case MOP_MTX_DESTROY:	return ("mtx_destroy");
	case MOP_MTX_LOCK:	return ("mtx_lock");
	case MOP_MTX_UNLOCK:	return ("mtx_unlock");
	case MOP_CV_INIT:	return ("cv_init");
	case MOP_CV_DESTROY:	return ("cv_destroy");
	case MOP_CV_WAIT:	return ("cv_wait");
	case MOP_CV_TIMEDWAIT:	return ("cv_timedwait");
	case MOP_CV_SIGNAL:	return ("cv_signal");
	case MOP_CV_WMESG:	return ("cv_wmesg");
	case MOP_CTR:		return ("CTR");
	case MOP_PHYS_HW:	return ("physmem_hardware_region");
	case MOP_PHYS_EXCL:	return ("physmem_exclude_region");
	case MOP_PRINTF:	return ("printf");
	case MOP_NOTE:		return ("callback");
	default:		return ("?");
	}
}

/*
 * Compare everything the environment observed during the port's run against
 * what it observed during the oracle's run: the full call/effect log, the
 * captured printf(9) output, the entire malloc arena (including every byte past
 * whatever was handed out), and the runaway-loop flag.  Returns 0 when
 * identical.
 */
int
model_diff(char *msg, size_t msgsz)
{
	size_t i;
	int n;

	if (model_runaway != model_srunaway) {
		snprintf(msg, msgsz, "runaway loop flag: ref=%d port=%d",
		    model_srunaway, model_runaway);
		return (1);
	}
	if (model_log_over != model_slog_over) {
		snprintf(msg, msgsz, "log overflow: ref=%d port=%d",
		    model_slog_over, model_log_over);
		return (1);
	}
	if (model_log_n != model_slog_n) {
		snprintf(msg, msgsz, "env call count: ref=%d port=%d",
		    model_slog_n, model_log_n);
		return (1);
	}
	for (n = 0; n < model_log_n; n++) {
		if (model_log[n].mev_op == model_slog[n].mev_op &&
		    model_log[n].mev_a == model_slog[n].mev_a &&
		    model_log[n].mev_b == model_slog[n].mev_b &&
		    model_log[n].mev_c == model_slog[n].mev_c &&
		    model_log[n].mev_d == model_slog[n].mev_d)
			continue;
		snprintf(msg, msgsz,
		    "env call %d: ref=%s(%llu,%llu,%llu,%llu) "
		    "port=%s(%llu,%llu,%llu,%llu)", n,
		    model_op_name(model_slog[n].mev_op), model_slog[n].mev_a,
		    model_slog[n].mev_b, model_slog[n].mev_c,
		    model_slog[n].mev_d,
		    model_op_name(model_log[n].mev_op), model_log[n].mev_a,
		    model_log[n].mev_b, model_log[n].mev_c,
		    model_log[n].mev_d);
		return (1);
	}
	if (model_out_over != model_sout_over || model_out_n != model_sout_n) {
		snprintf(msg, msgsz, "printf output length: ref=%zu port=%zu",
		    model_sout_n, model_out_n);
		return (1);
	}
	if (memcmp(model_out, model_sout, model_out_n) != 0) {
		snprintf(msg, msgsz, "printf output text differs");
		return (1);
	}
	if (model_arena_off != model_sarena_off) {
		snprintf(msg, msgsz, "arena bytes handed out: ref=%zu port=%zu",
		    model_sarena_off, model_arena_off);
		return (1);
	}
	if (model_sarena_valid) {
		if (memcmp(model_arena_mem, model_sarena,
		    MODEL_ARENA_SIZE) == 0)
			return (0);
		for (i = 0; i < MODEL_ARENA_SIZE; i++) {
			if (model_arena_mem[i] == model_sarena[i])
				continue;
			snprintf(msg, msgsz,
			    "arena byte %zu: ref=0x%02x port=0x%02x", i,
			    model_sarena[i], model_arena_mem[i]);
			return (1);
		}
		return (1);
	}
	if (model_arena_dirty) {
		for (i = 0; i < MODEL_ARENA_SIZE; i++) {
			if (model_arena_mem[i] == MODEL_GUARD)
				continue;
			snprintf(msg, msgsz,
			    "arena byte %zu written by the port only: 0x%02x",
			    i, model_arena_mem[i]);
			return (1);
		}
	}
	return (0);
}

/* KASSERT(9) */
void
model_assert(int cond)
{

	model_ev_log(MOP_ASSERT, (unsigned long long)(cond != 0), 0, 0, 0);
}

/* CTR3/CTR4/CTR5/CTR6 */
void
model_ctr(int arity, long long v1, long long v2, const char *s)
{

	model_ev_log(MOP_CTR, (unsigned long long)arity,
	    (unsigned long long)v1, (unsigned long long)v2,
	    model_strhash(s));
}

void
model_bzero(void *p, unsigned long len)
{

	model_ev_log(MOP_BZERO, (unsigned long long)len, 0, 0, 0);
	memset(p, 0, (size_t)len);
}

/* malloc(9) and free(9), served from a guard-filled bump arena. */
void *
model_kmalloc(unsigned long size, struct malloc_type *type, int flags)
{
	unsigned long long name;
	unsigned long need;
	void *ret;

	name = model_strhash(type != NULL ? type->ks_shortdesc : NULL);
	need = (size + 15UL) & ~15UL;
	if (model_malloc_fail != 0 || size > model_malloc_max ||
	    (unsigned long long)need >
	    (unsigned long long)(MODEL_ARENA_SIZE - model_arena_off)) {
		model_ev_log(MOP_MALLOC, (unsigned long long)size,
		    (unsigned long long)(unsigned int)flags, ~0ULL, name);
		return (NULL);
	}
	ret = &model_arena_mem[model_arena_off];
	model_ev_log(MOP_MALLOC, (unsigned long long)size,
	    (unsigned long long)(unsigned int)flags,
	    (unsigned long long)model_arena_off, name);
	model_arena_off += need;
	model_arena_dirty = 1;
	return (ret);
}

void
model_kfree(void *p, struct malloc_type *type)
{
	unsigned long long off;

	if (p == NULL)
		off = ~0ULL;
	else if ((unsigned char *)p >= model_arena_mem &&
	    (unsigned char *)p < model_arena_mem + MODEL_ARENA_SIZE)
		off = (unsigned long long)((unsigned char *)p -
		    model_arena_mem);
	else
		off = ~1ULL;
	model_ev_log(MOP_FREE, off, 0, 0,
	    model_strhash(type != NULL ? type->ks_shortdesc : NULL));
}

/* printf(9), captured. */
int
model_printf(const char *fmt, ...)
{
	char tmp[1024];
	va_list ap;
	size_t len;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	if (n > 0) {
		len = (size_t)n;
		if (len > sizeof(tmp) - 1)
			len = sizeof(tmp) - 1;
		if (model_out_n + len + 1 < MODEL_OUT_MAX) {
			memcpy(model_out + model_out_n, tmp, len);
			model_out_n += len;
			model_out[model_out_n] = '\0';
		} else
			model_out_over++;
	}
	model_ev_log(MOP_PRINTF, (unsigned long long)(long long)n, 0, 0, 0);
	return (n);
}

/* read_random_uio(9): deterministic, and never writes more than the cap. */
int
model_read_random_uio(struct uio *uio, bool nonblock)
{
	unsigned char *p;
	unsigned long n, i;

	model_ev_log(MOP_RANDOM_UIO, (unsigned long long)(nonblock ? 1 : 0),
	    (unsigned long long)(long long)uio->uio_resid,
	    (unsigned long long)(long long)uio->uio_iovcnt,
	    (unsigned long long)(long long)uio->uio_offset);
	if (nonblock && model_random_nb_fail != 0)
		return (EWOULDBLOCK);
	if (model_random_error != 0)
		return (model_random_error);
	if (uio->uio_resid <= 0)
		return (0);
	n = (unsigned long)uio->uio_resid;
	if (n > model_random_cap)
		n = model_random_cap;
	if (n > (unsigned long)uio->uio_iov[0].iov_len)
		n = (unsigned long)uio->uio_iov[0].iov_len;
	p = (unsigned char *)uio->uio_iov[0].iov_base;
	for (i = 0; i < n; i++)
		p[i] = (unsigned char)(model_random_seed + i * 7u + 0xa5u);
	uio->uio_iov[0].iov_base = p + n;
	uio->uio_iov[0].iov_len -= (size_t)n;
	uio->uio_resid -= (long)n;
	uio->uio_offset += (long long)n;
	return (0);
}

/* mutex(9) */
void
model_mtx_init(struct mtx *m, const char *name, const char *type, int opts)
{

	m->mtx_name_hash = model_strhash(name);
	m->mtx_type_hash = model_strhash(type);
	m->mtx_flags = opts;
	m->mtx_locked = 0;
	model_ev_log(MOP_MTX_INIT, (unsigned long long)(unsigned int)opts,
	    m->mtx_name_hash, m->mtx_type_hash, 0);
}

void
model_mtx_destroy(struct mtx *m)
{

	model_ev_log(MOP_MTX_DESTROY,
	    (unsigned long long)(unsigned int)m->mtx_flags,
	    (unsigned long long)(long long)m->mtx_locked, 0, 0);
	m->mtx_name_hash = 0;
	m->mtx_type_hash = 0;
	m->mtx_flags = 0;
}

void
model_mtx_lock(struct mtx *m)
{

	m->mtx_locked++;
	model_ev_log(MOP_MTX_LOCK,
	    (unsigned long long)(long long)m->mtx_locked,
	    (unsigned long long)model_tgt_value(),
	    (unsigned long long)model_tgt_waiters(), 0);
}

void
model_mtx_unlock(struct mtx *m)
{

	m->mtx_locked--;
	model_ev_log(MOP_MTX_UNLOCK,
	    (unsigned long long)(long long)m->mtx_locked,
	    (unsigned long long)model_tgt_value(),
	    (unsigned long long)model_tgt_waiters(), 0);
}

/* condvar(9) */
void
model_cv_init(struct cv *cv, const char *desc)
{

	cv->cv_description = desc;
	cv->cv_waits = 0;
	cv->cv_signals = 0;
	model_ev_log(MOP_CV_INIT, model_strhash(desc), 0, 0, 0);
}

void
model_cv_destroy(struct cv *cv)
{

	model_ev_log(MOP_CV_DESTROY, model_strhash(cv->cv_description),
	    (unsigned long long)(long long)cv->cv_waits,
	    (unsigned long long)(long long)cv->cv_signals, 0);
	cv->cv_description = NULL;
}

const char *
model_cv_wmesg(struct cv *cv)
{

	model_ev_log(MOP_CV_WMESG, model_strhash(cv->cv_description), 0, 0, 0);
	return (cv->cv_description);
}

void
model_cv_signal(struct cv *cv)
{

	cv->cv_signals++;
	model_ev_log(MOP_CV_SIGNAL,
	    (unsigned long long)(long long)cv->cv_signals,
	    (unsigned long long)model_tgt_value(),
	    (unsigned long long)model_tgt_waiters(), 0);
}

/*
 * cv_wait(9): stands in for another thread posting the semaphore.  The mutex is
 * dropped and reacquired, and after model_cv_release_after waits the target
 * semaphore gains model_cv_post_amount.  A port whose loop condition has been
 * inverted would spin here forever, so past MODEL_WAIT_RUNAWAY waits the model
 * alternates the value (which terminates a loop testing either sense) and
 * records that it had to, which the harness reports as a divergence.
 */
void
model_cv_wait(struct cv *cv, struct mtx *m)
{

	cv->cv_waits++;
	model_total_waits++;
	m->mtx_locked--;
	model_ev_log(MOP_CV_WAIT, (unsigned long long)(long long)cv->cv_waits,
	    (unsigned long long)model_tgt_value(),
	    (unsigned long long)model_tgt_waiters(),
	    (unsigned long long)(long long)m->mtx_locked);
	if (model_total_waits > MODEL_WAIT_RUNAWAY) {
		model_runaway = 1;
		if (model_sema_target != NULL)
			model_sema_target->sema_value = model_total_waits & 1;
	} else if (cv->cv_waits >= model_cv_release_after &&
	    model_sema_target != NULL)
		model_sema_target->sema_value += model_cv_post_amount;
	m->mtx_locked++;
}

int
model_cv_timedwait(struct cv *cv, struct mtx *m, int timo)
{

	cv->cv_waits++;
	model_total_waits++;
	m->mtx_locked--;
	model_ev_log(MOP_CV_TIMEDWAIT,
	    (unsigned long long)(long long)cv->cv_waits,
	    (unsigned long long)model_tgt_value(),
	    (unsigned long long)model_tgt_waiters(),
	    (unsigned long long)(long long)timo);
	m->mtx_locked++;
	if (model_total_waits > MODEL_WAIT_RUNAWAY) {
		model_runaway = 1;
		if (model_sema_target != NULL)
			model_sema_target->sema_value = model_total_waits & 1;
		return (EWOULDBLOCK);
	}
	if (model_timedwait_post_after > 0 &&
	    cv->cv_waits >= model_timedwait_post_after &&
	    model_sema_target != NULL)
		model_sema_target->sema_value += 1;
	if (model_timedwait_fail_after > 0 &&
	    cv->cv_waits >= model_timedwait_fail_after)
		return (EWOULDBLOCK);
	return (0);
}

/* physmem(9) */
void
model_physmem_hardware_region(uint64_t pa, uint64_t sz)
{

	model_ev_log(MOP_PHYS_HW, (unsigned long long)pa,
	    (unsigned long long)sz, 0, 0);
}

void
model_physmem_exclude_region(uint64_t pa, uint64_t sz, uint32_t flags)
{

	model_ev_log(MOP_PHYS_EXCL, (unsigned long long)pa,
	    (unsigned long long)sz, (unsigned long long)flags, 0);
}

/* Used by the harness' own efi_map callbacks. */
void
model_note(unsigned long long a, unsigned long long b, unsigned long long c,
    unsigned long long d)
{

	model_ev_log(MOP_NOTE, a, b, c, d);
}

/* ======================================================================== */
/* The macro layer binding the sources below to the model above.  These are  */
/* the names the kernel headers would define; no source line changes.        */
/* ======================================================================== */

#define	KASSERT(exp, msg)		model_assert((exp) ? 1 : 0)

#define	CTR3(m, f, a1, a2, a3)		model_ctr(3, 0, 0, (const char *)(a3))
#define	CTR4(m, f, a1, a2, a3, a4)	model_ctr(4, (long long)(a3), 0,	\
					    (const char *)(a4))
#define	CTR5(m, f, a1, a2, a3, a4, a5)	model_ctr(5, (long long)(a5), 0,	\
					    (const char *)(a3))
#define	CTR6(m, f, a1, a2, a3, a4, a5, a6)					\
					model_ctr(6, (long long)(a4),		\
					    (long long)(a6), (const char *)(a3))

#define	bzero(p, n)			model_bzero((p), (unsigned long)(n))
#define	malloc(s, t, f)			model_kmalloc((unsigned long)(s), (t), (f))
#define	free(p, t)			model_kfree((p), (t))
#define	printf(...)			model_printf(__VA_ARGS__)
#define	read_random_uio(u, nb)		model_read_random_uio((u), (nb))
#define	mtx_init(m, n, t, o)		model_mtx_init((m), (n), (t), (o))
#define	mtx_destroy(m)			model_mtx_destroy(m)
#define	mtx_lock(m)			model_mtx_lock(m)
#define	mtx_unlock(m)			model_mtx_unlock(m)
#define	cv_init(c, d)			model_cv_init((c), (d))
#define	cv_destroy(c)			model_cv_destroy(c)
#define	cv_wmesg(c)			model_cv_wmesg(c)
#define	cv_signal(c)			model_cv_signal(c)
#define	cv_wait(c, m)			model_cv_wait((c), (m))
#define	cv_timedwait(c, m, t)		model_cv_timedwait((c), (m), (t))
#define	physmem_hardware_region(a, s)	model_physmem_hardware_region((a), (s))
#define	physmem_exclude_region(a, s, f)	model_physmem_exclude_region((a), (s), (f))

/* ======================================================================== */
/* hbsd/src/sys/kern/sys_getrandom.c                                        */
/* ======================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Conrad Meyer <cem@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define GRND_VALIDFLAGS	(GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE)

/*
 * read_random_uio(9) returns EWOULDBLOCK if a nonblocking request would block,
 * but the Linux API name is EAGAIN.  On FreeBSD, they have the same numeric
 * value for now.
 */
CTASSERT(EWOULDBLOCK == EAGAIN);

int
ref_kern_getrandom(struct thread *td, void *user_buf, size_t buflen,
    unsigned int flags)
{
	struct uio auio;
	struct iovec aiov;
	int error;

	if ((flags & ~GRND_VALIDFLAGS) != 0)
		return (EINVAL);
	if (buflen > IOSIZE_MAX)
		return (EINVAL);

	/*
	 * Linux compatibility: We have two choices for handling Linux's
	 * GRND_INSECURE.
	 *
	 * 1. We could ignore it completely (like GRND_RANDOM).  However, this
	 * might produce the surprising result of GRND_INSECURE requests
	 * blocking, when the Linux API does not block.
	 *
	 * 2. Alternatively, we could treat GRND_INSECURE requests as requests
	 * for GRND_NONBLOCK.  Here, the surprising result for Linux programs
	 * is that invocations with unseeded random(4) will produce EAGAIN,
	 * rather than garbage.
	 *
	 * Honoring the flag in the way Linux does seems fraught.  If we
	 * actually use the output of a random(4) implementation prior to
	 * seeding, we leak some entropy about the initial seed to attackers.
	 * This seems unacceptable -- it defeats the purpose of blocking on
	 * initial seeding.
	 *
	 * Secondary to that concern, before seeding we may have arbitrarily
	 * little entropy collected; producing output from zero or a handful of
	 * entropy bits does not seem particularly useful to userspace.
	 *
	 * If userspace can accept garbage, insecure non-random bytes, they can
	 * create their own insecure garbage with srandom(time(NULL)) or
	 * similar.  Asking the kernel to produce it from the secure
	 * getrandom(2) API seems inane.
	 *
	 * We elect to emulate GRND_INSECURE as an alternative spelling of
	 * GRND_NONBLOCK (2).
	 */
	if ((flags & GRND_INSECURE) != 0)
		flags |= GRND_NONBLOCK;

	if (buflen == 0) {
		td->td_retval[0] = 0;
		return (0);
	}

	aiov.iov_base = user_buf;
	aiov.iov_len = buflen;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = 0;
	auio.uio_resid = buflen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_td = td;

	error = read_random_uio(&auio, (flags & GRND_NONBLOCK) != 0);
	if (error == 0)
		td->td_retval[0] = buflen - auio.uio_resid;
	return (error);
}

#ifndef _SYS_SYSPROTO_H_
struct getrandom_args {
	void		*buf;
	size_t		buflen;
	unsigned int	flags;
};
#endif

int
ref_sys_getrandom(struct thread *td, struct getrandom_args *uap)
{
	return (ref_kern_getrandom(td, uap->buf, uap->buflen, uap->flags));
}

/* ======================================================================== */
/* hbsd/src/sys/kern/subr_efi_map.c                                         */
/* ======================================================================== */

/*
 * Copyright (c) 2014 The FreeBSD Foundation
 * Copyright (c) 2018 Andrew Turner
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

void
ref_efi_map_foreach_entry(struct efi_map_header *efihdr, efi_map_entry_cb cb, void *argp)
{
	struct efi_md *map, *p;
	size_t efisz;
	int ndesc, i;

	/*
	 * Memory map data provided by UEFI via the GetMemoryMap
	 * Boot Services API.
	 */
	efisz = (sizeof(struct efi_map_header) + 0xf) & ~0xf;
	map = (struct efi_md *)((uint8_t *)efihdr + efisz);

	if (efihdr->descriptor_size == 0)
		return;
	ndesc = efihdr->memory_size / efihdr->descriptor_size;

	for (i = 0, p = map; i < ndesc; i++,
	    p = efi_next_descriptor(p, efihdr->descriptor_size)) {
		cb(p, argp);
	}
}

/*
 * Handle the EFI memory map list.
 *
 * We will make two passes at this, the first (exclude == false) to populate
 * physmem with valid physical memory ranges from recognized map entry types.
 * In the second pass we will exclude memory ranges from physmem which must not
 * be used for general allocations, either because they are used by runtime
 * firmware or otherwise reserved.
 *
 * Adding the runtime-reserved memory ranges to physmem and excluding them
 * later ensures that they are included in the DMAP, but excluded from
 * phys_avail[].
 *
 * Entry types not explicitly listed here are ignored and not mapped.
 */
void
ref_handle_efi_map_entry(struct efi_md *p, void *argp)
{
	bool exclude = *(bool *)argp;

	switch (p->md_type) {
	case EFI_MD_TYPE_RECLAIM:
		/*
		 * The recomended location for ACPI tables. Map into the
		 * DMAP so we can access them from userspace via /dev/mem.
		 */
	case EFI_MD_TYPE_RT_CODE:
		/*
		 * Some UEFI implementations put the system table in the
		 * runtime code section. Include it in the DMAP, but will
		 * be excluded from phys_avail.
		 */
	case EFI_MD_TYPE_RT_DATA:
		/*
		 * Runtime data will be excluded after the DMAP
		 * region is created to stop it from being added
		 * to phys_avail.
		 */
		if (exclude) {
			physmem_exclude_region(p->md_phys,
			    p->md_pages * EFI_PAGE_SIZE, EXFLAG_NOALLOC);
			break;
		}
		/* FALLTHROUGH */
	case EFI_MD_TYPE_CODE:
	case EFI_MD_TYPE_DATA:
	case EFI_MD_TYPE_BS_CODE:
	case EFI_MD_TYPE_BS_DATA:
	case EFI_MD_TYPE_FREE:
		/*
		 * We're allowed to use any entry with these types.
		 */
		if (!exclude)
			physmem_hardware_region(p->md_phys,
			    p->md_pages * EFI_PAGE_SIZE);
		break;
	default:
		/* Other types shall not be handled by physmem. */
		break;
	}
}

void
ref_efi_map_add_entries(struct efi_map_header *efihdr)
{
	bool exclude = false;
	ref_efi_map_foreach_entry(efihdr, ref_handle_efi_map_entry, &exclude);
}

void
ref_efi_map_exclude_entries(struct efi_map_header *efihdr)
{
	bool exclude = true;
	ref_efi_map_foreach_entry(efihdr, ref_handle_efi_map_entry, &exclude);
}

void
ref_print_efi_map_entry(struct efi_md *p, void *argp __unused)
{
	const char *type;
	static const char *types[] = {
		"Reserved",
		"LoaderCode",
		"LoaderData",
		"BootServicesCode",
		"BootServicesData",
		"RuntimeServicesCode",
		"RuntimeServicesData",
		"ConventionalMemory",
		"UnusableMemory",
		"ACPIReclaimMemory",
		"ACPIMemoryNVS",
		"MemoryMappedIO",
		"MemoryMappedIOPortSpace",
		"PalCode",
		"PersistentMemory"
	};

	if (p->md_type < nitems(types))
		type = types[p->md_type];
	else
		type = "<INVALID>";
	printf("%23s %012jx %012jx %08jx ", type, (uintmax_t)p->md_phys,
	    (uintmax_t)p->md_virt, (uintmax_t)p->md_pages);
	if (p->md_attr & EFI_MD_ATTR_UC)
		printf("UC ");
	if (p->md_attr & EFI_MD_ATTR_WC)
		printf("WC ");
	if (p->md_attr & EFI_MD_ATTR_WT)
		printf("WT ");
	if (p->md_attr & EFI_MD_ATTR_WB)
		printf("WB ");
	if (p->md_attr & EFI_MD_ATTR_UCE)
		printf("UCE ");
	if (p->md_attr & EFI_MD_ATTR_WP)
		printf("WP ");
	if (p->md_attr & EFI_MD_ATTR_RP)
		printf("RP ");
	if (p->md_attr & EFI_MD_ATTR_XP)
		printf("XP ");
	if (p->md_attr & EFI_MD_ATTR_NV)
		printf("NV ");
	if (p->md_attr & EFI_MD_ATTR_MORE_RELIABLE)
		printf("MORE_RELIABLE ");
	if (p->md_attr & EFI_MD_ATTR_RO)
		printf("RO ");
	if (p->md_attr & EFI_MD_ATTR_RT)
		printf("RUNTIME");
	printf("\n");
}

void
ref_efi_map_print_entries(struct efi_map_header *efihdr)
{

	printf("%23s %12s %12s %8s %4s\n",
	    "Type", "Physical", "Virtual", "#Pages", "Attr");
	ref_efi_map_foreach_entry(efihdr, ref_print_efi_map_entry, NULL);
}

/* ======================================================================== */
/* hbsd/src/sys/kern/subr_hash.c                                            */
/* ======================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1982, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static __inline int
ref_hash_mflags(int flags)
{

	return ((flags & HASH_NOWAIT) ? M_NOWAIT : M_WAITOK);
}

/*
 * General routine to allocate a hash table with control of memory flags.
 */
void *
ref_hashinit_flags(int elements, struct malloc_type *type, u_long *hashmask,
    int flags)
{
	long hashsize, i;
	LIST_HEAD(generic, generic) *hashtbl;

	KASSERT(elements > 0, ("%s: bad elements", __func__));
	/* Exactly one of HASH_WAITOK and HASH_NOWAIT must be set. */
	KASSERT((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT),
	    ("Bad flags (0x%x) passed to hashinit_flags", flags));

	for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
		continue;
	hashsize >>= 1;

	hashtbl = malloc((u_long)hashsize * sizeof(*hashtbl), type,
	    ref_hash_mflags(flags));
	if (hashtbl != NULL) {
		for (i = 0; i < hashsize; i++)
			LIST_INIT(&hashtbl[i]);
		*hashmask = hashsize - 1;
	}
	return (hashtbl);
}

/*
 * Allocate and initialize a hash table with default flag: may sleep.
 */
void *
ref_hashinit(int elements, struct malloc_type *type, u_long *hashmask)
{

	return (ref_hashinit_flags(elements, type, hashmask, HASH_WAITOK));
}

void
ref_hashdestroy(void *vhashtbl, struct malloc_type *type, u_long hashmask)
{
	LIST_HEAD(generic, generic) *hashtbl, *hp;

	hashtbl = vhashtbl;
	for (hp = hashtbl; hp <= &hashtbl[hashmask]; hp++)
		KASSERT(LIST_EMPTY(hp), ("%s: hashtbl %p not empty "
		    "(malloc type %s)", __func__, hashtbl, type->ks_shortdesc));
	free(hashtbl, type);
}

static const int primes[] = { 1, 13, 31, 61, 127, 251, 509, 761, 1021, 1531,
			2039, 2557, 3067, 3583, 4093, 4603, 5119, 5623, 6143,
			6653, 7159, 7673, 8191, 12281, 16381, 24571, 32749 };
#define	NPRIMES nitems(primes)

/*
 * General routine to allocate a prime number sized hash table with control of
 * memory flags.
 */
void *
ref_phashinit_flags(int elements, struct malloc_type *type, u_long *nentries, int flags)
{
	long hashsize, i;
	LIST_HEAD(generic, generic) *hashtbl;

	KASSERT(elements > 0, ("%s: bad elements", __func__));
	/* Exactly one of HASH_WAITOK and HASH_NOWAIT must be set. */
	KASSERT((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT),
	    ("Bad flags (0x%x) passed to phashinit_flags", flags));

	for (i = 1, hashsize = primes[1]; hashsize <= elements;) {
		i++;
		if (i == NPRIMES)
			break;
		hashsize = primes[i];
	}
	hashsize = primes[i - 1];

	hashtbl = malloc((u_long)hashsize * sizeof(*hashtbl), type,
	    ref_hash_mflags(flags));
	if (hashtbl == NULL)
		return (NULL);

	for (i = 0; i < hashsize; i++)
		LIST_INIT(&hashtbl[i]);
	*nentries = hashsize;
	return (hashtbl);
}

/*
 * Allocate and initialize a prime number sized hash table with default flag:
 * may sleep.
 */
void *
ref_phashinit(int elements, struct malloc_type *type, u_long *nentries)
{

	return (ref_phashinit_flags(elements, type, nentries, HASH_WAITOK));
}

/* ======================================================================== */
/* hbsd/src/sys/kern/kern_sema.c                                            */
/* ======================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2001 Jason Evans <jasone@freebsd.org>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible 
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Counting semaphores.
 *
 * Priority propagation will not generally raise the priority of semaphore
 * "owners" (a misnomer in the context of semaphores), so should not be relied
 * upon in combination with semaphores.
 */

void
ref_sema_init(struct sema *sema, int value, const char *description)
{

	KASSERT((value >= 0), ("%s(): negative value\n", __func__));

	bzero(sema, sizeof(*sema));
	mtx_init(&sema->sema_mtx, description, "sema backing lock",
	    MTX_DEF | MTX_NOWITNESS | MTX_QUIET);
	cv_init(&sema->sema_cv, description);
	sema->sema_value = value;

	CTR4(KTR_LOCK, "%s(%p, %d, \"%s\")", __func__, sema, value, description);
}

void
ref_sema_destroy(struct sema *sema)
{

	CTR3(KTR_LOCK, "%s(%p) \"%s\"", __func__, sema,
	    cv_wmesg(&sema->sema_cv));

	KASSERT((sema->sema_waiters == 0), ("%s(): waiters\n", __func__));

	mtx_destroy(&sema->sema_mtx);
	cv_destroy(&sema->sema_cv);
}

void
ref__sema_post(struct sema *sema, const char *file, int line)
{

	mtx_lock(&sema->sema_mtx);
	sema->sema_value++;
	if (sema->sema_waiters && sema->sema_value > 0)
		cv_signal(&sema->sema_cv);

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	mtx_unlock(&sema->sema_mtx);
}

void
ref__sema_wait(struct sema *sema, const char *file, int line)
{

	mtx_lock(&sema->sema_mtx);
	while (sema->sema_value == 0) {
		sema->sema_waiters++;
		cv_wait(&sema->sema_cv, &sema->sema_mtx);
		sema->sema_waiters--;
	}
	sema->sema_value--;

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	mtx_unlock(&sema->sema_mtx);
}

int
ref__sema_timedwait(struct sema *sema, int timo, const char *file, int line)
{
	int error;

	mtx_lock(&sema->sema_mtx);

	/*
	 * A spurious wakeup will cause the timeout interval to start over.
	 * This isn't a big deal as long as spurious wakeups don't occur
	 * continuously, since the timeout period is merely a lower bound on how
	 * long to wait.
	 */
	for (error = 0; sema->sema_value == 0 && error == 0;) {
		sema->sema_waiters++;
		error = cv_timedwait(&sema->sema_cv, &sema->sema_mtx, timo);
		sema->sema_waiters--;
	}
	if (sema->sema_value > 0) {
		/* Success. */
		sema->sema_value--;
		error = 0;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), file, line);
	}

	mtx_unlock(&sema->sema_mtx);
	return (error);
}

int
ref__sema_trywait(struct sema *sema, const char *file, int line)
{
	int ret;

	mtx_lock(&sema->sema_mtx);

	if (sema->sema_value > 0) {
		/* Success. */
		sema->sema_value--;
		ret = 1;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		ret = 0;

		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), file, line);
	}

	mtx_unlock(&sema->sema_mtx);
	return (ret);
}

int
ref_sema_value(struct sema *sema)
{
	int ret;

	mtx_lock(&sema->sema_mtx);
	ret = sema->sema_value;
	mtx_unlock(&sema->sema_mtx);
	return (ret);
}
