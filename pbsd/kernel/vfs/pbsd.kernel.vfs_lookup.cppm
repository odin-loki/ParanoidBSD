module;

export module pbsd.kernel.vfs_lookup;

export import pbsd.core;
import pbsd.kernel.namei;

/// Freestanding port of `kern/vfs_lookup.c` — namei lookup states.
export namespace pbsd::kernel::vfs_lookup {

inline constexpr unsigned kLookupShared = 0x0001;
inline constexpr unsigned kLookupRoot = 0x0002;
inline constexpr unsigned kLookupCrossmount = 0x0004;

[[nodiscard]] inline bool is_shared(unsigned flags) noexcept {
    return (flags & kLookupShared) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (flags & ~(kLookupShared | kLookupRoot | kLookupCrossmount)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_lookup
