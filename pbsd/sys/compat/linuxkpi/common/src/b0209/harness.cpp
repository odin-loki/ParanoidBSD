/*
 * PBSD batch b0209 -- differential test.
 *
 * Every ported function is driven against the ref_ oracle in oracle.c with
 * hand written edge cases and a fixed seed randomised sweep.  Both sides run
 * against the same modelled kernel environment; every call the bodies make
 * into that environment is recorded in a per-side log, so argument values,
 * call order and short circuit behaviour are compared, not just the return
 * value.
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

/* ------------------------------------------------------------------ */
/* Modelled kernel environment; must match oracle.c and port.cppm.     */
/* ------------------------------------------------------------------ */

extern "C" {

#define	MAXMEMDOM	8
struct domainset {
	int	ds_id;
};

struct pbsd_bsddev;
typedef struct pbsd_bsddev *device_t;
struct device {
	device_t	bsddev;
};

typedef unsigned int gfp_t;
struct page;
struct folio;

#define	PAGEVEC_SIZE	15
struct folio_batch {
	uint8_t		nr;
	struct folio	*folios[PAGEVEC_SIZE];
};

struct thread;
struct file;
struct eventfd_ctx;
struct cap_rights {
	int	cr_dummy;
};

/* The oracle. */
struct domainset *ref_linux_get_vm_domain_set(int node);
int ref_linux_dev_to_node(struct device *dev);
struct folio *ref_folio_alloc(gfp_t gfp, unsigned int order);
void ref___folio_batch_release(struct folio_batch *fbatch);
struct eventfd_ctx *ref_lkpi_eventfd_ctx_fdget(int fd);
void ref_lkpi_eventfd_ctx_put(struct eventfd_ctx *ctx);
const char *ref_video_get_options(const char *connector_name);

}

import pbsd.sys.compat.linuxkpi.common.src.b0209;

namespace P = pbsd::sys_compat_linuxkpi_common_src::b0209;

#define	GFP_COMP_BIT	0x4000u		/* __GFP_COMP, as modelled */

/* ------------------------------------------------------------------ */
/* Fake objects.  None of them is ever dereferenced by the model.      */
/* ------------------------------------------------------------------ */

static unsigned char page_backing[8][32];
static unsigned char folio_backing[8][32];
static unsigned char file_backing[4][32];
static unsigned char ctx_backing[4][32];
static unsigned char bsddev_backing[4][32];
static unsigned char thread_backing[32];

static struct page *page_pool[8];
static struct folio *folio_pool[8];
static struct file *file_pool[4];
static struct eventfd_ctx *ctx_pool[4];
static device_t bsddev_pool[4];

struct Reg {
	const void	*p;
	long		 id;
};
static Reg g_reg[64];
static int g_nreg;

static void
reg_add(const void *p, long id)
{
	g_reg[g_nreg].p = p;
	g_reg[g_nreg].id = id;
	g_nreg++;
}

static long
ptr_id(const void *p)
{
	if (p == NULL)
		return (0);
	for (int i = 0; i < g_nreg; i++)
		if (g_reg[i].p == p)
			return (g_reg[i].id);
	return ((long)(intptr_t)p);
}

/* ------------------------------------------------------------------ */
/* Call log.                                                           */
/* ------------------------------------------------------------------ */

enum EvKind {
	EV_NONE = 0,
	EV_PRINTF,
	EV_GETENV,
	EV_BUSDOM,
	EV_ALLOC,
	EV_RELEASE,
	EV_FGET,
	EV_EFD_GET,
	EV_EFD_PUT,
	EV_FDROP
};

struct Ev {
	int	kind;
	long	a, b, c;
	char	s[1024];
};

struct Log {
	int	n;
	int	overflow;
	Ev	ev[8];
};

static Log g_log_port, g_log_ref;
static Log *g_cur = &g_log_port;

static void
cap_begin(Log *l)
{
	l->n = 0;
	l->overflow = 0;
	g_cur = l;
}

static Ev *
log_push(int kind)
{
	static Ev sink;
	Ev *e;

	if (g_cur->n >= (int)(sizeof(g_cur->ev) / sizeof(g_cur->ev[0]))) {
		g_cur->overflow++;
		e = &sink;
	} else {
		e = &g_cur->ev[g_cur->n++];
	}
	e->kind = kind;
	e->a = e->b = e->c = 0;
	e->s[0] = '\0';
	return (e);
}

