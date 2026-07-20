module;
#include <cstdint>

export module pbsd.geom.shred;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/shred — secure-erase pass planner.
export namespace pbsd::geom::shred {

inline constexpr unsigned kDefaultPasses = 3;
inline constexpr unsigned kMaxPasses = 35;

enum class Pattern : unsigned char {
    Zeros = 0,
    Ones = 1,
    Random = 2,
    Gutmann = 3,
};

struct Plan {
    unsigned passes{kDefaultPasses};
    Pattern pattern{Pattern::Random};
    bool verify{false};
};

[[nodiscard]] inline Status validate_plan(const Plan& p) noexcept {
    if (p.passes == 0 || p.passes > kMaxPasses) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Plan default_plan() noexcept {
    return Plan{};
}

[[nodiscard]] inline std::uint64_t bytes_touched(std::uint64_t media_bytes,
                                                 const Plan& p) noexcept {
    if (validate_plan(p) != Status::Ok) {
        return 0;
    }
    return media_bytes * p.passes;
}

} // namespace pbsd::geom::shred
