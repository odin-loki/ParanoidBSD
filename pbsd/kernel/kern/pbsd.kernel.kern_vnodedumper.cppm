module;

export module pbsd.kernel.kern_vnodedumper;

export import pbsd.core;

/// Freestanding port of `kern/kern_vnodedumper.c` — vnodedumper helpers.
export namespace pbsd::kernel::kern_vnodedumper {

inline constexpr unsigned kVndMaxName = 256;

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    return len > 0 && len < kVndMaxName ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_vnodedumper
