// PBSD port of HardenedBSD sys/kern batch b0217.

module;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

export module pbsd.sys.kern.b0217;

namespace pbsd::sys_kern::b0217::detail {

#define __unused __attribute__((__unused__))
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CTASSERT(x) typedef char __ctassert[(x) ? 1 : -1] __attribute__((__unused__))

#define M_NOWAIT 0x0001
#define M_WAITOK 0x0002
#define M_ZERO   0x0100
#define HASH_WAITOK 0x00000001
#define HASH_NOWAIT 0x00000002
#define EINVAL 22
#define ENOENT 2
#define EWOULDBLOCK 35
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

using u_int = unsigned int;
using u_long = unsigned long;
using u_char = unsigned char;
using vm_offset_t = std::uintptr_t;
using caddr_t = char *;
using pid_t = int;

enum stack_sbuf_fmt {
	STACK_SBUF_FMT_NONE = 0,
	STACK_SBUF_FMT_LONG = 1,
	STACK_SBUF_FMT_COMPACT = 2,
};

#define LIST_HEAD(name, type) struct name { struct type *lh_first; }
#define LIST_ENTRY(type) struct { struct type *le_next; struct type **le_prev; }
#define LIST_FIRST(head) ((head)->lh_first)
#define LIST_NEXT(elm, field) ((elm)->field.le_next)
#define LIST_EMPTY(head) (LIST_FIRST(head) == NULL)
#define LIST_INIT(head) do { LIST_FIRST(head) = nullptr; } while (0)
#define LIST_FOREACH(var, head, field) for ((var) = LIST_FIRST(head); (var); (var) = LIST_NEXT(var, field))
#define LIST_INSERT_HEAD(head, elm, field) do { \
	if (((elm)->field.le_next = LIST_FIRST(head)) != nullptr) \
		LIST_FIRST(head)->field.le_prev = &(elm)->field.le_next; \
	LIST_FIRST(head) = (elm); \
	(elm)->field.le_prev = &LIST_FIRST(head); \
} while (0)
#define LIST_REMOVE(elm, field) do { \
	if ((elm)->field.le_next != nullptr) \
		(elm)->field.le_next->field.le_prev = (elm)->field.le_prev; \
	*(elm)->field.le_prev = (elm)->field.le_next; \
} while (0)

struct malloc_type { const char *ks_shortdesc; };
#define MALLOC_DEFINE_STUB(type, shortdesc, longdesc) \
	inline malloc_type type##_mt{ shortdesc }; \
	inline malloc_type *const type = &type##_mt

struct thread { char td_pad; };
struct mount { u_int mnt_hashseed; };
struct vnode {
	u_int v_hash;
	mount *v_mount;
	LIST_ENTRY(vnode) v_hashlist;
	int v_hold;
	int v_refs;
	int v_gone;
};
enum vgetstate { VGET_NONE, VGET_HOLDCNT, VGET_USECOUNT };
using vfs_hash_cmp_t = int(struct vnode *, void *);

