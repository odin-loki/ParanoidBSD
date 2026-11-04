module;

export module pbsd.kernel.mac_process;

import pbsd.core;

/// Freestanding port of `security/mac/mac_process.c` — process MAC checks.
export namespace pbsd::kernel::mac_process {

enum class Check : unsigned char {
    Exec = 0,
    Fork = 1,
    Signal = 2,
};

[[nodiscard]] inline Status validate_check(Check c) noexcept {
    switch (c) {
    case Check::Exec:
    case Check::Fork:
    case Check::Signal:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status check_transition(Check c, bool allowed) noexcept {
    if (validate_check(c) != Status::Ok) {
        return Status::Invalid;
    }
    return allowed ? Status::Ok : Status::Denied;
}

} // namespace pbsd::kernel::mac_process
