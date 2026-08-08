/*
 * oracle.c - reference implementation for PBSD batch b0217.
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

#ifndef LONG_BIT
#define LONG_BIT (8 * (int)sizeof(long))
#endif
#ifndef NBBY
#define NBBY 8
#endif
#ifndef __unused
#define __unused __attribute__((__unused__))
#endif
#ifndef __exclusive_cache_line
#define __exclusive_cache_line
#endif
#ifndef __assert_unreachable
#define __assert_unreachable() abort()
#endif

typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef uintptr_t vm_offset_t;
typedef char *caddr_t;
typedef long register_t;
typedef int pid_t;

#define nitems(x) (sizeof((x)) / sizeof((x)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CTASSERT(x) _Static_assert((x), "ctassert")

#define M_NOWAIT 0x0001
#define M_WAITOK 0x0002
#define M_ZERO   0x0100
#define HASH_WAITOK 0x00000001
#define HASH_NOWAIT 0x00000002

#define EINVAL 22
#define ENOENT 2
#ifndef EWOULDBLOCK
#define EWOULDBLOCK 35
#endif

#define LK_NOWAIT 0x0001
#define PID_MAX 99999
#define STACK_MAX 18

#define TS_ENTER 0
#define TS_EXIT 1
#define TS_THREAD 2
#define TS_EVENT 3

#define DDB 1
#define WITNESS 1
#define KTR 1

enum stack_sbuf_fmt {
	STACK_SBUF_FMT_NONE = 0,
	STACK_SBUF_FMT_LONG = 1,
	STACK_SBUF_FMT_COMPACT = 2,
};

#define LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;						\
}

#define LIST_ENTRY(type)						\
struct {								\
	struct type *le_next;						\
	struct type **le_prev;						\
}

#define LIST_FIRST(head)	((head)->lh_first)
#define LIST_NEXT(elm, field)	((elm)->field.le_next)
#define LIST_EMPTY(head)	(LIST_FIRST(head) == NULL)
#define LIST_INIT(head) do {						\
	LIST_FIRST(head) = NULL;					\
} while (0)

#define LIST_FOREACH(var, head, field)					\
	for ((var) = LIST_FIRST(head);					\
	    (var);							\
	    (var) = LIST_NEXT(var, field))

#define LIST_INSERT_HEAD(head, elm, field) do {				\
	if (((elm)->field.le_next = LIST_FIRST(head)) != NULL)		\
		LIST_FIRST(head)->field.le_prev = &(elm)->field.le_next;	\
	LIST_FIRST(head) = (elm);					\
	(elm)->field.le_prev = &LIST_FIRST(head);			\
} while (0)

#define LIST_REMOVE(elm, field) do {					\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev =			\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
} while (0)

struct malloc_type { const char *ks_shortdesc; };

#define MALLOC_DEFINE(type, shortdesc, longdesc)				\
	struct malloc_type type##_mt = { shortdesc };			\
	struct malloc_type * const type = &type##_mt

struct thread { char td_pad; };
struct mount { u_int mnt_hashseed; };
struct vnode {
	u_int v_hash;
	struct mount *v_mount;
	LIST_ENTRY(vnode) v_hashlist;
	int v_hold;
	int v_refs;
	int v_gone;
};
enum vgetstate { VGET_NONE, VGET_HOLDCNT, VGET_USECOUNT };
typedef int vfs_hash_cmp_t(struct vnode *, void *);

struct rwlock { int lk; const char *name; };
struct timecounter {
	u_int (*tc_get_timecount)(struct timecounter *);
	u_int tc_counter_mask;
	uint64_t tc_frequency;
	const char *tc_name;
};
struct stack { int depth; vm_offset_t pcs[STACK_MAX]; };
struct sbuf {
	char *s_buf;
	size_t s_size;
	size_t s_len;
	int s_error;
};
struct sysctl_oid;
struct sysctl_req { int ignored; };
#define SYSCTL_HANDLER_ARGS struct sysctl_oid *oidp, void *arg1, int arg2, struct sysctl_req *req
struct sysinit_tslog {
	void (*func)(void *);
	void *data;
	const char *name;
};
typedef void *c_linker_sym_t;
typedef struct { const char *name; } linker_symval_t;

u_long desiredvnodes = 64;
int bootverbose;
struct thread thread0;
struct thread *curthread = &thread0;
struct timecounter *timecounter;

#ifndef TSLOGSIZE
#define TSLOGSIZE 4096
#endif

enum {
	MODEL_GUARD = 0x7f,
	MODEL_ARENA = 262144,
	MODEL_OUT = 262144,
	MODEL_LOG = 16384,
};

struct model_ev {
	int op;
	unsigned long long a, b, c, d;
};

static struct model_ev model_log[MODEL_LOG];
static int model_log_n;
static struct model_ev model_slog[MODEL_LOG];
static int model_slog_n;
static char model_out[MODEL_OUT];
static size_t model_out_n;
static char model_sout[MODEL_OUT];
static size_t model_sout_n;
static unsigned char model_arena[MODEL_ARENA];
static size_t model_arena_off;
static int model_malloc_fail;
static unsigned long model_malloc_max = MODEL_ARENA;
static uint64_t model_cycle;
static int model_vget_error;
static int model_vget_enoent;
static int model_linker_fail;
static int model_linker_block;
static int model_sbuf_fail;
static int model_ktr_n;

static unsigned long long model_strhash(const char *s)
{
	unsigned long long h = 1469598103934665603ULL;

	if (s == NULL)
		return (0);
	while (*s != '\0') {
		h ^= (unsigned char)*s++;
		h *= 1099511628211ULL;
	}
	return (h);
}

static void model_ev(int op, unsigned long long a, unsigned long long b,
    unsigned long long c, unsigned long long d)
{
	if (model_log_n < MODEL_LOG)
		model_log[model_log_n++] = (struct model_ev){ op, a, b, c, d };
}

static void model_assert(int cond)
{
	model_ev(1, (unsigned long long)(cond != 0), 0, 0, 0);
}

#define KASSERT(e, m) model_assert((e) ? 1 : 0)

static void model_bzero(void *p, unsigned long len)
{
	model_ev(2, len, 0, 0, 0);
	memset(p, 0, (size_t)len);
}

static void *model_kmalloc(unsigned long size, struct malloc_type *type, int flags)
{
	unsigned long need = (size + 15UL) & ~15UL;
	void *ret;

	if (model_malloc_fail != 0 || size > model_malloc_max ||
	    model_arena_off + need > MODEL_ARENA) {
		model_ev(3, size, flags, ~0ULL,
		    model_strhash(type != NULL ? type->ks_shortdesc : NULL));
		return (NULL);
	}
	ret = &model_arena[model_arena_off];
	model_ev(3, size, flags, model_arena_off,
	    model_strhash(type != NULL ? type->ks_shortdesc : NULL));
	model_arena_off += need;
	return (ret);
}

static void model_kfree(void *p, struct malloc_type *type)
{
	unsigned long long off = ~0ULL;

	if (p != NULL && (unsigned char *)p >= model_arena &&
	    (unsigned char *)p < model_arena + MODEL_ARENA)
		off = (unsigned char *)p - model_arena;
	model_ev(4, off, 0, 0,
	    model_strhash(type != NULL ? type->ks_shortdesc : NULL));
}

static int model_printf(const char *fmt, ...)
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
		if (model_out_n + len + 1 < MODEL_OUT) {
			memcpy(model_out + model_out_n, tmp, len);
			model_out_n += len;
			model_out[model_out_n] = '\0';
		}
	}
	model_ev(5, (unsigned long long)(long long)n, 0, 0, 0);
	return (n);
}

static int model_hash_mflags(int flags)
{
	return ((flags & HASH_NOWAIT) ? M_NOWAIT : M_WAITOK);
}

void *hashinit(int elements, struct malloc_type *type, u_long *hashmask)
{
	long hashsize, i;
	LIST_HEAD(generic, generic) *hashtbl;

	KASSERT(elements > 0, ("bad elements"));
	for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
		continue;
	hashsize >>= 1;
	hashtbl = model_kmalloc((unsigned long)hashsize * sizeof(*hashtbl),
	    type, model_hash_mflags(HASH_WAITOK));
	if (hashtbl != NULL) {
		for (i = 0; i < hashsize; i++)
			LIST_INIT(&hashtbl[i]);
		*hashmask = (u_long)(hashsize - 1);
	}
	return (hashtbl);
}

static void rw_init(struct rwlock *rw, const char *name)
{
	rw->lk = 0;
	rw->name = name;
	model_ev(10, 0, 0, 0, model_strhash(name));
}

static void rw_rlock(struct rwlock *rw)
{
	rw->lk++;
	model_ev(11, (unsigned long long)rw->lk, 0, 0, 0);
}

static void rw_runlock(struct rwlock *rw)
{
	rw->lk--;
	model_ev(12, (unsigned long long)rw->lk, 0, 0, 0);
}

static void rw_wlock(struct rwlock *rw)
{
	rw->lk = -1;
	model_ev(13, 0, 0, 0, 0);
}

static void rw_wunlock(struct rwlock *rw)
{
	rw->lk = 0;
	model_ev(14, 0, 0, 0, 0);
}

static enum vgetstate vget_prep(struct vnode *vp)
{
	(void)vp;
	model_ev(20, 0, 0, 0, 0);
	return (VGET_NONE);
}

static int vget_finish(struct vnode *vp, int flags, enum vgetstate vs)
{
	(void)vp;
	(void)flags;
	(void)vs;
	if (model_vget_enoent != 0) {
		model_ev(21, ENOENT, 0, 0, 0);
		return (ENOENT);
	}
	if (model_vget_error != 0) {
		model_ev(21, (unsigned long long)model_vget_error, 0, 0, 0);
		return (model_vget_error);
	}
	model_ev(21, 0, 0, 0, 0);
	return (0);
}

static void vhold(struct vnode *vp)
{
	vp->v_hold++;
	model_ev(22, (unsigned long long)vp->v_hold, 0, 0, 0);
}

static void vref(struct vnode *vp)
{
	vp->v_refs++;
	model_ev(23, (unsigned long long)vp->v_refs, 0, 0, 0);
}

static void vdrop(struct vnode *vp)
{
	vp->v_hold--;
	model_ev(24, (unsigned long long)vp->v_hold, 0, 0, 0);
}

static void vput(struct vnode *vp)
{
	vp->v_refs--;
	model_ev(25, (unsigned long long)vp->v_refs, 0, 0, 0);
}

static void vgone(struct vnode *vp)
{
	vp->v_gone = 1;
	model_ev(26, 0, 0, 0, 0);
}

#define ASSERT_VOP_ELOCKED(vp, msg) KASSERT(1, (msg))

static uint64_t get_cyclecount(void)
{
	model_cycle++;
	model_ev(30, model_cycle, 0, 0, 0);
	return (model_cycle);
}

static long atomic_fetchadd_long(volatile long *p, long v)
{
	long old = *p;

	*p += v;
	model_ev(31, (unsigned long long)old, (unsigned long long)v, 0, 0);
	return (old);
}

#define atomic_load_ptr(p) (*(p))

static void cpu_spinwait(void)
{
	model_ev(32, 0, 0, 0, 0);
}

#define TSENTER() ((void)0)
#define TSEXIT() ((void)0)

static caddr_t preload_search_by_type(const char *type)
{
	(void)type;
	return (NULL);
}

static void *preload_fetch_addr(caddr_t p)
{
	(void)p;
	return (NULL);
}

static size_t preload_fetch_size(caddr_t p)
{
	(void)p;
	return (0);
}

static struct sbuf *sbuf_new_for_sysctl(struct sbuf *x, char *y, int z,
    struct sysctl_req *req)
{
	struct sbuf *sb;

	(void)x;
	(void)y;
	(void)z;
	(void)req;
	sb = model_kmalloc(sizeof(*sb), NULL, M_WAITOK);
	if (sb != NULL) {
		sb->s_size = 8192;
		sb->s_buf = model_kmalloc(sb->s_size, NULL, M_WAITOK);
		sb->s_len = 0;
		sb->s_error = 0;
		if (sb->s_buf != NULL)
			sb->s_buf[0] = '\0';
	}
	return (sb);
}

static int sbuf_bcat(struct sbuf *sb, const void *buf, size_t n)
{
	if (sb == NULL || sb->s_buf == NULL || sb->s_len + n >= sb->s_size)
		return (-1);
	memcpy(sb->s_buf + sb->s_len, buf, n);
	sb->s_len += n;
	sb->s_buf[sb->s_len] = '\0';
	return (0);
}

static int sbuf_printf(struct sbuf *sb, const char *fmt, ...)
{
	char tmp[512];
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	return (sbuf_bcat(sb, tmp, (size_t)n));
}

static int sbuf_cat(struct sbuf *sb, const char *s)
{
	return (sbuf_bcat(sb, s, strlen(s)));
}

static int sbuf_putc(struct sbuf *sb, int c)
{
	char ch = (char)c;

	return (sbuf_bcat(sb, &ch, 1));
}

static int sbuf_finish(struct sbuf *sb)
{
	(void)sb;
	return (model_sbuf_fail != 0 ? EINVAL : 0);
}

static void sbuf_delete(struct sbuf *sb)
{
	if (sb != NULL) {
		if (sb->s_buf != NULL)
			model_kfree(sb->s_buf, NULL);
		model_kfree(sb, NULL);
	}
}

static void sbuf_nl_terminate(struct sbuf *sb)
{
	if (sb != NULL && sb->s_len > 0 && sb->s_buf[sb->s_len - 1] != '\n')
		sbuf_putc(sb, '\n');
}

static char *kernel_strdup(const char *s, struct malloc_type *type)
{
	size_t n = strlen(s) + 1;
	char *p = model_kmalloc(n, type, M_WAITOK);

	if (p != NULL)
		memcpy(p, s, n);
	return (p);
}

static size_t strlcpy(char *dst, const char *src, size_t sz)
{
	size_t n = strlen(src);

	if (sz != 0) {
		size_t c = n < sz - 1 ? n : sz - 1;
		memcpy(dst, src, c);
		dst[c] = '\0';
	}
	return (n);
}

static int linker_search_symbol_name_flags(caddr_t pc, char *namebuf,
    u_int buflen, long *offset, int flags)
{
	model_ev(40, (unsigned long long)(uintptr_t)pc, 0, 0, 0);
	if (model_linker_block != 0 && (flags & M_NOWAIT) != 0)
		return (EWOULDBLOCK);
	if (model_linker_fail != 0) {
		*offset = 0;
		strlcpy(namebuf, "??", buflen);
		return (ENOENT);
	}
	snprintf(namebuf, buflen, "sym_%lx", (unsigned long)(uintptr_t)pc);
	*offset = (long)((uintptr_t)pc & 0xff);
	return (0);
}

static int linker_ddb_search_symbol(caddr_t pc, c_linker_sym_t *sym, long *offset)
{
	*sym = (c_linker_sym_t)(uintptr_t)pc;
	*offset = (long)((uintptr_t)pc & 0xff);
	return (0);
}

static int linker_ddb_symbol_values(c_linker_sym_t sym, linker_symval_t *sv)
{
	sv->name = (const char *)(uintptr_t)sym;
	return (0);
}

static void ktr_tracepoint(u_int mask, const char *file, int line,
    const char *fmt, ...)
{
	(void)mask;
	(void)file;
	(void)line;
	(void)fmt;
	model_ktr_n++;
	model_ev(41, (unsigned long long)model_ktr_n, 0, 0, 0);
}

void model_reset(void)
{
	model_log_n = 0;
	model_out_n = 0;
	model_out[0] = '\0';
	model_arena_off = 0;
	memset(model_arena, MODEL_GUARD, MODEL_ARENA);
	model_malloc_fail = 0;
	model_vget_error = 0;
	model_vget_enoent = 0;
	model_linker_fail = 0;
	model_linker_block = 0;
	model_sbuf_fail = 0;
	model_ktr_n = 0;
	model_cycle = 0;
}

void model_snapshot(void)
{
	memcpy(model_slog, model_log, sizeof(model_log[0]) * (size_t)model_log_n);
	model_slog_n = model_log_n;
	memcpy(model_sout, model_out, model_out_n + 1);
	model_sout_n = model_out_n;
}

int model_diff(char *msg, size_t msgsz)
{
	int n;

	if (model_log_n != model_slog_n) {
		snprintf(msg, msgsz, "env calls ref=%d port=%d",
		    model_slog_n, model_log_n);
		return (1);
	}
	for (n = 0; n < model_log_n; n++) {
		if (memcmp(&model_log[n], &model_slog[n], sizeof(model_log[0])) != 0) {
			snprintf(msg, msgsz, "env call %d differs", n);
			return (1);
		}
	}
	if (model_out_n != model_sout_n ||
	    memcmp(model_out, model_sout, model_out_n) != 0) {
		snprintf(msg, msgsz, "printf output differs");
		return (1);
	}
	return (0);
}

const char *model_out_text(void) { return (model_out); }
size_t model_out_length(void) { return (model_out_n); }

void ref_vfs_hashinit(void *dummy);
void ref_tslog_reset(void);

void oracle_reset(void)
{
	model_reset();
	ref_vfs_hashinit(NULL);
	ref_tslog_reset();
}

void oracle_malloc_fail_at(int n) { model_malloc_fail = n; }
void oracle_set_vget_enoent(int v) { model_vget_enoent = v; }
void oracle_set_vget_error(int v) { model_vget_error = v; }
void oracle_set_linker_fail(int v) { model_linker_fail = v; }
void oracle_set_linker_block(int v) { model_linker_block = v; }
void oracle_set_bootverbose(int v) { bootverbose = v; }
void oracle_set_sbuf_fail(int v) { model_sbuf_fail = v; }
int oracle_ktr_count(void) { return (model_ktr_n); }

long oracle_tslog_nrecs(void);
int oracle_tslog_get(long i, void **td, int *type, const char **f,
    const char **s, uint64_t *tsc);
int oracle_tslog_user_get(pid_t pid, pid_t *ppid, uint64_t *tf, uint64_t *te,
    const char **ex, const char **nm, int *reused);

#define bzero(p, n) model_bzero((p), (unsigned long)(n))
#define malloc(s, t, f) model_kmalloc((unsigned long)(s), (t), (f))
#define free(p, t) model_kfree((p), (t))
#define printf(...) model_printf(__VA_ARGS__)
#define strdup(s, t) kernel_strdup((s), (t))


/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 Poul-Henning Kamp
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
 *
 */


