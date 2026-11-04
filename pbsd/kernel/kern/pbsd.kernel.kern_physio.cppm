module;

export module pbsd.kernel.kern_physio;

export import pbsd.core;

/// Freestanding port of `kern/kern_physio.c` — physio helpers.
export namespace pbsd::kernel::kern_physio {

inline constexpr unsigned kBAsync = 0x0001;
inline constexpr unsigned kBRead = 0x0002;
inline constexpr unsigned kBWrite = 0x0004;

[[nodiscard]] inline bool is_async(unsigned flags) noexcept {
    return (flags & kBAsync) != 0;
}

[[nodiscard]] inline Status validate_io(unsigned flags) noexcept {
    const unsigned io = flags & (kBRead | kBWrite);
    if (io != kBRead && io != kBWrite) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_physio
