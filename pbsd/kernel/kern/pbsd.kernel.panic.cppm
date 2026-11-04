module;
#include <cstdint>

export module pbsd.kernel.panic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/kern_shutdown.c — panic / shutdown reason codes.
export namespace pbsd::kernel::panic {

enum class Reason : unsigned char {
    None = 0,
    Assert = 1,
    Trap = 2,
    OutOfMemory = 3,
    Deadlock = 4,
    UserRequest = 5,
};

struct State {
    Reason reason{Reason::None};
    bool dumping{false};
    bool rebooting{false};
    std::uint32_t cpu{};
};

[[nodiscard]] inline Status begin(State& s, Reason r, std::uint32_t cpu) noexcept {
    if (r == Reason::None) {
        return Status::Invalid;
    }
    s.reason = r;
    s.cpu = cpu;
    s.dumping = true;
    return Status::Ok;
}

[[nodiscard]] inline Status request_reboot(State& s) noexcept {
    if (s.reason == Reason::None) {
        return Status::Invalid;
    }
    s.rebooting = true;
    return Status::Ok;
}

} // namespace pbsd::kernel::panic