static const char *
ev_name(int kind)
{
	switch (kind) {
	case EV_PRINTF:		return ("printf");
	case EV_GETENV:		return ("kern_getenv");
	case EV_BUSDOM:		return ("bus_get_domain");
	case EV_ALLOC:		return ("alloc_pages");
	case EV_RELEASE:	return ("release_pages");
	case EV_FGET:		return ("fget_unlocked");
	case EV_EFD_GET:	return ("eventfd_get");
	case EV_EFD_PUT:	return ("eventfd_put");
	case EV_FDROP:		return ("fdrop");
	default:		return ("?");
	}
}

static bool
logs_equal(const Log *a, const Log *b)
{
	if (a->n != b->n || a->overflow != b->overflow)
		return (false);
	for (int i = 0; i < a->n; i++) {
		const Ev *x = &a->ev[i], *y = &b->ev[i];

		if (x->kind != y->kind || x->a != y->a || x->b != y->b ||
		    x->c != y->c)
			return (false);
		if (strcmp(x->s, y->s) != 0)
			return (false);
	}
	return (true);
}

static void
log_dump(const char *tag, const Log *l)
{
	printf("      %s: %d call(s)%s\n", tag, l->n,
	    l->overflow ? " (OVERFLOW)" : "");
	for (int i = 0; i < l->n; i++) {
		const Ev *e = &l->ev[i];

		printf("        %d: %-14s a=%ld b=%ld c=%ld s=\"%s\"\n", i,
		    ev_name(e->kind), e->a, e->b, e->c, e->s);
	}
}

/* ------------------------------------------------------------------ */
/* Scripted environment behaviour.                                     */
/* ------------------------------------------------------------------ */

extern "C" {

struct domainset domainset_roundrobin = { 0x5252 };
struct domainset domainset_prefer[MAXMEMDOM];
struct cap_rights cap_no_rights = { 0 };
struct thread *lkpi_curthread = (struct thread *)thread_backing;
int bootverbose = 0;

}

static int g_bus_ret;
static int g_bus_domain_out;
static struct page *g_alloc_result;
static const void *g_fbatch_base;
static int g_fget_ret;
static struct file *g_fget_fp;
static struct eventfd_ctx *g_efd_get_result;

static int g_env_specific_present;
static char g_env_specific_key[64];
static int g_env_default_present;
static char g_env_specific_val[] = "1024x768";
static char g_env_default_val[] = "640x480";

extern "C" int
lkpi_printf(const char *fmt, ...)
{
	Ev *e = log_push(EV_PRINTF);
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vsnprintf(e->s, sizeof(e->s), fmt, ap);
	va_end(ap);
	e->a = r;
	return (r);
}

extern "C" char *
kern_getenv(const char *name)
{
	Ev *e = log_push(EV_GETENV);
	size_t n = strlen(name);

	e->a = (long)n;
	if (n >= sizeof(e->s))
		n = sizeof(e->s) - 1;
	memcpy(e->s, name, n);
	e->s[n] = '\0';

	if (g_env_specific_present && strcmp(name, g_env_specific_key) == 0) {
		e->b = 1;
		return (g_env_specific_val);
	}
	if (g_env_default_present &&
	    strcmp(name, "kern.vt.fb.default_mode") == 0) {
		e->b = 2;
		return (g_env_default_val);
	}
	e->b = 0;
	return (NULL);
}

extern "C" int
bus_get_domain(device_t dev, int *domain)
{
	Ev *e = log_push(EV_BUSDOM);

	e->a = ptr_id(dev);
	e->b = g_bus_domain_out;
	e->c = g_bus_ret;
	*domain = g_bus_domain_out;
	return (g_bus_ret);
}

extern "C" struct page *
alloc_pages(gfp_t gfp, unsigned int order)
{
	Ev *e = log_push(EV_ALLOC);

	e->a = (long)(unsigned long)gfp;
	e->b = (long)(unsigned long)order;
	return (g_alloc_result);
}

extern "C" void
release_pages(struct folio **folios, int nr)
{
	Ev *e = log_push(EV_RELEASE);

	e->a = (long)((const char *)folios - (const char *)g_fbatch_base);
	e->b = nr;
}

