module;

export module pbsd.kernel.subr_lock;

import pbsd.core;

/// Freestanding port of `kern/subr_lock.c` — lock object registry helpers.
export namespace pbsd::kernel::subr_lock {

inline constexpr unsigned kClassMax = 15;
inline constexpr unsigned kInitialized = 0x00010000u;

[[nodiscard]] inline bool is_initialized(unsigned flags) noexcept {
    return (flags & kInitialized) != 0;
}

[[nodiscard]] inline Status validate_class(int class_id) noexcept {
    if (class_id < 0 || class_id > static_cast<int>(kClassMax)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned mark_initialized(unsigned flags) noexcept {
    return flags | kInitialized;
}

} // namespace pbsd::kernel::subr_lock
