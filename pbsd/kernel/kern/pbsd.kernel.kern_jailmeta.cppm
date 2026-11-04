module;

export module pbsd.kernel.kern_jailmeta;

export import pbsd.core;

/// Freestanding port of `kern/kern_jailmeta.c` — jailmeta helpers.
export namespace pbsd::kernel::kern_jailmeta {

inline constexpr unsigned kJmAttached = 0x0001;

[[nodiscard]] inline bool is_attached(unsigned flags) noexcept {
    return (flags & kJmAttached) != 0;
}

} // namespace pbsd::kernel::kern_jailmeta
