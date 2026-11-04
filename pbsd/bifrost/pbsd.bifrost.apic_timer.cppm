module;
#include <cstdint>

export module pbsd.bifrost.apic_timer;

import pbsd.core;

/// PROVENANCE: BIFROST HV — guest LAPIC timer arming scaffold.
export namespace pbsd::bifrost::apic_timer {

struct Timer {
    std::uint32_t initial{};
    std::uint32_t current{};
    bool periodic{false};
    bool armed{false};
};

[[nodiscard]] inline Status arm(Timer& t, std::uint32_t ticks, bool periodic) noexcept {
    if (ticks == 0) {
        return Status::Invalid;
    }
    t.initial = ticks;
    t.current = ticks;
    t.periodic = periodic;
    t.armed = true;
    return Status::Ok;
}

[[nodiscard]] inline Status tick(Timer& t) noexcept {
    if (!t.armed || t.current == 0) {
        return Status::Invalid;
    }
    --t.current;
    if (t.current == 0) {
        if (t.periodic) {
            t.current = t.initial;
        } else {
            t.armed = false;
        }
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::apic_timer
