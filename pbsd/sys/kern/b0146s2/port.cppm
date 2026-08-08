/*
 * Copyright (c) 2014 The FreeBSD Foundation
 * Copyright (c) 2018 Andrew Turner
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
/*
 * PBSD port of hbsd/src/sys/kern/subr_efi_map.c (batch b0146s2).
 *
 * The kernel environment this file lived in is not available here, so the
 * declarations that used to come from <sys/efi.h>, <sys/efi_map.h>,
 * <sys/physmem.h> and <machine/efi.h> are reproduced verbatim below.  The
 * kernel printf and the physmem region calls are left as external C symbols
 * so that the surrounding environment supplies them, exactly as the kernel
 * link does.
 */
module;

#include <cstddef>
#include <cstdint>

export module pbsd.sys.kern.b0146s2;

extern "C" {
void physmem_hardware_region(std::uint64_t pa, std::uint64_t sz);
void physmem_exclude_region(std::uint64_t pa, std::uint64_t sz,
    std::uint32_t flags);
int efi_test_printf(const char *fmt, ...);
}

export namespace pbsd::sys_kern::b0146s2 {

using std::size_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;
using std::uintmax_t;

/* <sys/efi.h> */
inline constexpr uint32_t EFI_MD_TYPE_NULL = 0;
inline constexpr uint32_t EFI_MD_TYPE_CODE = 1;		/* Loader text. */
inline constexpr uint32_t EFI_MD_TYPE_DATA = 2;		/* Loader data. */
inline constexpr uint32_t EFI_MD_TYPE_BS_CODE = 3;	/* Boot services text. */
inline constexpr uint32_t EFI_MD_TYPE_BS_DATA = 4;	/* Boot services data. */
inline constexpr uint32_t EFI_MD_TYPE_RT_CODE = 5;	/* Runtime services text. */
inline constexpr uint32_t EFI_MD_TYPE_RT_DATA = 6;	/* Runtime services data. */
inline constexpr uint32_t EFI_MD_TYPE_FREE = 7;		/* Unused/free memory. */
inline constexpr uint32_t EFI_MD_TYPE_BAD = 8;		/* Bad memory. */
inline constexpr uint32_t EFI_MD_TYPE_RECLAIM = 9;	/* ACPI reclaimable memory. */
inline constexpr uint32_t EFI_MD_TYPE_FIRMWARE = 10;	/* ACPI NVS memory. */
inline constexpr uint32_t EFI_MD_TYPE_IOMEM = 11;	/* Memory-mapped I/O. */
inline constexpr uint32_t EFI_MD_TYPE_IOPORT = 12;	/* I/O port space. */
inline constexpr uint32_t EFI_MD_TYPE_PALCODE = 13;	/* Itanium PAL code. */
inline constexpr uint32_t EFI_MD_TYPE_PERSISTENT = 14;	/* Persistent memory. */

inline constexpr uint64_t EFI_MD_ATTR_UC = 0x0000000000000001UL;
inline constexpr uint64_t EFI_MD_ATTR_WC = 0x0000000000000002UL;
inline constexpr uint64_t EFI_MD_ATTR_WT = 0x0000000000000004UL;
inline constexpr uint64_t EFI_MD_ATTR_WB = 0x0000000000000008UL;
inline constexpr uint64_t EFI_MD_ATTR_UCE = 0x0000000000000010UL;
inline constexpr uint64_t EFI_MD_ATTR_WP = 0x0000000000001000UL;
inline constexpr uint64_t EFI_MD_ATTR_RP = 0x0000000000002000UL;
inline constexpr uint64_t EFI_MD_ATTR_XP = 0x0000000000004000UL;
inline constexpr uint64_t EFI_MD_ATTR_NV = 0x0000000000008000UL;
inline constexpr uint64_t EFI_MD_ATTR_MORE_RELIABLE = 0x0000000000010000UL;
inline constexpr uint64_t EFI_MD_ATTR_RO = 0x0000000000020000UL;
inline constexpr uint64_t EFI_MD_ATTR_RT = 0x8000000000000000UL;

struct efi_md {
	uint32_t	md_type;
	uint32_t	__pad;
	uint64_t	md_phys;
	uint64_t	md_virt;
	uint64_t	md_pages;
	uint64_t	md_attr;
};

struct efi_map_header {
	size_t		memory_size;
	size_t		descriptor_size;
	uint32_t	descriptor_version;
};

/* <sys/efi_map.h> */
using efi_map_entry_cb = void (*)(struct efi_md *, void *);

/* <machine/efi.h> */
inline constexpr int EFI_PAGE_SHIFT = 12;
inline constexpr int EFI_PAGE_SIZE = 1 << EFI_PAGE_SHIFT;

inline struct efi_md *
efi_next_descriptor(struct efi_md *md, size_t size)
{

	return ((struct efi_md *)(((uint8_t *)md) + size));
}

/* <sys/physmem.h> */
inline constexpr uint32_t EXFLAG_NODUMP = 0x0001;
inline constexpr uint32_t EXFLAG_NOALLOC = 0x0002;

/* <sys/param.h> */
template <class T, size_t N>
constexpr size_t
nitems(T (&)[N])
{

	return (N);
}

void
efi_map_foreach_entry(struct efi_map_header *efihdr, efi_map_entry_cb cb, void *argp)
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
handle_efi_map_entry(struct efi_md *p, void *argp)
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
efi_map_add_entries(struct efi_map_header *efihdr)
{
	bool exclude = false;
	efi_map_foreach_entry(efihdr, handle_efi_map_entry, &exclude);
}

void
efi_map_exclude_entries(struct efi_map_header *efihdr)
{
	bool exclude = true;
	efi_map_foreach_entry(efihdr, handle_efi_map_entry, &exclude);
}

void
print_efi_map_entry(struct efi_md *p, void *argp [[maybe_unused]])
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
	efi_test_printf("%23s %012jx %012jx %08jx ", type, (uintmax_t)p->md_phys,
	    (uintmax_t)p->md_virt, (uintmax_t)p->md_pages);
	if (p->md_attr & EFI_MD_ATTR_UC)
		efi_test_printf("UC ");
	if (p->md_attr & EFI_MD_ATTR_WC)
		efi_test_printf("WC ");
	if (p->md_attr & EFI_MD_ATTR_WT)
		efi_test_printf("WT ");
	if (p->md_attr & EFI_MD_ATTR_WB)
		efi_test_printf("WB ");
	if (p->md_attr & EFI_MD_ATTR_UCE)
		efi_test_printf("UCE ");
	if (p->md_attr & EFI_MD_ATTR_WP)
		efi_test_printf("WP ");
	if (p->md_attr & EFI_MD_ATTR_RP)
		efi_test_printf("RP ");
	if (p->md_attr & EFI_MD_ATTR_XP)
		efi_test_printf("XP ");
	if (p->md_attr & EFI_MD_ATTR_NV)
		efi_test_printf("NV ");
	if (p->md_attr & EFI_MD_ATTR_MORE_RELIABLE)
		efi_test_printf("MORE_RELIABLE ");
	if (p->md_attr & EFI_MD_ATTR_RO)
		efi_test_printf("RO ");
	if (p->md_attr & EFI_MD_ATTR_RT)
		efi_test_printf("RUNTIME");
	efi_test_printf("\n");
}

void
efi_map_print_entries(struct efi_map_header *efihdr)
{

	efi_test_printf("%23s %12s %12s %8s %4s\n",
	    "Type", "Physical", "Virtual", "#Pages", "Attr");
	efi_map_foreach_entry(efihdr, print_efi_map_entry, NULL);
}

} /* namespace pbsd::sys_kern::b0146s2 */