extern "C" int
fget_unlocked(struct thread *td, int fd, const struct cap_rights *rights,
    struct file **fpp)
{
	Ev *e = log_push(EV_FGET);

	e->a = fd;
	e->b = (td == lkpi_curthread) ? 1 : ptr_id(td);
	e->c = (rights == &cap_no_rights) ? 1 : ptr_id(rights);
	*fpp = g_fget_fp;
	return (g_fget_ret);
}

extern "C" void
fdrop(struct file *fp, struct thread *td)
{
	Ev *e = log_push(EV_FDROP);

	e->a = ptr_id(fp);
	e->b = (td == lkpi_curthread) ? 1 : ptr_id(td);
}

extern "C" struct eventfd_ctx *
eventfd_get(struct file *fp)
{
	Ev *e = log_push(EV_EFD_GET);

	e->a = ptr_id(fp);
	return (g_efd_get_result);
}

extern "C" void
eventfd_put(struct eventfd_ctx *ctx)
{
	Ev *e = log_push(EV_EFD_PUT);

	e->a = ptr_id(ctx);
}

/* ------------------------------------------------------------------ */
/* Bookkeeping.                                                        */
/* ------------------------------------------------------------------ */

struct Stats {
	const char	*name;
	long		 cases;
	long		 fails;
};

static Stats st_domain_set = { "linux_get_vm_domain_set", 0, 0 };
static Stats st_dev_to_node = { "linux_dev_to_node", 0, 0 };
static Stats st_folio_alloc = { "folio_alloc", 0, 0 };
static Stats st_batch_rel = { "__folio_batch_release", 0, 0 };
static Stats st_efd_fdget = { "lkpi_eventfd_ctx_fdget", 0, 0 };
static Stats st_efd_put = { "lkpi_eventfd_ctx_put", 0, 0 };
static Stats st_video = { "video_get_options", 0, 0 };

#define	MAX_REPORTED	4

static bool
record(Stats *st, bool ok, const char *fmt, ...)
{
	va_list ap;

	st->cases++;
	if (ok)
		return (true);
	st->fails++;
	if (st->fails <= MAX_REPORTED) {
		printf("  [FAIL] %s: ", st->name);
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
		printf("\n");
		log_dump("port", &g_log_port);
		log_dump("ref ", &g_log_ref);
	}
	return (false);
}

/* Printable rendering of a byte string, for failure reports. */
static void
show(char *out, size_t outsz, const unsigned char *s, size_t n)
{
	size_t o = 0;

	for (size_t i = 0; i < n && o + 5 < outsz; i++) {
		unsigned char c = s[i];

		if (c >= 0x20 && c < 0x7f)
			out[o++] = (char)c;
		else
			o += (size_t)snprintf(out + o, outsz - o, "\\x%02x", c);
	}
	out[o < outsz ? o : outsz - 1] = '\0';
}

/* ------------------------------------------------------------------ */
/* Fixed seed PRNG.                                                    */
/* ------------------------------------------------------------------ */

static uint64_t g_rs;

static void
rnd_seed(uint64_t s)
{
	g_rs = s;
}

static uint64_t
rnd64(void)
{
	g_rs ^= g_rs << 13;
	g_rs ^= g_rs >> 7;
	g_rs ^= g_rs << 17;
	return (g_rs);
}

static uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

#define	SEED	0x0209DEADBEEF0209ULL
#define	ITERS	200000

/* ------------------------------------------------------------------ */
/* linux_get_vm_domain_set                                             */
/* ------------------------------------------------------------------ */

static long
ds_id(const struct domainset *p)
{
	if (p == &domainset_roundrobin)
		return (-1000000);
	return ((long)((const char *)p - (const char *)&domainset_prefer[0]) /
	    (long)sizeof(struct domainset));
}

static void
case_domain_set(int node)
{
	cap_begin(&g_log_port);
	struct domainset *a = P::linux_get_vm_domain_set(node);
	cap_begin(&g_log_ref);
	struct domainset *b = ref_linux_get_vm_domain_set(node);

	record(&st_domain_set, a == b && logs_equal(&g_log_port, &g_log_ref),
	    "node=%d port=%ld ref=%ld", node, ds_id(a), ds_id(b));
}

