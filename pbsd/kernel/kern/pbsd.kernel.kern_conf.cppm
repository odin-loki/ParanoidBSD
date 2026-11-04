module;

export module pbsd.kernel.kern_conf;

export import pbsd.core;

/// Freestanding port of `kern/kern_conf.c` — conf helpers.
export namespace pbsd::kernel::kern_conf {

inline constexpr unsigned kDOpen = 0x0001;
inline constexpr unsigned kDClose = 0x0002;
inline constexpr unsigned kDRead = 0x0004;
inline constexpr unsigned kDWrite = 0x0008;

[[nodiscard]] inline Status validate_devsw_flags(unsigned flags) noexcept {
    if ((flags & (kDOpen | kDClose | kDRead | kDWrite)) == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_conf