struct rwlock { int lk; const char *name; };
struct timecounter {
	u_int (*tc_get_timecount)(timecounter *);
	u_int tc_counter_mask;
	std::uint64_t tc_frequency;
	const char *tc_name;
};
struct stack { int depth; vm_offset_t pcs[STACK_MAX]; };
struct sbuf {
	char *s_buf;
	std::size_t s_size;
	std::size_t s_len;
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
using c_linker_sym_t = void *;
struct linker_symval_t { const char *name; };

inline u_long desiredvnodes = 64;
inline int bootverbose = 0;
inline thread thread0{};
inline thread *curthread = &thread0;
inline timecounter *g_timecounter = nullptr;

#ifndef TSLOGSIZE
#define TSLOGSIZE 4096
#endif

enum {
	MODEL_GUARD = 0x7f,
	MODEL_ARENA = 262144,
	MODEL_OUT = 262144,
	MODEL_LOG = 16384,
};

struct model_ev { int op; unsigned long long a, b, c, d; };
inline model_ev model_log[MODEL_LOG];
inline int model_log_n;
inline char model_out[MODEL_OUT];
inline std::size_t model_out_n;
inline unsigned char model_arena[MODEL_ARENA];
inline std::size_t model_arena_off;
inline int model_malloc_fail;
inline unsigned long model_malloc_max = MODEL_ARENA;
inline std::uint64_t model_cycle;
inline int model_vget_error;
inline int model_vget_enoent;
inline int model_linker_fail;
inline int model_linker_block;
inline int model_sbuf_fail;
inline int model_ktr_n;

inline unsigned long long model_strhash(const char *s) {
	unsigned long long h = 1469598103934665603ULL;
	if (!s) return 0;
	while (*s) { h ^= (unsigned char)*s++; h *= 1099511628211ULL; }
	return h;
}
inline void model_ev(int op, unsigned long long a, unsigned long long b,
    unsigned long long c, unsigned long long d) {
	if (model_log_n < MODEL_LOG)
		model_log[model_log_n++] = {op, a, b, c, d};
}
inline void model_assert(int c) { model_ev(1, c ? 1ULL : 0ULL, 0, 0, 0); }
#define KASSERT(e, m) detail::model_assert((e) ? 1 : 0)
inline void model_bzero(void *p, unsigned long len) {
	model_ev(2, len, 0, 0, 0); std::memset(p, 0, (std::size_t)len);
}
inline void *model_kmalloc(unsigned long size, malloc_type *type, int flags) {
	unsigned long need = (size + 15UL) & ~15UL;
	void *ret;
	if (model_malloc_fail || size > model_malloc_max || model_arena_off + need > MODEL_ARENA) {
		model_ev(3, size, flags, ~0ULL, model_strhash(type ? type->ks_shortdesc : nullptr));
		return nullptr;
	}
	ret = &model_arena[model_arena_off];
	model_ev(3, size, flags, model_arena_off, model_strhash(type ? type->ks_shortdesc : nullptr));
	model_arena_off += need;
	return ret;
}
inline void model_kfree(void *p, malloc_type *type) {
	unsigned long long off = ~0ULL;
	if (p && (unsigned char *)p >= model_arena && (unsigned char *)p < model_arena + MODEL_ARENA)
		off = (unsigned char *)p - model_arena;
	model_ev(4, off, 0, 0, model_strhash(type ? type->ks_shortdesc : nullptr));
}
inline int model_printf(const char *fmt, ...) {
	char tmp[1024]; va_list ap; va_start(ap, fmt);
	int n = std::vsnprintf(tmp, sizeof(tmp), fmt, ap); va_end(ap);
	if (n > 0) {
		std::size_t len = (std::size_t)n;
		if (len > sizeof(tmp) - 1) len = sizeof(tmp) - 1;
		if (model_out_n + len + 1 < MODEL_OUT) {
			std::memcpy(model_out + model_out_n, tmp, len);
			model_out_n += len; model_out[model_out_n] = '\0';
		}
	}
	model_ev(5, (unsigned long long)n, 0, 0, 0);
	return n;
}
inline int model_hash_mflags(int flags) { return (flags & HASH_NOWAIT) ? M_NOWAIT : M_WAITOK; }
struct generic_hash_head { vnode *lh_first; };
inline void *hashinit(int elements, malloc_type *type, u_long *hashmask) {
	long hashsize, i;
	generic_hash_head *hashtbl;
	KASSERT(elements > 0, ("bad"));
	for (hashsize = 1; hashsize <= elements; hashsize <<= 1) continue;
	hashsize >>= 1;
	hashtbl = (generic_hash_head *)model_kmalloc(
	    (unsigned long)hashsize * sizeof(*hashtbl), type, model_hash_mflags(HASH_WAITOK));
	if (hashtbl) { for (i = 0; i < hashsize; i++) LIST_INIT(&hashtbl[i]); *hashmask = (u_long)(hashsize - 1); }
	return hashtbl;
}
inline void rw_init(rwlock *rw, const char *name) { rw->lk = 0; rw->name = name; model_ev(10,0,0,0,model_strhash(name)); }
inline void rw_rlock(rwlock *rw) { rw->lk++; model_ev(11, rw->lk, 0, 0, 0); }
inline void rw_runlock(rwlock *rw) { rw->lk--; model_ev(12, rw->lk, 0, 0, 0); }
inline void rw_wlock(rwlock *rw) { rw->lk = -1; model_ev(13, 0, 0, 0, 0); }
inline void rw_wunlock(rwlock *rw) { rw->lk = 0; model_ev(14, 0, 0, 0, 0); }
inline vgetstate vget_prep(vnode *) { model_ev(20,0,0,0,0); return VGET_NONE; }
inline int vget_finish(vnode *, int, vgetstate) {
	if (model_vget_enoent) { model_ev(21, ENOENT, 0, 0, 0); return ENOENT; }
	if (model_vget_error) { model_ev(21, model_vget_error, 0, 0, 0); return model_vget_error; }
	model_ev(21, 0, 0, 0, 0); return 0;
}
inline void vhold(vnode *vp) { vp->v_hold++; model_ev(22, vp->v_hold, 0, 0, 0); }
inline void vref(vnode *vp) { vp->v_refs++; model_ev(23, vp->v_refs, 0, 0, 0); }
inline void vdrop(vnode *vp) { vp->v_hold--; model_ev(24, vp->v_hold, 0, 0, 0); }
inline void vput(vnode *vp) { vp->v_refs--; model_ev(25, vp->v_refs, 0, 0, 0); }
inline void vgone(vnode *vp) { vp->v_gone = 1; model_ev(26, 0, 0, 0, 0); }
#define ASSERT_VOP_ELOCKED(vp, msg) KASSERT(1, (msg))
inline std::uint64_t get_cyclecount() { model_cycle++; model_ev(30, model_cycle, 0, 0, 0); return model_cycle; }
inline long atomic_fetchadd_long(volatile long *p, long v) {
	long o = *p; *p += v; model_ev(31, o, v, 0, 0); return o;
}
template<typename T> inline T atomic_load_ptr(T *p) { return *p; }
inline void cpu_spinwait() { model_ev(32, 0, 0, 0, 0); }
#define TSENTER() ((void)0)
#define TSEXIT() ((void)0)
inline caddr_t preload_search_by_type(const char *) { return nullptr; }
inline void *preload_fetch_addr(caddr_t) { return nullptr; }
inline std::size_t preload_fetch_size(caddr_t) { return 0; }
inline sbuf *sbuf_new_for_sysctl(sbuf *, char *, int, sysctl_req *) {
	sbuf *sb = (sbuf *)model_kmalloc(sizeof(*sb), nullptr, M_WAITOK);
	if (sb) { sb->s_size = 8192; sb->s_buf = (char *)model_kmalloc(sb->s_size, nullptr, M_WAITOK);
		sb->s_len = 0; sb->s_error = 0; if (sb->s_buf) sb->s_buf[0] = '\0'; }
	return sb;
}
inline int sbuf_bcat(sbuf *sb, const void *buf, std::size_t n) {
	if (!sb || !sb->s_buf || sb->s_len + n >= sb->s_size) return -1;
	std::memcpy(sb->s_buf + sb->s_len, buf, n); sb->s_len += n; sb->s_buf[sb->s_len] = 0; return 0;
}
inline int sbuf_printf(sbuf *sb, const char *fmt, ...) {
	char tmp[512]; va_list ap; va_start(ap, fmt);
	int n = std::vsnprintf(tmp, sizeof(tmp), fmt, ap); va_end(ap);
	return sbuf_bcat(sb, tmp, (std::size_t)n);
}
inline int sbuf_cat(sbuf *sb, const char *s) { return sbuf_bcat(sb, s, std::strlen(s)); }
inline int sbuf_putc(sbuf *sb, int c) { char ch = (char)c; return sbuf_bcat(sb, &ch, 1); }
inline int sbuf_finish(sbuf *sb) { (void)sb; return model_sbuf_fail ? EINVAL : 0; }
inline void sbuf_delete(sbuf *sb) {
	if (sb) { if (sb->s_buf) model_kfree(sb->s_buf, nullptr); model_kfree(sb, nullptr); }
}
inline void sbuf_nl_terminate(sbuf *sb) {
	if (sb && sb->s_len > 0 && sb->s_buf[sb->s_len - 1] != '\n') sbuf_putc(sb, '\n');
}
inline char *kernel_strdup(const char *s, malloc_type *type) {
	std::size_t n = std::strlen(s) + 1; char *p = (char *)model_kmalloc(n, type, M_WAITOK);
	if (p) std::memcpy(p, s, n); return p;
}
inline std::size_t strlcpy(char *dst, const char *src, std::size_t sz) {
	std::size_t n = std::strlen(src);
	if (sz) { std::size_t c = n < sz - 1 ? n : sz - 1; std::memcpy(dst, src, c); dst[c] = 0; }
	return n;
}
inline int linker_search_symbol_name_flags(caddr_t pc, char *namebuf, u_int buflen, long *offset, int flags) {
	model_ev(40, (unsigned long long)(uintptr_t)pc, 0, 0, 0);
	if (model_linker_block && (flags & M_NOWAIT)) return EWOULDBLOCK;
	if (model_linker_fail) { *offset = 0; strlcpy(namebuf, "??", buflen); return ENOENT; }
	std::snprintf(namebuf, buflen, "sym_%lx", (unsigned long)(uintptr_t)pc);
	*offset = (long)((uintptr_t)pc & 0xff); return 0;
}
inline int linker_ddb_search_symbol(caddr_t pc, c_linker_sym_t *sym, long *offset) {
	*sym = (c_linker_sym_t)(uintptr_t)pc; *offset = (long)((uintptr_t)pc & 0xff); return 0;
}
inline int linker_ddb_symbol_values(c_linker_sym_t sym, linker_symval_t *sv) {
	static char namebuf[64];
	std::snprintf(namebuf, sizeof(namebuf), "sym_%lx", (unsigned long)(uintptr_t)sym);
	sv->name = namebuf;
	return 0;
}
inline void ktr_tracepoint(u_int, const char *, int, const char *, ...) {
	model_ktr_n++; model_ev(41, model_ktr_n, 0, 0, 0);
}
#define __assert_unreachable() std::abort()
#define bzero(p, n) detail::model_bzero((p), (unsigned long)(n))
#define malloc(s, t, f) detail::model_kmalloc((unsigned long)(s), (t), (f))
#define free(p, t) detail::model_kfree((p), (t))
#define printf(...) detail::model_printf(__VA_ARGS__)
#define strdup(s, t) detail::kernel_strdup((s), (t))

inline void env_reset() {
	model_log_n = 0; model_out_n = 0; model_out[0] = 0;
	model_arena_off = 0; std::memset(model_arena, MODEL_GUARD, MODEL_ARENA);
	model_malloc_fail = 0; model_vget_error = 0; model_vget_enoent = 0;
	model_linker_fail = 0; model_linker_block = 0; model_sbuf_fail = 0;
	model_ktr_n = 0; model_cycle = 0;
}

} // namespace detail

