module;
#include <cstdint>

export module pbsd.kernel.smr;

export import pbsd.core;

/// Wave 4/5 — safe memory reclamation from sys/smr.h, kern/subr_smr.c.
export namespace pbsd::kernel::smr {

inline constexpr unsigned kSmrTierMin = 0;
inline constexpr unsigned kSmrTierMax = 3;

enum class SmrState : unsigned char {
    Idle = 0,
    Entered = 1,
    Pending = 2,
};

struct SmrTier {
    unsigned id{};
    unsigned generation{};
    SmrState state{SmrState::Idle};
};

[[nodiscard]] constexpr Status validate_tier(unsigned tier) noexcept {
    if (tier > kSmrTierMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool can_reclaim(SmrState state) noexcept {
    return state == SmrState::Idle;
}

[[nodiscard]] constexpr unsigned next_generation(unsigned gen) noexcept {
    return gen + 1;
}

} // namespace pbsd::kernel::smr
