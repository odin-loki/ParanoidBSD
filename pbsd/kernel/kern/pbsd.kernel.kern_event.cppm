module;

export module pbsd.kernel.kern_event;

export import pbsd.core;

/// Freestanding port of `kern/kern_event.c` — event helpers.
export namespace pbsd::kernel::kern_event {

inline constexpr unsigned kEvAdd = 0x0001;
inline constexpr unsigned kEvDelete = 0x0002;
inline constexpr unsigned kEvEnable = 0x0004;
inline constexpr unsigned kEvDisable = 0x0008;
inline constexpr unsigned kEvOneshot = 0x0010;

[[nodiscard]] inline Status validate_fflags(unsigned fflags) noexcept {
    if (fflags & ~(kEvAdd | kEvDelete | kEvEnable | kEvDisable | kEvOneshot)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_event