static void
test_domain_set(void)
{
	static const int edges[] = {
		INT_MIN, INT_MIN + 1, -1000000, -65536, -3, -2, -1,
		0, 1, 2, 3, 4, 5, 6, MAXMEMDOM - 1
	};

	for (size_t i = 0; i < sizeof(edges) / sizeof(edges[0]); i++)
		case_domain_set(edges[i]);

	rnd_seed(SEED);
	for (long i = 0; i < ITERS; i++) {
		uint32_t r = rnd32();
		int node;

		switch (r & 3) {
		case 0:
			/* Dense around the sign boundary. */
			node = (int)((r >> 8) & 15) - 8;
			break;
		case 1:
			node = (int)((r >> 8) & 7);
			break;
		case 2:
			node = INT_MIN + (int)((r >> 8) & 0xffff);
			break;
		default:
			node = -(int)(rnd32() & 0x7fffffff);
			break;
		}
		if (node >= MAXMEMDOM)
			node = MAXMEMDOM - 1;	/* KASSERT precondition */
		case_domain_set(node);
	}
}

/* ------------------------------------------------------------------ */
/* linux_dev_to_node                                                   */
/* ------------------------------------------------------------------ */

static void
case_dev_to_node(int dev_null, device_t bsddev, int bus_ret, int bus_domain)
{
	enum { BUFSZ = 64 };
	unsigned char ba[BUFSZ], bb[BUFSZ], pristine[BUFSZ];

	memset(ba, 0x7f, BUFSZ);
	memset(bb, 0x7f, BUFSZ);

	struct device *da = (struct device *)(ba + 16);
	struct device *db = (struct device *)(bb + 16);

	da->bsddev = bsddev;
	db->bsddev = bsddev;
	memcpy(pristine, ba, BUFSZ);

	g_bus_ret = bus_ret;
	g_bus_domain_out = bus_domain;

	cap_begin(&g_log_port);
	int a = P::linux_dev_to_node(dev_null ? NULL : da);
	cap_begin(&g_log_ref);
	int b = ref_linux_dev_to_node(dev_null ? NULL : db);

	bool ok = a == b && logs_equal(&g_log_port, &g_log_ref) &&
	    memcmp(ba, bb, BUFSZ) == 0 && memcmp(ba, pristine, BUFSZ) == 0 &&
	    memcmp(bb, pristine, BUFSZ) == 0;

	record(&st_dev_to_node, ok,
	    "dev=%s bsddev=%ld bus_ret=%d bus_domain=%d port=%d ref=%d",
	    dev_null ? "NULL" : "obj", ptr_id(bsddev), bus_ret, bus_domain,
	    a, b);
}

static void
test_dev_to_node(void)
{
	static const int rets[] = { 0, 1, -1, 6, INT_MIN, INT_MAX };
	static const int doms[] = { 0, 1, -1, -2, 7, 42, INT_MIN, INT_MAX };

	case_dev_to_node(1, NULL, 0, 5);
	case_dev_to_node(1, bsddev_pool[0], 0, 5);
	case_dev_to_node(0, NULL, 0, 5);
	case_dev_to_node(0, NULL, 3, 5);
	for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
		for (size_t d = 0; d < sizeof(doms) / sizeof(doms[0]); d++)
			for (int p = 0; p < 4; p++)
				case_dev_to_node(0, bsddev_pool[p], rets[r],
				    doms[d]);

	rnd_seed(SEED ^ 0x11);
	for (long i = 0; i < ITERS; i++) {
		uint32_t r = rnd32();
		int dev_null = (r & 7) == 0;
		device_t bsddev = ((r >> 3) & 7) == 0 ? NULL :
		    bsddev_pool[(r >> 6) & 3];
		int bus_ret = ((r >> 8) & 1) ? 0 : (int)rnd32();
		int bus_domain = ((r >> 9) & 1) ? (int)(rnd32() & 15) - 4 :
		    (int)rnd32();

		case_dev_to_node(dev_null, bsddev, bus_ret, bus_domain);
	}
}

/* ------------------------------------------------------------------ */
/* folio_alloc                                                         */
/* ------------------------------------------------------------------ */

