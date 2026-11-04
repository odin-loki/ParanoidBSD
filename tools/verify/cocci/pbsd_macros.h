/* Macro hints for Coccinelle's C parser - FreeBSD's spellings of the
 * attributes that make a declaration unparseable if they are unknown.
 * Coccinelle ships standard.h for the GNU spellings; these are the ones
 * sys/sys/cdefs.h adds on top, plus the kernel's declaration macros. */
#define __unused
#define __used
#define __packed
#define __dead2
#define __pure
#define __pure2
#define __nonnull(x)
#define __nonnull_all
#define __returns_twice
#define __aligned(x)
#define __section(x)
#define __alloc_size(x)
#define __alloc_size2(n,x)
#define __alloc_align(x)
#define __result_use_check
#define __result_use_or_ignore
#define __printflike(a,b)
#define __printf0like(a,b)
#define __scanflike(a,b)
#define __format_arg(a)
#define __strfmonlike(a,b)
#define __strftimelike(a,b)
#define __read_mostly
#define __read_frequently
#define __exclusive_cache_line
#define __noinline
#define __always_inline inline
#define __noreturn
#define __nosanitizeaddress
#define __nosanitizememory
#define __nosanitizethread
#define __no_lock_analysis
#define __nodiscard
#define __weak_symbol
#define __malloc_like
#define __min_size(x)
#define __restrict
#define __volatile volatile
#define __asm asm
#define __inline inline
#define __const const
#define __signed signed
#define __has_c_attribute(x) 0
#define _Nonnull
#define _Nullable
#define _Null_unspecified
#define __guarded_by(x)
#define __pt_guarded_by(x)
#define __locks_exclusive(...)
#define __locks_shared(...)
#define __trylocks_exclusive(...)
#define __trylocks_shared(...)
#define __unlocks(...)
#define __asserts_exclusive(...)
#define __asserts_shared(...)
#define __requires_exclusive(...)
#define __requires_shared(...)
#define __requires_unlocked(...)
#define __no_lock_analysis
#define __lock_annotate(x)
#define __CONCAT(a,b) a##b
#define __XSTRING(x) #x
#define __STRING(x) #x
#define __DECONST(t,v) ((t)(v))
#define __DEVOLATILE(t,v) ((t)(v))
#define __DEQUALIFY(t,v) ((t)(v))
