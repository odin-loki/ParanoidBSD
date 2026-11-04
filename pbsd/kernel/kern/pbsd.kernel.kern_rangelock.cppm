module;

export module pbsd.kernel.kern_rangelock;

export import pbsd.core;

/// Freestanding port of `kern/kern_rangelock.c` — rangelock helpers.
export namespace pbsd::kernel::kern_rangelock {

inline constexpr unsigned kRlShared = 0x0001;
inline constexpr unsigned kRlExclusive = 0x0002;

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    const unsigned acc = mode & (kRlShared | kRlExclusive);
    if (acc != kRlShared && acc != kRlExclusive) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_rangelock
