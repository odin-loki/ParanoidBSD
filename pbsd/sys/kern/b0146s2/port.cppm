// PBSD port of HardenedBSD sys/kern/subr_efi_map.c (batch b0146s2).

module;

#include <cstddef>
#include <cstdint>
#include <cstdio>

export module pbsd.sys.kern.b0146s2;

namespace pbsd::sys_kern::b0146s2::detail {

#define __unused __attribute__((__unused__))
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))

#define EFI_PAGE_SIZE 4096
#define EXFLAG_NOALLOC 0x0001

#define EFI_MD_TYPE_RECLAIM 9
#define EFI_MD_TYPE_RT_CODE 5
#define EFI_MD_TYPE_RT_DATA 6
#define EFI_MD_TYPE_CODE 1
#define EFI_MD_TYPE_DATA 2
#define EFI_MD_TYPE_BS_CODE 3
#define EFI_MD_TYPE_BS_DATA 4
#define EFI_MD_TYPE_FREE 7

#define EFI_MD_ATTR_UC 0x0000000000000001UL
#define EFI_MD_ATTR_WC 0x0000000000000002UL
#define EFI_MD_ATTR_WT 0x0000000000000004UL
#define EFI_MD_ATTR_WB 0x0000000000000008UL
#define EFI_MD_ATTR_UCE 0x0000000000000010UL
#define EFI_MD_ATTR_WP 0x0000000000001000UL
#define EFI_MD_ATTR_RP 0x0000000000002000UL
#define EFI_MD_ATTR_XP 0x0000000000004000UL
#define EFI_MD_ATTR_NV 0x0000000000008000UL
#define EFI_MD_ATTR_MORE_RELIABLE 0x0000000000010000UL
#define EFI_MD_ATTR_RO 0x0000000000020000UL
#define EFI_MD_ATTR_RT 0x8000000000000000UL

struct efi_map_header {
	std::uint64_t memory_size;
	std::uint64_t descriptor_size;
	std::uint32_t descriptor_version;
};

struct efi_md {
	std::uint32_t md_type;
	std::uint32_t md_pad;
	std::uint64_t md_phys;
	std::uint64_t md_virt;
	std::uint64_t md_pages;
	std::uint64_t md_attr;
};

using efi_map_entry_cb = void (*)(efi_md *p, void *argp);

inline efi_md *
efi_next_descriptor(efi_md *md, std::size_t size)
{
	return (reinterpret_cast<efi_md *>(reinterpret_cast<std::uint8_t *>(md) +
	    size));
}

extern "C" void pbsd_physmem_hardware_region(std::uint64_t pa,
    std::uint64_t sz);
extern "C" void pbsd_physmem_exclude_region(std::uint64_t pa, std::uint64_t sz,
    std::uint32_t flags);
extern "C" int pbsd_kern_printf(const char *fmt, ...);

} // namespace pbsd::sys_kern::b0146s2::detail

export namespace pbsd::sys_kern::b0146s2 {

using detail::efi_map_entry_cb;
using detail::efi_map_header;
using detail::efi_md;

#define __unused __attribute__((__unused__))
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))
#define EFI_PAGE_SIZE 4096
#define EXFLAG_NOALLOC 0x0001
#define EFI_MD_TYPE_RECLAIM 9
#define EFI_MD_TYPE_RT_CODE 5
#define EFI_MD_TYPE_RT_DATA 6
#define EFI_MD_TYPE_CODE 1
#define EFI_MD_TYPE_DATA 2
#define EFI_MD_TYPE_BS_CODE 3
#define EFI_MD_TYPE_BS_DATA 4
#define EFI_MD_TYPE_FREE 7
#define EFI_MD_ATTR_UC 0x0000000000000001UL
#define EFI_MD_ATTR_WC 0x0000000000000002UL
#define EFI_MD_ATTR_WT 0x0000000000000004UL
#define EFI_MD_ATTR_WB 0x0000000000000008UL
#define EFI_MD_ATTR_UCE 0x0000000000000010UL
#define EFI_MD_ATTR_WP 0x0000000000001000UL
#define EFI_MD_ATTR_RP 0x0000000000002000UL
#define EFI_MD_ATTR_XP 0x0000000000004000UL
#define EFI_MD_ATTR_NV 0x0000000000008000UL
#define EFI_MD_ATTR_MORE_RELIABLE 0x0000000000010000UL
#define EFI_MD_ATTR_RO 0x0000000000020000UL
#define EFI_MD_ATTR_RT 0x8000000000000000UL

