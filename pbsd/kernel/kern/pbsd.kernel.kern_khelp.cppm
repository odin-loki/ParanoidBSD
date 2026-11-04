module;

export module pbsd.kernel.kern_khelp;

export import pbsd.core;

/// Freestanding port of `kern/kern_khelp.c` — khelp helpers.
export namespace pbsd::kernel::kern_khelp {

inline constexpr unsigned kKhMaxModules = 16;

[[nodiscard]] inline Status validate_module(unsigned idx) noexcept {
    return idx < kKhMaxModules ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_khelp