export namespace pbsd::sys_kern::b0217 {

using namespace detail;

#define __unused __attribute__((__unused__))
#ifndef __exclusive_cache_line
#define __exclusive_cache_line
#endif
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MALLOC_DEFINE_STUB(type, shortdesc, longdesc) 	inline malloc_type type##_mt{ shortdesc }; 	inline malloc_type *const type = &type##_mt
#define KASSERT(e, m) detail::model_assert((e) ? 1 : 0)
#define bzero(p, n) detail::model_bzero((p), (unsigned long)(n))
#define malloc(s, t, f) detail::model_kmalloc((unsigned long)(s), (t), (f))
#define free(p, t) detail::model_kfree((p), (t))
#define printf(...) detail::model_printf(__VA_ARGS__)
#define strdup(s, t) detail::kernel_strdup((s), (t))
#define atomic_load_ptr(p) (*(p))

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


MALLOC_DEFINE_STUB(M_VFS_HASH, "vfs_hash", "VFS hash table");

static LIST_HEAD(vfs_hash_head, vnode)	*vfs_hash_tbl;
static LIST_HEAD(,vnode)		vfs_hash_side;
static u_long				vfs_hash_mask;
static struct rwlock __exclusive_cache_line vfs_hash_lock;

void
vfs_hashinit(void *dummy __unused)
{

	vfs_hash_tbl = (vfs_hash_head *)hashinit(desiredvnodes, M_VFS_HASH, &vfs_hash_mask);
	rw_init(&vfs_hash_lock, "vfs hash");
	LIST_INIT(&vfs_hash_side);
}


u_int
vfs_hash_index(struct vnode *vp)
{

	return (vp->v_hash + vp->v_mount->mnt_hashseed);
}

static struct vfs_hash_head *
vfs_hash_bucket(const struct mount *mp, u_int hash)
{

	return (&vfs_hash_tbl[(hash + mp->mnt_hashseed) & vfs_hash_mask]);
}

int
vfs_hash_get(const struct mount *mp, u_int hash, int flags, struct thread *td,
    struct vnode **vpp, vfs_hash_cmp_t *fn, void *arg)
{
	struct vnode *vp;
	enum vgetstate vs;
	int error;