static MALLOC_DEFINE(M_VFS_HASH, "vfs_hash", "VFS hash table");

static LIST_HEAD(vfs_hash_head, vnode)	*vfs_hash_tbl;
static LIST_HEAD(,vnode)		vfs_hash_side;
static u_long				vfs_hash_mask;
static struct rwlock __exclusive_cache_line vfs_hash_lock;

void
ref_vfs_hashinit(void *dummy __unused)
{

	vfs_hash_tbl = hashinit(desiredvnodes, M_VFS_HASH, &vfs_hash_mask);
	rw_init(&vfs_hash_lock, "vfs hash");
	LIST_INIT(&vfs_hash_side);
}


static struct vfs_hash_head *
ref_vfs_hash_bucket(const struct mount *mp, u_int hash)
{

	return (&vfs_hash_tbl[(hash + mp->mnt_hashseed) & vfs_hash_mask]);
}

int
ref_vfs_hash_get(const struct mount *mp, u_int hash, int flags, struct thread *td,
    struct vnode **vpp, vfs_hash_cmp_t *fn, void *arg)
{
	struct vnode *vp;
	enum vgetstate vs;
	int error;

	while (1) {
		rw_rlock(&vfs_hash_lock);
		LIST_FOREACH(vp, ref_vfs_hash_bucket(mp, hash), v_hashlist) {
			if (vp->v_hash != hash)
				continue;
			if (vp->v_mount != mp)
				continue;
			if (fn != NULL && fn(vp, arg))
				continue;
			vs = vget_prep(vp);
			rw_runlock(&vfs_hash_lock);
			error = vget_finish(vp, flags, vs);
			if (error == ENOENT && (flags & LK_NOWAIT) == 0)
				break;
			if (error != 0)
				return (error);
			if (vp->v_hash != hash ||
			    (fn != NULL && fn(vp, arg))) {
				vput(vp);
				/* Restart the bucket walk. */
				break;
			}
			*vpp = vp;
			return (0);
		}
		if (vp == NULL) {
			rw_runlock(&vfs_hash_lock);
			*vpp = NULL;
			return (0);
		}
	}
}

