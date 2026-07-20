module;

export module pbsd.kernel.p1003_1b;

import pbsd.core;

/// Freestanding port of `kern/p1003_1b.c` — POSIX.1b realtime helpers.
export namespace pbsd::kernel::p1003_1b {

enum class Feature : unsigned char {
    Aio = 0,
    Mqueue = 1,
    Sem = 2,
    Shm = 3,
};

[[nodiscard]] inline Status validate_feature(Feature f) noexcept {
    switch (f) {
    case Feature::Aio:
    case Feature::Mqueue:
    case Feature::Sem:
    case Feature::Shm:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status runtime_supported(Feature f, bool enabled) noexcept {
    if (validate_feature(f) != Status::Ok) {
        return Status::Invalid;
    }
    return enabled ? Status::Ok : Status::NotImplemented;
}

} // namespace pbsd::kernel::p1003_1b
