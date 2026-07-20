module;

export module pbsd.kernel.kern_loginclass;

export import pbsd.core;

/// Freestanding port of `kern/kern_loginclass.c` — loginclass helpers.
export namespace pbsd::kernel::kern_loginclass {

inline constexpr unsigned kLcResource = 0x0001;

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    return len > 0 && len < 256 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_loginclass