void
ref_vfs_hash_ref(const struct mount *mp, u_int hash, struct thread *td,
    struct vnode **vpp, vfs_hash_cmp_t *fn, void *arg)
{
	struct vnode *vp;

	while (1) {
		rw_rlock(&vfs_hash_lock);
		LIST_FOREACH(vp, ref_vfs_hash_bucket(mp, hash), v_hashlist) {
			if (vp->v_hash != hash)
				continue;
			if (vp->v_mount != mp)
				continue;
			if (fn != NULL && fn(vp, arg))
				continue;
			vhold(vp);
			rw_runlock(&vfs_hash_lock);
			vref(vp);
			vdrop(vp);
			*vpp = vp;
			return;
		}
		if (vp == NULL) {
			rw_runlock(&vfs_hash_lock);
			*vpp = NULL;
			return;
		}
	}
}

void
ref_vfs_hash_remove(struct vnode *vp)
{

	rw_wlock(&vfs_hash_lock);
	LIST_REMOVE(vp, v_hashlist);
	rw_wunlock(&vfs_hash_lock);
}

int
ref_vfs_hash_insert(struct vnode *vp, u_int hash, int flags, struct thread *td,
    struct vnode **vpp, vfs_hash_cmp_t *fn, void *arg)
{
	struct vnode *vp2;
	enum vgetstate vs;
	int error;

