module;
#include <cstdint>

export module pbsd.kernel.power;

import pbsd.core;

/// Freestanding port of `kern/subr_power.c` power management registration.
export namespace pbsd::kernel::power {

inline constexpr unsigned kPmTypeNone = 0;

enum class Command : unsigned {
    Suspend = 1,
    Resume  = 2,
    Standby = 3,
};

struct Registration {
    unsigned pm_type{kPmTypeNone};
    bool     registered{};
};

[[nodiscard]] inline Status register_pm(Registration& reg, unsigned pm_type) noexcept {
    if (reg.registered) {
        return Status::Busy;
    }
    reg.pm_type = pm_type;
    reg.registered = true;
    return Status::Ok;
}

[[nodiscard]] inline Status unregister_pm(Registration& reg) noexcept {
    if (!reg.registered) {
        return Status::Invalid;
    }
    reg.registered = false;
    reg.pm_type = kPmTypeNone;
    return Status::Ok;
}

[[nodiscard]] inline Status validate_command(Command cmd) noexcept {
    switch (cmd) {
    case Command::Suspend:
    case Command::Resume:
    case Command::Standby:
        return Status::Ok;
    }
    return Status::Invalid;
}

} // namespace pbsd::kernel::power
