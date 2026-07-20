module;

export module pbsd.kernel.kern_procctl;

export import pbsd.core;

/// Freestanding port of `kern/kern_procctl.c` — procctl helpers.
export namespace pbsd::kernel::kern_procctl {

inline constexpr int kProcctlSetProtection = 1;
inline constexpr int kProcctlClearProtection = 2;
inline constexpr int kProcctlMaxId = 3;

[[nodiscard]] inline Status validate_id(int id) noexcept {
    if (id < 0 || id > kProcctlMaxId) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_procctl