	*vpp = NULL;
	while (1) {
		rw_wlock(&vfs_hash_lock);
		LIST_FOREACH(vp2,
		    ref_vfs_hash_bucket(vp->v_mount, hash), v_hashlist) {
			if (vp2->v_hash != hash)
				continue;
			if (vp2->v_mount != vp->v_mount)
				continue;
			if (fn != NULL && fn(vp2, arg))
				continue;
			vs = vget_prep(vp2);
			rw_wunlock(&vfs_hash_lock);
			error = vget_finish(vp2, flags, vs);
			if (error == ENOENT && (flags & LK_NOWAIT) == 0)
				break;
			rw_wlock(&vfs_hash_lock);
			LIST_INSERT_HEAD(&vfs_hash_side, vp, v_hashlist);
			rw_wunlock(&vfs_hash_lock);
			vgone(vp);
			vput(vp);
			if (!error)
				*vpp = vp2;
			return (error);
		}
		if (vp2 == NULL)
			break;
	}
	vp->v_hash = hash;
	LIST_INSERT_HEAD(ref_vfs_hash_bucket(vp->v_mount, hash), vp, v_hashlist);
	rw_wunlock(&vfs_hash_lock);
	return (0);
}

void
ref_vfs_hash_rehash(struct vnode *vp, u_int hash)
{
	ASSERT_VOP_ELOCKED(vp, "rehash requires excl lock");

	rw_wlock(&vfs_hash_lock);
	LIST_REMOVE(vp, v_hashlist);
	LIST_INSERT_HEAD(ref_vfs_hash_bucket(vp->v_mount, hash), vp, v_hashlist);
	vp->v_hash = hash;
	rw_wunlock(&vfs_hash_lock);
}