	while (1) {
		rw_rlock(&vfs_hash_lock);
		LIST_FOREACH(vp, vfs_hash_bucket(mp, hash), v_hashlist) {
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
vfs_hash_ref(const struct mount *mp, u_int hash, struct thread *td,
    struct vnode **vpp, vfs_hash_cmp_t *fn, void *arg)
{
	struct vnode *vp;

	while (1) {
		rw_rlock(&vfs_hash_lock);
		LIST_FOREACH(vp, vfs_hash_bucket(mp, hash), v_hashlist) {
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
vfs_hash_remove(struct vnode *vp)
{

	rw_wlock(&vfs_hash_lock);
	LIST_REMOVE(vp, v_hashlist);
	rw_wunlock(&vfs_hash_lock);
}

int
vfs_hash_insert(struct vnode *vp, u_int hash, int flags, struct thread *td,
    struct vnode **vpp, vfs_hash_cmp_t *fn, void *arg)
{
	struct vnode *vp2;
	enum vgetstate vs;
	int error;

	*vpp = NULL;
	while (1) {
		rw_wlock(&vfs_hash_lock);
		LIST_FOREACH(vp2,
		    vfs_hash_bucket(vp->v_mount, hash), v_hashlist) {
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
	LIST_INSERT_HEAD(vfs_hash_bucket(vp->v_mount, hash), vp, v_hashlist);
	rw_wunlock(&vfs_hash_lock);
	return (0);
}

void
vfs_hash_rehash(struct vnode *vp, u_int hash)
{
	ASSERT_VOP_ELOCKED(vp, "rehash requires excl lock");

	rw_wlock(&vfs_hash_lock);
	LIST_REMOVE(vp, v_hashlist);
	LIST_INSERT_HEAD(vfs_hash_bucket(vp->v_mount, hash), vp, v_hashlist);
	vp->v_hash = hash;
	rw_wunlock(&vfs_hash_lock);
}

void
vfs_hash_changesize(u_long newmaxvnodes)
{
	struct vfs_hash_head *vfs_hash_newtbl, *vfs_hash_oldtbl;
	u_long vfs_hash_newmask, vfs_hash_oldmask;
	struct vnode *vp;
	int i;

	vfs_hash_newtbl = (vfs_hash_head *)hashinit(newmaxvnodes, M_VFS_HASH,
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
			    vfs_hash_bucket(vp->v_mount, vp->v_hash),
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

volatile long nrecs = 0;
static struct timestamp {
	void * td;
	int type;
	const char * f;
	const char * s;
	uint64_t tsc;
} timestamps[TSLOGSIZE];

void
tslog(void * td, int type, const char * f, const char * s)
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
sysctl_debug_tslog(SYSCTL_HANDLER_ARGS)
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
	 * tslog() takes to write a record, it is highly unlikely that
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


MALLOC_DEFINE_STUB(M_TSLOGUSER, "tsloguser", "Strings used by userland tslog");
static struct procdata {
	pid_t ppid;
	uint64_t tsc_forked;
	uint64_t tsc_exited;
	char * execname;
	char * namei;
	int reused;
} procs[PID_MAX + 1];

void
tslog_user(pid_t pid, pid_t ppid, const char * execname, const char * namei)
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
sysctl_debug_tslog_user(SYSCTL_HANDLER_ARGS)
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
sysinit_tslog_shim(const void *data)
{
	const struct sysinit_tslog *x = (const struct sysinit_tslog *)data;

	tslog(curthread, TS_ENTER, "SYSINIT", x->name);
	(x->func)(x->data);
	tslog(curthread, TS_EXIT, "SYSINIT", x->name);
}

void
tslog_reset(void)
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
clockcalib(uint64_t (*clk)(void), const char *clkname)
{
	struct timecounter *tc = atomic_load_ptr(&g_timecounter);
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


MALLOC_DEFINE_STUB(M_STACK, "stack", "Stack Traces");

int stack_symbol(vm_offset_t pc, char *namebuf, u_int buflen,
	    long *offset, int flags);
int stack_symbol_ddb(vm_offset_t pc, const char **name, long *offset);

struct stack *
stack_create(int flags)
{
	struct stack *st;

	st = (stack *)malloc(sizeof(*st), M_STACK, flags | M_ZERO);
	return (st);
}

void
stack_destroy(struct stack *st)
{

	free(st, M_STACK);
}

int
stack_put(struct stack *st, vm_offset_t pc)
{

	if (st->depth < STACK_MAX) {
		st->pcs[st->depth++] = pc;
		return (0);
	} else
		return (-1);
}

void
stack_copy(const struct stack *src, struct stack *dst)
{

	*dst = *src;
}

void
stack_zero(struct stack *st)
{

	bzero(st, sizeof *st);
}

void
stack_print(const struct stack *st)
{
	char namebuf[64];
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		(void)stack_symbol(st->pcs[i], namebuf, sizeof(namebuf),
		    &offset, M_WAITOK);
		printf("#%d %p at %s+%#lx\n", i, (void *)st->pcs[i],
		    namebuf, offset);
	}
}

void
stack_print_short(const struct stack *st)
{
	char namebuf[64];
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		if (i > 0)
			printf(" ");
		if (stack_symbol(st->pcs[i], namebuf, sizeof(namebuf),
		    &offset, M_WAITOK) == 0)
			printf("%s+%#lx", namebuf, offset);
		else
			printf("%p", (void *)st->pcs[i]);
	}
	printf("\n");
}

void
stack_print_ddb(const struct stack *st)
{
	const char *name;
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		stack_symbol_ddb(st->pcs[i], &name, &offset);
		printf("#%d %p at %s+%#lx\n", i, (void *)st->pcs[i],
		    name, offset);
	}
}

#if defined(DDB) || defined(WITNESS)
void
stack_print_short_ddb(const struct stack *st)
{
	const char *name;
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		if (i > 0)
			printf(" ");
		if (stack_symbol_ddb(st->pcs[i], &name, &offset) == 0)
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
stack_sbuf_print_flags(struct sbuf *sb, const struct stack *st, int flags,
    enum stack_sbuf_fmt format)
{
	char namebuf[64];
	long offset;
	int i, error;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		error = stack_symbol(st->pcs[i], namebuf, sizeof(namebuf),
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
stack_sbuf_print(struct sbuf *sb, const struct stack *st)
{

	(void)stack_sbuf_print_flags(sb, st, M_WAITOK, STACK_SBUF_FMT_LONG);
}

#if defined(DDB) || defined(WITNESS)
void
stack_sbuf_print_ddb(struct sbuf *sb, const struct stack *st)
{
	const char *name;
	long offset;
	int i;

	KASSERT(st->depth <= STACK_MAX, ("bogus stack"));
	for (i = 0; i < st->depth; i++) {
		(void)stack_symbol_ddb(st->pcs[i], &name, &offset);
		sbuf_printf(sb, "#%d %p at %s+%#lx\n", i, (void *)st->pcs[i],
		    name, offset);
	}
}
#endif

#ifdef KTR
void
stack_ktr(u_int mask, const char *file, int line, const struct stack *st,
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
		(void)stack_symbol_ddb(st->pcs[i], &name, &offset);
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
stack_symbol(vm_offset_t pc, char *namebuf, u_int buflen, long *offset,
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
stack_symbol_ddb(vm_offset_t pc, const char **name, long *offset)
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



inline void reset_all() {
	detail::env_reset();
	vfs_hashinit(nullptr);
	tslog_reset();
}

using mount = detail::mount;
using vnode = detail::vnode;
using thread = detail::thread;
using stack = detail::stack;
using sbuf = detail::sbuf;
using sysctl_req = detail::sysctl_req;
using sysinit_tslog = detail::sysinit_tslog;
using timecounter = detail::timecounter;
using stack_sbuf_fmt = detail::stack_sbuf_fmt;

inline void malloc_fail_at(int n) { detail::model_malloc_fail = n; }
inline void set_vget_enoent(int v) { detail::model_vget_enoent = v; }
inline void set_vget_error(int v) { detail::model_vget_error = v; }
inline void set_linker_fail(int v) { detail::model_linker_fail = v; }
inline void set_linker_block(int v) { detail::model_linker_block = v; }
inline void set_bootverbose(int v) { detail::bootverbose = v; }
inline void set_timecounter(timecounter *tc) { detail::g_timecounter = tc; }
inline void set_sbuf_fail(int v) { detail::model_sbuf_fail = v; }
inline int ktr_count() { return detail::model_ktr_n; }
inline const char *out_text() { return detail::model_out; }
inline std::size_t out_length() { return detail::model_out_n; }
inline long tslog_record_count() { return nrecs; }

} // namespace
