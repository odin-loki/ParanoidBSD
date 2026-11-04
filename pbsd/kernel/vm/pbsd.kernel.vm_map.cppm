module;
#include <cstdint>

export module pbsd.kernel.vm_map;

import pbsd.core;
import pbsd.kernel.vm;

/// Freestanding port of `vm_map.h` entry flags + split-boundary helpers.
export namespace pbsd::kernel::vm_map {

using namespace pbsd::kernel::vm;

inline constexpr unsigned kMapEntryNosync       = 0x00000001u;
inline constexpr unsigned kMapEntryIsSubMap     = 0x00000002u;
inline constexpr unsigned kMapEntryCow          = 0x00000004u;
inline constexpr unsigned kMapEntryNeedsCopy    = 0x00000008u;
inline constexpr unsigned kMapEntryNofault      = 0x00000010u;
inline constexpr unsigned kMapEntryUserWired    = 0x00000020u;
inline constexpr unsigned kMapEntryBehavSequential = 0x00000040u;
inline constexpr unsigned kMapEntryBehavRandom  = 0x00000080u;
inline constexpr unsigned kMapEntryBehavMask    = 0x000000c0u;
inline constexpr unsigned kMapEntryGuard        = 0x00010000u;
inline constexpr unsigned kMapEntryStackGap     = 0x00020000u;
inline constexpr unsigned kMapEntryWritecnt     = 0x00008000u;

inline constexpr unsigned kMapWirefuture = 0x00000001u;
inline constexpr unsigned kMapIsSubMap   = 0x00000004u;

inline constexpr unsigned kSplitBoundaryShift = 20u;
inline constexpr unsigned kSplitBoundaryMask  = 0x00300000u;

[[nodiscard]] constexpr unsigned split_boundary_index(unsigned ef) noexcept {
    return (ef & kSplitBoundaryMask) >> kSplitBoundaryShift;
}

[[nodiscard]] constexpr bool is_guard_entry(unsigned ef) noexcept {
    return (ef & kMapEntryGuard) != 0;
}

[[nodiscard]] constexpr bool is_cow_entry(unsigned ef) noexcept {
    return (ef & kMapEntryCow) != 0;
}

[[nodiscard]] constexpr bool is_stack_gap(unsigned ef) noexcept {
    return (ef & kMapEntryStackGap) != 0;
}

struct MapEntry {
    std::uint64_t start{};
    std::uint64_t end{};
    unsigned      ef{};
    unsigned char max_prot{kVmProtAll};
    unsigned char cur_prot{kVmProtNone};
};

/// `vm_map_protect` — new prot must fit entry max and W^X policy.
[[nodiscard]] inline Status protect_entry(MapEntry& e, unsigned char new_prot) noexcept {
    if (is_guard_entry(e.ef) && new_prot != kVmProtNone) {
        return Status::Denied;
    }
    if (validate_wx_vm(new_prot) != Status::Ok) {
        return Status::Denied;
    }
    const unsigned char max = entry_max_protection(
        merge_entry_offset(e.cur_prot, e.max_prot), e.max_prot);
    if ((new_prot & ~max) != 0) {
        return Status::Denied;
    }
    e.cur_prot = new_prot;
    return Status::Ok;
}

/// `vm_map_stack` — stack entries require RW and MAP_STACK semantics.
[[nodiscard]] constexpr Status validate_stack_prot(unsigned char prot) noexcept {
    if ((prot & (kProtRead | kProtWrite)) != (kProtRead | kProtWrite)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vm_map
