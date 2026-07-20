module;

export module pbsd.kernel.kern_cpu;

export import pbsd.core;

/// Freestanding port of `kern/kern_cpu.c` — cpu helpers.
export namespace pbsd::kernel::kern_cpu {

inline constexpr unsigned kCpuOnline = 0x0001;
inline constexpr unsigned kCpuPresent = 0x0002;
inline constexpr unsigned kCpuBsp = 0x0004;

[[nodiscard]] inline bool is_online(unsigned state) noexcept {
    return (state & kCpuOnline) != 0;
}

[[nodiscard]] inline bool is_bsp(unsigned state) noexcept {
    return (state & kCpuBsp) != 0;
}

} // namespace pbsd::kernel::kern_cpu
