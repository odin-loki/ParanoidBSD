module;
#include <cstdint>

export module pbsd.bifrost.mtrr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_mtrr.c
export namespace pbsd::bifrost::mtrr {

enum class Type : unsigned char {
    Uncacheable = 0,
    WriteCombining = 1,
    WriteThrough = 4,
    WriteProtected = 5,
    WriteBack = 6,
};

struct Range {
    std::uint64_t base{};
    std::uint64_t mask{};
    Type type{Type::WriteBack};
};

[[nodiscard]] inline Status validate_range(const Range& r) noexcept {
    if (r.base == 0 || r.mask == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::mtrr