void
ref_vfs_hash_changesize(u_long newmaxvnodes)
{
	struct vfs_hash_head *vfs_hash_newtbl, *vfs_hash_oldtbl;
	u_long vfs_hash_newmask, vfs_hash_oldmask;
	struct vnode *vp;
	int i;

	vfs_hash_newtbl = hashinit(newmaxvnodes, M_VFS_HASH,
		&vfs_hash_newmask);
	/* If same hash table size, nothing to do */
	if (vfs_hash_mask == vfs_hash_newmask) {
		free(vfs_hash_newtbl, M_VFS_HASH);
		return;
	}
	/*
	 * Move everything from the old hash table to the new table.
	 * None of the vnodes in the table can be recycled because to
	 * do so, they have to be removed from the hash table.
	 */
	rw_wlock(&vfs_hash_lock);
	vfs_hash_oldtbl = vfs_hash_tbl;
	vfs_hash_oldmask = vfs_hash_mask;
	vfs_hash_tbl = vfs_hash_newtbl;
	vfs_hash_mask = vfs_hash_newmask;
	for (i = 0; i <= vfs_hash_oldmask; i++) {
		while ((vp = LIST_FIRST(&vfs_hash_oldtbl[i])) != NULL) {
			LIST_REMOVE(vp, v_hashlist);
			LIST_INSERT_HEAD(
			    ref_vfs_hash_bucket(vp->v_mount, vp->v_hash),
			    vp, v_hashlist);
		}
	}
	rw_wunlock(&vfs_hash_lock);
	free(vfs_hash_oldtbl, M_VFS_HASH);
}
/*-
 * Copyright (c) 2017 Colin Percival
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



#ifndef TSLOGSIZE
#define TSLOGSIZE 262144
#endif

static volatile long nrecs = 0;
static struct timestamp {
	void * td;
	int type;
	const char * f;
	const char * s;
	uint64_t tsc;
} timestamps[TSLOGSIZE];

void
ref_tslog(void * td, int type, const char * f, const char * s)
{
	uint64_t tsc = get_cyclecount();
	long pos;

	/* A NULL thread is thread0 before curthread is set. */
	if (td == NULL)
		td = &thread0;

	/* Grab a slot. */
	pos = atomic_fetchadd_long(&nrecs, 1);

	/* Store record. */
	if (pos < nitems(timestamps)) {
		timestamps[pos].td = td;
		timestamps[pos].type = type;
		timestamps[pos].f = f;
		timestamps[pos].s = s;
		timestamps[pos].tsc = tsc;
	}
}

int
ref_sysctl_debug_tslog(SYSCTL_HANDLER_ARGS)
{
	int error;
	struct sbuf *sb;
	size_t i, limit;
	caddr_t loader_tslog;
	void * loader_tslog_buf;
	size_t loader_tslog_len;

	/*
	 * This code can race against the code in tslog() which stores
	 * records: Theoretically we could end up reading a record after
	 * its slots have been reserved but before it has been written.
	 * Since this code takes orders of magnitude longer to run than
	 * ref_tslog() takes to write a record, it is highly unlikely that
	 * anyone will ever experience this race.
	 */
	sb = sbuf_new_for_sysctl(NULL, NULL, 1024, req);

	/* Get data from the boot loader, if it provided any. */
	loader_tslog = preload_search_by_type("TSLOG data");
	if (loader_tslog != NULL) {
		loader_tslog_buf = preload_fetch_addr(loader_tslog);
		loader_tslog_len = preload_fetch_size(loader_tslog);
		sbuf_bcat(sb, loader_tslog_buf, loader_tslog_len);
	}

	/* Add data logged within the kernel. */
	limit = MIN(nrecs, nitems(timestamps));
	for (i = 0; i < limit; i++) {
		sbuf_printf(sb, "%p", timestamps[i].td);
		sbuf_printf(sb, " %llu",
		    (unsigned long long)timestamps[i].tsc);
		switch (timestamps[i].type) {
		case TS_ENTER:
			sbuf_cat(sb, " ENTER");
			break;
		case TS_EXIT:
			sbuf_cat(sb, " EXIT");
			break;
		case TS_THREAD:
			sbuf_cat(sb, " THREAD");
			break;
		case TS_EVENT:
			sbuf_cat(sb, " EVENT");
			break;
		}
		sbuf_printf(sb, " %s", timestamps[i].f ? timestamps[i].f : "(null)");
		if (timestamps[i].s)
			sbuf_printf(sb, " %s\n", timestamps[i].s);
		else
			sbuf_putc(sb, '\n');
	}
	error = sbuf_finish(sb);
	sbuf_delete(sb);
	return (error);
}


