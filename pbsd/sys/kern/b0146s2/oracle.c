/*
 * PBSD batch b0146s2 -- reference oracle.
 *
 * hbsd/src/sys/kern/subr_efi_map.c concatenated with every function renamed
 * with a "ref_" prefix.  Bodies are otherwise UNMODIFIED.  Kernel environment
 * declarations that the in-tree headers would supply live below.
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
#define	LONG_BIT		(8 * (int)sizeof(long))
#endif
#ifndef NBBY
#define	NBBY			8
#endif
#ifndef __unused
#define	__unused		__attribute__((__unused__))
#endif

#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))

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

/* Kernel services provided by the differential harness. */
void pbsd_physmem_hardware_region(uint64_t pa, uint64_t sz);
void pbsd_physmem_exclude_region(uint64_t pa, uint64_t sz, uint32_t flags);
int pbsd_kern_printf(const char *fmt, ...);

#define	physmem_hardware_region(a, s)	pbsd_physmem_hardware_region((a), (s))
#define	physmem_exclude_region(a, s, f)	pbsd_physmem_exclude_region((a), (s), (f))
#define	printf(...)			pbsd_kern_printf(__VA_ARGS__)

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
