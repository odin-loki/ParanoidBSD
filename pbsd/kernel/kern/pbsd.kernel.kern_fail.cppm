module;

export module pbsd.kernel.kern_fail;

export import pbsd.core;

/// Freestanding port of `kern/kern_fail.c` — fail helpers.
export namespace pbsd::kernel::kern_fail {

inline constexpr unsigned kFailPointOnce = 0x0001;

[[nodiscard]] inline Status trigger(unsigned& count, unsigned flags) noexcept {
    if ((flags & kFailPointOnce) != 0 && count != 0) {
        return Status::Denied;
    }
    ++count;
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_fail
