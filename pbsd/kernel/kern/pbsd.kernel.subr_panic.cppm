module;
#include <cstdint>

export module pbsd.kernel.subr_panic;

import pbsd.core;
import pbsd.kernel.panic;

/// PROVENANCE: hbsd/src/sys/kern/kern_shutdown.c — panic/reboot subr.
export namespace pbsd::kernel::subr_panic {

[[nodiscard]] inline Status enter(panic::State& s, panic::Reason r,
                                  std::uint32_t cpu) noexcept {
    return panic::begin(s, r, cpu);
}

[[nodiscard]] inline Status schedule_reboot(panic::State& s) noexcept {
    return panic::request_reboot(s);
}

[[nodiscard]] inline bool is_active(panic::State const& s) noexcept {
    return s.reason != panic::Reason::None;
}

} // namespace pbsd::kernel::subr_panic