MALLOC_DEFINE(M_TSLOGUSER, "tsloguser", "Strings used by userland tslog");
static struct procdata {
	pid_t ppid;
	uint64_t tsc_forked;
	uint64_t tsc_exited;
	char * execname;
	char * namei;
	int reused;
} procs[PID_MAX + 1];

void
ref_tslog_user(pid_t pid, pid_t ppid, const char * execname, const char * namei)
{
	uint64_t tsc = get_cyclecount();

	/* If we wrapped, do nothing. */
	if (procs[pid].reused)
		return;

	/* If we have a ppid, we're recording a fork. */
	if (ppid != (pid_t)(-1)) {
		/* If we have a ppid already, we wrapped. */
		if (procs[pid].ppid) {
			procs[pid].reused = 1;
			return;
		}

		/* Fill in some fields. */
		procs[pid].ppid = ppid;
		procs[pid].tsc_forked = tsc;
		return;
	}

	/* If we have an execname, record it. */
	if (execname != NULL) {
		if (procs[pid].execname != NULL)
			free(procs[pid].execname, M_TSLOGUSER);
		procs[pid].execname = strdup(execname, M_TSLOGUSER);
		return;
	}

	/* Record the first namei for the process. */
	if (namei != NULL) {
		if (procs[pid].namei == NULL)
			procs[pid].namei = strdup(namei, M_TSLOGUSER);
		return;
	}

	/* Otherwise we're recording an exit. */
	procs[pid].tsc_exited = tsc;
}

int
ref_sysctl_debug_tslog_user(SYSCTL_HANDLER_ARGS)
{
	int error;
	struct sbuf *sb;
	pid_t pid;

	sb = sbuf_new_for_sysctl(NULL, NULL, 1024, req);

	/* Export the data we logged. */
	for (pid = 0; pid <= PID_MAX; pid++) {
		sbuf_printf(sb, "%zu", (size_t)pid);
		sbuf_printf(sb, " %zu", (size_t)procs[pid].ppid);
		sbuf_printf(sb, " %llu",
		    (unsigned long long)procs[pid].tsc_forked);
		sbuf_printf(sb, " %llu",
		    (unsigned long long)procs[pid].tsc_exited);
		sbuf_printf(sb, " \"%s\"", procs[pid].execname ?
		    procs[pid].execname : "");
		sbuf_printf(sb, " \"%s\"", procs[pid].namei ?
		    procs[pid].namei : "");
		sbuf_putc(sb, '\n');
	}
	error = sbuf_finish(sb);
	sbuf_delete(sb);
	return (error);
}


void
ref_sysinit_tslog_shim(const void *data)
{
	const struct sysinit_tslog *x = data;

	ref_tslog(curthread, TS_ENTER, "SYSINIT", x->name);
	(x->func)(x->data);
	ref_tslog(curthread, TS_EXIT, "SYSINIT", x->name);
}

void
ref_tslog_reset(void)
{
	long i;

	nrecs = 0;
	for (i = 0; i < (long)nitems(timestamps); i++) {
		timestamps[i].td = NULL;
		timestamps[i].type = 0;
		timestamps[i].f = NULL;
		timestamps[i].s = NULL;
		timestamps[i].tsc = 0;
	}
	for (i = 0; i <= PID_MAX; i++) {
		if (procs[i].execname != NULL)
			free(procs[i].execname, M_TSLOGUSER);
		if (procs[i].namei != NULL)
			free(procs[i].namei, M_TSLOGUSER);
	}
	memset(procs, 0, sizeof(procs));
}
/*-
 * Copyright (c) 2022 Colin Percival
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


/**
 * clockcalib(clk, clkname):
 * Return the frequency of the provided timer, as calibrated against the
 * current best-available timecounter.
 */