/*
 * Copyright (c) 2014 The FreeBSD Foundation
 * Copyright (c) 2018 Andrew Turner
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

void
efi_map_foreach_entry(efi_map_header *efihdr, efi_map_entry_cb cb, void *argp)
{
	efi_md *map, *p;
	std::size_t efisz;
	int ndesc, i;

	/*
	 * Memory map data provided by UEFI via the GetMemoryMap
	 * Boot Services API.
	 */
	efisz = (sizeof(efi_map_header) + 0xf) & ~0xf;
	map = reinterpret_cast<efi_md *>(reinterpret_cast<std::uint8_t *>(efihdr) +
	    efisz);

	if (efihdr->descriptor_size == 0)
		return;
	ndesc = static_cast<int>(efihdr->memory_size / efihdr->descriptor_size);

	for (i = 0, p = map; i < ndesc; i++,
	    p = detail::efi_next_descriptor(p, efihdr->descriptor_size)) {
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
static void
handle_efi_map_entry(efi_md *p, void *argp)
{
	bool exclude = *static_cast<bool *>(argp);

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
			detail::pbsd_physmem_exclude_region(p->md_phys,
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
			detail::pbsd_physmem_hardware_region(p->md_phys,
			    p->md_pages * EFI_PAGE_SIZE);
		break;
	default:
		/* Other types shall not be handled by physmem. */
		break;
	}
}

void
efi_map_add_entries(efi_map_header *efihdr)
{
	bool exclude = false;
	efi_map_foreach_entry(efihdr, handle_efi_map_entry, &exclude);
}

void
efi_map_exclude_entries(efi_map_header *efihdr)
{
	bool exclude = true;
	efi_map_foreach_entry(efihdr, handle_efi_map_entry, &exclude);
}

static void
print_efi_map_entry(efi_md *p, void *argp __unused)
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
	detail::pbsd_kern_printf("%23s %012jx %012jx %08jx ", type,
	    static_cast<std::uintmax_t>(p->md_phys),
	    static_cast<std::uintmax_t>(p->md_virt),
	    static_cast<std::uintmax_t>(p->md_pages));
	if (p->md_attr & EFI_MD_ATTR_UC)
		detail::pbsd_kern_printf("UC ");
	if (p->md_attr & EFI_MD_ATTR_WC)
		detail::pbsd_kern_printf("WC ");
	if (p->md_attr & EFI_MD_ATTR_WT)
		detail::pbsd_kern_printf("WT ");
	if (p->md_attr & EFI_MD_ATTR_WB)
		detail::pbsd_kern_printf("WB ");
	if (p->md_attr & EFI_MD_ATTR_UCE)
		detail::pbsd_kern_printf("UCE ");
	if (p->md_attr & EFI_MD_ATTR_WP)
		detail::pbsd_kern_printf("WP ");
	if (p->md_attr & EFI_MD_ATTR_RP)
		detail::pbsd_kern_printf("RP ");
	if (p->md_attr & EFI_MD_ATTR_XP)
		detail::pbsd_kern_printf("XP ");
	if (p->md_attr & EFI_MD_ATTR_NV)
		detail::pbsd_kern_printf("NV ");
	if (p->md_attr & EFI_MD_ATTR_MORE_RELIABLE)
		detail::pbsd_kern_printf("MORE_RELIABLE ");
	if (p->md_attr & EFI_MD_ATTR_RO)
		detail::pbsd_kern_printf("RO ");
	if (p->md_attr & EFI_MD_ATTR_RT)
		detail::pbsd_kern_printf("RUNTIME");
	detail::pbsd_kern_printf("\n");
}

void
efi_map_print_entries(efi_map_header *efihdr)
{

	detail::pbsd_kern_printf("%23s %12s %12s %8s %4s\n",
	    "Type", "Physical", "Virtual", "#Pages", "Attr");
	efi_map_foreach_entry(efihdr, print_efi_map_entry, nullptr);
}

} // namespace pbsd::sys_kern::b0146s2
