/*
 * oracle.c -- reference implementation for PBSD batch b0146.
 *
 * The original HardenedBSD kernel sources are concatenated below with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 * Supporting types, macros, and shims are added only where the original files
 * obtained them from kernel headers.
 */

#include <stdbool.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#define __unused __attribute__((__unused__))

#define KASSERT(cond, msg) ((void)0)
#define CTASSERT(x) typedef char __ctassert[(x) ? 1 : -1] __attribute__((__unused__))

#define nitems(x) (sizeof((x)) / sizeof((x)[0]))

typedef unsigned long u_long;

#define M_NOWAIT 0x0001
#define M_WAITOK 0x0002

#define HASH_WAITOK 0x00000001
#define HASH_NOWAIT 0x00000002

#define LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;						\
}

#define LIST_INIT(head) do {						\
	(head)->lh_first = NULL;					\
} while (0)

#define LIST_EMPTY(head) ((head)->lh_first == NULL)

LIST_HEAD(generic, generic);

struct malloc_type {
	const char *ks_shortdesc;
};

#define GRND_NONBLOCK 0x0001
#define GRND_RANDOM   0x0002
#define GRND_INSECURE 0x0004
#define GRND_VALIDFLAGS (GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE)

#define IOSIZE_MAX 0x80000000UL

#define UIO_USERSPACE 1
#define UIO_READ      1

#define EINVAL 22
#define EWOULDBLOCK 35
#define EAGAIN 35

struct thread {
	long td_retval[2];
};

struct iovec {
	void *iov_base;
	size_t iov_len;
};

struct uio {
	struct iovec *uio_iov;
	int uio_iovcnt;
	off_t uio_offset;
	size_t uio_resid;
	int uio_segflg;
	int uio_rw;
	struct thread *uio_td;
};

struct getrandom_args {
	void *buf;
	size_t buflen;
	unsigned int flags;
};

#define EFI_PAGE_SIZE 4096
#define EXFLAG_NOALLOC 1

#define EFI_MD_TYPE_RESERVED              0
#define EFI_MD_TYPE_LOADER_CODE           1
#define EFI_MD_TYPE_LOADER_DATA           2
#define EFI_MD_TYPE_BS_CODE               3
#define EFI_MD_TYPE_BS_DATA               4
#define EFI_MD_TYPE_RT_CODE               5
#define EFI_MD_TYPE_RT_DATA               6
#define EFI_MD_TYPE_FREE                  7
#define EFI_MD_TYPE_UNUSABLE              8
#define EFI_MD_TYPE_RECLAIM               9
#define EFI_MD_TYPE_ACPI_NVS              10
#define EFI_MD_TYPE_MMIO                  11
#define EFI_MD_TYPE_MMIO_PORT             12
#define EFI_MD_TYPE_PAL_CODE              13
#define EFI_MD_TYPE_PERSISTENT            14

#define EFI_MD_TYPE_CODE EFI_MD_TYPE_LOADER_CODE
#define EFI_MD_TYPE_DATA EFI_MD_TYPE_LOADER_DATA

#define EFI_MD_ATTR_UC              0x1
#define EFI_MD_ATTR_WC              0x2
#define EFI_MD_ATTR_WT              0x4
#define EFI_MD_ATTR_WB              0x8
#define EFI_MD_ATTR_UCE             0x10
#define EFI_MD_ATTR_WP              0x20
#define EFI_MD_ATTR_RP              0x40
#define EFI_MD_ATTR_XP              0x80
#define EFI_MD_ATTR_NV              0x100
#define EFI_MD_ATTR_MORE_RELIABLE   0x200
#define EFI_MD_ATTR_RO              0x400
#define EFI_MD_ATTR_RT              0x800

struct efi_map_header {
	size_t memory_size;
	size_t descriptor_size;
};

struct efi_md {
	uint32_t md_type;
	uint32_t md_pad;
	uint64_t md_phys;
	uint64_t md_virt;
	uint64_t md_pages;
	uint64_t md_attr;
};

typedef void (*efi_map_entry_cb)(struct efi_md *p, void *argp);

static inline struct efi_md *
efi_next_descriptor(struct efi_md *md, size_t size)
{
	return ((struct efi_md *)((uint8_t *)md + size));
}

#define MTX_DEF 0
#define MTX_NOWITNESS 0
#define MTX_QUIET 0

