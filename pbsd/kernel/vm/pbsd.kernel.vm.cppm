module;
#include <cstddef>
#include <cstdint>

export module pbsd.kernel.vm;

export import pbsd.handles;
import pbsd.core;
import pbsd.kernel.capsicum;

/// VM mapping tokens with real hbsd flag tables (`vm.h`, `mman.h`, `vm_map.c`).
export namespace pbsd::kernel::vm {

// vm.h — vm_prot_t
inline constexpr unsigned char kVmProtNone    = 0x00u;
inline constexpr unsigned char kVmProtRead    = 0x01u;
inline constexpr unsigned char kVmProtWrite   = 0x02u;
inline constexpr unsigned char kVmProtExecute = 0x04u;
inline constexpr unsigned char kVmProtCopy    = 0x08u;
inline constexpr unsigned char kVmProtAll     = kVmProtRead | kVmProtWrite | kVmProtExecute;
inline constexpr unsigned char kVmProtRw      = kVmProtRead | kVmProtWrite;

// mman.h — PROT_* / MAP_* (vm_mmap.c relies on VM_PROT_* == PROT_*)
inline constexpr unsigned char kProtNone    = 0x00u;
inline constexpr unsigned char kProtRead    = 0x01u;
inline constexpr unsigned char kProtWrite   = 0x02u;
inline constexpr unsigned char kProtExec    = 0x04u;
inline constexpr unsigned char kProtAll     = kProtRead | kProtWrite | kProtExec;

inline constexpr unsigned kMapShared         = 0x0001u;
inline constexpr unsigned kMapPrivate        = 0x0002u;
inline constexpr unsigned kMapFixed          = 0x0010u;
inline constexpr unsigned kMapHassemaphore   = 0x0200u;
inline constexpr unsigned kMapStack          = 0x0400u;
inline constexpr unsigned kMapNosync         = 0x0800u;
inline constexpr unsigned kMapAnon           = 0x1000u;
inline constexpr unsigned kMapGuard          = 0x00002000u;
inline constexpr unsigned kMapExcl           = 0x00004000u;
inline constexpr unsigned kMapNocore         = 0x00020000u;
inline constexpr unsigned kMapPrefaultRead   = 0x00040000u;
inline constexpr unsigned kMap32bit          = 0x00080000u;
inline constexpr unsigned kMapAlignmentShift = 24u;
inline constexpr unsigned kMapAlignmentMask  = 0xff000000u;
inline constexpr unsigned kMapAlignedSuper   = 1u << kMapAlignmentShift;
inline constexpr unsigned kMapReserved0020   = 0x0020u;
inline constexpr unsigned kMapReserved0040   = 0x0040u;

inline constexpr unsigned kMapAllowedMask =
    kMapShared | kMapPrivate | kMapFixed | kMapHassemaphore | kMapStack |
    kMapNosync | kMapAnon | kMapExcl | kMapNocore | kMapPrefaultRead |
    kMapGuard | kMap32bit | kMapAlignmentMask;

// mman.h — PROT_MAX encoding in vm_map entry offset field
inline constexpr unsigned kProtMaxShift = 16u;
inline constexpr unsigned char kProtMaxMask =
    kVmProtRead | kVmProtWrite | kVmProtExecute;

[[nodiscard]] constexpr unsigned prot_max(unsigned char prot) noexcept {
    return static_cast<unsigned>(prot) << kProtMaxShift;
}

[[nodiscard]] constexpr unsigned char prot_max_extract(unsigned encoded) noexcept {
    return static_cast<unsigned char>((encoded >> kProtMaxShift) & kProtMaxMask);
}

[[nodiscard]] constexpr unsigned char prot_current_extract(unsigned encoded) noexcept {
    return static_cast<unsigned char>(encoded & kProtMaxMask);
}

struct MmapRequest {
    unsigned char prot{kProtNone};
    unsigned      flags{kMapPrivate};
    int           fd{-1};
    std::uint64_t offset{};
    std::size_t   length{};
};

/// `kern_mmap` PROT bit validation — vm_mmap.c:209-214.
[[nodiscard]] inline Status validate_mmap_prot(unsigned char prot) noexcept {
    const unsigned char max = prot_max_extract(prot);
    const unsigned char cur = prot_current_extract(prot);
    if (max != 0 && (max & cur) != cur) {
        return Status::Invalid;
    }
    return Status::Ok;
}

/// `kern_mmap` flag constraints — vm_mmap.c:241-292.
[[nodiscard]] inline Status validate_mmap_flags(MmapRequest req) noexcept {
    unsigned flags = req.flags;
    flags &= ~(kMapReserved0020 | kMapReserved0040);

    if (req.length == 0) {
        return Status::Invalid;
    }
    if ((flags & kMapAnon) != 0) {
        if (req.fd != -1 || req.offset != 0) {
            return Status::Invalid;
        }
    }
    if ((flags & kMapStack) != 0) {
        if (req.fd != -1 ||
            (req.prot & (kProtRead | kProtWrite)) != (kProtRead | kProtWrite)) {
            return Status::Invalid;
        }
    }
    if ((flags & ~kMapAllowedMask) != 0) {
        return Status::Invalid;
    }
    if ((flags & (kMapExcl | kMapFixed)) == kMapExcl) {
        return Status::Invalid;
    }
    if ((flags & (kMapShared | kMapPrivate)) == (kMapShared | kMapPrivate)) {
        return Status::Invalid;
    }
    if ((flags & kMapGuard) != 0) {
        if (req.prot != kProtNone || req.fd != -1 || req.offset != 0 ||
            (flags & ~(kMapFixed | kMapGuard | kMapExcl | kMap32bit |
                       kMapAlignmentMask)) != 0) {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

/// `vm_mmap.c` — derive cap rights from mmap prot + shared flag.
[[nodiscard]] inline capsicum::CapRights mmap_prot_to_cap_rights(unsigned char prot,
                                                                 unsigned flags) noexcept {
    capsicum::CapRights rights = capsicum::cap_none();
    if ((prot & kProtRead) != 0) {
        rights = capsicum::cap_rights_set_one(rights, capsicum::kCapMmapR);
    }
    if ((flags & kMapShared) != 0 && (prot & kProtWrite) != 0) {
        rights = capsicum::cap_rights_set_one(rights, capsicum::kCapMmapW);
    }
    if ((prot & kProtExec) != 0) {
        rights = capsicum::cap_rights_set_one(rights, capsicum::kCapMmapX);
    }
    return rights;
}

enum class MapProtection : unsigned char {
    None    = kVmProtNone,
    Read    = kVmProtRead,
    Write   = kVmProtWrite,
    Execute = kVmProtExecute,
    Copy    = kVmProtCopy,
};

[[nodiscard]] constexpr MapProtection operator|(MapProtection a, MapProtection b) noexcept {
    return static_cast<MapProtection>(static_cast<unsigned char>(a)
                                    | static_cast<unsigned char>(b));
}

[[nodiscard]] constexpr bool has_prot(MapProtection set, MapProtection bit) noexcept {
    return (static_cast<unsigned char>(set) & static_cast<unsigned char>(bit)) != 0;
}

[[nodiscard]] constexpr MapProtection from_vm_prot(unsigned char vm_prot) noexcept {
    return static_cast<MapProtection>(vm_prot & kProtAll);
}

[[nodiscard]] constexpr unsigned char to_vm_prot(MapProtection prot) noexcept {
    return static_cast<unsigned char>(prot) & kProtAll;
}

/// SI-3 / PaX MPROTECT: reject simultaneous W+X (also catches CAP_MMAP_WX).
[[nodiscard]] constexpr Status validate_wx(MapProtection prot) noexcept {
    if (has_prot(prot, MapProtection::Write) && has_prot(prot, MapProtection::Execute)) {
        return Status::Denied;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_wx_vm(unsigned char vm_prot) noexcept {
    return validate_wx(from_vm_prot(vm_prot));
}

/// `vm_map.c` entry offset merge — current prot | PROT_MAX(max).
[[nodiscard]] constexpr unsigned merge_entry_offset(unsigned char current_prot,
                                                         unsigned char max_prot) noexcept {
    return static_cast<unsigned>(current_prot) | prot_max(max_prot);
}

/// `vm_map.c` protection change — extract max from entry offset.
[[nodiscard]] constexpr unsigned char entry_max_protection(unsigned entry_offset,
                                                           unsigned char fallback_max) noexcept {
    const unsigned char extracted = prot_max_extract(entry_offset);
    return extracted != 0 ? extracted : fallback_max;
}

/// `sys_capability.c:cap_rights_to_vmprot` bridge.
[[nodiscard]] inline MapProtection from_cap_rights(const capsicum::CapRights& rights) noexcept {
    return from_vm_prot(capsicum::cap_rights_to_vmprot(rights));
}

using MapToken = unsigned long long;
inline constexpr MapToken kInvalidMapToken = 0;

struct MapGrant {
    MapToken       token{kInvalidMapToken};
    MapProtection  prot{MapProtection::None};
    unsigned char  max_prot{kVmProtNone};
    std::uintptr_t base{};
    std::size_t    length{};
};

class VmMapper {
public:
    [[nodiscard]] Result<MapGrant> map(VmHandle& obj, MapProtection prot,
                                       std::uintptr_t base, std::size_t length,
                                       unsigned char max_prot = kVmProtAll) noexcept {
        if (!obj.valid() || !obj.has_right(CapabilityRights::Map)) {
            return {Status::Denied, MapGrant{}};
        }
        if (validate_wx(prot) != Status::Ok) {
            return {Status::Denied, MapGrant{}};
        }
        // max_prot is a ceiling (PROT_MAX), not the active mapping — SI-3 applies per page.
        const unsigned encoded = merge_entry_offset(to_vm_prot(prot), max_prot);
        (void)encoded;
        MapGrant grant{
            .token     = next_token_++,
            .prot      = prot,
            .max_prot  = max_prot,
            .base      = base,
            .length    = length,
        };
        return {Status::Ok, grant};
    }

    [[nodiscard]] Result<MapGrant> map_from_cap(VmHandle& obj,
                                                const capsicum::CapRights& rights,
                                                std::uintptr_t base,
                                                std::size_t length) noexcept {
        const MapProtection prot = from_cap_rights(rights);
        return map(obj, prot, base, length, capsicum::cap_rights_to_vmprot(rights));
    }

    [[nodiscard]] Status change_protection(MapGrant& grant, MapProtection new_prot) noexcept {
        if (grant.token == kInvalidMapToken) {
            return Status::Invalid;
        }
        if (validate_wx(new_prot) != Status::Ok) {
            return Status::Denied;
        }
        const unsigned char new_vm = to_vm_prot(new_prot);
        if ((new_vm & ~grant.max_prot) != 0) {
            return Status::Denied;
        }
        grant.prot = new_prot;
        return Status::Ok;
    }

    [[nodiscard]] Status unmap(MapToken token) noexcept {
        if (token == kInvalidMapToken) {
            return Status::Invalid;
        }
        (void)token;
        return Status::Ok;
    }

private:
    MapToken next_token_{1};
};

namespace proofs {
static_assert(prot_max_extract(prot_max(kVmProtRead | kVmProtWrite)) ==
              (kVmProtRead | kVmProtWrite));
static_assert(validate_wx(MapProtection::Write | MapProtection::Execute) == Status::Denied);
} // namespace proofs

} // namespace pbsd::kernel::vm
