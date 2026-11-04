module;

export module pbsd.kernel.kern_ctf;

export import pbsd.core;

/// Freestanding port of `kern/kern_ctf.c` — ctf helpers.
export namespace pbsd::kernel::kern_ctf {

inline constexpr unsigned kCtfMaxSections = 64;

[[nodiscard]] inline Status validate_section(unsigned idx) noexcept {
    return idx < kCtfMaxSections ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_ctf