#define KTR_LOCK 0
#define CTR3(a, b, c, d)
#define CTR4(a, b, c, d, e)
#define CTR5(a, b, c, d, e)
#define CTR6(a, b, c, d, e, f)

struct mtx {
	pthread_mutex_t lock;
	const char *name;
};

struct cv {
	pthread_cond_t cond;
	const char *name;
};

struct sema {
	struct mtx sema_mtx;
	struct cv sema_cv;
	int sema_value;
	int sema_waiters;
};

static int g_malloc_calls;
static int g_malloc_fail_at;

void
oracle_malloc_reset(void)
{
	g_malloc_calls = 0;
	g_malloc_fail_at = 0;
}

void
oracle_malloc_fail_at(int n)
{
	g_malloc_fail_at = n;
}

static void *
kern_malloc(size_t size, struct malloc_type *type, int flags)
{
	void *p;

	(void)type;
	(void)flags;

	g_malloc_calls++;
	if (g_malloc_fail_at != 0 && g_malloc_calls >= g_malloc_fail_at)
		return (NULL);
	p = malloc(size);
	return (p);
}

static void
kern_free(void *addr, struct malloc_type *type)
{
	(void)type;
	free(addr);
}

#define malloc kern_malloc
#define free kern_free

static void
bzero(void *p, size_t n)
{
	memset(p, 0, n);
}

void
mtx_init(struct mtx *m, const char *name, const char *desc, int opts)
{
	(void)desc;
	(void)opts;
	m->name = name;
	pthread_mutex_init(&m->lock, NULL);
}

void
mtx_destroy(struct mtx *m)
{
	pthread_mutex_destroy(&m->lock);
}

void
mtx_lock(struct mtx *m)
{
	pthread_mutex_lock(&m->lock);
}

void
mtx_unlock(struct mtx *m)
{
	pthread_mutex_unlock(&m->lock);
}

void
cv_init(struct cv *c, const char *name)
{
	c->name = name;
	pthread_cond_init(&c->cond, NULL);
}

void
cv_destroy(struct cv *c)
{
	pthread_cond_destroy(&c->cond);
}

void
cv_signal(struct cv *c)
{
	pthread_cond_signal(&c->cond);
}

void
cv_wait(struct cv *c, struct mtx *m)
{
	pthread_cond_wait(&c->cond, &m->lock);
}

int
cv_timedwait(struct cv *c, struct mtx *m, int timo)
{
	struct timespec ts;

	if (timo == 0)
		return (EWOULDBLOCK);
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += timo / 1000;
	ts.tv_nsec += (timo % 1000) * 1000000L;
	if (ts.tv_nsec >= 1000000000L) {
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000L;
	}
	if (pthread_cond_timedwait(&c->cond, &m->lock, &ts) != 0)
		return (EWOULDBLOCK);
	return (0);
}

const char *
cv_wmesg(struct cv *c)
{
	return (c->name);
}

static int g_read_random_error;
static int g_read_random_block;
static ssize_t g_read_random_transfer;

void
oracle_read_random_reset(void)
{
	g_read_random_error = 0;
	g_read_random_block = 0;
	g_read_random_transfer = -1;
}

void
oracle_read_random_configure(int error, int block, ssize_t transfer)
{
	g_read_random_error = error;
	g_read_random_block = block;
	g_read_random_transfer = transfer;
}

static int
read_random_uio(struct uio *auio, int nonblock)
{
	size_t transfer;
	struct iovec *iov;
	unsigned char *bp;
	size_t i;

	if (g_read_random_error != 0)
		return (g_read_random_error);
	if (nonblock && g_read_random_block)
		return (EWOULDBLOCK);
	transfer = auio->uio_resid;
	if (g_read_random_transfer >= 0 &&
	    transfer > (size_t)g_read_random_transfer)
		transfer = (size_t)g_read_random_transfer;
	iov = auio->uio_iov;
	bp = (unsigned char *)iov->iov_base;
	for (i = 0; i < transfer; i++)
		bp[i] = (unsigned char)(0xa5 ^ (i & 0xff));
	auio->uio_resid -= transfer;
	return (0);
}

#define PHYSMEM_LOG_MAX 4096

struct physmem_log_entry {
	int exclude;
	uint64_t phys;
	uint64_t size;
	int exflag;
};

static struct physmem_log_entry g_physmem_log[PHYSMEM_LOG_MAX];
static int g_physmem_log_count;

void
oracle_physmem_reset(void)
{
	g_physmem_log_count = 0;
}

int
oracle_physmem_log_count(void)
{
	return (g_physmem_log_count);
}

