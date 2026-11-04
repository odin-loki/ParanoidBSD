module;
#include <cstdint>

export module pbsd.kernel.vm_map_helpers;

import pbsd.core;
import pbsd.kernel.vm;
import pbsd.kernel.vm_map;

/// Freestanding vm_map(9) insertion/split helpers from `vm_map.c` policy surface.
export namespace pbsd::kernel::vm_map_helpers {

using namespace pbsd::kernel::vm;
using namespace pbsd::kernel::vm_map;

struct InsertParams {
    std::uint64_t start{};
    std::uint64_t end{};
    unsigned      ef{};
    unsigned char prot{kVmProtAll};
};

[[nodiscard]] inline Status validate_insert(const InsertParams& p) noexcept {
    if (p.end <= p.start) {
        return Status::Invalid;
    }
    if (validate_wx_vm(p.prot) != Status::Ok) {
        return Status::Denied;
    }
    if (is_guard_entry(p.ef) && p.prot != kVmProtNone) {
        return Status::Denied;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status split_at(MapEntry& e, std::uint64_t split) noexcept {
    if (split <= e.start || split >= e.end) {
        return Status::Invalid;
    }
    (void)split_boundary_index(e.ef);
    return Status::Ok;
}

[[nodiscard]] inline bool entries_overlap(const MapEntry& a,
                                          const MapEntry& b) noexcept {
    return a.start < b.end && b.start < a.end;
}

[[nodiscard]] inline Status merge_compatible(const MapEntry& a,
                                             const MapEntry& b) noexcept {
    if (a.ef != b.ef || a.cur_prot != b.cur_prot || a.max_prot != b.max_prot) {
        return Status::Protocol;
    }
    if (a.end != b.start) {
        return Status::NotFound;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vm_map_helpers
