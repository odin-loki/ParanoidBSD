/* s3_shim.h   Just enough of the kernel environments (Linux and FreeBSD) for small pure helpers to compile on their own, for step 3 random testing. */
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef uint64_t u64;
typedef int8_t s8; typedef int16_t s16; typedef int32_t s32; typedef int64_t s64;
typedef uint8_t __u8; typedef uint16_t __u16; typedef uint32_t __u32; typedef uint64_t __u64;
typedef int8_t __s8; typedef int16_t __s16; typedef int32_t __s32; typedef int64_t __s64;
typedef unsigned long ulong;
#define likely(x) (x)
#define unlikely(x) (x)
#define __predict_true(x) (x)
#define __predict_false(x) (x)
#define __always_inline inline
#define noinline
#define notrace
#define __init
#define __exit
#define __cold
#define __hot
#define __pure
#define __attribute_const__
#define __maybe_unused
#define __unused
#define __must_check
#define __read_mostly
#define BIT(n) (1UL << (n))
#define BIT_ULL(n) (1ULL << (n))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min_t(t, a, b) ((t)(a) < (t)(b) ? (t)(a) : (t)(b))
#define max_t(t, a, b) ((t)(a) > (t)(b) ? (t)(a) : (t)(b))
#define clamp(v, lo, hi) min(max(v, lo), hi)
#ifndef MIN
#define MIN(a, b) min(a, b)
#define MAX(a, b) max(a, b)
#endif
#define WARN_ON(x) (!!(x))
#define WARN_ON_ONCE(x) (!!(x))
#define BUG_ON(x) do { if (x) __builtin_trap(); } while (0)
#define KASSERT(x, m) do { } while (0)
#define MPASS(x) do { } while (0)
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