const struct physmem_log_entry *
oracle_physmem_log_entry(int i)
{
	return (&g_physmem_log[i]);
}

static void
physmem_hardware_region(uint64_t phys, uint64_t size)
{
	if (g_physmem_log_count < PHYSMEM_LOG_MAX) {
		g_physmem_log[g_physmem_log_count].exclude = 0;
		g_physmem_log[g_physmem_log_count].phys = phys;
		g_physmem_log[g_physmem_log_count].size = size;
		g_physmem_log[g_physmem_log_count].exflag = 0;
		g_physmem_log_count++;
	}
}

static void
physmem_exclude_region(uint64_t phys, uint64_t size, int exflag)
{
	if (g_physmem_log_count < PHYSMEM_LOG_MAX) {
		g_physmem_log[g_physmem_log_count].exclude = 1;
		g_physmem_log[g_physmem_log_count].phys = phys;
		g_physmem_log[g_physmem_log_count].size = size;
		g_physmem_log[g_physmem_log_count].exflag = exflag;
		g_physmem_log_count++;
	}
}

/* --- sys_getrandom.c --- */

#define GRND_VALIDFLAGS	(GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE)

CTASSERT(EWOULDBLOCK == EAGAIN);

static int
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

/* --- subr_efi_map.c --- */

void
ref_efi_map_foreach_entry(struct efi_map_header *efihdr, efi_map_entry_cb cb, void *argp)
{
	struct efi_md *map, *p;
	size_t efisz;
	int ndesc, i;

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

static void
ref_handle_efi_map_entry(struct efi_md *p, void *argp)
{
	bool exclude = *(bool *)argp;

	switch (p->md_type) {
	case EFI_MD_TYPE_RECLAIM:
	case EFI_MD_TYPE_RT_CODE:
	case EFI_MD_TYPE_RT_DATA:
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
		if (!exclude)
			physmem_hardware_region(p->md_phys,
			    p->md_pages * EFI_PAGE_SIZE);
		break;
	default:
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

static void
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

/* --- subr_hash.c --- */

static __inline int
ref_hash_mflags(int flags)
{

	return ((flags & HASH_NOWAIT) ? M_NOWAIT : M_WAITOK);
}

void *
ref_hashinit_flags(int elements, struct malloc_type *type, u_long *hashmask,
    int flags)
{
	long hashsize, i;
	LIST_HEAD(generic, generic) *hashtbl;

	KASSERT(elements > 0, ("%s: bad elements", __func__));
	KASSERT((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT),
	    ("Bad flags (0x%x) passed to hashinit_flags", flags));

	for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
		continue;
	hashsize >>= 1;

	hashtbl = kern_malloc((u_long)hashsize * sizeof(*hashtbl), type,
	    ref_hash_mflags(flags));
	if (hashtbl != NULL) {
		for (i = 0; i < hashsize; i++)
			LIST_INIT(&hashtbl[i]);
		*hashmask = hashsize - 1;
	}
	return (hashtbl);
}

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

void *
ref_phashinit_flags(int elements, struct malloc_type *type, u_long *nentries, int flags)
{
	long hashsize, i;
	LIST_HEAD(generic, generic) *hashtbl;

	KASSERT(elements > 0, ("%s: bad elements", __func__));
	KASSERT((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT),
	    ("Bad flags (0x%x) passed to phashinit_flags", flags));

	for (i = 1, hashsize = primes[1]; hashsize <= elements;) {
		i++;
		if (i == NPRIMES)
			break;
		hashsize = primes[i];
	}
	hashsize = primes[i - 1];

	hashtbl = kern_malloc((u_long)hashsize * sizeof(*hashtbl), type,
	    ref_hash_mflags(flags));
	if (hashtbl == NULL)
		return (NULL);

	for (i = 0; i < hashsize; i++)
		LIST_INIT(&hashtbl[i]);
	*nentries = hashsize;
	return (hashtbl);
}

void *
ref_phashinit(int elements, struct malloc_type *type, u_long *nentries)
{

	return (ref_phashinit_flags(elements, type, nentries, HASH_WAITOK));
}

/* --- kern_sema.c --- */

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

	for (error = 0; sema->sema_value == 0 && error == 0;) {
		sema->sema_waiters++;
		error = cv_timedwait(&sema->sema_cv, &sema->sema_mtx, timo);
		sema->sema_waiters--;
	}
	if (sema->sema_value > 0) {
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
