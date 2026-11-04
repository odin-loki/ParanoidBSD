module;

export module pbsd.kernel.kern_ntptime;

export import pbsd.core;

/// Freestanding port of `kern/kern_ntptime.c` — ntptime helpers.
export namespace pbsd::kernel::kern_ntptime {

inline constexpr int kModOffset = 0x0001;
inline constexpr int kModFrequency = 0x0002;
inline constexpr int kModStatus = 0x0010;
inline constexpr int kModTai = 0x0080;

[[nodiscard]] inline Status validate_modes(int modes) noexcept {
    if (modes < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_ntptime
