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
