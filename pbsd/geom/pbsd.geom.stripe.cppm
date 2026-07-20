module;
#include <cstdint>

export module pbsd.geom.stripe;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/stripe/g_stripe.h — GEOM_STRIPE constants.
export namespace pbsd::geom::stripe {

inline constexpr unsigned kClassName = 0x53545250; // "STRP"

enum class State : unsigned char {
    New    = 0,
    Active = 1,
    Failed = 2,
};

[[nodiscard]] inline Status validate_width(unsigned width) noexcept {
    if (width < 2) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::New:
    case State::Active:
    case State::Failed:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::stripe
