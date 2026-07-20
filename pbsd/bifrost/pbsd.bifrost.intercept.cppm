module;
#include <cstdint>

export module pbsd.bifrost.intercept;

import pbsd.core;

/// PROVENANCE: BIFROST HV — VM-exit intercept bitmap scaffold.
export namespace pbsd::bifrost::intercept {

enum class Kind : unsigned char {
    Cpuid = 0,
    Msr = 1,
    Io = 2,
    Hlt = 3,
    TripleFault = 4,
};

struct Bitmap {
    std::uint32_t bits{};
};

[[nodiscard]] inline Status enable(Bitmap& b, Kind k) noexcept {
    const auto bit = static_cast<unsigned>(k);
    if (bit >= 32) {
        return Status::Invalid;
    }
    b.bits |= (1u << bit);
    return Status::Ok;
}

[[nodiscard]] inline Status disable(Bitmap& b, Kind k) noexcept {
    const auto bit = static_cast<unsigned>(k);
    if (bit >= 32) {
        return Status::Invalid;
    }
    b.bits &= ~(1u << bit);
    return Status::Ok;
}

[[nodiscard]] inline bool is_enabled(const Bitmap& b, Kind k) noexcept {
    const auto bit = static_cast<unsigned>(k);
    if (bit >= 32) {
        return false;
    }
    return (b.bits & (1u << bit)) != 0;
}

} // namespace pbsd::bifrost::intercept
