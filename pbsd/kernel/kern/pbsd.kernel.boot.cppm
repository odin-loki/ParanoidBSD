module;

export module pbsd.kernel.boot;

export import pbsd.core;

/// Freestanding port of `sys/boot.h` / `kern/subr_boot.c`.
export namespace pbsd::kernel::boot {

inline constexpr unsigned kBootNormal = 0;
inline constexpr unsigned kBootSingle = 1;
inline constexpr unsigned kBootVerbose = 2;
inline constexpr unsigned kBootQuiet = 4;

[[nodiscard]] inline bool is_single(unsigned howto) noexcept {
    return (howto & kBootSingle) != 0;
}

[[nodiscard]] inline Status validate_howto(unsigned howto) noexcept {
    (void)howto;
    return Status::Ok;
}

} // namespace pbsd::kernel::boot
