/* Common kernel typedefs for the Frama-C contract stubs (see frama_gen.awk). Standard names come from Frama-C's own libc headers; kernel names are typedef'd
   to the usual fixed-width types so that ACSL typing of integer expressions is realistic. */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
typedef unsigned char u8; typedef unsigned short u16; typedef unsigned int u32; typedef unsigned long long u64;
typedef signed char s8; typedef short s16; typedef int s32; typedef long long s64;
typedef unsigned char __u8; typedef unsigned short __u16; typedef unsigned int __u32; typedef unsigned long long __u64;
typedef signed char __s8; typedef short __s16; typedef int __s32; typedef long long __s64;
typedef unsigned short __le16; typedef unsigned int __le32; typedef unsigned long long __le64;
typedef unsigned short __be16; typedef unsigned int __be32; typedef unsigned long long __be64;
typedef unsigned int uint; typedef unsigned long ulong; typedef long long loff_t;   /* ssize_t, off_t, size_t, uintptr_t ... come from Frama-C's libc */
typedef unsigned int gfp_t; typedef unsigned long long dma_addr_t; typedef unsigned long long phys_addr_t;
/* additions for the pilot: common kernel/BSD names */
typedef unsigned char uchar; typedef unsigned short ushort; typedef unsigned char u_char; typedef unsigned short u_short; typedef unsigned int u_int; typedef unsigned long u_long;
typedef unsigned long long u_quad_t; typedef long long quad_t;
typedef int atomic_t; typedef int bool_t;
#ifndef NULL
#define NULL ((void*)0)
#endif
#define __iomem
#define __user
#define __force
#define __rcu
#define __must_check
#define __init
#define __exit
#define __always_inline inline
#define __inline inline
#define __unused
#define __packed
#define __read_mostly
#define __maybe_unused
#define __cold
#define notrace
#define EXPORT_SYMBOL(x)
#define EXPORT_SYMBOL_GPL(x)
#define BIT(n) (1UL << (n))
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define EINVAL 22
#define ENOMEM 12
#define EIO 5
#define ENODEV 19
#define EBUSY 16
#define ENOENT 2
#define EFAULT 14
#define EAGAIN 11
#define ERANGE 34
#define EOPNOTSUPP 95
#define ENOSPC 28
#define ETIMEDOUT 110
#define EPERM 1
#define EEXIST 17
#define EOVERFLOW 75
