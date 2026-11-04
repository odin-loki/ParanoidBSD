module;

export module pbsd.kernel.kern_devctl;

export import pbsd.core;

/// Freestanding port of `kern/kern_devctl.c` — devctl helpers.
export namespace pbsd::kernel::kern_devctl {

inline constexpr unsigned kDevctlMaxName = 256;

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kDevctlMaxName) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_devctl