uint64_t
ref_clockcalib(uint64_t (*clk)(void), const char *clkname)
{
	struct timecounter *tc = atomic_load_ptr(&timecounter);
	uint64_t clk0, clk1, clk_delay, n, passes = 0;
	uint64_t t0, t1, tadj, tlast;
	double mu_clk = 0;
	double mu_t = 0;
	double va_clk = 0;
	double va_t = 0;
	double cva = 0;
	double d1, d2;
	double inv_n;
	uint64_t freq;

	TSENTER();
	/*-
	 * The idea here is to compute a best-fit linear regression between
	 * the clock we're calibrating and the reference clock; the slope of
	 * that line multiplied by the frequency of the reference clock gives
	 * us the frequency we're looking for.
	 *
	 * To do this, we calculate the
	 * (a) mean of the target clock measurements,
	 * (b) variance of the target clock measurements,
	 * (c) mean of the reference clock measurements,
	 * (d) variance of the reference clock measurements, and
	 * (e) covariance of the target clock and reference clock measurements
	 * on an ongoing basis, updating all five values after each new data
	 * point arrives, stopping when we're confident that we've accurately
	 * measured the target clock frequency.
	 *
	 * Given those five values, the important formulas to remember from
	 * introductory statistics are:
	 * 1. slope of regression line = covariance(x, y) / variance(x)
	 * 2. (relative uncertainty in slope)^2 =
	 *    (variance(x) * variance(y) - covariance(x, y)^2)
	 *    ------------------------------------------------
	 *              covariance(x, y)^2 * (N - 2)
	 *
	 * We adjust the second formula slightly, adding a term to each of
	 * the variance values to reflect the measurement quantization.
	 *
	 * Finally, we need to determine when to stop gathering data.  We
	 * can't simply stop as soon as the computed uncertainty estimate
	 * is below our threshold; this would make us overconfident since it
	 * would introduce a multiple-comparisons problem (cf. sequential
	 * analysis in clinical trials).  Instead, we stop with N data points
	 * if the estimated uncertainty of the first k data points meets our
	 * target for all N/2 < k <= N; this is not theoretically optimal,
	 * but in practice works well enough.
	 */

	/*
	 * Initial values for clocks; we'll subtract these off from values
	 * we measure later in order to reduce floating-point rounding errors.
	 * We keep track of an adjustment for values read from the reference
	 * timecounter, since it can wrap.
	 */
	clk0 = clk();
	t0 = tc->tc_get_timecount(tc) & tc->tc_counter_mask;
	tadj = 0;
	tlast = t0;

	/* Loop until we give up or decide that we're calibrated. */
	for (n = 1; ; n++) {
		/* Get a new data point. */
		clk1 = clk() - clk0;
		t1 = tc->tc_get_timecount(tc) & tc->tc_counter_mask;
		while (t1 + tadj < tlast)
			tadj += (uint64_t)tc->tc_counter_mask + 1;
		tlast = t1 + tadj;
		t1 += tadj - t0;

		/* If we spent too long, bail. */
		if (t1 > tc->tc_frequency) {
			printf("Statistical %s calibration failed!  "
			    "Clocks might be ticking at variable rates.\n",
			     clkname);
			printf("Falling back to slow %s calibration.\n",
			    clkname);
			freq = (double)(tc->tc_frequency) * clk1 / t1;
			break;
		}

		/* Precompute to save on divisions later. */
		inv_n = 1.0 / n;

		/* Update mean and variance of recorded TSC values. */
		d1 = clk1 - mu_clk;
		mu_clk += d1 * inv_n;
		d2 = d1 * (clk1 - mu_clk);
		va_clk += (d2 - va_clk) * inv_n;

		/* Update mean and variance of recorded time values. */
		d1 = t1 - mu_t;
		mu_t += d1 * inv_n;
		d2 = d1 * (t1 - mu_t);
		va_t += (d2 - va_t) * inv_n;

		/* Update covariance. */
		d2 = d1 * (clk1 - mu_clk);
		cva += (d2 - cva) * inv_n;

		/*
		 * Count low-uncertainty iterations.  This is a rearrangement
		 * of "relative uncertainty < 1 PPM" avoiding division.
		 */
#define TSC_PPM_UNCERTAINTY	1
#define TSC_UNCERTAINTY		TSC_PPM_UNCERTAINTY * 0.000001
#define TSC_UNCERTAINTY_SQR	TSC_UNCERTAINTY * TSC_UNCERTAINTY
		if (TSC_UNCERTAINTY_SQR * (n - 2) * cva * cva >
		    (va_t + 4) * (va_clk + 4) - cva * cva)
			passes++;
		else
			passes = 0;

		/* Break if we're consistently certain. */
		if (passes * 2 > n) {
			freq = (double)(tc->tc_frequency) * cva / va_t;
			if (bootverbose)
				printf("Statistical %s calibration took"
				    " %lu us and %lu data points\n",
				    clkname, (unsigned long)(t1 *
					1000000.0 / tc->tc_frequency),
				    (unsigned long)n);
			break;
		}

		/*
		 * Add variable delay to avoid theoretical risk of aliasing
		 * resulting from this loop synchronizing with the frequency
		 * of the reference clock.  On the nth iteration, we spend
		 * O(1 / n) time here -- long enough to avoid aliasing, but
		 * short enough to be insignificant as n grows.
		 */
		clk_delay = clk() + (clk() - clk0) / (n * n);
		while (clk() < clk_delay)
			cpu_spinwait(); /* Do nothing. */
	}
	TSEXIT();
	return (freq);
}
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 Antoine Brodin
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


#ifdef KTR
#endif


MALLOC_DEFINE(M_STACK, "stack", "Stack Traces");

int ref_stack_symbol(vm_offset_t pc, char *namebuf, u_int buflen,
	    long *offset, int flags);
int ref_stack_symbol_ddb(vm_offset_t pc, const char **name, long *offset);

struct stack *
ref_stack_create(int flags)
{
	struct stack *st;

	st = malloc(sizeof(*st), M_STACK, flags | M_ZERO);
	return (st);
}

void
ref_stack_destroy(struct stack *st)
{

	free(st, M_STACK);
}

int
ref_stack_put(struct stack *st, vm_offset_t pc)
{

	if (st->depth < STACK_MAX) {
		st->pcs[st->depth++] = pc;
		return (0);
	} else
		return (-1);
}

void
ref_stack_copy(const struct stack *src, struct stack *dst)
{

	*dst = *src;
}

void
ref_stack_zero(struct stack *st)
{

	bzero(st, sizeof *st);
}

void
ref_stack_print(const struct stack *st)
{
	char namebuf[64];
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		(void)ref_stack_symbol(st->pcs[i], namebuf, sizeof(namebuf),
		    &offset, M_WAITOK);
		printf("#%d %p at %s+%#lx\n", i, (void *)st->pcs[i],
		    namebuf, offset);
	}
}

