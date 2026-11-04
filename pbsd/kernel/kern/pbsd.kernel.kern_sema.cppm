module;

export module pbsd.kernel.kern_sema;

export import pbsd.core;

/// Freestanding port of `kern/kern_sema.c` — sema helpers.
export namespace pbsd::kernel::kern_sema {

inline constexpr unsigned kSemValueMax = 65535;

[[nodiscard]] inline Status validate_value(unsigned value) noexcept {
    return value <= kSemValueMax ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_sema