static void
case_folio_alloc(gfp_t gfp, unsigned int order, struct page *result)
{
	g_alloc_result = result;

	cap_begin(&g_log_port);
	struct folio *a = P::folio_alloc(gfp, order);
	cap_begin(&g_log_ref);
	struct folio *b = ref_folio_alloc(gfp, order);

	record(&st_folio_alloc, a == b && logs_equal(&g_log_port, &g_log_ref),
	    "gfp=0x%08x order=%u result=%ld port=%ld ref=%ld", gfp, order,
	    ptr_id(result), ptr_id(a), ptr_id(b));
}

static void
test_folio_alloc(void)
{
	static const gfp_t gfps[] = {
		0u, 1u, GFP_COMP_BIT, ~GFP_COMP_BIT, GFP_COMP_BIT - 1u,
		GFP_COMP_BIT + 1u, 0xffffffffu, 0x80000000u, 0x3fffu, 0x8000u,
		0xdeadbeefu
	};
	static const unsigned int orders[] = {
		0u, 1u, 2u, 3u, 9u, 15u, 255u, 65535u, 0xfffffffeu, 0xffffffffu
	};

	for (size_t g = 0; g < sizeof(gfps) / sizeof(gfps[0]); g++)
		for (size_t o = 0; o < sizeof(orders) / sizeof(orders[0]); o++) {
			case_folio_alloc(gfps[g], orders[o], NULL);
			case_folio_alloc(gfps[g], orders[o], page_pool[0]);
			case_folio_alloc(gfps[g], orders[o], page_pool[7]);
		}

	rnd_seed(SEED ^ 0x22);
	for (long i = 0; i < ITERS; i++) {
		uint32_t r = rnd32();
		gfp_t gfp;
		unsigned int order;

		switch (r & 3) {
		case 0:
			gfp = (gfp_t)rnd32();
			break;
		case 1:
			gfp = (gfp_t)(rnd32() & 0xffffu);
			break;
		case 2:
			gfp = (gfp_t)(GFP_COMP_BIT | (rnd32() & 7u));
			break;
		default:
			gfp = (gfp_t)(rnd32() & ~GFP_COMP_BIT);
			break;
		}
		order = ((r >> 2) & 1) ? (unsigned int)(rnd32() & 31u) :
		    (unsigned int)rnd32();

		struct page *res = ((r >> 3) & 3) == 0 ? NULL :
		    page_pool[(r >> 5) & 7];

		case_folio_alloc(gfp, order, res);
	}
}

/* ------------------------------------------------------------------ */
/* __folio_batch_release                                               */
/* ------------------------------------------------------------------ */

static void
case_batch_release(unsigned int nr, struct folio *const *folios)
{
	enum { BUFSZ = sizeof(struct folio_batch) + 32 };
	alignas(16) unsigned char ba[BUFSZ], bb[BUFSZ];

	memset(ba, 0x7f, BUFSZ);
	memset(bb, 0x7f, BUFSZ);

	struct folio_batch *fa = (struct folio_batch *)(ba + 16);
	struct folio_batch *fb = (struct folio_batch *)(bb + 16);

	fa->nr = (uint8_t)nr;
	fb->nr = (uint8_t)nr;
	for (int i = 0; i < PAGEVEC_SIZE; i++) {
		fa->folios[i] = folios[i];
		fb->folios[i] = folios[i];
	}

	g_fbatch_base = fa;
	cap_begin(&g_log_port);
	P::__folio_batch_release(fa);
	g_fbatch_base = fb;
	cap_begin(&g_log_ref);
	ref___folio_batch_release(fb);

	bool ok = logs_equal(&g_log_port, &g_log_ref) &&
	    memcmp(ba, bb, BUFSZ) == 0;

	if (!record(&st_batch_rel, ok, "nr=%u port_nr=%u ref_nr=%u", nr,
	    (unsigned)fa->nr, (unsigned)fb->nr) &&
	    st_batch_rel.fails <= MAX_REPORTED) {
		printf("      port buf:");
		for (int i = 0; i < (int)BUFSZ; i++)
			printf(" %02x", ba[i]);
		printf("\n      ref  buf:");
		for (int i = 0; i < (int)BUFSZ; i++)
			printf(" %02x", bb[i]);
		printf("\n");
	}
}

