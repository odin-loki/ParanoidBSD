module;

export module pbsd.kernel.kern_ucoredump;

export import pbsd.core;

/// Freestanding port of `kern/kern_ucoredump.c` — ucoredump helpers.
export namespace pbsd::kernel::kern_ucoredump {

inline constexpr unsigned kUcoreActive = 0x0001;

[[nodiscard]] inline bool is_active(unsigned flags) noexcept {
    return (flags & kUcoreActive) != 0;
}

} // namespace pbsd::kernel::kern_ucoredump
