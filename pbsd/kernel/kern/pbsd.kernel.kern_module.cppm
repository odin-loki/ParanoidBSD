module;

export module pbsd.kernel.kern_module;

export import pbsd.core;

/// Freestanding port of `kern/kern_module.c` — module helpers.
export namespace pbsd::kernel::kern_module {

inline constexpr int kModUnload = 0x0001;
inline constexpr int kModQuiet = 0x0002;
inline constexpr int kModForce = 0x0004;

[[nodiscard]] inline Status validate_load_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool force_unload(int flags) noexcept {
    return (flags & (kModUnload | kModForce)) == (kModUnload | kModForce);
}

} // namespace pbsd::kernel::kern_module