void
ref_stack_print_short(const struct stack *st)
{
	char namebuf[64];
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		if (i > 0)
			printf(" ");
		if (ref_stack_symbol(st->pcs[i], namebuf, sizeof(namebuf),
		    &offset, M_WAITOK) == 0)
			printf("%s+%#lx", namebuf, offset);
		else
			printf("%p", (void *)st->pcs[i]);
	}
	printf("\n");
}

void
ref_stack_print_ddb(const struct stack *st)
{
	const char *name;
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		ref_stack_symbol_ddb(st->pcs[i], &name, &offset);
		printf("#%d %p at %s+%#lx\n", i, (void *)st->pcs[i],
		    name, offset);
	}
}

#if defined(DDB) || defined(WITNESS)
void
ref_stack_print_short_ddb(const struct stack *st)
{
	const char *name;
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		if (i > 0)
			printf(" ");
		if (ref_stack_symbol_ddb(st->pcs[i], &name, &offset) == 0)
			printf("%s+%#lx", name, offset);
		else
			printf("%p", (void *)st->pcs[i]);
	}
	printf("\n");
}
#endif

/*
 * Format stack into sbuf from live kernel.
 *
 * flags - M_WAITOK or M_NOWAIT (EWOULDBLOCK).
 */
int
ref_stack_sbuf_print_flags(struct sbuf *sb, const struct stack *st, int flags,
    enum stack_sbuf_fmt format)
{
	char namebuf[64];
	long offset;
	int i, error;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		error = ref_stack_symbol(st->pcs[i], namebuf, sizeof(namebuf),
		    &offset, flags);
		if (error == EWOULDBLOCK)
			return (error);
		switch (format) {
		case STACK_SBUF_FMT_LONG:
			sbuf_printf(sb, "#%d %p at %s+%#lx\n", i,
			    (void *)st->pcs[i], namebuf, offset);
			break;
		case STACK_SBUF_FMT_COMPACT:
			sbuf_printf(sb, "%s+%#lx ", namebuf, offset);
			break;
		default:
			__assert_unreachable();
		}
	}
	sbuf_nl_terminate(sb);
	return (0);
}

void
ref_stack_sbuf_print(struct sbuf *sb, const struct stack *st)
{

	(void)ref_stack_sbuf_print_flags(sb, st, M_WAITOK, STACK_SBUF_FMT_LONG);
}

#if defined(DDB) || defined(WITNESS)
void
ref_stack_sbuf_print_ddb(struct sbuf *sb, const struct stack *st)
{
	const char *name;
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		(void)ref_stack_symbol_ddb(st->pcs[i], &name, &offset);
		sbuf_printf(sb, "#%d %p at %s+%#lx\n", i, (void *)st->pcs[i],
		    name, offset);
	}
}
#endif

#ifdef KTR
void
ref_stack_ktr(u_int mask, const char *file, int line, const struct stack *st,
    u_int depth)
{
#ifdef DDB
	const char *name;
	long offset;
	int i;
#endif

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
#ifdef DDB
	if (depth == 0 || st->depth < depth)
		depth = st->depth;
	for (i = 0; i < depth; i++) {
		(void)ref_stack_symbol_ddb(st->pcs[i], &name, &offset);
		ktr_tracepoint(mask, file, line, "#%d %p at %s+%#lx",
		    i, st->pcs[i], (u_long)name, offset, 0, 0);
	}
#endif
}
#endif

/*
 * Two variants of stack symbol lookup -- one that uses the DDB interfaces
 * and bypasses linker locking, and the other that doesn't.
 */
int
ref_stack_symbol(vm_offset_t pc, char *namebuf, u_int buflen, long *offset,
    int flags)
{
	int error;

	error = linker_search_symbol_name_flags((caddr_t)pc, namebuf, buflen,
	    offset, flags);
	if (error == 0 || error == EWOULDBLOCK)
		return (error);

	*offset = 0;
	strlcpy(namebuf, "??", buflen);
	return (ENOENT);
}

int
ref_stack_symbol_ddb(vm_offset_t pc, const char **name, long *offset)
{
	linker_symval_t symval;
	c_linker_sym_t sym;

	if (linker_ddb_search_symbol((caddr_t)pc, &sym, offset) != 0)
		goto out;
	if (linker_ddb_symbol_values(sym, &symval) != 0)
		goto out;
	if (symval.name != NULL) {
		*name = symval.name;
		return (0);
	}
 out:
	*offset = 0;
	*name = "??";
	return (ENOENT);
}

long oracle_tslog_nrecs(void)
{
	return (nrecs);
}

int oracle_tslog_get(long i, void **td, int *type, const char **f,
    const char **s, uint64_t *tsc)
{
	if (i < 0 || i >= (long)nitems(timestamps))
		return (0);
	*td = timestamps[i].td;
	*type = timestamps[i].type;
	*f = timestamps[i].f;
	*s = timestamps[i].s;
	*tsc = timestamps[i].tsc;
	return (1);
}

int oracle_tslog_user_get(pid_t pid, pid_t *ppid, uint64_t *tf, uint64_t *te,
    const char **ex, const char **nm, int *reused)
{
	if (pid < 0 || pid > PID_MAX)
		return (0);
	*ppid = procs[pid].ppid;
	*tf = procs[pid].tsc_forked;
	*te = procs[pid].tsc_exited;
	*ex = procs[pid].execname;
	*nm = procs[pid].namei;
	*reused = procs[pid].reused;
	return (1);
}