static void
test_batch_release(void)
{
	struct folio *f[PAGEVEC_SIZE];
	static const unsigned int nrs[] = {
		0, 1, 2, 3, 13, 14, 15, 16, 17, 126, 127, 128, 129, 253, 254,
		255
	};

	for (size_t n = 0; n < sizeof(nrs) / sizeof(nrs[0]); n++) {
		for (int i = 0; i < PAGEVEC_SIZE; i++)
			f[i] = NULL;
		case_batch_release(nrs[n], f);
		for (int i = 0; i < PAGEVEC_SIZE; i++)
			f[i] = folio_pool[i & 7];
		case_batch_release(nrs[n], f);
		for (int i = 0; i < PAGEVEC_SIZE; i++)
			f[i] = (i & 1) ? NULL : folio_pool[0];
		case_batch_release(nrs[n], f);
	}

	rnd_seed(SEED ^ 0x33);
	for (long i = 0; i < ITERS; i++) {
		uint32_t r = rnd32();
		unsigned int nr;

		switch (r & 3) {
		case 0:
			nr = (unsigned int)(rnd32() & 3u);
			break;
		case 1:
			nr = 13u + (unsigned int)(rnd32() % 5u);
			break;
		case 2:
			nr = 252u + (unsigned int)(rnd32() & 3u);
			break;
		default:
			nr = (unsigned int)(rnd32() & 255u);
			break;
		}
		for (int j = 0; j < PAGEVEC_SIZE; j++) {
			uint32_t q = rnd32();

			f[j] = (q & 7) == 0 ? NULL : folio_pool[(q >> 3) & 7];
		}
		case_batch_release(nr, f);
	}
}

/* ------------------------------------------------------------------ */
/* lkpi_eventfd_ctx_fdget / lkpi_eventfd_ctx_put                       */
/* ------------------------------------------------------------------ */

static void
case_efd_fdget(int fd, int fget_ret, struct file *fp, struct eventfd_ctx *ctx)
{
	g_fget_ret = fget_ret;
	g_fget_fp = fp;
	g_efd_get_result = ctx;

	cap_begin(&g_log_port);
	struct eventfd_ctx *a = P::lkpi_eventfd_ctx_fdget(fd);
	cap_begin(&g_log_ref);
	struct eventfd_ctx *b = ref_lkpi_eventfd_ctx_fdget(fd);

	record(&st_efd_fdget, a == b && logs_equal(&g_log_port, &g_log_ref),
	    "fd=%d fget_ret=%d fp=%ld ctx=%ld port=%ld ref=%ld", fd, fget_ret,
	    ptr_id(fp), ptr_id(ctx), ptr_id(a), ptr_id(b));
}

static void
test_efd_fdget(void)
{
	static const int fds[] = {
		INT_MIN, -2, -1, 0, 1, 2, 3, 1024, INT_MAX
	};
	static const int rets[] = { 0, 1, -1, 9, -9, INT_MIN, INT_MAX };

	for (size_t i = 0; i < sizeof(fds) / sizeof(fds[0]); i++)
		for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
			for (int fpi = -1; fpi < 4; fpi++)
				for (int ci = -1; ci < 4; ci++)
					case_efd_fdget(fds[i], rets[r],
					    fpi < 0 ? NULL : file_pool[fpi],
					    ci < 0 ? NULL : ctx_pool[ci]);

	rnd_seed(SEED ^ 0x44);
	for (long i = 0; i < ITERS; i++) {
		uint32_t r = rnd32();
		int fd = ((r & 3) == 0) ? (int)rnd32() :
		    (int)(rnd32() & 7u) - 2;
		int fget_ret = ((r >> 2) & 1) ? 0 : (int)rnd32();
		struct file *fp = ((r >> 3) & 3) == 0 ? NULL :
		    file_pool[(r >> 5) & 3];
		struct eventfd_ctx *ctx = ((r >> 7) & 1) ? NULL :
		    ctx_pool[(r >> 8) & 3];

		case_efd_fdget(fd, fget_ret, fp, ctx);
	}
}

static void
case_efd_put(struct eventfd_ctx *ctx)
{
	cap_begin(&g_log_port);
	P::lkpi_eventfd_ctx_put(ctx);
	cap_begin(&g_log_ref);
	ref_lkpi_eventfd_ctx_put(ctx);

	record(&st_efd_put, logs_equal(&g_log_port, &g_log_ref), "ctx=%ld",
	    ptr_id(ctx));
}

static void
test_efd_put(void)
{
	case_efd_put(NULL);
	for (int i = 0; i < 4; i++)
		case_efd_put(ctx_pool[i]);
	case_efd_put((struct eventfd_ctx *)(intptr_t)-EBADF);

	rnd_seed(SEED ^ 0x55);
	for (long i = 0; i < ITERS; i++) {
		uint32_t r = rnd32();
		struct eventfd_ctx *ctx = (r & 3) == 0 ? NULL :
		    ctx_pool[(r >> 2) & 3];

		case_efd_put(ctx);
	}
}

/* ------------------------------------------------------------------ */
/* video_get_options                                                   */
/* ------------------------------------------------------------------ */

enum { VBUFSZ = 512 };

static long
env_id(const char *p)
{
	if (p == NULL)
		return (0);
	if (p == g_env_specific_val)
		return (1);
	if (p == g_env_default_val)
		return (2);
	return ((long)(intptr_t)p);
}

static void
case_video(const unsigned char *name, size_t namelen, int bv, int spec,
    int def)
{
	unsigned char ba[VBUFSZ], bb[VBUFSZ], pristine[VBUFSZ];
	char expect[64];

	memset(ba, 0x7f, VBUFSZ);
	memset(bb, 0x7f, VBUFSZ);
	memcpy(ba + 16, name, namelen);
	ba[16 + namelen] = '\0';
	memcpy(bb + 16, name, namelen);
	bb[16 + namelen] = '\0';
	memcpy(pristine, ba, VBUFSZ);

	/*
	 * The key the environment answers to is derived independently of the
	 * code under test, so a port that builds a different tunable string
	 * misses the lookup and takes the fallback.
	 */
	snprintf(expect, sizeof(expect), "kern.vt.fb.modes.%s",
	    (const char *)(ba + 16));
	memcpy(g_env_specific_key, expect, sizeof(expect));
	g_env_specific_present = spec;
	g_env_default_present = def;
	bootverbose = bv;

	cap_begin(&g_log_port);
	const char *a = P::video_get_options((const char *)(ba + 16));
	cap_begin(&g_log_ref);
	const char *b = ref_video_get_options((const char *)(bb + 16));

	bool ok = a == b && logs_equal(&g_log_port, &g_log_ref) &&
	    memcmp(ba, bb, VBUFSZ) == 0 && memcmp(ba, pristine, VBUFSZ) == 0 &&
	    memcmp(bb, pristine, VBUFSZ) == 0;

	if (ok) {
		record(&st_video, true, "-");
	} else {
		char pretty[1024];

		show(pretty, sizeof(pretty), name, namelen);
		record(&st_video, false,
		    "name(len=%zu)=\"%s\" bv=%d spec=%d def=%d port=%ld ref=%ld",
		    namelen, pretty, bv, spec, def, env_id(a), env_id(b));
	}
}

static void
video_all_modes(const unsigned char *name, size_t namelen)
{
	for (int bv = 0; bv < 2; bv++)
		for (int spec = 0; spec < 2; spec++)
			for (int def = 0; def < 2; def++)
				case_video(name, namelen, bv, spec, def);
}

static void
test_video(void)
{
	unsigned char buf[VBUFSZ];

	/* Empty, single character, plain names. */
	video_all_modes((const unsigned char *)"", 0);
	video_all_modes((const unsigned char *)"a", 1);
	video_all_modes((const unsigned char *)"LVDS", 4);
	video_all_modes((const unsigned char *)"kern.vt.fb.default_mode", 23);
	video_all_modes((const unsigned char *)"%s%d%n", 6);

	/*
	 * "kern.vt.fb.modes." is 17 bytes, so a 46 byte name is the longest
	 * one that survives a 64 byte buffer intact.  Walk both sides of that
	 * boundary one byte at a time.
	 */
	for (size_t len = 40; len <= 56; len++) {
		for (size_t i = 0; i < len; i++)
			buf[i] = (unsigned char)('A' + (i % 26));
		video_all_modes(buf, len);
	}

	/* High bit bytes, NUL heavy, long. */
	for (size_t len = 1; len <= 64; len++) {
		for (size_t i = 0; i < len; i++)
			buf[i] = (unsigned char)(0x80 + (i % 0x80));
		video_all_modes(buf, len);
	}
	for (size_t i = 0; i < 64; i++)
		buf[i] = (unsigned char)(i == 0 ? 'x' : 0x00);
	video_all_modes(buf, 64);
	for (size_t i = 0; i < 64; i++)
		buf[i] = 0x00;
	video_all_modes(buf, 64);
	for (size_t i = 0; i < 50; i++)
		buf[i] = (unsigned char)(i == 46 ? 0x00 : 0xff);
	video_all_modes(buf, 50);
	for (size_t i = 0; i < 300; i++)
		buf[i] = (unsigned char)('a' + (i % 26));
	video_all_modes(buf, 300);
	for (size_t i = 0; i < 300; i++)
		buf[i] = 0xa5;
	video_all_modes(buf, 300);

	rnd_seed(SEED ^ 0x66);
	for (long i = 0; i < ITERS; i++) {
		uint32_t r = rnd32();
		size_t len;

		switch (r & 3) {
		case 0:
			len = (size_t)(rnd32() % 8u);
			break;
		case 1:
			len = 40u + (size_t)(rnd32() % 17u);
			break;
		case 2:
			len = (size_t)(rnd32() % 70u);
			break;
		default:
			len = (size_t)(rnd32() % 200u);
			break;
		}
		for (size_t j = 0; j < len; j++) {
			uint32_t q = rnd32();

			switch ((q >> 8) & 7) {
			case 0:
			case 1:
				buf[j] = (unsigned char)(0x80 + (q & 0x7f));
				break;
			case 2:
				buf[j] = (unsigned char)(q & 0xff);
				break;
			case 3:
				buf[j] = (unsigned char)((q & 1) ? 0x00 : 0xff);
				break;
			default:
				buf[j] = (unsigned char)(0x20 + (q % 0x5f));
				break;
			}
		}
		case_video(buf, len, (int)((r >> 4) & 1), (int)((r >> 5) & 1),
		    (int)((r >> 6) & 1));
	}
}

/* ------------------------------------------------------------------ */

static void
pools_init(void)
{
	for (int i = 0; i < 8; i++) {
		page_pool[i] = (struct page *)page_backing[i];
		folio_pool[i] = (struct folio *)folio_backing[i];
		reg_add(page_backing[i], 100 + i);
		reg_add(folio_backing[i], 200 + i);
	}
	for (int i = 0; i < 4; i++) {
		file_pool[i] = (struct file *)file_backing[i];
		ctx_pool[i] = (struct eventfd_ctx *)ctx_backing[i];
		bsddev_pool[i] = (device_t)bsddev_backing[i];
		reg_add(file_backing[i], 300 + i);
		reg_add(ctx_backing[i], 400 + i);
		reg_add(bsddev_backing[i], 500 + i);
	}
	reg_add(thread_backing, 600);
	reg_add(&cap_no_rights, 700);
	for (int i = 0; i < MAXMEMDOM; i++)
		domainset_prefer[i].ds_id = 0x1000 + i;
}

int
main(void)
{
	pools_init();

	test_domain_set();
	test_dev_to_node();
	test_folio_alloc();
	test_batch_release();
	test_efd_fdget();
	test_efd_put();
	test_video();

	Stats *all[] = {
		&st_domain_set, &st_dev_to_node, &st_folio_alloc,
		&st_batch_rel, &st_efd_fdget, &st_efd_put, &st_video
	};
	long total_cases = 0, total_fails = 0;

	printf("\n%-28s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	printf("--------------------------------------------------------------"
	    "\n");
	for (size_t i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
		printf("%-28s %12ld %12ld  %s\n", all[i]->name, all[i]->cases,
		    all[i]->fails, all[i]->fails == 0 ? "PASS" : "FAIL");
		total_cases += all[i]->cases;
		total_fails += all[i]->fails;
	}
	printf("--------------------------------------------------------------"
	    "\n");
	printf("%-28s %12ld %12ld  %s\n", "TOTAL", total_cases, total_fails,
	    total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
